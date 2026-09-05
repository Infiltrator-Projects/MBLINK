// SPDX-License-Identifier: GPL-3.0-or-later
import Combine
import Foundation
import UIKit


typealias DiagnosticParameter = LinkDiagnosticParameter

typealias DiagnosticModule = LinkDiagnosticModule

typealias PIDConfigurationItem = LinkPIDConfigurationItem

typealias SavedVehicleProfileSummary = LinkSavedVehicleProfileSummary

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

typealias DiagnosticFault = LinkDiagnosticFault

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
    let ratedTorqueNM: UInt32?
    let fuel: String?
    let plant: String?
    let country: String?
    let steering: String?
    let serialNumber: String?
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
    @Published private(set) var pidConfigurationModules = [DiagnosticModule]()
    @Published private(set) var pidConfigurationSourceText =
        "Connect once to learn which PIDs each controller supports"
    @Published private(set) var savedVehicleProfiles = [SavedVehicleProfileSummary]()
    @Published private(set) var selectedVehicleVIN: String?
    @Published private(set) var manufacturerDataScanActive = false
    @Published private(set) var manufacturerDataScanStatusText = "Not scanned"
    @Published private(set) var manufacturerDataScanModuleID: String?
    @Published private(set) var mercedesTargetSignals = [MercedesTargetSignal]()
    @Published private(set) var mercedesNativeDataIdentities = [MercedesNativeDataIdentity]()
    @Published private(set) var recordedSampleCount = 0
    @Published private(set) var csvExportURL: URL?
    @Published private(set) var isPreparingCSV = false
    @Published private(set) var languageTags = [String]()
    @Published private(set) var languageNames = [String]()
    @Published private(set) var selectedLanguageID = "en-AU"
    @Published private(set) var measurementKeys = [String]()
    @Published private(set) var measurementNames = [String]()
    @Published private(set) var selectedMeasurementID = "metric"

    private let controller = MBLinkDiagnosticsController()
    private let vehicleProfileStore = LinkVehicleProfileStore(
        productNamespace: "mblink",
        legacyProfileKey: "mblink.vehicleProfiles.v1",
        legacySelectedVINKey: "mblink.selectedVehicleVIN.v1",
        legacyAdapterMappingKey: "mblink.adapterPeripheralByVehicle.v1")
    private let pidSelectionStore = LinkPIDSelectionStore(
        productNamespace: "mblink",
        legacyGlobalKey: "mblink.polling.enabledStableKeys.v2",
        legacyVehicleKey: "mblink.pidSelectionsByVehicle.v1")
    private var lastConnectionAlertText: String?
    private var manufacturerNumericHistory = [String: [Double]]()
    private var manufacturerLastRawByParameter = [String: String]()

    /*
     * v2 changes first-run policy from an automatic core set to explicit
     * opt-in. Existing user choices are preserved, but the old untouched
     * automatic default is recognised and migrated to an empty selection.
     */
    private static let legacyPollingDefaultsKey = "mblink.polling.enabledStableKeys.v1"
    private var pidSupportByModule = [String: Set<UInt8>]()
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
        migrateLegacySharedSettings()
        selectedVehicleVIN = vehicleProfileStore.selectedVehicleVIN
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

        guard let presenter = presentingViewController() else {
            beginConnection(.automatic)
            return
        }

        let currentVehicleText: String
        if let identity = vehicleIdentity,
           let model = identity.model, !model.isEmpty {
            currentVehicleText = "\(model) · \(identity.vin)"
        } else if let vin = selectedVehicleVIN {
            currentVehicleText = vin
        } else {
            currentVehicleText = "No saved vehicle loaded"
        }

        let picker = LinkConnectionPickerViewController(
            vehicleText: currentVehicleText,
            knownAdapterIdentifier: associatedAdapterIdentifier(
                for: selectedVehicleVIN)
        ) { [weak self] source in
            Task { @MainActor [weak self] in
                self?.beginConnection(source)
            }
        }
        let navigation = UINavigationController(rootViewController: picker)
        navigation.modalPresentationStyle = .pageSheet
        presenter.present(navigation, animated: true)
    }

    private func beginConnection(_ source: LinkConnectionSource) {
        guard !isActive else { return }
        switch source {
        case .automatic:
            isSimulationActive = false
            controller.start()
        case .simulated:
            isSimulationActive = true
            controller.startSimulated()
        case .peripheral(let identifier):
            isSimulationActive = false
            controller.start(withPeripheralIdentifier: identifier)
        }
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
        guard let module =
            diagnosticModule(id: moduleID) ?? pidConfigurationModule(id: moduleID)
        else { return [] }

        let selected = modulePIDSelectionSet(moduleID: moduleID)
        return loadDiagnosticParameters(
            responderCANIdentifier: module.responseCANIdentifier,
            extendedID: module.extendedID,
            sourceLabel: "\(module.name) · \(module.addressText)")
            .map { parameter in
                DiagnosticParameter(
                    id: parameter.id,
                    protocolName: parameter.protocolName,
                    moduleIdentifier: parameter.moduleIdentifier,
                    parameterIdentifier: parameter.parameterIdentifier,
                    shortName: parameter.shortName,
                    title: parameter.title,
                    suffix: parameter.suffix,
                    formattedValue: parameter.formattedValue,
                    value: parameter.value,
                    structuredValue: parameter.structuredValue,
                    rawHex: parameter.rawHex,
                    vehicleSupported: parameter.vehicleSupported,
                    favourite: parameter.favourite,
                    pollingEnabled: selected.contains(parameter.id),
                    history: parameter.history,
                    sourceLabel: parameter.sourceLabel,
                    qualityNote: parameter.qualityNote)
            }
    }

    func pidConfigurationModule(id: String) -> DiagnosticModule? {
        pidConfigurationModules.first { $0.id == id }
    }

    func selectSavedVehicle(vin: String) {
        // A live VIN is authoritative. Saved-profile selection is an offline
        // operation and must never override the physical car.
        guard !controller.isActive,
              vehicleProfileStore.selectOfflineVehicle(withVIN: vin) else { return }
        selectedVehicleVIN = vin
        mercedesVINText = vin
        vehicleIdentity = decodeVehicleIdentity(vin: vin)
        vehicleProfileStatusText = "Saved vehicle profile loaded · offline"
        mercedesIdentitySummaryText = "Saved vehicle profile · offline"
        mercedesProbeStatusText = "Disconnected · saved vehicle profile"
        refreshPIDConfiguration()
        applyConfiguredPollingForSelectedVehicle()
        refreshPresentation()
    }

    func pidConfigurationItems(moduleID: String) -> [PIDConfigurationItem] {
        let advertised = pidSupportByModule[moduleID] ?? []
        let selected = modulePIDSelectionSet(moduleID: moduleID)
        let count = mblink_obd2_pid_definition_count()
        guard count > 0, !advertised.isEmpty else { return [] }

        var result = [PIDConfigurationItem]()
        for index in 0..<count {
            guard let definition = mblink_obd2_pid_definition_at(index) else { continue }
            let metadata = definition.pointee
            guard metadata.mode == 0x01 else { continue }
            let pid = metadata.pid
            // 00/20/40/... are bitmap capability queries, not user data values.
            guard (pid & 0x1F) != 0 else { continue }
            // Controller pages are responder-scoped, not copies of the global
            // SAE catalogue. Only values positively advertised/observed on this
            // exact CAN responder belong in this module.
            guard advertised.contains(pid) else { continue }

            let scalar = mblink_parameter_obd2_definition(pid)
            let title = scalar != nil
                ? string(from: scalar!.pointee.name)
                : string(from: metadata.name)
            let shortName = scalar != nil
                ? string(from: scalar!.pointee.short_name)
                : String(format: "PID %02X", pid)
            let stableKey = standardStableKey(for: pid)
            result.append(PIDConfigurationItem(
                id: stableKey,
                pid: pid,
                shortName: shortName,
                title: title,
                pollingEnabled: selected.contains(stableKey),
                favourite: controller.favourite(forPID: pid),
                advertised: true))
        }
        return result.sorted {
            if $0.pid != $1.pid { return $0.pid < $1.pid }
            return $0.title < $1.title
        }
    }

    func setPIDSelection(
        _ enabled: Bool,
        moduleID: String,
        stableKey: String
    ) {
        guard let pid = pidForStableKey(stableKey),
              (pidSupportByModule[moduleID] ?? []).contains(pid)
        else { return }
        var selection = modulePIDSelectionSet(moduleID: moduleID)
        if enabled { selection.insert(stableKey) }
        else { selection.remove(stableKey) }
        storeModulePIDSelection(selection, moduleID: moduleID)
        applyConfiguredPollingForSelectedVehicle()
        refreshPresentation()
        refreshPIDConfiguration()
    }

    func setPolling(_ enabled: Bool, moduleID: String) {
        let items = pidConfigurationItems(moduleID: moduleID)
        guard !items.isEmpty else { return }
        let selection = enabled ? Set(items.map(\.id)) : Set<String>()
        storeModulePIDSelection(selection, moduleID: moduleID)
        applyConfiguredPollingForSelectedVehicle()
        refreshPresentation()
        refreshPIDConfiguration()
    }

    var configuredPollingCount: Int {
        guard effectivePIDConfigurationVIN != nil else {
            return storedPollingKeys().count
        }
        return Set(pidConfigurationModules.flatMap {
            modulePIDSelectionSet(moduleID: $0.id)
        }).count
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

    func manufacturerLivePollingSupported(moduleID: String) -> Bool {
        controller.manufacturerLivePollingSupported(
            forModuleIdentifier: moduleID)
    }

    func manufacturerLivePollingEnabled(moduleID: String) -> Bool {
        controller.manufacturerLivePollingEnabled(
            forModuleIdentifier: moduleID)
    }

    func setManufacturerLivePolling(_ enabled: Bool, moduleID: String) {
        controller.setManufacturerLivePollingEnabled(
            enabled, forModuleIdentifier: moduleID)
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
        pidSelectionStore.setGlobalStableKeys(Array(enabledKeys).sorted())
        controller.setPollingEnabled(enabled, forPID: pid)
        refresh()
    }

    func refreshPresentation() {
        diagnosticParameters = loadPrimaryDiagnosticParameters()
        dashboardParameters = loadDashboardParameters()
    }

    var interfaceLocaleIdentifier: String {
        LinkInterfaceLanguage.canonical(
            selectedLanguageID, aliases: mbLegacyLanguageAliases)
    }

    func localizedText(_ key: String) -> String {
        controller.localizedText(forKey: key)
    }

    func selectLanguage(_ id: String) {
        controller.setSelectedLanguageTag(id)
        refresh()
    }

    func selectMeasurementSystem(_ id: String) {
        controller.setSelectedMeasurementSystemKey(id)
        refresh()
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

        isPreparingCSV = true

        Task { [weak self] in
            do {
                let url = try await LinkEvidenceExport.prepareTemporaryCSV(
                    data, productName: "MBLINK")
                guard let self else {
                    LinkEvidenceExport.removeTemporaryFile(url)
                    return
                }
                self.clearPreparedExport()
                self.csvExportURL = url
            } catch {
                // Export remains unavailable; live diagnostics continue.
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
        LinkEvidenceExport.removeTemporaryFile(csvExportURL)
        csvExportURL = nil
    }

    private var effectivePIDConfigurationVIN: String? {
        // The controller deliberately retains its last VIN after disconnect.
        // It is authoritative only while a live diagnostic session is active.
        if controller.isActive,
           let liveVIN = controller.mercedesVINText,
           liveVIN.count == 17 {
            return liveVIN
        }
        return selectedVehicleVIN
    }

    private func supportedStableKeys(moduleID: String) -> Set<String> {
        let advertised = pidSupportByModule[moduleID] ?? []
        return Set(advertised.compactMap { pid in
            guard (pid & 0x1F) != 0,
                  mblink_obd2_pid_definition(0x01, pid) != nil
            else { return nil }
            return standardStableKey(for: pid)
        })
    }

    private func modulePIDSelectionSet(moduleID: String) -> Set<String> {
        let supported = supportedStableKeys(moduleID: moduleID)
        guard !supported.isEmpty else { return [] }

        guard let vin = effectivePIDConfigurationVIN else {
            return Set(pidSelectionStore.globalStableKeys).intersection(supported)
        }
        if pidSelectionStore.hasSelection(
            forVIN: vin, controllerIdentifier: moduleID) {
            return Set(pidSelectionStore.stableKeys(
                forVIN: vin, controllerIdentifier: moduleID)).intersection(supported)
        }
        // A global legacy choice may seed only the exact responder that
        // advertised it; LINK owns persistence after that migration.
        return Set(pidSelectionStore.globalStableKeys).intersection(supported)
    }

    private func storeModulePIDSelection(
        _ selection: Set<String>,
        moduleID: String
    ) {
        let boundedSelection =
            selection.intersection(supportedStableKeys(moduleID: moduleID))

        guard let vin = effectivePIDConfigurationVIN else {
            pidSelectionStore.setGlobalStableKeys(Array(boundedSelection).sorted())
            return
        }

        pidSelectionStore.setStableKeys(
            Array(boundedSelection).sorted(),
            forVIN: vin,
            controllerIdentifier: moduleID)
    }

    private func applyConfiguredPollingForSelectedVehicle() {
        let selectedKeys: Set<String>
        if effectivePIDConfigurationVIN != nil {
            selectedKeys = Set(pidConfigurationModules.flatMap {
                modulePIDSelectionSet(moduleID: $0.id)
            })
        } else {
            selectedKeys = storedPollingKeys()
        }

        // Keep the legacy/global polling key in sync with the responder-bounded
        // union so the existing live-data engine polls each requested Mode 01
        // PID once without leaking unsupported selections between modules.
        pidSelectionStore.setGlobalStableKeys(Array(selectedKeys).sorted())

        let count = mblink_obd2_pid_definition_count()
        guard count > 0 else { return }
        for index in 0..<count {
            guard let definition = mblink_obd2_pid_definition_at(index) else { continue }
            let metadata = definition.pointee
            guard metadata.mode == 0x01 else { continue }
            let pid = metadata.pid
            guard (pid & 0x1F) != 0 else { continue }
            controller.setPollingEnabled(
                selectedKeys.contains(standardStableKey(for: pid)),
                forPID: pid)
        }
    }

    private func refreshSavedVehicleProfiles() {
        var summaries = [SavedVehicleProfileSummary]()
        for profile in vehicleProfileStore.savedProfiles {
            guard let vin = profile["vin"] as? String, vin.count == 17 else { continue }
            let modules = profile["modules"] as? [[String: Any]] ?? []
            let fallbackDisplayName =
                decodeVehicleIdentity(vin: vin)?.model ?? "Mercedes-Benz vehicle"
            if let summary = SavedVehicleProfileSummary(
                profile: profile as NSDictionary,
                moduleCount: modules.count,
                fallbackDisplayName: fallbackDisplayName) {
                summaries.append(summary)
            }
        }
        savedVehicleProfiles = summaries.sorted {
            ($0.updatedAt ?? .distantPast) > ($1.updatedAt ?? .distantPast)
        }

        if let selectedVehicleVIN,
           savedVehicleProfiles.contains(where: { $0.vin == selectedVehicleVIN }) {
            return
        }

        selectedVehicleVIN = nil
        vehicleProfileStore.clearSelectedVehicle()
    }

    private func storedPollingKeys() -> Set<String> {
        if pidSelectionStore.hasGlobalSelection {
            return Set(pidSelectionStore.globalStableKeys)
        }

        // One-time compatibility decision for the old automatic v1 set.
        // LINK owns all standard PID persistence after this point.
        let defaults = UserDefaults.standard
        var initial = Set<String>()
        if let legacyValues =
            defaults.array(forKey: Self.legacyPollingDefaultsKey) as? [String] {
            let legacy = Set(legacyValues)
            if legacy != Self.legacyAutomaticPollingStableKeys {
                initial = legacy
            }
        }

        pidSelectionStore.setGlobalStableKeys(Array(initial).sorted())
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


    private func migrateLegacySharedSettings() {
        let defaults = UserDefaults.standard

        if defaults.object(forKey: "link.displayLanguage") == nil,
           let legacy = defaults.string(forKey: "mblink.language") {
            controller.setSelectedLanguageTag(
                LinkInterfaceLanguage.canonical(
                    legacy, aliases: mbLegacyLanguageAliases))
        }

        if defaults.object(forKey: "link.measurementSystem") == nil,
           let legacy = defaults.string(forKey: "mblink.units") {
            controller.setSelectedMeasurementSystemKey(
                legacy == "us"
                    ? "us-customary" : "metric")
        }
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

    private func displayScalar(pid: UInt8, rawValue: Double) -> Double {
    controller.displayValue(pid: pid, canonicalValue: rawValue)
}

private func displaySuffix(
    pid: UInt8,
    definition: UnsafePointer<MblinkParameterDefinition>
) -> String {
    let unit = controller.displayUnit(pid: pid)
    if !unit.isEmpty { return " \(unit)" }
    return string(from: definition.pointee.suffix)
}

private func formattedValue(
    pid: UInt8,
    definition: UnsafePointer<MblinkParameterDefinition>,
    value: Double?
) -> String {
    guard let value else { return "N/A" }
    let displayed = displayScalar(pid: pid, rawValue: value)
    let suffix = displaySuffix(pid: pid, definition: definition)
    let places = Int(definition.pointee.decimal_places)
    return String(format: "%.*f%@", places, displayed, suffix)
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
                suffix = displaySuffix(pid: pid, definition: scalarDefinition)
                history = rawHistory.map { displayScalar(pid: pid, rawValue: $0) }
                value = rawValue.map { displayScalar(pid: pid, rawValue: $0) }
                formatted = formattedValue(pid: pid, definition: scalarDefinition, value: rawValue)
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
    private func manufacturerHistory(
        id: String,
        value: Double,
        rawHex: String
    ) -> [Double] {
        if manufacturerLastRawByParameter[id] != rawHex {
            manufacturerLastRawByParameter[id] = rawHex
            var history = manufacturerNumericHistory[id] ?? []
            history.append(value)
            if history.count > 240 {
                history.removeFirst(history.count - 240)
            }
            manufacturerNumericHistory[id] = history
        }
        return manufacturerNumericHistory[id] ?? []
    }

    private func manufacturerBytes(_ rawHex: String) -> [UInt8] {
        let hex = rawHex.filter { !$0.isWhitespace }
        guard hex.count >= 2, hex.count % 2 == 0 else { return [] }
        var result = [UInt8]()
        result.reserveCapacity(hex.count / 2)
        var index = hex.startIndex
        while index < hex.endIndex {
            let next = hex.index(index, offsetBy: 2)
            guard let byte = UInt8(hex[index..<next], radix: 16) else {
                return []
            }
            result.append(byte)
            index = next
        }
        return result
    }

    private func manufacturerBE16(_ bytes: [UInt8], _ offset: Int) -> UInt16? {
        guard offset >= 0, offset + 1 < bytes.count else { return nil }
        return (UInt16(bytes[offset]) << 8) | UInt16(bytes[offset + 1])
    }

    private func egs52GearText(_ code: UInt8) -> String {
        switch code {
        case 0: return "N"
        case 1...7: return "\(code)"
        case 8: return "D-CVT"
        case 9: return "R-CVT"
        case 10: return "R3"
        case 11: return "R"
        case 12: return "R2"
        case 13: return "P"
        case 14: return "No force"
        default: return "Unavailable"
        }
    }

    private func egs52RecognisedGearText(_ code: UInt8) -> String {
        switch code {
        case 0: return "Inactive"
        case 1...5: return "D\(code)"
        case 6: return "R"
        case 7: return "R2"
        case 23: return "Wrong gear"
        case 88: return "Calculating"
        case 255: return "Signal unavailable"
        default: return String(format: "Code 0x%02X", code)
        }
    }

    private func egs52TCCStateText(_ code: UInt8) -> String {
        switch code {
        case 0: return "Open"
        case 1: return "Open → slipping"
        case 2: return "Slipping → open"
        case 3: return "Slipping"
        case 4: return "Slipping → locked"
        case 5: return "Locked → slipping"
        case 6: return "Locked"
        default: return String(format: "Code 0x%02X", code)
        }
    }

    private func egs52ShiftValveText(_ code: UInt8) -> String {
        switch code {
        case 0: return "No shift valve"
        case 1: return "1-2/4-5"
        case 2: return "2-3"
        case 3: return "1-2/4-5 + 2-3"
        case 4: return "3-4"
        case 5: return "1-2/4-5 + 3-4"
        case 6: return "2-3 + 3-4"
        case 7: return "All shift valves"
        default: return String(format: "Code 0x%02X", code)
        }
    }

    private func transmissionDiagnosticParameters() -> [DiagnosticParameter] {
        guard let module = diagnosticModules.first(where: {
            !$0.extendedID &&
                $0.requestCANIdentifier == 0x7E1 &&
                $0.responseCANIdentifier == 0x7E9
        }) else { return [] }

        var values = [UInt16: MercedesModuleDataValue]()
        for value in manufacturerData(moduleID: module.id)
            where value.service == 0x21 {
            values[value.identifier] = value
        }
        let source = "\(module.name) · \(module.addressText)"
        var result = [DiagnosticParameter]()

        func addNumeric(
            id: String,
            identifier: UInt16,
            shortName: String,
            title: String,
            value: Double,
            suffix: String = "",
            rawHex: String,
            quality: String
        ) {
            result.append(DiagnosticParameter(
                id: id,
                protocolName: "kwp2000",
                moduleIdentifier: 0x7E1,
                parameterIdentifier: UInt32(0x2100) | UInt32(identifier),
                shortName: shortName,
                title: title,
                suffix: suffix,
                formattedValue: String(format: "%.6g%@", value, suffix),
                value: value,
                structuredValue: nil,
                rawHex: rawHex,
                vehicleSupported: true,
                favourite: false,
                pollingEnabled: true,
                history: manufacturerHistory(
                    id: id, value: value, rawHex: rawHex),
                sourceLabel: source,
                qualityNote: quality))
        }

        func addText(
            id: String,
            identifier: UInt16,
            shortName: String,
            title: String,
            text: String,
            rawHex: String,
            quality: String
        ) {
            result.append(DiagnosticParameter(
                id: id,
                protocolName: "kwp2000",
                moduleIdentifier: 0x7E1,
                parameterIdentifier: UInt32(0x2100) | UInt32(identifier),
                shortName: shortName,
                title: title,
                suffix: "",
                formattedValue: text,
                value: nil,
                structuredValue: text,
                rawHex: rawHex,
                vehicleSupported: true,
                favourite: false,
                pollingEnabled: true,
                history: [],
                sourceLabel: source,
                qualityNote: quality))
        }

        let dasQuality =
            "Mercedes EGS52/DAS-compatible RLI layout corroborated by public EGS52 emulation source"

        if let rli30 = values[0x30] {
            let b = manufacturerBytes(rli30.rawHex)
            if b.count >= 24 {
                if let raw = manufacturerBE16(b, 0) {
                    addNumeric(id: "mercedes.transmission.tcc_delta_speed",
                               identifier: 0x30, shortName: "TCC Δ",
                               title: "TCC delta speed (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli30.rawHex, quality: dasQuality)
                }
                if let raw = manufacturerBE16(b, 2) {
                    addNumeric(id: "mercedes.transmission.tcc_speed",
                               identifier: 0x30, shortName: "TCC SPD",
                               title: "TCC speed (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli30.rawHex, quality: dasQuality)
                }
                if let raw = manufacturerBE16(b, 4) {
                    addNumeric(id: "mercedes.transmission.tcc_pressure",
                               identifier: 0x30, shortName: "TCC P",
                               title: "TCC pressure (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli30.rawHex, quality: dasQuality)
                }
                addText(id: "mercedes.transmission.tcc_state",
                        identifier: 0x30, shortName: "TCC",
                        title: "Torque converter clutch state",
                        text: egs52TCCStateText(b[6]),
                        rawHex: rli30.rawHex, quality: dasQuality)
                addNumeric(id: "mercedes.transmission.selector_position",
                           identifier: 0x30, shortName: "SELECT",
                           title: "Selector position code",
                           value: Double(b[7]), suffix: " raw",
                           rawHex: rli30.rawHex, quality: dasQuality)
                addNumeric(id: "mercedes.transmission.drive_program",
                           identifier: 0x30, shortName: "PROGRAM",
                           title: "Transmission drive program code",
                           value: Double(b[8]), suffix: " raw",
                           rawHex: rli30.rawHex, quality: dasQuality)
                addText(id: "mercedes.transmission.recognised_gear",
                        identifier: 0x30, shortName: "REC GEAR",
                        title: "Recognised gear",
                        text: egs52RecognisedGearText(b[9]),
                        rawHex: rli30.rawHex, quality: dasQuality)

                let actual = b[10] & 0x0F
                let target = (b[10] >> 4) & 0x0F
                addText(id: "mercedes.transmission.actual_gear",
                        identifier: 0x30, shortName: "GEAR",
                        title: "Current gear",
                        text: egs52GearText(actual),
                        rawHex: rli30.rawHex, quality: dasQuality)
                addText(id: "mercedes.transmission.target_gear",
                        identifier: 0x30, shortName: "TARGET",
                        title: "Target gear",
                        text: egs52GearText(target),
                        rawHex: rli30.rawHex, quality: dasQuality)

                let celsius = Double(b[11]) - 50.0
                let displayedValue = controller.displayTemperature(celsius: celsius)
                let temperatureSuffix = " " + controller.displayTemperatureUnit()
                addNumeric(id: "mercedes.transmission.oil_temperature",
                           identifier: 0x30, shortName: "ATF",
                           title: "Transmission oil temperature",
                           value: displayedValue, suffix: temperatureSuffix,
                           rawHex: rli30.rawHex, quality: dasQuality)

                if let raw = manufacturerBE16(b, 12) {
                    addNumeric(id: "mercedes.transmission.engine_torque_raw",
                               identifier: 0x30, shortName: "ENG TQ",
                               title: "Engine torque value (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli30.rawHex, quality: dasQuality)
                }
                if let raw = manufacturerBE16(b, 14) {
                    addNumeric(id: "mercedes.transmission.converter_torque_raw",
                               identifier: 0x30, shortName: "CONV TQ",
                               title: "Converter torque value (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli30.rawHex, quality: dasQuality)
                }
                if let raw = manufacturerBE16(b, 16) {
                    addNumeric(id: "mercedes.transmission.output_speed_raw",
                               identifier: 0x30, shortName: "OUT SPD",
                               title: "Transmission output speed (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli30.rawHex, quality: dasQuality)
                }
            }
        }

        if let rli31 = values[0x31] {
            let b = manufacturerBytes(rli31.rawHex)
            if b.count >= 20 {
                let fields: [(String, String, String, Int, String)] = [
                    ("mercedes.transmission.n2_pulse_count", "N2", "N2 pulse count", 0, ""),
                    ("mercedes.transmission.n3_pulse_count", "N3", "N3 pulse count", 2, ""),
                    ("mercedes.transmission.input_rpm", "INPUT", "Transmission input speed", 4, " rpm"),
                    ("mercedes.transmission.engine_rpm", "ENG RPM", "Engine speed reported by TCU", 6, " rpm"),
                    ("mercedes.transmission.wheel_fl_raw", "FL WHEEL", "Front-left wheel speed (raw)", 8, " raw"),
                    ("mercedes.transmission.wheel_fr_raw", "FR WHEEL", "Front-right wheel speed (raw)", 10, " raw"),
                    ("mercedes.transmission.wheel_rl_raw", "RL WHEEL", "Rear-left wheel speed (raw)", 12, " raw"),
                    ("mercedes.transmission.wheel_rr_raw", "RR WHEEL", "Rear-right wheel speed (raw)", 14, " raw"),
                    ("mercedes.transmission.rear_vehicle_speed", "REAR SPD", "Vehicle speed from rear wheels", 16, " km/h"),
                    ("mercedes.transmission.front_vehicle_speed", "FRONT SPD", "Vehicle speed from front wheels", 18, " km/h")
                ]
                for field in fields {
                    if let raw = manufacturerBE16(b, field.3) {
                        addNumeric(id: field.0, identifier: 0x31,
                                   shortName: field.1, title: field.2,
                                   value: Double(raw), suffix: field.4,
                                   rawHex: rli31.rawHex, quality: dasQuality)
                    }
                }
            }
        }

        if let rli32 = values[0x32] {
            let b = manufacturerBytes(rli32.rawHex)
            if b.count >= 12 {
                addNumeric(id: "mercedes.transmission.pedal_percent",
                           identifier: 0x32, shortName: "PEDAL",
                           title: "Accelerator pedal position",
                           value: Double(b[0]), suffix: " %",
                           rawHex: rli32.rawHex, quality: dasQuality)
                if let raw = manufacturerBE16(b, 1) {
                    addNumeric(id: "mercedes.transmission.upshift_delta_rpm",
                               identifier: 0x32, shortName: "UP ΔRPM",
                               title: "Upshift RPM delta",
                               value: Double(raw), suffix: " rpm",
                               rawHex: rli32.rawHex, quality: dasQuality)
                }
                if let raw = manufacturerBE16(b, 3) {
                    addNumeric(id: "mercedes.transmission.downshift_delta_rpm",
                               identifier: 0x32, shortName: "DOWN ΔRPM",
                               title: "Downshift RPM delta",
                               value: Double(raw), suffix: " rpm",
                               rawHex: rli32.rawHex, quality: dasQuality)
                }
                addNumeric(id: "mercedes.transmission.pedal_delta_percent",
                           identifier: 0x32, shortName: "PEDAL Δ",
                           title: "Accelerator pedal change",
                           value: Double(b[5]), suffix: " %",
                           rawHex: rli32.rawHex, quality: dasQuality)
                if let raw = manufacturerBE16(b, 6) {
                    addNumeric(id: "mercedes.transmission.pitch_raw",
                               identifier: 0x32, shortName: "PITCH",
                               title: "Transmission pitch value (raw)",
                               value: Double(raw), suffix: " raw",
                               rawHex: rli32.rawHex, quality: dasQuality)
                }
                addNumeric(id: "mercedes.transmission.driving_status",
                           identifier: 0x32, shortName: "DRV STAT",
                           title: "Transmission driving-status code",
                           value: Double(b[8]), suffix: " raw",
                           rawHex: rli32.rawHex, quality: dasQuality)
                addNumeric(id: "mercedes.transmission.warmup_shift_state",
                           identifier: 0x32, shortName: "WARMUP",
                           title: "Warm-up shift-state code",
                           value: Double(b[9]), suffix: " raw",
                           rawHex: rli32.rawHex, quality: dasQuality)
                addNumeric(id: "mercedes.transmission.low_gear_limit",
                           identifier: 0x32, shortName: "LOW LIM",
                           title: "Requested low gear-range limit",
                           value: Double(b[10]), suffix: " raw",
                           rawHex: rli32.rawHex, quality: dasQuality)
                addNumeric(id: "mercedes.transmission.high_gear_limit",
                           identifier: 0x32, shortName: "HIGH LIM",
                           title: "Requested high gear-range limit",
                           value: Double(b[11]), suffix: " raw",
                           rawHex: rli32.rawHex, quality: dasQuality)
            }
        }

        if let rli33 = values[0x33] {
            let b = manufacturerBytes(rli33.rawHex)
            if b.count >= 15 {
                addNumeric(id: "mercedes.transmission.valve_flag",
                           identifier: 0x33, shortName: "VALVE",
                           title: "Shift-valve active flag",
                           value: Double(b[0]), suffix: " raw",
                           rawHex: rli33.rawHex, quality: dasQuality)
                addText(id: "mercedes.transmission.shift_valve_state",
                        identifier: 0x33, shortName: "SHIFT VALVE",
                        title: "Shift-valve state",
                        text: egs52ShiftValveText(b[1]),
                        rawHex: rli33.rawHex, quality: dasQuality)

                let fields: [(String, String, String, Int)] = [
                    ("mercedes.transmission.spc_pressure_raw", "SPC P", "SPC pressure (raw)", 2),
                    ("mercedes.transmission.mpc_pressure_raw", "MPC P", "MPC pressure (raw)", 4),
                    ("mercedes.transmission.spc_target_current_raw", "SPC TGT", "SPC target current (raw)", 6),
                    ("mercedes.transmission.spc_actual_current_raw", "SPC ACT", "SPC actual current (raw)", 8),
                    ("mercedes.transmission.mpc_target_current_raw", "MPC TGT", "MPC target current (raw)", 10),
                    ("mercedes.transmission.mpc_actual_current_raw", "MPC ACT", "MPC actual current (raw)", 12)
                ]
                for field in fields {
                    if let raw = manufacturerBE16(b, field.3) {
                        addNumeric(id: field.0, identifier: 0x33,
                                   shortName: field.1, title: field.2,
                                   value: Double(raw), suffix: " raw",
                                   rawHex: rli33.rawHex, quality: dasQuality)
                    }
                }
                addNumeric(id: "mercedes.transmission.tcc_pwm_raw",
                           identifier: 0x33, shortName: "TCC PWM",
                           title: "TCC PWM (raw)",
                           value: Double(b[14]), suffix: " raw",
                           rawHex: rli33.rawHex, quality: dasQuality)
            }
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

        var parameters: [DiagnosticParameter]
        if let primary {
            parameters = loadDiagnosticParameters(
                responderCANIdentifier: primary.responseCANIdentifier,
                extendedID: primary.extendedID,
                sourceLabel: "\(primary.name) · \(primary.addressText)")
        } else {
            parameters = loadDiagnosticParameters()
        }

        let existing = Set(parameters.map(\.id))
        parameters.append(contentsOf:
            transmissionDiagnosticParameters().filter {
                !existing.contains($0.id)
            })
        return parameters
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

        let existing = Set(parameters.map(\.id))
        parameters.append(contentsOf:
            transmissionDiagnosticParameters().filter {
                !existing.contains($0.id)
            })
        return parameters
    }

    private func offlineModuleName(
        tx: UInt32,
        rx: UInt32,
        extended: Bool,
        kind: Int
    ) -> String {
        if !extended && tx == 0x7E0 && rx == 0x7E8 { return "Engine ECU" }
        if !extended && tx == 0x7E1 && rx == 0x7E9 {
            return "Transmission ECU / GS"
        }

        // Persisted module-kind values are useful even when the exact family
        // identity has not been saved into the profile.
        switch kind {
        case 1: return "Engine control unit"
        case 2: return "Transmission control unit"
        case 3: return "ABS / ESP control unit"
        case 4: return "Airbag / restraint control unit"
        case 5: return "Instrument cluster"
        case 6: return "Body control unit"
        case 7: return "Gateway control unit"
        default:
            if extended {
                return String(format: "Mercedes ECU 0x%08X", tx)
            }
            return String(format: "Mercedes ECU 0x%03X", tx)
        }
    }

    private func loadSavedPIDConfiguration() -> (
        modules: [DiagnosticModule],
        support: [String: Set<UInt8>],
        label: String
    ) {
        guard !vehicleProfileStore.savedProfiles.isEmpty else {
            return ([], [:],
                    "Connect once to learn which PIDs each controller supports")
        }

        let liveVIN = controller.isActive ? controller.mercedesVINText : nil
        let selectedVIN: String? =
            (liveVIN?.count == 17 ? liveVIN : selectedVehicleVIN)

        // While connected, only the physical car's VIN may select a profile.
        // While offline, only the remembered/explicitly selected VIN may do so.
        guard let vin = selectedVIN,
              let profile = vehicleProfileStore.profile(forVIN: vin) as? [String: Any] else {
            let label = controller.isActive && liveVIN?.count == 17
                ? "New vehicle detected · learning controller map"
                : "No vehicle loaded · connect to a vehicle"
            return ([], [:], label)
        }

        var responderPIDs = [String: Set<UInt8>]()
for responder in LinkVehicleProfileStandardResponders(profile as NSDictionary) {
    let rx = responder.responderCANIdentifier
    let extended = responder.isExtendedID
    let key = String(format: "%@:%08X", extended ? "29" : "11", rx)
    let pids = responder.pids.compactMap { UInt8(exactly: $0.uintValue) }
    responderPIDs[key, default: []].formUnion(pids)
}

        var modules = [DiagnosticModule]()
        var support = [String: Set<UInt8>]()
        var seenResponderKeys = Set<String>()

        if let savedModules = profile["modules"] as? [[String: Any]] {
            for saved in savedModules {
                guard let txNumber = saved["tx"] as? NSNumber,
                      let rxNumber = saved["rx"] as? NSNumber else { continue }
                let tx = txNumber.uint32Value
                let rx = rxNumber.uint32Value
                let extended =
                    (saved["extended"] as? NSNumber)?.boolValue ?? false
                let kind = (saved["kind"] as? NSNumber)?.intValue ?? 0
                let moduleID = String(
                    format: "%@:%08X:%08X",
                    extended ? "29" : "11", tx, rx)
                let responderKey = String(
                    format: "%@:%08X", extended ? "29" : "11", rx)
                let pids = responderPIDs[responderKey] ?? []
                seenResponderKeys.insert(responderKey)
                support[moduleID] = pids

                let offlineName = (saved["name"] as? String) ??
                    offlineModuleName(
                        tx: tx, rx: rx, extended: extended, kind: kind)
                modules.append(DiagnosticModule(
                    id: moduleID,
                    name: offlineName,
                    designation: "Saved vehicle controller",
                    network: "Saved VIN profile",
                    kind: offlineName.lowercased(),
                    protocolName: (saved["protocolName"] as? String) ??
                        (tx == 0x7E1 ? "KWP2000 / SAE OBD-II" : "Saved diagnostic route"),
                    requestCANIdentifier: tx,
                    responseCANIdentifier: rx,
                    extendedID: extended,
                    identityText: saved["identity"] as? String,
                    partNumber: saved["sparePart"] as? String,
                    softwareNumber: saved["software"] as? String,
                    hardwareNumber: saved["hardware"] as? String,
                    faultStatus: "Saved vehicle profile",
                    faultCount: 0,
                    faults: [],
                    evidenceDetails: [],
                    obdAdvertisedPIDCount: pids.count,
                    livePIDCount: pids.filter {
                        ($0 & 0x1F) != 0 &&
                        mblink_obd2_pid_definition(0x01, $0) != nil
                    }.count))
            }
        }

        /*
         * A functional Mode 01 responder can be learned before Mercedes module
         * identity is available. Do not lose it from offline PID setup.
         */
        for (responderKey, pids) in responderPIDs
            where !seenResponderKeys.contains(responderKey) {
            let parts = responderKey.split(separator: ":")
            guard parts.count == 2,
                  let rx = UInt32(parts[1], radix: 16) else { continue }
            let extended = parts[0] == "29"
            let tx: UInt32
            if !extended && rx >= 8 { tx = rx - 8 }
            else { tx = rx }
            let moduleID = String(
                format: "%@:%08X:%08X",
                extended ? "29" : "11", tx, rx)
            support[moduleID] = pids
            modules.append(DiagnosticModule(
                id: moduleID,
                name: offlineModuleName(
                    tx: tx, rx: rx, extended: extended, kind: 0),
                designation: "Saved SAE OBD-II responder",
                network: "Saved VIN profile",
                kind: "saved",
                protocolName: "SAE Mode 01 / ISO 15765-4",
                requestCANIdentifier: tx,
                responseCANIdentifier: rx,
                extendedID: extended,
                identityText: nil,
                partNumber: nil,
                softwareNumber: nil,
                hardwareNumber: nil,
                faultStatus: "Saved vehicle profile",
                faultCount: 0,
                faults: [],
                evidenceDetails: [],
                obdAdvertisedPIDCount: pids.count,
                livePIDCount: pids.filter {
                    ($0 & 0x1F) != 0 &&
                    mblink_obd2_pid_definition(0x01, $0) != nil
                }.count))
        }

        let label: String
        if let vin = selectedVIN, vin.count == 17 {
            label = "Saved vehicle profile · \(modules.count) controllers · available offline"
        } else {
            label = "Saved vehicle profile · available offline"
        }

        return (
            modules.sorted {
                if $0.requestCANIdentifier != $1.requestCANIdentifier {
                    return $0.requestCANIdentifier < $1.requestCANIdentifier
                }
                return $0.name < $1.name
            },
            support,
            label)
    }

    private func refreshPIDConfiguration() {
        let live = diagnosticModules
        if isActive && !live.isEmpty {
            var support = [String: Set<UInt8>]()
            for module in live {
                let pids = controller.observedPIDs(
                    forResponderCANIdentifier: module.responseCANIdentifier,
                    extendedID: module.extendedID)
                    .compactMap { UInt8(exactly: $0.uintValue) }
                support[module.id] = Set(pids)
            }
            pidSupportByModule = support
            pidConfigurationModules = live.sorted {
                if $0.requestCANIdentifier != $1.requestCANIdentifier {
                    return $0.requestCANIdentifier < $1.requestCANIdentifier
                }
                return $0.name < $1.name
            }
            pidConfigurationSourceText = isActive
                ? "Current vehicle · controller capability map"
                : "Last active vehicle · controller capability map"
            return
        }

        let saved = loadSavedPIDConfiguration()
        pidSupportByModule = saved.support
        pidConfigurationModules = saved.modules
        pidConfigurationSourceText = saved.label
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
            ratedTorqueNM: definition.flatMap { $0.rated_torque_nm == 0 ? nil : $0.rated_torque_nm },
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


    private func associatedAdapterIdentifier(for vin: String?) -> String? {
        guard let vin, vin.count == 17 else { return nil }
        return vehicleProfileStore.associatedAdapterIdentifier(forVIN: vin)
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
        refreshSavedVehicleProfiles()

        let capturedVIN = controller.isActive ? controller.mercedesVINText : nil
        let currentVIN = capturedVIN?.count == 17
            ? capturedVIN : selectedVehicleVIN
        mercedesVINText = currentVIN ?? "Not captured"
        vehicleIdentity = decodeVehicleIdentity(vin: currentVIN)

        if controller.isActive {
            mercedesProbeStatusText = controller.mercedesProbeStatusText
            mercedesProbeEndpointText = controller.mercedesProbeEndpointText ?? "Source-corroborated endpoint not selected"
            mercedesIdentitySummaryText = controller.mercedesIdentitySummaryText
            mercedesIdentityResults = controller.mercedesIdentityResults
            mercedesCrd3SummaryText = controller.mercedesCrd3SummaryText
            mercedesUDSFaultStatusText = controller.mercedesUDSFaultStatusText
            mercedesUDSFaults = controller.mercedesUDSFaults
            vehicleProfileStatusText = controller.vehicleProfileStatusText
        } else if let vin = selectedVehicleVIN {
            let profile = vehicleProfileStore.profile(forVIN: vin) as? [String: Any]
            let modules = profile?["modules"] as? [[String: Any]] ?? []
            mercedesProbeStatusText = "Disconnected · saved vehicle profile"
            mercedesProbeEndpointText =
                (profile?["probeEndpoint"] as? String) ?? "Saved profile · endpoint not recorded"
            mercedesIdentitySummaryText =
                "Saved vehicle profile · \(modules.count) controller\(modules.count == 1 ? "" : "s") · offline"
            mercedesIdentityResults = []
            mercedesCrd3SummaryText =
                (profile?["crd3Summary"] as? String) ?? "Saved profile · identity not recorded"
            mercedesUDSFaultStatusText = "Disconnected · saved fault state not refreshed"
            mercedesUDSFaults = []
            vehicleProfileStatusText = "Saved vehicle profile loaded · offline"
        } else {
            mercedesProbeStatusText = "Not connected"
            mercedesProbeEndpointText = "No vehicle loaded"
            mercedesIdentitySummaryText = "No vehicle loaded"
            mercedesIdentityResults = []
            mercedesCrd3SummaryText = "Not available"
            mercedesUDSFaultStatusText = "Not scanned"
            mercedesUDSFaults = []
            vehicleProfileStatusText = "No vehicle loaded · connect to a vehicle"
        }
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

        languageTags = controller.availableLanguageTags
        languageNames = controller.availableLanguageNames
        selectedLanguageID = controller.selectedLanguageTag
        measurementKeys = controller.availableMeasurementSystemKeys
        measurementNames = controller.availableMeasurementSystemNames
        selectedMeasurementID = controller.selectedMeasurementSystemKey

        isActive = controller.isActive
        isReady = controller.isReady
        if controller.isActive, !isSimulationActive,
           let liveVIN = controller.mercedesVINText,
           liveVIN.count == 17 {
            // LINK records the live VIN as authoritative and updates the
            // optional per-vehicle adapter association.
            vehicleProfileStore.recordLiveVIN(liveVIN)
            selectedVehicleVIN = liveVIN
        }
        diagnosticModules = controller.isActive ? loadDiagnosticModules() : []
        refreshPIDConfiguration()
        diagnosticParameters = loadPrimaryDiagnosticParameters()
        manufacturerDataScanActive = controller.isManufacturerDataScanActive
        manufacturerDataScanStatusText = controller.manufacturerDataScanStatusText
        manufacturerDataScanModuleID = controller.manufacturerDataScanModuleIdentifier
        dashboardParameters = loadDashboardParameters()
        recordedSampleCount = Int(clamping: controller.recordedSampleCount)
    }
}
