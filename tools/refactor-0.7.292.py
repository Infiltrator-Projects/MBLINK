#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def write(path: str, content: str) -> None:
    target = ROOT / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def regex_once(text: str, pattern: str, replacement: str, label: str) -> str:
    updated, count = re.subn(pattern, replacement, text, count=1, flags=re.S)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one regex match, found {count}")
    return updated


DOMAIN_SWIFT = r'''// SPDX-License-Identifier: GPL-3.0-or-later
import Foundation

/// Machine state used by the MBLINK presentation layer. Human-readable status
/// strings are deliberately not parsed to decide behaviour.
enum MBFaultScanState: Equatable {
    case notStarted
    case running
    case complete
    case failed
}

enum MBConnectionPhase: Equatable {
    case preparing
    case openingAdapter
    case initialisingAdapter
    case readingMercedes
    case readingVehicleDiagnostics
    case ready
    case failed

    var title: String {
        switch self {
        case .preparing: return "Preparing diagnostic session"
        case .openingAdapter: return "Opening adapter channel"
        case .initialisingAdapter: return "Initialising diagnostic adapter"
        case .readingMercedes: return "Reading Mercedes control units"
        case .readingVehicleDiagnostics: return "Reading vehicle diagnostics"
        case .ready: return "Diagnostic session ready"
        case .failed: return "Diagnostic session failed"
        }
    }
}

enum MBConnectionStateResolver {
    static func standardFaultState(
        controller: MBLinkDiagnosticsController
    ) -> MBFaultScanState {
        guard let pointer = controller.diagnosticFlow() else {
            return controller.isActive ? .running : .notStarted
        }
        let flow = pointer.pointee
        if flow.stage == LINK_DIAGNOSTIC_FLOW_FAILED {
            return .failed
        }
        if flow.standard_dtc_inventory_complete {
            return .complete
        }
        if flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_STORED_DTCS ||
            flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PENDING_DTCS ||
            flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PERMANENT_DTCS {
            return .running
        }
        return controller.isActive ? .running : .notStarted
    }

    static func connectionPhase(
        controller: MBLinkDiagnosticsController
    ) -> MBConnectionPhase {
        guard controller.isActive else { return .preparing }
        guard let pointer = controller.diagnosticFlow() else {
            return .openingAdapter
        }
        let stage = pointer.pointee.stage
        if stage == LINK_DIAGNOSTIC_FLOW_FAILED {
            return .failed
        }
        if stage == LINK_DIAGNOSTIC_FLOW_MANUFACTURER_EXTENSION ||
            stage == LINK_DIAGNOSTIC_FLOW_RESTORING_AFTER_MANUFACTURER {
            return .readingMercedes
        }
        if stage == LINK_DIAGNOSTIC_FLOW_SCANNING_STORED_DTCS ||
            stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PENDING_DTCS ||
            stage == LINK_DIAGNOSTIC_FLOW_SCANNING_PERMANENT_DTCS ||
            stage == LINK_DIAGNOSTIC_FLOW_READING_STANDARD_VIN ||
            stage == LINK_DIAGNOSTIC_FLOW_READING_READINESS ||
            stage == LINK_DIAGNOSTIC_FLOW_READING_FREEZE_FRAME {
            return .readingVehicleDiagnostics
        }
        if stage == LINK_DIAGNOSTIC_FLOW_LIVE ||
            stage == LINK_DIAGNOSTIC_FLOW_READING_LIVE {
            return .ready
        }
        if stage == LINK_DIAGNOSTIC_FLOW_INITIALIZING ||
            stage == LINK_DIAGNOSTIC_FLOW_CONFIGURING_PID_DISCOVERY_HEADERS ||
            stage == LINK_DIAGNOSTIC_FLOW_DISCOVERING_PIDS ||
            stage == LINK_DIAGNOSTIC_FLOW_RESTORING_PID_DISCOVERY_HEADERS ||
            stage == LINK_DIAGNOSTIC_FLOW_CONFIGURING_LIVE_HEADERS {
            return .initialisingAdapter
        }
        return .openingAdapter
    }
}

/// Owns persistence and legacy-key canonicalisation for Mercedes live choices.
/// ConnectionViewModel no longer edits UserDefaults dictionaries directly.
final class MBManufacturerSelectionStore {
    private let defaults: UserDefaults
    private let defaultsKey = "mblink.manufacturer.pidSelectionsByVehicle.v1"
    private let aliases = [
        "mercdes.transmission.actual_gear":
            "mercedes.transmission.actual_gear",
        "mercdes.transmission.selector_position":
            "mercedes.transmission.selector_position",
        "mercdes.transmission.drive_program":
            "mercedes.transmission.drive_program"
    ]

    init(defaults: UserDefaults = .standard) {
        self.defaults = defaults
    }

    func canonical(_ stableKey: String) -> String {
        aliases[stableKey] ?? stableKey
    }

    func selection(vin: String, moduleID: String) -> Set<String> {
        guard let vehicles = defaults.dictionary(forKey: defaultsKey),
              let modules = vehicles[vin] as? [String: Any],
              let values = modules[moduleID] as? [String]
        else { return [] }
        let stored = Set(values)
        let canonicalised = Set(values.map(canonical))
        if canonicalised != stored {
            store(canonicalised, vin: vin, moduleID: moduleID)
        }
        return canonicalised
    }

    func store(_ selection: Set<String>, vin: String, moduleID: String) {
        var vehicles = defaults.dictionary(forKey: defaultsKey) ?? [:]
        var modules = vehicles[vin] as? [String: Any] ?? [:]
        modules[moduleID] = Array(selection).sorted()
        vehicles[vin] = modules
        defaults.set(vehicles, forKey: defaultsKey)
    }

    func reset(vin: String) {
        var vehicles = defaults.dictionary(forKey: defaultsKey) ?? [:]
        vehicles.removeValue(forKey: vin)
        defaults.set(vehicles, forKey: defaultsKey)
    }
}

/// Owns product-neutral UDS catalogue projection so the connection view model
/// does not reach directly into codec and safety C APIs.
enum MBUDSCatalogueProvider {
    static func services() -> [MBUDSServiceCatalogueItem] {
        var items = [MBUDSServiceCatalogueItem]()
        let count = Int(link_uds_standard_service_count())
        items.reserveCapacity(count)
        for index in 0..<count {
            guard let pointer = link_uds_standard_service_at(index) else {
                continue
            }
            let definition = pointer.pointee
            let name = definition.name.map { String(cString: $0) } ?? "Unknown"
            let effect = link_uds_service_effect_name(definition.effect)
                .map { String(cString: $0) } ?? "unknown"
            var sid = definition.service
            let safety = withUnsafePointer(to: &sid) {
                link_safety_classify($0, 1)
            }
            let reason = link_safety_reason_string(safety.reason)
                .map { String(cString: $0) } ?? "unknown safety decision"
            let allowed = safety.decision == LINK_SAFETY_ALLOW_READ_ONLY
            items.append(MBUDSServiceCatalogueItem(
                id: definition.service,
                name: name,
                effect: effect,
                transmissionPolicy: allowed
                    ? "Read-only allowed · \(reason)"
                    : "Codec only · \(reason)",
                readOnlyTransmissionAllowed: allowed))
        }
        return items
    }

    static func dtcReports() -> [MBUDSDTCReportCatalogueItem] {
        var items = [MBUDSDTCReportCatalogueItem]()
        let count = Int(link_uds_dtc_report_definition_count())
        items.reserveCapacity(count)
        for index in 0..<count {
            guard let pointer = link_uds_dtc_report_definition_at(index) else {
                continue
            }
            let definition = pointer.pointee
            items.append(MBUDSDTCReportCatalogueItem(
                id: definition.subfunction,
                name: definition.name.map { String(cString: $0) } ?? "Unknown",
                withdrawnIn2020: definition.withdrawn_in_2020))
        }
        return items
    }
}

/// Converts raw DTC strings into structured LINK diagnostic faults. Mercedes
/// reference enrichment stays here rather than in connection orchestration.
enum MBFaultResolver {
    static func resolve(
        _ codes: [String],
        state: String
    ) -> [LinkDiagnosticFault] {
        codes.map { resolve($0, state: state) }
    }

    private static func string(from cString: UnsafePointer<CChar>?) -> String {
        guard let cString else { return "" }
        return String(cString: cString)
    }

    private static func fixedCString<T>(_ value: T) -> String {
        var copy = value
        return withUnsafePointer(to: &copy) { pointer in
            pointer.withMemoryRebound(
                to: CChar.self,
                capacity: MemoryLayout<T>.size
            ) { String(cString: $0) }
        }
    }

    private static func resolve(
        _ code: String,
        state: String
    ) -> LinkDiagnosticFault {
        var knowledge = LinkDtcKnowledge()
        let resolved = code.withCString {
            link_dtc_resolve($0, &knowledge)
        }
        guard resolved else {
            return LinkDiagnosticFault(
                code: code,
                title: "Invalid diagnostic trouble code",
                system: "Unknown",
                category: "Unclassified",
                origin: "Unknown",
                source: "Invalid raw code",
                state: state,
                definitionKnown: false)
        }

        let normalizedCode = fixedCString(knowledge.code)
        let system = string(from: link_dtc_system_name(knowledge.system))
        let origin = string(from: link_dtc_origin_name(knowledge.origin))
        let source = string(from: link_dtc_source_name(knowledge.source))
        let known = knowledge.definition_known
        if !known && knowledge.origin == LINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC {
            var mercedes = MblinkMercedesReferenceDtcKnowledge()
            let referenceFound = code.withCString {
                mblink_mercedes_reference_dtc_resolve($0, &mercedes)
            }
            if referenceFound {
                let referenceTitle = fixedCString(mercedes.title)
                let referenceArea = fixedCString(mercedes.area)
                let referenceSource = fixedCString(mercedes.source)
                return LinkDiagnosticFault(
                    code: normalizedCode.isEmpty ? code : normalizedCode,
                    title: referenceTitle.isEmpty
                        ? "Mercedes reference definition unavailable"
                        : referenceTitle,
                    system: system,
                    category: referenceArea.isEmpty
                        ? "Mercedes reference" : referenceArea,
                    origin: mercedes.ambiguous
                        ? "Mercedes manufacturer reference · ambiguous"
                        : "Mercedes manufacturer reference",
                    source: referenceSource.isEmpty
                        ? "Supplied Mercedes reference catalogue"
                        : referenceSource,
                    state: state,
                    definitionKnown: !mercedes.ambiguous)
            }
        }

        let title = known
            ? fixedCString(knowledge.title)
            : (knowledge.origin == LINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC
                ? "Manufacturer-specific definition not yet mapped"
                : "Diagnostic definition not yet mapped")
        return LinkDiagnosticFault(
            code: normalizedCode.isEmpty ? code : normalizedCode,
            title: title,
            system: system,
            category: known ? fixedCString(knowledge.category) : "Unmapped",
            origin: origin,
            source: source,
            state: state,
            definitionKnown: known)
    }
}
'''

