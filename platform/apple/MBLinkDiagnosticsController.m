// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkDiagnosticsController.h"

#import "../../src/link/platform/apple/LinkDiagnosticsController.h"
#import "link/diagnostic_request.h"
#import "mblink/elm327.h"
#import "mblink/mercedes.h"
#import "mblink/mercedes_module_scan.h"
#import "mblink/mercedes_data_scan.h"
#import "mblink/mercedes_ecu_pack.h"
#import "mblink/mercedes_documented_ecus.h"
#import "mblink/mercedes_transmission.h"

#include <stdio.h>
#include <string.h>

#import "MBLinkDiagnosticsModels.inc"

static NSString * const MBLinkVehicleProfilesDefaultsKey =
    @"mblink.vehicleProfiles.v1";
static const NSInteger MBLinkVehicleProfileSchemaVersion = 5;
static const NSInteger MBLinkOldestReadableVehicleProfileSchemaVersion = 3;
/*
 * Recurring Mercedes live work is an opaque LINK scheduler job. LINK
 * owns the one adapter-wire queue; MBLINK owns the GS route, KWP
 * request set and decoder. One GS refresh can yield gear, target gear,
 * ATF temperature and the other RLI actual values without competing
 * with a second timer/poll loop.
 */
static const uint32_t MBLinkScheduledTransmissionLiveJobToken =
    UINT32_C(0x4D420001);
static const uint32_t MBLinkScheduledTransmissionLiveIntervalMs =
    UINT32_C(750);
static NSString * const MBLinkDisabledManufacturerPollingDefaultsKey =
    @"mblink.disabledManufacturerLivePolling.v1";

typedef NS_ENUM(NSUInteger, MBLinkScheduledRestoreStage) {
    MBLinkScheduledRestoreNone = 0,
    MBLinkScheduledRestoreHeader,
    MBLinkScheduledRestoreFilter
};

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
- (void)beginStartupModuleDiscovery;
- (void)beginMercedesModuleScan;
- (void)beginCurrentMercedesModuleScanCommand;
- (void)processMercedesModuleScanResponse:(const MblinkElm327Response *)response;
- (void)updateMercedesModuleFaultEvidenceInProgress;
- (void)updateMercedesModuleScanSummary;
- (BOOL)beginNextStartupModuleDataRead;
- (nullable const MblinkMercedesModuleScanEntry *)
    moduleEntryForIdentifier:(NSString *)identifier;
- (void)beginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                        forceFullScan:(BOOL)forceFullScan;
- (void)beginManufacturerDataOperationForModuleIdentifier:
            (NSString *)identifier
                                               forceFullScan:(BOOL)forceFullScan
                                                    liveOnly:(BOOL)liveOnly
                                           candidateCommands:
            (nullable NSArray<NSNumber *> *)candidateCommands;
- (void)tryBeginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                             generation:(NSUInteger)generation
                                                attempt:(NSUInteger)attempt
                                               liveOnly:(BOOL)liveOnly
                                      candidateCommands:
            (nullable NSArray<NSNumber *> *)candidateCommands;
- (NSArray<NSNumber *> *)documentedPIDCommandsForModuleIdentifier:
    (NSString *)identifier;
- (void)updateScheduledManufacturerLiveJob;
- (void)beginScheduledTransmissionLiveJob;
- (void)beginScheduledManufacturerChannelRestore;
- (void)processScheduledManufacturerRestoreResponse:
    (const MblinkElm327Response *)response;
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
- (void)persistDiscoveredCapabilities;
- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event;
- (NSArray<NSNumber *> *)cachedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID;
- (void)finishMercedesExtensionRestoringAdapter:(BOOL)restore;
@end

@implementation MBLinkDiagnosticsController {
    LinkVehicleProfileStore *_vehicleProfileStore;
    MblinkMercedesModuleScan _mercedesModuleScan;
    BOOL _moduleScanActive;
    BOOL _cachedModuleRefreshActive;
    BOOL _startupModuleDiscoveryStarted;
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
    BOOL _manufacturerDataScanLiveOnly;
    BOOL _startupModuleDataPassActive;
    size_t _startupModuleDataIndex;
    BOOL _scheduledManufacturerJobRegistered;
    BOOL _scheduledManufacturerJobActive;
    MBLinkScheduledRestoreStage _scheduledManufacturerRestoreStage;
    NSMutableDictionary<NSString *, NSSet<NSNumber *> *> *
        _selectedManufacturerLiveCommandsByModule;
    NSUInteger _scheduledManufacturerModuleCursor;
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

static NSDictionary *MBLinkPersistedMercedesDataSnapshot(
    MBLinkMercedesDataSnapshot *snapshot)
{
    if (snapshot == nil) return nil;
    NSMutableDictionary *value = [@{
        @"service": @(snapshot.service),
        @"identifier": @(snapshot.identifier),
        @"codeText": snapshot.codeText ?: @"",
        @"formattedValue": snapshot.formattedValue ?: @"",
        @"rawHex": snapshot.rawHex ?: @"",
        @"mapped": @(snapshot.isMapped),
        @"numericAvailable": @(snapshot.isNumericValueAvailable),
        @"numericValue": @(snapshot.numericValue)
    } mutableCopy];
    if (snapshot.name.length != 0U) value[@"name"] = snapshot.name;
    if (snapshot.unit.length != 0U) value[@"unit"] = snapshot.unit;
    if (snapshot.rawData.length != 0U) {
        value[@"rawData"] =
            [snapshot.rawData base64EncodedStringWithOptions:0];
    }
    return [value copy];
}

static MBLinkMercedesDataSnapshot *
MBLinkMercedesDataSnapshotFromProfile(NSDictionary *value)
{
    if (![value isKindOfClass:[NSDictionary class]]) return nil;
    NSNumber *service = value[@"service"];
    NSNumber *identifier = value[@"identifier"];
    if (![service isKindOfClass:[NSNumber class]] ||
        ![identifier isKindOfClass:[NSNumber class]] ||
        service.unsignedIntegerValue > UINT8_MAX ||
        identifier.unsignedIntegerValue > UINT16_MAX) {
        return nil;
    }

    MBLinkMercedesDataSnapshot *snapshot =
        [[MBLinkMercedesDataSnapshot alloc] init];
    snapshot.service = (uint8_t)service.unsignedIntegerValue;
    snapshot.identifier = (uint16_t)identifier.unsignedIntegerValue;
    snapshot.codeText = [value[@"codeText"] isKindOfClass:[NSString class]]
        ? value[@"codeText"] : @"";
    snapshot.name = [value[@"name"] isKindOfClass:[NSString class]]
        ? value[@"name"] : nil;
    snapshot.unit = [value[@"unit"] isKindOfClass:[NSString class]]
        ? value[@"unit"] : nil;
    snapshot.formattedValue =
        [value[@"formattedValue"] isKindOfClass:[NSString class]]
            ? value[@"formattedValue"] : @"";
    snapshot.rawHex = [value[@"rawHex"] isKindOfClass:[NSString class]]
        ? value[@"rawHex"] : @"";
    NSString *encoded =
        [value[@"rawData"] isKindOfClass:[NSString class]]
            ? value[@"rawData"] : nil;
    snapshot.rawData = encoded.length != 0U
        ? [[NSData alloc] initWithBase64EncodedString:encoded options:0]
        : [NSData data];
    snapshot.mapped = [value[@"mapped"] boolValue];
    snapshot.numericValueAvailable = [value[@"numericAvailable"] boolValue];
    snapshot.numericValue = [value[@"numericValue"] doubleValue];
    return snapshot;
}

/*
 * Preserve the diagnostic service together with the local identifier all the
 * way from PID Setup to the wire. KWP controllers legitimately mix 0x1A and
 * 0x21 reads; an identifier alone is therefore not a complete command key.
 */
static NSNumber *MBLinkManufacturerCommandToken(
    uint8_t service, uint16_t identifier)
{
    const uint32_t token =
        ((uint32_t)service << 16U) | (uint32_t)identifier;
    return @(token);
}

static BOOL MBLinkDecodeManufacturerCommandToken(
    NSNumber *number, uint8_t *service, uint16_t *identifier)
{
    if (![number isKindOfClass:[NSNumber class]] ||
        service == NULL || identifier == NULL) {
        return NO;
    }
    const NSUInteger candidate = number.unsignedIntegerValue;
    if (candidate > UINT32_MAX) return NO;
    *service = (uint8_t)((candidate >> 16U) & UINT32_C(0xff));
    *identifier = (uint16_t)(candidate & UINT32_C(0xffff));
    return mblink_mercedes_documented_read_is_safe(*service, *identifier);
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
    return link_diagnostic_response_route_preferred(
        candidate.responderCANIdentifier,
        candidate.isExtendedID,
        YES,
        current.responderCANIdentifier,
        current.isExtendedID,
        UINT32_C(0x7e8),
        false);
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

static BOOL
MBLinkMercedesModuleIsTransmissionController(
    const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL ||
        module->kind != MBLINK_MERCEDES_MODULE_TRANSMISSION) {
        return NO;
    }

    if (module->definition != NULL && module->definition->key != NULL &&
        strcmp(module->definition->key, "selector") == 0) {
        return NO;
    }

    if (module->identity_available) {
        NSString *identity =
            [MBLinkStringFromCString(module->identity) uppercaseString];
        if ([identity containsString:@"DIRECT SELECT"] ||
            [identity hasPrefix:@"ISM"] ||
            [identity hasPrefix:@"EWM"] ||
            [identity hasPrefix:@"ESM"]) {
            return NO;
        }
    }
    return YES;
}

static MblinkMercedesTransmissionFamily
MBLinkTransmissionFamilyForModule(const MblinkMercedesModuleScanEntry *module)
{
    MblinkMercedesTransmissionFamily family;

    if (!MBLinkMercedesModuleIsTransmissionController(module)) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN;
    }

    if (module->controller_family != NULL &&
        module->controller_family->key != NULL) {
        family =
            mblink_mercedes_transmission_family_from_controller_family_key(
                module->controller_family->key);
        if (family != MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN)
            return family;
    }

    if (module->identity_available) {
        family = mblink_mercedes_transmission_family_from_identity(
            module->identity);
        if (family != MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN)
            return family;
    }
    if (module->software_number_available) {
        family = mblink_mercedes_transmission_family_from_identity(
            module->software_number);
        if (family != MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN)
            return family;
    }
    if (module->hardware_number_available) {
        family = mblink_mercedes_transmission_family_from_identity(
            module->hardware_number);
        if (family != MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN)
            return family;
    }
    return MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN;
}

/*
 * Exact-route evidence can authorise a safe 21 30 read while family
 * identity is unresolved. A positively identified incompatible family
 * wins over route coincidence; Ultimate NAG52 0x30 is clutch speeds.
 */
