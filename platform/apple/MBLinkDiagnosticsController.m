// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "../../src/link/platform/apple/LinkDiagnosticsController.h"
#import "mblink/elm327.h"
#import "mblink/mercedes.h"
#import "mblink/mercedes_probe.h"
#import "mblink/mercedes_module_scan.h"
#import "mblink/mercedes_data_scan.h"
#import "mblink/uds_dtc.h"

#include <stdio.h>
#include <string.h>

@interface MBLinkStandardDataSnapshot ()
@property(nonatomic, readwrite) uint8_t pid;
@property(nonatomic, readwrite) uint32_t responderCANIdentifier;
@property(nonatomic, readwrite, getter=isExtendedID) BOOL extendedID;
@property(nonatomic, readwrite) NSUInteger valueKind;
@property(nonatomic, readwrite) NSUInteger signalCount;
@property(nonatomic, copy, readwrite) NSString *formattedValue;
@property(nonatomic, copy, readwrite) NSString *rawHex;
@end
@implementation MBLinkStandardDataSnapshot
@end

@interface MBLinkMercedesDataSnapshot ()
@property(nonatomic, readwrite) uint16_t identifier;
@property(nonatomic, readwrite) uint8_t service;
@property(nonatomic, copy, readwrite) NSString *codeText;
@property(nonatomic, copy, readwrite, nullable) NSString *name;
@property(nonatomic, copy, readwrite, nullable) NSString *unit;
@property(nonatomic, copy, readwrite) NSString *formattedValue;
@property(nonatomic, copy, readwrite) NSString *rawHex;
@property(nonatomic, readwrite, getter=isMapped) BOOL mapped;
@property(nonatomic, readwrite, getter=isNumericValueAvailable)
    BOOL numericValueAvailable;
@property(nonatomic, readwrite) double numericValue;
@end

@implementation MBLinkMercedesDataSnapshot
@end

@interface MBLinkMercedesModuleSnapshot ()
@property(nonatomic, copy, readwrite) NSString *identifier;
@property(nonatomic, copy, readwrite) NSString *name;
@property(nonatomic, copy, readwrite) NSString *designation;
@property(nonatomic, copy, readwrite) NSString *network;
@property(nonatomic, copy, readwrite) NSString *kind;
@property(nonatomic, copy, readwrite) NSString *protocolName;
@property(nonatomic, readwrite) uint32_t requestCANIdentifier;
@property(nonatomic, readwrite) uint32_t responseCANIdentifier;
@property(nonatomic, readwrite, getter=isExtendedID) BOOL extendedID;
@property(nonatomic, copy, readwrite, nullable) NSString *identityText;
@property(nonatomic, copy, readwrite, nullable) NSString *partNumber;
@property(nonatomic, copy, readwrite, nullable) NSString *softwareNumber;
@property(nonatomic, copy, readwrite, nullable) NSString *hardwareNumber;
@property(nonatomic, copy, readwrite) NSString *faultStatus;
@property(nonatomic, readwrite) NSUInteger faultCount;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *faults;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *evidenceDetails;
@end

@implementation MBLinkMercedesModuleSnapshot
@end

static NSString * const MBLinkVehicleProfilesDefaultsKey =
    @"mblink.vehicleProfiles.v1";
static const NSInteger MBLinkVehicleProfileSchemaVersion = 5;
static const NSInteger MBLinkOldestReadableVehicleProfileSchemaVersion = 3;

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
@property(nonatomic, readwrite, getter=isManufacturerDataScanActive)
    BOOL manufacturerDataScanActive;
@property(nonatomic, copy, readwrite) NSString *manufacturerDataScanStatusText;
@property(nonatomic, copy, readwrite, nullable)
    NSString *manufacturerDataScanModuleIdentifier;

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
- (nullable const MblinkMercedesModuleScanEntry *)
    moduleEntryForIdentifier:(NSString *)identifier;
- (void)beginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                        forceFullScan:(BOOL)forceFullScan;
- (void)tryBeginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                             generation:(NSUInteger)generation
                                                attempt:(NSUInteger)attempt;
- (void)beginCurrentMercedesDataScanCommand;
- (void)processMercedesDataScanResponse:
    (const MblinkElm327Response *)response;
- (void)publishManufacturerDataScanResults;
- (void)finishManufacturerDataScanWithStatus:(NSString *)status;
- (nullable NSString *)automaticTransmissionTemperatureModuleIdentifier;
- (nullable NSDictionary *)savedVehicleProfileForVIN:(NSString *)vin;
- (void)loadSavedVehicleProfileForVIN:(NSString *)vin;
- (void)saveCurrentVehicleProfile;
- (void)removeSavedVehicleProfileForVIN:(NSString *)vin;
- (BOOL)beginCachedVehicleProfileRefresh;
- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event;
- (NSArray<NSNumber *> *)cachedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID;
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
    MblinkMercedesDataScan _manufacturerDataScan;
    NSMutableDictionary<NSString *, NSArray<MBLinkMercedesDataSnapshot *> *> *
        _manufacturerDataByModule;
    NSMutableDictionary<NSString *, MBLinkStandardDataSnapshot *> *
        _standardDataByResponder;
    NSMutableDictionary<NSNumber *, MBLinkStandardDataSnapshot *> *
        _standardDataLatest;
    NSUInteger _manufacturerDataRequestGeneration;
    BOOL _manufacturerDataForceFullScan;
    BOOL _automaticTransmissionTemperatureProbeAttempted;
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

static NSString *MBLinkStandardDataKey(uint8_t pid, uint32_t responder, BOOL extended)
{
    return [NSString stringWithFormat:@"%@:%08X:%02X",
        extended ? @"29" : @"11", (unsigned int)responder, (unsigned int)pid];
}

/*
 * Generic callers must never be "last responder wins". A Mode 01 PID can be
 * returned by several ECUs with legitimately different values. Keep the exact
 * responder-keyed snapshots as the source of truth and make the compatibility
 * "latest" view deterministic: prefer the legislated engine responder 0x7E8,
 * then 11-bit over 29-bit, then the lowest CAN identifier.
 */
static BOOL MBLinkStandardSnapshotPreferred(
    MBLinkStandardDataSnapshot *candidate,
    MBLinkStandardDataSnapshot *current)
{
    if (candidate == nil) return NO;
    if (current == nil) return YES;
    if (candidate.responderCANIdentifier == current.responderCANIdentifier &&
        candidate.isExtendedID == current.isExtendedID) {
        return YES;
    }

    const BOOL candidateEngine =
        !candidate.isExtendedID &&
        candidate.responderCANIdentifier == UINT32_C(0x7e8);
    const BOOL currentEngine =
        !current.isExtendedID &&
        current.responderCANIdentifier == UINT32_C(0x7e8);
    if (candidateEngine != currentEngine) return candidateEngine;

    if (candidate.isExtendedID != current.isExtendedID)
        return !candidate.isExtendedID;

    return candidate.responderCANIdentifier < current.responderCANIdentifier;
}

static NSString *MBLinkDecodedRawHex(const LinkObd2DecodedPid *decoded)
{
    if (decoded == NULL || decoded->raw_length == 0U) return @"";
    NSMutableString *text = [[NSMutableString alloc] initWithCapacity:decoded->raw_length * 3U];
    for (size_t i = 0U; i < decoded->raw_length; ++i) {
        if (i != 0U) [text appendString:@" "];
        [text appendFormat:@"%02X", (unsigned int)decoded->raw[i]];
    }
    return [text copy];
}

static NSString *MBLinkDecodedDisplay(const LinkObd2DecodedPid *decoded)
{
    if (decoded == NULL) return @"";
    if (decoded->text_available && decoded->text[0] != '\0')
        return MBLinkStringFromCString(decoded->text);
    if (decoded->signal_count != 0U) {
        NSMutableArray<NSString *> *parts = [[NSMutableArray alloc] init];
        for (size_t i = 0U; i < decoded->signal_count; ++i) {
            const LinkObd2DecodedSignal *signal = &decoded->signals[i];
            NSString *label = signal->label != NULL && signal->label[0] != '\0'
                ? MBLinkStringFromCString(signal->label) : @"";
            NSString *unit = signal->unit != NULL && signal->unit[0] != '\0'
                ? MBLinkStringFromCString(signal->unit) : @"";
            NSString *value = unit.length != 0U
                ? [NSString stringWithFormat:@"%.6g %@", signal->value, unit]
                : [NSString stringWithFormat:@"%.6g", signal->value];
            [parts addObject:label.length != 0U
                ? [NSString stringWithFormat:@"%@ %@", label, value] : value];
        }
        return [parts componentsJoinedByString:@" · "];
    }
    NSString *raw = MBLinkDecodedRawHex(decoded);
    return raw.length != 0U ? [@"RAW " stringByAppendingString:raw] : @"RAW";
}

