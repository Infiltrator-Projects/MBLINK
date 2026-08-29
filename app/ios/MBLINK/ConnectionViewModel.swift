// SPDX-License-Identifier: GPL-3.0-or-later
import Combine
import Foundation
import UIKit

enum MBLINKUnitProfile: String, CaseIterable, Identifiable {
    case metric
    case usCustomary = "us"

    var id: String { rawValue }
    var displayName: String {
        switch self {
        case .metric: return "Metric"
        case .usCustomary: return "US customary"
        }
    }
}

struct DiagnosticParameter: Identifiable {
    let id: String
    let protocolName: String
    let moduleIdentifier: UInt32
    let parameterIdentifier: UInt32
    let shortName: String
    let title: String
    let suffix: String
    let formattedValue: String
    let value: Double?
    let favourite: Bool
    let pollingEnabled: Bool
    let history: [Double]

    var isAvailable: Bool { value != nil }
}

struct DiagnosticFault: Identifiable {
    let code: String
    let title: String
    let system: String
    let category: String
    let origin: String
    let source: String
    let state: String
    let definitionKnown: Bool

    var id: String { "\(state):\(code)" }
    var displayText: String { "\(code) — \(title)" }
}

struct MercedesTargetSignal: Identifiable {
    let id: String
    let title: String
    let category: String
    let status: String
    let provenance: String
}

/// Structured, presentation-ready facts decoded directly from a Mercedes VIN.
/// Raw diagnostic evidence remains in `mercedesIdentityResults`; the Vehicle
/// screen uses this model so protocol delimiters never leak into the UI.
struct MercedesVehicleIdentity: Equatable {
    let vin: String
    let manufacturer: String
    let model: String?
    let chassis: String?
    let bodyStyle: String?
    let baumuster: String?
    let productionYears: String?
    let engineCode: String?
    let engineFamily: String?
    let displacementCC: UInt32?
    let ratedPowerKW: UInt32?
    let fuel: String?
    let plant: String?
    let country: String?
    let steering: String?
    let serialNumber: String?
}

private func mblinkLocalized(_ key: String) -> String {
    let stored = UserDefaults.standard.string(forKey: "mblink.language") ?? "en-AU"
    let language: String
    switch stored {
    case "en": language = "en-AU"
    case "de": language = "de-DE"
    case "pl": language = "pl-PL"
    default: language = stored
    }
    guard let path = Bundle.main.path(forResource: language, ofType: "lproj"),
          let bundle = Bundle(path: path) else {
        return key
    }
    return bundle.localizedString(forKey: key, value: key, table: nil)
}

@MainActor
final class ConnectionViewModel: NSObject, ObservableObject, MBLinkDiagnosticsControllerDelegate {
    @Published private(set) var statusText = "Idle"
    @Published private(set) var peripheralName = "No adapter"
    @Published private(set) var adapterIdentifier = "Unknown"
    @Published private(set) var mercedesProbeStatusText = "Not attempted"
    @Published private(set) var mercedesProbeEndpointText = "Source-corroborated endpoint not selected"
    @Published private(set) var mercedesVINText = "Not captured"
    @Published private(set) var vehicleIdentity: MercedesVehicleIdentity?
    @Published private(set) var mercedesIdentitySummaryText = "Not attempted"
    @Published private(set) var mercedesIdentityResults = [String]()
    @Published private(set) var mercedesCrd3SummaryText = "Not attempted"
    @Published private(set) var mercedesUDSFaultStatusText = "Not scanned"
    @Published private(set) var mercedesUDSFaults = [String]()
    @Published private(set) var vehicleProfileStatusText = "Waiting for VIN"
    @Published private(set) var faultScanStatusText = "Not scanned"
    @Published private(set) var storedDTCs = [String]()
    @Published private(set) var pendingDTCs = [String]()
    @Published private(set) var permanentDTCs = [String]()
    @Published private(set) var storedFaults = [DiagnosticFault]()
    @Published private(set) var pendingFaults = [DiagnosticFault]()
    @Published private(set) var permanentFaults = [DiagnosticFault]()
    @Published private(set) var isActive = false
    @Published private(set) var isReady = false
    @Published private(set) var isSimulationActive = false

