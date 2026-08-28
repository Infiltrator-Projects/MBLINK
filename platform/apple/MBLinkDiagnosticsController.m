// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "../../src/link/platform/apple/LinkDiagnosticsController.h"
#import "mblink/elm327.h"
#import "mblink/mercedes.h"
#import "mblink/mercedes_probe.h"
#import "mblink/mercedes_module_scan.h"
#import "mblink/uds_dtc.h"

#include <stdio.h>
#include <string.h>

static NSString * const MBLinkVehicleProfilesDefaultsKey =
    @"mblink.vehicleProfiles.v1";
static const NSInteger MBLinkVehicleProfileSchemaVersion = 1;

@interface MBLinkDiagnosticsController () <LinkDiagnosticsControllerDelegate>
@property(nonatomic, copy, readwrite) NSString *mercedesProbeStatusText;
@property(nonatomic, copy, readwrite, nullable) NSString *mercedesProbeEndpointText;
@property(nonatomic, copy, readwrite, nullable) NSString *mercedesVINText;
@property(nonatomic, copy, readwrite) NSString *mercedesIdentitySummaryText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesIdentityResults;
@property(nonatomic, copy, readwrite) NSString *mercedesCrd3SummaryText;
@property(nonatomic, copy, readwrite) NSString *mercedesUDSFaultStatusText;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesUDSFaults;
@property(nonatomic, copy, readwrite) NSString *vehicleProfileStatusText;

- (void)resetMercedesState;
- (void)notifyDelegate;
- (void)setStatus:(NSString *)status;
- (void)markFlowFailure:(NSString *)status;
- (void)beginMercedesProbe;
- (void)beginCurrentMercedesProbeCommand;
- (void)processMercedesProbeResponse:(const MblinkElm327Response *)response;
- (void)beginMercedesModuleScan;
- (void)beginCurrentMercedesModuleScanCommand;
- (void)processMercedesModuleScanResponse:(const MblinkElm327Response *)response;
- (void)updateMercedesModuleFaultEvidenceInProgress;
- (void)updateMercedesModuleScanSummary;
- (nullable NSDictionary *)savedVehicleProfileForVIN:(NSString *)vin;
- (void)loadSavedVehicleProfileForVIN:(NSString *)vin;
- (void)saveCurrentVehicleProfile;
- (void)removeSavedVehicleProfileForVIN:(NSString *)vin;
- (BOOL)beginCachedVehicleProfileRefresh;
- (void)finishMercedesExtensionRestoringAdapter:(BOOL)restore;
- (void)updateMercedesProbeEvidenceSummary;
- (NSString *)mercedesProbeFailureText;
@end