LIFECYCLE_H = r'''// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_MERCEDES_CONNECTION_LIFECYCLE_H
#define MBLINK_MERCEDES_CONNECTION_LIFECYCLE_H

#include "mblink/elm327.h"
#include "mblink/mercedes_module_scan_core.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_CONNECTION_SESSION_NONE = 0,
    MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH,
    MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN
} MblinkMercedesConnectionSessionMode;

typedef enum {
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE = 0,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_PROTOCOL,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_HEADER,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_RECEIVE,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_CONTROL
} MblinkMercedesConnectionSessionStage;

typedef struct {
    MblinkMercedesConnectionSessionMode mode;
    MblinkMercedesConnectionSessionStage stage;
    size_t module_index;
    bool disconnect_requested;
} MblinkMercedesConnectionLifecycle;

void mblink_mercedes_connection_lifecycle_init(
    MblinkMercedesConnectionLifecycle *lifecycle);
void mblink_mercedes_connection_lifecycle_begin(
    MblinkMercedesConnectionLifecycle *lifecycle,
    MblinkMercedesConnectionSessionMode mode);
MblinkMercedesConnectionSessionMode mblink_mercedes_connection_lifecycle_finish(
    MblinkMercedesConnectionLifecycle *lifecycle);
bool mblink_mercedes_connection_lifecycle_active(
    const MblinkMercedesConnectionLifecycle *lifecycle);

/** Resolve one ECU pack's documented enter/quit session commands. */
bool mblink_mercedes_connection_session_commands(
    const MblinkMercedesModuleScanEntry *module,
    char enter_command[5],
    char quit_command[5]);

/** True only for a positively decoded DiagnosticSessionControl response. */
bool mblink_mercedes_connection_session_response_positive(
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
'''

