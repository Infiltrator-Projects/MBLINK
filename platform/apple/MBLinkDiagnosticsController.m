// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "MBLinkBLETransport+MBLINK.h"
#import "mblink/elm327.h"
#import "mblink/elm327_session.h"
#import "mblink/mercedes.h"
#import "mblink/mercedes_probe.h"
#import "mblink/mercedes_module_scan.h"
#import "mblink/telemetry.h"
#import "link/diagnostic_flow.h"
#import "link/elm327_simulator.h"

#include <stdio.h>
#include <string.h>

@interface MBLinkDiagnosticsController () <MBLinkBLETransportDelegate>
@property(nonatomic, copy, readwrite) NSString *statusText;
@property(nonatomic, copy, readwrite, nullable) NSString *peripheralName;
@property(nonatomic, copy, readwrite, nullable) NSString *adapterIdentifier;
@property(nonatomic, copy, readwrite) NSString *mercedesProbeStatusText;
@property(nonatomic, copy, readwrite, nullable) NSString *mercedesProbeEndpointText;
@property(nonatomic, copy, readwrite, nullable) NSString *mercedesVINText;
@property(nonatomic, copy, readwrite) NSString *mercedesIdentitySummaryText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesIdentityResults;
@property(nonatomic, copy, readwrite) NSString *mercedesCrd3SummaryText;
@property(nonatomic, copy, readwrite) NSString *mercedesUDSFaultStatusText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesUDSFaults;
@property(nonatomic, copy, readwrite) NSString *faultScanStatusText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *storedDTCs;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *pendingDTCs;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *permanentDTCs;
@property(nonatomic, readwrite, getter=isActive) BOOL active;
@property(nonatomic, readwrite, getter=isReady) BOOL ready;

- (void)prepareForStart;
- (void)handleSessionEvent:(const MblinkElm327Session *)session;
- (void)processCompletedResponse;
- (void)beginPortableSession;
- (BOOL)beginCommand:(const char *)command timeout:(uint64_t)timeoutMs;
- (void)startTickTimer;
- (void)stopTickTimer;
- (void)driveDiagnosticFlow;
- (BOOL)applyFlowEvent:(const LinkDiagnosticFlowEvent *)event;
- (void)markFlowFailure:(NSString *)status;
- (void)beginMercedesProbe;
- (void)beginCurrentMercedesProbeCommand;
- (void)processMercedesProbeResponse:(const MblinkElm327Response *)response;
- (void)beginMercedesModuleScan;
- (void)beginCurrentMercedesModuleScanCommand;
- (void)processMercedesModuleScanResponse:(const MblinkElm327Response *)response;
- (void)updateMercedesModuleScanSummary;
- (void)finishMercedesExtensionRestoringAdapter:(BOOL)restore;
- (void)updateMercedesProbeEvidenceSummary;
- (NSString *)mercedesProbeFailureText;
@end

@implementation MBLinkDiagnosticsController {
    MBLinkBLETransport *_provider;
    MblinkElm327Session _session;
    BOOL _sessionInitialized;
    BOOL _simulated;
    LinkElm327Simulator _simulator;
    LinkDiagnosticFlow _flow;
    MblinkMercedesEcuProbe _mercedesProbe;
    BOOL _manufacturerProbeActive;
    MblinkMercedesModuleScan _mercedesModuleScan;
    BOOL _moduleScanActive;
    MblinkTelemetryStore _telemetry;
    MblinkTelemetryRecorder _recorder;
    MblinkTelemetrySessionMetadata _sessionMetadata;
    NSMutableData *_sessionCSV;
    dispatch_source_t _tickTimer;
    NSUInteger _pollGeneration;
    uint64_t _sessionMonotonicStartMs;
}

static uint64_t MBLinkMonotonicMilliseconds(void)
{
    NSTimeInterval uptime = NSProcessInfo.processInfo.systemUptime;
    if (uptime <= 0.0) return 0U;
    const double milliseconds = uptime * 1000.0;
    return milliseconds >= (double)UINT64_MAX ? UINT64_MAX : (uint64_t)milliseconds;
}

static uint64_t MBLinkElapsedMilliseconds(uint64_t startedMs)
{
    const uint64_t nowMs = MBLinkMonotonicMilliseconds();
    return nowMs >= startedMs ? nowMs - startedMs : 0U;
}

static uint64_t MBLinkEpochMilliseconds(void)
{
    NSTimeInterval seconds = [NSDate date].timeIntervalSince1970;
    if (seconds <= 0.0) return 0U;
    const double milliseconds = seconds * 1000.0;
    return milliseconds >= (double)UINT64_MAX ? UINT64_MAX : (uint64_t)milliseconds;
}

static unsigned int MBLinkBitCount32(uint32_t value)
{
    unsigned int count = 0U;
    while (value != 0U) { count += value & 1U; value >>= 1U; }
    return count;
}

static bool MBLinkAppendCSV(void *context, const char *bytes, size_t length)
{
    if (context == NULL || bytes == NULL) return false;
    NSMutableData *data = (__bridge NSMutableData *)context;
    [data appendBytes:bytes length:length];
    return true;
}

static void MBLinkSessionEvent(void *context, const MblinkElm327Session *session)
{
    MBLinkDiagnosticsController *controller = (__bridge MBLinkDiagnosticsController *)context;
    if (controller != nil && session != NULL) [controller handleSessionEvent:session];
}

static NSString *MBLinkStringFromCString(const char *value)
{
    if (value == NULL) return @"unknown";
    NSString *string = [NSString stringWithUTF8String:value];
    return string != nil ? string : @"unknown";
}

static NSString *MBLinkMercedesEndpointText(const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    if (endpoint == NULL) return nil;
    NSString *name = MBLinkStringFromCString(endpoint->name);
    if (endpoint->address.tx_extended_id) {
        return [NSString stringWithFormat:@"%@ · 0x%08X → 0x%08X", name,
            (unsigned int)endpoint->address.tx_can_id,
            (unsigned int)endpoint->address.rx_can_id];
    }
    return [NSString stringWithFormat:@"%@ · 0x%03X → 0x%03X", name,
        (unsigned int)endpoint->address.tx_can_id,
        (unsigned int)endpoint->address.rx_can_id];
}