    @Published private(set) var diagnosticParameters = [DiagnosticParameter]()
    @Published private(set) var mercedesTargetSignals = [MercedesTargetSignal]()
    @Published private(set) var recordedSampleCount = 0
    @Published private(set) var csvExportURL: URL?
    @Published private(set) var isPreparingCSV = false

    private let controller = MBLinkDiagnosticsController()

    /*
     * v2 changes first-run policy from an automatic core set to explicit
     * opt-in. Existing user choices are preserved, but the old untouched
     * automatic default is recognised and migrated to an empty selection.
     */
    private static let pollingDefaultsKey = "mblink.polling.enabledStableKeys.v2"
    private static let legacyPollingDefaultsKey = "mblink.polling.enabledStableKeys.v1"
    private static let legacyAutomaticPollingStableKeys: Set<String> = [
        "obd2.engine.rpm", "obd2.vehicle.speed", "obd2.engine.coolant",
        "obd2.diesel.rail_pressure", "obd2.engine.throttle",
        "obd2.driver.accelerator_pedal_d",
        "obd2.driver.accelerator_pedal_e",
        "obd2.environment.ambient_air",
        "obd2.fuel.tank_level"
    ]

    var obdFaultScanComplete: Bool {
        faultScanStatusText.hasPrefix("Complete ·") || faultScanStatusText == "Complete"
    }

    var obdFaultScanFailed: Bool {
        let value = faultScanStatusText.lowercased()
        return value.contains("timed out") || value.contains("error") || value.contains("failed")
    }

    override init() {
        super.init()
        applyStoredPollingPolicy()
        controller.delegate = self
        mercedesTargetSignals = loadMercedesTargetSignals()
        refresh()
    }

    func connect() {
        clearPreparedExport()
        if isActive { return }

        let alert = UIAlertController(
            title: mblinkLocalized("Connection Test"),
            message: mblinkLocalized("Real Adapter supports ELM/Vgate diagnostics and genuine MB-xxxx Mercedes me native Bluetooth capture. Simulated ELM327 runs the same ELM, OBD, UDS, Mercedes probe, telemetry and evidence stack against an in-process byte-stream emulator."),
            preferredStyle: .alert
        )
        alert.addAction(UIAlertAction(title: mblinkLocalized("Real Adapter"), style: .default) { [weak self] _ in
            Task { @MainActor [weak self] in
                guard let self else { return }
                self.isSimulationActive = false
                self.controller.start()
            }
        })
        alert.addAction(UIAlertAction(title: mblinkLocalized("Simulated ELM327"), style: .default) { [weak self] _ in
            Task { @MainActor [weak self] in
                guard let self else { return }
                self.isSimulationActive = true
                self.controller.startSimulated()
            }
        })
        alert.addAction(UIAlertAction(title: mblinkLocalized("Cancel"), style: .cancel))

        guard let presenter = presentingViewController() else {
            isSimulationActive = false
            controller.start()
            return
        }
        presenter.present(alert, animated: true)
    }

    func disconnect() {
        controller.disconnect()
        isSimulationActive = false
    }

    func parameter(stableKey: String) -> DiagnosticParameter? {
        diagnosticParameters.first { $0.id == stableKey }
    }

    func mercedesSignals(category: String) -> [MercedesTargetSignal] {
        mercedesTargetSignals.filter { $0.category == category }
    }

    func toggleFavourite(stableKey: String) {
        let pid: UInt8? = stableKey.withCString { key in
            guard let definition = mblink_parameter_obd2_definition_for_stable_key(key) else { return nil }
            return UInt8(exactly: definition.pointee.key.identifier)
        }
        guard let pid else { return }
        controller.setFavourite(!controller.favourite(forPID: pid), forPID: pid)
        refresh()
    }