LIFECYCLE_C = r'''// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_connection_lifecycle.h"

#include "mblink/elm327_can.h"
#include "mblink/mercedes_ecu_pack.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>

void mblink_mercedes_connection_lifecycle_init(
    MblinkMercedesConnectionLifecycle *lifecycle)
{
    if (lifecycle == NULL) return;
    *lifecycle = (MblinkMercedesConnectionLifecycle){0};
}

void mblink_mercedes_connection_lifecycle_begin(
    MblinkMercedesConnectionLifecycle *lifecycle,
    MblinkMercedesConnectionSessionMode mode)
{
    if (lifecycle == NULL) return;
    lifecycle->mode = mode;
    lifecycle->stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;
    lifecycle->module_index = 0U;
}

MblinkMercedesConnectionSessionMode mblink_mercedes_connection_lifecycle_finish(
    MblinkMercedesConnectionLifecycle *lifecycle)
{
    if (lifecycle == NULL)
        return MBLINK_MERCEDES_CONNECTION_SESSION_NONE;
    const MblinkMercedesConnectionSessionMode finished = lifecycle->mode;
    lifecycle->mode = MBLINK_MERCEDES_CONNECTION_SESSION_NONE;
    lifecycle->stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;
    lifecycle->module_index = 0U;
    return finished;
}

bool mblink_mercedes_connection_lifecycle_active(
    const MblinkMercedesConnectionLifecycle *lifecycle)
{
    return lifecycle != NULL &&
        lifecycle->mode != MBLINK_MERCEDES_CONNECTION_SESSION_NONE;
}

static bool simple_diagnostic_control_command(
    const char *source,
    char command[5])
{
    if (source == NULL || command == NULL) return false;
    if (source[0] == '0' && (source[1] == 'x' || source[1] == 'X'))
        source += 2;
    if (strlen(source) != 4U) return false;
    for (size_t index = 0U; index < 4U; ++index) {
        const unsigned char value = (unsigned char)source[index];
        if (!isxdigit(value)) return false;
        command[index] = (char)toupper(value);
    }
    command[4] = '\0';
    return true;
}

bool mblink_mercedes_connection_session_commands(
    const MblinkMercedesModuleScanEntry *module,
    char enter_command[5],
    char quit_command[5])
{
    MblinkMercedesEcuPack pack;
    if (module == NULL ||
        !mblink_mercedes_ecu_pack_resolve_module(module, &pack) ||
        pack.observed_protocol_conflict ||
        !simple_diagnostic_control_command(
            pack.session_command, enter_command) ||
        !simple_diagnostic_control_command(
            pack.quit_command, quit_command)) {
        return false;
    }

    /* Field evidence shows that automatic ESP extended-session entry can
     * affect steering behaviour while moving. Keep that safety exception in
     * the Mercedes policy layer, not in the Apple coordinator. */
    if (module->definition != NULL &&
        module->definition->key != NULL &&
        strcmp(module->definition->key, "esp") == 0) {
        return false;
    }
    return true;
}

bool mblink_mercedes_connection_session_response_positive(
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t length = 0U;
    return response != NULL &&
        response->result == MBLINK_ELM327_RESULT_OK &&
        mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &length) ==
            MBLINK_ELM327_CAN_RESULT_OK &&
        length >= 2U && pdu[0] == UINT8_C(0x50);
}
'''