static NSArray<NSString *> *MBLinkDTCStrings(const LinkObd2DtcList *list)
{
    if (list == NULL || list->count == 0U) return @[];
    NSMutableArray<NSString *> *values = [[NSMutableArray alloc] initWithCapacity:list->count];
    for (size_t index = 0U; index < list->count; ++index) {
        NSString *code = MBLinkStringFromCString(list->entries[index].code);
        if (code.length != 0U) [values addObject:code];
    }
    return [values copy];
}

static NSArray<NSString *> *MBLinkMercedesUDSDTCStrings(const MblinkUdsDtcList *list)
{
    if (list == NULL || list->count == 0U) return @[];
    NSMutableArray<NSString *> *values = [[NSMutableArray alloc] initWithCapacity:list->count];
    for (size_t index = 0U; index < list->count; ++index) {
        char code[7];
        if (!mblink_uds_dtc_format_hex(list->records[index].code, code, sizeof(code))) continue;
        [values addObject:[NSString stringWithFormat:@"%@ · status 0x%02X",
            MBLinkStringFromCString(code), (unsigned int)list->records[index].status]];
    }
    return [values copy];
}

static BOOL MBLinkFlowIsFaultScan(const LinkDiagnosticFlow *flow)
{
    if (flow == NULL) return NO;
    return flow->stage == LINK_DIAGNOSTIC_FLOW_SCANNING_STORED_DTCS ||
           flow->stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PENDING_DTCS ||
           flow->stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PERMANENT_DTCS;
}

static bool MBLinkSimulatorResponder(void *context, const char *command,
                                     char *response, size_t responseSize)
{
    (void)context;
    struct Pair { const char *command; const char *response; };
    static const struct Pair replies[] = {
        { "3E00", "7E00" },
        { "22F190", "62F1905744443230373330323246313233343536" },
        { "22F18C", "62F18CAA" },
        { "22F187", "62F187AA" },
        { "22F188", "62F188AA" },
        { "22F189", "62F189AA" },
        { "22F191", "62F191AA" },
        { "22F197", "62F197AA" },
        { "22F100", "62F10002100001" },
        { "22F154", "62F15440" },
        { "22F196", "62F196010203040506" },
        { "221001", "621001AABBCCDD" },
        { "221002", "6210021122" },
        { "1902FF", "5902FF12345609ABCDEF28" }
    };
    for (size_t index = 0U; index < sizeof(replies) / sizeof(replies[0]); ++index) {
        if (strcmp(command, replies[index].command) != 0) continue;
        const int written = snprintf(response, responseSize, "%s", replies[index].response);
        return written >= 0 && (size_t)written < responseSize;
    }
    return false;
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
        _mercedesCrd3SummaryText = @"Not attempted";
        _mercedesUDSFaultStatusText = @"Not scanned";
        _mercedesUDSFaults = @[];
        _faultScanStatusText = @"Not scanned";
        _storedDTCs = @[];
        _pendingDTCs = @[];
        _permanentDTCs = @[];
        LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
        flowConfig.manufacturer_extension_after_pid_discovery = true;
        flowConfig.restore_adapter_after_manufacturer_extension = true;
        (void)link_diagnostic_flow_init(&_flow, &flowConfig);
        mblink_telemetry_store_init(&_telemetry);
        mblink_telemetry_recorder_init(&_recorder);
        _sessionCSV = [[NSMutableData alloc] init];
        mblink_telemetry_store_set_favourite(&_telemetry, 0x0cU, true);
        mblink_telemetry_store_set_favourite(&_telemetry, 0x0dU, true);
        mblink_telemetry_store_set_favourite(&_telemetry, 0x05U, true);
        mblink_telemetry_store_set_favourite(&_telemetry, 0x0bU, true);
        mblink_telemetry_session_metadata_init(&_sessionMetadata, 0U, NULL, NULL);
    }
    return self;
}

- (void)dealloc
{
    _provider.delegate = nil;
    [self stopTickTimer];
    if (_recorder.started && !_recorder.finished) {
        (void)mblink_telemetry_recorder_finish(&_recorder, MBLinkEpochMilliseconds());
    }
    if (_sessionInitialized) {
        _sessionInitialized = NO;
        mblink_elm327_session_disconnect(&_session);
        mblink_elm327_session_deinit(&_session);
    } else if (!_simulated) {
        [_provider disconnect];
    }
}

- (void)notifyDelegate
{
    id<MBLinkDiagnosticsControllerDelegate> delegate = self.delegate;
    if (delegate != nil) [delegate diagnosticsControllerDidUpdate:self];
}

- (void)setStatus:(NSString *)status
{
    self.statusText = status;
    [self notifyDelegate];
}

- (void)prepareForStart
{
    _pollGeneration++;
    self.active = YES;
    self.ready = NO;
    self.mercedesProbeStatusText = @"Not attempted";
    self.mercedesProbeEndpointText = nil;
    self.mercedesVINText = nil;
    self.mercedesIdentitySummaryText = @"Not attempted";
    self.mercedesIdentityResults = @[];
    self.mercedesCrd3SummaryText = @"Not attempted";
    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes ECU probe";
    self.mercedesUDSFaults = @[];
    self.faultScanStatusText = @"Waiting for vehicle connection";
    self.storedDTCs = @[];
    self.pendingDTCs = @[];
    self.permanentDTCs = @[];
    _mercedesProbe = (MblinkMercedesEcuProbe){0};
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
    flowConfig.manufacturer_extension_after_pid_discovery = true;
    flowConfig.restore_adapter_after_manufacturer_extension = true;
    (void)link_diagnostic_flow_init(&_flow, &flowConfig);
    mblink_telemetry_store_clear_samples(&_telemetry);
    mblink_telemetry_recorder_init(&_recorder);
    _sessionCSV = [[NSMutableData alloc] init];
    _sessionMonotonicStartMs = MBLinkMonotonicMilliseconds();
    mblink_telemetry_session_metadata_init(&_sessionMetadata, MBLinkEpochMilliseconds(), NULL, NULL);
}

- (void)start
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{ [self start]; });
        return;
    }
    if (self.active) return;
    _simulated = NO;
    [self prepareForStart];
    self.peripheralName = nil;
    self.adapterIdentifier = nil;
    [self notifyDelegate];
    [_provider start];
}

- (void)startSimulated
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{ [self startSimulated]; });
        return;
    }
    if (self.active) return;

    _simulated = YES;
    [self prepareForStart];
    self.peripheralName = @"Simulated ELM327";
    self.adapterIdentifier = nil;

    LinkElm327SimulatorConfig config = LINK_ELM327_SIMULATOR_CONFIG_INIT;
    config.adapter_identifier = "ELM327 v2.3 MBLINK SIM";
    config.vin = "WDD2073022F123456";
    config.custom_responder = MBLinkSimulatorResponder;
    link_elm327_simulator_init(&_simulator, &config);
    [self notifyDelegate];
    [self beginPortableSession];
}