@implementation MBLinkDiagnosticsController {
    LinkDiagnosticsController *_shared;
    MblinkMercedesEcuProbe _mercedesProbe;
    BOOL _manufacturerProbeActive;
    MblinkMercedesModuleScan _mercedesModuleScan;
    BOOL _moduleScanActive;
    BOOL _cachedModuleRefreshActive;
    NSDictionary *_Nullable _cachedVehicleProfile;
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

static NSString *MBLinkStringFromCString(const char *value)
{
    if (value == NULL) return @"unknown";
    NSString *string = [NSString stringWithUTF8String:value];
    return string != nil ? string : @"unknown";
}

static NSString *MBLinkMercedesEndpointText(
    const MblinkMercedesEcuEndpointDefinition *endpoint)
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

static void MBLinkCopyProfileString(
    id value,
    char *destination,
    size_t destinationCapacity,
    bool *available)
{
    if (available != NULL) *available = false;
    if (destination == NULL || destinationCapacity == 0U) return;
    destination[0] = '\0';
    if (![value isKindOfClass:[NSString class]]) return;
    NSString *text = (NSString *)value;
    if (text.length == 0U) return;
    const char *utf8 = text.UTF8String;
    if (utf8 == NULL) return;
    (void)snprintf(destination, destinationCapacity, "%s", utf8);
    if (available != NULL) *available = destination[0] != '\0';
}

static BOOL MBLinkPopulateModuleEntryFromProfile(
    NSDictionary *dictionary,
    MblinkMercedesModuleScanEntry *entry)
{
    if (![dictionary isKindOfClass:[NSDictionary class]] || entry == NULL)
        return NO;

    NSNumber *tx = dictionary[@"tx"];
    NSNumber *rx = dictionary[@"rx"];
    NSNumber *extended = dictionary[@"extended"];
    if (![tx isKindOfClass:[NSNumber class]] ||
        ![rx isKindOfClass:[NSNumber class]] ||
        ![extended isKindOfClass:[NSNumber class]]) {
        return NO;
    }

    memset(entry, 0, sizeof(*entry));
    entry->tx_can_id = tx.unsignedIntValue;
    entry->rx_can_id = rx.unsignedIntValue;
    entry->extended_id = extended.boolValue;

    NSNumber *kind = dictionary[@"kind"];
    entry->kind = [kind isKindOfClass:[NSNumber class]]
        ? (MblinkMercedesModuleKind)kind.unsignedIntegerValue
        : mblink_mercedes_module_scan_kind(
            entry->tx_can_id, entry->extended_id);
    entry->identification_status = MBLINK_MERCEDES_DEFINITION_CANDIDATE;
    entry->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED;

    MBLinkCopyProfileString(
        dictionary[@"identity"], entry->identity,
        sizeof(entry->identity), &entry->identity_available);
    MBLinkCopyProfileString(
        dictionary[@"sparePart"], entry->spare_part_number,
        sizeof(entry->spare_part_number),
        &entry->spare_part_number_available);
    MBLinkCopyProfileString(
        dictionary[@"software"], entry->software_number,
        sizeof(entry->software_number),
        &entry->software_number_available);
    MBLinkCopyProfileString(
        dictionary[@"hardware"], entry->hardware_number,
        sizeof(entry->hardware_number),
        &entry->hardware_number_available);
    if (entry->identity_available)
        mblink_mercedes_module_scan_classify_identity(entry);

    const uint32_t maxID = entry->extended_id
        ? UINT32_C(0x1fffffff) : UINT32_C(0x7ff);
    return entry->tx_can_id <= maxID && entry->rx_can_id <= maxID;
}

static NSArray<NSString *> *MBLinkMercedesUDSDTCStrings(
    const MblinkUdsDtcList *list)
{
    if (list == NULL || list->count == 0U) return @[];
    NSMutableArray<NSString *> *values =
        [[NSMutableArray alloc] initWithCapacity:list->count];
    for (size_t index = 0U; index < list->count; ++index) {
        char code[7];
        if (!mblink_uds_dtc_format_hex(
                list->records[index].code, code, sizeof(code))) {
            continue;
        }
        [values addObject:[NSString stringWithFormat:
            @"%@ · status 0x%02X",
            MBLinkStringFromCString(code),
            (unsigned int)list->records[index].status]];
    }
    return [values copy];
}

static bool MBLinkSimulatorResponder(
    void *context,
    const char *command,
    char *response,
    size_t responseSize)
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
    for (size_t index = 0U;
         index < sizeof(replies) / sizeof(replies[0]);
         ++index) {
        if (strcmp(command, replies[index].command) != 0) continue;
        const int written = snprintf(
            response, responseSize, "%s", replies[index].response);
        return written >= 0 && (size_t)written < responseSize;
    }
    return false;
}

- (instancetype)init
{
    self = [super init];
    if (self == nil) return nil;

    LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
    /*
     * Finish the standard OBD stored/pending/permanent DTC inventory first.
     * Mercedes module discovery can be lengthy and must not prevent ordinary
     * OBD evidence from reaching the UI.
     */
    flowConfig.manufacturer_extension_after_pid_discovery = false;
    flowConfig.manufacturer_extension_after_standard_dtcs = true;
    flowConfig.restore_adapter_after_manufacturer_extension = true;
    _shared = [[LinkDiagnosticsController alloc]
        initWithProductSlug:@"mblink"
        flowConfig:flowConfig
        liveStatusText:@"Live OBD-II and diesel scheduler active"
        simulatedLiveStatusText:
            @"Simulated ELM327 · live OBD-II and diesel data"
        standardVINStatusText:@"Reading standard vehicle VIN"];
    _shared.delegate = self;

    [self resetMercedesState];
    return self;
}

- (void)dealloc
{
    _shared.delegate = nil;
}

- (void)resetMercedesState
{
    self.mercedesProbeStatusText = @"Not attempted";
    self.mercedesProbeEndpointText = nil;
    self.mercedesVINText = nil;
    self.mercedesIdentitySummaryText = @"Not attempted";
    self.mercedesIdentityResults = @[];
    self.mercedesCrd3SummaryText = @"Not attempted";
    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes ECU probe";
    self.mercedesUDSFaults = @[];
    self.vehicleProfileStatusText = @"Waiting for VIN";
    _mercedesProbe = (MblinkMercedesEcuProbe){0};
    _mercedesModuleScan = (MblinkMercedesModuleScan){0};
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    _cachedModuleRefreshActive = NO;
    _cachedVehicleProfile = nil;
}

- (void)notifyDelegate
{
    id<MBLinkDiagnosticsControllerDelegate> delegate = self.delegate;
    if (delegate != nil)
        [delegate diagnosticsControllerDidUpdate:self];
}

- (void)setStatus:(NSString *)status
{
    [_shared updateStatusText:status];
}

- (void)markFlowFailure:(NSString *)status
{
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    [_shared failWithStatus:status];
}

