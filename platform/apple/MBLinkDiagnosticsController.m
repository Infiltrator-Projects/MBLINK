// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "MBLinkBLETransport+MBLINK.h"
#import "mblink/elm327.h"
#import "mblink/elm327_session.h"
#import "mblink/mercedes.h"
#import "mblink/mercedes_probe.h"
#import "mblink/obd2.h"
#import "mblink/scheduler.h"
#import "mblink/telemetry.h"

typedef NS_ENUM(NSInteger, MBLinkDiagnosticsPhase) {
    MBLinkDiagnosticsPhaseIdle = 0,
    MBLinkDiagnosticsPhaseInitializing,
    MBLinkDiagnosticsPhaseCheckingPids,
    MBLinkDiagnosticsPhaseProbingMercedes,
    MBLinkDiagnosticsPhaseRestoringOBD,
    MBLinkDiagnosticsPhaseScanningStoredDTCs,
    MBLinkDiagnosticsPhaseScanningPendingDTCs,
    MBLinkDiagnosticsPhaseScanningPermanentDTCs,
    MBLinkDiagnosticsPhaseReadingLive,
    MBLinkDiagnosticsPhaseLive,
    MBLinkDiagnosticsPhaseFailed
};

@interface MBLinkDiagnosticsController () <MBLinkBLETransportDelegate>
@property(nonatomic, copy, readwrite) NSString *statusText;
@property(nonatomic, copy, readwrite, nullable) NSString *peripheralName;
@property(nonatomic, copy, readwrite, nullable) NSString *adapterIdentifier;
@property(nonatomic, copy, readwrite) NSString *mercedesProbeStatusText;
@property(nonatomic, copy, readwrite, nullable) NSString *mercedesProbeEndpointText;
@property(nonatomic, copy, readwrite, nullable) NSString *mercedesVINText;
@property(nonatomic, copy, readwrite) NSString *mercedesIdentitySummaryText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesIdentityResults;
@property(nonatomic, copy, readwrite) NSString *faultScanStatusText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *storedDTCs;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *pendingDTCs;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *permanentDTCs;
@property(nonatomic, readwrite, getter=isActive) BOOL active;
@property(nonatomic, readwrite, getter=isReady) BOOL ready;

- (void)handleSessionEvent:(const MblinkElm327Session *)session;
- (void)processCompletedResponse;
- (void)beginPortableSession;
- (void)beginCurrentInitializationCommand;
- (BOOL)beginCommand:(const char *)command timeout:(uint64_t)timeoutMs;
- (void)startTickTimer;
- (void)stopTickTimer;
- (void)processInitializationResponse:(const MblinkElm327Response *)response;
- (void)processSupportedPidResponse:(const MblinkElm327Response *)response;
- (void)beginMercedesProbe;
- (void)beginCurrentMercedesProbeCommand;
- (void)processMercedesProbeResponse:(const MblinkElm327Response *)response;
- (void)updateMercedesProbeEvidenceSummary;
- (NSString *)mercedesProbeFailureText;
- (void)beginPostMercedesRestore;
- (void)beginFaultScan;
- (void)beginFaultScanKind:(MblinkObd2DtcKind)kind;
- (void)processFaultScanResponse:(const MblinkElm327Response *)response;
- (void)completeLiveSetup;
- (void)processLiveResponse:(const MblinkElm327Response *)response;
- (void)scheduleNextLiveRequest;
@end