- (void)disconnect
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{ [self disconnect]; });
        return;
    }

    _pollGeneration++;
    [self stopTickTimer];
    if (_sessionInitialized) {
        _sessionInitialized = NO;
        mblink_elm327_session_disconnect(&_session);
        mblink_elm327_session_deinit(&_session);
    } else if (!_simulated) {
        [_provider disconnect];
    }
    const uint64_t endedEpochMs = MBLinkEpochMilliseconds();
    mblink_telemetry_session_metadata_finish(&_sessionMetadata, endedEpochMs);
    if (_recorder.started && !_recorder.finished) {
        (void)mblink_telemetry_recorder_finish(&_recorder, endedEpochMs);
    }
    LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
    flowConfig.manufacturer_extension_after_pid_discovery = true;
    flowConfig.restore_adapter_after_manufacturer_extension = true;
    (void)link_diagnostic_flow_init(&_flow, &flowConfig);
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    _simulated = NO;
    self.active = NO;
    self.ready = NO;
    [self setStatus:@"Disconnected"];
}

- (void)bleTransportDidUpdate:(MBLinkBLETransport *)transport
{
    if (_simulated) return;
    self.peripheralName = transport.peripheralName;
    self.adapterIdentifier = transport.adapterIdentifier;
    if (transport.adapterIdentifier != nil) {
        mblink_telemetry_session_metadata_set_adapter(&_sessionMetadata, transport.adapterIdentifier.UTF8String);
    }

    if (transport.isReady && !_sessionInitialized) {
        [self beginPortableSession];
        return;
    }
    if (!transport.isReady && _sessionInitialized && transport.state != MBLinkBLETransportStateProbing) {
        _pollGeneration++;
        [self stopTickTimer];
        _sessionInitialized = NO;
        mblink_elm327_session_deinit(&_session);
        link_diagnostic_flow_fail(&_flow, LINK_DIAGNOSTIC_FLOW_RESULT_ELM_ERROR);
        _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
        self.ready = NO;
    }
    if (!_sessionInitialized) self.statusText = transport.statusText;
    [self notifyDelegate];
}

- (void)beginPortableSession
{
    MblinkTransport transport = _simulated
        ? link_elm327_simulator_transport(&_simulator)
        : MBLinkBLETransportMakeCTransport(_provider);
    if (!mblink_transport_is_valid(&transport) ||
        !mblink_elm327_session_init(&_session, &transport, MBLinkSessionEvent, (__bridge void *)self)) {
        [self markFlowFailure:@"Failed to initialise portable diagnostic session"];
        return;
    }

    _sessionInitialized = YES;
    if (_simulated && mblink_elm327_session_connect(&_session) != MBLINK_TRANSPORT_OK) {
        _sessionInitialized = NO;
        mblink_elm327_session_deinit(&_session);
        [self markFlowFailure:@"Failed to connect simulated ELM327 transport"];
        return;
    }
    if (!_recorder.started &&
        !mblink_telemetry_recorder_begin(&_recorder, &_sessionMetadata,
                                         MBLinkAppendCSV, (__bridge void *)_sessionCSV)) {
        _sessionInitialized = NO;
        mblink_elm327_session_disconnect(&_session);
        mblink_elm327_session_deinit(&_session);
        [self markFlowFailure:@"Could not start portable session recorder"];
        return;
    }

    LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
    flowConfig.manufacturer_extension_after_pid_discovery = true;
    flowConfig.restore_adapter_after_manufacturer_extension = true;
    (void)link_diagnostic_flow_init(&_flow, &flowConfig);
    if (link_diagnostic_flow_start(&_flow) != LINK_DIAGNOSTIC_FLOW_RESULT_OK) {
        [self markFlowFailure:@"Could not start shared diagnostic flow"];
        return;
    }
    [self startTickTimer];
    [self setStatus:_simulated ? @"Initialising simulated ELM327 adapter" : @"Initialising ELM327 adapter"];
    [self driveDiagnosticFlow];
}

- (void)startTickTimer
{
    [self stopTickTimer];
    _tickTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0U, 0U, dispatch_get_main_queue());
    dispatch_source_set_timer(_tickTimer,
                              dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC),
                              100 * NSEC_PER_MSEC, 20 * NSEC_PER_MSEC);
    __weak MBLinkDiagnosticsController *weakSelf = self;
    dispatch_source_set_event_handler(_tickTimer, ^{
        MBLinkDiagnosticsController *strongSelf = weakSelf;
        if (strongSelf == nil || !strongSelf->_sessionInitialized) return;
        (void)mblink_elm327_session_tick(&strongSelf->_session, MBLinkMonotonicMilliseconds());
    });
    dispatch_resume(_tickTimer);
}

- (void)stopTickTimer
{
    if (_tickTimer != nil) { dispatch_source_cancel(_tickTimer); _tickTimer = nil; }
}

- (BOOL)beginCommand:(const char *)command timeout:(uint64_t)timeoutMs
{
    if (!_sessionInitialized || command == NULL) return NO;
    MblinkElm327SessionOpResult result = mblink_elm327_session_begin(
        &_session, command, MBLinkMonotonicMilliseconds(), timeoutMs);
    if (result != MBLINK_ELM327_SESSION_OP_OK) {
        NSString *reason = MBLinkStringFromCString(mblink_elm327_session_op_result_name(result));
        link_diagnostic_flow_fail(&_flow, LINK_DIAGNOSTIC_FLOW_RESULT_ELM_ERROR);
        [self setStatus:[NSString stringWithFormat:@"Diagnostic command failed: %@", reason]];
        return NO;
    }
    return YES;
}

