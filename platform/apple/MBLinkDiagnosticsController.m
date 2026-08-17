// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "MBLinkBLETransport+MBLINK.h"
#import "mblink/elm327.h"
#import "mblink/elm327_session.h"
#import "mblink/obd2.h"

typedef NS_ENUM(NSInteger, MBLinkDiagnosticsPhase) {
    MBLinkDiagnosticsPhaseIdle = 0,
    MBLinkDiagnosticsPhaseInitializing,
    MBLinkDiagnosticsPhaseCheckingPids,
    MBLinkDiagnosticsPhaseReadingRPM,
    MBLinkDiagnosticsPhaseReadingCoolant,
    MBLinkDiagnosticsPhaseLive,
    MBLinkDiagnosticsPhaseFailed
};

@interface MBLinkDiagnosticsController () <MBLinkBLETransportDelegate>
@property(nonatomic, copy, readwrite) NSString *statusText;
@property(nonatomic, copy, readwrite, nullable) NSString *peripheralName;
@property(nonatomic, copy, readwrite, nullable) NSString *adapterIdentifier;
@property(nonatomic, readwrite, getter=isActive) BOOL active;
@property(nonatomic, readwrite, getter=isReady) BOOL ready;
@property(nonatomic, readwrite) BOOL hasRPM;
@property(nonatomic, readwrite) double rpm;
@property(nonatomic, readwrite) BOOL hasCoolantTemperature;
@property(nonatomic, readwrite) double coolantTemperatureCelsius;
- (void)handleSessionEvent:(const MblinkElm327Session *)session;
- (void)processCompletedResponse;
- (void)beginPortableSession;
- (void)beginCurrentInitializationCommand;
- (BOOL)beginCommand:(const char *)command timeout:(uint64_t)timeoutMs;
- (void)startTickTimer;
- (void)stopTickTimer;
- (void)processInitializationResponse:(const MblinkElm327Response *)response;
- (void)processSupportedPidResponse:(const MblinkElm327Response *)response;
- (void)processLiveResponse:(const MblinkElm327Response *)response pid:(uint8_t)pid;
- (void)scheduleNextLiveRequestAfter:(NSTimeInterval)delay;
@end

@implementation MBLinkDiagnosticsController {
    MBLinkBLETransport *_provider;
    MblinkElm327Session _session;
    BOOL _sessionInitialized;
    MblinkElm327InitState _initialization;
    MblinkObd2PidSet _supportedPids;
    MBLinkDiagnosticsPhase _phase;
    dispatch_source_t _tickTimer;
    BOOL _rpmSupported;
    BOOL _coolantSupported;
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
    }
    return self;
}