    func setPolling(_ enabled: Bool, stableKey: String) {
        let pid: UInt8? = stableKey.withCString { key in
            guard let definition = mblink_parameter_obd2_definition_for_stable_key(key) else { return nil }
            return UInt8(exactly: definition.pointee.key.identifier)
        }
        guard let pid else { return }
        var enabledKeys = storedPollingKeys()
        if enabled { enabledKeys.insert(stableKey) } else { enabledKeys.remove(stableKey) }
        UserDefaults.standard.set(Array(enabledKeys).sorted(), forKey: Self.pollingDefaultsKey)
        controller.setPollingEnabled(enabled, forPID: pid)
        refresh()
    }

    func refreshPresentation() {
        diagnosticParameters = loadDiagnosticParameters()
    }

    func udsStatusText(_ status: UInt8) -> String {
        var buffer = [CChar](repeating: 0, count: Int(LINK_DTC_STATUS_TEXT_LENGTH))
        let success = buffer.withUnsafeMutableBufferPointer { storage in
            link_dtc_format_uds_status(status, storage.baseAddress, storage.count)
        }
        guard success else { return String(format: "Status 0x%02X", status) }
        return buffer.withUnsafeBufferPointer { storage in
            guard let baseAddress = storage.baseAddress else {
                return String(format: "Status 0x%02X", status)
            }
            return String(cString: baseAddress)
        }
    }

    func prepareCSVExport() {
        guard !isPreparingCSV else { return }
        /*
         * Snapshot the recorder's mutable bytes on the main actor, then move
         * UTF-8/file-system work away from CoreBluetooth and the 100 ms
         * diagnostic-session tick. Preparing evidence must never stop polling.
         */
        guard let data = controller.csvDataSnapshot() else { return }

        let filename = "MBLINK-diagnostic-evidence-\(UUID().uuidString).csv"
        let url = FileManager.default.temporaryDirectory.appendingPathComponent(filename)
        isPreparingCSV = true

        Task { [weak self] in
            do {
                try await Task.detached(priority: .utility) {
                    try data.write(to: url, options: .atomic)
                }.value
                guard let self else {
                    try? FileManager.default.removeItem(at: url)
                    return
                }
                self.clearPreparedExport()
                self.csvExportURL = url
            } catch {
                try? FileManager.default.removeItem(at: url)
            }
            self?.isPreparingCSV = false
        }
    }

    nonisolated func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {
        Task { @MainActor [weak self] in self?.refresh() }
    }

    private func presentingViewController() -> UIViewController? {
        guard let scene = UIApplication.shared.connectedScenes
            .compactMap({ $0 as? UIWindowScene })
            .first(where: { $0.activationState == .foregroundActive }),
              let root = scene.windows.first(where: \.isKeyWindow)?.rootViewController else {
            return nil
        }
        return topViewController(root)
    }

    private func topViewController(_ controller: UIViewController) -> UIViewController {
        if let presented = controller.presentedViewController { return topViewController(presented) }
        if let navigation = controller as? UINavigationController,
           let visible = navigation.visibleViewController { return topViewController(visible) }
        if let tabs = controller as? UITabBarController,
           let selected = tabs.selectedViewController { return topViewController(selected) }
        return controller
    }

    private func clearPreparedExport() {
        if let url = csvExportURL { try? FileManager.default.removeItem(at: url) }
        csvExportURL = nil
    }

    private func storedPollingKeys() -> Set<String> {
        let defaults = UserDefaults.standard

        if let values = defaults.array(forKey: Self.pollingDefaultsKey) as? [String] {
            return Set(values)
        }

        /*
         * Upgrade from the previous automatic-core policy. If v1 still holds
         * exactly the old built-in set, it was never a meaningful user choice,
         * so v2 starts empty. If the set differs, the user changed it and those
         * explicit choices are preserved.
         */
        var initial = Set<String>()
        if let legacyValues =
            defaults.array(forKey: Self.legacyPollingDefaultsKey) as? [String] {
            let legacy = Set(legacyValues)
            if legacy != Self.legacyAutomaticPollingStableKeys {
                initial = legacy
            }
        }

        defaults.set(Array(initial).sorted(), forKey: Self.pollingDefaultsKey)
        return initial
    }