- (void)handleSessionEvent:(const MblinkElm327Session *)session
{
    if (session->status == MBLINK_ELM327_SESSION_COMPLETE) {
        dispatch_async(dispatch_get_main_queue(), ^{ [self processCompletedResponse]; });
        return;
    }
    if (session->status == MBLINK_ELM327_SESSION_TIMED_OUT) {
        if (_manufacturerProbeActive) {
            self.mercedesProbeStatusText = @"Probe timed out; reconnect required to resynchronise the adapter";
            self.mercedesIdentitySummaryText = @"Probe did not complete";
            if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION) {
                self.mercedesUDSFaultStatusText = @"Mercedes UDS fault read timed out";
            }
        }
        if (_moduleScanActive) { self.mercedesProbeStatusText = @"Mercedes module scan timed out"; self.mercedesUDSFaultStatusText = @"Module fault inventory timed out"; }
        if (MBLinkFlowIsFaultScan(&_flow)) self.faultScanStatusText = @"Fault scan timed out; reconnect required";
        _flow.elm_failure = session->elm_result;
        link_diagnostic_flow_fail(&_flow, LINK_DIAGNOSTIC_FLOW_RESULT_ELM_ERROR);
        _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
        [self setStatus:@"Diagnostic request timed out; reconnect to resynchronise"];
        return;
    }
    if (session->status == MBLINK_ELM327_SESSION_FAILED) {
        NSString *reason = MBLinkStringFromCString(mblink_elm327_result_name(session->elm_result));
        if (_manufacturerProbeActive) {
            self.mercedesProbeStatusText = [NSString stringWithFormat:@"Adapter response failed during probe: %@", reason];
            self.mercedesIdentitySummaryText = @"Probe did not complete";
            if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION) {
                self.mercedesUDSFaultStatusText = [NSString stringWithFormat:@"Mercedes UDS fault read adapter error: %@", reason];
            }
        }
        if (_moduleScanActive) { self.mercedesProbeStatusText = [NSString stringWithFormat:@"Mercedes module scan adapter error: %@", reason]; self.mercedesUDSFaultStatusText = @"Module fault inventory interrupted"; }
        if (MBLinkFlowIsFaultScan(&_flow)) {
            self.faultScanStatusText = [NSString stringWithFormat:@"Fault scan adapter error: %@", reason];
        }
        _flow.elm_failure = session->elm_result;
        link_diagnostic_flow_fail(&_flow, LINK_DIAGNOSTIC_FLOW_RESULT_ELM_ERROR);
        _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
        [self setStatus:[NSString stringWithFormat:@"Adapter response failed: %@", reason]];
        return;
    }
    if (session->status == MBLINK_ELM327_SESSION_CANCELLED) {
        LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
        flowConfig.manufacturer_extension_after_pid_discovery = true;
        flowConfig.restore_adapter_after_manufacturer_extension = true;
        (void)link_diagnostic_flow_init(&_flow, &flowConfig);
        _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
        [self setStatus:@"Diagnostic request cancelled"];
    }
}

- (void)processCompletedResponse
{
    const MblinkElm327Response *response = mblink_elm327_session_response(&_session);
    if (response == NULL) { [self markFlowFailure:@"Diagnostic response was unavailable"]; return; }

    (void)mblink_telemetry_store_record_transcript(
        &_telemetry, MBLinkElapsedMilliseconds(_sessionMonotonicStartMs),
        _session.parser.command, response);
    if (_recorder.started && !_recorder.finished &&
        !mblink_telemetry_recorder_record_response(
            &_recorder, MBLinkElapsedMilliseconds(_sessionMonotonicStartMs),
            _session.parser.command, response)) {
        [self markFlowFailure:@"Could not append diagnostic transcript"];
        return;
    }

    if (_moduleScanActive) { [self processMercedesModuleScanResponse:response]; return; }
    if (_manufacturerProbeActive) { [self processMercedesProbeResponse:response]; return; }

    LinkDiagnosticFlowEvent event;
    LinkDiagnosticFlowResult result = link_diagnostic_flow_accept_response(
        &_flow, (const LinkElm327Response *)response,
        MBLinkMonotonicMilliseconds(), &event);
    if (result != LINK_DIAGNOSTIC_FLOW_RESULT_OK) {
        NSString *reason = MBLinkStringFromCString(link_diagnostic_flow_result_name(result));
        [self setStatus:[NSString stringWithFormat:@"Shared diagnostic flow failed: %@", reason]];
        return;
    }
    if (![self applyFlowEvent:&event]) return;
    [self driveDiagnosticFlow];
}

- (void)driveDiagnosticFlow
{
    const BOOL transportReady = _simulated
        ? (_sessionInitialized && mblink_elm327_session_is_connected(&_session))
        : _provider.isReady;
    if (!_sessionInitialized || !transportReady ||
        _flow.stage == LINK_DIAGNOSTIC_FLOW_FAILED || _manufacturerProbeActive || _moduleScanActive) return;

    LinkDiagnosticFlowAction action;
    LinkDiagnosticFlowResult result = link_diagnostic_flow_next_action(
        &_flow, MBLinkMonotonicMilliseconds(), &action);
    if (result != LINK_DIAGNOSTIC_FLOW_RESULT_OK) {
        NSString *reason = MBLinkStringFromCString(link_diagnostic_flow_result_name(result));
        [self setStatus:[NSString stringWithFormat:@"Shared diagnostic flow failed: %@", reason]];
        return;
    }

    switch (action.kind) {
    case LINK_DIAGNOSTIC_FLOW_ACTION_NONE:
        return;
    case LINK_DIAGNOSTIC_FLOW_ACTION_SEND_COMMAND:
        if (_flow.stage == LINK_DIAGNOSTIC_FLOW_INITIALIZING) {
            self.statusText = _simulated ? @"Initialising simulated ELM327 adapter" : @"Initialising ELM327 adapter";
        } else if (_flow.stage == LINK_DIAGNOSTIC_FLOW_RESTORING_AFTER_MANUFACTURER) {
            self.statusText = @"Restoring standard OBD-II adapter channel";
        } else if (_flow.stage == LINK_DIAGNOSTIC_FLOW_DISCOVERING_PIDS) {
            self.statusText = _flow.supported_pid_base == 0U
                ? @"Checking standard OBD-II capabilities"
                : [NSString stringWithFormat:@"Checking OBD-II PID block 0x%02X", (unsigned int)_flow.supported_pid_base];
        } else if (_flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_STORED_DTCS) {
            self.faultScanStatusText = @"Scanning stored, pending and permanent OBD-II faults";
            self.statusText = @"Scanning stored OBD-II fault codes";
        } else if (_flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PENDING_DTCS) {
            self.statusText = @"Scanning pending OBD-II fault codes";
        } else if (_flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PERMANENT_DTCS) {
            self.statusText = @"Scanning permanent OBD-II fault codes";
        } else if (_flow.stage == LINK_DIAGNOSTIC_FLOW_READING_LIVE) {
            self.statusText = _simulated ? @"Simulated ELM327 · live OBD-II and diesel data" : @"Live OBD-II and diesel scheduler active";
        }
        [self notifyDelegate];
        (void)[self beginCommand:action.command timeout:action.timeout_ms];
        return;
    case LINK_DIAGNOSTIC_FLOW_ACTION_WAIT: {
        uint64_t waitMs = action.wait_ms > 60000U ? 60000U : action.wait_ms;
        const NSUInteger generation = _pollGeneration;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)waitMs * NSEC_PER_MSEC),
                       dispatch_get_main_queue(), ^{
            if (generation == self->_pollGeneration) [self driveDiagnosticFlow];
        });
        return;
    }
    case LINK_DIAGNOSTIC_FLOW_ACTION_MANUFACTURER_EXTENSION:
        [self beginMercedesProbe];
        return;
    case LINK_DIAGNOSTIC_FLOW_ACTION_READY:
        self.ready = YES;
        if (_flow.scheduler.count == 0U) {
            [self setStatus:@"Connected; no supported dashboard PIDs were advertised"];
        } else {
            [self setStatus:_simulated ? @"Simulated ELM327 · live OBD-II and diesel data" : @"Live OBD-II and diesel scheduler active"];
        }
        return;
    case LINK_DIAGNOSTIC_FLOW_ACTION_FAILED:
        [self markFlowFailure:@"Shared diagnostic flow entered the failed state"];
        return;
    }
}