static NSString *MBLinkMercedesModuleIdentifier(
    const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL) return @"";
    return [NSString stringWithFormat:@"%@:%08X:%08X",
        module->extended_id ? @"29" : @"11",
        (unsigned int)module->tx_can_id,
        (unsigned int)module->rx_can_id];
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
    NSNumber *protocol = dictionary[@"protocol"];
    if (![tx isKindOfClass:[NSNumber class]] ||
        ![rx isKindOfClass:[NSNumber class]] ||
        ![extended isKindOfClass:[NSNumber class]] ||
        ![protocol isKindOfClass:[NSNumber class]] ||
        protocol.unsignedIntegerValue >
            (NSUInteger)MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        return NO;
    }

    memset(entry, 0, sizeof(*entry));
    entry->tx_can_id = tx.unsignedIntValue;
    entry->rx_can_id = rx.unsignedIntValue;
    entry->extended_id = extended.boolValue;
    entry->protocol =
        (MblinkMercedesDiagnosticProtocol)protocol.unsignedIntegerValue;

    NSNumber *kind = dictionary[@"kind"];
    entry->kind = [kind isKindOfClass:[NSNumber class]]
        ? (MblinkMercedesModuleKind)kind.unsignedIntegerValue
        : mblink_mercedes_module_scan_kind(
            entry->tx_can_id, entry->extended_id);
    /*
     * Exact family identity remains evidence-gated, but Mercedes CAN
     * definitions explicitly name 0x7E1/0x7E9 as GS gearbox-control
     * diagnostic request/response. Route classification may therefore retain
     * transmission kind even when the ECU declines the UDS identity DIDs.
     */
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
    if (!entry->extended_id &&
        entry->tx_can_id == UINT32_C(0x7e1) &&
        entry->rx_can_id == UINT32_C(0x7e9)) {
        entry->kind = MBLINK_MERCEDES_MODULE_TRANSMISSION;
        entry->definition = NULL;
        entry->identification_status =
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
    }
    if (entry->identity_available)
        mblink_mercedes_module_scan_classify_identity(entry);
    if (!entry->extended_id) {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_known_route_for_tx(entry->tx_can_id);
        if (route != NULL && route->rx_can_id == entry->rx_can_id) {
            const MblinkMercedesModuleDefinition *definition =
                mblink_mercedes_c207_module_definition_for_key(
                    route->module_key);
            entry->protocol = route->protocol;
            if (definition != NULL) {
                entry->definition = definition;
                entry->kind = definition->kind;
                entry->identification_status = definition->status;
            }
        }
    }

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

static void MBLinkAppendMercedesModuleFaultStrings(
    NSMutableArray<NSString *> *faults,
    const MblinkMercedesModuleScanEntry *module,
    NSString *name,
    NSString *address)
{
    if (faults == nil || module == NULL || name == nil || address == nil)
        return;

    if (mblink_mercedes_module_scan_entry_protocol(module) ==
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        for (size_t index = 0U; index < module->kwp_dtcs.count; ++index) {
            const MblinkKwp2000Dtc *record =
                &module->kwp_dtcs.entries[index];
            const char *module_key = module->definition != NULL
                ? module->definition->key : NULL;
            const MblinkMercedesKwpDtcDefinition *definition =
                mblink_mercedes_kwp_dtc_find(module_key, record->code);
            if (definition != NULL) {
                [faults addObject:[NSString stringWithFormat:
                    @"%@ · %@ · %04X — %@ · %@ · KWP2000 status 0x%02X · %@ · applies: %@ · source: %@",
                    name, address, (unsigned int)record->code,
                    MBLinkStringFromCString(definition->description),
                    MBLinkStringFromCString(definition->subsystem),
                    (unsigned int)record->status,
                    MBLinkStringFromCString(
                        mblink_mercedes_definition_status_name(
                            definition->status)),
                    MBLinkStringFromCString(definition->applicability),
                    MBLinkStringFromCString(definition->provenance)]];
            } else {
                char formatted[MBLINK_MERCEDES_DTC_TEXT_LENGTH];
                if (!mblink_mercedes_kwp_dtc_format(
                        module_key, record->code, record->status,
                        formatted, sizeof(formatted))) {
                    continue;
                }
                [faults addObject:[NSString stringWithFormat:
                    @"%@ · %@ · %@", name, address,
                    MBLinkStringFromCString(formatted)]];
            }
        }
        return;
    }

    for (size_t index = 0U; index < module->dtcs.count; ++index) {
        char formatted[MBLINK_MERCEDES_DTC_TEXT_LENGTH];
        const char *module_key = module->definition != NULL
            ? module->definition->key : NULL;
        if (!mblink_mercedes_uds_dtc_format(
                module_key, module->dtcs.records[index].code,
                module->dtcs.records[index].status,
                formatted, sizeof(formatted))) {
            continue;
        }
        [faults addObject:[NSString stringWithFormat:
            @"%@ · %@ · %@", name, address,
            MBLinkStringFromCString(formatted)]];
    }
}

static NSString *MBLinkMercedesModuleFaultStatus(
    const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL) return @"Not attempted";
    switch (module->dtc_result) {
    case MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED:
        return @"Not attempted";
    case MBLINK_MERCEDES_MODULE_DTC_AVAILABLE:
        return mblink_mercedes_module_scan_entry_dtc_count(module) == 0U
            ? @"Checked · no faults" : @"Fault records captured";
    case MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE:
        return @"No response to fault read";
    case MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE:
        return [NSString stringWithFormat:@"Fault read rejected · NRC 0x%02X",
            (unsigned int)module->dtc_negative_response_code];
    case MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE:
        return @"Incomplete or invalid fault response";
    }
    return @"Unknown fault state";
}

static NSString *MBLinkProbeASCIIValue(
    const MblinkMercedesEcuProbe *probe,
    size_t requestIndex)
{
    if (probe == NULL) return nil;
    const LinkEcuProbeDidResult *result =
        link_ecu_probe_did_result_at(&probe->shared, requestIndex);
    if (result == NULL || result->status != LINK_ECU_PROBE_READ_AVAILABLE ||
        result->data_length == 0U) {
        return nil;
    }
    size_t length = result->data_length;
    while (length > 0U &&
           (result->data[length - 1U] == UINT8_C(0xff) ||
            result->data[length - 1U] == UINT8_C(0x00))) {
        --length;
    }
    if (length == 0U) return nil;
    for (size_t index = 0U; index < length; ++index) {
        if (result->data[index] < UINT8_C(0x20) ||
            result->data[index] > UINT8_C(0x7e)) {
            return nil;
        }
    }
    return [[NSString alloc]
        initWithBytes:result->data
               length:length
             encoding:NSASCIIStringEncoding];
}

static NSString *MBLinkProbeHexValue(
    const MblinkMercedesEcuProbe *probe,
    size_t requestIndex)
{
    if (probe == NULL) return nil;
    const LinkEcuProbeDidResult *result =
        link_ecu_probe_did_result_at(&probe->shared, requestIndex);
    if (result == NULL || result->status != LINK_ECU_PROBE_READ_AVAILABLE ||
        result->data_length == 0U) {
        return nil;
    }
    NSMutableString *value = [[NSMutableString alloc] init];
    for (size_t index = 0U; index < result->data_length; ++index) {
        if (index != 0U) [value appendString:@" "];
        [value appendFormat:@"%02X", (unsigned int)result->data[index]];
    }
    return [value copy];
}