LIFECYCLE_TEST = r'''// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_connection_lifecycle.h"

#include <assert.h>

int main(void)
{
    MblinkMercedesConnectionLifecycle lifecycle;
    mblink_mercedes_connection_lifecycle_init(&lifecycle);
    assert(!mblink_mercedes_connection_lifecycle_active(&lifecycle));
    assert(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_NONE);
    assert(lifecycle.stage == MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE);
    assert(lifecycle.module_index == 0U);
    assert(!lifecycle.disconnect_requested);

    mblink_mercedes_connection_lifecycle_begin(
        &lifecycle, MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    assert(mblink_mercedes_connection_lifecycle_active(&lifecycle));
    assert(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    lifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_CONTROL;
    lifecycle.module_index = 4U;
    lifecycle.disconnect_requested = true;

    const MblinkMercedesConnectionSessionMode finished =
        mblink_mercedes_connection_lifecycle_finish(&lifecycle);
    assert(finished == MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    assert(!mblink_mercedes_connection_lifecycle_active(&lifecycle));
    assert(lifecycle.stage == MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE);
    assert(lifecycle.module_index == 0U);
    /* Disconnect intent survives finishing an establishment operation. */
    assert(lifecycle.disconnect_requested);

    mblink_mercedes_connection_lifecycle_begin(
        &lifecycle, MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);
    assert(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);
    assert(lifecycle.disconnect_requested);
    return 0;
}
'''

write("app/ios/MBLINK/MBLinkDiagnosticsDomain.swift", DOMAIN_SWIFT)
write("include/mblink/mercedes_connection_lifecycle.h", LIFECYCLE_H)
write("src/mercedes/connection_lifecycle.c", LIFECYCLE_C)
write("tests/test_mercedes_connection_lifecycle.c", LIFECYCLE_TEST)

# ---- ConnectionViewModel: remove persistence/codec/fault/state responsibilities.
path = "app/ios/MBLINK/ConnectionViewModel.swift"
text = read(path)
text = regex_once(
    text,
    r'''    /// Complete product-neutral UDS codec catalogue from LINK\..*?    private var controller: MBLinkDiagnosticsController \{\n        productController as! MBLinkDiagnosticsController\n    \}\n''',
    '''    var udsServiceCatalogue: [MBUDSServiceCatalogueItem] {\n        MBUDSCatalogueProvider.services()\n    }\n\n    var udsDTCReportCatalogue: [MBUDSDTCReportCatalogueItem] {\n        MBUDSCatalogueProvider.dtcReports()\n    }\n\n    private let mblinkController: MBLinkDiagnosticsController\n    private var controller: MBLinkDiagnosticsController { mblinkController }\n''',
    "extract UDS projection and typed controller")
text = regex_once(
    text,
    r'''    /\*\n     \* v2 changes first-run policy.*?    private var pidSupportByModule = \[String: Set<UInt8>\]\(\)\n''',
    '''    private let manufacturerSelections = MBManufacturerSelectionStore()\n    private static let moduleStatusMILStableKey = "obd2.readiness.mil"\n    private var pidSupportByModule = [String: Set<UInt8>]()\n''',
    "extract manufacturer selection store")
text = regex_once(
    text,
    r'''    var obdFaultScanComplete: Bool \{.*?    init\(\) \{\n        let controller = MBLinkDiagnosticsController\(\)\n''',
    '''    var obdFaultScanComplete: Bool {\n        MBConnectionStateResolver.standardFaultState(controller: controller) == .complete\n    }\n\n    var obdFaultScanFailed: Bool {\n        MBConnectionStateResolver.standardFaultState(controller: controller) == .failed\n    }\n\n    var mercedesFaultScanComplete: Bool {\n        controller.mercedesFaultScanState == .complete\n    }\n\n    var mercedesFaultScanFailed: Bool {\n        controller.mercedesFaultScanState == .failed\n    }\n\n    var connectionPhaseTitle: String {\n        MBConnectionStateResolver.connectionPhase(controller: controller).title\n    }\n\n    init() {\n        let controller = MBLinkDiagnosticsController()\n        mblinkController = controller\n''',
    "replace string-derived state")
text = regex_once(
    text,
    r'''        // Remove the entire per-VIN Mercedes selection subtree, including\n        // selections for modules that are no longer present in the current map\.\n        let defaults = UserDefaults\.standard\n        var vehicles = defaults\.dictionary\(\n            forKey: Self\.manufacturerSelectionDefaultsKey\) \?\? \[:\]\n        vehicles\.removeValue\(forKey: vin\)\n        defaults\.set\(vehicles, forKey: Self\.manufacturerSelectionDefaultsKey\)\n''',
    '''        // Remove every saved Mercedes selection for this VIN, including\n        // controllers no longer present in the current topology.\n        manufacturerSelections.reset(vin: vin)\n''',
    "delegate manufacturer selection reset")
text = regex_once(
    text,
    r'''    private func manufacturerSelectionSet\(moduleID: String\) -> Set<String> \{.*?    private func applyManufacturerPollingSelection\(moduleID: String\) \{''',
    '''    private func manufacturerSelectionSet(moduleID: String) -> Set<String> {\n        guard let vin = effectivePIDConfigurationVIN else { return [] }\n        return manufacturerSelections.selection(vin: vin, moduleID: moduleID)\n    }\n\n    private func storeManufacturerSelection(\n        _ selection: Set<String>,\n        moduleID: String\n    ) {\n        guard let vin = effectivePIDConfigurationVIN else { return }\n        manufacturerSelections.store(selection, vin: vin, moduleID: moduleID)\n    }\n\n    private func applyManufacturerPollingSelection(moduleID: String) {''',
    "delegate manufacturer selection persistence")