@implementation MBLinkDiagnosticsController {
    MBLinkBLETransport *_provider;
    MblinkElm327Session _session;
    BOOL _sessionInitialized;
    MblinkElm327InitState _initialization;
    MblinkMercedesEcuProbe _mercedesProbe;
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
    uint8_t _supportedPidBase;
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

static unsigned int MBLinkBitCount32(uint32_t value)
{
    unsigned int count = 0U;
    while (value != 0U) {
        count += value & 1U;
        value >>= 1U;
    }
    return count;
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

static NSString *MBLinkStringFromCString(const char *value)
{
    if (value == NULL) {
        return @"unknown";
    }
    NSString *string = [NSString stringWithUTF8String:value];
    return string != nil ? string : @"unknown";
}

static NSString *MBLinkMercedesEndpointText(
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    if (endpoint == NULL) {
        return nil;
    }

    NSString *name = MBLinkStringFromCString(endpoint->name);
    if (endpoint->address.tx_extended_id) {
        return [NSString stringWithFormat:@"%@ · 0x%08X → 0x%08X",
                                          name,
                                          (unsigned int)endpoint->address.tx_can_id,
                                          (unsigned int)endpoint->address.rx_can_id];
    }
    return [NSString stringWithFormat:@"%@ · 0x%03X → 0x%03X",
                                      name,
                                      (unsigned int)endpoint->address.tx_can_id,
                                      (unsigned int)endpoint->address.rx_can_id];
}

static NSArray<NSString *> *MBLinkDTCStrings(const MblinkObd2DtcList *list)
{
    if (list == NULL || list->count == 0U) {
        return @[];
    }

    NSMutableArray<NSString *> *values =
        [[NSMutableArray alloc] initWithCapacity:list->count];
    for (size_t index = 0U; index < list->count; ++index) {
        NSString *code = MBLinkStringFromCString(list->entries[index].code);
        if (code.length != 0U) {
            [values addObject:code];
        }
    }
    return [values copy];
}

- (instancetype)init
{
    self = [super init];
    if (self != nil) {
        _provider = [[MBLinkBLETransport alloc] init];
        _provider.delegate = self;
        _statusText = @"Idle";
        _mercedesProbeStatusText = @"Not attempted";
        _mercedesIdentitySummaryText = @"Not attempted";
        _mercedesIdentityResults = @[];
        _faultScanStatusText = @"Not scanned";
        _storedDTCs = @[];
        _pendingDTCs = @[];
        _permanentDTCs = @[];
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

- (void)start
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self start];
        });
        return;
    }
    if (self.active) {
        return;
    }

    _pollGeneration++;
    self.active = YES;
    self.ready = NO;
    self.mercedesProbeStatusText = @"Not attempted";
    self.mercedesProbeEndpointText = nil;
    self.mercedesVINText = nil;
    self.mercedesIdentitySummaryText = @"Not attempted";
    self.mercedesIdentityResults = @[];
    self.faultScanStatusText = @"Waiting for vehicle connection";
    self.storedDTCs = @[];
    self.pendingDTCs = @[];
    self.permanentDTCs = @[];
    _mercedesProbe = (MblinkMercedesEcuProbe){0};
    _phase = MBLinkDiagnosticsPhaseIdle;
    _activePid = 0U;
    _activeScheduleIndex = 0U;
    _supportedPidBase = 0U;
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
            MBLinkStringFromCString(mblink_elm327_session_op_result_name(result));
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
        if (_phase == MBLinkDiagnosticsPhaseProbingMercedes) {
            self.mercedesProbeStatusText =
                @"Probe timed out; reconnect required to resynchronise the adapter";
            self.mercedesIdentitySummaryText = @"Probe did not complete";
        }
        if (_phase == MBLinkDiagnosticsPhaseScanningStoredDTCs ||
            _phase == MBLinkDiagnosticsPhaseScanningPendingDTCs ||
            _phase == MBLinkDiagnosticsPhaseScanningPermanentDTCs) {
            self.faultScanStatusText = @"Fault scan timed out; reconnect required";
        }
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Diagnostic request timed out; reconnect to resynchronise"];
        return;
    }

    if (session->status == MBLINK_ELM327_SESSION_FAILED) {
        NSString *reason = MBLinkStringFromCString(
            mblink_elm327_result_name(session->elm_result));
        if (_phase == MBLinkDiagnosticsPhaseProbingMercedes) {
            self.mercedesProbeStatusText = [NSString stringWithFormat:
                @"Adapter response failed during probe: %@", reason];
            self.mercedesIdentitySummaryText = @"Probe did not complete";
        }
        if (_phase == MBLinkDiagnosticsPhaseScanningStoredDTCs ||
            _phase == MBLinkDiagnosticsPhaseScanningPendingDTCs ||
            _phase == MBLinkDiagnosticsPhaseScanningPermanentDTCs) {
            self.faultScanStatusText = [NSString stringWithFormat:
                @"Fault scan adapter error: %@", reason];
        }
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
    case MBLinkDiagnosticsPhaseRestoringOBD:
        [self processInitializationResponse:response];
        break;
    case MBLinkDiagnosticsPhaseCheckingPids:
        [self processSupportedPidResponse:response];
        break;
    case MBLinkDiagnosticsPhaseProbingMercedes:
        [self processMercedesProbeResponse:response];
        break;
    case MBLinkDiagnosticsPhaseScanningStoredDTCs:
    case MBLinkDiagnosticsPhaseScanningPendingDTCs:
    case MBLinkDiagnosticsPhaseScanningPermanentDTCs:
        [self processFaultScanResponse:response];
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
    const BOOL restoringOBD = _phase == MBLinkDiagnosticsPhaseRestoringOBD;
    MblinkElm327Result result =
        mblink_elm327_init_accept(&_initialization, response);
    if (result != MBLINK_ELM327_RESULT_OK ||
        _initialization.stage == MBLINK_ELM327_INIT_FAILED) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:restoringOBD
            ? @"Could not restore the standard OBD-II adapter channel"
            : @"ELM327 initialisation failed"];
        return;
    }

    if (_initialization.adapter_id[0] != '\0') {
        self.adapterIdentifier =
            [NSString stringWithUTF8String:_initialization.adapter_id];
        mblink_telemetry_session_metadata_set_adapter(
            &_sessionMetadata, _initialization.adapter_id);
    }

    if (_initialization.stage == MBLINK_ELM327_INIT_COMPLETE) {
        if (restoringOBD) {
            [self beginFaultScan];
            return;
        }

        char command[8];
        _supportedPidBase = 0x00U;
        MblinkObd2Result build =
            mblink_obd2_build_supported_pid_request(
                _supportedPidBase, command, sizeof(command));
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
            response, _supportedPidBase, &_supportedPids, &hasMore);
    if (result != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Vehicle did not provide a valid OBD-II PID map"];
        return;
    }

    if (hasMore && _supportedPidBase <= 0xc0U) {
        char command[8];
        _supportedPidBase = (uint8_t)(_supportedPidBase + 0x20U);
        result = mblink_obd2_build_supported_pid_request(
            _supportedPidBase, command, sizeof(command));
        if (result != MBLINK_OBD2_RESULT_OK) {
            _phase = MBLinkDiagnosticsPhaseFailed;
            [self setStatus:@"Could not continue OBD-II capability discovery"];
            return;
        }
        [self setStatus:[NSString stringWithFormat:
            @"Checking OBD-II PID block 0x%02X", (unsigned int)_supportedPidBase]];
        (void)[self beginCommand:command timeout:3000U];
        return;
    }

    [self beginMercedesProbe];
}