- (NSString *)statusText { return _shared.statusText; }
- (nullable NSString *)peripheralName { return _shared.peripheralName; }
- (nullable NSString *)adapterIdentifier { return _shared.adapterIdentifier; }
- (NSString *)faultScanStatusText { return _shared.faultScanStatusText; }
- (NSArray<NSString *> *)storedDTCs { return _shared.storedDTCs; }
- (NSArray<NSString *> *)pendingDTCs { return _shared.pendingDTCs; }
- (NSArray<NSString *> *)permanentDTCs { return _shared.permanentDTCs; }
- (BOOL)isActive { return _shared.isActive; }
- (BOOL)isReady { return _shared.isReady; }
- (NSUInteger)recordedSampleCount { return _shared.recordedSampleCount; }

- (void)start
{
    [self resetMercedesState];
    [_shared start];
}

- (void)startSimulated
{
    [self resetMercedesState];
    [_shared startSimulatedWithAdapterIdentifier:"ELM327 v2.3 MBLINK SIM"
                                             vin:"WDD2073022F123456"
                                 customResponder:MBLinkSimulatorResponder
                                         context:NULL];
}

- (void)disconnect
{
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    [_shared disconnect];
}

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit
{
    return [_shared recentValuesForPID:pid limit:limit];
}

- (BOOL)favouriteForPID:(uint8_t)pid
{
    return [_shared favouriteForPID:pid];
}

- (void)setFavourite:(BOOL)favourite forPID:(uint8_t)pid
{
    [_shared setFavourite:favourite forPID:pid];
}

- (nullable NSString *)csvSnapshot
{
    return [_shared csvSnapshot];
}

- (void)linkDiagnosticsControllerDidUpdate:
    (LinkDiagnosticsController *)controller
{
    (void)controller;
    [self notifyDelegate];
}

- (void)linkDiagnosticsController:(LinkDiagnosticsController *)controller
              didReceiveFlowEvent:(const LinkDiagnosticFlowEvent *)event
{
    (void)controller;
    if (event == NULL ||
        event->kind != LINK_DIAGNOSTIC_FLOW_EVENT_STANDARD_VIN) {
        return;
    }

    if (!event->vin_available || event->vin == NULL) {
        self.mercedesVINText = nil;
        self.mercedesIdentitySummaryText =
            @"Standard OBD VIN not returned; Mercedes ECU identity pending";
        [self notifyDelegate];
        return;
    }

    self.mercedesVINText = MBLinkStringFromCString(event->vin);
    NSMutableArray<NSString *> *identity = [[NSMutableArray alloc] init];
    MblinkMercedesVinDecode decoded;
    if (mblink_mercedes_vin_decode(event->vin, &decoded)) {
        if (decoded.baumuster_definition != NULL) {
            [identity addObject:[NSString stringWithFormat:
                @"VIN · %@ · %@ · %@ · %@",
                MBLinkStringFromCString(decoded.baumuster),
                MBLinkStringFromCString(
                    decoded.baumuster_definition->chassis_family),
                MBLinkStringFromCString(
                    decoded.baumuster_definition->model),
                MBLinkStringFromCString(
                    decoded.baumuster_definition->engine_code)]];
            [identity addObject:[NSString stringWithFormat:
                @"ENGINE · %@ · %u cc · %@",
                MBLinkStringFromCString(
                    decoded.baumuster_definition->engine_code),
                decoded.baumuster_definition->displacement_cc,
                MBLinkStringFromCString(
                    mblink_mercedes_fuel_type_name(
                        decoded.baumuster_definition->fuel))]];
        } else if (decoded.baumuster_available) {
            [identity addObject:[NSString stringWithFormat:
                @"VIN · Baumuster %@ · series %@",
                MBLinkStringFromCString(decoded.baumuster),
                MBLinkStringFromCString(decoded.series_number)]];
        }
        if (decoded.plant_definition != NULL) {
            [identity addObject:[NSString stringWithFormat:
                @"BUILD · %@, %@ · %@ · serial %@",
                MBLinkStringFromCString(decoded.plant_definition->plant),
                MBLinkStringFromCString(decoded.plant_definition->country),
                MBLinkStringFromCString(
                    mblink_mercedes_steering_name(decoded.steering)),
                MBLinkStringFromCString(decoded.serial_number)]];
        }
    }

    self.mercedesIdentityResults = [identity copy];
    self.mercedesIdentitySummaryText = identity.count != 0U
        ? @"Standard OBD VIN captured and decoded; Mercedes ECU identity pending"
        : @"Standard OBD VIN captured; Mercedes ECU identity pending";
    [self loadSavedVehicleProfileForVIN:self.mercedesVINText];
    [self notifyDelegate];
}

- (void)linkDiagnosticsControllerBeginManufacturerExtension:
    (LinkDiagnosticsController *)controller
{
    (void)controller;
    if ([self beginCachedVehicleProfileRefresh]) return;
    [self beginMercedesProbe];
}

- (void)linkDiagnosticsController:(LinkDiagnosticsController *)controller
  didReceiveManufacturerResponse:(const LinkElm327Response *)response
{
    (void)controller;
    if (_moduleScanActive) {
        [self processMercedesModuleScanResponse:
            (const MblinkElm327Response *)response];
        return;
    }
    if (_manufacturerProbeActive) {
        [self processMercedesProbeResponse:
            (const MblinkElm327Response *)response];
        return;
    }
    [_shared failWithStatus:
        @"Mercedes manufacturer response arrived without an active probe"];
}

