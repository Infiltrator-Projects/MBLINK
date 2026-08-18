// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "MBLinkBLETransport+MBLINK.h"
#import "mblink/elm327.h"
#import "mblink/elm327_session.h"
#import "mblink/obd2.h"
#import "mblink/scheduler.h"
#import "mblink/telemetry.h"

typedef NS_ENUM(NSInteger, MBLinkDiagnosticsPhase) {
    MBLinkDiagnosticsPhaseIdle = 0,
    MBLinkDiagnosticsPhaseInitializing,
    MBLinkDiagnosticsPhaseCheckingPids,
    MBLinkDiagnosticsPhaseReadingLive,
    MBLinkDiagnosticsPhaseLive,
    MBLinkDiagnosticsPhaseFailed
};

@interface MBLinkDiagnosticsController () <MBLinkBLETransportDelegate>
@property(nonatomic, copy, readwrite) NSString *statusText;
@property(nonatomic, copy, readwrite, nullable) NSString *peripheralName;
@property(nonatomic, copy, readwrite, nullable) NSString *adapterIdentifier;
@property(nonatomic, readwrite, getter=isActive) BOOL active;
@property(nonatomic, readwrite, getter=isReady) BOOL ready;

@property(nonatomic, readwrite) BOOL hasEngineLoad;
@property(nonatomic, readwrite) double engineLoadPercent;
@property(nonatomic, readwrite) BOOL hasCoolantTemperature;
@property(nonatomic, readwrite) double coolantTemperatureCelsius;
@property(nonatomic, readwrite) BOOL hasManifoldPressure;
@property(nonatomic, readwrite) double manifoldPressureKPa;
@property(nonatomic, readwrite) BOOL hasRPM;
@property(nonatomic, readwrite) double rpm;
@property(nonatomic, readwrite) BOOL hasVehicleSpeed;
@property(nonatomic, readwrite) double vehicleSpeedKmh;
@property(nonatomic, readwrite) BOOL hasIntakeAirTemperature;
@property(nonatomic, readwrite) double intakeAirTemperatureCelsius;
@property(nonatomic, readwrite) BOOL hasMassAirFlow;
@property(nonatomic, readwrite) double massAirFlowGramsPerSecond;
@property(nonatomic, readwrite) BOOL hasThrottlePosition;
@property(nonatomic, readwrite) double throttlePositionPercent;

- (void)handleSessionEvent:(const MblinkElm327Session *)session;
- (void)processCompletedResponse;
- (void)beginPortableSession;
- (void)beginCurrentInitializationCommand;
- (BOOL)beginCommand:(const char *)command timeout:(uint64_t)timeoutMs;
- (void)startTickTimer;
- (void)stopTickTimer;
- (void)processInitializationResponse:(const MblinkElm327Response *)response;
- (void)processSupportedPidResponse:(const MblinkElm327Response *)response;
- (void)processLiveResponse:(const MblinkElm327Response *)response;
- (void)scheduleNextLiveRequest;
- (void)applyMeasurement:(const MblinkObd2Sample *)sample;
- (void)resetPublishedMeasurements;
@end

@implementation MBLinkDiagnosticsController {
    MBLinkBLETransport *_provider;
    MblinkElm327Session _session;
    BOOL _sessionInitialized;
    MblinkElm327InitState _initialization;
    MblinkObd2PidSet _supportedPids;
    MblinkScheduler _scheduler;
    MblinkTelemetryStore _telemetry;
    MblinkTelemetryRecorder _recorder;
    MblinkTelemetrySessionMetadata _sessionMetadata;
    NSMutableData *_sessionCSV;
    MBLinkDiagnosticsPhase _phase;
    dispatch_source_t _tickTimer;
    size_t _activeScheduleIndex;
    uint8_t _activePid;
    NSUInteger _pollGeneration;
    uint64_t _sessionMonotonicStartMs;
}

static uint64_t MBLinkMonotonicMilliseconds(void)
{
    NSTimeInterval uptime = NSProcessInfo.processInfo.systemUptime;
    if (uptime <= 0.0) {
        return 0U;
    }
    double milliseconds = uptime * 1000.0;
    if (milliseconds >= (double)UINT64_MAX) {
        return UINT64_MAX;
    }
    return (uint64_t)milliseconds;
}