static NSArray<NSString *> *MBLinkEngineProbeEvidence(
    const MblinkMercedesEcuProbe *probe)
{
    if (probe == NULL) return @[];
    NSMutableArray<NSString *> *values = [[NSMutableArray alloc] init];
    NSString *serial = MBLinkProbeASCIIValue(probe, 1U);
    NSString *erotan = MBLinkProbeASCIIValue(
        probe, MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 3U);
    NSString *f100 = MBLinkProbeHexValue(
        probe, MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 1U);
    NSString *f154 = MBLinkProbeHexValue(
        probe, MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 2U);
    if (serial.length != 0U)
        [values addObject:[NSString stringWithFormat:
            @"ECU serial (F18C) · %@", serial]];
    if (erotan.length != 0U)
        [values addObject:[NSString stringWithFormat:
            @"EROTAN (F196) · %@", erotan]];
    if (f100.length != 0U)
        [values addObject:[NSString stringWithFormat:
            @"Session / variant (F100) raw · %@", f100]];
    if (f154.length != 0U)
        [values addObject:[NSString stringWithFormat:
            @"Supplier identifier (F154) raw · %@", f154]];
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
    /*
     * Capability discovery must retain the responder CAN header as well.
     * Otherwise a functional 01xx request produces a useful union but the UI
     * cannot know which ECU advertised which PID until that PID has already
     * been polled. That circular dependency was why the per-ECU selector
     * showed only a handful of values.
     */
    flowConfig.preserve_pid_discovery_response_headers = true;
    /*
     * Keep live EOBD responder CAN IDs in the raw evidence stream so
     * simultaneous 7E8/7E9 replies remain attributable while the shared
     * decoder continues to present the first matching standard value.
     */
    flowConfig.preserve_live_response_headers = true;
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
    _manufacturerDataScan = (MblinkMercedesDataScan){0};
    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanStatusText = @"Not scanned";
    self.manufacturerDataScanModuleIdentifier = nil;
    _manufacturerDataByModule = [[NSMutableDictionary alloc] init];
    _standardDataByResponder = [[NSMutableDictionary alloc] init];
    _standardDataLatest = [[NSMutableDictionary alloc] init];
    _manufacturerDataForceFullScan = NO;
    _automaticTransmissionTemperatureProbeAttempted = NO;
    ++_manufacturerDataRequestGeneration;
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
- (NSString *)readinessStatusText { return _shared.readinessStatusText; }
- (NSArray<NSString *> *)readinessMonitorStatus
{
    return _shared.readinessMonitorStatus;
}
- (NSArray<NSString *> *)freezeFrameContext
{
    return _shared.freezeFrameContext;
}
- (NSString *)diagnosticCapabilityText
{
    return _shared.diagnosticCapabilityText;
}
- (NSString *)diagnosticCapabilityDetailText
{
    return _shared.diagnosticCapabilityDetailText;
}
- (NSString *)standardResponderSummary
{
    return _shared.standardResponderSummary;
}
- (NSString *)supportedPIDSummary
{
    return _shared.supportedPIDSummary;
}
- (NSString *)standardVINText
{
    return _shared.standardVINText;
}
- (NSArray<NSString *> *)standardLiveValueRows
{
    return _shared.standardLiveValueRows;
}
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
    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanModuleIdentifier = nil;
    _manufacturerDataForceFullScan = NO;
    ++_manufacturerDataRequestGeneration;
    [_shared disconnect];
}

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit
{
    return [_shared recentValuesForPID:pid limit:limit];
}

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                     responderCANIdentifier:(uint32_t)responderCANIdentifier
                                  extendedID:(BOOL)extendedID
                                       limit:(NSUInteger)limit
{
    return [_shared recentValuesForPID:pid
                responderCANIdentifier:responderCANIdentifier
                             extendedID:extendedID
                                  limit:limit];
}

- (nullable MBLinkStandardDataSnapshot *)standardDataSnapshotForPID:(uint8_t)pid
{
    return _standardDataLatest[@(pid)];
}

- (nullable MBLinkStandardDataSnapshot *)standardDataSnapshotForPID:(uint8_t)pid
                     responderCANIdentifier:(uint32_t)responderCANIdentifier
                                  extendedID:(BOOL)extendedID
{
    return _standardDataByResponder[
        MBLinkStandardDataKey(pid, responderCANIdentifier, extendedID)];
}

- (NSArray<NSNumber *> *)observedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID
{
    /*
     * "Available" for selection is capability-driven, not sample-driven.
     * Keep cached/live evidence as a fallback for older profiles/adapters, but
     * put the exact responder's 0100/0120/... advertised bitmap first.
     */
    NSMutableOrderedSet<NSNumber *> *pids =
        [[NSMutableOrderedSet alloc] initWithArray:
            [_shared supportedPIDsForResponderCANIdentifier:
                responderCANIdentifier extendedID:extendedID]];
    [pids addObjectsFromArray:
        [self cachedPIDsForResponderCANIdentifier:
            responderCANIdentifier extendedID:extendedID]];
    [pids addObjectsFromArray:
        [_shared observedPIDsForResponderCANIdentifier:
            responderCANIdentifier extendedID:extendedID]];
    return [[pids array] sortedArrayUsingSelector:@selector(compare:)];
}