- (void)linkDiagnosticsController:(LinkDiagnosticsController *)controller
 manufacturerExtensionDidFailWithStatus:(NSString *)status
{
    (void)controller;
    if (_moduleScanActive) {
        const size_t capturedModules =
            mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
        const size_t capturedFaults =
            mblink_mercedes_module_scan_total_dtc_count(&_mercedesModuleScan);
        /*
         * A transport watchdog firing late in a census must not erase useful
         * evidence already returned by the car.  Preserve the partial module
         * map and any DTCs that were decoded before the interruption.
         */
        if (capturedModules != 0U)
            [self updateMercedesModuleScanSummary];
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Mercedes module scan interrupted: %@", status];
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"Partial · %zu responding modules · %zu Mercedes factory fault record%@ retained",
            capturedModules, capturedFaults, capturedFaults == 1U ? @"" : @"s"];
    } else if (_manufacturerProbeActive) {
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Mercedes ECU probe interrupted: %@", status];
        self.mercedesIdentitySummaryText = @"Probe did not complete";
        if (_mercedesProbe.stage ==
            MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION) {
            self.mercedesUDSFaultStatusText =
                @"Mercedes UDS fault read interrupted";
        }
    }
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    _cachedModuleRefreshActive = NO;
    [self notifyDelegate];
}

- (void)beginMercedesProbe
{
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        mblink_mercedes_generic_engine_endpoint();
    if (endpoint == NULL || !mblink_mercedes_ecu_endpoint_is_valid(endpoint)) {
        self.mercedesProbeStatusText = @"Generic Mercedes engine endpoint unavailable";
        self.mercedesIdentitySummaryText = @"Not attempted";
        self.mercedesCrd3SummaryText = @"Not attempted";
        self.mercedesUDSFaultStatusText = @"Not attempted";
        [self finishMercedesExtensionRestoringAdapter:NO];
        return;
    }

    self.mercedesProbeEndpointText = MBLinkMercedesEndpointText(endpoint);
    MblinkMercedesEcuProbeResult result =
        mblink_mercedes_ecu_probe_begin(&_mercedesProbe, endpoint);
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
    self.mercedesProbeStatusText = @"Identifying Mercedes engine ECU with read-only UDS";
    self.mercedesIdentitySummaryText = @"Waiting for VIN and standard ECU identity";
    self.mercedesCrd3SummaryText = @"Family-specific fingerprint pending identification";
    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes UDS fault read";
    [self setStatus:@"Probing Mercedes engine ECU"];
    [self notifyDelegate];
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
    (void)[_shared beginManufacturerCommand:command timeout:4000U];
}

- (void)processMercedesProbeResponse:(const MblinkElm327Response *)response
{
    MblinkMercedesEcuProbeResult result = mblink_mercedes_ecu_probe_accept(&_mercedesProbe, response);
    if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
        [self updateMercedesProbeEvidenceSummary];
        if ([self beginCachedVehicleProfileRefresh]) return;
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
    if (_shared.isSimulated) {
        memset(&_mercedesModuleScan, 0, sizeof(_mercedesModuleScan));
        _mercedesModuleScan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE;
        _mercedesModuleScan.module_count = 1U;
        MblinkMercedesModuleScanEntry *module = &_mercedesModuleScan.modules[0];
        module->tx_can_id = UINT32_C(0x7e0);
        module->rx_can_id = UINT32_C(0x7e8);
        module->kind = MBLINK_MERCEDES_MODULE_ENGINE;
        module->tester_present_response = true;
        (void)snprintf(
            module->identity, sizeof(module->identity), "%s", "CRD3-SIM");
        module->identity_available = true;
        mblink_mercedes_module_scan_classify_identity(module);
        module->dtcs = _mercedesProbe.dtcs;
        module->dtc_result = _mercedesProbe.dtc_result == MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE
  ? MBLINK_MERCEDES_MODULE_DTC_AVAILABLE : MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE;
        [self updateMercedesModuleScanSummary];
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin(&_mercedesModuleScan);
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
        self.mercedesProbeStatusText = @"Mercedes module discovery could not start";
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    _moduleScanActive = YES;
    /*
     * Initial iPhone connection deliberately uses only the eight standard
     * EOBD physical endpoints.  A 263-target gateway census must not hold the
     * connection hostage before VIN, faults and live OBD data reach the UI.
     * Deeper gateway/forensic discovery remains a separate workstation task.
     */
    self.mercedesProbeStatusText =
        @"Mercedes quick module scan · eight EOBD physical targets (read-only)";
    self.mercedesUDSFaultStatusText =
        @"Quick EOBD module discovery in progress";
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
    (void)[_shared beginManufacturerCommand:command timeout:mblink_mercedes_module_scan_timeout_ms(&_mercedesModuleScan)];
}

- (void)processMercedesModuleScanResponse:(const MblinkElm327Response *)response
{
    MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_accept(&_mercedesModuleScan, response);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE) {
        _moduleScanActive = NO;
        if (_cachedModuleRefreshActive) {
            const size_t expected =
                mblink_mercedes_module_scan_module_count(
                    &_mercedesModuleScan);
            const size_t fresh =
                mblink_mercedes_module_scan_fresh_response_count(
                    &_mercedesModuleScan);
            if (expected == 0U || fresh != expected) {
                NSString *vin = self.mercedesVINText;
                _cachedModuleRefreshActive = NO;
                _cachedVehicleProfile = nil;
                if (vin.length != 0U)
                    [self removeSavedVehicleProfileForVIN:vin];
                self.vehicleProfileStatusText =
                    @"Saved module map changed; rebuilding vehicle profile";
                self.mercedesProbeStatusText =
                    @"Saved module map did not validate; running one fresh discovery";
                _mercedesModuleScan = (MblinkMercedesModuleScan){0};
                [self notifyDelegate];
                [self beginMercedesProbe];
                return;
            }
        }
        [self updateMercedesModuleScanSummary];
        [self saveCurrentVehicleProfile];
        _cachedModuleRefreshActive = NO;
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK ||
        _mercedesModuleScan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED) {
        _moduleScanActive = NO;
        [self updateMercedesModuleFaultEvidenceInProgress];
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Module scan incomplete: %@",
            MBLinkStringFromCString(
                mblink_mercedes_module_scan_result_name(result))];
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    [self updateMercedesModuleFaultEvidenceInProgress];
    [self beginCurrentMercedesModuleScanCommand];
}