static uint64_t MBLinkElapsedMilliseconds(uint64_t startedMs)
{
    const uint64_t nowMs = MBLinkMonotonicMilliseconds();
    return nowMs >= startedMs ? nowMs - startedMs : 0U;
}

static uint64_t MBLinkEpochMilliseconds(void)
{
    NSTimeInterval seconds = [NSDate date].timeIntervalSince1970;
    if (seconds <= 0.0) {
        return 0U;
    }
    double milliseconds = seconds * 1000.0;
    if (milliseconds >= (double)UINT64_MAX) {
        return UINT64_MAX;
    }
    return (uint64_t)milliseconds;
}

static bool MBLinkAppendCSV(void *context, const char *bytes, size_t length)
{
    if (context == NULL || bytes == NULL) {
        return false;
    }
    NSMutableData *data = (__bridge NSMutableData *)context;
    [data appendBytes:bytes length:length];
    return true;
}

static void MBLinkSessionEvent(void *context,
                               const MblinkElm327Session *session)
{
    MBLinkDiagnosticsController *controller =
        (__bridge MBLinkDiagnosticsController *)context;
    if (controller == nil || session == NULL) {
        return;
    }
    [controller handleSessionEvent:session];
}

- (instancetype)init
{
    self = [super init];
    if (self != nil) {
        _provider = [[MBLinkBLETransport alloc] init];
        _provider.delegate = self;
        _statusText = @"Idle";
        _phase = MBLinkDiagnosticsPhaseIdle;
        mblink_obd2_pid_set_clear(&_supportedPids);
        mblink_scheduler_init(&_scheduler);
        mblink_telemetry_store_init(&_telemetry);
        mblink_telemetry_recorder_init(&_recorder);
        _sessionCSV = [[NSMutableData alloc] init];
        mblink_telemetry_store_set_favourite(&_telemetry, 0x0cU, true);
        mblink_telemetry_store_set_favourite(&_telemetry, 0x0dU, true);
        mblink_telemetry_store_set_favourite(&_telemetry, 0x05U, true);
        mblink_telemetry_store_set_favourite(&_telemetry, 0x0bU, true);
        mblink_telemetry_session_metadata_init(
            &_sessionMetadata, 0U, NULL, NULL);
    }
    return self;
}

- (void)dealloc
{
    _provider.delegate = nil;
    [self stopTickTimer];
    if (_recorder.started && !_recorder.finished) {
        (void)mblink_telemetry_recorder_finish(
            &_recorder, MBLinkEpochMilliseconds());
    }
    if (_sessionInitialized) {
        _sessionInitialized = NO;
        mblink_elm327_session_disconnect(&_session);
        mblink_elm327_session_deinit(&_session);
    } else {
        [_provider disconnect];
    }
}

- (void)notifyDelegate
{
    id<MBLinkDiagnosticsControllerDelegate> delegate = self.delegate;
    if (delegate != nil) {
        [delegate diagnosticsControllerDidUpdate:self];
    }
}

- (void)setStatus:(NSString *)status
{
    self.statusText = status;
    [self notifyDelegate];
}

- (void)resetPublishedMeasurements
{
    self.hasEngineLoad = NO;
    self.engineLoadPercent = 0.0;
    self.hasCoolantTemperature = NO;
    self.coolantTemperatureCelsius = 0.0;
    self.hasManifoldPressure = NO;
    self.manifoldPressureKPa = 0.0;
    self.hasRPM = NO;
    self.rpm = 0.0;
    self.hasVehicleSpeed = NO;
    self.vehicleSpeedKmh = 0.0;
    self.hasIntakeAirTemperature = NO;
    self.intakeAirTemperatureCelsius = 0.0;
    self.hasMassAirFlow = NO;
    self.massAirFlowGramsPerSecond = 0.0;
    self.hasThrottlePosition = NO;
    self.throttlePositionPercent = 0.0;
}