- (NSArray<MBLinkMercedesModuleSnapshot *> *)mercedesModuleSnapshots
{
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    NSMutableArray<MBLinkMercedesModuleSnapshot *> *snapshots =
        [[NSMutableArray alloc] initWithCapacity:count];
    NSArray<NSString *> *engineEvidence =
        MBLinkEngineProbeEvidence(&_mercedesProbe);
    if (engineEvidence.count == 0U &&
        [_cachedVehicleProfile[@"engineEvidence"]
            isKindOfClass:[NSArray class]]) {
        engineEvidence = _cachedVehicleProfile[@"engineEvidence"];
    }

    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (module == NULL) continue;

        MBLinkMercedesModuleSnapshot *snapshot =
            [[MBLinkMercedesModuleSnapshot alloc] init];
        snapshot.identifier = MBLinkMercedesModuleIdentifier(module);
        snapshot.name = MBLinkStringFromCString(
            mblink_mercedes_module_scan_module_name(module));
        snapshot.kind = MBLinkStringFromCString(
            mblink_mercedes_module_kind_name(module->kind));
        snapshot.protocolName = MBLinkStringFromCString(
            mblink_mercedes_diagnostic_protocol_name(
                mblink_mercedes_module_scan_entry_protocol(module)));
        snapshot.requestCANIdentifier = module->tx_can_id;
        snapshot.responseCANIdentifier = module->rx_can_id;
        snapshot.extendedID = module->extended_id;
        snapshot.designation = module->definition != NULL &&
                module->definition->component_designation != NULL
            ? MBLinkStringFromCString(
                module->definition->component_designation) : @"";
        snapshot.network = module->definition != NULL &&
                module->definition->network != NULL
            ? MBLinkStringFromCString(module->definition->network) : @"";
        snapshot.identityText = module->identity_available
            ? MBLinkStringFromCString(module->identity) : nil;
        snapshot.partNumber = module->spare_part_number_available
            ? MBLinkStringFromCString(module->spare_part_number) : nil;
        snapshot.softwareNumber = module->software_number_available
            ? MBLinkStringFromCString(module->software_number) : nil;
        snapshot.hardwareNumber = module->hardware_number_available
            ? MBLinkStringFromCString(module->hardware_number) : nil;
        if (!module->extended_id &&
            module->tx_can_id == UINT32_C(0x7e0)) {
            if (snapshot.identityText.length == 0U &&
                _mercedesProbe.ecu_system_name_available) {
                snapshot.identityText = MBLinkStringFromCString(
                    _mercedesProbe.ecu_system_name);
            }
            if (snapshot.partNumber.length == 0U &&
                _mercedesProbe.ecu_spare_part_number_available) {
                snapshot.partNumber = MBLinkStringFromCString(
                    _mercedesProbe.ecu_spare_part_number);
            }
            if (snapshot.softwareNumber.length == 0U &&
                _mercedesProbe.ecu_software_number_available) {
                snapshot.softwareNumber = MBLinkStringFromCString(
                    _mercedesProbe.ecu_software_number);
            }
            if (snapshot.hardwareNumber.length == 0U &&
                _mercedesProbe.ecu_hardware_number_available) {
                snapshot.hardwareNumber = MBLinkStringFromCString(
                    _mercedesProbe.ecu_hardware_number);
            }
            snapshot.evidenceDetails = engineEvidence;
        } else if (!module->extended_id &&
                   module->tx_can_id == UINT32_C(0x7e1) &&
                   module->rx_can_id == UINT32_C(0x7e9)) {
            snapshot.evidenceDetails = @[
                @"Mercedes GS route · D_RQ_GS 0x7E1 → D_RS_GS 0x7E9",
                @"Read-only 21 30 · ATF temperature + current-gear candidate",
                @"Passive GS 0x218 · target/actual gear · converter · shift · limp · overheat · kickdown",
                @"Passive GS 0x338 · gearbox output speed + turbine-speed raw values",
                @"Passive GS 0x418 · selector/program · temperature raw · target/actual gear"
            ];
        } else {
            snapshot.evidenceDetails = @[];
        }
        snapshot.faultStatus = MBLinkMercedesModuleFaultStatus(module);
        snapshot.faultCount =
            mblink_mercedes_module_scan_entry_dtc_count(module);
        NSMutableArray<NSString *> *faults = [[NSMutableArray alloc] init];
        NSString *address = module->extended_id
            ? [NSString stringWithFormat:@"0x%08X → 0x%08X",
                (unsigned int)module->tx_can_id,
                (unsigned int)module->rx_can_id]
            : [NSString stringWithFormat:@"0x%03X → 0x%03X",
                (unsigned int)module->tx_can_id,
                (unsigned int)module->rx_can_id];
        MBLinkAppendMercedesModuleFaultStrings(
            faults, module, snapshot.name, address);
        snapshot.faults = [faults copy];
        [snapshots addObject:snapshot];
    }

    /*
     * A standards-based live response is independent proof that a control
     * unit exists. In the captured C207 session 7E9 continued answering Mode
     * 01 after its UDS probe returned no data, so do not erase that responder
     * from the vehicle map merely because the manufacturer session is quiet.
     */
    for (uint32_t responseID = UINT32_C(0x7e8);
         responseID <= UINT32_C(0x7ef);
         ++responseID) {
        NSArray<NSNumber *> *pids =
            [self observedPIDsForResponderCANIdentifier:
                responseID extendedID:NO];
        if (pids.count == 0U) continue;
        BOOL alreadyPresent = NO;
        for (MBLinkMercedesModuleSnapshot *existing in snapshots) {
            if (!existing.isExtendedID &&
                existing.responseCANIdentifier == responseID) {
                alreadyPresent = YES;
                break;
            }
        }
        if (alreadyPresent) continue;

        const uint32_t requestID = responseID - UINT32_C(8);
        const MblinkMercedesModuleKind kind =
            mblink_mercedes_module_scan_kind(requestID, false);
        MBLinkMercedesModuleSnapshot *snapshot =
            [[MBLinkMercedesModuleSnapshot alloc] init];
        snapshot.identifier = [NSString stringWithFormat:
            @"11:%08X:%08X", (unsigned int)requestID,
            (unsigned int)responseID];
        snapshot.name = requestID == UINT32_C(0x7e0)
            ? @"Engine ECU"
            : requestID == UINT32_C(0x7e1)
                ? @"Transmission ECU / GS"
                : [NSString stringWithFormat:
                    @"OBD responder 0x%03X", (unsigned int)responseID];
        snapshot.designation = requestID == UINT32_C(0x7e1)
            ? @"GS gearbox-control diagnostic responder"
            : @"Observed legislated-OBD responder";
        snapshot.network = @"Powertrain CAN / legislated OBD";
        snapshot.kind = MBLinkStringFromCString(
            mblink_mercedes_module_kind_name(kind));
        snapshot.protocolName = @"SAE Mode 01 / ISO 15765-4";
        snapshot.requestCANIdentifier = requestID;
        snapshot.responseCANIdentifier = responseID;
        snapshot.extendedID = NO;
        snapshot.faultStatus =
            @"Live responder observed · module fault state not established";
        snapshot.faultCount = 0U;
        snapshot.faults = @[];
        if (requestID == UINT32_C(0x7e1)) {
            snapshot.evidenceDetails = @[
                [NSString stringWithFormat:
                    @"Live Mode 01 responder · %lu confirmed PID%@",
                    (unsigned long)pids.count, pids.count == 1U ? @"" : @"s"],
                @"Mercedes GS route · D_RQ_GS 0x7E1 → D_RS_GS 0x7E9",
                @"Read-only 21 30 · ATF temperature + current-gear candidate",
                @"Passive GS 0x218 / 0x338 / 0x418 decoder support compiled"
            ];
        } else {
            snapshot.evidenceDetails = @[
                [NSString stringWithFormat:
                    @"Live Mode 01 responder · %lu confirmed PID%@",
                    (unsigned long)pids.count, pids.count == 1U ? @"" : @"s"]
            ];
        }
        [snapshots addObject:snapshot];
    }
    return [snapshots copy];
}

- (BOOL)favouriteForPID:(uint8_t)pid
{
    return [_shared favouriteForPID:pid];
}

- (void)setFavourite:(BOOL)favourite forPID:(uint8_t)pid
{
    [_shared setFavourite:favourite forPID:pid];
}

- (BOOL)pollingEnabledForPID:(uint8_t)pid
{
    return [_shared pollingEnabledForPID:pid];
}

- (void)setPollingEnabled:(BOOL)enabled forPID:(uint8_t)pid
{
    [_shared setPollingEnabled:enabled forPID:pid];
}

- (BOOL)supportsPID:(uint8_t)pid
{
    const LinkDiagnosticFlow *flow = [_shared diagnosticFlow];
    return flow != NULL &&
        link_obd2_pid_set_contains(&flow->supported_pids, pid);
}

- (nullable NSData *)csvDataSnapshot
{
    return [_shared csvDataSnapshot];
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
    if (event == NULL) return;
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE ||
        event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_STRUCTURED) {
        for (size_t i = 0U; i < event->responder_decoded.count; ++i) {
            const LinkObd2ResponderDecodedPid *entry = &event->responder_decoded.entries[i];
            if (!entry->responder_id_available || entry->decoded.definition == NULL) continue;
            MBLinkStandardDataSnapshot *snapshot = [[MBLinkStandardDataSnapshot alloc] init];
            snapshot.pid = entry->decoded.definition->pid;
            snapshot.responderCANIdentifier = entry->responder_id;
            snapshot.extendedID = entry->extended_id;
            snapshot.valueKind = (NSUInteger)entry->decoded.definition->value_kind;
            snapshot.signalCount = entry->decoded.signal_count;
            snapshot.formattedValue = MBLinkDecodedDisplay(&entry->decoded);
            snapshot.rawHex = MBLinkDecodedRawHex(&entry->decoded);
            _standardDataByResponder[MBLinkStandardDataKey(
                snapshot.pid, snapshot.responderCANIdentifier, snapshot.extendedID)] = snapshot;
            MBLinkStandardDataSnapshot *current =
                _standardDataLatest[@(snapshot.pid)];
            if (MBLinkStandardSnapshotPreferred(snapshot, current))
                _standardDataLatest[@(snapshot.pid)] = snapshot;
        }
        [self persistCapabilitiesFromFlowEvent:event];
        [self notifyDelegate];
        return;
    }
    if (event->kind != LINK_DIAGNOSTIC_FLOW_EVENT_STANDARD_VIN) return;

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
    if (self.manufacturerDataScanActive) {
        [self processMercedesDataScanResponse:
            (const MblinkElm327Response *)response];
        return;
    }
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
    if (self.manufacturerDataScanActive) {
        [self publishManufacturerDataScanResults];
        NSString *module = self.manufacturerDataScanModuleIdentifier ?: @"module";
        self.manufacturerDataScanStatusText = [NSString stringWithFormat:
            @"%@ manufacturer-data scan interrupted: %@ · %zu positive retained",
            module, status,
            mblink_mercedes_data_scan_record_count(&_manufacturerDataScan)];
        self.manufacturerDataScanActive = NO;
        self.manufacturerDataScanModuleIdentifier = nil;
        ++_manufacturerDataRequestGeneration;
        [self notifyDelegate];
        return;
    }
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

- (nullable const MblinkMercedesModuleScanEntry *)
    moduleEntryForIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return NULL;
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (module == NULL) continue;
        if ([MBLinkMercedesModuleIdentifier(module)
                isEqualToString:identifier]) {
            return module;
        }
    }
    return NULL;
}

- (nullable NSString *)automaticTransmissionTemperatureModuleIdentifier
{
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (module == NULL || module->extended_id) continue;
        if (module->tx_can_id == UINT32_C(0x7e1) &&
            module->rx_can_id == UINT32_C(0x7e9)) {
            return MBLinkMercedesModuleIdentifier(module);
        }
    }
    return nil;
}