- (void)updateMercedesModuleFaultEvidenceInProgress
{
    NSMutableArray<NSString *> *faults = [[NSMutableArray alloc] init];
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    const size_t totalFaults =
        mblink_mercedes_module_scan_total_dtc_count(&_mercedesModuleScan);

    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (module == NULL) continue;
        NSString *name = MBLinkStringFromCString(
            mblink_mercedes_module_scan_module_name(module));
        NSString *address = module->extended_id
            ? [NSString stringWithFormat:@"0x%08X → 0x%08X",
                (unsigned int)module->tx_can_id,
                (unsigned int)module->rx_can_id]
            : [NSString stringWithFormat:@"0x%03X → 0x%03X",
                (unsigned int)module->tx_can_id,
                (unsigned int)module->rx_can_id];
        for (size_t dtcIndex = 0U;
             dtcIndex < module->dtcs.count; ++dtcIndex) {
            char code[7];
            if (!mblink_uds_dtc_format_hex(
                    module->dtcs.records[dtcIndex].code,
                    code, sizeof(code))) {
                continue;
            }
            [faults addObject:[NSString stringWithFormat:
                @"%@ · %@ · %@ · status 0x%02X",
                name, address, MBLinkStringFromCString(code),
                (unsigned int)module->dtcs.records[dtcIndex].status]];
        }
    }

    self.mercedesUDSFaults = [faults copy];
    self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
        @"Scanning · %zu responding modules · %zu Mercedes factory fault record%@ captured",
        count, totalFaults, totalFaults == 1U ? @"" : @"s"];
    [self notifyDelegate];
}

