// SPDX-License-Identifier: GPL-3.0-or-later
import Combine
import Foundation


typealias DiagnosticParameter = LinkDiagnosticParameter

typealias DiagnosticModule = LinkDiagnosticModule

typealias PIDConfigurationItem = LinkPIDConfigurationItem

typealias SavedVehicleProfileSummary = LinkSavedVehicleProfileSummary

struct MBPIDCatalogueItem: Identifiable {
    enum Source: Equatable {
        case standard
        case manufacturer
    }

    let id: String
    let source: Source
    let service: UInt8
    let identifier: UInt16
    let shortName: String
    let title: String
    let provenance: String
    let pollingEnabled: Bool
    let advertised: Bool

    var codeText: String {
        if source == .standard {
            return String(format: "01 %02X", identifier)
        }
        return String(format: "%02X %02X", service, identifier)
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
final class ConnectionViewModel: LinkStandardProductViewModel,
    MBLinkDiagnosticsControllerDelegate {
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
    @Published private(set) var storedFaults = [DiagnosticFault]()
    @Published private(set) var pendingFaults = [DiagnosticFault]()
    @Published private(set) var permanentFaults = [DiagnosticFault]()
    @Published private(set) var connectionAlertText: String?

    @Published private(set) var diagnosticModules = [DiagnosticModule]()
    @Published private(set) var pidConfigurationModules = [DiagnosticModule]()
    @Published private(set) var pidConfigurationSourceText =
        "Connect once to learn vehicle and module PID support"
    @Published private(set) var manufacturerDataScanActive = false
    @Published private(set) var manufacturerDataScanStatusText = "Not scanned"
    @Published private(set) var manufacturerDataScanModuleID: String?
    @Published private(set) var mercedesTargetSignals = [MercedesTargetSignal]()
    @Published private(set) var mercedesNativeDataIdentities = [MercedesNativeDataIdentity]()
    private var controller: MBLinkDiagnosticsController {
        productController as! MBLinkDiagnosticsController
    }
    private var pidSelectionStore: LinkPIDSelectionStore {
        pollingSelectionStore
    }
    private var lastConnectionAlertText: String?
    private var manufacturerNumericHistory = [String: [Double]]()
    private var manufacturerLastRawByParameter = [String: String]()
    private var manufacturerHistoryVIN: String?
    private var manufacturerHistorySessionActive = false
    private var appliedPollingConfigurationKey: String?

    /*
     * v2 changes first-run policy from an automatic core set to explicit
     * opt-in. Existing user choices are preserved, but the old untouched
     * automatic default is recognised and migrated to an empty selection.
     */
    private static let legacyPollingDefaultsKey = "mblink.polling.enabledStableKeys.v1"
    private static let manufacturerSelectionDefaultsKey =
        "mblink.manufacturer.pidSelectionsByVehicle.v1"
    private static let manufacturerCatalogueDefaultsKey =
        "mblink.manufacturer.pidCatalogueByVehicle.v1"
    private static let standardSelectionControllerIdentifier = "standard-obd"
    private static let standardSelectionMigrationDefaultsKey =
        "mblink.standard.pidSelectionsGlobalMigrated.v1"
    private static let manufacturerStableKeyAliases = [
        "mercdes.transmission.actual_gear":
            "mercedes.transmission.actual_gear",
        "mercdes.transmission.selector_position":
            "mercedes.transmission.selector_position",
        "mercdes.transmission.drive_program":
            "mercedes.transmission.drive_program"
    ]
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

    init() {
        let controller = MBLinkDiagnosticsController()
        let version = mblink_version().map { String(cString: $0) } ?? "Unknown"
        super.init(
            controller: controller,
            configuration: LinkStandardProductConfiguration(
                productName: "MBLINK",
                productNamespace: "mblink",
                manufacturerName: "Mercedes-Benz",
                vehicleName: "Mercedes-Benz vehicle",
                versionText: version,
                legacyProfileKey: "mblink.vehicleProfiles.v1",
                legacySelectedVINKey: "mblink.selectedVehicleVIN.v1",
                legacyAdapterMappingKey:
                    "mblink.adapterPeripheralByVehicle.v1",
                dashboardSelectionNamespace: "mblink-dashboard",
                pollingSelectionNamespace: "mblink",
                legacyPollingGlobalKey:
                    "mblink.polling.enabledStableKeys.v2",
                legacyPollingVehicleKey:
                    "mblink.pidSelectionsByVehicle.v1",
                seedDefaultPollingSelection: false,
                defaultDashboardStableKeys: [
                    "obd2.engine.rpm", "obd2.vehicle.speed",
                    "obd2.engine.coolant", "obd2.diesel.rail_pressure",
                    "obd2.fuel.tank_level"
                ],
                standardPIDStableKey: { pid in
                    if let definition = mblink_parameter_obd2_definition(pid),
                       let key = definition.pointee.stable_key {
                        let value = String(cString: key)
                        if !value.isEmpty { return value }
                    }
                    return String(format: "sae.obd2.mode01.%02X", pid)
                }))
        migrateLegacySharedSettings()
        applyStoredPollingPolicy()
        controller.delegate = self
        mercedesTargetSignals = loadMercedesTargetSignals()
        mercedesNativeDataIdentities = loadMercedesNativeDataIdentities()
        refreshStandardState()
#if MBLINK_CI_SIMULATED_FLOW
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1" {
            startSimulatedDiagnostics()
        }
#endif
    }

    override func connect() {
        connectionAlertText = nil
        lastConnectionAlertText = nil
        super.connect()
    }

    func dismissConnectionAlert() {
        connectionAlertText = nil
    }

    func parameter(stableKey: String) -> DiagnosticParameter? {
        diagnosticParameters.first { $0.id == stableKey }
    }

    var standardVINText: String {
        controller.standardVINText
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

        let selected = storedPollingKeys()
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
                    qualityNote: parameter.qualityNote,
                    dashboardMinimum: parameter.dashboardMinimum,
                    dashboardMaximum: parameter.dashboardMaximum)
            }
    }

    func pidConfigurationModule(id: String) -> DiagnosticModule? {
        pidConfigurationModules.first { $0.id == id }
    }

    override func selectSavedVehicle(vin: String) {
        // A live VIN is authoritative. Saved-profile selection is an offline
        // operation and must never override the physical car.
        guard !isActive else { return }
        super.selectSavedVehicle(vin: vin)
        guard selectedVehicleVIN == vin else { return }
        mercedesVINText = vin
        vehicleIdentity = decodeVehicleIdentity(vin: vin)
        vehicleProfileStatusText = "Saved vehicle profile loaded · offline"
        mercedesIdentitySummaryText = "Saved vehicle profile · offline"
        mercedesProbeStatusText = "Disconnected · saved vehicle profile"
        refreshPIDConfiguration()
        applyConfiguredPollingIfNeeded(force: true)
        refreshStandardState()
    }

    func standardPIDCatalogueItems() -> [MBPIDCatalogueItem] {
        let selected = storedPollingKeys()
        let advertised = Set(pidSupportByModule.values.flatMap { $0 })
        let count = mblink_obd2_pid_definition_count()
        guard count > 0 else { return [] }

        var result = [MBPIDCatalogueItem]()
        for index in 0..<count {
            guard let definition = mblink_obd2_pid_definition_at(index) else {
                continue
            }
            let metadata = definition.pointee
            guard metadata.mode == 0x01 else { continue }
            let pid = metadata.pid
            guard (pid & 0x1F) != 0 else { continue }

            let scalar = mblink_parameter_obd2_definition(pid)
            let title = scalar != nil
                ? string(from: scalar!.pointee.name)
                : string(from: metadata.name)
            let shortName = scalar != nil
                ? string(from: scalar!.pointee.short_name)
                : String(format: "PID %02X", pid)
            let stableKey = standardStableKey(for: pid)
            result.append(MBPIDCatalogueItem(
                id: stableKey,
                source: .standard,
                service: 0x01,
                identifier: UInt16(pid),
                shortName: shortName,
                title: title,
                provenance: "SAE J1979 / ISO 15031-5",
                pollingEnabled: selected.contains(stableKey),
                advertised: advertised.contains(pid)))
        }
        return result.sorted {
            if $0.identifier != $1.identifier {
                return $0.identifier < $1.identifier
            }
            return $0.title < $1.title
        }
    }

    func manufacturerPIDCatalogueItems(moduleID: String) -> [MBPIDCatalogueItem] {
        let liveDefinitions = controller.documentedDataDefinitions(
            forModuleIdentifier: moduleID)
        if !liveDefinitions.isEmpty {
            let cached = liveDefinitions.map { definition in
                [
                    "id": definition.stableKey,
                    "service": Int(definition.service),
                    "identifier": Int(definition.identifier),
                    "shortName": definition.shortName,
                    "title": definition.title,
                    "provenance": definition.provenance
                ] as [String: Any]
            }
            cacheManufacturerCatalogue(cached, moduleID: moduleID)
        }

        let definitions: [[String: Any]]
        if !liveDefinitions.isEmpty {
            definitions = liveDefinitions.map { definition in
                [
                    "id": definition.stableKey,
                    "service": Int(definition.service),
                    "identifier": Int(definition.identifier),
                    "shortName": definition.shortName,
                    "title": definition.title,
                    "provenance": definition.provenance
                ]
            }
        } else {
            definitions = cachedManufacturerCatalogue(moduleID: moduleID)
        }

        let selected = manufacturerSelectionSet(moduleID: moduleID)
        return definitions.compactMap { value in
            guard let storedStableKey = value["id"] as? String,
                  let serviceNumber = value["service"] as? NSNumber,
                  let identifierNumber = value["identifier"] as? NSNumber,
                  let shortName = value["shortName"] as? String,
                  let title = value["title"] as? String
            else { return nil }
            let provenance =
                value["provenance"] as? String ?? "MBLINK documented catalogue"
            let stableKey = canonicalManufacturerStableKey(storedStableKey)
            return MBPIDCatalogueItem(
                id: stableKey,
                source: .manufacturer,
                service: serviceNumber.uint8Value,
                identifier: identifierNumber.uint16Value,
                shortName: shortName,
                title: title,
                provenance: provenance,
                pollingEnabled: selected.contains(stableKey),
                advertised: true)
        }.sorted {
            if $0.identifier != $1.identifier {
                return $0.identifier < $1.identifier
            }
            return $0.title < $1.title
        }
    }

    func setStandardPIDSelection(_ enabled: Bool, stableKey: String) {
        setPolling(enabled, stableKey: stableKey)
    }

    func setManufacturerPIDSelection(
        _ enabled: Bool,
        moduleID: String,
        stableKey: String
    ) {
        let catalogue = manufacturerPIDCatalogueItems(moduleID: moduleID)
        guard catalogue.contains(where: { $0.id == stableKey }) else { return }
        var selected = manufacturerSelectionSet(moduleID: moduleID)
        if enabled { selected.insert(stableKey) }
        else { selected.remove(stableKey) }
        storeManufacturerSelection(selected, moduleID: moduleID)
        applyManufacturerPollingSelection(moduleID: moduleID)
        refreshPresentation()
    }

    var configuredPollingCount: Int {
        let standard = storedPollingKeys().count
        let manufacturer = pidConfigurationModules.reduce(0) {
            $0 + manufacturerSelectionSet(moduleID: $1.id).count
        }
        return standard + manufacturer
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
        refreshStandardState()
    }

    func rescanManufacturerData(moduleID: String) {
        guard isActive else { return }
        controller.rescanManufacturerData(
            forModuleIdentifier: moduleID)
        refreshStandardState()
    }

    func setPolling(_ enabled: Bool, stableKey: String) {
        guard !controller.isActive || effectivePIDConfigurationVIN != nil
        else { return }
        guard let pid = pidForStableKey(stableKey) else { return }
        var enabledKeys = storedPollingKeys()
        if enabled { enabledKeys.insert(stableKey) } else { enabledKeys.remove(stableKey) }
        storeStandardPollingKeys(enabledKeys)
        controller.setPollingEnabled(enabled, forPID: pid)
        refreshStandardState()
    }

    func refreshPresentation() {
        refreshStandardState()
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

    nonisolated func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {
        Task { @MainActor [weak self] in self?.refreshStandardState() }
    }

    private var activeVehicleVIN: String? {
        guard controller.isActive else { return nil }
        if let mercedesVIN = controller.mercedesVINText,
           mercedesVIN.count == 17 {
            return mercedesVIN
        }
        let standardVIN = controller.standardVINText
        return standardVIN.count == 17 ? standardVIN : nil
    }

    private var effectivePIDConfigurationVIN: String? {
        // Never apply a previously selected offline vehicle's polling choices
        // while a different live vehicle is still waiting for its VIN.
        controller.isActive ? activeVehicleVIN : selectedVehicleVIN
    }

    private func manufacturerSelectionSet(moduleID: String) -> Set<String> {
        guard let vin = effectivePIDConfigurationVIN else { return [] }
        let defaults = UserDefaults.standard
        guard let vehicles = defaults.dictionary(
                forKey: Self.manufacturerSelectionDefaultsKey),
              let modules = vehicles[vin] as? [String: Any],
              let values = modules[moduleID] as? [String]
        else { return [] }
        let stored = Set(values)
        let canonical = Set(values.map(canonicalManufacturerStableKey))
        if canonical != stored {
            storeManufacturerSelection(canonical, moduleID: moduleID)
        }
        return canonical
    }

    private func storeManufacturerSelection(
        _ selection: Set<String>,
        moduleID: String
    ) {
        guard let vin = effectivePIDConfigurationVIN else { return }
        let defaults = UserDefaults.standard
        var vehicles = defaults.dictionary(
            forKey: Self.manufacturerSelectionDefaultsKey) ?? [:]
        var modules = vehicles[vin] as? [String: Any] ?? [:]
        modules[moduleID] = Array(selection).sorted()
        vehicles[vin] = modules
        defaults.set(vehicles, forKey: Self.manufacturerSelectionDefaultsKey)
    }

    private func cachedManufacturerCatalogue(
        moduleID: String
    ) -> [[String: Any]] {
        guard let vin = effectivePIDConfigurationVIN else { return [] }
        let defaults = UserDefaults.standard
        guard let vehicles = defaults.dictionary(
                forKey: Self.manufacturerCatalogueDefaultsKey),
              let modules = vehicles[vin] as? [String: Any],
              let values = modules[moduleID] as? [[String: Any]]
        else { return [] }
        return values
    }

    private func cacheManufacturerCatalogue(
        _ catalogue: [[String: Any]],
        moduleID: String
    ) {
        guard let vin = effectivePIDConfigurationVIN else { return }
        let defaults = UserDefaults.standard
        var vehicles = defaults.dictionary(
            forKey: Self.manufacturerCatalogueDefaultsKey) ?? [:]
        var modules = vehicles[vin] as? [String: Any] ?? [:]
        modules[moduleID] = catalogue
        vehicles[vin] = modules
        defaults.set(vehicles, forKey: Self.manufacturerCatalogueDefaultsKey)
    }

    private func applyManufacturerPollingSelection(moduleID: String) {
        let catalogue = manufacturerPIDCatalogueItems(moduleID: moduleID)
        let selected = manufacturerSelectionSet(moduleID: moduleID)
        let wireIdentifiers = Set(
            catalogue
                .filter { selected.contains($0.id) }
                .map { NSNumber(value: $0.identifier) })
        controller.setManufacturerLivePollingIdentifiers(
            Array(wireIdentifiers),
            forModuleIdentifier: moduleID)
    }

    private func applyConfiguredPollingForSelectedVehicle() {
        let selectedKeys = storedPollingKeys()
        let count = mblink_obd2_pid_definition_count()
        if count > 0 {
            for index in 0..<count {
                guard let definition = mblink_obd2_pid_definition_at(index)
                else { continue }
                let metadata = definition.pointee
                guard metadata.mode == 0x01 else { continue }
                let pid = metadata.pid
                guard (pid & 0x1F) != 0 else { continue }
                controller.setPollingEnabled(
                    selectedKeys.contains(standardStableKey(for: pid)),
                    forPID: pid)
            }
        }
        for module in pidConfigurationModules {
            applyManufacturerPollingSelection(moduleID: module.id)
        }
    }

    private func applyConfiguredPollingIfNeeded(force: Bool = false) {
        let moduleKey = pidConfigurationModules.map(\.id).sorted()
            .joined(separator: ",")
        let vehicleKey: String
        if controller.isActive {
            vehicleKey = activeVehicleVIN.map { "live:\($0)" }
                ?? "live:unknown"
        } else {
            vehicleKey = selectedVehicleVIN.map { "offline:\($0)" }
                ?? "offline:global"
        }
        let configurationKey = "\(vehicleKey)|\(moduleKey)"
        guard force || configurationKey != appliedPollingConfigurationKey
        else { return }

        /* Mark first because setPollingEnabled notifies the view model. Any
         * queued refresh caused by this application must observe the same key
         * and must not recursively apply the selection again. */
        appliedPollingConfigurationKey = configurationKey
        applyConfiguredPollingForSelectedVehicle()
    }

    private func storedPollingKeys() -> Set<String> {
        if controller.isActive && activeVehicleVIN == nil {
            return []
        }
        if let vin = effectivePIDConfigurationVIN {
            let controllerID = Self.standardSelectionControllerIdentifier
            if pidSelectionStore.hasSelection(
                forVIN: vin,
                controllerIdentifier: controllerID
            ) {
                return Set(pidSelectionStore.stableKeys(
                    forVIN: vin,
                    controllerIdentifier: controllerID))
            }

            /* v0.7.186 stored standard choices under each responder module.
             * Preserve the union when moving to the one functional Mode 01
             * request used by the new Standard OBD section. An explicitly
             * empty old selection is still a real selection. */
            var foundModuleSelection = false
            var moduleSelection = Set<String>()
            for module in pidConfigurationModules {
                if pidSelectionStore.hasSelection(
                    forVIN: vin,
                    controllerIdentifier: module.id
                ) {
                    foundModuleSelection = true
                    moduleSelection.formUnion(pidSelectionStore.stableKeys(
                        forVIN: vin,
                        controllerIdentifier: module.id))
                }
            }
            if foundModuleSelection {
                pidSelectionStore.setStableKeys(
                    Array(moduleSelection).sorted(),
                    forVIN: vin,
                    controllerIdentifier: controllerID)
                return moduleSelection
            }

            /* During startup the saved/live module list may not have arrived
             * yet. Do not consume the one global fallback until it has, or a
             * per-module v0.7.186 selection could be lost. */
            if pidConfigurationModules.isEmpty {
                return legacyGlobalPollingKeys()
            }

            /* Preserve one existing explicit global choice for the first VIN
             * encountered after this migration. Every later new VIN starts
             * empty, as the product contract requires. */
            let defaults = UserDefaults.standard
            let initial: Set<String>
            if !defaults.bool(
                forKey: Self.standardSelectionMigrationDefaultsKey
            ) {
                initial = legacyGlobalPollingKeys()
                defaults.set(
                    true,
                    forKey: Self.standardSelectionMigrationDefaultsKey)
            } else {
                initial = []
            }
            pidSelectionStore.setStableKeys(
                Array(initial).sorted(),
                forVIN: vin,
                controllerIdentifier: controllerID)
            return initial
        }
        return legacyGlobalPollingKeys()
    }

    private func storeStandardPollingKeys(_ selection: Set<String>) {
        let sorted = Array(selection).sorted()
        if let vin = effectivePIDConfigurationVIN {
            pidSelectionStore.setStableKeys(
                sorted,
                forVIN: vin,
                controllerIdentifier:
                    Self.standardSelectionControllerIdentifier)
        } else if !controller.isActive {
            pidSelectionStore.setGlobalStableKeys(sorted)
        }
    }

    private func legacyGlobalPollingKeys() -> Set<String> {
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

    private func canonicalManufacturerStableKey(_ stableKey: String) -> String {
        Self.manufacturerStableKeyAliases[stableKey] ?? stableKey
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
        controller.displayValue(forPID: pid, canonicalValue: rawValue)
    }

    private func displaySuffix(
        pid: UInt8,
        definition: UnsafePointer<MblinkParameterDefinition>
    ) -> String {
        let unit = controller.displayUnit(forPID: pid)
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

            let dashboardRange: (Double, Double)? = {
                guard let scalarDefinition else { return nil }
                var range = LinkDashboardGaugeRange()
                guard link_dashboard_gauge_range_for_parameter(
                    scalarDefinition, &range) else { return nil }
                return (
                    displayScalar(pid: pid, rawValue: range.minimum),
                    displayScalar(pid: pid, rawValue: range.maximum))
            }()

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
                qualityNote: qualityNote,
                dashboardMinimum: dashboardRange?.0,
                dashboardMaximum: dashboardRange?.1))
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

    private func transmissionDiagnosticParameters() -> [DiagnosticParameter] {
        guard let module = diagnosticModules.first(where: {
            !$0.extendedID &&
                $0.requestCANIdentifier == 0x7E1 &&
                $0.responseCANIdentifier == 0x7E9
        }) else { return [] }

        let source = "\(module.name) · \(module.addressText)"
        let selected = manufacturerSelectionSet(moduleID: module.id)
        return controller.transmissionLiveValueSnapshots()
            .filter { selected.contains($0.identifier) }
            .map { snapshot in
                let numeric = snapshot.isNumericValueAvailable
                    ? snapshot.numericValue : nil
                return DiagnosticParameter(
                    id: snapshot.identifier,
                    protocolName: "kwp2000",
                    moduleIdentifier: 0x7E1,
                    parameterIdentifier: UInt32(0x2100) |
                        UInt32(snapshot.localIdentifier),
                    shortName: snapshot.shortName,
                    title: snapshot.title,
                    suffix: snapshot.suffix,
                    formattedValue: snapshot.formattedValue,
                    value: numeric,
                    structuredValue: numeric == nil
                        ? snapshot.formattedValue : nil,
                    rawHex: snapshot.rawHex,
                    vehicleSupported: true,
                    favourite: false,
                    pollingEnabled: true,
                    history: numeric.map {
                        manufacturerHistory(
                            id: snapshot.identifier,
                            value: $0,
                            rawHex: snapshot.rawHex)
                    } ?? [],
                    sourceLabel: source,
                    qualityNote: snapshot.qualityNote)
            }
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
                    "Connect once to learn vehicle and module PID support")
        }

        let liveVIN = activeVehicleVIN
        let selectedVIN: String? = controller.isActive
            ? liveVIN : selectedVehicleVIN

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
        for responder in LinkVehicleProfileStandardResponders(profile) {
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
        let count = Int(mblink_mercedes_me_data_id_count())
        guard count > 0 else { return [] }

        var result = [MercedesNativeDataIdentity]()
        result.reserveCapacity(count)
        for index in 0..<count {
            guard let definition = mblink_mercedes_me_data_id_at(index) else { continue }
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


    override func productModuleCountForVehicleProfile(
        _ profile: [AnyHashable: Any]
    ) -> Int {
        (profile["modules"] as? [[String: Any]])?.count ?? 0
    }

    override func productDiagnosticParameters(
        standard: [LinkDiagnosticParameter]
    ) -> [LinkDiagnosticParameter] {
        loadPrimaryDiagnosticParameters()
    }

    override func productDidRefreshStandardState() {
        // Never join samples from separate sessions or vehicles in one graph.
        let liveHistoryVIN = activeVehicleVIN
        if !isActive || !manufacturerHistorySessionActive ||
            manufacturerHistoryVIN != liveHistoryVIN {
            manufacturerNumericHistory.removeAll()
            manufacturerLastRawByParameter.removeAll()
        }
        manufacturerHistorySessionActive = isActive
        manufacturerHistoryVIN = isActive ? liveHistoryVIN : nil
        let updatedStatus = statusText
        let isTransportBoundary =
            updatedStatus.contains("Bluetooth Classic Mercedes adapter") ||
            updatedStatus.contains("No compatible BLE diagnostic adapter found")
        if isTransportBoundary && updatedStatus != lastConnectionAlertText {
            lastConnectionAlertText = updatedStatus
            connectionAlertText = updatedStatus
        }

        let capturedVIN = activeVehicleVIN
        let currentVIN = isActive ? capturedVIN : selectedVehicleVIN
        mercedesVINText = currentVIN ?? "Not captured"
        vehicleIdentity = decodeVehicleIdentity(vin: currentVIN)

        if isActive {
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
        storedFaults = resolveFaults(storedDTCs, state: "Stored")
        pendingFaults = resolveFaults(pendingDTCs, state: "Pending")
        permanentFaults = resolveFaults(permanentDTCs, state: "Permanent")
        diagnosticModules = isActive ? loadDiagnosticModules() : []
        refreshPIDConfiguration()
        applyConfiguredPollingIfNeeded()
#if MBLINK_CI_SIMULATED_FLOW
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1",
           isSimulationActive {
            let liveVIN = controller.mercedesVINText ?? ""
            let selectionVIN = effectivePIDConfigurationVIN ?? ""
            let standardSelectionCount = storedPollingKeys().count
            let standardSelectionScoped = selectionVIN.count == 17 &&
                pidSelectionStore.hasSelection(
                    forVIN: selectionVIN,
                    controllerIdentifier:
                        Self.standardSelectionControllerIdentifier)
            let failed = controller.statusText.localizedCaseInsensitiveContains("failed")
            let state = isReady && liveVIN.count == 17
                ? "ready" : (failed ? "failed" : "pending")
            let marker = "state=\(state)\n" +
                "vin=\(liveVIN)\n" +
                "active=\(isActive)\n" +
                "ready=\(isReady)\n" +
                "status=\(controller.statusText)\n" +
                "fault_status=\(faultScanStatusText)\n" +
                "stored_codes=\(storedFaults.map(\.code).joined(separator: ","))\n" +
                "stored_states=\(storedFaults.map(\.state).joined(separator: ","))\n" +
                "stored_faults=\(storedFaults.map(\.displayText).joined(separator: " | "))\n" +
                "probe=\(controller.mercedesProbeStatusText)\n" +
                "profile=\(controller.vehicleProfileStatusText)\n" +
                "standard_selection_vin=\(selectionVIN)\n" +
                "standard_selection_count=\(standardSelectionCount)\n" +
                "standard_selection_scoped=\(standardSelectionScoped)\n"
            if let directory = FileManager.default.urls(
                    for: .documentDirectory, in: .userDomainMask).first {
                try? FileManager.default.createDirectory(
                    at: directory, withIntermediateDirectories: true)
                try? marker.write(
                    to: directory.appendingPathComponent(
                        "mblink-ci-simulated-flow.ok"),
                    atomically: true, encoding: .utf8)
            }
        }
#endif
        manufacturerDataScanActive = controller.isManufacturerDataScanActive
        manufacturerDataScanStatusText = controller.manufacturerDataScanStatusText
        manufacturerDataScanModuleID = controller.manufacturerDataScanModuleIdentifier
    }
}