- (BOOL)applyFlowEvent:(const LinkDiagnosticFlowEvent *)event
{
    if (event == NULL) return NO;
    switch (event->kind) {
    case LINK_DIAGNOSTIC_FLOW_EVENT_NONE:
        return YES;
    case LINK_DIAGNOSTIC_FLOW_EVENT_ADAPTER_IDENTIFIED: {
        const char *identifier = link_diagnostic_flow_adapter_identifier(&_flow);
        if (identifier != NULL) {
            self.adapterIdentifier = MBLinkStringFromCString(identifier);
            mblink_telemetry_session_metadata_set_adapter(&_sessionMetadata, identifier);
        }
        return YES;
    }
    case LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE:
        return YES;
    case LINK_DIAGNOSTIC_FLOW_EVENT_DTC_LIST: {
        NSArray<NSString *> *codes = MBLinkDTCStrings(event->dtc_list);
        switch (event->dtc_kind) {
        case LINK_OBD2_DTC_STORED: self.storedDTCs = codes; break;
        case LINK_OBD2_DTC_PENDING: self.pendingDTCs = codes; break;
        case LINK_OBD2_DTC_PERMANENT:
            self.permanentDTCs = codes;
            self.faultScanStatusText = [NSString stringWithFormat:
                @"Complete · %lu stored · %lu pending · %lu permanent",
                (unsigned long)self.storedDTCs.count,
                (unsigned long)self.pendingDTCs.count,
                (unsigned long)self.permanentDTCs.count];
            break;
        }
        if (event->became_ready) self.ready = YES;
        [self notifyDelegate];
        return YES;
    }
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE: {
        const MblinkObd2Sample *sample = (const MblinkObd2Sample *)&event->sample;
        if (!mblink_telemetry_store_record(
                &_telemetry, MBLinkElapsedMilliseconds(_sessionMonotonicStartMs), sample)) {
            [self markFlowFailure:@"Could not record live telemetry sample"];
            return NO;
        }
        MblinkTelemetrySample recorded;
        if (_recorder.started && !_recorder.finished &&
            mblink_telemetry_store_latest(&_telemetry, sample->pid, &recorded) &&
            !mblink_telemetry_recorder_record_sample(
                &_recorder, &recorded,
                mblink_telemetry_store_is_favourite(&_telemetry, sample->pid))) {
            [self markFlowFailure:@"Could not append session recording"];
            return NO;
        }
        self.ready = YES;
        self.statusText = _simulated ? @"Simulated ELM327 · live OBD-II and diesel data" : @"Live OBD-II and diesel data";
        [self notifyDelegate];
        return YES;
    }
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_NO_DATA:
        self.statusText = @"Live OBD-II data; one PID returned no data";
        [self notifyDelegate];
        return YES;
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_UNSUPPORTED:
        self.statusText = @"Live OBD-II data; one advertised diesel sub-field is unavailable";
        [self notifyDelegate];
        return YES;
    }
    return YES;
}

- (void)markFlowFailure:(NSString *)status
{
    link_diagnostic_flow_fail(&_flow, LINK_DIAGNOSTIC_FLOW_RESULT_INVALID_STATE);
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    self.ready = NO;
    [self setStatus:status];
}

- (void)beginMercedesProbe
{
    const MblinkMercedesVehicleProfile *profile = mblink_mercedes_c207_om651_profile();
    if (profile == NULL || !mblink_mercedes_vehicle_profile_is_valid(profile)) {
        self.mercedesProbeStatusText = @"C207 / OM651 development profile unavailable";
        self.mercedesIdentitySummaryText = @"Not attempted";
        self.mercedesCrd3SummaryText = @"Not attempted";
        self.mercedesUDSFaultStatusText = @"Not attempted";
        [self finishMercedesExtensionRestoringAdapter:NO];
        return;
    }

    const MblinkMercedesEcuEndpointDefinition *endpoint = mblink_mercedes_profile_find_endpoint(
        profile, "c207-om651-engine-eobd-11bit");
    if (endpoint == NULL) {
        self.mercedesProbeStatusText = @"No engine endpoint candidate is defined";
        self.mercedesIdentitySummaryText = @"Not attempted";
        self.mercedesCrd3SummaryText = @"Not attempted";
        self.mercedesUDSFaultStatusText = @"Not attempted";
        [self finishMercedesExtensionRestoringAdapter:NO];
        return;
    }

    self.mercedesProbeEndpointText = MBLinkMercedesEndpointText(endpoint);
    MblinkMercedesEcuProbeResult result = mblink_mercedes_ecu_probe_begin(&_mercedesProbe, endpoint);
    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK) {
        self.mercedesProbeStatusText = [NSString stringWithFormat:@"Probe could not start: %@",
            MBLinkStringFromCString(mblink_mercedes_ecu_probe_result_name(result))];
        self.mercedesIdentitySummaryText = @"Not attempted";
        self.mercedesCrd3SummaryText = @"Not attempted";
        self.mercedesUDSFaultStatusText = @"Not attempted";
        [self finishMercedesExtensionRestoringAdapter:NO];
        return;
    }

    _manufacturerProbeActive = YES;
    _flow.config.restore_adapter_after_manufacturer_extension = true;
    self.mercedesProbeStatusText = @"Probing Delphi CRD3.x candidate with read-only UDS TesterPresent";
    self.mercedesIdentitySummaryText = @"Waiting for UDS endpoint response";
    self.mercedesCrd3SummaryText = @"Waiting for CRD3 fingerprint";
    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes UDS fault read";
    [self setStatus:@"Probing Mercedes-Benz CRD3.x engine ECU (read-only)"];
    [self beginCurrentMercedesProbeCommand];
}