- (void)updateMercedesModuleScanSummary
{
    NSMutableArray<NSString *> *identity = [[NSMutableArray alloc] init];
    for (NSString *line in self.mercedesIdentityResults ?: @[]) {
        if ([line hasPrefix:@"MODULE ·"] ||
            [line hasPrefix:@"MODULE MAP ·"] ||
            [line hasPrefix:@"  SYSTEM ·"] ||
            [line hasPrefix:@"  PART ·"] ||
            [line hasPrefix:@"  SOFTWARE ·"] ||
            [line hasPrefix:@"  HARDWARE ·"]) {
            continue;
        }
        [identity addObject:line];
    }
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
        if (module->definition != NULL) {
            [identity addObject:[NSString stringWithFormat:
                @"MODULE · %@ · %@ · %@ · %zu fault record%@",
                name,
                MBLinkStringFromCString(
                    module->definition->component_designation),
                address, module->dtcs.count,
                module->dtcs.count == 1U ? @"" : @"s"]];
        } else {
            [identity addObject:[NSString stringWithFormat:
                @"MODULE · %@ · %@ · unresolved family · %zu fault record%@",
                name, address, module->dtcs.count,
                module->dtcs.count == 1U ? @"" : @"s"]];
        }
        if (module->identity_available)
            [identity addObject:[NSString stringWithFormat:
                @"  SYSTEM · %@", MBLinkStringFromCString(module->identity)]];
        if (module->spare_part_number_available)
            [identity addObject:[NSString stringWithFormat:
                @"  PART · %@", MBLinkStringFromCString(
                    module->spare_part_number)]];
        if (module->software_number_available)
            [identity addObject:[NSString stringWithFormat:
                @"  SOFTWARE · %@", MBLinkStringFromCString(
                    module->software_number)]];
        if (module->hardware_number_available)
            [identity addObject:[NSString stringWithFormat:
                @"  HARDWARE · %@", MBLinkStringFromCString(
                    module->hardware_number)]];
        for (size_t dtcIndex = 0U; dtcIndex < module->dtcs.count; ++dtcIndex) {
  char code[7];
  if (!mblink_uds_dtc_format_hex(module->dtcs.records[dtcIndex].code, code, sizeof(code))) continue;
  [faults addObject:[NSString stringWithFormat:@"%@ · %@ · %@ · status 0x%02X",
      name, address, MBLinkStringFromCString(code),
      (unsigned int)module->dtcs.records[dtcIndex].status]];
        }
    }
    {
        const size_t classified =
            mblink_mercedes_module_scan_classified_count(
                &_mercedesModuleScan);
        [identity addObject:[NSString stringWithFormat:
            @"MODULE MAP · %zu responding · %zu classified · %zu unresolved · %@",
            count, classified, count - classified,
            MBLinkStringFromCString(
                mblink_mercedes_module_scan_scope_name(
                    _mercedesModuleScan.scope))]];
        self.mercedesIdentityResults = [identity copy];
        self.mercedesIdentitySummaryText = [NSString stringWithFormat:
            @"%@ · %zu responding · %zu classified · %zu unresolved",
            self.mercedesIdentitySummaryText,
            count, classified, count - classified];
        self.mercedesUDSFaults = [faults copy];
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"Complete · %zu modules · %zu classified · %zu Mercedes factory fault record%@%@",
            count, classified, totalFaults,
            totalFaults == 1U ? @"" : @"s",
            _mercedesModuleScan.truncated
                ? @" · module list truncated" : @""];
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Mercedes %@ module scan complete · %zu responding · %zu classified · per-module fault memory read",
            MBLinkStringFromCString(
                mblink_mercedes_module_scan_scope_name(
                    _mercedesModuleScan.scope)),
            count, classified];
    }
    [self notifyDelegate];
}

- (nullable NSDictionary *)savedVehicleProfileForVIN:(NSString *)vin
{
    if (vin.length == 0U) return nil;

    NSDictionary *profiles = [[NSUserDefaults standardUserDefaults]
        dictionaryForKey:MBLinkVehicleProfilesDefaultsKey];
    id candidate = profiles[vin];
    NSDictionary *profile = [candidate isKindOfClass:[NSDictionary class]]
        ? (NSDictionary *)candidate : nil;
    NSNumber *schema = profile[@"schema"];
    NSArray *modules = [profile[@"modules"] isKindOfClass:[NSArray class]]
        ? profile[@"modules"] : nil;
    if (profile == nil || ![schema isKindOfClass:[NSNumber class]] ||
        schema.integerValue != MBLinkVehicleProfileSchemaVersion ||
        modules.count == 0U) {
        return nil;
    }
    return profile;
}

- (void)loadSavedVehicleProfileForVIN:(NSString *)vin
{
    _cachedVehicleProfile = [self savedVehicleProfileForVIN:vin];
    if (_cachedVehicleProfile == nil) {
        self.vehicleProfileStatusText =
            @"New VIN · module profile will be learned once";
        return;
    }

    NSArray *modules = _cachedVehicleProfile[@"modules"];
    NSString *endpoint = [_cachedVehicleProfile[@"probeEndpoint"]
        isKindOfClass:[NSString class]]
        ? _cachedVehicleProfile[@"probeEndpoint"] : nil;
    NSString *crd3 = [_cachedVehicleProfile[@"crd3Summary"]
        isKindOfClass:[NSString class]]
        ? _cachedVehicleProfile[@"crd3Summary"] : nil;
    if (endpoint.length != 0U)
        self.mercedesProbeEndpointText = endpoint;
    if (crd3.length != 0U)
        self.mercedesCrd3SummaryText = crd3;

    NSMutableArray<NSString *> *identity =
        [self.mercedesIdentityResults mutableCopy] ?: [[NSMutableArray alloc] init];
    size_t validModules = 0U;
    for (id value in modules) {
        if (![value isKindOfClass:[NSDictionary class]]) continue;
        MblinkMercedesModuleScanEntry module;
        if (!MBLinkPopulateModuleEntryFromProfile(
                (NSDictionary *)value, &module)) {
            continue;
        }
        ++validModules;
        NSString *name = MBLinkStringFromCString(
            mblink_mercedes_module_scan_module_name(&module));
        NSString *address = module.extended_id
            ? [NSString stringWithFormat:@"0x%08X → 0x%08X",
                (unsigned int)module.tx_can_id,
                (unsigned int)module.rx_can_id]
            : [NSString stringWithFormat:@"0x%03X → 0x%03X",
                (unsigned int)module.tx_can_id,
                (unsigned int)module.rx_can_id];
        [identity addObject:[NSString stringWithFormat:
            @"MODULE · %@ · %@ · saved VIN profile", name, address]];
        if (module.identity_available)
            [identity addObject:[NSString stringWithFormat:
                @"  SYSTEM · %@", MBLinkStringFromCString(module.identity)]];
        if (module.spare_part_number_available)
            [identity addObject:[NSString stringWithFormat:
                @"  PART · %@",
                MBLinkStringFromCString(module.spare_part_number)]];
        if (module.software_number_available)
            [identity addObject:[NSString stringWithFormat:
                @"  SOFTWARE · %@",
                MBLinkStringFromCString(module.software_number)]];
        if (module.hardware_number_available)
            [identity addObject:[NSString stringWithFormat:
                @"  HARDWARE · %@",
                MBLinkStringFromCString(module.hardware_number)]];
    }

    if (validModules == 0U) {
        [self removeSavedVehicleProfileForVIN:vin];
        _cachedVehicleProfile = nil;
        self.vehicleProfileStatusText =
            @"Saved VIN profile was invalid; rebuilding";
        return;
    }

    [identity addObject:[NSString stringWithFormat:
        @"MODULE MAP · %zu saved · VIN-keyed profile", validModules]];
    self.mercedesIdentityResults = [identity copy];
    self.mercedesIdentitySummaryText = [NSString stringWithFormat:
        @"Fresh VIN confirmed · %zu saved module route%@ loaded",
        validModules, validModules == 1U ? @"" : @"s"];
    self.vehicleProfileStatusText = [NSString stringWithFormat:
        @"Saved VIN profile loaded · %zu module%@ · validating",
        validModules, validModules == 1U ? @"" : @"s"];
}