- (void)start
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self start];
        });
        return;
    }

    _pollGeneration++;
    self.active = YES;
    self.ready = NO;
    [self resetPublishedMeasurements];
    _phase = MBLinkDiagnosticsPhaseIdle;
    _activePid = 0U;
    _activeScheduleIndex = 0U;
    mblink_obd2_pid_set_clear(&_supportedPids);
    mblink_scheduler_init(&_scheduler);
    mblink_telemetry_store_clear_samples(&_telemetry);
    mblink_telemetry_recorder_init(&_recorder);
    _sessionCSV = [[NSMutableData alloc] init];
    _sessionMonotonicStartMs = MBLinkMonotonicMilliseconds();
    mblink_telemetry_session_metadata_init(
        &_sessionMetadata, MBLinkEpochMilliseconds(), NULL, NULL);
    [self notifyDelegate];
    [_provider start];
}

- (void)disconnect
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self disconnect];
        });
        return;
    }

    _pollGeneration++;
    [self stopTickTimer];
    if (_sessionInitialized) {
        _sessionInitialized = NO;
        mblink_elm327_session_disconnect(&_session);
        mblink_elm327_session_deinit(&_session);
    } else {
        [_provider disconnect];
    }
    const uint64_t endedEpochMs = MBLinkEpochMilliseconds();
    mblink_telemetry_session_metadata_finish(
        &_sessionMetadata, endedEpochMs);
    if (_recorder.started && !_recorder.finished) {
        (void)mblink_telemetry_recorder_finish(&_recorder, endedEpochMs);
    }
    _phase = MBLinkDiagnosticsPhaseIdle;
    self.active = NO;
    self.ready = NO;
    [self resetPublishedMeasurements];
    [self setStatus:@"Disconnected"];
}

- (void)bleTransportDidUpdate:(MBLinkBLETransport *)transport
{
    self.peripheralName = transport.peripheralName;
    self.adapterIdentifier = transport.adapterIdentifier;
    if (transport.adapterIdentifier != nil) {
        mblink_telemetry_session_metadata_set_adapter(
            &_sessionMetadata, transport.adapterIdentifier.UTF8String);
    }

    if (transport.isReady && !_sessionInitialized) {
        [self beginPortableSession];
        return;
    }

    if (!transport.isReady && _sessionInitialized &&
        transport.state != MBLinkBLETransportStateProbing) {
        _pollGeneration++;
        [self stopTickTimer];
        _sessionInitialized = NO;
        mblink_elm327_session_deinit(&_session);
        self.ready = NO;
        [self resetPublishedMeasurements];
    }

    if (!_sessionInitialized) {
        self.statusText = transport.statusText;
    }
    [self notifyDelegate];
}

- (void)beginPortableSession
{
    MblinkTransport transport = MBLinkBLETransportMakeCTransport(_provider);
    if (!mblink_transport_is_valid(&transport) ||
        !mblink_elm327_session_init(&_session,
                                    &transport,
                                    MBLinkSessionEvent,
                                    (__bridge void *)self)) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Failed to initialise portable diagnostic session"];
        return;
    }

    _sessionInitialized = YES;
    if (!_recorder.started &&
        !mblink_telemetry_recorder_begin(
            &_recorder, &_sessionMetadata, MBLinkAppendCSV,
            (__bridge void *)_sessionCSV)) {
        _sessionInitialized = NO;
        mblink_elm327_session_disconnect(&_session);
        mblink_elm327_session_deinit(&_session);
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Could not start portable session recorder"];
        return;
    }
    mblink_elm327_init_begin(&_initialization);
    _phase = MBLinkDiagnosticsPhaseInitializing;
    [self startTickTimer];
    [self setStatus:@"Initialising ELM327 adapter"];
    [self beginCurrentInitializationCommand];
}

- (void)startTickTimer
{
    [self stopTickTimer];

    _tickTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                                        0U,
                                        0U,
                                        dispatch_get_main_queue());
    dispatch_source_set_timer(_tickTimer,
                              dispatch_time(DISPATCH_TIME_NOW,
                                            100 * NSEC_PER_MSEC),
                              100 * NSEC_PER_MSEC,
                              20 * NSEC_PER_MSEC);
    __weak MBLinkDiagnosticsController *weakSelf = self;
    dispatch_source_set_event_handler(_tickTimer, ^{
        MBLinkDiagnosticsController *strongSelf = weakSelf;
        if (strongSelf == nil || !strongSelf->_sessionInitialized) {
            return;
        }
        (void)mblink_elm327_session_tick(&strongSelf->_session,
                                         MBLinkMonotonicMilliseconds());
    });
    dispatch_resume(_tickTimer);
}