- (void)beginMercedesProbe
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    if (profile == NULL || !mblink_mercedes_vehicle_profile_is_valid(profile)) {
        self.mercedesProbeStatusText = @"C207 / OM651 development profile unavailable";
        self.mercedesIdentitySummaryText = @"Not attempted";
        [self beginFaultScan];
        return;
    }

    const MblinkMercedesEcuEndpointDefinition *endpoint =
        mblink_mercedes_profile_find_endpoint(
            profile, "c207-om651-engine-eobd-11bit");
    if (endpoint == NULL) {
        self.mercedesProbeStatusText = @"No engine endpoint candidate is defined";
        self.mercedesIdentitySummaryText = @"Not attempted";
        [self beginFaultScan];
        return;
    }

    self.mercedesProbeEndpointText = MBLinkMercedesEndpointText(endpoint);
    MblinkMercedesEcuProbeResult result =
        mblink_mercedes_ecu_probe_begin(&_mercedesProbe, endpoint);
    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK) {
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Probe could not start: %@",
            MBLinkStringFromCString(mblink_mercedes_ecu_probe_result_name(result))];
        self.mercedesIdentitySummaryText = @"Not attempted";
        [self beginFaultScan];
        return;
    }

    self.mercedesProbeStatusText =
        @"Probing candidate with read-only UDS TesterPresent";
    self.mercedesIdentitySummaryText = @"Waiting for UDS endpoint response";
    _phase = MBLinkDiagnosticsPhaseProbingMercedes;
    [self setStatus:@"Probing Mercedes-Benz engine ECU (read-only)"];
    [self beginCurrentMercedesProbeCommand];
}