- (void)saveCurrentVehicleProfile
{
    if (_shared.isSimulated || self.mercedesVINText.length == 0U)
        return;

    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    if (count == 0U) return;

    NSMutableArray<NSDictionary *> *modules =
        [[NSMutableArray alloc] initWithCapacity:count];
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (module == NULL) continue;

        NSMutableDictionary *dictionary = [@{
            @"tx": @(module->tx_can_id),
            @"rx": @(module->rx_can_id),
            @"extended": @(module->extended_id),
            @"kind": @((NSUInteger)module->kind)
        } mutableCopy];
        if (module->identity_available)
            dictionary[@"identity"] =
                MBLinkStringFromCString(module->identity);
        if (module->spare_part_number_available)
            dictionary[@"sparePart"] =
                MBLinkStringFromCString(module->spare_part_number);
        if (module->software_number_available)
            dictionary[@"software"] =
                MBLinkStringFromCString(module->software_number);
        if (module->hardware_number_available)
            dictionary[@"hardware"] =
                MBLinkStringFromCString(module->hardware_number);
        [modules addObject:[dictionary copy]];
    }
    if (modules.count == 0U) return;

    NSMutableDictionary *profile = [@{
        @"schema": @(MBLinkVehicleProfileSchemaVersion),
        @"vin": self.mercedesVINText,
        @"updatedAt": @([[NSDate date] timeIntervalSince1970]),
        @"modules": [modules copy]
    } mutableCopy];
    if (self.mercedesProbeEndpointText.length != 0U)
        profile[@"probeEndpoint"] = self.mercedesProbeEndpointText;
    if (self.mercedesCrd3SummaryText.length != 0U &&
        ![self.mercedesCrd3SummaryText isEqualToString:@"Not attempted"]) {
        profile[@"crd3Summary"] = self.mercedesCrd3SummaryText;
    }

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *profiles =
        [[defaults dictionaryForKey:MBLinkVehicleProfilesDefaultsKey]
            mutableCopy] ?: [[NSMutableDictionary alloc] init];
    profiles[self.mercedesVINText] = [profile copy];
    [defaults setObject:[profiles copy]
                 forKey:MBLinkVehicleProfilesDefaultsKey];

    _cachedVehicleProfile = [profile copy];
    self.vehicleProfileStatusText = [NSString stringWithFormat:
        @"VIN profile saved · %lu module%@ · future connections reuse it",
        (unsigned long)modules.count,
        modules.count == 1U ? @"" : @"s"];
}

- (void)removeSavedVehicleProfileForVIN:(NSString *)vin
{
    if (vin.length == 0U) return;

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *profiles =
        [[defaults dictionaryForKey:MBLinkVehicleProfilesDefaultsKey]
            mutableCopy];
    if (profiles == nil || profiles[vin] == nil) return;

    [profiles removeObjectForKey:vin];
    [defaults setObject:[profiles copy]
                 forKey:MBLinkVehicleProfilesDefaultsKey];
}