- (void)stopTickTimer
{
    if (_tickTimer != nil) {
        dispatch_source_cancel(_tickTimer);
        _tickTimer = nil;
    }
}

- (BOOL)beginCommand:(const char *)command timeout:(uint64_t)timeoutMs
{
    if (!_sessionInitialized || command == NULL) {
        return NO;
    }

    MblinkElm327SessionOpResult result =
        mblink_elm327_session_begin(&_session,
                                    command,
                                    MBLinkMonotonicMilliseconds(),
                                    timeoutMs);
    if (result != MBLINK_ELM327_SESSION_OP_OK) {
        NSString *reason =
            [NSString stringWithUTF8String:
                mblink_elm327_session_op_result_name(result)];
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:[NSString stringWithFormat:
            @"Diagnostic command failed: %@", reason]];
        return NO;
    }
    return YES;
}

- (void)beginCurrentInitializationCommand
{
    const char *command = mblink_elm327_init_command(&_initialization);
    if (command == NULL) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"ELM327 initialisation state is invalid"];
        return;
    }
    (void)[self beginCommand:command timeout:4000U];
}

- (void)handleSessionEvent:(const MblinkElm327Session *)session
{
    if (session->status == MBLINK_ELM327_SESSION_COMPLETE) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self processCompletedResponse];
        });
        return;
    }

    if (session->status == MBLINK_ELM327_SESSION_TIMED_OUT) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Diagnostic request timed out; reconnect to resynchronise"];
        return;
    }

    if (session->status == MBLINK_ELM327_SESSION_FAILED) {
        NSString *reason =
            [NSString stringWithUTF8String:
                mblink_elm327_result_name(session->elm_result)];
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:[NSString stringWithFormat:
            @"Adapter response failed: %@", reason]];
        return;
    }

    if (session->status == MBLINK_ELM327_SESSION_CANCELLED) {
        _phase = MBLinkDiagnosticsPhaseIdle;
        [self setStatus:@"Diagnostic request cancelled"];
    }
}

- (void)processCompletedResponse
{
    const MblinkElm327Response *response =
        mblink_elm327_session_response(&_session);
    if (response == NULL) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Diagnostic response was unavailable"];
        return;
    }

    (void)mblink_telemetry_store_record_transcript(
        &_telemetry,
        MBLinkElapsedMilliseconds(_sessionMonotonicStartMs),
        _session.parser.command,
        response);
    if (_recorder.started && !_recorder.finished &&
        !mblink_telemetry_recorder_record_response(
            &_recorder,
            MBLinkElapsedMilliseconds(_sessionMonotonicStartMs),
            _session.parser.command, response)) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Could not append diagnostic transcript"];
        return;
    }

    switch (_phase) {
    case MBLinkDiagnosticsPhaseInitializing:
        [self processInitializationResponse:response];
        break;
    case MBLinkDiagnosticsPhaseCheckingPids:
        [self processSupportedPidResponse:response];
        break;
    case MBLinkDiagnosticsPhaseReadingLive:
        [self processLiveResponse:response];
        break;
    case MBLinkDiagnosticsPhaseLive:
    case MBLinkDiagnosticsPhaseIdle:
    case MBLinkDiagnosticsPhaseFailed:
        break;
    }
}