static BOOL MBLinkTransmissionModuleSupportsCanonical2130(
    const MblinkMercedesModuleScanEntry *module)
{
    if (!MBLinkMercedesModuleIsTransmissionController(module) ||
        mblink_mercedes_module_scan_entry_protocol(module) !=
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        return NO;
    }

    const MblinkMercedesTransmissionFamily family =
        MBLinkTransmissionFamilyForModule(module);
    return family != MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN &&
        mblink_mercedes_transmission_family_uses_2130_actual_values(family);
}

static uint64_t MBLinkTransmission2130FieldBit(
    NSString *identifier)
{
    if ([identifier isEqualToString:
            @"mercedes.transmission.oil_temperature"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_OIL_TEMPERATURE);
    if ([identifier isEqualToString:
            @"mercedes.transmission.actual_gear"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_ACTUAL_GEAR);
    if ([identifier isEqualToString:
            @"mercedes.transmission.target_gear"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_TARGET_GEAR);
    if ([identifier isEqualToString:
            @"mercedes.transmission.tcc_state"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_TCC_STATE);
    if ([identifier isEqualToString:
            @"mercedes.transmission.recognised_gear"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_RECOGNISED_GEAR);
    if ([identifier isEqualToString:
            @"mercedes.transmission.selector_position"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_SELECTOR_POSITION);
    if ([identifier isEqualToString:
            @"mercedes.transmission.drive_program"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_DRIVE_PROGRAM);
    if ([identifier isEqualToString:
            @"mercedes.transmission.tcc_delta_speed_raw"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_TCC_DELTA_SPEED);
    if ([identifier isEqualToString:
            @"mercedes.transmission.tcc_speed_raw"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_TCC_SPEED);
    if ([identifier isEqualToString:
            @"mercedes.transmission.tcc_pressure_raw"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_TCC_PRESSURE);
    if ([identifier isEqualToString:
            @"mercedes.transmission.engine_torque_signed_raw"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_ENGINE_TORQUE);
    if ([identifier isEqualToString:
            @"mercedes.transmission.converter_torque_signed_raw"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_CONVERTER_TORQUE);
    if ([identifier isEqualToString:
            @"mercedes.transmission.output_speed_raw"])
        return MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(
            MBLINK_MERCEDES_TRANSMISSION_2130_OUTPUT_SPEED);
    return UINT64_C(0);
}

static BOOL MBLinkDecodeTransmissionLive2130(
    const MblinkMercedesModuleScanEntry *module,
    const uint8_t *data,
    size_t dataLength,
    uint64_t fieldMask,
    MblinkMercedesTransmissionLive2130 *decoded)
{
    if (!MBLinkTransmissionModuleSupportsCanonical2130(module) ||
        fieldMask == UINT64_C(0)) {
        return NO;
    }
    const MblinkMercedesTransmissionFamily family =
        MBLinkTransmissionFamilyForModule(module);
    if (family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN)
        return NO;
    return mblink_mercedes_transmission_decode_live_2130_mask_for_family(
        family, data, dataLength, fieldMask, decoded);
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

    NSString *identity = [dictionary[@"identity"] isKindOfClass:[NSString class]]
        ? dictionary[@"identity"] : nil;
    NSString *sparePart = [dictionary[@"sparePart"] isKindOfClass:[NSString class]]
        ? dictionary[@"sparePart"] : nil;
    NSString *software = [dictionary[@"software"] isKindOfClass:[NSString class]]
        ? dictionary[@"software"] : nil;
    NSString *hardware = [dictionary[@"hardware"] isKindOfClass:[NSString class]]
        ? dictionary[@"hardware"] : nil;
    (void)mblink_mercedes_module_scan_resolve_controller(
        tx.unsignedIntValue,
        rx.unsignedIntValue,
        extended.boolValue,
        (MblinkMercedesDiagnosticProtocol)protocol.unsignedIntegerValue,
        identity.UTF8String,
        sparePart.UTF8String,
        software.UTF8String,
        hardware.UTF8String,
        entry);
    NSString *savedFamilyKey =
        [dictionary[@"controllerFamily"] isKindOfClass:[NSString class]]
            ? dictionary[@"controllerFamily"] : nil;
    if (savedFamilyKey.length != 0U) {
        const MblinkMercedesControllerFamilyDefinition *savedFamily =
            mblink_mercedes_controller_family_definition_for_key(
                savedFamilyKey.UTF8String);
        if (savedFamily != NULL &&
            entry->definition != NULL &&
            strcmp(savedFamily->module_key, entry->definition->key) == 0) {
            entry->controller_family = savedFamily;
        }
    }

    const uint32_t maxID = entry->extended_id
        ? UINT32_C(0x1fffffff) : UINT32_C(0x7ff);
    return entry->tx_can_id <= maxID && entry->rx_can_id <= maxID;
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
    LinkDiagnosticFlowConfig flowConfig = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
    /*
     * Resolve the physical vehicle before doing the broader diagnostic work.
     * LINK reads Mode 09 VIN immediately after ELM initialisation, MBLINK then
     * selects/creates the authoritative VIN profile and validates/learns the
     * Mercedes controller map. Full SAE capability/fault context follows.
     */
    flowConfig.manufacturer_extension_after_standard_vin = true;
    flowConfig.manufacturer_extension_after_pid_discovery = false;
    flowConfig.manufacturer_extension_after_standard_dtcs = false;
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
    self = [super
        initWithProductSlug:@"mblink"
        flowConfig:flowConfig
        liveStatusText:@"Live OBD-II and diesel scheduler active"
        simulatedLiveStatusText:
            @"Simulated ELM327 · live OBD-II and diesel data"
        standardVINStatusText:@"Reading standard vehicle VIN"
        simulatedAdapterIdentifier:@"ELM327 v2.3 MBLINK SIM"
        simulatedVIN:@"WDD2073022F123456"];
    if (self == nil) return nil;

    _vehicleProfileStore = [[LinkVehicleProfileStore alloc]
        initWithProductNamespace:@"mblink"
        legacyProfileKey:MBLinkVehicleProfilesDefaultsKey
        legacySelectedVINKey:@"mblink.selectedVehicleVIN.v1"
        legacyAdapterMappingKey:@"mblink.adapterPeripheralByVehicle.v1"];

    [self resetMercedesState];
    return self;
}

- (void)resetMercedesState
{
    self.mercedesProbeStatusText = @"Not attempted";
    self.mercedesProbeEndpointText = nil;
    self.mercedesVINText = nil;
    self.mercedesIdentitySummaryText = @"Not attempted";
    self.mercedesIdentityResults = @[];
    self.mercedesCrd3SummaryText = @"Not attempted";
    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes module scan";
    self.mercedesUDSFaults = @[];
    self.vehicleProfileStatusText = @"Waiting for VIN";
    _mercedesModuleScan = (MblinkMercedesModuleScan){0};
    _moduleScanActive = NO;
    _cachedModuleRefreshActive = NO;
    _startupModuleDiscoveryStarted = NO;
    _cachedVehicleProfile = nil;
    _manufacturerDataScan = (MblinkMercedesDataScan){0};
    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanStatusText = @"Not scanned";
    self.manufacturerDataScanModuleIdentifier = nil;
    _manufacturerDataByModule = [[NSMutableDictionary alloc] init];
    _standardDataByResponder = [[NSMutableDictionary alloc] init];
    _standardDataLatest = [[NSMutableDictionary alloc] init];
    _manufacturerDataForceFullScan = NO;
    _manufacturerDataScanLiveOnly = NO;
    _startupModuleDataPassActive = NO;
    _startupModuleDataIndex = 0U;
    _scheduledManufacturerJobRegistered = NO;
    _scheduledManufacturerJobActive = NO;
    _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
    _selectedManufacturerLiveCommandsByModule =
        [[NSMutableDictionary alloc] init];
    _scheduledManufacturerModuleCursor = 0U;
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
    _moduleScanActive = NO;
    [_shared failWithStatus:status];
}

- (double)displayValueForPID:(uint8_t)pid canonicalValue:(double)value
{
    return [_shared displayValueForPID:pid canonicalValue:value];
}

- (NSString *)displayUnitForPID:(uint8_t)pid
{
    return [_shared displayUnitForPID:pid];
}

- (double)displayTemperatureCelsius:(double)celsius
{
    return [_shared displayTemperatureCelsius:celsius];
}

- (NSString *)displayTemperatureUnit
{
    return _shared.displayTemperatureUnit;
}

- (void)start
{
    [self resetMercedesState];
    [_shared start];
}

- (void)startWithPeripheralIdentifier:(NSString *)peripheralIdentifier
{
    [self resetMercedesState];
    [_shared startWithPeripheralIdentifier:peripheralIdentifier];
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
    _moduleScanActive = NO;
    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanModuleIdentifier = nil;
    _manufacturerDataForceFullScan = NO;
    _manufacturerDataScanLiveOnly = NO;
    _startupModuleDataPassActive = NO;
    _startupModuleDataIndex = 0U;
    _scheduledManufacturerJobRegistered = NO;
    _scheduledManufacturerJobActive = NO;
    _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
    ++_manufacturerDataRequestGeneration;
    [_shared disconnect];
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

- (NSArray<NSString *> *)cachedEngineEvidence
{
    id evidence = _cachedVehicleProfile[@"engineEvidence"];
    return [evidence isKindOfClass:[NSArray class]] ? evidence : @[];
}

- (NSString *)resolvedMercedesModuleNameForRequestCANIdentifier:
        (uint32_t)requestCANIdentifier
    responseCANIdentifier:(uint32_t)responseCANIdentifier
    extendedID:(BOOL)extendedID
    protocol:(NSUInteger)protocol
    identityText:(nullable NSString *)identityText
    partNumber:(nullable NSString *)partNumber
    softwareNumber:(nullable NSString *)softwareNumber
    hardwareNumber:(nullable NSString *)hardwareNumber
{
    MblinkMercedesModuleScanEntry resolved;
    MblinkMercedesDiagnosticProtocol diagnosticProtocol =
        protocol <= (NSUInteger)MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
            ? (MblinkMercedesDiagnosticProtocol)protocol
            : MBLINK_MERCEDES_DIAGNOSTIC_UDS;

    (void)mblink_mercedes_module_scan_resolve_controller(
        requestCANIdentifier,
        responseCANIdentifier,
        extendedID,
        diagnosticProtocol,
        identityText.UTF8String,
        partNumber.UTF8String,
        softwareNumber.UTF8String,
        hardwareNumber.UTF8String,
        &resolved);
    return MBLinkStringFromCString(
        mblink_mercedes_module_scan_module_name(&resolved));
}

- (NSArray<MBLinkMercedesModuleSnapshot *> *)mercedesModuleSnapshots
{
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    NSMutableArray<MBLinkMercedesModuleSnapshot *> *snapshots =
        [[NSMutableArray alloc] initWithCapacity:count];
    NSArray<NSString *> *engineEvidence = [self cachedEngineEvidence];

    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (module == NULL) continue;

        MblinkMercedesModuleScanEntry resolvedModule = *module;
        if (!module->extended_id &&
            module->tx_can_id == UINT32_C(0x7e0) &&
            module->rx_can_id == UINT32_C(0x7e8)) {
            (void)mblink_mercedes_module_scan_resolve_controller(
                module->tx_can_id,
                module->rx_can_id,
                module->extended_id,
                mblink_mercedes_module_scan_entry_protocol(module),
                module->identity_available ? module->identity : NULL,
                module->spare_part_number_available
                    ? module->spare_part_number : NULL,
                module->software_number_available
                    ? module->software_number : NULL,
                module->hardware_number_available
                    ? module->hardware_number : NULL,
                &resolvedModule);
        }

        MBLinkMercedesModuleSnapshot *snapshot =
            [[MBLinkMercedesModuleSnapshot alloc] init];
        snapshot.identifier = MBLinkMercedesModuleIdentifier(module);
        snapshot.name = MBLinkStringFromCString(
            mblink_mercedes_module_scan_module_name(&resolvedModule));
        snapshot.kind = MBLinkStringFromCString(
            mblink_mercedes_module_kind_name(resolvedModule.kind));
        snapshot.protocolName = MBLinkStringFromCString(
            mblink_mercedes_diagnostic_protocol_name(
                mblink_mercedes_module_scan_entry_protocol(&resolvedModule)));
        snapshot.requestCANIdentifier = module->tx_can_id;
        snapshot.responseCANIdentifier = module->rx_can_id;
        snapshot.extendedID = module->extended_id;
        snapshot.designation = resolvedModule.definition != NULL &&
                resolvedModule.definition->component_designation != NULL
            ? MBLinkStringFromCString(
                resolvedModule.definition->component_designation)
            : ([snapshot.name hasPrefix:@"Likely "]
                ? @"Online candidate · not yet C207-confirmed" : @"");
        snapshot.network = resolvedModule.definition != NULL &&
                resolvedModule.definition->network != NULL
            ? MBLinkStringFromCString(resolvedModule.definition->network) : @"";
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
     * Legislated OBD-II responders are intentionally not represented as
     * Mercedes modules. Standard SAE/EOBD live data has its own vehicle-wide
     * PID source in PID Setup; Modules contains manufacturer ECUs only.
     */
    return [snapshots copy];
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
    if (event->became_ready) {
        [self updateScheduledManufacturerLiveJob];
    }
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE) {
        [self persistDiscoveredCapabilities];
        [self notifyDelegate];
        return;
    }
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
        ? @"VIN captured and decoded; Mercedes ECU identity pending"
        : @"VIN captured; Mercedes ECU identity pending";
    [self loadSavedVehicleProfileForVIN:self.mercedesVINText];
    [self notifyDelegate];
}

- (void)linkDiagnosticsControllerBeginManufacturerExtension:
    (LinkDiagnosticsController *)controller
{
    (void)controller;
    [self beginStartupModuleDiscovery];
}

- (void)beginStartupModuleDiscovery
{
    /*
     * Module identification and saved-profile validation are startup-only.
     * They may run once for each connection, before the live scheduler starts.
     * Every startup path that can enter module discovery comes through this
     * single gate.
     */
    if (_startupModuleDiscoveryStarted) {
        (void)[_shared completeManufacturerExtensionRestoringAdapter:NO];
        return;
    }
    _startupModuleDiscoveryStarted = YES;

    if ([self beginCachedVehicleProfileRefresh]) return;
    /*
     * The normal Connect path establishes the fitted-module map first.  The
     * older engine/CRD3 evidence probe is not a prerequisite for module
     * identification and must not insert a module-specific fingerprint sweep
     * ahead of the bounded vehicle census.
     */
    [self beginMercedesModuleScan];
}

- (void)linkDiagnosticsController:(LinkDiagnosticsController *)controller
  beginScheduledManufacturerJob:(uint32_t)token
{
    (void)controller;
    if (token != MBLinkScheduledTransmissionLiveJobToken) {
        [_shared failWithStatus:@"Unknown scheduled Mercedes live job"];
        return;
    }
    [self beginScheduledTransmissionLiveJob];
}

- (void)linkDiagnosticsController:(LinkDiagnosticsController *)controller
  didReceiveManufacturerResponse:(const LinkElm327Response *)response
{
    (void)controller;
    if (_scheduledManufacturerRestoreStage != MBLinkScheduledRestoreNone) {
        [self processScheduledManufacturerRestoreResponse:
            (const MblinkElm327Response *)response];
        return;
    }
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
    [_shared failWithStatus:
        @"Mercedes manufacturer response arrived without an active operation"];
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
        _manufacturerDataForceFullScan = NO;
        _manufacturerDataScanLiveOnly = NO;
        _scheduledManufacturerJobActive = NO;
        _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
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
            @"Mercedes startup module scan interrupted: %@ · partial results retained; no live-mode rescan",
            status];
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"Partial · %zu module routes · %zu Mercedes factory fault record%@ retained · next connection will validate again",
            capturedModules, capturedFaults, capturedFaults == 1U ? @"" : @"s"];
    }
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

- (BOOL)populateCachedModuleEntry:(MblinkMercedesModuleScanEntry *)entry
                    forIdentifier:(NSString *)identifier
{
    if (entry == NULL || identifier.length == 0U ||
        ![_cachedVehicleProfile[@"modules"] isKindOfClass:[NSArray class]]) {
        return NO;
    }

    for (id value in _cachedVehicleProfile[@"modules"]) {
        if (![value isKindOfClass:[NSDictionary class]]) continue;
        MblinkMercedesModuleScanEntry candidate;
        if (!MBLinkPopulateModuleEntryFromProfile(
                (NSDictionary *)value, &candidate)) {
            continue;
        }
        if (![MBLinkMercedesModuleIdentifier(&candidate)
                isEqualToString:identifier]) {
            continue;
        }
        *entry = candidate;
        return YES;
    }
    return NO;
}

- (nullable NSString *)automaticTransmissionTemperatureModuleIdentifier
{
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, index);
        if (!MBLinkMercedesModuleIsTransmissionController(module) ||
            mblink_mercedes_module_scan_entry_protocol(module) !=
                MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
            continue;
        }
        if (MBLinkTransmissionModuleSupportsCanonical2130(module)) {
            return MBLinkMercedesModuleIdentifier(module);
        }
    }
    return nil;
}

- (nullable NSString *)selectedManufacturerModuleIdentifierAdvancing:
    (BOOL)advance
{
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    if (count == 0U) return nil;

    const size_t start = _scheduledManufacturerModuleCursor % count;
    for (size_t offset = 0U; offset < count; ++offset) {
        const size_t index = (start + offset) % count;
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(&_mercedesModuleScan, index);
        if (module == NULL) continue;
        NSString *identifier = MBLinkMercedesModuleIdentifier(module);
        NSSet<NSNumber *> *selected =
            _selectedManufacturerLiveCommandsByModule[identifier];
        if (selected.count == 0U) continue;

        NSMutableSet<NSNumber *> *available = [NSMutableSet setWithArray:
            [self documentedPIDCommandsForModuleIdentifier:identifier]];
        [available intersectSet:selected];
        if (available.count == 0U) continue;

        if (advance)
            _scheduledManufacturerModuleCursor = (index + 1U) % count;
        return identifier;
    }
    return nil;
}

static NSArray<NSNumber *> *MBLinkFilterCommandsBySelection(
    NSArray<NSNumber *> *identifiers,
    NSSet<NSNumber *> *selected)
{
    if (identifiers.count == 0U || selected.count == 0U) return @[];
    NSMutableOrderedSet<NSNumber *> *filtered =
        [[NSMutableOrderedSet alloc] init];
    for (NSNumber *identifier in identifiers) {
        if ([selected containsObject:identifier])
            [filtered addObject:identifier];
    }
    return [filtered array];
}

- (void)updateScheduledManufacturerLiveJob
{
    /*
     * Do not reserve a recurring Mercedes slot during startup discovery.
     * Standard OBD must reach its real live scheduler first; otherwise an
     * already-overdue manufacturer job can become the first LIVE action and
     * hide a broken standard-polling handoff.
     */
    if (!_shared.isActive || !_shared.isReady) return;

    const BOOL shouldEnable =
        [self selectedManufacturerModuleIdentifierAdvancing:NO].length != 0U;

    if (!_scheduledManufacturerJobRegistered) {
        if (!shouldEnable) return;
        if (![_shared registerLiveManufacturerJobWithToken:
                MBLinkScheduledTransmissionLiveJobToken
                intervalMilliseconds:
                    MBLinkScheduledTransmissionLiveIntervalMs
                priority:LINK_SCHEDULER_PRIORITY_HIGH]) {
            self.manufacturerDataScanStatusText =
                @"Could not register Mercedes live job with LINK scheduler";
            [self notifyDelegate];
            return;
        }
        _scheduledManufacturerJobRegistered = YES;
    } else {
        (void)[_shared setLiveManufacturerJobEnabled:shouldEnable
            token:MBLinkScheduledTransmissionLiveJobToken];
    }
}

- (void)beginScheduledTransmissionLiveJob
{
    if (!_shared.isActive) return;

    if (self.manufacturerDataScanActive ||
        self.manufacturerDataScanModuleIdentifier.length != 0U ||
        _moduleScanActive || _cachedModuleRefreshActive) {
        _scheduledManufacturerJobActive = NO;
        (void)[_shared completeManufacturerExtensionRestoringAdapter:NO];
        return;
    }

    NSString *moduleIdentifier =
        [self selectedManufacturerModuleIdentifierAdvancing:YES];
    if (moduleIdentifier.length == 0U) {
        _scheduledManufacturerJobActive = NO;
        if (_scheduledManufacturerJobRegistered) {
            (void)[_shared setLiveManufacturerJobEnabled:NO
                token:MBLinkScheduledTransmissionLiveJobToken];
        }
        (void)[_shared completeManufacturerExtensionRestoringAdapter:NO];
        return;
    }

    const MblinkMercedesModuleScanEntry *module =
        [self moduleEntryForIdentifier:moduleIdentifier];
    NSSet<NSNumber *> *selected =
        _selectedManufacturerLiveCommandsByModule[moduleIdentifier];
    if (module == NULL || selected.count == 0U) {
        _scheduledManufacturerJobActive = NO;
        (void)[_shared completeManufacturerExtensionRestoringAdapter:NO];
        return;
    }

    NSArray<NSNumber *> *documented = MBLinkFilterCommandsBySelection(
        [self documentedPIDCommandsForModuleIdentifier:moduleIdentifier],
        selected);
    if (documented.count == 0U) {
        _scheduledManufacturerJobActive = NO;
        (void)[_shared completeManufacturerExtensionRestoringAdapter:NO];
        [self updateScheduledManufacturerLiveJob];
        return;
    }

    _scheduledManufacturerJobActive = YES;
    [self beginManufacturerDataOperationForModuleIdentifier:
        moduleIdentifier
                                          forceFullScan:NO
                                               liveOnly:YES
                                   candidateCommands:documented];
}

- (void)beginScheduledManufacturerChannelRestore
{
    _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreHeader;
    if (![_shared beginManufacturerCommand:"ATSH7DF" timeout:1000U]) {
        _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
        _scheduledManufacturerJobActive = NO;
        if (![_shared completeManufacturerExtensionRestoringAdapter:YES])
            [_shared failWithStatus:@"Could not restore OBD channel after Mercedes live job"];
    }
}

- (void)processScheduledManufacturerRestoreResponse:
    (const MblinkElm327Response *)response
{
    if (response == NULL || response->result != MBLINK_ELM327_RESULT_OK ||
        !response->ok_seen) {
        _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
        _scheduledManufacturerJobActive = NO;
        if (![_shared completeManufacturerExtensionRestoringAdapter:YES])
            [_shared failWithStatus:@"Mercedes live channel restore failed"];
        return;
    }

    if (_scheduledManufacturerRestoreStage ==
            MBLinkScheduledRestoreHeader) {
        _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreFilter;
        if (![_shared beginManufacturerCommand:"ATAR" timeout:1000U]) {
            _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
            _scheduledManufacturerJobActive = NO;
            if (![_shared completeManufacturerExtensionRestoringAdapter:YES])
                [_shared failWithStatus:@"Could not clear Mercedes receive filter"];
        }
        return;
    }

    _scheduledManufacturerRestoreStage = MBLinkScheduledRestoreNone;
    _scheduledManufacturerJobActive = NO;
    if (![_shared completeManufacturerExtensionRestoringAdapter:NO]) {
        [_shared failWithStatus:
            @"Could not resume standard diagnostics after Mercedes live job"];
        return;
    }
    [self updateScheduledManufacturerLiveJob];
    [self notifyDelegate];
}

- (NSArray<MBLinkMercedesDataSnapshot *> *)
    manufacturerDataSnapshotsForModuleIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return @[];
    NSArray<MBLinkMercedesDataSnapshot *> *values =
        _manufacturerDataByModule[identifier];
    return values != nil ? [values copy] : @[];
}

- (NSArray<MBLinkMercedesDataSnapshot *> *)
    startupDataSnapshotsForModuleIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return @[];

    MblinkMercedesModuleScanEntry cachedModule;
    const MblinkMercedesModuleScanEntry *module =
        [self moduleEntryForIdentifier:identifier];
    MblinkMercedesEcuPack pack;
    if (module == NULL &&
        [self populateCachedModuleEntry:&cachedModule
                          forIdentifier:identifier]) {
        module = &cachedModule;
    }
    if (module == NULL ||
        !mblink_mercedes_ecu_pack_resolve_module(module, &pack)) {
        return @[];
    }

    NSArray<MBLinkMercedesDataSnapshot *> *values =
        _manufacturerDataByModule[identifier] ?: @[];
    if (values.count == 0U) return @[];

    NSMutableArray<MBLinkMercedesDataSnapshot *> *result =
        [[NSMutableArray alloc] init];
    NSMutableSet<NSNumber *> *startupCommands =
        [[NSMutableSet alloc] init];
    size_t cursor = 0U;
    MblinkMercedesEcuDataItem item;
    while (mblink_mercedes_ecu_pack_next_item(
            &pack, MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
            &cursor, &item)) {
        [startupCommands addObject:MBLinkManufacturerCommandToken(
            item.service, item.identifier)];
    }

    for (MBLinkMercedesDataSnapshot *snapshot in values) {
        if ([startupCommands containsObject:MBLinkManufacturerCommandToken(
                snapshot.service, snapshot.identifier)]) {
            [result addObject:snapshot];
        }
    }

    return [result copy];
}


static NSString *MBLinkManufacturerStableKey(
    NSString *moduleIdentifier,
    uint8_t service,
    uint16_t identifier)
{
    return [NSString stringWithFormat:@"mercedes.%@.%02X.%04X",
        moduleIdentifier, (unsigned int)service, (unsigned int)identifier];
}

static void MBLinkAppendManufacturerDefinition(
    NSMutableArray<MBLinkManufacturerPIDDefinitionSnapshot *> *values,
    NSMutableSet<NSString *> *seenWireKeys,
    NSString *stableKey,
    uint8_t service,
    uint16_t identifier,
    NSString *shortName,
    NSString *title,
    NSString *provenance,
    BOOL allowDuplicateWire)
{
    if (values == nil || stableKey.length == 0U || title.length == 0U) return;
    NSString *wireKey = [NSString stringWithFormat:@"%02X:%04X",
        (unsigned int)service, (unsigned int)identifier];
    if (!allowDuplicateWire && [seenWireKeys containsObject:wireKey]) return;

    MBLinkManufacturerPIDDefinitionSnapshot *snapshot =
        [[MBLinkManufacturerPIDDefinitionSnapshot alloc] init];
    snapshot.stableKey = stableKey;
    snapshot.identifier = identifier;
    snapshot.service = service;
    snapshot.shortName = shortName.length != 0U ? shortName :
        [NSString stringWithFormat:@"%02X %04X",
            (unsigned int)service, (unsigned int)identifier];
    snapshot.title = title;
    snapshot.provenance = provenance.length != 0U
        ? provenance : @"MBLINK source-backed catalogue";
    [values addObject:snapshot];
    [seenWireKeys addObject:wireKey];
}

- (NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *)
    documentedDataDefinitionsForModuleIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return @[];
    MblinkMercedesModuleScanEntry cachedModule;
    const MblinkMercedesModuleScanEntry *module =
        [self moduleEntryForIdentifier:identifier];
    MblinkMercedesEcuPack pack;

    if (module == NULL &&
        [self populateCachedModuleEntry:&cachedModule
                          forIdentifier:identifier]) {
        module = &cachedModule;
    }
    if (module == NULL ||
        !mblink_mercedes_ecu_pack_resolve_module(module, &pack)) {
        return @[];
    }

    NSMutableArray<MBLinkManufacturerPIDDefinitionSnapshot *> *values =
        [[NSMutableArray alloc] init];
    NSMutableSet<NSString *> *seenWireKeys = [[NSMutableSet alloc] init];
    size_t cursor = 0U;
    MblinkMercedesEcuDataItem item;

    while (mblink_mercedes_ecu_pack_next_item(
            &pack, MBLINK_MERCEDES_ECU_DATA_USER_POLLING,
            &cursor, &item)) {

        NSString *stableKey = item.stable_key != NULL
            ? MBLinkStringFromCString(item.stable_key)
            : MBLinkManufacturerStableKey(
                identifier, item.service, item.identifier);
        NSString *shortName = item.short_name != NULL
            ? MBLinkStringFromCString(item.short_name)
            : [NSString stringWithFormat:@"%02X %04X",
                (unsigned int)item.service,
                (unsigned int)item.identifier];
        NSString *title = item.name != NULL && item.name[0] != '\0'
            ? MBLinkStringFromCString(item.name)
            : [NSString stringWithFormat:@"Documented %@ read 0x%04X",
                item.service == UINT8_C(0x22) ? @"UDS" : @"KWP",
                (unsigned int)item.identifier];
        NSString *provenance = item.provenance != NULL &&
                               item.provenance[0] != '\0'
            ? MBLinkStringFromCString(item.provenance)
            : @"MBLINK ECU definition pack";
        if (pack.ecu_name != NULL && pack.ecu_name[0] != '\0') {
            provenance = [NSString stringWithFormat:@"%@ · %@",
                provenance, MBLinkStringFromCString(pack.ecu_name)];
        }

        MBLinkAppendManufacturerDefinition(
            values, seenWireKeys,
            stableKey,
            item.service,
            item.identifier,
            shortName,
            title,
            provenance,
            item.allow_duplicate_wire);
    }

    [values sortUsingComparator:^NSComparisonResult(
        MBLinkManufacturerPIDDefinitionSnapshot *left,
        MBLinkManufacturerPIDDefinitionSnapshot *right) {
        if (left.identifier < right.identifier) return NSOrderedAscending;
        if (left.identifier > right.identifier) return NSOrderedDescending;
        return [left.title compare:right.title];
    }];
    return [values copy];
}

- (NSArray<NSNumber *> *)documentedPIDCommandsForModuleIdentifier:
    (NSString *)identifier
{
    /*
     * Documentation defines the catalogue. Preserve service + identifier as
     * one wire-command token because KWP controllers can expose both 0x1A and
     * 0x21 reads. Runtime response never changes catalogue membership.
     */
    NSMutableOrderedSet<NSNumber *> *commands =
        [[NSMutableOrderedSet alloc] init];
    for (MBLinkManufacturerPIDDefinitionSnapshot *definition in
         [self documentedDataDefinitionsForModuleIdentifier:identifier]) {
        [commands addObject:MBLinkManufacturerCommandToken(
            definition.service, definition.identifier)];
    }
    return [[commands array] sortedArrayUsingSelector:@selector(compare:)];
}

- (void)loadSavedVehicleProfileForPIDConfiguration:(NSString *)vin
{
    if (vin.length != 17U || _shared.isActive) return;
    [self loadSavedVehicleProfileForVIN:vin];
}

- (NSArray<NSNumber *> *)
    manufacturerLivePollingCommandsForModuleIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return @[];
    NSSet<NSNumber *> *selected =
        _selectedManufacturerLiveCommandsByModule[identifier];
    if (selected.count == 0U) return @[];
    return [[selected allObjects] sortedArrayUsingSelector:@selector(compare:)];
}

- (void)setManufacturerLivePollingCommands:(NSArray<NSNumber *> *)commands
                        forModuleIdentifier:(NSString *)identifier
{
    if (identifier.length == 0U) return;

    MblinkMercedesModuleScanEntry cachedModule;
    const MblinkMercedesModuleScanEntry *module =
        [self moduleEntryForIdentifier:identifier];
    MblinkMercedesEcuPack pack;
    if (module == NULL &&
        [self populateCachedModuleEntry:&cachedModule
                          forIdentifier:identifier]) {
        module = &cachedModule;
    }
    const BOOL hasPack = module != NULL &&
        mblink_mercedes_ecu_pack_resolve_module(module, &pack);

    NSMutableSet<NSNumber *> *valid = [[NSMutableSet alloc] init];
    NSMutableSet<NSNumber *> *pollingCommands = [[NSMutableSet alloc] init];
    if (hasPack) {
        size_t cursor = 0U;
        MblinkMercedesEcuDataItem item;
        while (mblink_mercedes_ecu_pack_next_item(
                &pack, MBLINK_MERCEDES_ECU_DATA_USER_POLLING,
                &cursor, &item)) {
            [pollingCommands addObject:MBLinkManufacturerCommandToken(
                item.service, item.identifier)];
        }
    }

    for (NSNumber *number in commands ?: @[]) {
        uint8_t service = 0U;
        uint16_t localIdentifier = 0U;
        if (!MBLinkDecodeManufacturerCommandToken(
                number, &service, &localIdentifier)) {
            continue;
        }
        NSNumber *command = MBLinkManufacturerCommandToken(
            service, localIdentifier);
        if ([pollingCommands containsObject:command]) {
            [valid addObject:command];
        }
    }
    if (valid.count == 0U) {
        [_selectedManufacturerLiveCommandsByModule removeObjectForKey:identifier];
    } else {
        _selectedManufacturerLiveCommandsByModule[identifier] = [valid copy];
    }
    [self updateScheduledManufacturerLiveJob];
    [self notifyDelegate];
}

- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots
{
    return [self transmissionLiveValueSnapshotsForIdentifiers:@[
        @"mercedes.transmission.oil_temperature",
        @"mercedes.transmission.actual_gear",
        @"mercedes.transmission.target_gear",
        @"mercedes.transmission.tcc_state",
        @"mercedes.transmission.recognised_gear",
        @"mercedes.transmission.selector_position",
        @"mercedes.transmission.drive_program",
        @"mercedes.transmission.tcc_delta_speed_raw",
        @"mercedes.transmission.tcc_speed_raw",
        @"mercedes.transmission.tcc_pressure_raw",
        @"mercedes.transmission.engine_torque_signed_raw",
        @"mercedes.transmission.converter_torque_signed_raw",
        @"mercedes.transmission.output_speed_raw"
    ]];
}

- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)
    transmissionLiveValueSnapshotsForIdentifiers:(NSArray<NSString *> *)identifiers
{
    const MblinkMercedesModuleScanEntry *module = NULL;
    NSString *moduleIdentifier = nil;
    const size_t count =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesModuleScanEntry *candidate =
            mblink_mercedes_module_scan_module_at(&_mercedesModuleScan, index);
        if (candidate == NULL || candidate->extended_id ||
            candidate->tx_can_id != UINT32_C(0x7e1) ||
            candidate->rx_can_id != UINT32_C(0x7e9) ||
            mblink_mercedes_module_scan_entry_protocol(candidate) !=
                MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) continue;
        module = candidate;
        moduleIdentifier = MBLinkMercedesModuleIdentifier(candidate);
        break;
    }
    if (module == NULL || moduleIdentifier.length == 0U) return @[];

    MBLinkMercedesDataSnapshot *rli30 = nil;
    for (MBLinkMercedesDataSnapshot *candidate in
         _manufacturerDataByModule[moduleIdentifier] ?: @[]) {
        if (candidate.service == UINT8_C(0x21) &&
            candidate.identifier == UINT16_C(0x30) &&
            candidate.rawData.length != 0U) {
            rli30 = candidate;
            break;
        }
    }
    if (rli30 == nil) return @[];

    if (!MBLinkTransmissionModuleSupportsCanonical2130(module)) return @[];

    uint64_t fieldMask = UINT64_C(0);
    for (NSString *identifier in identifiers ?: @[]) {
        fieldMask |= MBLinkTransmission2130FieldBit(identifier);
    }
    if (fieldMask == UINT64_C(0)) return @[];

    MblinkMercedesTransmissionLive2130 decoded;
    if (!MBLinkDecodeTransmissionLive2130(
            module, (const uint8_t *)rli30.rawData.bytes,
            rli30.rawData.length, fieldMask, &decoded)) return @[];

    const MblinkMercedesTransmissionFamily family =
        MBLinkTransmissionFamilyForModule(module);
    NSString *qualification = family ==
            MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN
        ? @"family unresolved · exact-route vehicle-positive evidence"
        : [NSString stringWithFormat:@"%@ family-qualified",
            MBLinkStringFromCString(
                mblink_mercedes_transmission_family_name(family))];
    NSString *quality = [NSString stringWithFormat:
        @"Mercedes GS 21 30 %@ response · portable MBLINK decoder · %@",
        decoded.rich_layout ? @"rich" : @"compact", qualification];
    NSMutableArray<MBLinkTransmissionLiveValueSnapshot *> *values =
        [[NSMutableArray alloc] init];

    if (decoded.oil_temperature_available) {
        const double display =
            [_shared displayTemperatureCelsius:decoded.oil_temperature_c];
        NSString *unit = _shared.displayTemperatureUnit;
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.oil_temperature";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"ATF";
        value.title = @"Transmission oil temperature";
        value.suffix = [@" " stringByAppendingString:unit];
        value.formattedValue = [NSString stringWithFormat:@"%.6g %@", display, unit];
        value.numericValueAvailable = YES;
        value.numericValue = display;
        value.rawHex = rli30.rawHex;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.actual_gear_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.actual_gear";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"GEAR";
        value.title = @"Current gear";
        value.suffix = @"";
        value.formattedValue = MBLinkStringFromCString(
            mblink_mercedes_transmission_actual_gear_name(
                decoded.actual_gear_code));
        value.numericValueAvailable = NO;
        value.rawHex = rli30.rawHex;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.target_gear_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.target_gear";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"TARGET";
        value.title = @"Target gear";
        value.suffix = @"";
        value.formattedValue = MBLinkStringFromCString(
            mblink_mercedes_transmission_target_gear_name(
                decoded.target_gear_code));
        value.numericValueAvailable = NO;
        value.rawHex = rli30.rawHex;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.tcc_status_available &&
        family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53) {
        const char *name =
            mblink_mercedes_transmission_egs53_rli30_tcc_state_name(
                decoded.tcc_status_code);
        if (name != NULL) {
            MBLinkTransmissionLiveValueSnapshot *value =
                [[MBLinkTransmissionLiveValueSnapshot alloc] init];
            value.identifier = @"mercedes.transmission.tcc_state";
            value.localIdentifier = UINT16_C(0x30);
            value.shortName = @"TCC";
            value.title = @"Torque converter clutch state";
            value.suffix = @"";
            value.formattedValue = MBLinkStringFromCString(name);
            value.numericValueAvailable = NO;
            value.rawHex = rli30.rawHex;
            value.qualityNote = quality;
            [values addObject:value];
        }
    }

    if (decoded.recognised_gear_available &&
        family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53) {
        const char *name =
            mblink_mercedes_transmission_egs53_rli30_recognised_gear_name(
                decoded.recognised_gear_code);
        if (name != NULL) {
            MBLinkTransmissionLiveValueSnapshot *value =
                [[MBLinkTransmissionLiveValueSnapshot alloc] init];
            value.identifier = @"mercedes.transmission.recognised_gear";
            value.localIdentifier = UINT16_C(0x30);
            value.shortName = @"RECOG";
            value.title = @"Recognised transmission gear";
            value.suffix = @"";
            value.formattedValue = MBLinkStringFromCString(name);
            value.numericValueAvailable = NO;
            value.rawHex = rli30.rawHex;
            value.qualityNote = quality;
            [values addObject:value];
        }
    }

    if (decoded.selector_position_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.selector_position";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"SELECT";
        value.title = @"Selector position";
        const char *selectorName =
            family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53
                ? mblink_mercedes_transmission_egs53_rli30_selector_name(
                    decoded.selector_position_code)
                : NULL;
        value.suffix = selectorName != NULL ? @"" : @" raw";
        value.formattedValue = selectorName != NULL
            ? MBLinkStringFromCString(selectorName)
            : [NSString stringWithFormat:@"%u raw",
                (unsigned int)decoded.selector_position_code];
        value.numericValueAvailable = selectorName == NULL;
        value.numericValue = (double)decoded.selector_position_code;
        value.rawHex = rli30.rawHex;
        value.qualityNote = quality;
        [values addObject:value];
    }

    if (decoded.drive_program_available) {
        MBLinkTransmissionLiveValueSnapshot *value =
            [[MBLinkTransmissionLiveValueSnapshot alloc] init];
        value.identifier = @"mercedes.transmission.drive_program";
        value.localIdentifier = UINT16_C(0x30);
        value.shortName = @"PROGRAM";
        value.title = @"Transmission drive program";
        const char *programName =
            family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53
                ? mblink_mercedes_transmission_egs53_rli30_program_name(
                    decoded.drive_program_code)
                : NULL;
        value.suffix = programName != NULL ? @"" : @" raw";
        value.formattedValue = programName != NULL
            ? MBLinkStringFromCString(programName)
            : [NSString stringWithFormat:@"%u raw",
                (unsigned int)decoded.drive_program_code];
        value.numericValueAvailable = programName == NULL;
        value.numericValue = (double)decoded.drive_program_code;
        value.rawHex = rli30.rawHex;
        value.qualityNote = quality;
        [values addObject:value];
    }

#define APPEND_RLI30_UNSIGNED(KEY, SHORT, TITLE, AVAILABLE, FIELD) do { \
        if ((AVAILABLE)) { \
            MBLinkTransmissionLiveValueSnapshot *value = \
                [[MBLinkTransmissionLiveValueSnapshot alloc] init]; \
            value.identifier = (KEY); \
            value.localIdentifier = UINT16_C(0x30); \
            value.shortName = (SHORT); \
            value.title = (TITLE); \
            value.suffix = @" raw"; \
            value.formattedValue = [NSString stringWithFormat:@"%u raw", \
                (unsigned int)(FIELD)]; \
            value.numericValueAvailable = YES; \
            value.numericValue = (double)(FIELD); \
            value.rawHex = rli30.rawHex; \
            value.qualityNote = quality; \
            [values addObject:value]; \
        } \
    } while (0)

#define APPEND_RLI30_SIGNED(KEY, SHORT, TITLE, AVAILABLE, FIELD) do { \
        if ((AVAILABLE)) { \
            MBLinkTransmissionLiveValueSnapshot *value = \
                [[MBLinkTransmissionLiveValueSnapshot alloc] init]; \
            value.identifier = (KEY); \
            value.localIdentifier = UINT16_C(0x30); \
            value.shortName = (SHORT); \
            value.title = (TITLE); \
            value.suffix = @" signed raw"; \
            value.formattedValue = [NSString stringWithFormat:@"%d signed raw", \
                (int)(FIELD)]; \
            value.numericValueAvailable = YES; \
            value.numericValue = (double)(FIELD); \
            value.rawHex = rli30.rawHex; \
            value.qualityNote = quality; \
            [values addObject:value]; \
        } \
    } while (0)

    APPEND_RLI30_UNSIGNED(
        @"mercedes.transmission.tcc_delta_speed_raw", @"TCC Δ",
        @"Torque converter delta speed",
        decoded.tcc_delta_speed_available, decoded.tcc_delta_speed_raw);
    APPEND_RLI30_UNSIGNED(
        @"mercedes.transmission.tcc_speed_raw", @"TCC SPD",
        @"Torque converter speed",
        decoded.tcc_speed_available, decoded.tcc_speed_raw);
    APPEND_RLI30_UNSIGNED(
        @"mercedes.transmission.tcc_pressure_raw", @"TCC P",
        @"Torque converter pressure",
        decoded.tcc_pressure_available, decoded.tcc_pressure_raw);
    APPEND_RLI30_SIGNED(
        @"mercedes.transmission.engine_torque_signed_raw", @"ENG TQ",
        @"Engine torque",
        decoded.engine_torque_available, decoded.engine_torque_signed_raw);
    APPEND_RLI30_SIGNED(
        @"mercedes.transmission.converter_torque_signed_raw", @"CONV TQ",
        @"Converter torque",
        decoded.converter_torque_available,
        decoded.converter_torque_signed_raw);
    APPEND_RLI30_UNSIGNED(
        @"mercedes.transmission.output_speed_raw", @"OUT SPD",
        @"Transmission output speed",
        decoded.output_speed_available, decoded.output_speed_raw);

#undef APPEND_RLI30_SIGNED
#undef APPEND_RLI30_UNSIGNED

    if (rli30.isStale) {
        for (MBLinkTransmissionLiveValueSnapshot *value in values) {
            value.formattedValue = [@"Stale · " stringByAppendingString:value.formattedValue];
            value.numericValueAvailable = NO;
            value.qualityNote = @"Latest live refresh failed; showing the last received value";
        }
    }
    return [values copy];
}