- (NSArray<MBLinkMercedesDataSnapshot *> *)
    manufacturerDataSnapshotsForModuleIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return @[];
    NSArray<MBLinkMercedesDataSnapshot *> *values =
        _manufacturerDataByModule[identifier];
    return values != nil ? [values copy] : @[];
}

- (void)discoverManufacturerDataForModuleIdentifier:(NSString *)identifier
{
    [self beginManufacturerDataScanForModuleIdentifier:identifier
                                         forceFullScan:NO];
}

- (void)rescanManufacturerDataForModuleIdentifier:(NSString *)identifier
{
    [self beginManufacturerDataScanForModuleIdentifier:identifier
                                         forceFullScan:YES];
}

- (void)beginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                        forceFullScan:(BOOL)forceFullScan
{
    if (identifier.length == 0U || !_shared.isActive) return;

    if (self.manufacturerDataScanActive) {
        self.manufacturerDataScanStatusText =
            [self.manufacturerDataScanModuleIdentifier
                isEqualToString:identifier]
                ? @"This module scan is already in progress"
                : @"Another module scan is already in progress";
        [self notifyDelegate];
        return;
    }

    /*
     * Refresh is deliberately non-destructive: it re-reads the identifiers
     * already proven positive. Full rescan searches the bounded safe range
     * again so newly responding identifiers can be added. Neither path is
     * allowed to erase historical positive evidence merely because one pass
     * times out or returns NO DATA.
     */
    _manufacturerDataForceFullScan = forceFullScan;
    const NSUInteger generation = ++_manufacturerDataRequestGeneration;
    self.manufacturerDataScanModuleIdentifier = identifier;
    self.manufacturerDataScanStatusText = forceFullScan
        ? @"Waiting for a safe gap to rescan the full module data range"
        : @"Waiting for a safe gap to refresh known module data";
    [self notifyDelegate];

    [self tryBeginManufacturerDataScanForModuleIdentifier:identifier
                                               generation:generation
                                                  attempt:0U];
}

- (void)tryBeginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                             generation:(NSUInteger)generation
                                                attempt:(NSUInteger)attempt
{
    if (generation != _manufacturerDataRequestGeneration ||
        !_shared.isActive ||
        ![self.manufacturerDataScanModuleIdentifier
            isEqualToString:identifier]) {
        return;
    }

    const MblinkMercedesModuleScanEntry *module =
        [self moduleEntryForIdentifier:identifier];
    if (module == NULL) {
        self.manufacturerDataScanStatusText =
            @"The selected Mercedes module is no longer in the active VIN profile";
        self.manufacturerDataScanModuleIdentifier = nil;
        _manufacturerDataForceFullScan = NO;
        [self notifyDelegate];
        return;
    }

    if (![_shared beginLiveManufacturerExtension]) {
        if (attempt < 80U) {
            dispatch_after(
                dispatch_time(
                    DISPATCH_TIME_NOW,
                    (int64_t)UINT64_C(125) * NSEC_PER_MSEC),
                dispatch_get_main_queue(), ^{
                    [self tryBeginManufacturerDataScanForModuleIdentifier:
                        identifier generation:generation attempt:attempt + 1U];
                });
            return;
        }
        self.manufacturerDataScanStatusText =
            @"Could not pause standard live polling for the Mercedes data scan";
        self.manufacturerDataScanModuleIdentifier = nil;
        _manufacturerDataForceFullScan = NO;
        [self notifyDelegate];
        return;
    }

    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            module->tx_can_id,
            module->rx_can_id,
            module->extended_id,
            mblink_mercedes_module_scan_entry_protocol(module),
            module->kind);
    NSArray<MBLinkMercedesDataSnapshot *> *knownValues =
        _manufacturerDataByModule[identifier];
    NSArray<NSNumber *> *persistedIdentifiers = nil;
    if (knownValues.count == 0U &&
        [_cachedVehicleProfile[@"modules"] isKindOfClass:[NSArray class]]) {
        for (id value in _cachedVehicleProfile[@"modules"]) {
            if (![value isKindOfClass:[NSDictionary class]]) continue;
            NSDictionary *savedModule = (NSDictionary *)value;
            NSNumber *savedTx = savedModule[@"tx"];
            NSNumber *savedRx = savedModule[@"rx"];
            NSNumber *savedExtended = savedModule[@"extended"];
            if (![savedTx isKindOfClass:[NSNumber class]] ||
                ![savedRx isKindOfClass:[NSNumber class]] ||
                ![savedExtended isKindOfClass:[NSNumber class]] ||
                savedTx.unsignedIntValue != module->tx_can_id ||
                savedRx.unsignedIntValue != module->rx_can_id ||
                savedExtended.boolValue != module->extended_id) {
                continue;
            }
            if ([savedModule[@"manufacturerDataIDs"]
                    isKindOfClass:[NSArray class]]) {
                persistedIdentifiers = savedModule[@"manufacturerDataIDs"];
            }
            break;
        }
    }

    const NSUInteger knownIdentifierCount =
        knownValues.count != 0U ? knownValues.count : persistedIdentifiers.count;
    BOOL targetedRefresh =
        !_manufacturerDataForceFullScan &&
        knownIdentifierCount > 0U &&
        knownIdentifierCount <= MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS;
    MblinkMercedesDataScanResult result;

    if (targetedRefresh) {
        uint16_t identifiers[MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS];
        size_t identifierCount = 0U;
        if (knownValues.count != 0U) {
            for (MBLinkMercedesDataSnapshot *snapshot in knownValues) {
                if (identifierCount >= MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS)
                    break;
                identifiers[identifierCount++] = snapshot.identifier;
            }
        } else {
            for (id value in persistedIdentifiers) {
                if (identifierCount >= MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS)
                    break;
                if (![value isKindOfClass:[NSNumber class]]) continue;
                const NSUInteger candidate =
                    ((NSNumber *)value).unsignedIntegerValue;
                if (candidate > UINT16_MAX) continue;
                identifiers[identifierCount++] = (uint16_t)candidate;
            }
            if (identifierCount == 0U) targetedRefresh = NO;
        }
        result = targetedRefresh
            ? mblink_mercedes_data_scan_begin_identifiers(
                &_manufacturerDataScan, &config,
                identifiers, identifierCount)
            : mblink_mercedes_data_scan_begin(
                &_manufacturerDataScan, &config);
        if (result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK) {
            /*
             * Cached presentation data must never make discovery impossible.
             * If a stale/invalid cached list cannot initialise, fall back to
             * the normal bounded range scan.
             */
            targetedRefresh = NO;
            result = mblink_mercedes_data_scan_begin(
                &_manufacturerDataScan, &config);
        }
    } else {
        result = mblink_mercedes_data_scan_begin(
            &_manufacturerDataScan, &config);
    }
    if (result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK) {
        (void)[_shared completeManufacturerExtensionRestoringAdapter:YES];
        self.manufacturerDataScanStatusText = [NSString stringWithFormat:
            @"Mercedes data scan could not start: %@",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_result_name(result))];
        self.manufacturerDataScanModuleIdentifier = nil;
        _manufacturerDataForceFullScan = NO;
        [self notifyDelegate];
        return;
    }

    self.manufacturerDataScanActive = YES;
    self.manufacturerDataScanStatusText = targetedRefresh
        ? [NSString stringWithFormat:
            @"Refreshing %zu known-positive Mercedes data ID%@ · %@",
            _manufacturerDataScan.identifier_count,
            _manufacturerDataScan.identifier_count == 1U ? @"" : @"s",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_stage_name(
                    _manufacturerDataScan.stage))]
        : [NSString stringWithFormat:
            @"%@ · %@",
            _manufacturerDataForceFullScan
                ? @"Rescanning full Mercedes manufacturer-data range"
                : @"Reading Mercedes manufacturer data",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_stage_name(
                    _manufacturerDataScan.stage))];
    [self notifyDelegate];
    [self beginCurrentMercedesDataScanCommand];
}

