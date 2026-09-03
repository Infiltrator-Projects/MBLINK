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
    let structuredValue: String?
    let rawHex: String?
    let vehicleSupported: Bool
    let favourite: Bool
    let pollingEnabled: Bool
    let history: [Double]
    let sourceLabel: String?
    let qualityNote: String?

    var isAvailable: Bool { value != nil || !(structuredValue ?? "").isEmpty }
    var isSupported: Bool { vehicleSupported }

    /*
     * Never expose a bare "N/A" for an ordinary catalogue state. A missing
     * sample can mean three very different things: the vehicle did not
     * advertise the PID, the user has not enabled polling, or polling is
     * enabled but the first sample has not arrived yet.
     */
    var presentationValue: String {
        if value != nil {
            return formattedValue == "N/A" ? "Decode error" : formattedValue
        }
        if let structuredValue, !structuredValue.isEmpty { return structuredValue }
        if !vehicleSupported { return "Not advertised" }
        if !pollingEnabled { return "Not polled" }
        return "Waiting for sample"
    }

    var hasLiveValue: Bool { pollingEnabled && isAvailable }
}

struct DiagnosticModule: Identifiable {
    let id: String
    let name: String
    let designation: String
    let network: String
    let kind: String
    let protocolName: String
    let requestCANIdentifier: UInt32
    let responseCANIdentifier: UInt32
    let extendedID: Bool
    let identityText: String?
    let partNumber: String?
    let softwareNumber: String?
    let hardwareNumber: String?
    let faultStatus: String
    let faultCount: Int
    let faults: [String]
    let evidenceDetails: [String]
    let obdAdvertisedPIDCount: Int
    let livePIDCount: Int

    var addressText: String {
        if extendedID {
            return String(format: "0x%08X → 0x%08X",
                          requestCANIdentifier, responseCANIdentifier)
        }
        return String(format: "0x%03X → 0x%03X",
                      requestCANIdentifier, responseCANIdentifier)
    }

    var faultCountLabel: String {
        if faultCount > 0 { return "\(faultCount) fault\(faultCount == 1 ? "" : "s")" }
        if faultStatus == "Checked · no faults" { return "0 faults" }
        return "faults unknown"
    }
}

struct MercedesModuleDataValue: Identifiable {
    let id: String
    let moduleID: String
    let identifier: UInt16
    let service: UInt8
    let codeText: String
    let title: String
    let formattedValue: String
    let rawHex: String
    let mapped: Bool
    let unit: String?
    let numericValue: Double?

    var serviceName: String {
        switch service {
        case 0x22: return "UDS ReadDataByIdentifier"
        case 0x21: return "KWP2000 ReadDataByLocalIdentifier"
        default: return String(format: "Service 0x%02X", service)
        }
    }
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

struct MercedesNativeDataIdentity: Identifiable {
    let id: String
    let symbol: String
    let dataID: String
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
    @Published private(set) var readinessStatusText = "Not collected"
    @Published private(set) var readinessMonitorStatus = [String]()
    @Published private(set) var freezeFrameContext = [String]()
    @Published private(set) var diagnosticCapabilityText = "Unknown / probing"
    @Published private(set) var diagnosticCapabilityDetailText = ""
    @Published private(set) var standardResponderSummary = "0 physical responders"
    @Published private(set) var supportedPIDSummary = "0 advertised PIDs"
    @Published private(set) var standardVINText = "Unavailable / not yet read"
    @Published private(set) var standardLiveValueRows = [String]()
    @Published private(set) var isActive = false
    @Published private(set) var isReady = false
    @Published private(set) var isSimulationActive = false
    @Published private(set) var connectionAlertText: String?

    @Published private(set) var diagnosticParameters = [DiagnosticParameter]()
    @Published private(set) var dashboardParameters = [DiagnosticParameter]()
    @Published private(set) var diagnosticModules = [DiagnosticModule]()
    @Published private(set) var manufacturerDataScanActive = false
    @Published private(set) var manufacturerDataScanStatusText = "Not scanned"
    @Published private(set) var manufacturerDataScanModuleID: String?
    @Published private(set) var mercedesTargetSignals = [MercedesTargetSignal]()
    @Published private(set) var mercedesNativeDataIdentities = [MercedesNativeDataIdentity]()
    @Published private(set) var recordedSampleCount = 0
    @Published private(set) var csvExportURL: URL?
    @Published private(set) var isPreparingCSV = false

    private let controller = MBLinkDiagnosticsController()
    private var lastConnectionAlertText: String?

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

    var mercedesFaultScanComplete: Bool {
        mercedesUDSFaultStatusText.hasPrefix("Complete ·")
    }