- (void)discoverManufacturerDataForModuleIdentifier:(NSString *)identifier
{
    [self beginManufacturerDataScanForModuleIdentifier:identifier
                                         forceFullScan:NO];
}

- (void)rescanManufacturerDataForModuleIdentifier:(NSString *)identifier
{
    [self beginManufacturerDataScanForModuleIdentifier:identifier
                                         forceFullScan:NO];
}

- (void)beginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                        forceFullScan:(BOOL)forceFullScan
{
    [self beginManufacturerDataOperationForModuleIdentifier:identifier
                                              forceFullScan:forceFullScan
                                                   liveOnly:NO
                                       candidateCommands:nil];
}

- (void)beginManufacturerDataOperationForModuleIdentifier:
            (NSString *)identifier
                                               forceFullScan:(BOOL)forceFullScan
                                                    liveOnly:(BOOL)liveOnly
                                        candidateCommands:
            (nullable NSArray<NSNumber *> *)candidateCommands
{
    if (identifier.length == 0U || !_shared.isActive) return;

    if (self.manufacturerDataScanActive ||
        self.manufacturerDataScanModuleIdentifier.length != 0U) {
        self.manufacturerDataScanStatusText =
            [self.manufacturerDataScanModuleIdentifier
                isEqualToString:identifier]
                ? @"This module scan is already in progress"
                : @"Another module scan is already in progress";
        [self notifyDelegate];
        return;
    }

    /*
     * Manual refresh is deliberately non-destructive and re-reads identifiers
     * already proven positive. A full rescan can still search the bounded safe
     * range. The automatic live path is narrower again: it uses only proven
     * runtime identifiers or a one-shot controller-family candidate list.
     */
    _manufacturerDataForceFullScan = forceFullScan;
    _manufacturerDataScanLiveOnly = liveOnly;
    const NSUInteger generation = ++_manufacturerDataRequestGeneration;
    self.manufacturerDataScanModuleIdentifier = identifier;
    self.manufacturerDataScanStatusText = liveOnly
        ? @"Waiting for a safe gap to refresh selected documented Mercedes data"
        : @"Waiting for a safe gap to read documented ECU data";
    [self notifyDelegate];

    [self tryBeginManufacturerDataScanForModuleIdentifier:identifier
                                               generation:generation
                                                  attempt:0U
                                                 liveOnly:liveOnly
                                     candidateCommands:candidateCommands];
}