- (void)beginCurrentMercedesDataScanCommand
{
    char command[MBLINK_ELM327_MAX_COMMAND];
    size_t written = 0U;
    MblinkMercedesDataScanResult result =
        mblink_mercedes_data_scan_command(
            &_manufacturerDataScan,
            command, sizeof(command), &written);
    if (result == MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE) {
        [self finishManufacturerDataScanWithStatus:@"Complete"];
        return;
    }
    if (result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK ||
        written == 0U) {
        [self finishManufacturerDataScanWithStatus:
            @"Manufacturer-data command generation failed"];
        return;
    }

    self.manufacturerDataScanStatusText = [NSString stringWithFormat:
        @"Mercedes data · %@ · ID 0x%04X · %zu checked · %zu positive",
        MBLinkStringFromCString(
            mblink_mercedes_data_scan_stage_name(
                _manufacturerDataScan.stage)),
        (unsigned int)_manufacturerDataScan.current_identifier,
        _manufacturerDataScan.attempted_count,
        mblink_mercedes_data_scan_record_count(&_manufacturerDataScan)];
    [self notifyDelegate];

    if (![_shared beginManufacturerCommand:command
                                   timeout:mblink_mercedes_data_scan_timeout_ms(
                                       &_manufacturerDataScan)]) {
        [self finishManufacturerDataScanWithStatus:
            @"Mercedes manufacturer-data command could not be sent"];
    }
}

- (void)processMercedesDataScanResponse:
    (const MblinkElm327Response *)response
{
    MblinkMercedesDataScanResult result =
        mblink_mercedes_data_scan_accept(
            &_manufacturerDataScan, response);
    if (result == MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE ||
        _manufacturerDataScan.stage ==
            MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE) {
        [self finishManufacturerDataScanWithStatus:@"Complete"];
        return;
    }
    if (result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK ||
        _manufacturerDataScan.stage ==
            MBLINK_MERCEDES_DATA_SCAN_STAGE_FAILED) {
        [self finishManufacturerDataScanWithStatus:[NSString stringWithFormat:
            @"Incomplete · %@",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_result_name(result))]];
        return;
    }
    [self beginCurrentMercedesDataScanCommand];
}

- (void)publishManufacturerDataScanResults
{
    NSString *identifier = self.manufacturerDataScanModuleIdentifier;
    if (identifier.length == 0U) return;

    const MblinkMercedesModuleScanEntry *module =
        [self moduleEntryForIdentifier:identifier];
    if (module == NULL) return;

    const size_t count =
        mblink_mercedes_data_scan_record_count(&_manufacturerDataScan);
    NSMutableArray<MBLinkMercedesDataSnapshot *> *values =
        [[NSMutableArray alloc] initWithCapacity:count];

    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesDataRecord *record =
            mblink_mercedes_data_scan_record_at(
                &_manufacturerDataScan, index);
        if (record == NULL) continue;

        char code[64];
        char raw[MBLINK_MERCEDES_DATA_SCAN_MAX_DATA * 2U + 1U];
        if (!mblink_mercedes_data_record_format_code(
                record, code, sizeof(code))) {
            (void)snprintf(
                code, sizeof(code), "Data ID 0x%04X",
                (unsigned int)record->identifier);
        }
        if (!mblink_mercedes_data_record_format_hex(
                record, raw, sizeof(raw))) {
            (void)snprintf(raw, sizeof(raw), "%s", "<truncated>");
        }

        MBLinkMercedesDataSnapshot *snapshot =
            [[MBLinkMercedesDataSnapshot alloc] init];
        snapshot.identifier = record->identifier;
        snapshot.service = record->service;
        snapshot.codeText = MBLinkStringFromCString(code);
        snapshot.rawHex = MBLinkStringFromCString(raw);

        double numeric = 0.0;
        const char *name = NULL;
        const char *unit = NULL;
        if (mblink_mercedes_data_record_decode_known_numeric_for_route(
                module->tx_can_id,
                module->rx_can_id,
                module->extended_id,
                module->kind,
                record, &numeric, &name, &unit)) {
            snapshot.mapped = YES;
            snapshot.numericValueAvailable = YES;
            snapshot.numericValue = numeric;
            snapshot.name = MBLinkStringFromCString(name);
            snapshot.unit = MBLinkStringFromCString(unit);
            snapshot.formattedValue =
                [snapshot.unit isEqualToString:@"°C"]
                    ? [NSString stringWithFormat:@"%.1f %@", numeric, snapshot.unit]
                    : [NSString stringWithFormat:@"%.3f %@", numeric, snapshot.unit];
        } else {
            snapshot.mapped = NO;
            snapshot.numericValueAvailable = NO;
            snapshot.numericValue = 0.0;
            snapshot.name = nil;
            snapshot.unit = nil;
            snapshot.formattedValue = [NSString stringWithFormat:
                @"RAW %@", snapshot.rawHex];
        }
        [values addObject:snapshot];
    }

    /*
     * A positive response is durable discovery evidence. A later timeout or
     * NO DATA is not proof that the identifier ceased to exist, especially on
     * a busy in-vehicle CAN network. Merge refreshed values into the previous
     * module set instead of replacing the set with only this pass.
     */
    NSArray<MBLinkMercedesDataSnapshot *> *previous =
        _manufacturerDataByModule[identifier] ?: @[];
    NSMutableDictionary<NSString *, MBLinkMercedesDataSnapshot *> *merged =
        [[NSMutableDictionary alloc] initWithCapacity:
            previous.count + values.count];

    for (MBLinkMercedesDataSnapshot *snapshot in previous) {
        NSString *key = [NSString stringWithFormat:@"%02X:%04X",
            (unsigned int)snapshot.service,
            (unsigned int)snapshot.identifier];
        merged[key] = snapshot;
    }
    for (MBLinkMercedesDataSnapshot *snapshot in values) {
        NSString *key = [NSString stringWithFormat:@"%02X:%04X",
            (unsigned int)snapshot.service,
            (unsigned int)snapshot.identifier];
        merged[key] = snapshot;
    }

    NSArray<MBLinkMercedesDataSnapshot *> *retained =
        [[merged allValues] sortedArrayUsingComparator:
            ^NSComparisonResult(
                MBLinkMercedesDataSnapshot *left,
                MBLinkMercedesDataSnapshot *right) {
                if (left.service < right.service) return NSOrderedAscending;
                if (left.service > right.service) return NSOrderedDescending;
                if (left.identifier < right.identifier) return NSOrderedAscending;
                if (left.identifier > right.identifier) return NSOrderedDescending;
                return NSOrderedSame;
            }];
    _manufacturerDataByModule[identifier] = retained;
}