text = replace_once(
    text,
    '''    private func canonicalManufacturerStableKey(_ stableKey: String) -> String {\n        Self.manufacturerStableKeyAliases[stableKey] ?? stableKey\n    }\n''',
    '''    private func canonicalManufacturerStableKey(_ stableKey: String) -> String {\n        manufacturerSelections.canonical(stableKey)\n    }\n''',
    "delegate manufacturer stable-key migration")
text = regex_once(
    text,
    r'''    private func resolveFault\(_ code: String, state: String\) -> DiagnosticFault \{.*?    private func displayScalar\(pid: UInt8, rawValue: Double\) -> Double \{''',
    '''    private func resolveFaults(_ codes: [String], state: String) -> [DiagnosticFault] {\n        MBFaultResolver.resolve(codes, state: state)\n    }\n\n    private func displayScalar(pid: UInt8, rawValue: Double) -> Double {''',
    "extract DTC resolution")
write(path, text)

# ---- Apple controller: typed Mercedes scan state + portable session lifecycle.
path = "platform/apple/MBLinkDiagnosticsController.h"
header = read(path)
header = replace_once(
    header,
    '''@class MBLinkDiagnosticsController;\n\n''',
    '''@class MBLinkDiagnosticsController;\n\ntypedef NS_ENUM(NSUInteger, MBLinkMercedesFaultScanState) {\n    MBLinkMercedesFaultScanStateNotStarted = 0,\n    MBLinkMercedesFaultScanStateRunning,\n    MBLinkMercedesFaultScanStateComplete,\n    MBLinkMercedesFaultScanStateFailed\n};\n\n''',
    "add typed Mercedes scan state enum")
header = replace_once(
    header,
    '''@property(nonatomic, copy, readonly) NSString *mercedesUDSFaultStatusText;\n@property(nonatomic, copy, readonly) NSArray<NSString *> *mercedesUDSFaults;\n''',
    '''@property(nonatomic, copy, readonly) NSString *mercedesUDSFaultStatusText;\n@property(nonatomic, readonly) MBLinkMercedesFaultScanState mercedesFaultScanState;\n@property(nonatomic, copy, readonly) NSArray<NSString *> *mercedesUDSFaults;\n''',
    "publish typed Mercedes scan state")
write(path, header)

path = "platform/apple/MBLinkDiagnosticsController.m"
controller = read(path)
controller = replace_once(
    controller,
    '''#import "mblink/mercedes_ecu_pack.h"\n#import "mblink/mercedes_documented_ecus.h"\n''',
    '''#import "mblink/mercedes_ecu_pack.h"\n#import "mblink/mercedes_connection_lifecycle.h"\n#import "mblink/mercedes_documented_ecus.h"\n''',
    "import portable connection lifecycle")
controller = regex_once(
    controller,
    r'''typedef NS_ENUM\(NSUInteger, MBLinkConnectionSessionMode\) \{.*?\};\n\ntypedef NS_ENUM\(NSUInteger, MBLinkConnectionSessionStage\) \{.*?\};\n\n''',
    "",
    "remove Apple-owned session enums")
controller = replace_once(
    controller,
    '''@property(nonatomic, copy, readwrite) NSString *mercedesUDSFaultStatusText;\n@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesUDSFaults;\n''',
    '''@property(nonatomic, copy, readwrite) NSString *mercedesUDSFaultStatusText;\n@property(nonatomic, readwrite) MBLinkMercedesFaultScanState mercedesFaultScanState;\n@property(nonatomic, copy, readwrite) NSArray<NSString *> *mercedesUDSFaults;\n''',
    "make typed Mercedes scan state writable internally")
controller = replace_once(
    controller,
    '''    MBLinkConnectionSessionMode _connectionSessionMode;\n    MBLinkConnectionSessionStage _connectionSessionStage;\n    size_t _connectionSessionModuleIndex;\n    BOOL _disconnectRequested;\n''',
    '''    MblinkMercedesConnectionLifecycle _connectionLifecycle;\n''',
    "collapse Apple session primitive state")
controller = regex_once(
    controller,
    r'''static BOOL MBLinkSimpleDiagnosticControlCommand\(.*?static NSString \*MBLinkStandardDataKey''',
    '''static NSString *MBLinkStandardDataKey''',
    "move session command/response policy to portable C")