- (void)beginCurrentMercedesProbeCommand
{
    char command[MBLINK_ELM327_MAX_COMMAND];
    size_t written = 0U;
    MblinkMercedesEcuProbeResult result =
        mblink_mercedes_ecu_probe_command(
            &_mercedesProbe, command, sizeof(command), &written);
    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK || written == 0U) {
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Probe command failed: %@",
            MBLinkStringFromCString(mblink_mercedes_ecu_probe_result_name(result))];
        self.mercedesIdentitySummaryText = @"Probe did not complete";
        [self beginPostMercedesRestore];
        return;
    }

    if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN) {
        self.mercedesProbeStatusText =
            @"UDS endpoint confirmed; reading standardized VIN (F190)";
        self.mercedesIdentitySummaryText = @"Reading standardized vehicle identity";
        [self notifyDelegate];
    } else if (_mercedesProbe.stage ==
               MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY) {
        const size_t index = _mercedesProbe.identity_index;
        const uint16_t did = mblink_mercedes_ecu_probe_identity_did_at(index);
        const char *name = mblink_mercedes_ecu_probe_identity_did_name(index);
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Reading standardized ECU identity %zu/%zu · %04X · %@",
            index + 1U,
            mblink_mercedes_ecu_probe_identity_did_count(),
            (unsigned int)did,
            MBLinkStringFromCString(name)];
        self.mercedesIdentitySummaryText = [NSString stringWithFormat:
            @"Identity sweep in progress · %zu/%zu",
            index + 1U,
            mblink_mercedes_ecu_probe_identity_did_count()];
        [self notifyDelegate];
    }

    (void)[self beginCommand:command timeout:4000U];
}

- (void)processMercedesProbeResponse:(const MblinkElm327Response *)response
{
    MblinkMercedesEcuProbeResult result =
        mblink_mercedes_ecu_probe_accept(&_mercedesProbe, response);

    if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
        [self updateMercedesProbeEvidenceSummary];
        [self beginPostMercedesRestore];
        return;
    }

    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
        _mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED) {
        self.mercedesProbeStatusText = [self mercedesProbeFailureText];
        self.mercedesIdentitySummaryText = @"Probe did not complete";
        [self beginPostMercedesRestore];
        return;
    }

    [self beginCurrentMercedesProbeCommand];
}