- (void)finishManufacturerDataScanWithStatus:(NSString *)status
{
    [self publishManufacturerDataScanResults];
    /*
     * Positive manufacturer identifiers are expensive to discover but cheap to
     * refresh. Persist them against the VIN/module route immediately so a later
     * session can target only known positives instead of repeating the full
     * 0x20xx / KWP local-ID sweep.
     */
    [self saveCurrentVehicleProfile];

    const size_t positive =
        mblink_mercedes_data_scan_record_count(&_manufacturerDataScan);
    const size_t attempted = _manufacturerDataScan.attempted_count;
    NSString *module = self.manufacturerDataScanModuleIdentifier ?: @"Module";
    const NSUInteger retained =
        _manufacturerDataByModule[module].count;
    self.manufacturerDataScanStatusText = [NSString stringWithFormat:
        @"%@ · %@ · %zu checked · %zu responded this pass · %lu retained",
        module, status, attempted, positive, (unsigned long)retained];

    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanModuleIdentifier = nil;
    _manufacturerDataForceFullScan = NO;
    ++_manufacturerDataRequestGeneration;

    if (![_shared completeManufacturerExtensionRestoringAdapter:YES]) {
        [_shared failWithStatus:
            @"Could not resume standard diagnostics after Mercedes data scan"];
    }
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
        mblink_mercedes_module_scan_begin_mobile_census(&_mercedesModuleScan);
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) {
        self.mercedesProbeStatusText = @"Mercedes module discovery could not start";
        [self finishMercedesExtensionRestoringAdapter:YES];
        return;
    }
    _moduleScanActive = YES;
    /*
     * A new VIN gets one read-only mobile census so MBLINK can learn the
     * vehicle's real module topology instead of caching only legislated OBD
     * powertrain endpoints. The mobile census uses the 47-slot Mercedes
     * gateway request/response lattice with exact receive filters, the small
     * source-backed exception set and the eight legislated OBD physical slots.
     * Deeper DTC/identity reads happen only after a responder is proven.
     *
     * The resulting routes are saved against the VIN and future connections
     * use the bounded cached refresh path. The exhaustive 11/29-bit sweep is
     * deliberately left to the workstation forensic tool.
     */
    self.mercedesProbeStatusText =
        @"Mercedes first-VIN mobile census · 47-slot gateway discovery";
    self.mercedesUDSFaultStatusText =
        @"Learning complete Mercedes module topology for this VIN";
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

        /*
         * One evidence-gated automatic read makes the source-corroborated
         * 7E1/7E9 21 30 transmission-temperature candidate visible without
         * requiring the driver to open the module screen first. It is only
         * attempted once per connection; continuous polling remains disabled
         * until the development car confirms the response shape/value.
         */
        NSString *temperatureModule =
            [self automaticTransmissionTemperatureModuleIdentifier];
        const BOOL shouldProbeTransmissionTemperature =
            !_automaticTransmissionTemperatureProbeAttempted &&
            temperatureModule.length != 0U;
        if (shouldProbeTransmissionTemperature)
            _automaticTransmissionTemperatureProbeAttempted = YES;

        [self finishMercedesExtensionRestoringAdapter:YES];

        if (shouldProbeTransmissionTemperature) {
            dispatch_after(
                dispatch_time(
                    DISPATCH_TIME_NOW,
                    (int64_t)UINT64_C(500) * NSEC_PER_MSEC),
                dispatch_get_main_queue(), ^{
                    if (self.isActive &&
                        !self.manufacturerDataScanActive) {
                        [self beginManufacturerDataScanForModuleIdentifier:
                            temperatureModule forceFullScan:NO];
                    }
                });
        }
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
        MBLinkAppendMercedesModuleFaultStrings(
            faults, module, name, address);
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
            [line hasPrefix:@"  PROTOCOL ·"] ||
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
        const size_t moduleFaultCount =
            mblink_mercedes_module_scan_entry_dtc_count(module);
        NSString *protocol = MBLinkStringFromCString(
            mblink_mercedes_diagnostic_protocol_name(
                mblink_mercedes_module_scan_entry_protocol(module)));
        if (module->definition != NULL) {
            [identity addObject:[NSString stringWithFormat:
                @"MODULE · %@ · %@ · %@ · %zu fault record%@",
                name,
                MBLinkStringFromCString(
                    module->definition->component_designation),
                address, moduleFaultCount,
                moduleFaultCount == 1U ? @"" : @"s"]];
            [identity addObject:[NSString stringWithFormat:
                @"  PROTOCOL · %@", protocol]];
        } else if (module->kind != MBLINK_MERCEDES_MODULE_OTHER) {
            [identity addObject:[NSString stringWithFormat:
                @"MODULE · %@ · %@ · %@ candidate · %zu fault record%@",
                name, address,
                MBLinkStringFromCString(
                    mblink_mercedes_module_kind_name(module->kind)),
                moduleFaultCount,
                moduleFaultCount == 1U ? @"" : @"s"]];
            [identity addObject:[NSString stringWithFormat:
                @"  PROTOCOL · %@", protocol]];
        } else {
            [identity addObject:[NSString stringWithFormat:
                @"MODULE · %@ · %@ · unresolved family · %zu fault record%@",
                name, address, moduleFaultCount,
                moduleFaultCount == 1U ? @"" : @"s"]];
            [identity addObject:[NSString stringWithFormat:
                @"  PROTOCOL · %@", protocol]];
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
        MBLinkAppendMercedesModuleFaultStrings(
            faults, module, name, address);
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
        schema.integerValue < MBLinkOldestReadableVehicleProfileSchemaVersion ||
        schema.integerValue > MBLinkVehicleProfileSchemaVersion ||
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

- (NSArray<NSNumber *> *)cachedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID
{
    NSArray *responders = [_cachedVehicleProfile[@"liveResponders"]
        isKindOfClass:[NSArray class]]
        ? _cachedVehicleProfile[@"liveResponders"] : @[];
    NSMutableOrderedSet<NSNumber *> *pids =
        [[NSMutableOrderedSet alloc] init];
    for (id value in responders) {
        if (![value isKindOfClass:[NSDictionary class]]) continue;
        NSDictionary *responder = (NSDictionary *)value;
        NSNumber *rx = responder[@"rx"];
        NSNumber *extended = responder[@"extended"];
        NSArray *storedPIDs = [responder[@"pids"] isKindOfClass:[NSArray class]]
            ? responder[@"pids"] : @[];
        if (![rx isKindOfClass:[NSNumber class]] ||
            ![extended isKindOfClass:[NSNumber class]] ||
            rx.unsignedIntValue != responderCANIdentifier ||
            extended.boolValue != extendedID) {
            continue;
        }
        for (id pid in storedPIDs) {
            if ([pid isKindOfClass:[NSNumber class]] &&
                ((NSNumber *)pid).unsignedIntegerValue <= UINT8_MAX) {
                [pids addObject:pid];
            }
        }
    }
    return [[pids array] sortedArrayUsingSelector:@selector(compare:)];
}

- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event
{
    if (event == NULL ||
        (event->kind != LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE &&
         event->kind != LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_STRUCTURED) ||
        (event->responder_samples.count == 0U &&
         event->responder_decoded.count == 0U) ||
        self.mercedesVINText.length == 0U) {
        return;
    }

    NSDictionary *existing = _cachedVehicleProfile;
    if (existing == nil)
        existing = [self savedVehicleProfileForVIN:self.mercedesVINText];
    if (existing == nil) return;

    NSMutableDictionary *profile = [existing mutableCopy];
    NSMutableArray<NSMutableDictionary *> *responders =
        [[NSMutableArray alloc] init];
    NSArray *storedResponders = [profile[@"liveResponders"]
        isKindOfClass:[NSArray class]] ? profile[@"liveResponders"] : @[];
    for (id value in storedResponders) {
        if ([value isKindOfClass:[NSDictionary class]])
            [responders addObject:[(NSDictionary *)value mutableCopy]];
    }

    BOOL changed = NO;
    for (size_t index = 0U;
         index < event->responder_samples.count;
         ++index) {
        const LinkObd2ResponderSample *sample =
            &event->responder_samples.samples[index];
        if (!sample->responder_id_available) continue;

        NSMutableDictionary *match = nil;
        for (NSMutableDictionary *candidate in responders) {
            NSNumber *rx = candidate[@"rx"];
            NSNumber *extended = candidate[@"extended"];
            if ([rx isKindOfClass:[NSNumber class]] &&
                [extended isKindOfClass:[NSNumber class]] &&
                rx.unsignedIntValue == sample->responder_id &&
                extended.boolValue == sample->extended_id) {
                match = candidate;
                break;
            }
        }
        if (match == nil) {
            match = [@{
                @"rx": @(sample->responder_id),
                @"extended": @(sample->extended_id),
                @"pids": @[]
            } mutableCopy];
            [responders addObject:match];
            changed = YES;
        }

        NSMutableOrderedSet<NSNumber *> *pids =
            [[NSMutableOrderedSet alloc] initWithArray:
                [match[@"pids"] isKindOfClass:[NSArray class]]
                    ? match[@"pids"] : @[]];
        NSNumber *pid = @(sample->sample.pid);
        if (![pids containsObject:pid]) {
            [pids addObject:pid];
            match[@"pids"] = [[pids array]
                sortedArrayUsingSelector:@selector(compare:)];
            changed = YES;
        }
    }
    /*
     * Structured/raw SAE PIDs may not have a scalar responder sample. Persist
     * responder capability from the decoded list too, so cached VIN profiles
     * retain DPF/NOx/aftertreatment and raw assigned Mode 01 identifiers.
     */
    for (size_t index = 0U;
         index < event->responder_decoded.count;
         ++index) {
        const LinkObd2ResponderDecodedPid *entry =
            &event->responder_decoded.entries[index];
        if (!entry->responder_id_available ||
            entry->decoded.definition == NULL) {
            continue;
        }

        NSMutableDictionary *match = nil;
        for (NSMutableDictionary *candidate in responders) {
            NSNumber *rx = candidate[@"rx"];
            NSNumber *extended = candidate[@"extended"];
            if ([rx isKindOfClass:[NSNumber class]] &&
                [extended isKindOfClass:[NSNumber class]] &&
                rx.unsignedIntValue == entry->responder_id &&
                extended.boolValue == entry->extended_id) {
                match = candidate;
                break;
            }
        }
        if (match == nil) {
            match = [@{
                @"rx": @(entry->responder_id),
                @"extended": @(entry->extended_id),
                @"pids": @[]
            } mutableCopy];
            [responders addObject:match];
            changed = YES;
        }

        NSMutableOrderedSet<NSNumber *> *pids =
            [[NSMutableOrderedSet alloc] initWithArray:
                [match[@"pids"] isKindOfClass:[NSArray class]]
                    ? match[@"pids"] : @[]];
        NSNumber *pid = @(entry->decoded.definition->pid);
        if (![pids containsObject:pid]) {
            [pids addObject:pid];
            match[@"pids"] = [[pids array]
                sortedArrayUsingSelector:@selector(compare:)];
            changed = YES;
        }
    }

    if (!changed) return;

    profile[@"schema"] = @(MBLinkVehicleProfileSchemaVersion);
    profile[@"updatedAt"] = @([[NSDate date] timeIntervalSince1970]);
    profile[@"liveResponders"] = [responders copy];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *profiles =
        [[defaults dictionaryForKey:MBLinkVehicleProfilesDefaultsKey]
            mutableCopy] ?: [[NSMutableDictionary alloc] init];
    profiles[self.mercedesVINText] = [profile copy];
    [defaults setObject:[profiles copy]
                 forKey:MBLinkVehicleProfilesDefaultsKey];
    _cachedVehicleProfile = [profile copy];
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
            @"protocol": @((NSUInteger)
                mblink_mercedes_module_scan_entry_protocol(module)),
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

        NSString *moduleIdentifier = MBLinkMercedesModuleIdentifier(module);
        NSArray<MBLinkMercedesDataSnapshot *> *knownManufacturerData =
            _manufacturerDataByModule[moduleIdentifier];
        NSMutableOrderedSet<NSNumber *> *manufacturerIDs =
            [[NSMutableOrderedSet alloc] init];
        for (MBLinkMercedesDataSnapshot *snapshot in knownManufacturerData) {
            [manufacturerIDs addObject:@(snapshot.identifier)];
        }
        if (manufacturerIDs.count == 0U &&
            [_cachedVehicleProfile[@"modules"] isKindOfClass:[NSArray class]]) {
            for (id savedValue in _cachedVehicleProfile[@"modules"]) {
                if (![savedValue isKindOfClass:[NSDictionary class]]) continue;
                NSDictionary *savedModule = (NSDictionary *)savedValue;
                NSNumber *savedTx = savedModule[@"tx"];
                NSNumber *savedRx = savedModule[@"rx"];
                NSNumber *savedExtended = savedModule[@"extended"];
                if (![savedTx isKindOfClass:[NSNumber class]] ||
                    ![savedRx isKindOfClass:[NSNumber class]] ||
                    ![savedExtended isKindOfClass:[NSNumber class]] ||
                    savedTx.unsignedIntValue != module->tx_can_id ||
                    savedRx.unsignedIntValue != module->rx_can_id ||
                    savedExtended.boolValue != module->extended_id) {
                    continue;
                }
                NSArray *savedIDs =
                    [savedModule[@"manufacturerDataIDs"]
                        isKindOfClass:[NSArray class]]
                        ? savedModule[@"manufacturerDataIDs"] : @[];
                for (id savedID in savedIDs) {
                    if ([savedID isKindOfClass:[NSNumber class]] &&
                        ((NSNumber *)savedID).unsignedIntegerValue <= UINT16_MAX) {
                        [manufacturerIDs addObject:savedID];
                    }
                }
                break;
            }
        }
        if (manufacturerIDs.count != 0U) {
            dictionary[@"manufacturerDataIDs"] =
                [[manufacturerIDs array]
                    sortedArrayUsingSelector:@selector(compare:)];
        }
        [modules addObject:[dictionary copy]];
    }
    if (modules.count == 0U) return;

    NSMutableDictionary *profile = [@{
        @"schema": @(MBLinkVehicleProfileSchemaVersion),
        @"vin": self.mercedesVINText,
        @"updatedAt": @([[NSDate date] timeIntervalSince1970]),
        @"discoveryScope": @(MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS),
        @"modules": [modules copy]
    } mutableCopy];
    if (self.mercedesProbeEndpointText.length != 0U)
        profile[@"probeEndpoint"] = self.mercedesProbeEndpointText;
    if (self.mercedesCrd3SummaryText.length != 0U &&
        ![self.mercedesCrd3SummaryText isEqualToString:@"Not attempted"]) {
        profile[@"crd3Summary"] = self.mercedesCrd3SummaryText;
    }
    NSArray *liveResponders = [_cachedVehicleProfile[@"liveResponders"]
        isKindOfClass:[NSArray class]]
        ? _cachedVehicleProfile[@"liveResponders"] : nil;
    if (liveResponders.count != 0U)
        profile[@"liveResponders"] = liveResponders;
    NSArray<NSString *> *engineEvidence =
        MBLinkEngineProbeEvidence(&_mercedesProbe);
    if (engineEvidence.count == 0U &&
        [_cachedVehicleProfile[@"engineEvidence"]
            isKindOfClass:[NSArray class]]) {
        engineEvidence = _cachedVehicleProfile[@"engineEvidence"];
    }
    if (engineEvidence.count != 0U)
        profile[@"engineEvidence"] = engineEvidence;

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
    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanModuleIdentifier = nil;
    if (![_shared completeManufacturerExtensionRestoringAdapter:restore]) {
        [_shared failWithStatus:
            @"Could not resume shared diagnostic flow after Mercedes probe"];
    }
}