- (void)processInitializationResponse:(const MblinkElm327Response *)response
{
    MblinkElm327Result result =
        mblink_elm327_init_accept(&_initialization, response);
    if (result != MBLINK_ELM327_RESULT_OK ||
        _initialization.stage == MBLINK_ELM327_INIT_FAILED) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"ELM327 initialisation failed"];
        return;
    }

    if (_initialization.adapter_id[0] != '\0') {
        self.adapterIdentifier =
            [NSString stringWithUTF8String:_initialization.adapter_id];
        mblink_telemetry_session_metadata_set_adapter(
            &_sessionMetadata, _initialization.adapter_id);
    }

    if (_initialization.stage == MBLINK_ELM327_INIT_COMPLETE) {
        char command[8];
        MblinkObd2Result build =
            mblink_obd2_build_supported_pid_request(
                0x00U, command, sizeof(command));
        if (build != MBLINK_OBD2_RESULT_OK) {
            _phase = MBLinkDiagnosticsPhaseFailed;
            [self setStatus:@"Could not build OBD-II capability request"];
            return;
        }

        _phase = MBLinkDiagnosticsPhaseCheckingPids;
        [self setStatus:@"Checking standard OBD-II capabilities"];
        (void)[self beginCommand:command timeout:3000U];
        return;
    }

    [self beginCurrentInitializationCommand];
}

- (void)processSupportedPidResponse:(const MblinkElm327Response *)response
{
    bool hasMore = false;
    MblinkObd2Result result =
        mblink_obd2_accept_supported_pids(
            response, 0x00U, &_supportedPids, &hasMore);
    if (result != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Vehicle did not provide a valid OBD-II PID map"];
        return;
    }
    (void)hasMore;

    MblinkSchedulerResult scheduleResult =
        mblink_scheduler_configure_standard_obd2(
            &_scheduler, &_supportedPids, MBLinkMonotonicMilliseconds());
    if (scheduleResult != MBLINK_SCHEDULER_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Could not configure portable live-data scheduler"];
        return;
    }

    self.ready = YES;
    _phase = MBLinkDiagnosticsPhaseLive;
    [self notifyDelegate];

    if (_scheduler.count == 0U) {
        [self setStatus:@"Connected; no supported dashboard PIDs were advertised"];
        return;
    }

    [self setStatus:@"Live OBD-II scheduler active"];
    [self scheduleNextLiveRequest];
}

- (void)scheduleNextLiveRequest
{
    if (!_sessionInitialized || !_provider.isReady ||
        _phase == MBLinkDiagnosticsPhaseFailed) {
        return;
    }

    MblinkSchedulerDispatch dispatch;
    MblinkSchedulerNextResult next =
        mblink_scheduler_next(&_scheduler,
                              MBLinkMonotonicMilliseconds(),
                              &dispatch);

    if (next == MBLINK_SCHEDULER_NEXT_EMPTY ||
        next == MBLINK_SCHEDULER_NEXT_PAUSED) {
        _phase = MBLinkDiagnosticsPhaseLive;
        return;
    }

    if (next == MBLINK_SCHEDULER_NEXT_WAITING) {
        uint64_t waitMs = dispatch.wait_ms;
        if (waitMs > 60000U) {
            waitMs = 60000U;
        }
        const NSUInteger generation = _pollGeneration;
        dispatch_after(
            dispatch_time(DISPATCH_TIME_NOW,
                          (int64_t)waitMs * NSEC_PER_MSEC),
            dispatch_get_main_queue(), ^{
                if (generation != self->_pollGeneration) {
                    return;
                }
                [self scheduleNextLiveRequest];
            });
        return;
    }

    char command[8];
    MblinkObd2Result build =
        mblink_obd2_build_live_pid_request(
            dispatch.pid, command, sizeof(command));
    if (build != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Could not build scheduled OBD-II request"];
        return;
    }

    _activePid = dispatch.pid;
    _activeScheduleIndex = dispatch.index;
    _phase = MBLinkDiagnosticsPhaseReadingLive;
    const uint64_t nowMs = MBLinkMonotonicMilliseconds();
    if ([self beginCommand:command timeout:2000U]) {
        (void)mblink_scheduler_mark_dispatched(
            &_scheduler, _activeScheduleIndex, nowMs);
    }
}