- (void)tryBeginManufacturerDataScanForModuleIdentifier:(NSString *)identifier
                                             generation:(NSUInteger)generation
                                                attempt:(NSUInteger)attempt
                                               liveOnly:(BOOL)liveOnly
                                   candidateCommands:
            (nullable NSArray<NSNumber *> *)candidateCommands
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
        _manufacturerDataScanLiveOnly = NO;
        [self notifyDelegate];
        if (liveOnly && _scheduledManufacturerJobActive) {
            _scheduledManufacturerJobActive = NO;
            (void)[_shared completeManufacturerExtensionRestoringAdapter:NO];
        }
        return;
    }

    if (!_scheduledManufacturerJobActive &&
        ![_shared beginLiveManufacturerExtension]) {
        if (attempt < 80U) {
            dispatch_after(
                dispatch_time(
                    DISPATCH_TIME_NOW,
                    (int64_t)UINT64_C(125) * NSEC_PER_MSEC),
                dispatch_get_main_queue(), ^{
                    [self tryBeginManufacturerDataScanForModuleIdentifier:
                        identifier
                        generation:generation
                        attempt:attempt + 1U
                        liveOnly:liveOnly
                        candidateCommands:candidateCommands];
                });
            return;
        }
        self.manufacturerDataScanStatusText =
            @"Could not pause standard live polling for the documented Mercedes read";
        self.manufacturerDataScanModuleIdentifier = nil;
        _manufacturerDataForceFullScan = NO;
        _manufacturerDataScanLiveOnly = NO;
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
    MblinkMercedesEcuPack ecuPack;
    const BOOL hasEcuPack =
        mblink_mercedes_ecu_pack_resolve_module(module, &ecuPack);
    const size_t ecuPackItemCount = hasEcuPack
        ? mblink_mercedes_ecu_pack_data_item_count(&ecuPack) : 0U;

    MblinkMercedesDataScanResult result =
        MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;

    if (candidateCommands.count != 0U) {
        /*
         * iPhone polling receives only commands selected from the exact
         * identified ECU catalogue. Keep the documented service with the
         * identifier so KWP 1A xx can never be rewritten as 21 xx.
         */
        MblinkMercedesDataProbeCommand
            commands[MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS];
        size_t commandCount = 0U;
        for (NSNumber *number in candidateCommands) {
            uint8_t service = 0U;
            uint16_t localIdentifier = 0U;
            if (commandCount >= MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS)
                break;
            if (!MBLinkDecodeManufacturerCommandToken(
                    number, &service, &localIdentifier)) {
                continue;
            }
            commands[commandCount].service = service;
            commands[commandCount].identifier = localIdentifier;
            ++commandCount;
        }
        if (commandCount != 0U) {
            result = mblink_mercedes_data_scan_begin_documented_commands(
                &_manufacturerDataScan, &config,
                commands, commandCount);
        }
    } else if (!liveOnly && ecuPackItemCount != 0U) {
        /*
         * Factory Readings issues only source-backed commands belonging to the
         * specifically identified controller. It never searches a PID range.
         */
        MblinkMercedesDataProbeCommand
            commands[MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS];
        size_t commandCount = 0U;

#define APPEND_READ_COMMAND(SERVICE, IDENTIFIER) do { \
        const uint8_t appendService = (SERVICE); \
        const uint16_t appendIdentifier = (IDENTIFIER); \
        BOOL duplicate = NO; \
        for (size_t seenIndex = 0U; seenIndex < commandCount; ++seenIndex) { \
            if (commands[seenIndex].service == appendService && \
                commands[seenIndex].identifier == appendIdentifier) { \
                duplicate = YES; \
                break; \
            } \
        } \
        if (!duplicate && \
            commandCount < MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS && \
            mblink_mercedes_documented_read_is_safe( \
                appendService, appendIdentifier)) { \
            commands[commandCount].service = appendService; \
            commands[commandCount].identifier = appendIdentifier; \
            ++commandCount; \
        } \
    } while (0)

        for (size_t itemIndex = 0U;
             itemIndex < ecuPackItemCount; ++itemIndex) {
            MblinkMercedesEcuDataItem item;
            if (!mblink_mercedes_ecu_pack_data_item_at(
                    &ecuPack, itemIndex, &item)) {
                continue;
            }
            APPEND_READ_COMMAND(item.service, item.identifier);
        }

#undef APPEND_READ_COMMAND

        if (commandCount != 0U) {
            result = mblink_mercedes_data_scan_begin_documented_commands(
                &_manufacturerDataScan, &config,
                commands, commandCount);
        }
    }

    /*
     * Live refresh never discovers anything. If there is no selected,
     * documented identifier to read, restore the shared SAE channel.
     */
    if (liveOnly &&
        result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK) {
        const BOOL scheduled = _scheduledManufacturerJobActive;
        _scheduledManufacturerJobActive = NO;
        (void)[_shared completeManufacturerExtensionRestoringAdapter:
            scheduled ? NO : YES];
        self.manufacturerDataScanStatusText =
            @"No selected documented Mercedes PIDs remain for this module";
        self.manufacturerDataScanModuleIdentifier = nil;
        _manufacturerDataForceFullScan = NO;
        _manufacturerDataScanLiveOnly = NO;
        ++_manufacturerDataRequestGeneration;
        [self updateScheduledManufacturerLiveJob];
        [self notifyDelegate];
        return;
    }

    if (result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK) {
        (void)[_shared completeManufacturerExtensionRestoringAdapter:YES];
        self.manufacturerDataScanStatusText = [NSString stringWithFormat:
            @"Documented Mercedes data read could not start: %@",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_result_name(result))];
        self.manufacturerDataScanModuleIdentifier = nil;
        _manufacturerDataForceFullScan = NO;
        _manufacturerDataScanLiveOnly = NO;
        [self notifyDelegate];
        return;
    }

    self.manufacturerDataScanActive = YES;
    if (liveOnly) {
        const size_t count = _manufacturerDataScan.identifier_list_active
            ? _manufacturerDataScan.identifier_count : 1U;
        self.manufacturerDataScanStatusText = [NSString stringWithFormat:
            @"Live Mercedes documented refresh · %zu ID%@ · %@",
            count, count == 1U ? @"" : @"s",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_stage_name(
                    _manufacturerDataScan.stage))];
    } else {
        self.manufacturerDataScanStatusText = [NSString stringWithFormat:
            @"Reading documented Mercedes ECU data · %@",
            MBLinkStringFromCString(
                mblink_mercedes_data_scan_stage_name(
                    _manufacturerDataScan.stage))];
    }
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
            @"Documented Mercedes data command could not be sent"];
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
        snapshot.rawData = [NSData dataWithBytes:record->data
                                         length:record->data_length];

        double numeric = 0.0;
        const char *numericName = NULL;
        const char *unit = NULL;
        char structured[1024];
        const char *structuredName = NULL;
        const MblinkMercedesTransmissionFamily transmissionFamily =
            MBLinkTransmissionFamilyForModule(module);
        const BOOL transmissionMetadata =
            module->kind == MBLINK_MERCEDES_MODULE_TRANSMISSION &&
            record->service ==
                MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER &&
            record->identifier >= UINT16_C(0x00e0) &&
            record->identifier <= UINT16_C(0x00eb);
        const BOOL allowOemTransmissionValueDecode =
            module->kind != MBLINK_MERCEDES_MODULE_TRANSMISSION ||
            transmissionMetadata ||
            transmissionFamily == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52 ||
            transmissionFamily == MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2;
        const BOOL numericMapped =
            allowOemTransmissionValueDecode &&
            mblink_mercedes_data_record_decode_known_numeric_for_route(
                module->tx_can_id,
                module->rx_can_id,
                module->extended_id,
                module->kind,
                record, &numeric, &numericName, &unit);
        BOOL structuredMapped = NO;
        const BOOL egs53VariantCoding =
            transmissionFamily ==
                MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53 &&
            record->service ==
                MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER &&
            record->identifier == UINT16_C(0x00b1);
        if (egs53VariantCoding) {
            structuredMapped =
                mblink_mercedes_transmission_format_egs53_variant_coding(
                    record->data, record->data_length,
                    structured, sizeof(structured));
            if (structuredMapped)
                structuredName = "EGS53 variant / SCN coding";
        } else if (allowOemTransmissionValueDecode) {
            structuredMapped =
                mblink_mercedes_data_record_format_known_for_route(
                    module->tx_can_id,
                    module->rx_can_id,
                    module->extended_id,
                    module->kind,
                    record, structured, sizeof(structured),
                    &structuredName);
        }
        const char *profileName =
            MBLinkMercedesModuleIsTransmissionController(module) &&
            record->service ==
                MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER
                ? mblink_mercedes_transmission_kwp_read_identifier_name_for_family(
                    transmissionFamily, (uint8_t)record->identifier)
                : mblink_mercedes_documented_read_name(
                    record->service, record->identifier);

        if (numericMapped || structuredMapped) {
            snapshot.mapped = YES;
            snapshot.numericValueAvailable = numericMapped;
            snapshot.numericValue = numericMapped ? numeric : 0.0;
            snapshot.name = MBLinkStringFromCString(
                structuredName != NULL ? structuredName : numericName);
            snapshot.unit = numericMapped
                ? MBLinkStringFromCString(unit) : nil;
            if (structuredMapped) {
                snapshot.formattedValue =
                    MBLinkStringFromCString(structured);
            } else {
                snapshot.formattedValue =
                    [snapshot.unit isEqualToString:@"°C"]
                        ? [NSString stringWithFormat:@"%.1f %@", numeric, snapshot.unit]
                        : [NSString stringWithFormat:@"%.3f %@", numeric, snapshot.unit];
            }
        } else {
            snapshot.mapped = NO;
            snapshot.numericValueAvailable = NO;
            snapshot.numericValue = 0.0;
            snapshot.name = profileName != NULL
                ? MBLinkStringFromCString(profileName) : nil;
            snapshot.unit = nil;
            snapshot.formattedValue = [NSString stringWithFormat:
                @"RAW %@", snapshot.rawHex];
        }
        [values addObject:snapshot];
    }

    NSMutableSet<NSNumber *> *requestedLiveCommands = [[NSMutableSet alloc] init];
    if (_manufacturerDataScanLiveOnly && _manufacturerDataScan.identifier_list_active) {
        for (size_t index = 0U; index < _manufacturerDataScan.identifier_count; ++index) {
            [requestedLiveCommands addObject:MBLinkManufacturerCommandToken(
                _manufacturerDataScan.services[index],
                _manufacturerDataScan.identifiers[index])];
        }
    }
    _manufacturerDataByModule[identifier] = MBLinkMergeMercedesDataSnapshots(
        _manufacturerDataByModule[identifier] ?: @[], values, requestedLiveCommands);
}