    var mercedesFaultScanFailed: Bool {
        let value = mercedesUDSFaultStatusText.lowercased()
        return value.contains("partial") || value.contains("interrupted") ||
            value.contains("incomplete") || value.contains("failed")
    }

    var connectionPhaseTitle: String {
        let value = statusText.lowercased()
        if value.contains("retry") || value.contains("scanning") {
            return "Finding Bluetooth adapter"
        }
        if value.contains("connecting") || value.contains("discovering") ||
            value.contains("validating") {
            return "Opening adapter channel"
        }
        if value.contains("initial") || value.contains("elm327") {
            return "Initialising diagnostic adapter"
        }
        if value.contains("module") || value.contains("mercedes") ||
            value.contains("probe") {
            return "Reading Mercedes control units"
        }
        if value.contains("vin") || value.contains("fault") ||
            value.contains("pid") {
            return "Reading vehicle diagnostics"
        }
        return "Preparing diagnostic session"
    }

    override init() {
        super.init()
        applyStoredPollingPolicy()
        controller.delegate = self
        mercedesTargetSignals = loadMercedesTargetSignals()
        mercedesNativeDataIdentities = loadMercedesNativeDataIdentities()
        refresh()
    }

    func connect() {
        clearPreparedExport()
        if isActive { return }
        connectionAlertText = nil
        lastConnectionAlertText = nil

        let alert = UIAlertController(
            title: mblinkLocalized("Connection Test"),
            message: mblinkLocalized("Real Adapter supports ELM/Vgate BLE diagnostics and MB-1/8/9 Mercedes me BLE adapters. MB-2/3/4/5/6/7 adapters use Bluetooth Classic, which iPhone exposes only through an accessory-authorised External Accessory protocol. Simulated ELM327 runs the same diagnostic stack against an in-process byte-stream emulator."),
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

    func dismissConnectionAlert() {
        connectionAlertText = nil
    }

    func parameter(stableKey: String) -> DiagnosticParameter? {
        diagnosticParameters.first { $0.id == stableKey }
    }

    func mercedesSignals(category: String) -> [MercedesTargetSignal] {
        mercedesTargetSignals.filter { $0.category == category }
    }

    func diagnosticModule(id: String) -> DiagnosticModule? {
        diagnosticModules.first { $0.id == id }
    }

    func moduleParameters(moduleID: String) -> [DiagnosticParameter] {
        guard let module = diagnosticModule(id: moduleID) else { return [] }
        return loadDiagnosticParameters(
            responderCANIdentifier: module.responseCANIdentifier,
            extendedID: module.extendedID,
            sourceLabel: "\(module.name) · \(module.addressText)")
    }

    func manufacturerData(moduleID: String) -> [MercedesModuleDataValue] {
        controller.manufacturerDataSnapshots(forModuleIdentifier: moduleID)
            .map { snapshot in
                let code = snapshot.codeText
                let title = snapshot.name ?? code
                return MercedesModuleDataValue(
                    id: "\(moduleID):\(snapshot.service):\(snapshot.identifier)",
                    moduleID: moduleID,
                    identifier: snapshot.identifier,
                    service: snapshot.service,
                    codeText: code,
                    title: title,
                    formattedValue: snapshot.formattedValue,
                    rawHex: snapshot.rawHex,
                    mapped: snapshot.isMapped,
                    unit: snapshot.unit,
                    numericValue: snapshot.isNumericValueAvailable
                        ? snapshot.numericValue : nil)
            }
    }

    func discoverManufacturerData(moduleID: String) {
        guard isActive else { return }
        controller.discoverManufacturerData(
            forModuleIdentifier: moduleID)
        refresh()
    }

    func rescanManufacturerData(moduleID: String) {
        guard isActive else { return }
        controller.rescanManufacturerData(
            forModuleIdentifier: moduleID)
        refresh()
    }

    func toggleFavourite(stableKey: String) {
        guard let pid = pidForStableKey(stableKey) else { return }
        controller.setFavourite(!controller.favourite(forPID: pid), forPID: pid)
        refresh()
    }

    func setPolling(_ enabled: Bool, stableKey: String) {
        guard let pid = pidForStableKey(stableKey) else { return }
        var enabledKeys = storedPollingKeys()
        if enabled { enabledKeys.insert(stableKey) } else { enabledKeys.remove(stableKey) }
        UserDefaults.standard.set(Array(enabledKeys).sorted(), forKey: Self.pollingDefaultsKey)
        controller.setPollingEnabled(enabled, forPID: pid)
        refresh()
    }

    func refreshPresentation() {
        diagnosticParameters = loadPrimaryDiagnosticParameters()
        dashboardParameters = loadDashboardParameters()
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

    private func standardStableKey(for pid: UInt8) -> String {
        if let scalar = mblink_parameter_obd2_definition(pid) {
            let key = string(from: scalar.pointee.stable_key)
            if !key.isEmpty { return key }
        }
        return String(format: "sae.obd2.mode01.%02X", pid)
    }

    private func pidForStableKey(_ stableKey: String) -> UInt8? {
        if let parameter = diagnosticParameters.first(where: { $0.id == stableKey }) {
            return UInt8(exactly: parameter.parameterIdentifier)
        }
        if stableKey.hasPrefix("sae.obd2.mode01."),
           let value = UInt8(stableKey.suffix(2), radix: 16) { return value }
        return stableKey.withCString { key in
            guard let definition = mblink_parameter_obd2_definition_for_stable_key(key) else { return nil }
            return UInt8(exactly: definition.pointee.key.identifier)
        }
    }

    private func applyStoredPollingPolicy() {
        let enabled = storedPollingKeys()
        let count = mblink_obd2_pid_definition_count()
        guard count > 0 else { return }
        for index in 0..<count {
            guard let definition = mblink_obd2_pid_definition_at(index) else { continue }
            let metadata = definition.pointee
            guard metadata.mode == 0x01 else { continue }
            let pid = metadata.pid
            guard (pid & 0x1F) != 0 else { continue }
            controller.setPollingEnabled(
                enabled.contains(standardStableKey(for: pid)), forPID: pid)
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
        if !known && knowledge.origin == LINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC {
            var mercedes = MblinkMercedesReferenceDtcKnowledge()
            let referenceFound = code.withCString {
                mblink_mercedes_reference_dtc_resolve($0, &mercedes)
            }
            if referenceFound {
                let referenceTitle = stringFromFixedCString(mercedes.title)
                let referenceArea = stringFromFixedCString(mercedes.area)
                let referenceSource = stringFromFixedCString(mercedes.source)
                return DiagnosticFault(
                    code: normalizedCode.isEmpty ? code : normalizedCode,
                    title: referenceTitle.isEmpty
                        ? "Mercedes reference definition unavailable"
                        : referenceTitle,
                    system: system,
                    category: referenceArea.isEmpty ? "Mercedes reference" : referenceArea,
                    origin: mercedes.ambiguous
                        ? "Mercedes manufacturer reference · ambiguous"
                        : "Mercedes manufacturer reference",
                    source: referenceSource.isEmpty
                        ? "Supplied Mercedes reference catalogue"
                        : referenceSource,
                    state: state,
                    definitionKnown: !mercedes.ambiguous
                )
            }
        }

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

    private func loadDiagnosticParameters(
        responderCANIdentifier: UInt32? = nil,
        extendedID: Bool = false,
        sourceLabel: String? = nil
    ) -> [DiagnosticParameter] {
        let count = mblink_obd2_pid_definition_count()
        guard count > 0 else { return [] }
        var result = [DiagnosticParameter]()
        result.reserveCapacity(Int(count))

        for index in 0..<count {
            guard let catalogueDefinition = mblink_obd2_pid_definition_at(index) else { continue }
            let catalogue = catalogueDefinition.pointee
            guard catalogue.mode == 0x01 else { continue }
            let pid = catalogue.pid
            // 00/20/.../E0 are support bitmaps, not user-selectable live values.
            guard (pid & 0x1F) != 0 else { continue }

            let scalarDefinition = mblink_parameter_obd2_definition(pid)
            let rawHistory: [Double]
            if scalarDefinition != nil {
                if let responderCANIdentifier {
                    rawHistory = controller.recentValues(
                        forPID: pid,
                        responderCANIdentifier: responderCANIdentifier,
                        extendedID: extendedID,
                        limit: 60).map(\.doubleValue)
                } else {
                    rawHistory = controller.recentValues(forPID: pid, limit: 60).map(\.doubleValue)
                }
            } else {
                rawHistory = []
            }

            let snapshot: MBLinkStandardDataSnapshot?
            if let responderCANIdentifier {
                snapshot = controller.standardDataSnapshot(
                    forPID: pid,
                    responderCANIdentifier: responderCANIdentifier,
                    extendedID: extendedID)
            } else {
                snapshot = controller.standardDataSnapshot(forPID: pid)
            }

            let rawValue = rawHistory.last
            let stableKey = standardStableKey(for: pid)
            let title: String
            let shortName: String
            let suffix: String
            let history: [Double]
            let value: Double?
            let formatted: String

            if let scalarDefinition {
                title = string(from: scalarDefinition.pointee.name)
                shortName = string(from: scalarDefinition.pointee.short_name)
                suffix = displaySuffix(definition: scalarDefinition)
                history = rawHistory.map { displayScalar(definition: scalarDefinition, rawValue: $0) }
                value = rawValue.map { displayScalar(definition: scalarDefinition, rawValue: $0) }
                formatted = formattedValue(definition: scalarDefinition, value: rawValue)
            } else {
                title = string(from: catalogue.name)
                shortName = String(format: "PID %02X", pid)
                let unit = string(from: catalogue.unit)
                suffix = unit.isEmpty ? "" : " \(unit)"
                history = []
                value = nil
                formatted = snapshot?.formattedValue ?? "N/A"
            }

            let vehicleSupported: Bool
            if let responderCANIdentifier {
                vehicleSupported = controller.observedPIDs(
                    forResponderCANIdentifier: responderCANIdentifier,
                    extendedID: extendedID).contains { $0.uint8Value == pid }
            } else {
                vehicleSupported = controller.supportsPID(pid)
            }

            let qualityNote: String?
            if responderCANIdentifier != nil && pid == 0x2F,
               let rawValue, rawValue >= 99.5 {
                qualityNote = "ECU reported 100%; value retained without correction"
            } else if scalarDefinition == nil && snapshot != nil {
                qualityNote = "Structured SAE value · full payload retained by LINK"
            } else {
                qualityNote = nil
            }

            result.append(DiagnosticParameter(
                id: stableKey,
                protocolName: "obd2",
                moduleIdentifier: 0,
                parameterIdentifier: UInt32(pid),
                shortName: shortName,
                title: title,
                suffix: suffix,
                formattedValue: formatted,
                value: value,
                structuredValue: scalarDefinition == nil ? snapshot?.formattedValue : nil,
                rawHex: snapshot?.rawHex,
                vehicleSupported: vehicleSupported,
                favourite: controller.favourite(forPID: pid),
                pollingEnabled: controller.pollingEnabled(forPID: pid),
                history: history,
                sourceLabel: sourceLabel,
                qualityNote: qualityNote))
        }
        return result
    }

    /*
     * The top-level Table/Graphs surfaces must never use an aggregate
     * "latest PID" stream, because several physical OBD responders can return
     * the same PID with different legitimate values. Use one exact responder
     * for those generic surfaces (0x7E8 when present); every other responder
     * remains available through its own module screen.
     */
    private func loadPrimaryDiagnosticParameters() -> [DiagnosticParameter] {
        let primary = diagnosticModules.first(where: {
            !$0.extendedID && $0.responseCANIdentifier == 0x7E8 &&
                $0.livePIDCount > 0
        }) ?? diagnosticModules.first(where: { $0.livePIDCount > 0 })

        guard let primary else {
            return loadDiagnosticParameters()
        }
        return loadDiagnosticParameters(
            responderCANIdentifier: primary.responseCANIdentifier,
            extendedID: primary.extendedID,
            sourceLabel: "\(primary.name) · \(primary.addressText)")
    }

    private func loadDashboardParameters() -> [DiagnosticParameter] {
        var parameters: [DiagnosticParameter]
        if let engine = diagnosticModules.first(where: {
            !$0.extendedID && $0.responseCANIdentifier == 0x7E8 &&
                $0.livePIDCount > 0
        }) {
            parameters = loadDiagnosticParameters(
                responderCANIdentifier: engine.responseCANIdentifier,
                extendedID: engine.extendedID,
                sourceLabel: "\(engine.name) · \(engine.addressText)")
        } else {
            parameters = diagnosticParameters
        }

        /*
         * Source-corroborated Mercedes extended PID 21 30 on 7E1/7E9.
         * The controller performs one automatic evidence-gated read after the
         * module census. Once a real positive response exists, surface it on
         * the same dashboard as SAE values without pretending it is Mode 01.
         */
        if let transmissionModule = diagnosticModules.first(where: {
            !$0.extendedID &&
                $0.requestCANIdentifier == 0x7E1 &&
                $0.responseCANIdentifier == 0x7E9
        }),
           let temperature = manufacturerData(moduleID: transmissionModule.id)
                .first(where: {
                    $0.service == 0x21 &&
                    $0.identifier == 0x30 &&
                    $0.mapped &&
                    $0.numericValue != nil
                }),
           let celsius = temperature.numericValue {
            let displayedValue: Double
            let suffix: String
            if unitProfile == .usCustomary {
                displayedValue = celsius * 9.0 / 5.0 + 32.0
                suffix = " °F"
            } else {
                displayedValue = celsius
                suffix = " °C"
            }

            parameters.append(DiagnosticParameter(
                id: "mercedes.transmission.oil_temperature",
                protocolName: "kwp2000",
                moduleIdentifier: 0x7E1,
                parameterIdentifier: 0x2130,
                shortName: "ATF",
                title: "Transmission oil temperature",
                suffix: suffix,
                formattedValue: String(format: "%.1f%@", displayedValue, suffix),
                value: displayedValue,
                structuredValue: nil,
                rawHex: temperature.rawHex,
                vehicleSupported: true,
                favourite: false,
                pollingEnabled: true,
                history: [],
                sourceLabel: "\(transmissionModule.name) · \(transmissionModule.addressText)",
                qualityNote: "Mercedes extended PID 0x2130 · source-corroborated; this value is shown only after a real positive response"))
        }

        return parameters
    }

    private func loadDiagnosticModules() -> [DiagnosticModule] {
        controller.mercedesModuleSnapshots.map { snapshot in
            let advertised = controller.observedPIDs(
                forResponderCANIdentifier: snapshot.responseCANIdentifier,
                extendedID: snapshot.isExtendedID)
            let advertisedSet = Set(advertised.map(\.uint8Value))
            var selectableCount = 0
            for pid in advertisedSet {
                guard (pid & 0x1F) != 0,
                      mblink_obd2_pid_definition(0x01, pid) != nil else { continue }
                selectableCount += 1
            }

            return DiagnosticModule(
                id: snapshot.identifier,
                name: snapshot.name,
                designation: snapshot.designation,
                network: snapshot.network,
                kind: snapshot.kind,
                protocolName: snapshot.protocolName,
                requestCANIdentifier: snapshot.requestCANIdentifier,
                responseCANIdentifier: snapshot.responseCANIdentifier,
                extendedID: snapshot.isExtendedID,
                identityText: snapshot.identityText,
                partNumber: snapshot.partNumber,
                softwareNumber: snapshot.softwareNumber,
                hardwareNumber: snapshot.hardwareNumber,
                faultStatus: snapshot.faultStatus,
                faultCount: Int(snapshot.faultCount),
                faults: snapshot.faults,
                evidenceDetails: snapshot.evidenceDetails,
                obdAdvertisedPIDCount: advertised.count,
                livePIDCount: selectableCount)
        }
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

    private func loadMercedesNativeDataIdentities() -> [MercedesNativeDataIdentity] {
        let count = Int(link_mercedes_me_data_id_count())
        guard count > 0 else { return [] }

        var result = [MercedesNativeDataIdentity]()
        result.reserveCapacity(count)
        for index in 0..<count {
            guard let definition = link_mercedes_me_data_id_at(index) else { continue }
            let symbol = string(from: definition.pointee.symbol)
            let dataID = string(from: definition.pointee.data_id)
            guard !symbol.isEmpty, !dataID.isEmpty else { continue }
            result.append(MercedesNativeDataIdentity(
                id: symbol,
                symbol: symbol,
                dataID: dataID))
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
        let updatedStatus = controller.statusText
        statusText = updatedStatus
        let isTransportBoundary =
            updatedStatus.contains("Bluetooth Classic Mercedes adapter") ||
            updatedStatus.contains("No compatible BLE diagnostic adapter found")
        if isTransportBoundary && updatedStatus != lastConnectionAlertText {
            lastConnectionAlertText = updatedStatus
            connectionAlertText = updatedStatus
        }
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
        readinessStatusText = controller.readinessStatusText
        readinessMonitorStatus = controller.readinessMonitorStatus
        freezeFrameContext = controller.freezeFrameContext
        diagnosticCapabilityText = controller.diagnosticCapabilityText
        diagnosticCapabilityDetailText = controller.diagnosticCapabilityDetailText
        standardResponderSummary = controller.standardResponderSummary
        supportedPIDSummary = controller.supportedPIDSummary
        standardVINText = controller.standardVINText
        standardLiveValueRows = controller.standardLiveValueRows

        isActive = controller.isActive
        isReady = controller.isReady
        diagnosticModules = loadDiagnosticModules()
        diagnosticParameters = loadPrimaryDiagnosticParameters()
        manufacturerDataScanActive = controller.isManufacturerDataScanActive
        manufacturerDataScanStatusText = controller.manufacturerDataScanStatusText
        manufacturerDataScanModuleID = controller.manufacturerDataScanModuleIdentifier
        dashboardParameters = loadDashboardParameters()
        recordedSampleCount = Int(clamping: controller.recordedSampleCount)
    }
}