- (BOOL)beginCachedVehicleProfileRefresh
{
    if (_cachedVehicleProfile == nil || _shared.isSimulated)
        return NO;

    NSArray *modules = _cachedVehicleProfile[@"modules"];
    if (![modules isKindOfClass:[NSArray class]] || modules.count == 0U)
        return NO;

    MblinkMercedesModuleScanEntry cached[
        MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES];
    size_t count = 0U;
    for (id value in modules) {
        if (count >= MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES) break;
        if (![value isKindOfClass:[NSDictionary class]]) continue;
        if (MBLinkPopulateModuleEntryFromProfile(
                (NSDictionary *)value, &cached[count])) {
            ++count;
        }
    }

    if (count == 0U) {
        [self removeSavedVehicleProfileForVIN:self.mercedesVINText];
        _cachedVehicleProfile = nil;
        self.vehicleProfileStatusText =
            @"Saved VIN profile was invalid; rebuilding";
        return NO;
    }

    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_cached(
            &_mercedesModuleScan, cached, count);
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
        [self removeSavedVehicleProfileForVIN:self.mercedesVINText];
        _cachedVehicleProfile = nil;
        self.vehicleProfileStatusText =
            @"Saved VIN profile could not be loaded; rebuilding";
        return NO;
    }

    _cachedModuleRefreshActive = YES;
    _moduleScanActive = YES;
    self.vehicleProfileStatusText = [NSString stringWithFormat:
        @"Saved VIN profile · validating %zu known module route%@",
        count, count == 1U ? @"" : @"s"];
    self.mercedesProbeStatusText =
        @"Known vehicle profile loaded; refreshing module presence and faults";
    self.mercedesUDSFaultStatusText =
        @"Refreshing faults from saved module topology";
    [self setStatus:@"Known VIN; validating saved Mercedes module profile"];
    [self notifyDelegate];
    [self beginCurrentMercedesModuleScanCommand];
    return YES;
}

- (void)finishMercedesExtensionRestoringAdapter:(BOOL)restore
{
    _manufacturerProbeActive = NO;
    _moduleScanActive = NO;
    _cachedModuleRefreshActive = NO;
    if (![_shared completeManufacturerExtensionRestoringAdapter:restore]) {
        [_shared failWithStatus:
            @"Could not resume shared diagnostic flow after Mercedes probe"];
    }
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
    if (_mercedesProbe.vin_result ==
            MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE &&
        _mercedesProbe.vin[0] != '\0') {
        MblinkMercedesVinDecode decoded;
        if (mblink_mercedes_vin_decode(_mercedesProbe.vin, &decoded)) {
            if (decoded.baumuster_definition != NULL) {
                [identityResults addObject:[NSString stringWithFormat:
                    @"VIN · %@ · %@ · %@ · %@",
                    MBLinkStringFromCString(decoded.baumuster),
                    MBLinkStringFromCString(decoded.baumuster_definition->chassis_family),
                    MBLinkStringFromCString(decoded.baumuster_definition->model),
                    MBLinkStringFromCString(decoded.baumuster_definition->engine_code)]];
                [identityResults addObject:[NSString stringWithFormat:
                    @"ENGINE · %@ · %u cc · %@",
                    MBLinkStringFromCString(decoded.baumuster_definition->engine_code),
                    decoded.baumuster_definition->displacement_cc,
                    MBLinkStringFromCString(
                        mblink_mercedes_fuel_type_name(
                            decoded.baumuster_definition->fuel))]];
            } else if (decoded.baumuster_available) {
                [identityResults addObject:[NSString stringWithFormat:
                    @"VIN · Baumuster %@ · series %@ · catalogue entry unavailable",
                    MBLinkStringFromCString(decoded.baumuster),
                    MBLinkStringFromCString(decoded.series_number)]];
            }
            if (decoded.plant_definition != NULL) {
                [identityResults addObject:[NSString stringWithFormat:
                    @"BUILD · %@, %@ · %@ · serial %@",
                    MBLinkStringFromCString(decoded.plant_definition->plant),
                    MBLinkStringFromCString(decoded.plant_definition->country),
                    MBLinkStringFromCString(
                        mblink_mercedes_steering_name(decoded.steering)),
                    MBLinkStringFromCString(decoded.serial_number)]];
            }
        }
    }
    self.mercedesIdentityResults = [identityResults copy];

    NSString *vinSummary = nil;
    if (_mercedesProbe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE && _mercedesProbe.vin[0] != '\0') {
        self.mercedesVINText = MBLinkStringFromCString(_mercedesProbe.vin);
        [_shared setVehicleIdentifier:_mercedesProbe.vin];
        [self loadSavedVehicleProfileForVIN:self.mercedesVINText];
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
    } else if (!_mercedesProbe.crd3_fingerprint_attempted) {
        NSString *family = _mercedesProbe.identified_profile != NULL
            ? MBLinkStringFromCString(_mercedesProbe.identified_profile->engine_family)
            : @"unidentified";
        self.mercedesCrd3SummaryText = [NSString stringWithFormat:
            @"CRD3 fingerprint not selected · identified engine family %@",
            family];
    } else {
        self.mercedesCrd3SummaryText = @"CRD3 fingerprint attempted but no decodable F100/F154 identity returned";
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


@end