- (void)finishManufacturerDataScanWithStatus:(NSString *)status
{
    [self publishManufacturerDataScanResults];
    /*
     * Retain documented-read results against the VIN/module profile so later
     * sessions can show prior evidence without performing PID discovery.
     */
    [self saveCurrentVehicleProfile];

    const size_t positive =
        mblink_mercedes_data_scan_record_count(&_manufacturerDataScan);
    const size_t attempted = _manufacturerDataScan.attempted_count;
    NSString *module = self.manufacturerDataScanModuleIdentifier ?: @"Module";
    const MblinkMercedesModuleScanEntry *finishedModule =
        [self moduleEntryForIdentifier:
            self.manufacturerDataScanModuleIdentifier];
    const NSUInteger retained =
        _manufacturerDataByModule[module].count;
    self.manufacturerDataScanStatusText = [NSString stringWithFormat:
        @"%@ · %@ · %zu checked · %zu responded this pass · %lu retained",
        module, status, attempted, positive, (unsigned long)retained];

    const BOOL scheduledLive =
        _scheduledManufacturerJobActive && _manufacturerDataScanLiveOnly;
    const BOOL fastCanRestore =
        scheduledLive && finishedModule != NULL &&
        !finishedModule->extended_id &&
        (mblink_mercedes_module_scan_entry_protocol(finishedModule) ==
             MBLINK_MERCEDES_DIAGNOSTIC_UDS ||
         mblink_mercedes_module_scan_entry_protocol(finishedModule) ==
             MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);

    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanModuleIdentifier = nil;
    _manufacturerDataForceFullScan = NO;
    _manufacturerDataScanLiveOnly = NO;
    ++_manufacturerDataRequestGeneration;

    if (_startupModuleDataPassActive) {
        if ([self beginNextStartupModuleDataRead]) {
            [self notifyDelegate];
            return;
        }
        _startupModuleDataPassActive = NO;
        _scheduledManufacturerJobActive = NO;
        if (![_shared completeManufacturerExtensionRestoringAdapter:YES]) {
            [_shared failWithStatus:
                @"Could not resume standard diagnostics after startup module data"];
        }
        [self updateScheduledManufacturerLiveJob];
        [self notifyDelegate];
        return;
    }

    if (fastCanRestore) {
        [self beginScheduledManufacturerChannelRestore];
        [self notifyDelegate];
        return;
    }

    _scheduledManufacturerJobActive = NO;
    if (![_shared completeManufacturerExtensionRestoringAdapter:YES]) {
        [_shared failWithStatus:
            @"Could not resume standard diagnostics after Mercedes data scan"];
    }
    [self updateScheduledManufacturerLiveJob];
    [self notifyDelegate];
}