- (void)updateMercedesProbeEvidenceSummary
{
    const unsigned int positive =
        MBLinkBitCount32(_mercedesProbe.identity_positive_mask);
    const unsigned int negative =
        MBLinkBitCount32(_mercedesProbe.identity_negative_mask);
    const unsigned int noResponse =
        MBLinkBitCount32(_mercedesProbe.identity_no_response_mask);
    const unsigned int invalid =
        MBLinkBitCount32(_mercedesProbe.identity_invalid_mask);
    const size_t total = mblink_mercedes_ecu_probe_identity_did_count();

    self.mercedesIdentitySummaryText = [NSString stringWithFormat:
        @"%u/%zu positive · %u negative · %u no response · %u invalid",
        positive, total, negative, noResponse, invalid];

    NSMutableArray<NSString *> *identityResults =
        [[NSMutableArray alloc] initWithCapacity:total];
    for (size_t index = 0U; index < total; ++index) {
        const uint32_t bit = (uint32_t)1U << index;
        const uint16_t did = mblink_mercedes_ecu_probe_identity_did_at(index);
        NSString *name = MBLinkStringFromCString(
            mblink_mercedes_ecu_probe_identity_did_name(index));
        NSString *state = @"not classified";
        if ((_mercedesProbe.identity_positive_mask & bit) != 0U) {
            state = @"response captured";
        } else if ((_mercedesProbe.identity_negative_mask & bit) != 0U) {
            state = @"negative response";
        } else if ((_mercedesProbe.identity_no_response_mask & bit) != 0U) {
            state = @"no response";
        } else if ((_mercedesProbe.identity_invalid_mask & bit) != 0U) {
            state = @"invalid response";
        }
        [identityResults addObject:[NSString stringWithFormat:
            @"%04X · %@ · %@", (unsigned int)did, name, state]];
    }
    self.mercedesIdentityResults = [identityResults copy];

    NSString *vinSummary = nil;
    if (_mercedesProbe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE &&
        _mercedesProbe.vin[0] != '\0') {
        self.mercedesVINText = MBLinkStringFromCString(_mercedesProbe.vin);
        mblink_telemetry_session_metadata_set_vehicle(
            &_sessionMetadata, _mercedesProbe.vin);
        vinSummary = [NSString stringWithFormat:@"VIN %@", self.mercedesVINText];
    } else {
        self.mercedesVINText = nil;
        switch (_mercedesProbe.vin_result) {
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE:
            vinSummary = @"standard VIN not returned";
            break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE:
            vinSummary = [NSString stringWithFormat:
                @"standard VIN negative response NRC 0x%02X",
                (unsigned int)_mercedesProbe.vin_negative_response_code];
            break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE:
            vinSummary = @"standard VIN response was not a valid 17-character VIN";
            break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED:
            vinSummary = @"standard VIN was not attempted";
            break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE:
            vinSummary = @"standard VIN response was empty";
            break;
        }
    }

    self.mercedesProbeStatusText = [NSString stringWithFormat:
        @"Positive UDS endpoint response captured; %@; standardized identity %@; endpoint remains a candidate pending fixture verification",
        vinSummary,
        self.mercedesIdentitySummaryText];
}

- (NSString *)mercedesProbeFailureText
{
    if (_mercedesProbe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR) {
        if (_mercedesProbe.uds_negative_response_code != 0U) {
            return [NSString stringWithFormat:
                @"UDS endpoint replied with negative response NRC 0x%02X; candidate not promoted",
                (unsigned int)_mercedesProbe.uds_negative_response_code];
        }
        return [NSString stringWithFormat:
            @"UDS response validation failed: %@",
            MBLinkStringFromCString(
                mblink_uds_result_name(_mercedesProbe.uds_failure))];
    }

    if (_mercedesProbe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR) {
        return [NSString stringWithFormat:
            @"No valid UDS PDU from candidate: %@ (%@)",
            MBLinkStringFromCString(
                mblink_elm327_can_result_name(_mercedesProbe.elm_can_failure)),
            MBLinkStringFromCString(
                mblink_elm327_result_name(_mercedesProbe.elm_failure))];
    }

    if (_mercedesProbe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR) {
        return [NSString stringWithFormat:
            @"Could not configure candidate CAN channel: %@ (%@)",
            MBLinkStringFromCString(
                mblink_elm327_can_result_name(_mercedesProbe.elm_can_failure)),
            MBLinkStringFromCString(
                mblink_elm327_result_name(_mercedesProbe.elm_failure))];
    }

    return [NSString stringWithFormat:
        @"Mercedes probe failed: %@",
        MBLinkStringFromCString(
            mblink_mercedes_ecu_probe_result_name(_mercedesProbe.failure))];
}

- (void)beginPostMercedesRestore
{
    mblink_elm327_init_begin(&_initialization);
    _phase = MBLinkDiagnosticsPhaseRestoringOBD;
    [self setStatus:@"Restoring standard OBD-II adapter channel"];
    [self beginCurrentInitializationCommand];
}

- (void)beginFaultScan
{
    self.storedDTCs = @[];
    self.pendingDTCs = @[];
    self.permanentDTCs = @[];
    self.faultScanStatusText = @"Scanning stored, pending and permanent OBD-II faults";
    [self beginFaultScanKind:MBLINK_OBD2_DTC_STORED];
}