- (void)dealloc
{
    _provider.delegate = nil;
    [self stopTickTimer];
    if (_sessionInitialized) {
        mblink_elm327_session_disconnect(&_session);
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

    self.active = YES;
    self.ready = NO;
    self.hasRPM = NO;
    self.hasCoolantTemperature = NO;
    _rpmSupported = NO;
    _coolantSupported = NO;
    _phase = MBLinkDiagnosticsPhaseIdle;
    mblink_obd2_pid_set_clear(&_supportedPids);
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

    [self stopTickTimer];
    if (_sessionInitialized) {
        mblink_elm327_session_disconnect(&_session);
        _sessionInitialized = NO;
    } else {
        [_provider disconnect];
    }
    _phase = MBLinkDiagnosticsPhaseIdle;
    self.active = NO;
    self.ready = NO;
    self.hasRPM = NO;
    self.hasCoolantTemperature = NO;
    [self setStatus:@"Disconnected"];
}

- (void)bleTransportDidUpdate:(MBLinkBLETransport *)transport
{
    self.peripheralName = transport.peripheralName;
    self.adapterIdentifier = transport.adapterIdentifier;

    if (transport.isReady && !_sessionInitialized) {
        [self beginPortableSession];
        return;
    }

    if (!transport.isReady && _sessionInitialized &&
        transport.state != MBLinkBLETransportStateProbing) {
        [self stopTickTimer];
        _sessionInitialized = NO;
        self.ready = NO;
        self.hasRPM = NO;
        self.hasCoolantTemperature = NO;
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
            [NSString stringWithUTF8String:mblink_elm327_session_op_result_name(result)];
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:[NSString stringWithFormat:@"Diagnostic command failed: %@",
                                                    reason]];
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
        [self setStatus:@"Diagnostic request timed out"];
        return;
    }

    if (session->status == MBLINK_ELM327_SESSION_FAILED) {
        NSString *reason =
            [NSString stringWithUTF8String:mblink_elm327_result_name(session->elm_result)];
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:[NSString stringWithFormat:@"Adapter response failed: %@",
                                                    reason]];
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

    switch (_phase) {
    case MBLinkDiagnosticsPhaseInitializing:
        [self processInitializationResponse:response];
        break;
    case MBLinkDiagnosticsPhaseCheckingPids:
        [self processSupportedPidResponse:response];
        break;
    case MBLinkDiagnosticsPhaseReadingRPM:
        [self processLiveResponse:response pid:0x0cU];
        break;
    case MBLinkDiagnosticsPhaseReadingCoolant:
        [self processLiveResponse:response pid:0x05U];
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
    }

    if (_initialization.stage == MBLINK_ELM327_INIT_COMPLETE) {
        char command[8];
        MblinkObd2Result build =
            mblink_obd2_build_supported_pid_request(0x00U,
                                                     command,
                                                     sizeof(command));
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
        mblink_obd2_accept_supported_pids(response,
                                          0x00U,
                                          &_supportedPids,
                                          &hasMore);
    if (result != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        [self setStatus:@"Vehicle did not provide a valid OBD-II PID map"];
        return;
    }

    _rpmSupported = mblink_obd2_pid_set_contains(&_supportedPids, 0x0cU);
    _coolantSupported = mblink_obd2_pid_set_contains(&_supportedPids, 0x05U);
    (void)hasMore;

    self.ready = YES;
    [self notifyDelegate];

    if (!_rpmSupported && !_coolantSupported) {
        _phase = MBLinkDiagnosticsPhaseLive;
        [self setStatus:@"Connected; RPM and coolant PIDs are not advertised"];
        return;
    }

    [self scheduleNextLiveRequestAfter:0.0];
}

- (void)scheduleNextLiveRequestAfter:(NSTimeInterval)delay
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
                                 (int64_t)(delay * NSEC_PER_SEC)),
                   dispatch_get_main_queue(), ^{
        if (!self->_sessionInitialized || !self->_provider.isReady) {
            return;
        }

        char command[8];
        uint8_t pid;
        if (self->_rpmSupported &&
            (self->_phase != MBLinkDiagnosticsPhaseReadingRPM)) {
            pid = 0x0cU;
            self->_phase = MBLinkDiagnosticsPhaseReadingRPM;
        } else if (self->_coolantSupported) {
            pid = 0x05U;
            self->_phase = MBLinkDiagnosticsPhaseReadingCoolant;
        } else {
            pid = 0x0cU;
            self->_phase = MBLinkDiagnosticsPhaseReadingRPM;
        }

        MblinkObd2Result result =
            mblink_obd2_build_live_pid_request(pid, command, sizeof(command));
        if (result != MBLINK_OBD2_RESULT_OK) {
            self->_phase = MBLinkDiagnosticsPhaseFailed;
            [self setStatus:@"Could not build live OBD-II request"];
            return;
        }
        (void)[self beginCommand:command timeout:2000U];
    });
}

- (void)processLiveResponse:(const MblinkElm327Response *)response
                        pid:(uint8_t)pid
{
    MblinkObd2Sample sample;
    MblinkObd2Result result =
        mblink_obd2_decode_live_pid(response, pid, &sample);
    if (result != MBLINK_OBD2_RESULT_OK) {
        _phase = MBLinkDiagnosticsPhaseFailed;
        NSString *reason =
            [NSString stringWithUTF8String:mblink_obd2_result_name(result)];
        [self setStatus:[NSString stringWithFormat:@"Live OBD-II decode failed: %@",
                                                    reason]];
        return;
    }

    if (pid == 0x0cU) {
        self.rpm = sample.value;
        self.hasRPM = YES;
    } else if (pid == 0x05U) {
        self.coolantTemperatureCelsius = sample.value;
        self.hasCoolantTemperature = YES;
    }

    self.ready = YES;
    self.statusText = @"Live OBD-II data";
    [self notifyDelegate];

    [self scheduleNextLiveRequestAfter:0.35];
}

@end
