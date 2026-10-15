// SPDX-License-Identifier: GPL-3.0-or-later
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
            let name = definition.name.map { String(cString: $0) } ?? "Unknown"
            items.append(MBUDSDTCReportCatalogueItem(
                id: definition.subfunction,
                name: name,
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