    private func applyStoredPollingPolicy() {
        let enabled = storedPollingKeys()
        let count = mblink_parameter_obd2_definition_count()
        guard count > 0 else { return }
        for index in 0..<count {
            guard let definition = mblink_parameter_obd2_definition_at(index) else { continue }
            let metadata = definition.pointee
            guard let pid = UInt8(exactly: metadata.key.identifier) else { continue }
            let stableKey = string(from: metadata.stable_key)
            controller.setPollingEnabled(enabled.contains(stableKey), forPID: pid)
        }
    }

    private var unitProfile: MBLINKUnitProfile {
        let stored = UserDefaults.standard.string(forKey: "mblink.units") ??
            MBLINKUnitProfile.metric.rawValue
        return MBLINKUnitProfile(rawValue: stored) ?? .metric
    }

    private func string(from cString: UnsafePointer<CChar>?) -> String {
        guard let cString else { return "" }
        return String(cString: cString)
    }

    private func stringFromFixedCString<T>(_ value: T) -> String {
        var copy = value
        return withUnsafePointer(to: &copy) { pointer in
            pointer.withMemoryRebound(to: CChar.self, capacity: MemoryLayout<T>.size) {
                String(cString: $0)
            }
        }
    }

    private func resolveFault(_ code: String, state: String) -> DiagnosticFault {
        var knowledge = LinkDtcKnowledge()
        let resolved = code.withCString { rawCode in link_dtc_resolve(rawCode, &knowledge) }
        guard resolved else {
            return DiagnosticFault(code: code,
                                   title: "Invalid diagnostic trouble code",
                                   system: "Unknown",
                                   category: "Unclassified",
                                   origin: "Unknown",
                                   source: "Invalid raw code",
                                   state: state,
                                   definitionKnown: false)
        }

        let normalizedCode = stringFromFixedCString(knowledge.code)
        let system = string(from: link_dtc_system_name(knowledge.system))
        let origin = string(from: link_dtc_origin_name(knowledge.origin))
        let source = string(from: link_dtc_source_name(knowledge.source))
        let known = knowledge.definition_known
        let title = known
            ? stringFromFixedCString(knowledge.title)
            : (knowledge.origin == LINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC
                ? "Manufacturer-specific definition not yet mapped"
                : "Diagnostic definition not yet mapped")
        let category = known ? stringFromFixedCString(knowledge.category) : "Unmapped"
        return DiagnosticFault(code: normalizedCode.isEmpty ? code : normalizedCode,
                               title: title,
                               system: system,
                               category: category,
                               origin: origin,
                               source: source,
                               state: state,
                               definitionKnown: known)
    }

    private func resolveFaults(_ codes: [String], state: String) -> [DiagnosticFault] {
        codes.map { resolveFault($0, state: state) }
    }

    private func displayScalar(
        definition: UnsafePointer<MblinkParameterDefinition>,
        rawValue: Double
    ) -> Double {
        guard unitProfile == .usCustomary else { return rawValue }
        switch string(from: definition.pointee.suffix) {
        case " °C": return rawValue * 9.0 / 5.0 + 32.0
        case " km/h": return rawValue * 0.621371192237334
        case " kPa": return rawValue * 0.14503773773020923
        case " L/h": return rawValue * 0.2641720523581484
        default: return rawValue
        }
    }

    private func displaySuffix(
        definition: UnsafePointer<MblinkParameterDefinition>
    ) -> String {
        guard unitProfile == .usCustomary else { return string(from: definition.pointee.suffix) }
        switch string(from: definition.pointee.suffix) {
        case " °C": return " °F"
        case " km/h": return " mph"
        case " kPa": return " psi"
        case " L/h": return " US gal/h"
        default: return string(from: definition.pointee.suffix)
        }
    }