- (void)beginCurrentMercedesProbeCommand
{
    char command[MBLINK_ELM327_MAX_COMMAND];
    size_t written = 0U;
    MblinkMercedesEcuProbeResult result = mblink_mercedes_ecu_probe_command(
        &_mercedesProbe, command, sizeof(command), &written);
    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK || written == 0U) {
        self.mercedesProbeStatusText = [NSString stringWithFormat:@"Probe command failed: %@",
            MBLinkStringFromCString(mblink_mercedes_ecu_probe_result_name(result))];
        self.mercedesIdentitySummaryText = @"Probe did not complete";
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }

    if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN) {
        self.mercedesProbeStatusText = @"UDS endpoint confirmed; reading standardized VIN (F190)";
        self.mercedesIdentitySummaryText = @"Reading standardized vehicle identity";
        [self notifyDelegate];
    } else if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY) {
        const size_t index = _mercedesProbe.identity_index;
        const uint16_t did = mblink_mercedes_ecu_probe_identity_did_at(index);
        const char *name = mblink_mercedes_ecu_probe_identity_did_name(index);
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Reading standardized ECU identity %zu/%u · %04X · %@", index + 1U,
            (unsigned int)MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT,
            (unsigned int)did, MBLinkStringFromCString(name)];
        self.mercedesIdentitySummaryText = [NSString stringWithFormat:
            @"Standard identity sweep in progress · %zu/%u", index + 1U,
            (unsigned int)MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT];
        [self notifyDelegate];
    } else if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_CRD3_FINGERPRINT) {
        const size_t index = _mercedesProbe.crd3_index;
        const uint16_t did = mblink_mercedes_ecu_probe_crd3_did_at(index);
        const char *name = mblink_mercedes_ecu_probe_crd3_did_name(index);
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Reading CRD3 fingerprint %zu/%u · %04X · %@", index + 1U,
            (unsigned int)MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT,
            (unsigned int)did, MBLinkStringFromCString(name)];
        self.mercedesCrd3SummaryText = [NSString stringWithFormat:
            @"CRD3 fingerprint in progress · %zu/%u", index + 1U,
            (unsigned int)MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT];
        [self notifyDelegate];
    } else if (_mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION) {
        self.mercedesProbeStatusText = @"Reading Mercedes UDS fault memory (19 02 FF)";
        self.mercedesUDSFaultStatusText = @"Reading Mercedes UDS fault memory";
        [self notifyDelegate];
    }
    (void)[self beginCommand:command timeout:4000U];
}

- (void)processMercedesProbeResponse:(const MblinkElm327Response *)response
{
    MblinkMercedesEcuProbeResult result = mblink_mercedes_ecu_probe_accept(&_mercedesProbe, response);
    if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
        [self updateMercedesProbeEvidenceSummary];
        [self beginMercedesModuleScan];
        return;
    }
    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
        _mercedesProbe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED) {
        self.mercedesProbeStatusText = [self mercedesProbeFailureText];
        self.mercedesIdentitySummaryText = @"Probe did not complete";
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    [self beginCurrentMercedesProbeCommand];
}