for old, new in [
    ("_connectionSessionMode", "_connectionLifecycle.mode"),
    ("_connectionSessionStage", "_connectionLifecycle.stage"),
    ("_connectionSessionModuleIndex", "_connectionLifecycle.module_index"),
    ("_disconnectRequested", "_connectionLifecycle.disconnect_requested"),
    ("MBLinkConnectionSessionMode", "MblinkMercedesConnectionSessionMode"),
    ("MBLinkConnectionSessionNone", "MBLINK_MERCEDES_CONNECTION_SESSION_NONE"),
    ("MBLinkConnectionSessionEstablish", "MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH"),
    ("MBLinkConnectionSessionTeardown", "MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN"),
    ("MBLinkConnectionSessionStage", "MblinkMercedesConnectionSessionStage"),
    ("MBLinkConnectionSessionStageNone", "MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE"),
    ("MBLinkConnectionSessionStageProtocol", "MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_PROTOCOL"),
    ("MBLinkConnectionSessionStageHeader", "MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_HEADER"),
    ("MBLinkConnectionSessionStageReceive", "MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_RECEIVE"),
    ("MBLinkConnectionSessionStageControl", "MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_CONTROL"),
    ("MBLinkModuleConnectionSessionCommands", "mblink_mercedes_connection_session_commands"),
    ("MBLinkDiagnosticSessionResponsePositive", "mblink_mercedes_connection_session_response_positive"),
]:
    controller = controller.replace(old, new)
controller = replace_once(
    controller,
    '''    _connectionLifecycle.mode = MBLINK_MERCEDES_CONNECTION_SESSION_NONE;\n    _connectionLifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;\n    _connectionLifecycle.module_index = 0U;\n    _connectionLifecycle.disconnect_requested = NO;\n''',
    '''    mblink_mercedes_connection_lifecycle_init(&_connectionLifecycle);\n''',
    "initialise portable connection lifecycle")
controller = replace_once(
    controller,
    '''    _connectionLifecycle.mode = MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH;\n    _connectionLifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;\n    _connectionLifecycle.module_index = 0U;\n''',
    '''    mblink_mercedes_connection_lifecycle_begin(\n        &_connectionLifecycle,\n        MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);\n''',
    "begin connection-owned ECU sessions")
controller = replace_once(
    controller,
    '''        _connectionLifecycle.mode = MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN;\n        _connectionLifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;\n        _connectionLifecycle.module_index = 0U;\n''',
    '''        mblink_mercedes_connection_lifecycle_begin(\n            &_connectionLifecycle,\n            MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);\n''',
    "begin teardown through lifecycle")
controller = replace_once(
    controller,
    '''    const MblinkMercedesConnectionSessionMode finished = _connectionLifecycle.mode;\n    _connectionLifecycle.mode = MBLINK_MERCEDES_CONNECTION_SESSION_NONE;\n    _connectionLifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;\n    _connectionLifecycle.module_index = 0U;\n''',
    '''    const MblinkMercedesConnectionSessionMode finished =\n        mblink_mercedes_connection_lifecycle_finish(&_connectionLifecycle);\n''',
    "finish connection lifecycle")
controller = replace_once(
    controller,
    '''    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes module scan";\n    self.mercedesUDSFaults = @[];\n''',
    '''    self.mercedesUDSFaultStatusText = @"Waiting for Mercedes module scan";\n    self.mercedesFaultScanState = MBLinkMercedesFaultScanStateNotStarted;\n    self.mercedesUDSFaults = @[];\n''',
    "reset typed Mercedes scan state")
controller = replace_once(
    controller,
    '''- (void)markFlowFailure:(NSString *)status\n{\n''',
    '''- (void)markFlowFailure:(NSString *)status\n{\n    self.mercedesFaultScanState = MBLinkMercedesFaultScanStateFailed;\n''',
    "mark typed Mercedes scan failure")
controller = replace_once(
    controller,
    '''- (void)beginMercedesModuleScan\n{\n''',
    '''- (void)beginMercedesModuleScan\n{\n    self.mercedesFaultScanState = MBLinkMercedesFaultScanStateRunning;\n''',
    "mark Mercedes scan running")
controller = replace_once(
    controller,
    '''    self.mercedesUDSFaultStatusText = [NSString stringWithFormat:\n        @"Scanning · %zu module routes · %zu Mercedes factory fault record%@ captured",\n''',
    '''    self.mercedesFaultScanState = MBLinkMercedesFaultScanStateRunning;\n    self.mercedesUDSFaultStatusText = [NSString stringWithFormat:\n        @"Scanning · %zu module routes · %zu Mercedes factory fault record%@ captured",\n''',
    "keep typed scan state during progress")
controller = replace_once(
    controller,
    '''    self.mercedesUDSFaultStatusText =\n        @"Refreshing faults from saved module topology";\n''',
    '''    self.mercedesFaultScanState = MBLinkMercedesFaultScanStateRunning;\n    self.mercedesUDSFaultStatusText =\n        @"Refreshing faults from saved module topology";\n''',
    "mark cached topology refresh running")
start = controller.index("- (void)updateMercedesModuleScanSummary\n{")
end = controller.index("\n}\n\n- (", start) + 2
summary = controller[start:end]
needle = "    [self notifyDelegate];"
if summary.count(needle) != 1:
    raise RuntimeError("updateMercedesModuleScanSummary: unexpected notify count")
summary = summary.replace(
    needle,
    '''    self.mercedesFaultScanState =\n        (_mercedesModuleScan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE &&\n         mblink_mercedes_module_scan_fresh_response_count(&_mercedesModuleScan) == count)\n            ? MBLinkMercedesFaultScanStateComplete\n            : MBLinkMercedesFaultScanStateFailed;\n    [self notifyDelegate];''')
controller = controller[:start] + summary + controller[end:]
write(path, controller)

# ---- Portable build/test wiring.
path = "CMakeLists.txt"
cmake = read(path)
cmake = replace_once(
    cmake,
    '''    src/mercedes/ecu_pack.c\n    src/mercedes/factory_data.c''',
    '''    src/mercedes/ecu_pack.c src/mercedes/connection_lifecycle.c\n    src/mercedes/factory_data.c''',
    "compile portable connection lifecycle")