- (void)beginFaultScanKind:(MblinkObd2DtcKind)kind
{
    char command[8];
    MblinkObd2Result result =
        mblink_obd2_build_dtc_request(kind, command, sizeof(command));
    if (result != MBLINK_OBD2_RESULT_OK) {
        self.faultScanStatusText = @"Could not build OBD-II fault request";
        [self completeLiveSetup];
        return;
    }

    switch (kind) {
    case MBLINK_OBD2_DTC_STORED:
        _phase = MBLinkDiagnosticsPhaseScanningStoredDTCs;
        [self setStatus:@"Scanning stored OBD-II fault codes"];
        break;
    case MBLINK_OBD2_DTC_PENDING:
        _phase = MBLinkDiagnosticsPhaseScanningPendingDTCs;
        [self setStatus:@"Scanning pending OBD-II fault codes"];
        break;
    case MBLINK_OBD2_DTC_PERMANENT:
        _phase = MBLinkDiagnosticsPhaseScanningPermanentDTCs;
        [self setStatus:@"Scanning permanent OBD-II fault codes"];
        break;
    }
    (void)[self beginCommand:command timeout:3000U];
}

- (void)processFaultScanResponse:(const MblinkElm327Response *)response
{
    MblinkObd2DtcKind kind;
    if (_phase == MBLinkDiagnosticsPhaseScanningStoredDTCs) {
        kind = MBLINK_OBD2_DTC_STORED;
    } else if (_phase == MBLinkDiagnosticsPhaseScanningPendingDTCs) {
        kind = MBLINK_OBD2_DTC_PENDING;
    } else if (_phase == MBLinkDiagnosticsPhaseScanningPermanentDTCs) {
        kind = MBLINK_OBD2_DTC_PERMANENT;
    } else {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Fault scan state was invalid"];
        return;
    }

    MblinkObd2DtcList list = {0};
    MblinkObd2Result result = MBLINK_OBD2_RESULT_OK;
    if (response->result != MBLINK_ELM327_RESULT_NO_DATA) {
        result = mblink_obd2_decode_dtcs(response, kind, &list);
    }

    if (result != MBLINK_OBD2_RESULT_OK) {
        self.faultScanStatusText = [NSString stringWithFormat:
            @"Fault scan decode stopped: %@",
            MBLinkStringFromCString(mblink_obd2_result_name(result))];
        [self completeLiveSetup];
        return;
    }

    NSArray<NSString *> *codes = MBLinkDTCStrings(&list);
    switch (kind) {
    case MBLINK_OBD2_DTC_STORED:
        self.storedDTCs = codes;
        [self beginFaultScanKind:MBLINK_OBD2_DTC_PENDING];
        return;
    case MBLINK_OBD2_DTC_PENDING:
        self.pendingDTCs = codes;
        [self beginFaultScanKind:MBLINK_OBD2_DTC_PERMANENT];
        return;
    case MBLINK_OBD2_DTC_PERMANENT:
        self.permanentDTCs = codes;
        self.faultScanStatusText = [NSString stringWithFormat:
            @"Complete · %lu stored · %lu pending · %lu permanent",
            (unsigned long)self.storedDTCs.count,
            (unsigned long)self.pendingDTCs.count,
            (unsigned long)self.permanentDTCs.count];
        [self completeLiveSetup];
        return;
    }
}

- (void)completeLiveSetup
{
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

    [self setStatus:@"Live OBD-II and diesel scheduler active"];
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
    if (result == MBLINK_OBD2_RESULT_UNSUPPORTED_PID) {
        _phase = MBLinkDiagnosticsPhaseLive;
        self.statusText = @"Live OBD-II data; one advertised diesel sub-field is unavailable";
        [self notifyDelegate];
        [self scheduleNextLiveRequest];
        return;
    }
    if (result != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        NSString *reason = MBLinkStringFromCString(
            mblink_obd2_result_name(result));
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

    self.ready = YES;
    _phase = MBLinkDiagnosticsPhaseLive;
    self.statusText = @"Live OBD-II and diesel data";
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