- (void)updateMercedesProbeEvidenceSummary
{
    NSString *standardOBDVIN = self.mercedesVINText;
    NSArray<NSString *> *standardOBDIdentity = self.mercedesIdentityResults;
    const BOOL mercedesVINAvailable =
        _mercedesProbe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE &&
        _mercedesProbe.vin[0] != '\0';
    const unsigned int positive = MBLinkBitCount32(_mercedesProbe.identity_positive_mask);
    const unsigned int negative = MBLinkBitCount32(_mercedesProbe.identity_negative_mask);
    const unsigned int noResponse = MBLinkBitCount32(_mercedesProbe.identity_no_response_mask);
    const unsigned int invalid = MBLinkBitCount32(_mercedesProbe.identity_invalid_mask);
    const size_t total = mblink_mercedes_ecu_probe_identity_did_count();
    self.mercedesIdentitySummaryText = [NSString stringWithFormat:
        @"%u/%zu positive · %u negative · %u no response · %u invalid",
        positive, total, negative, noResponse, invalid];

    NSMutableArray<NSString *> *identityResults = [[NSMutableArray alloc] initWithCapacity:total];
    if (!mercedesVINAvailable) {
        for (NSString *line in standardOBDIdentity ?: @[]) {
            if ([line hasPrefix:@"VIN ·"] ||
                [line hasPrefix:@"ENGINE ·"] ||
                [line hasPrefix:@"BUILD ·"]) {
                [identityResults addObject:line];
            }
        }
    }
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
    if (mercedesVINAvailable) {
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
    if (mercedesVINAvailable) {
        self.mercedesVINText = MBLinkStringFromCString(_mercedesProbe.vin);
        [_shared setVehicleIdentifier:_mercedesProbe.vin];
        [self loadSavedVehicleProfileForVIN:self.mercedesVINText];
        vinSummary = [NSString stringWithFormat:
            @"Mercedes ECU VIN %@", self.mercedesVINText];
    } else {
        switch (_mercedesProbe.vin_result) {
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE:
            vinSummary = @"Mercedes ECU VIN did not respond"; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE:
            vinSummary = [NSString stringWithFormat:@"Mercedes ECU VIN negative response NRC 0x%02X",
                (unsigned int)_mercedesProbe.vin_negative_response_code]; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE:
            vinSummary = @"Mercedes ECU VIN was not a valid 17-character VIN"; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED:
            vinSummary = @"Mercedes ECU VIN was not attempted"; break;
        case MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE:
            vinSummary = @"Mercedes ECU VIN response was empty"; break;
        }
        if (standardOBDVIN.length != 0U) {
            self.mercedesVINText = standardOBDVIN;
            vinSummary = [NSString stringWithFormat:
                @"%@; standard OBD VIN %@ retained",
                vinSummary, standardOBDVIN];
        } else {
            self.mercedesVINText = nil;
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