- (void)beginMercedesModuleScan
{
    if (_simulated) {
        memset(&_mercedesModuleScan, 0, sizeof(_mercedesModuleScan));
        _mercedesModuleScan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE;
        _mercedesModuleScan.module_count = 1U;
        MblinkMercedesModuleScanEntry *module = &_mercedesModuleScan.modules[0];
        module->tx_can_id = UINT32_C(0x7e0);
        module->rx_can_id = UINT32_C(0x7e8);
        module->kind = MBLINK_MERCEDES_MODULE_ENGINE;
        module->tester_present_response = true;
        module->dtcs = _mercedesProbe.dtcs;
        module->dtc_result = _mercedesProbe.dtc_result == MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE
  ? MBLINK_MERCEDES_MODULE_DTC_AVAILABLE : MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE;
        [self updateMercedesModuleScanSummary];
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_begin(&_mercedesModuleScan);
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
        self.mercedesProbeStatusText = @"Mercedes module discovery could not start";
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    _moduleScanActive = YES;
    self.mercedesProbeStatusText = @"Discovering Mercedes-Benz control modules (read-only)";
    self.mercedesUDSFaultStatusText = @"Module discovery in progress";
    [self notifyDelegate];
    [self beginCurrentMercedesModuleScanCommand];
}

- (void)beginCurrentMercedesModuleScanCommand
{
    char command[MBLINK_ELM327_MAX_COMMAND];
    size_t written = 0U;
    MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_command(
        &_mercedesModuleScan, command, sizeof(command), &written);
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK || written == 0U) {
        _moduleScanActive = NO;
        self.mercedesProbeStatusText = [NSString stringWithFormat:@"Module scan command failed: %@",
  MBLinkStringFromCString(mblink_mercedes_module_scan_result_name(result))];
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    self.mercedesProbeStatusText = [NSString stringWithFormat:
        @"Mercedes module scan · %@ · %zu found",
        MBLinkStringFromCString(mblink_mercedes_module_scan_stage_name(_mercedesModuleScan.stage)),
        _mercedesModuleScan.module_count];
    [self notifyDelegate];
    (void)[self beginCommand:command timeout:mblink_mercedes_module_scan_timeout_ms(&_mercedesModuleScan)];
}

- (void)processMercedesModuleScanResponse:(const MblinkElm327Response *)response
{
    MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_accept(&_mercedesModuleScan, response);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE) {
        _moduleScanActive = NO;
        [self updateMercedesModuleScanSummary];
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK ||
        _mercedesModuleScan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED) {
        _moduleScanActive = NO;
        self.mercedesProbeStatusText = [NSString stringWithFormat:@"Module scan incomplete: %@",
  MBLinkStringFromCString(mblink_mercedes_module_scan_result_name(result))];
        self.mercedesUDSFaultStatusText = @"Module fault inventory incomplete";
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    [self beginCurrentMercedesModuleScanCommand];
}

- (void)updateMercedesModuleScanSummary
{
    NSMutableArray<NSString *> *identity = self.mercedesIdentityResults != nil
        ? [self.mercedesIdentityResults mutableCopy] : [[NSMutableArray alloc] init];
    NSMutableArray<NSString *> *faults = [[NSMutableArray alloc] init];
    const size_t count = mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    const size_t totalFaults = mblink_mercedes_module_scan_total_dtc_count(&_mercedesModuleScan);
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module = mblink_mercedes_module_scan_module_at(&_mercedesModuleScan, index);
        if (module == NULL) continue;
        NSString *name = MBLinkStringFromCString(mblink_mercedes_module_scan_module_name(module));
        NSString *address = module->extended_id
  ? [NSString stringWithFormat:@"0x%08X → 0x%08X", (unsigned int)module->tx_can_id, (unsigned int)module->rx_can_id]
  : [NSString stringWithFormat:@"0x%03X → 0x%03X", (unsigned int)module->tx_can_id, (unsigned int)module->rx_can_id];
        [identity addObject:[NSString stringWithFormat:@"MODULE · %@ · %@ · %zu fault record%@",
  name, address, module->dtcs.count, module->dtcs.count == 1U ? @"" : @"s"]];
        for (size_t dtcIndex = 0U; dtcIndex < module->dtcs.count; ++dtcIndex) {
  char code[7];
  if (!mblink_uds_dtc_format_hex(module->dtcs.records[dtcIndex].code, code, sizeof(code))) continue;
  [faults addObject:[NSString stringWithFormat:@"%@ · %@ · %@ · status 0x%02X",
      name, address, MBLinkStringFromCString(code),
      (unsigned int)module->dtcs.records[dtcIndex].status]];
        }
    }
    self.mercedesIdentityResults = [identity copy];
    self.mercedesIdentitySummaryText = [NSString stringWithFormat:@"%@ · %zu responding module%@",
        self.mercedesIdentitySummaryText, count, count == 1U ? @"" : @"s"];
    self.mercedesUDSFaults = [faults copy];
    self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
        @"Complete · %zu modules · %zu Mercedes factory fault record%@%@",
        count, totalFaults, totalFaults == 1U ? @"" : @"s",
        _mercedesModuleScan.truncated ? @" · module list truncated" : @""];
    self.mercedesProbeStatusText = [NSString stringWithFormat:
        @"Mercedes full module discovery complete · %zu responding module%@ · per-module fault memory read",
        count, count == 1U ? @"" : @"s"];
    [self notifyDelegate];
}

- (void)finishMercedesExtensionRestoringAdapter:(BOOL)restore
{
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    _flow.config.restore_adapter_after_manufacturer_extension = restore;
    LinkDiagnosticFlowResult result = link_diagnostic_flow_resume_after_manufacturer(&_flow);
    if (result != LINK_DIAGNOSTIC_FLOW_RESULT_OK) {
        [self markFlowFailure:@"Could not resume shared diagnostic flow after Mercedes probe"];
        return;
    }
    if (restore) [self setStatus:@"Restoring standard OBD-II adapter channel"];
    [self driveDiagnosticFlow];
}