- (void)applyMeasurement:(const MblinkObd2Sample *)sample
{
    if (sample == NULL) {
        return;
    }

    switch (sample->pid) {
    case 0x04U:
        self.engineLoadPercent = sample->value;
        self.hasEngineLoad = YES;
        break;
    case 0x05U:
        self.coolantTemperatureCelsius = sample->value;
        self.hasCoolantTemperature = YES;
        break;
    case 0x0bU:
        self.manifoldPressureKPa = sample->value;
        self.hasManifoldPressure = YES;
        break;
    case 0x0cU:
        self.rpm = sample->value;
        self.hasRPM = YES;
        break;
    case 0x0dU:
        self.vehicleSpeedKmh = sample->value;
        self.hasVehicleSpeed = YES;
        break;
    case 0x0fU:
        self.intakeAirTemperatureCelsius = sample->value;
        self.hasIntakeAirTemperature = YES;
        break;
    case 0x10U:
        self.massAirFlowGramsPerSecond = sample->value;
        self.hasMassAirFlow = YES;
        break;
    case 0x11U:
        self.throttlePositionPercent = sample->value;
        self.hasThrottlePosition = YES;
        break;
    default:
        break;
    }
}

- (void)processLiveResponse:(const MblinkElm327Response *)response
{
    if (response->result == MBLINK_ELM327_RESULT_NO_DATA) {
        _phase = MBLinkDiagnosticsPhaseLive;
        self.statusText = @"Live OBD-II data; one PID returned no data";
        [self notifyDelegate];
        [self scheduleNextLiveRequest];
        return;
    }

    MblinkObd2Sample sample;
    MblinkObd2Result result =
        mblink_obd2_decode_live_pid(response, _activePid, &sample);
    if (result != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        NSString *reason =
            [NSString stringWithUTF8String:
                mblink_obd2_result_name(result)];
        [self setStatus:[NSString stringWithFormat:
            @"Live OBD-II decode failed: %@", reason]];
        return;
    }

    if (!mblink_telemetry_store_record(
            &_telemetry,
            MBLinkElapsedMilliseconds(_sessionMonotonicStartMs), &sample)) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Could not record live telemetry sample"];
        return;
    }

    MblinkTelemetrySample recorded;
    if (_recorder.started && !_recorder.finished &&
        mblink_telemetry_store_latest(&_telemetry, sample.pid, &recorded) &&
        !mblink_telemetry_recorder_record_sample(
            &_recorder, &recorded,
            mblink_telemetry_store_is_favourite(&_telemetry, sample.pid))) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Could not append session recording"];
        return;
    }

    [self applyMeasurement:&sample];
    self.ready = YES;
    _phase = MBLinkDiagnosticsPhaseLive;
    self.statusText = @"Live OBD-II data";
    [self notifyDelegate];
    [self scheduleNextLiveRequest];
}

- (NSUInteger)recordedSampleCount
{
    uint64_t total = mblink_telemetry_store_total_sample_count(&_telemetry);
    if (total > (uint64_t)NSUIntegerMax) {
        return NSUIntegerMax;
    }
    return (NSUInteger)total;
}

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit
{
    if (limit == 0U) {
        return @[];
    }

    NSMutableArray<NSNumber *> *values =
        [[NSMutableArray alloc] initWithCapacity:limit];
    const size_t count =
        mblink_telemetry_store_history_count(&_telemetry);

    for (size_t reverseIndex = count;
         reverseIndex > 0U && values.count < limit;
         --reverseIndex) {
        MblinkTelemetrySample sample;
        if (!mblink_telemetry_store_history_at(
                &_telemetry, reverseIndex - 1U, &sample)) {
            continue;
        }
        if (sample.measurement.pid != pid) {
            continue;
        }
        [values insertObject:@(sample.measurement.value) atIndex:0U];
    }
    return values;
}

- (BOOL)favouriteForPID:(uint8_t)pid
{
    return mblink_telemetry_store_is_favourite(&_telemetry, pid);
}

- (void)setFavourite:(BOOL)favourite forPID:(uint8_t)pid
{
    mblink_telemetry_store_set_favourite(&_telemetry, pid, favourite);
    [self notifyDelegate];
}

- (nullable NSString *)csvSnapshot
{
    if (_sessionCSV.length == 0U) {
        return nil;
    }
    return [[NSString alloc] initWithData:[_sessionCSV copy]
                                 encoding:NSUTF8StringEncoding];
}

@end