cmake = replace_once(
    cmake,
    '''    mblink_add_test(mblink-test-mercedes-ecu-pack tests/test_mercedes_ecu_pack.c)\n    mblink_add_test(mblink-test-mercedes-transmission tests/test_mercedes_transmission.c)''',
    '''    mblink_add_test(mblink-test-mercedes-ecu-pack tests/test_mercedes_ecu_pack.c)\n    mblink_add_test(mblink-test-mercedes-connection-lifecycle tests/test_mercedes_connection_lifecycle.c)\n    mblink_add_test(mblink-test-mercedes-transmission tests/test_mercedes_transmission.c)''',
    "test portable connection lifecycle")
write(path, cmake)

# ---- Xcode project wiring and release version.
path = "app/ios/MBLINK.xcodeproj/project.pbxproj"
pbx = read(path)
pbx = replace_once(
    pbx,
    '''\t\t66B05F8E65443E6407074F75 /* ConnectionViewModel.swift in Sources */ = {isa = PBXBuildFile; fileRef = 9026C4BA44795B23ABC260F9 /* ConnectionViewModel.swift */; };\n''',
    '''\t\t66B05F8E65443E6407074F75 /* ConnectionViewModel.swift in Sources */ = {isa = PBXBuildFile; fileRef = 9026C4BA44795B23ABC260F9 /* ConnectionViewModel.swift */; };\n\t\tD29200000000000000000001 /* MBLinkDiagnosticsDomain.swift in Sources */ = {isa = PBXBuildFile; fileRef = D29200000000000000000002 /* MBLinkDiagnosticsDomain.swift */; };\n''',
    "add Swift domain build file")
pbx = replace_once(
    pbx,
    '''\t\tE0A1B2C3D4E5F60718293B15 /* ecu_pack.c in Sources */ = {isa = PBXBuildFile; fileRef = F0A1B2C3D4E5F60718293A15 /* ecu_pack.c */; };\n''',
    '''\t\tE0A1B2C3D4E5F60718293B15 /* ecu_pack.c in Sources */ = {isa = PBXBuildFile; fileRef = F0A1B2C3D4E5F60718293A15 /* ecu_pack.c */; };\n\t\tD29200000000000000000003 /* connection_lifecycle.c in Sources */ = {isa = PBXBuildFile; fileRef = D29200000000000000000004 /* connection_lifecycle.c */; };\n''',
    "add lifecycle C build file")
pbx = replace_once(
    pbx,
    '''\t\t9026C4BA44795B23ABC260F9 /* ConnectionViewModel.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; name = "ConnectionViewModel.swift"; path = "MBLINK/ConnectionViewModel.swift"; sourceTree = SOURCE_ROOT; };\n''',
    '''\t\t9026C4BA44795B23ABC260F9 /* ConnectionViewModel.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; name = "ConnectionViewModel.swift"; path = "MBLINK/ConnectionViewModel.swift"; sourceTree = SOURCE_ROOT; };\n\t\tD29200000000000000000002 /* MBLinkDiagnosticsDomain.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; name = "MBLinkDiagnosticsDomain.swift"; path = "MBLINK/MBLinkDiagnosticsDomain.swift"; sourceTree = SOURCE_ROOT; };\n''',
    "add Swift domain file reference")
pbx = replace_once(
    pbx,
    '''\t\tF0A1B2C3D4E5F60718293A15 /* ecu_pack.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = "ecu_pack.c"; path = "../../src/mercedes/ecu_pack.c"; sourceTree = SOURCE_ROOT; };\n''',
    '''\t\tF0A1B2C3D4E5F60718293A15 /* ecu_pack.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = "ecu_pack.c"; path = "../../src/mercedes/ecu_pack.c"; sourceTree = SOURCE_ROOT; };\n\t\tD29200000000000000000004 /* connection_lifecycle.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = "connection_lifecycle.c"; path = "../../src/mercedes/connection_lifecycle.c"; sourceTree = SOURCE_ROOT; };\n''',
    "add lifecycle C file reference")
pbx = replace_once(
    pbx,
    '''\t\t\t\t9026C4BA44795B23ABC260F9 /* ConnectionViewModel.swift */,\n\t\t\t\tD26700000000000000000002 /* StandardPIDSelectionMigrator.swift */,\n''',
    '''\t\t\t\t9026C4BA44795B23ABC260F9 /* ConnectionViewModel.swift */,\n\t\t\t\tD29200000000000000000002 /* MBLinkDiagnosticsDomain.swift */,\n\t\t\t\tD26700000000000000000002 /* StandardPIDSelectionMigrator.swift */,\n''',
    "show Swift domain in App group")
pbx = replace_once(
    pbx,
    '''\t\t\t\tF0A1B2C3D4E5F60718293A15 /* ecu_pack.c */,\n\t\t\t\tF0A1B2C3D4E5F60718293A16 /* module_catalog.c */,\n''',
    '''\t\t\t\tF0A1B2C3D4E5F60718293A15 /* ecu_pack.c */,\n\t\t\t\tD29200000000000000000004 /* connection_lifecycle.c */,\n\t\t\t\tF0A1B2C3D4E5F60718293A16 /* module_catalog.c */,\n''',
    "show lifecycle in Portable Core group")