    private func formattedValue(
        definition: UnsafePointer<MblinkParameterDefinition>,
        value: Double?
    ) -> String {
        guard let value else { return "N/A" }
        if unitProfile == .usCustomary {
            let displayed = displayScalar(definition: definition, rawValue: value)
            let suffix = displaySuffix(definition: definition)
            switch suffix {
            case " °F", " mph": return String(format: "%.1f%@", displayed, suffix)
            case " psi":
                return abs(displayed) < 10.0
                    ? String(format: "%.2f%@", displayed, suffix)
                    : String(format: "%.1f%@", displayed, suffix)
            case " US gal/h": return String(format: "%.2f%@", displayed, suffix)
            default: break
            }
        }
        var buffer = [CChar](repeating: 0, count: 96)
        let success = buffer.withUnsafeMutableBufferPointer { storage in
            mblink_parameter_format_value(definition, true, value,
                                          storage.baseAddress, storage.count)
        }
        guard success else { return "N/A" }
        return buffer.withUnsafeBufferPointer { storage in
            guard let baseAddress = storage.baseAddress else { return "N/A" }
            return String(cString: baseAddress)
        }
    }

    private func loadDiagnosticParameters() -> [DiagnosticParameter] {
        let count = mblink_parameter_obd2_definition_count()
        guard count > 0 else { return [] }
        var result = [DiagnosticParameter]()
        result.reserveCapacity(count)

        for index in 0..<count {
            guard let definition = mblink_parameter_obd2_definition_at(index) else { continue }
            let metadata = definition.pointee
            guard let pid = UInt8(exactly: metadata.key.identifier) else { continue }
            let rawHistory = controller.recentValues(forPID: pid, limit: 60).map(\.doubleValue)
            let rawValue = rawHistory.last
            let history = rawHistory.map { displayScalar(definition: definition, rawValue: $0) }
            let value = rawValue.map { displayScalar(definition: definition, rawValue: $0) }
            let stableKey = string(from: metadata.stable_key)
            guard !stableKey.isEmpty else { continue }
            result.append(DiagnosticParameter(
                id: stableKey,
                protocolName: string(from: mblink_parameter_protocol_name(metadata.key.protocol)),
                moduleIdentifier: metadata.key.module,
                parameterIdentifier: metadata.key.identifier,
                shortName: string(from: metadata.short_name),
                title: string(from: metadata.name),
                suffix: displaySuffix(definition: definition),
                formattedValue: formattedValue(definition: definition, value: rawValue),
                value: value,
                favourite: controller.favourite(forPID: pid),
                pollingEnabled: controller.pollingEnabled(forPID: pid),
                history: history))
        }
        return result
    }

    private func loadMercedesTargetSignals() -> [MercedesTargetSignal] {
        let count = Int(mblink_mercedes_om651_catalog_count())
        guard count > 0 else { return [] }
        var result = [MercedesTargetSignal]()
        result.reserveCapacity(count)
        for index in 0..<count {
            guard let definition = mblink_mercedes_om651_catalog_at(index) else { continue }
            let metadata = definition.pointee
            let key = string(from: metadata.key)
            guard !key.isEmpty else { continue }
            result.append(MercedesTargetSignal(
                id: key,
                title: string(from: metadata.name),
                category: string(from: mblink_mercedes_om651_signal_category_name(metadata.category)),
                status: string(from: mblink_mercedes_om651_signal_status_name(metadata.status)),
                provenance: string(from: metadata.provenance)))
        }
        return result
    }