- (void)updateMercedesProbeEvidenceSummary
{
    const unsigned int positive = MBLinkBitCount32(_mercedesProbe.identity_positive_mask);
    const unsigned int negative = MBLinkBitCount32(_mercedesProbe.identity_negative_mask);
    const unsigned int noResponse = MBLinkBitCount32(_mercedesProbe.identity_no_response_mask);
    const unsigned int invalid = MBLinkBitCount32(_mercedesProbe.identity_invalid_mask);
    const size_t total = mblink_mercedes_ecu_probe_identity_did_count();
    self.mercedesIdentitySummaryText = [NSString stringWithFormat:
        @"%u/%zu positive · %u negative · %u no response · %u invalid",
        positive, total, negative, noResponse, invalid];

    NSMutableArray<NSString *> *identityResults = [[NSMutableArray alloc] initWithCapacity:total];
    for (size_t index = 0U; index < total; ++index) {
        const uint32_t bit = (uint32_t)1U << index;
        const uint16_t did = mblink_mercedes_ecu_probe_identity_did_at(index);
        NSString *name = MBLinkStringFromCString(mblink_mercedes_ecu_probe_identity_did_name(index));
        NSString *state = @"not classified";
        if ((_mercedesProbe.identity_positive_mask & bit) != 0U) state = @"response captured";
        else if ((_mercedesProbe.identity_negative_mask & bit) != 0U) state = @"negative response";
        else if ((_mercedesProbe.identity_no_response_mask & bit) != 0U) state = @"no response";
        else if ((_mercedesProbe.identity_invalid_mask & bit) != 0U) state = @"invalid response";
        [identityResults addObject:[NSString stringWithFormat:@"%04X · %@ · %@", (unsigned int)did, name, state]];
    }
    self.mercedesIdentityResults = [identityResults copy];

    NSString *vinSummary = nil;
    if (_mercedesProbe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE && _mercedesProbe.vin[0] != '\0') {
        self.mercedesVINText = MBLinkStringFromCString(_mercedesProbe.vin);
        mblink_telemetry_session_metadata_set_vehicle(&_sessionMetadata, _mercedesProbe.vin);
        vinSummary = [NSString stringWithFormat:@"VIN %@", self.mercedesVINText];
    } else {
        self.mercedesVINText = nil;
        switch (_mercedesProbe.vin_result) {
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE: vinSummary = @"standard VIN not returned"; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE:
            vinSummary = [NSString stringWithFormat:@"standard VIN negative response NRC 0x%02X",
                (unsigned int)_mercedesProbe.vin_negative_response_code]; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE:
            vinSummary = @"standard VIN response was not a valid 17-character VIN"; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED: vinSummary = @"standard VIN was not attempted"; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE: vinSummary = @"standard VIN response was empty"; break;
        }
    }

    if (_mercedesProbe.crd3_session_variant_available && _mercedesProbe.crd3_supplier_available) {
        NSString *supplier = MBLinkStringFromCString(_mercedesProbe.crd3_supplier.supplier_name);
        if (mblink_mercedes_ecu_probe_matches_om651_cdid3_delphi_signature(&_mercedesProbe)) {
            self.mercedesCrd3SummaryText = [NSString stringWithFormat:
                @"OM651/CDID3 signature matched · %@ · diagnostic version %02X %02X %02X",
                supplier,
                (unsigned int)_mercedesProbe.crd3_session_variant.gateway_mode,
                (unsigned int)(_mercedesProbe.crd3_session_variant.variant >> 8U),
                (unsigned int)(_mercedesProbe.crd3_session_variant.variant & 0xffU)];
        } else {
            self.mercedesCrd3SummaryText = [NSString stringWithFormat:
                @"CRD3 fingerprint · %@ · gateway 0x%02X · variant 0x%04X · session 0x%02X",
                supplier,
                (unsigned int)_mercedesProbe.crd3_session_variant.gateway_mode,
                (unsigned int)_mercedesProbe.crd3_session_variant.variant,
                (unsigned int)_mercedesProbe.crd3_session_variant.session];
        }
    } else if (_mercedesProbe.crd3_session_variant_available) {
        self.mercedesCrd3SummaryText = [NSString stringWithFormat:
            @"CRD3 variant 0x%04X captured; supplier unavailable",
            (unsigned int)_mercedesProbe.crd3_session_variant.variant];
    } else if (_mercedesProbe.crd3_supplier_available) {
        self.mercedesCrd3SummaryText = [NSString stringWithFormat:
            @"CRD3 supplier %@ captured; variant unavailable",
            MBLinkStringFromCString(_mercedesProbe.crd3_supplier.supplier_name)];
    } else {
        self.mercedesCrd3SummaryText = @"No decodable F100/F154 CRD3 identity returned";
    }

    if (_mercedesProbe.crd3_hardware_profile != NULL) {
        NSString *family = MBLinkStringFromCString(
            _mercedesProbe.crd3_hardware_profile->ecu_family);
        NSString *mcu = MBLinkStringFromCString(
            _mercedesProbe.crd3_hardware_profile->microcontroller);
        NSString *match = MBLinkStringFromCString(
            mblink_mercedes_crd3_profile_match_name(
                _mercedesProbe.crd3_hardware_match));
        self.mercedesCrd3SummaryText = [NSString stringWithFormat:
            @"%@ · source profile %@ · %@ · %@",
            self.mercedesCrd3SummaryText, family, mcu, match];
    }

    self.mercedesUDSFaults = MBLinkMercedesUDSDTCStrings(&_mercedesProbe.dtcs);
    switch (_mercedesProbe.dtc_result) {
    case MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE:
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"Complete · %lu Mercedes UDS fault record%@ · availability 0x%02X%@",
            (unsigned long)self.mercedesUDSFaults.count,
            self.mercedesUDSFaults.count == 1U ? @"" : @"s",
            (unsigned int)_mercedesProbe.dtcs.availability_mask,
            _mercedesProbe.dtcs.truncated ? @" · truncated" : @""];
        break;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NO_RESPONSE:
        self.mercedesUDSFaultStatusText = @"No response to Mercedes UDS fault read"; break;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NEGATIVE_RESPONSE:
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"Mercedes UDS fault read negative response · NRC 0x%02X",
            (unsigned int)_mercedesProbe.dtc_negative_response_code]; break;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_INVALID_RESPONSE:
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"Mercedes UDS fault response invalid · %@",
            MBLinkStringFromCString(mblink_uds_result_name(_mercedesProbe.dtc_uds_result))]; break;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NOT_ATTEMPTED:
        self.mercedesUDSFaultStatusText = @"Mercedes UDS fault read not attempted"; break;
    }

    self.mercedesProbeStatusText = [NSString stringWithFormat:
        @"Positive UDS endpoint response captured; %@; %@; Mercedes UDS fault read %@; endpoint remains a candidate pending fixture verification",
        vinSummary, self.mercedesCrd3SummaryText,
        MBLinkStringFromCString(mblink_mercedes_ecu_probe_dtc_result_name(_mercedesProbe.dtc_result))];
}

- (NSString *)mercedesProbeFailureText
{
    if (_mercedesProbe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR) {
        if (_mercedesProbe.uds_negative_response_code != 0U) {
            return [NSString stringWithFormat:
                @"UDS endpoint replied with negative response NRC 0x%02X; candidate not promoted",
                (unsigned int)_mercedesProbe.uds_negative_response_code];
        }
        return [NSString stringWithFormat:@"UDS response validation failed: %@",
            MBLinkStringFromCString(mblink_uds_result_name(_mercedesProbe.uds_failure))];
    }
    if (_mercedesProbe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR) {
        return [NSString stringWithFormat:@"No valid UDS PDU from candidate: %@ (%@)",
            MBLinkStringFromCString(mblink_elm327_can_result_name(_mercedesProbe.elm_can_failure)),
            MBLinkStringFromCString(mblink_elm327_result_name(_mercedesProbe.elm_failure))];
    }
    if (_mercedesProbe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR) {
        return [NSString stringWithFormat:@"Could not configure candidate CAN channel: %@ (%@)",
            MBLinkStringFromCString(mblink_elm327_can_result_name(_mercedesProbe.elm_can_failure)),
            MBLinkStringFromCString(mblink_elm327_result_name(_mercedesProbe.elm_failure))];
    }
    return [NSString stringWithFormat:@"Mercedes probe failed: %@",
        MBLinkStringFromCString(mblink_mercedes_ecu_probe_result_name(_mercedesProbe.failure))];
}

- (NSUInteger)recordedSampleCount
{
    uint64_t total = mblink_telemetry_store_total_sample_count(&_telemetry);
    return total > (uint64_t)NSUIntegerMax ? NSUIntegerMax : (NSUInteger)total;
}

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid limit:(NSUInteger)limit
{
    if (limit == 0U) return @[];
    NSMutableArray<NSNumber *> *values = [[NSMutableArray alloc] initWithCapacity:limit];
    const size_t count = mblink_telemetry_store_history_count(&_telemetry);
    for (size_t reverseIndex = count; reverseIndex > 0U && values.count < limit; --reverseIndex) {
        MblinkTelemetrySample sample;
        if (!mblink_telemetry_store_history_at(&_telemetry, reverseIndex - 1U, &sample) ||
            sample.measurement.pid != pid) continue;
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
    if (_sessionCSV.length == 0U) return nil;
    return [[NSString alloc] initWithData:[_sessionCSV copy] encoding:NSUTF8StringEncoding];
}

@end