- (void)beginMercedesModuleScan
{
    if (_shared.isSimulated) {
        memset(&_mercedesModuleScan, 0, sizeof(_mercedesModuleScan));
        _mercedesModuleScan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE;
        _mercedesModuleScan.scope = MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS;
        _mercedesModuleScan.module_count = 4U;

        MblinkMercedesModuleScanEntry *module = &_mercedesModuleScan.modules[0];
        module->tx_can_id = UINT32_C(0x7e0);
        module->rx_can_id = UINT32_C(0x7e8);
        module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_UDS;
        module->kind = MBLINK_MERCEDES_MODULE_ENGINE;
        module->tester_present_response = true;
        (void)snprintf(
            module->identity, sizeof(module->identity), "%s", "CRD3-SIM");
        module->identity_available = true;
        mblink_mercedes_module_scan_classify_identity(module);
        module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE;

        module = &_mercedesModuleScan.modules[1];
        module->tx_can_id = UINT32_C(0x7e1);
        module->rx_can_id = UINT32_C(0x7e9);
        module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
        module->kind = MBLINK_MERCEDES_MODULE_TRANSMISSION;
        module->tester_present_response = true;
        module->definition =
            mblink_mercedes_module_definition_for_key("transmission-vgs");
        module->identification_status =
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        (void)snprintf(
            module->identity, sizeof(module->identity), "%s", "VGS3_0402-SIM");
        module->identity_available = true;
        mblink_mercedes_module_scan_classify_identity(module);

        module = &_mercedesModuleScan.modules[2];
        module->tx_can_id = UINT32_C(0x632);
        module->rx_can_id = UINT32_C(0x486);
        module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_UDS;
        module->kind = MBLINK_MERCEDES_MODULE_ABS_ESP;
        module->tester_present_response = true;
        module->definition = mblink_mercedes_module_definition_for_key("esp");
        module->identification_status =
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        (void)snprintf(
            module->identity, sizeof(module->identity), "%s", "ESP212-SIM");
        module->identity_available = true;
        mblink_mercedes_module_scan_classify_identity(module);

        module = &_mercedesModuleScan.modules[3];
        module->tx_can_id = UINT32_C(0x64a);
        module->rx_can_id = UINT32_C(0x489);
        module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
        module->kind = MBLINK_MERCEDES_MODULE_RESTRAINTS;
        module->tester_present_response = true;
        module->definition =
            mblink_mercedes_module_definition_for_key("restraints-orc");
        module->identification_status =
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        (void)snprintf(
            module->identity, sizeof(module->identity), "%s", "ORC_212-SIM");
        module->identity_available = true;
        mblink_mercedes_module_scan_classify_identity(module);

        [self updateMercedesModuleScanSummary];
        [self saveCurrentVehicleProfile];
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
        @"Mercedes first-VIN module identification · 57 exact routes";
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
    if (![_shared beginManufacturerCommand:command
                                  timeout:mblink_mercedes_module_scan_timeout_ms(
                                      &_mercedesModuleScan)]) {
        _moduleScanActive = NO;
        [self updateMercedesModuleFaultEvidenceInProgress];
        [self updateMercedesModuleScanSummary];
        self.mercedesProbeStatusText =
            @"Mercedes module scan command could not be sent; responses already captured were retained";
        [self notifyDelegate];
        [self finishMercedesExtensionRestoringAdapter:YES];
    }
}

- (BOOL)beginNextStartupModuleDataRead
{
    const size_t moduleCount =
        mblink_mercedes_module_scan_module_count(&_mercedesModuleScan);

    while (_startupModuleDataIndex < moduleCount) {
        const MblinkMercedesModuleScanEntry *module =
            mblink_mercedes_module_scan_module_at(
                &_mercedesModuleScan, _startupModuleDataIndex++);
        MblinkMercedesEcuPack pack;
        MblinkMercedesDataProbeCommand
            commands[MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS];
        size_t commandCount = 0U;

        if (module == NULL ||
            !mblink_mercedes_ecu_pack_resolve_module(module, &pack)) {
            continue;
        }

        size_t cursor = 0U;
        MblinkMercedesEcuDataItem item;
        while (mblink_mercedes_ecu_pack_next_item(
                &pack, MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
                &cursor, &item)) {
            BOOL duplicate = NO;

            if (item.acquired_during_identification ||
                !mblink_mercedes_documented_read_is_safe(
                    item.service, item.identifier)) {
                continue;
            }

            for (size_t seen = 0U; seen < commandCount; ++seen) {
                if (commands[seen].service == item.service &&
                    commands[seen].identifier == item.identifier) {
                    duplicate = YES;
                    break;
                }
            }
            if (duplicate ||
                commandCount >= MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS) {
                continue;
            }

            commands[commandCount].service = item.service;
            commands[commandCount].identifier = item.identifier;
            ++commandCount;
        }

        if (commandCount == 0U) continue;

        MblinkMercedesDataScanConfig config =
            mblink_mercedes_data_scan_default_config(
                module->tx_can_id, module->rx_can_id,
                module->extended_id,
                mblink_mercedes_module_scan_entry_protocol(module),
                module->kind);
        if (mblink_mercedes_data_scan_begin_documented_commands(
                &_manufacturerDataScan, &config,
                commands, commandCount) !=
            MBLINK_MERCEDES_DATA_SCAN_RESULT_OK) {
            continue;
        }

        self.manufacturerDataScanActive = YES;
        self.manufacturerDataScanModuleIdentifier =
            MBLinkMercedesModuleIdentifier(module);
        _manufacturerDataForceFullScan = NO;
        _manufacturerDataScanLiveOnly = NO;
        self.manufacturerDataScanStatusText = [NSString stringWithFormat:
            @"Reading startup module data · %zu item%@",
            commandCount, commandCount == 1U ? @"" : @"s"];
        [self setStatus:@"Reading one-time Mercedes module data"];
        [self notifyDelegate];
        [self beginCurrentMercedesDataScanCommand];
        return YES;
    }

    return NO;
}

- (void)processMercedesModuleScanResponse:(const MblinkElm327Response *)response
{
    MblinkMercedesModuleScanResult result = mblink_mercedes_module_scan_accept(&_mercedesModuleScan, response);
    if (self.mercedesVINText.length == 0U && _mercedesModuleScan.vin[0] != '\0') {
        /* Publish through LINK's normal VIN event before profile persistence.
         * This loads VIN-scoped choices while the current census continues. */
        (void)[_shared adoptManufacturerVIN:_mercedesModuleScan.vin];
    }
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE) {
        _moduleScanActive = NO;
        [self updateMercedesModuleScanSummary];
        [self saveCurrentVehicleProfile];
        if (_cachedModuleRefreshActive) {
            const size_t expected =
                mblink_mercedes_module_scan_module_count(
                    &_mercedesModuleScan);
            const size_t fresh =
                mblink_mercedes_module_scan_fresh_response_count(
                    &_mercedesModuleScan);
            if (expected == 0U || fresh != expected) {
                /* Silence after an ignition change or transport recovery is
                 * not evidence that the VIN's topology has changed. Keep the
                 * known routes and their identity/data-ID knowledge. */
                self.vehicleProfileStatusText = [NSString stringWithFormat:
                    @"%zu of %zu saved routes responded; silent routes retained",
                    fresh, expected];
            }
        }
        _cachedModuleRefreshActive = NO;

        /*
         * Module discovery is complete. Every resolved ECU pack now exposes
         * two explicit data sections: one-time startup data and user-selected
         * recurring data. Read the startup section here, one module at a time,
         * before normal live polling is allowed to begin.
         */
        _startupModuleDataPassActive = YES;
        _startupModuleDataIndex = 0U;
        if ([self beginNextStartupModuleDataRead])
            return;
        _startupModuleDataPassActive = NO;

        /* Recurring Mercedes work starts only after all one-time module data
         * has either responded or been attempted. */
        [self updateScheduledManufacturerLiveJob];
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
        MBLinkAppendMercedesModuleFaultStrings(
            faults, module, name, address);
    }

    self.mercedesUDSFaults = [faults copy];
    self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
        @"Scanning · %zu module routes · %zu Mercedes factory fault record%@ captured",
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
        } else if ([name hasPrefix:@"Likely "]) {
            [identity addObject:[NSString stringWithFormat:
                @"MODULE · %@ · %@ · online candidate · %zu fault record%@",
                name, address, moduleFaultCount,
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
            @"MODULE MAP · %zu routes · %zu catalogue matches · %zu unresolved · %@",
            count, classified, count - classified,
            MBLinkStringFromCString(
                mblink_mercedes_module_scan_scope_name(
                    _mercedesModuleScan.scope))]];
        self.mercedesIdentityResults = [identity copy];
        self.mercedesIdentitySummaryText = [NSString stringWithFormat:
            @"%zu module routes · %zu catalogue matches · %zu unresolved",
            count, classified, count - classified];
        self.mercedesUDSFaults = [faults copy];
        self.mercedesUDSFaultStatusText = [NSString stringWithFormat:
            @"%@ · %zu module routes · %zu catalogue matches · %zu Mercedes factory fault record%@%@",
            _mercedesModuleScan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE &&
                mblink_mercedes_module_scan_fresh_response_count(&_mercedesModuleScan) == count
                ? @"Complete" : @"Partial",
            count, classified, totalFaults,
            totalFaults == 1U ? @"" : @"s",
            _mercedesModuleScan.truncated
                ? @" · module list truncated" : @""];
        self.mercedesProbeStatusText = [NSString stringWithFormat:
            @"Mercedes %@ module scan · %zu routes · %zu catalogue matches · see per-module results",
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

    NSDictionary *profile = [_vehicleProfileStore profileForVIN:vin];
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
        NSArray *savedStartup =
            [((NSDictionary *)value)[@"startupData"]
                isKindOfClass:[NSArray class]]
                ? ((NSDictionary *)value)[@"startupData"] : @[];
        if (savedStartup.count != 0U) {
            NSMutableArray<MBLinkMercedesDataSnapshot *> *restored =
                [[NSMutableArray alloc] initWithCapacity:savedStartup.count];
            for (id savedData in savedStartup) {
                MBLinkMercedesDataSnapshot *snapshot =
                    MBLinkMercedesDataSnapshotFromProfile(savedData);
                if (snapshot != nil) [restored addObject:snapshot];
            }
            if (restored.count != 0U) {
                _manufacturerDataByModule[
                    MBLinkMercedesModuleIdentifier(&module)] =
                    [restored copy];
            }
        }
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
    return LinkVehicleProfileCachedPIDs(
        _cachedVehicleProfile, responderCANIdentifier, extendedID);
}

- (void)persistDiscoveredCapabilities
{
    if (_shared.isSimulated || self.mercedesVINText.length == 0U) return;
    const LinkDiagnosticFlow *flow = [_shared diagnosticFlow];
    if (flow == NULL) return;
    if ([_vehicleProfileStore
            mergeStandardCapabilitiesFromDiagnosticFlow:flow
            forVIN:self.mercedesVINText]) {
        _cachedVehicleProfile = [_vehicleProfileStore
            profileForVIN:self.mercedesVINText];
    }
}

- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event
{
    if (event == NULL || self.mercedesVINText.length == 0U) return;
    if ([_vehicleProfileStore
            mergeStandardCapabilitiesFromFlowEvent:event
            forVIN:self.mercedesVINText]) {
        _cachedVehicleProfile = [_vehicleProfileStore
            profileForVIN:self.mercedesVINText];
    }
}

- (void)saveCurrentVehicleProfile
{
    /* LINK owns liveResponders; MBLINK deliberately does not copy or rewrite it. */
    const BOOL persistCISimulatedProfile =
        _shared.isSimulated &&
        [[NSProcessInfo.processInfo.environment
            objectForKey:@"MBLINK_CI_PERSIST_SIMULATED_PROFILE"]
            isEqualToString:@"1"];
    if ((_shared.isSimulated && !persistCISimulatedProfile) ||
        self.mercedesVINText.length == 0U)
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
            @"kind": @((NSUInteger)module->kind),
            @"name": MBLinkStringFromCString(
                mblink_mercedes_module_scan_module_name(module))
        } mutableCopy];
        if (module->definition != NULL) {
            if (module->definition->key != NULL)
                dictionary[@"moduleKey"] =
                    MBLinkStringFromCString(module->definition->key);
            if (module->definition->component_designation != NULL)
                dictionary[@"designation"] =
                    MBLinkStringFromCString(
                        module->definition->component_designation);
            if (module->definition->network != NULL)
                dictionary[@"network"] =
                    MBLinkStringFromCString(module->definition->network);
        }
        if (module->controller_family != NULL &&
            module->controller_family->key != NULL) {
            dictionary[@"controllerFamily"] =
                MBLinkStringFromCString(module->controller_family->key);
        }
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
        NSArray<MBLinkMercedesDataSnapshot *> *startupData =
            [self startupDataSnapshotsForModuleIdentifier:moduleIdentifier];
        if (startupData.count != 0U) {
            NSMutableArray<NSDictionary *> *savedStartup =
                [[NSMutableArray alloc] initWithCapacity:startupData.count];
            for (MBLinkMercedesDataSnapshot *snapshot in startupData) {
                NSDictionary *saved =
                    MBLinkPersistedMercedesDataSnapshot(snapshot);
                if (saved != nil) [savedStartup addObject:saved];
            }
            if (savedStartup.count != 0U)
                dictionary[@"startupData"] = [savedStartup copy];
        }

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
    NSArray<NSString *> *engineEvidence = [self cachedEngineEvidence];
    if (engineEvidence.count != 0U)
        profile[@"engineEvidence"] = engineEvidence;

    [_vehicleProfileStore mergeProfileFields:[profile copy]
                                     forVIN:self.mercedesVINText];

    _cachedVehicleProfile = [_vehicleProfileStore
        profileForVIN:self.mercedesVINText];
    self.vehicleProfileStatusText = [NSString stringWithFormat:
        @"VIN profile saved · %lu module%@ · future connections reuse it",
        (unsigned long)modules.count,
        modules.count == 1U ? @"" : @"s"];
}

- (void)removeSavedVehicleProfileForVIN:(NSString *)vin
{
    if (vin.length == 0U) return;

    [_vehicleProfileStore removeProfileForVIN:vin];
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
    _moduleScanActive = NO;
    _cachedModuleRefreshActive = NO;
    _startupModuleDataPassActive = NO;
    _startupModuleDataIndex = 0U;
    self.manufacturerDataScanActive = NO;
    self.manufacturerDataScanModuleIdentifier = nil;
    if (![_shared completeManufacturerExtensionRestoringAdapter:restore]) {
        [_shared failWithStatus:
            @"Could not resume shared diagnostic flow after Mercedes extension"];
    }
}

@end