    private func decodeVehicleIdentity(vin: String?) -> MercedesVehicleIdentity? {
        guard let vin, !vin.isEmpty else { return nil }

        var decoded = MblinkMercedesVinDecode()
        let succeeded = vin.withCString { rawVIN in
            mblink_mercedes_vin_decode(rawVIN, &decoded)
        }
        guard succeeded else { return nil }

        let definition = decoded.baumuster_definition?.pointee
        let plant = decoded.plant_definition?.pointee
        let wmi = decoded.wmi_definition?.pointee
        let baumuster = decoded.baumuster_available
            ? stringFromFixedCString(decoded.baumuster) : ""
        let serial = stringFromFixedCString(decoded.serial_number)
        let steering = string(from: mblink_mercedes_steering_name(decoded.steering))

        func nonempty(_ value: String) -> String? {
            value.isEmpty || value == "unknown" ? nil : value
        }

        return MercedesVehicleIdentity(
            vin: stringFromFixedCString(decoded.vin),
            manufacturer: nonempty(string(from: wmi?.manufacturer)) ?? "Mercedes-Benz",
            model: definition.map { nonempty(string(from: $0.model)) ?? "Mercedes-Benz" },
            chassis: definition.flatMap { nonempty(string(from: $0.chassis_family)) },
            bodyStyle: definition.flatMap { nonempty(string(from: $0.body_style)) },
            baumuster: nonempty(baumuster),
            productionYears: definition.flatMap { nonempty(string(from: $0.production_years)) },
            engineCode: definition.flatMap { nonempty(string(from: $0.engine_code)) },
            engineFamily: definition.flatMap { nonempty(string(from: $0.engine_family)) },
            displacementCC: definition.flatMap { $0.displacement_cc == 0 ? nil : $0.displacement_cc },
            ratedPowerKW: definition.flatMap { $0.rated_power_kw == 0 ? nil : $0.rated_power_kw },
            fuel: definition.flatMap {
                nonempty(string(from: mblink_mercedes_fuel_type_name($0.fuel))).map {
                    $0.prefix(1).uppercased() + $0.dropFirst()
                }
            },
            plant: plant.flatMap { nonempty(string(from: $0.plant)) },
            country: plant.flatMap { nonempty(string(from: $0.country)) },
            steering: nonempty(steering).map {
                $0.prefix(1).uppercased() + $0.dropFirst()
            },
            serialNumber: nonempty(serial))
    }

    private func refresh() {
        statusText = controller.statusText
        peripheralName = controller.peripheralName ?? "No adapter"
        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        mercedesProbeStatusText = controller.mercedesProbeStatusText
        mercedesProbeEndpointText = controller.mercedesProbeEndpointText ?? "Source-corroborated endpoint not selected"
        let capturedVIN = controller.mercedesVINText
        mercedesVINText = capturedVIN ?? "Not captured"
        vehicleIdentity = decodeVehicleIdentity(vin: capturedVIN)
        mercedesIdentitySummaryText = controller.mercedesIdentitySummaryText
        mercedesIdentityResults = controller.mercedesIdentityResults
        mercedesCrd3SummaryText = controller.mercedesCrd3SummaryText
        mercedesUDSFaultStatusText = controller.mercedesUDSFaultStatusText
        mercedesUDSFaults = controller.mercedesUDSFaults
        vehicleProfileStatusText = controller.vehicleProfileStatusText
        faultScanStatusText = controller.faultScanStatusText

        let rawStoredDTCs = controller.storedDTCs
        let rawPendingDTCs = controller.pendingDTCs
        let rawPermanentDTCs = controller.permanentDTCs
        storedFaults = resolveFaults(rawStoredDTCs, state: "Stored")
        pendingFaults = resolveFaults(rawPendingDTCs, state: "Pending")
        permanentFaults = resolveFaults(rawPermanentDTCs, state: "Permanent")
        storedDTCs = storedFaults.map(\.displayText)
        pendingDTCs = pendingFaults.map(\.displayText)
        permanentDTCs = permanentFaults.map(\.displayText)

        isActive = controller.isActive
        isReady = controller.isReady
        diagnosticParameters = loadDiagnosticParameters()
        recordedSampleCount = Int(clamping: controller.recordedSampleCount)
    }
}