pbx = replace_once(
    pbx,
    '''\t\t\t\t66B05F8E65443E6407074F75 /* ConnectionViewModel.swift in Sources */,\n\t\t\t\tD26700000000000000000001 /* StandardPIDSelectionMigrator.swift in Sources */,\n''',
    '''\t\t\t\t66B05F8E65443E6407074F75 /* ConnectionViewModel.swift in Sources */,\n\t\t\t\tD29200000000000000000001 /* MBLinkDiagnosticsDomain.swift in Sources */,\n\t\t\t\tD26700000000000000000001 /* StandardPIDSelectionMigrator.swift in Sources */,\n''',
    "compile Swift domain in app target")
pbx = replace_once(
    pbx,
    '''\t\t\t\tE0A1B2C3D4E5F60718293B15 /* ecu_pack.c in Sources */,\n\t\t\t\tE0A1B2C3D4E5F60718293B16 /* module_catalog.c in Sources */,\n''',
    '''\t\t\t\tE0A1B2C3D4E5F60718293B15 /* ecu_pack.c in Sources */,\n\t\t\t\tD29200000000000000000003 /* connection_lifecycle.c in Sources */,\n\t\t\t\tE0A1B2C3D4E5F60718293B16 /* module_catalog.c in Sources */,\n''',
    "compile lifecycle in MBLINKCore")
pbx = pbx.replace("MARKETING_VERSION = 0.7.291;", "MARKETING_VERSION = 0.7.292;")
if "MARKETING_VERSION = 0.7.291;" in pbx or pbx.count("MARKETING_VERSION = 0.7.292;") != 4:
    raise RuntimeError("unexpected Xcode MARKETING_VERSION replacement")
write(path, pbx)

# ---- Release metadata.
write("VERSION", "0.7.292\n")
version_h = read("include/mblink/version.h")
version_h = replace_once(
    version_h,
    '#define MBLINK_VERSION_STRING "0.7.291"',
    '#define MBLINK_VERSION_STRING "0.7.292"',
    "bump C version")
write("include/mblink/version.h", version_h)

changelog = read("CHANGELOG.md")
changelog = replace_once(
    changelog,
    "# Changelog\n\n",
    "# Changelog\n\n## 0.7.292 — 2026-10-12\n\n"
    "- Split iPhone diagnostic presentation/persistence concerns out of ConnectionViewModel into typed domain services for UDS catalogue projection, manufacturer selection persistence and DTC resolution; keep a strongly typed MBLINK controller reference instead of force-casting the generic controller.\n"
    "- Replace fault/connection behaviour inferred from human-readable status strings with machine state from LINK's diagnostic flow and an explicit Mercedes fault-scan state.\n"
    "- Move Mercedes connection-owned ECU session state plus enter/quit command and positive-response policy into a tested portable lifecycle module, reducing temporal state and ECU-pack policy inside the Apple diagnostics controller.\n"
    "- Run release-critical Linux validation on GitHub-hosted runners so publication no longer waits on an offline local runner.\n\n",
    "prepend changelog")
write("CHANGELOG.md", changelog)

architecture = read("docs/ARCHITECTURE.md")
architecture = replace_once(
    architecture,
    "Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery.\n",
    "Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery. Mercedes connection-owned diagnostic-session mode/stage/index state and session-command policy live in the portable Mercedes connection lifecycle; the Apple controller performs platform I/O around that state rather than owning another private session state machine.\n\nThe iPhone presentation edge consumes typed diagnostic state. Connection/fault behaviour is derived from LINK diagnostic-flow state plus the explicit Mercedes fault-scan state, never by searching human-readable status strings. Manufacturer PID persistence, generic UDS catalogue projection and DTC presentation enrichment are separate domain services rather than responsibilities of `ConnectionViewModel`.\n",
    "document reduced coupling")
write("docs/ARCHITECTURE.md", architecture)

# Release builds must not depend on Shannon's local self-hosted runner.
ci = read(".github/workflows/ci.yml")
old_runner = "runs-on: [self-hosted, Linux, X64, linux-native]"
count = ci.count(old_runner)
if count != 3:
    raise RuntimeError(f"expected 3 local Linux runner jobs, found {count}")
ci = ci.replace(old_runner, "runs-on: ubuntu-24.04")
write(".github/workflows/ci.yml", ci)

# Sanity checks on the refactor itself.
model = read("app/ios/MBLINK/ConnectionViewModel.swift")
if "productController as! MBLinkDiagnosticsController" in model:
    raise RuntimeError("force-cast controller still present")
for forbidden in (
    'faultScanStatusText.lowercased()',
    'mercedesUDSFaultStatusText.lowercased()',
    'statusText.lowercased()',
    'manufacturerSelectionDefaultsKey',
    'manufacturerStableKeyAliases',
):
    if forbidden in model:
        raise RuntimeError(f"legacy coupled state remains: {forbidden}")
controller = read("platform/apple/MBLinkDiagnosticsController.m")
for forbidden in (
    "_connectionSessionMode",
    "_connectionSessionStage",
    "_connectionSessionModuleIndex",
    "MBLinkModuleConnectionSessionCommands",
    "MBLinkDiagnosticSessionResponsePositive",
):
    if forbidden in controller:
        raise RuntimeError(f"Apple session coupling remains: {forbidden}")

print("MBLINK 0.7.292 architecture refactor staged successfully")
