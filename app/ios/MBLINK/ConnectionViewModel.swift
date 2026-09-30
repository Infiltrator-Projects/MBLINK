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

    /// Globally unique presentation identity: controller + interface + value.
    let id: String
    /// Stable unscoped logical key stored inside the owning controller's selection.
    let selectionKey: String
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

    var sourceText: String {
        switch source {
        case .standard:
            return "OBD-II"
        case .manufacturer:
            switch service {
            case 0x1A, 0x21: return "KWP2000"
            case 0x22: return "UDS"
            default: return "MERCEDES"
            }
        }
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
    let rawData: Data
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

struct EGS53VariantCodingFact: Identifiable {
    let id: String
    let label: String
    let value: String
    let confidence: String
}

struct EGS53VariantCodingSummary {
    let facts: [EGS53VariantCodingFact]
    let undecodedRaw: String
    let fullRaw: String
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

/// Product presentation of LINK's generic UDS service metadata.  This model
/// deliberately carries both codec capability and Discover policy so the UI
/// cannot confuse "serializable" with "permitted to transmit".
struct MBUDSServiceCatalogueItem: Identifiable {
    let id: UInt8
    let name: String
    let effect: String
    let transmissionPolicy: String
    let readOnlyTransmissionAllowed: Bool

    var codeText: String {
        String(format: "0x%02X", id)
    }
}

/// Presentation-only view of LINK's complete ReadDTCInformation report
/// catalogue.  Target-ECU support is intentionally not inferred here.
struct MBUDSDTCReportCatalogueItem: Identifiable {
    let id: UInt8
    let name: String
    let withdrawnIn2020: Bool

    var codeText: String {
        String(format: "19 %02X", id)
    }
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

    /// Complete product-neutral UDS codec catalogue from LINK.  Safety is
    /// evaluated independently for each SID through LINK's deny-by-default
    /// Discover classifier; no request is transmitted by constructing this
    /// presentation model.
    var udsServiceCatalogue: [MBUDSServiceCatalogueItem] {
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
            let policy = allowed
                ? "Read-only allowed · \(reason)"
                : "Codec only · \(reason)"

            items.append(MBUDSServiceCatalogueItem(
                id: definition.service,
                name: name,
                effect: effect,
                transmissionPolicy: policy,
                readOnlyTransmissionAllowed: allowed))
        }
        return items
    }

    /// Complete ReadDTCInformation catalogue from LINK.  These entries mean
    /// that the generic codec can construct/validate the report; they do not
    /// claim that the connected Mercedes ECU implements the report type.
    var udsDTCReportCatalogue: [MBUDSDTCReportCatalogueItem] {
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
    private var livePollingReadyRearmSignature: String?
    /*
     * v2 changes first-run policy from an automatic core set to explicit
     * opt-in. Existing user choices are preserved, but the old untouched
     * automatic default is recognised and migrated to an empty selection.
     */
    private static let legacyPollingDefaultsKey = "mblink.polling.enabledStableKeys.v1"
    private static let manufacturerSelectionDefaultsKey =
        "mblink.manufacturer.pidSelectionsByVehicle.v1"
    private static let manufacturerCatalogueDefaultsKey =
        "mblink.manufacturer.pidCatalogueByVehicle.v5"
    private static let standardSelectionControllerIdentifier = "standard-obd"
    private static let moduleStatusMILStableKey = "obd2.readiness.mil"
    private static let standardSelectionMigrationDefaultsKey =
        "mblink.standard.pidSelectionsGlobalMigrated.v1"
    private static let standardSelectionExplicitDefaultsKey =
        "mblink.standard.pidSelectionsExplicitByVehicle.v1"
    private static let standardControllerScopedMigrationDefaultsKey =
        "mblink.standard.pidSelectionsControllerScopedByVehicle.v1"
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
        if ProcessInfo.processInfo.environment["MBLINK_CI_SAVED_PID_RESTORE"] == "1" {
            writeSavedPIDCatalogueRegressionMarker()
        }
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1" {
            if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_POLLING"] == "1" {
                let simulatedVIN = "WDD2073022F123456"
                pidSelectionStore.setStableKeys(
                    ["obd2.engine.rpm", "obd2.vehicle.speed"],
                    forVIN: simulatedVIN,
                    controllerIdentifier: Self.standardSelectionControllerIdentifier)
                markStandardSelectionExplicitlyEdited(vin: simulatedVIN)
                if ProcessInfo.processInfo.environment[
                    "MBLINK_CI_PERSIST_SIMULATED_PROFILE"] == "1" {
                    vehicleProfileStore.recordLiveVIN(simulatedVIN)
                }
                appliedPollingConfigurationKey = nil
            }
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

    // PID Setup owns display membership as well as polling. Consult the saved
    // selection directly so cached samples can never keep an OFF channel visible.
    var enabledDisplayParameters: [DiagnosticParameter] {
        /*
         * Every DiagnosticParameter is now already scoped to its physical
         * controller and diagnostic interface. PID Setup is therefore the sole
         * display-membership authority simply by the parameter's own enabled
         * state; two controllers may legitimately expose the same SAE value.
         */
        diagnosticParameters.filter(\.pollingEnabled).sorted {
            if $0.title != $1.title { return $0.title < $1.title }
            if ($0.sourceLabel ?? "") != ($1.sourceLabel ?? "") {
                return ($0.sourceLabel ?? "") < ($1.sourceLabel ?? "")
            }
            return $0.id < $1.id
        }
    }

    var standardVINText: String {
        controller.standardVINText
    }

    func diagnosticModule(id: String) -> DiagnosticModule? {
        diagnosticModules.first { $0.id == id }
    }

    /// Responder-attributed Mode 01 PID 01 state captured once during startup.
    /// These are module-status facts, not recurring live channels.
    func moduleStartupReadinessFields(
        moduleID: String
    ) -> [LinkReadinessFieldSnapshot] {
        guard isActive,
              let module = diagnosticModule(id: moduleID) else { return [] }
        let mask = startupReadinessFieldMask()
        guard mask != 0 else { return [] }
        return controller.readinessFieldSnapshots(
            forResponderCANIdentifier: module.responseCANIdentifier,
            extendedID: module.extendedID,
            fieldMask: mask
        ).filter(\.valueAvailable)
    }

    func moduleMILState(moduleID: String) -> Bool? {
        guard let field = moduleStartupReadinessFields(moduleID: moduleID)
            .first(where: { $0.stableKey == Self.moduleStatusMILStableKey }),
              field.numericValueAvailable
        else { return nil }
        return field.numericValue != 0
    }

    func moduleParameters(moduleID: String) -> [DiagnosticParameter] {
        guard let module =
            diagnosticModule(id: moduleID) ?? pidConfigurationModule(id: moduleID)
        else { return [] }

        return loadDiagnosticParameters(
            moduleID: moduleID,
            responderCANIdentifier: module.responseCANIdentifier,
            extendedID: module.extendedID,
            sourceLabel: "\(module.name) · OBD-II · \(module.addressText)")
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
        controller.loadSavedVehicleProfileForPIDConfiguration(vin: vin)
        mercedesVINText = vin
        vehicleIdentity = decodeVehicleIdentity(vin: vin)
        vehicleProfileStatusText = "Saved vehicle profile loaded · offline"
        mercedesIdentitySummaryText = "Saved vehicle profile · offline"
        mercedesProbeStatusText = "Disconnected · saved vehicle profile"
        refreshPIDConfiguration()
        applyConfiguredPollingIfNeeded(force: true)
        refreshStandardState()
    }

    override func productDidRestoreVehicleProfile(
        _ profile: [AnyHashable: Any],
        vin: String
    ) {
        controller.loadSavedVehicleProfileForPIDConfiguration(vin: vin)
    }

    func standardPIDCatalogueItems(
        moduleID: String
    ) -> [MBPIDCatalogueItem] {
        guard pidConfigurationModule(id: moduleID) != nil else { return [] }
        let selected = moduleStandardSelectionSet(moduleID: moduleID)
        let advertised = pidSupportByModule[moduleID] ?? []
        guard !advertised.isEmpty else { return [] }

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
            // Capability-bitmap PIDs describe the next block; they are not
            // user-facing measurements.
            guard (pid & 0x1F) != 0, advertised.contains(pid) else { continue }

            if pid == 0x01 { continue }

            let scalar = mblink_parameter_obd2_definition(pid)
            let title = scalar != nil
                ? string(from: scalar!.pointee.name)
                : string(from: metadata.name)
            let shortName = scalar != nil
                ? string(from: scalar!.pointee.short_name)
                : String(format: "PID %02X", pid)
            let stableKey = standardStableKey(for: pid)
            result.append(MBPIDCatalogueItem(
                id: scopedChannelID(
                    moduleID: moduleID,
                    source: .standard,
                    selectionKey: stableKey),
                selectionKey: stableKey,
                source: .standard,
                service: 0x01,
                identifier: UInt16(pid),
                shortName: shortName,
                title: title,
                provenance:
                    "SAE J1979 / ISO 15031-5",
                pollingEnabled: selected.contains(stableKey),
                advertised: true))
        }
        return result.sorted(by: controllerCatalogueSort)
    }

    func manufacturerPIDCatalogueItems(moduleID: String) -> [MBPIDCatalogueItem] {
        let documentedDefinitions = controller.documentedDataDefinitions(
            forModuleIdentifier: moduleID)
        let definitions: [[String: Any]]
        if !documentedDefinitions.isEmpty {
            /*
             * The controller already exposes only the resolved ECU pack's
             * user-polling section. Do not repeat startup/static exceptions in
             * the UI: acquisition ownership belongs to the ECU pack.
             */
            definitions = documentedDefinitions.map { definition in
                [
                    "id": definition.stableKey,
                    "service": Int(definition.service),
                    "identifier": Int(definition.identifier),
                    "shortName": definition.shortName,
                    "title": definition.title,
                    "provenance": definition.provenance
                ]
            }
            cacheManufacturerCatalogue(definitions, moduleID: moduleID)
        } else if controller.isActive {
            // A live identified ECU never inherits another vehicle's catalogue.
            definitions = []
        } else {
            definitions = cachedManufacturerCatalogue(moduleID: moduleID)
        }

        var selected = manufacturerSelectionSet(moduleID: moduleID)
        if !documentedDefinitions.isEmpty {
            let documentedStableKeys = Set(definitions.compactMap { value -> String? in
                guard let storedStableKey = value["id"] as? String else {
                    return nil
                }
                return canonicalManufacturerStableKey(storedStableKey)
            })
            let sanitized = selected.intersection(documentedStableKeys)
            if sanitized != selected {
                storeManufacturerSelection(sanitized, moduleID: moduleID)
                selected = sanitized
            }
        }

        return definitions.compactMap { value in
            guard let storedStableKey = value["id"] as? String,
                  let serviceNumber = value["service"] as? NSNumber,
                  let identifierNumber = value["identifier"] as? NSNumber,
                  let shortName = value["shortName"] as? String,
                  let title = value["title"] as? String
            else { return nil }
            let rawProvenance =
                value["provenance"] as? String ?? "MBLINK documented ECU PID"
            let service = serviceNumber.uint8Value
            let identifier = identifierNumber.uint16Value
            let provenance = manufacturerPIDPresentationProvenance(
                raw: rawProvenance,
                service: service,
                identifier: identifier)
            let stableKey = canonicalManufacturerStableKey(storedStableKey)
            return MBPIDCatalogueItem(
                id: scopedChannelID(
                    moduleID: moduleID,
                    source: .manufacturer,
                    selectionKey: stableKey),
                selectionKey: stableKey,
                source: .manufacturer,
                service: service,
                identifier: identifier,
                shortName: shortName,
                title: title,
                provenance: provenance,
                pollingEnabled: selected.contains(stableKey),
                advertised: true)
        }.sorted(by: controllerCatalogueSort)
    }

    func controllerPIDCatalogueItems(
        moduleID: String
    ) -> [MBPIDCatalogueItem] {
        (standardPIDCatalogueItems(moduleID: moduleID) +
         manufacturerPIDCatalogueItems(moduleID: moduleID))
            .sorted(by: controllerCatalogueSort)
    }

    func setStandardPIDSelection(
        _ enabled: Bool,
        moduleID: String,
        stableKey: String
    ) {
        guard !controller.isActive || effectivePIDConfigurationVIN != nil
        else { return }
        let catalogue = standardPIDCatalogueItems(moduleID: moduleID)
        guard catalogue.contains(where: { $0.selectionKey == stableKey })
        else { return }

        var selected = moduleStandardSelectionSet(moduleID: moduleID)
        if enabled { selected.insert(stableKey) }
        else { selected.remove(stableKey) }
        storeModuleStandardSelection(selected, moduleID: moduleID)
        if let vin = effectivePIDConfigurationVIN {
            markStandardSelectionExplicitlyEdited(vin: vin)
            markControllerScopedStandardMigration(vin: vin)
        }

        /*
         * Physical Mode 01 scheduling is vehicle-wide and functional. Rebuild
         * it from the union of controller choices so disabling one ECU's copy
         * never stops a source still selected on another ECU.
         */
        applyStoredPollingPolicy()
        refreshStandardState()
    }

    func setManufacturerPIDSelection(
        _ enabled: Bool,
        moduleID: String,
        stableKey: String
    ) {
        let catalogue = manufacturerPIDCatalogueItems(moduleID: moduleID)
        guard catalogue.contains(where: { $0.selectionKey == stableKey })
        else { return }
        var selected = manufacturerSelectionSet(moduleID: moduleID)
        if enabled { selected.insert(stableKey) }
        else { selected.remove(stableKey) }
        storeManufacturerSelection(selected, moduleID: moduleID)
        applyManufacturerPollingSelection(moduleID: moduleID)
        refreshPresentation()
    }

    var configuredPollingCount: Int {
        let standard = pidConfigurationModules.reduce(0) {
            $0 + moduleStandardSelectionSet(moduleID: $1.id).count
        }
        let manufacturer = pidConfigurationModules.reduce(0) {
            $0 + manufacturerSelectionSet(moduleID: $1.id).count
        }
        return standard + manufacturer
    }

    var pidConfigurationVehicleVIN: String? {
        effectivePIDConfigurationVIN
    }

    func resetPIDSelectionsForCurrentVehicle() {
        guard let vin = effectivePIDConfigurationVIN else { return }

        for module in pidConfigurationModules {
            pidSelectionStore.setStableKeys(
                [],
                forVIN: vin,
                controllerIdentifier: module.id)
        }
        // Retire the 0.7.257 vehicle-wide standard bucket for this VIN too.
        pidSelectionStore.setStableKeys(
            [],
            forVIN: vin,
            controllerIdentifier: Self.standardSelectionControllerIdentifier)
        markStandardSelectionExplicitlyEdited(vin: vin)
        markControllerScopedStandardMigration(vin: vin)

        // Remove the entire per-VIN Mercedes selection subtree, including
        // selections for modules that are no longer present in the current map.
        let defaults = UserDefaults.standard
        var vehicles = defaults.dictionary(
            forKey: Self.manufacturerSelectionDefaultsKey) ?? [:]
        vehicles.removeValue(forKey: vin)
        defaults.set(vehicles, forKey: Self.manufacturerSelectionDefaultsKey)

        appliedPollingConfigurationKey = nil
        applyConfiguredPollingIfNeeded(force: true)
        refreshPresentation()
        refreshStandardState()
    }

    func startupModuleData(moduleID: String) -> [MercedesModuleDataValue] {
        controller.startupDataSnapshots(forModuleIdentifier: moduleID)
            .map { snapshot in
                let code = snapshot.codeText
                let title = snapshot.name ?? code
                return MercedesModuleDataValue(
                    id: "\(moduleID):startup:\(snapshot.service):\(snapshot.identifier)",
                    moduleID: moduleID,
                    identifier: snapshot.identifier,
                    service: snapshot.service,
                    codeText: code,
                    title: title,
                    formattedValue: snapshot.formattedValue,
                    rawHex: snapshot.rawHex,
                    rawData: snapshot.rawData,
                    mapped: snapshot.isMapped,
                    unit: snapshot.unit,
                    numericValue: snapshot.isNumericValueAvailable
                        ? snapshot.numericValue : nil)
            }
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
                    rawData: snapshot.rawData,
                    mapped: snapshot.isMapped,
                    unit: snapshot.unit,
                    numericValue: snapshot.isNumericValueAvailable
                        ? snapshot.numericValue : nil)
            }
    }

    func egs53VariantCoding(
        moduleID: String
    ) -> EGS53VariantCodingSummary? {
        guard let value = startupModuleData(moduleID: moduleID).first(where: {
            $0.service == 0x21 && $0.identifier == 0x00B1
        }), value.rawData.count >= 42 else { return nil }

        var decoded = MblinkMercedesEgs53VariantCoding()
        let decodedOK = value.rawData.withUnsafeBytes { bytes -> Bool in
            guard let base = bytes.bindMemory(to: UInt8.self).baseAddress
            else { return false }
            return mblink_mercedes_transmission_decode_egs53_variant_coding(
                base, value.rawData.count, &decoded)
        }
        guard decodedOK else { return nil }

        let raw = Array(value.rawData)
        let variant = String(
            bytes: raw[0..<4], encoding: .ascii)?.uppercased() ?? "Unknown"
        var programs = [String]()
        if decoded.comfort_sport_coding { programs.append("Comfort / Sport") }
        if decoded.manual_program_coding { programs.append("Manual") }
        if decoded.agility_program_coding {
            programs.append("Adaptive / Agility")
        }
        if programs.isEmpty { programs.append("No mapped flags set") }

        func hex(_ range: Range<Int>) -> String {
            guard range.lowerBound >= 0, range.upperBound <= raw.count else {
                return "N/A"
            }
            return raw[range]
                .map { String(format: "%02X", $0) }
                .joined(separator: " ")
        }

        let fingerprintRange = 42..<min(raw.count, 46)
        let fingerprint = fingerprintRange.isEmpty
            ? "Not returned" : hex(fingerprintRange)
        let crcText = decoded.crc_valid
            ? String(format: "Valid · 0x%04X", decoded.stored_crc)
            : String(
                format: "INVALID · stored 0x%04X / calculated 0x%04X",
                decoded.stored_crc, decoded.calculated_crc)

        let facts = [
            EGS53VariantCodingFact(
                id: "variant", label: "Variant coding",
                value: variant, confidence: "decoded"),
            EGS53VariantCodingFact(
                id: "programs", label: "Drive programs",
                value: programs.joined(separator: " · ") +
                    String(format: " · byte 28 = 0x%02X",
                           decoded.drive_program_flags),
                confidence: "corroborated"),
            EGS53VariantCodingFact(
                id: "paddles", label: "Paddle-shift coding",
                value: (decoded.paddle_coding_bit_set ? "Bit set" : "Bit clear") +
                    String(format: " · byte 29 = 0x%02X",
                           decoded.paddle_coding_flags),
                confidence: "best current mapping"),
            EGS53VariantCodingFact(
                id: "axle", label: "Rear axle ratio",
                value: String(
                    format: "%.3f:1",
                    Double(decoded.rear_axle_ratio_milli) / 1000.0),
                confidence: "corroborated"),
            EGS53VariantCodingFact(
                id: "tyre", label: "Tyre circumference",
                value: "\(decoded.tyre_circumference_mm) mm",
                confidence: "corroborated"),
            EGS53VariantCodingFact(
                id: "inertia", label: "Engine inertia",
                value: "\(decoded.engine_inertia_nm) Nm",
                confidence: "corroborated"),
            EGS53VariantCodingFact(
                id: "crc", label: "Coding integrity",
                value: crcText, confidence: "verified"),
            EGS53VariantCodingFact(
                id: "fingerprint", label: "Possible coding fingerprint",
                value: fingerprint, confidence: "inferred")
        ]

        let undecoded = [
            "Bytes 5–27: \(hex(4..<27))",
            "Byte 30: \(hex(29..<30))",
            "Bytes 33–36: \(hex(32..<36))",
            "Byte 40: \(hex(39..<40))"
        ].joined(separator: " · ")

        return EGS53VariantCodingSummary(
            facts: facts,
            undecodedRaw: undecoded,
            fullRaw: value.rawHex)
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

    func refreshPresentation() {
        refreshStandardState()
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
        // Refreshes read this catalogue frequently. Preserve the persisted
        // snapshot without rewriting the whole VIN dictionary when unchanged.
        if let existing = modules[moduleID] as? [[String: Any]],
           NSArray(array: existing).isEqual(to: catalogue) { return }
        modules[moduleID] = catalogue
        vehicles[vin] = modules
        defaults.set(vehicles, forKey: Self.manufacturerCatalogueDefaultsKey)
    }

    private func applyManufacturerPollingSelection(moduleID: String) {
        let catalogue = manufacturerPIDCatalogueItems(moduleID: moduleID)
        let selected = manufacturerSelectionSet(moduleID: moduleID)
        /*
         * Keep service + identifier together. KWP controller catalogues mix
         * 0x1A and 0x21 reads, so reducing a selection to the identifier alone
         * can send the wrong command. Multiple signal stable keys that share
         * one wire command still collapse here to a single scheduled request.
         */
        let wireCommands = Set(
            catalogue
                .filter { selected.contains($0.selectionKey) }
                .map {
                    NSNumber(value:
                        UInt32($0.service) << 16 | UInt32($0.identifier))
                })
        controller.setManufacturerLivePollingCommands(
            Array(wireCommands),
            forModuleIdentifier: moduleID)
    }

    private func applyConfiguredPollingForSelectedVehicle() {
        applyStoredPollingPolicy()
        for module in pidConfigurationModules {
            applyManufacturerPollingSelection(moduleID: module.id)
        }
    }

    private func livePollingSelectionSignature(vin: String) -> String {
        let standard = pidConfigurationModules.map { module in
            let keys = moduleStandardSelectionSet(moduleID: module.id)
                .sorted().joined(separator: ",")
            return "\(module.id)=\(keys)"
        }.sorted().joined(separator: "|")
        let manufacturer = pidConfigurationModules.map { module in
            let keys = manufacturerSelectionSet(moduleID: module.id)
                .sorted().joined(separator: ",")
            return "\(module.id)=\(keys)"
        }.sorted().joined(separator: "|")
        return "\(vin)|standard=\(standard)|manufacturer=\(manufacturer)"
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

    private func standardSelectionExplicitlyEdited(vin: String) -> Bool {
        let values = UserDefaults.standard.dictionary(
            forKey: Self.standardSelectionExplicitDefaultsKey) ?? [:]
        return (values[vin] as? NSNumber)?.boolValue ?? false
    }

    private func markStandardSelectionExplicitlyEdited(vin: String) {
        guard vin.count == 17 else { return }
        let defaults = UserDefaults.standard
        var values = defaults.dictionary(
            forKey: Self.standardSelectionExplicitDefaultsKey) ?? [:]
        values[vin] = true
        defaults.set(values, forKey: Self.standardSelectionExplicitDefaultsKey)
    }

    private func recoverLegacyStandardPollingKeys(vin: String) -> Set<String> {
        var recovered = Set<String>()
        for module in pidConfigurationModules {
            if pidSelectionStore.hasSelection(
                forVIN: vin,
                controllerIdentifier: module.id
            ) {
                recovered.formUnion(pidSelectionStore.stableKeys(
                    forVIN: vin,
                    controllerIdentifier: module.id))
            }
        }

        /* The pre-vehicle-wide selector was global. It is safe to recover a
         * non-empty explicit legacy choice only when this installation has at
         * most one saved vehicle; otherwise ownership is genuinely ambiguous. */
        if recovered.isEmpty && vehicleProfileStore.savedProfiles.count <= 1 {
            recovered.formUnion(legacyGlobalPollingKeys())
        }
        return recovered
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
                let existing = Set(pidSelectionStore.stableKeys(
                    forVIN: vin,
                    controllerIdentifier: controllerID))
                if !existing.isEmpty || standardSelectionExplicitlyEdited(vin: vin) {
                    return existing
                }

                /* v0.7.191 could materialise an empty vehicle-wide selection
                 * before it had recovered the older explicit per-module/global
                 * choices. Heal that empty migration once, but never override
                 * a deliberate all-off choice made in the current model. */
                let recovered = recoverLegacyStandardPollingKeys(vin: vin)
                if !recovered.isEmpty {
                    pidSelectionStore.setStableKeys(
                        Array(recovered).sorted(),
                        forVIN: vin,
                        controllerIdentifier: controllerID)
                    return recovered
                }
                return existing
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

    /*
     * Keep raw source identifiers such as DT_Motortyp_Motortyp in the
     * evidence catalogue, but never leak those internal German CBF symbols
     * into PID Setup. The user-facing provenance is deliberately short,
     * English and free of source-code underscores.
     */
    private func manufacturerPIDPresentationProvenance(
        raw: String,
        service: UInt8,
        identifier: UInt16
    ) -> String {
        let lower = raw.lowercased()
        let source: String
        if lower.contains("vediamo") || lower.contains(".cbf") {
            source = "Mercedes Vediamo CBF"
        } else if lower.contains("ultimate nag52") {
            source = "Ultimate NAG52 documentation"
        } else if lower.contains("xentry") || lower.contains("foxwell") {
            source = "Mercedes diagnostic documentation"
        } else {
            source = "MBLINK documented Mercedes data"
        }

        let command: String
        if service == 0x22 {
            command = String(
                format: "%02X %04X", Int(service), Int(identifier))
        } else {
            command = String(
                format: "%02X %02X", Int(service), Int(identifier))
        }
        return "\(source) · \(command)"
    }

    private func standardStableKey(for pid: UInt8) -> String {
        if let scalar = mblink_parameter_obd2_definition(pid) {
            let key = string(from: scalar.pointee.stable_key)
            if !key.isEmpty { return key }
        }
        return String(format: "sae.obd2.mode01.%02X", pid)
    }

    private func scopedChannelID(
        moduleID: String,
        source: MBPIDCatalogueItem.Source,
        selectionKey: String
    ) -> String {
        let interface = source == .standard ? "obd2" : "mercedes"
        return "\(moduleID)|\(interface)|\(selectionKey)"
    }

    private func controllerCatalogueSort(
        _ left: MBPIDCatalogueItem,
        _ right: MBPIDCatalogueItem
    ) -> Bool {
        let titleOrder = left.title.localizedCaseInsensitiveCompare(right.title)
        if titleOrder != .orderedSame { return titleOrder == .orderedAscending }
        if left.source != right.source { return left.source == .standard }
        if left.service != right.service { return left.service < right.service }
        if left.identifier != right.identifier {
            return left.identifier < right.identifier
        }
        return left.selectionKey < right.selectionKey
    }

    private func readinessFields() -> [LinkReadinessFieldSnapshot] {
        controller.readinessFieldSnapshots()
    }

    private func startupReadinessFieldMask() -> UInt64 {
        readinessFields().reduce(into: UInt64(0)) { mask, field in
            guard field.fieldIndex < 64 else { return }
            mask |= UInt64(1) << UInt64(field.fieldIndex)
        }
    }

    private func sanitizedStandardPollingKeys(
        _ selection: Set<String>
    ) -> Set<String> {
        var sanitized = selection
        sanitized.remove(standardStableKey(for: 0x01))
        sanitized.subtract(readinessFields().map(\.stableKey))
        return sanitized
    }

    private func controllerScopedStandardMigrationComplete(
        vin: String
    ) -> Bool {
        let values = UserDefaults.standard.dictionary(
            forKey: Self.standardControllerScopedMigrationDefaultsKey) ?? [:]
        return (values[vin] as? NSNumber)?.boolValue ?? false
    }

    private func markControllerScopedStandardMigration(vin: String) {
        guard vin.count == 17 else { return }
        let defaults = UserDefaults.standard
        var values = defaults.dictionary(
            forKey: Self.standardControllerScopedMigrationDefaultsKey) ?? [:]
        values[vin] = true
        defaults.set(
            values, forKey: Self.standardControllerScopedMigrationDefaultsKey)
    }

    private func preferredStandardControllerID() -> String? {
        let candidates = pidConfigurationModules.filter {
            !(pidSupportByModule[$0.id] ?? []).isEmpty
        }
        return candidates.first(where: {
            !$0.extendedID && $0.responseCANIdentifier == 0x7E8
        })?.id ?? candidates.first?.id
    }

    private func ensureControllerScopedStandardSelections() {
        guard let vin = effectivePIDConfigurationVIN,
              !pidConfigurationModules.isEmpty,
              !controllerScopedStandardMigrationComplete(vin: vin)
        else { return }

        /*
         * v0.7.186 already stored standard choices per responder. Preserve any
         * such explicit selections. Otherwise move the 0.7.257 vehicle-wide
         * choice to one deterministic primary OBD responder (7E8 when present)
         * so an upgrade cannot unexpectedly enable duplicate channels on every
         * ECU that implements the same SAE PID.
         */
        let existingPerModule = pidConfigurationModules.contains {
            pidSelectionStore.hasSelection(
                forVIN: vin, controllerIdentifier: $0.id)
        }
        let primaryID = preferredStandardControllerID()

        /*
         * The Mercedes census can arrive before responder-specific SAE
         * capability discovery. Do not consume the one-time migration while
         * every controller still has an empty OBD capability map.
         */
        guard existingPerModule || primaryID != nil else { return }

        let legacy = sanitizedStandardPollingKeys(storedPollingKeys())
        for module in pidConfigurationModules {
            if existingPerModule &&
               pidSelectionStore.hasSelection(
                    forVIN: vin, controllerIdentifier: module.id) {
                let expanded = sanitizedStandardPollingKeys(Set(
                    pidSelectionStore.stableKeys(
                        forVIN: vin, controllerIdentifier: module.id)))
                pidSelectionStore.setStableKeys(
                    Array(expanded).sorted(),
                    forVIN: vin,
                    controllerIdentifier: module.id)
                continue
            }

            var migrated = Set<String>()
            if !existingPerModule, module.id == primaryID {
                let supported = pidSupportByModule[module.id] ?? []
                migrated = Set(legacy.filter { key in
                    guard let pid = pidForStableKey(key) else { return false }
                    return supported.contains(pid)
                })
            }
            pidSelectionStore.setStableKeys(
                Array(migrated).sorted(),
                forVIN: vin,
                controllerIdentifier: module.id)
        }
        markControllerScopedStandardMigration(vin: vin)
    }

    private func moduleStandardSelectionSet(
        moduleID: String
    ) -> Set<String> {
        guard let vin = effectivePIDConfigurationVIN else { return [] }
        ensureControllerScopedStandardSelections()
        guard pidSelectionStore.hasSelection(
            forVIN: vin, controllerIdentifier: moduleID)
        else { return [] }

        let stored = Set(pidSelectionStore.stableKeys(
            forVIN: vin, controllerIdentifier: moduleID))
        let expanded = sanitizedStandardPollingKeys(stored)
        if expanded != stored {
            pidSelectionStore.setStableKeys(
                Array(expanded).sorted(),
                forVIN: vin,
                controllerIdentifier: moduleID)
        }
        return expanded
    }

    private func storeModuleStandardSelection(
        _ selection: Set<String>,
        moduleID: String
    ) {
        guard let vin = effectivePIDConfigurationVIN else { return }
        pidSelectionStore.setStableKeys(
            Array(selection).sorted(),
            forVIN: vin,
            controllerIdentifier: moduleID)
    }

    private func aggregateStandardPollingKeys() -> Set<String> {
        ensureControllerScopedStandardSelections()
        return pidConfigurationModules.reduce(into: Set<String>()) {
            $0.formUnion(moduleStandardSelectionSet(moduleID: $1.id))
        }
    }

    private func applyStandardPollingSelection(
        for pid: UInt8,
        selection: Set<String>
    ) {
        if pid == 0x01 {
            // PID 01 is acquired once by LINK during startup diagnostic context.
            // It must never become a recurring live scheduler item.
            controller.setPollingFieldMask(0, forPID: pid)
            controller.setPollingEnabled(false, forPID: pid)
            return
        }
        controller.setPollingEnabled(
            selection.contains(standardStableKey(for: pid)), forPID: pid)
    }

    private func pidForStableKey(_ stableKey: String) -> UInt8? {
        if let field = readinessFields().first(where: {
            $0.stableKey == stableKey
        }) {
            return field.sourcePID
        }
        if stableKey.hasPrefix("sae.obd2.mode01."),
           let value = UInt8(stableKey.suffix(2), radix: 16) { return value }
        return stableKey.withCString { key in
            guard let definition =
                mblink_parameter_obd2_definition_for_stable_key(key)
            else { return nil }
            return UInt8(exactly: definition.pointee.key.identifier)
        }
    }

    private func applyStoredPollingPolicy() {
        let enabled = aggregateStandardPollingKeys()
        let count = mblink_obd2_pid_definition_count()
        guard count > 0 else { return }
        for index in 0..<count {
            guard let definition = mblink_obd2_pid_definition_at(index)
            else { continue }
            let metadata = definition.pointee
            guard metadata.mode == 0x01 else { continue }
            let pid = metadata.pid
            guard (pid & 0x1F) != 0 else { continue }
            applyStandardPollingSelection(for: pid, selection: enabled)
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

    private func formattedValue(pid: UInt8, value: Double?) -> String {
        guard let value else { return "N/A" }
        return controller.formattedDisplayValue(
            forPID: pid, canonicalValue: value)
    }

    private func loadDiagnosticParameters(
        moduleID: String,
        responderCANIdentifier: UInt32,
        extendedID: Bool,
        sourceLabel: String? = nil
    ) -> [DiagnosticParameter] {
        let count = mblink_obd2_pid_definition_count()
        guard count > 0 else { return [] }

        let selectedStandard = moduleStandardSelectionSet(moduleID: moduleID)
        let supported = pidSupportByModule[moduleID] ?? []
        let sourceModule = pidConfigurationModule(id: moduleID)
            ?? diagnosticModule(id: moduleID)
        let requestIdentifier = sourceModule?.requestCANIdentifier ?? 0

        var result = [DiagnosticParameter]()
        result.reserveCapacity(Int(count) + 18)

        for index in 0..<count {
            guard let catalogueDefinition =
                mblink_obd2_pid_definition_at(index) else { continue }
            let catalogue = catalogueDefinition.pointee
            guard catalogue.mode == 0x01 else { continue }
            let pid = catalogue.pid
            guard (pid & 0x1F) != 0, supported.contains(pid) else { continue }

            if pid == 0x01 { continue }

            let scalarDefinition = mblink_parameter_obd2_definition(pid)
            let rawHistory: [Double]
            if scalarDefinition != nil {
                rawHistory = controller.recentValues(
                    forPID: pid,
                    responderCANIdentifier: responderCANIdentifier,
                    extendedID: extendedID,
                    limit: 60).map(\.doubleValue)
            } else {
                rawHistory = []
            }

            let snapshot = controller.standardDataSnapshot(
                forPID: pid,
                responderCANIdentifier: responderCANIdentifier,
                extendedID: extendedID)
            let rawValue = rawHistory.last
            let selectionKey = standardStableKey(for: pid)
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
                history = rawHistory.map {
                    displayScalar(pid: pid, rawValue: $0)
                }
                value = rawValue.map {
                    displayScalar(pid: pid, rawValue: $0)
                }
                formatted = formattedValue(pid: pid, value: rawValue)
            } else {
                title = string(from: catalogue.name)
                shortName = String(format: "PID %02X", pid)
                let unit = string(from: catalogue.unit)
                suffix = unit.isEmpty ? "" : " \(unit)"
                history = []
                value = nil
                formatted = snapshot?.formattedValue ?? "N/A"
            }

            let qualityNote: String?
            if pid == 0x2F, let rawValue, rawValue >= 99.5 {
                qualityNote =
                    "OBD-II · ECU reported 100%; value retained without correction"
            } else if scalarDefinition == nil && snapshot != nil {
                qualityNote =
                    "OBD-II · structured SAE value · exact responder · full payload retained by LINK"
            } else {
                qualityNote = "OBD-II · SAE J1979 · exact responder"
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
                id: scopedChannelID(
                    moduleID: moduleID,
                    source: .standard,
                    selectionKey: selectionKey),
                protocolName: "obd2",
                moduleIdentifier: requestIdentifier,
                parameterIdentifier: UInt32(pid),
                shortName: shortName,
                title: title,
                suffix: suffix,
                formattedValue: formatted,
                value: value,
                structuredValue:
                    scalarDefinition == nil ? snapshot?.formattedValue : nil,
                rawHex: snapshot?.rawHex,
                vehicleSupported: true,
                favourite: controller.favourite(forPID: pid),
                pollingEnabled: selectedStandard.contains(selectionKey),
                history: history,
                sourceLabel: sourceLabel,
                qualityNote: qualityNote,
                dashboardMinimum: dashboardRange?.0,
                dashboardMaximum: dashboardRange?.1))
        }
        return result
    }

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
        return controller.transmissionLiveValueSnapshots(
            identifiers: Array(selected))
            .map { snapshot in
                let numeric = snapshot.isNumericValueAvailable
                    ? snapshot.numericValue : nil
                return DiagnosticParameter(
                    id: scopedChannelID(
                        moduleID: module.id,
                        source: .manufacturer,
                        selectionKey: snapshot.identifier),
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
        /*
         * A functional OBD request can yield several legitimate ECU replies.
         * Keep every controller's logical channels distinct in presentation;
         * physical scheduling is de-duplicated separately by LINK.
         */
        var parameters = [DiagnosticParameter]()
        for module in pidConfigurationModules {
            parameters.append(contentsOf: loadDiagnosticParameters(
                moduleID: module.id,
                responderCANIdentifier: module.responseCANIdentifier,
                extendedID: module.extendedID,
                sourceLabel:
                    "\(module.name) · OBD-II · \(module.addressText)"))
        }

        parameters.append(contentsOf: transmissionDiagnosticParameters())

        // Keep selected manufacturer channels visible before their first reply,
        // and expose non-transmission factory values on the same displays.
        var included = Set(parameters.map(\.id))
        for module in pidConfigurationModules {
            let records = manufacturerData(moduleID: module.id)
            for item in manufacturerPIDCatalogueItems(moduleID: module.id)
                where item.pollingEnabled && !included.contains(item.id) {
                let record = item.selectionKey.hasPrefix(
                    "mercedes.transmission.")
                    ? nil : records.first {
                        $0.service == item.service &&
                        $0.identifier == item.identifier
                    }
                let numeric = record?.numericValue
                let history = numeric.map {
                    manufacturerHistory(
                        id: item.id,
                        value: $0,
                        rawHex: record?.rawHex ?? "")
                } ?? []
                parameters.append(DiagnosticParameter(
                    id: item.id,
                    protocolName: item.sourceText.lowercased(),
                    moduleIdentifier: module.requestCANIdentifier,
                    parameterIdentifier:
                        UInt32(item.service) << 16 |
                        UInt32(item.identifier),
                    shortName: item.shortName,
                    title: item.title,
                    suffix: record?.unit ?? "",
                    formattedValue:
                        record?.formattedValue ?? "Waiting for sample",
                    value: numeric,
                    structuredValue:
                        numeric == nil ? record?.formattedValue : nil,
                    rawHex: record?.rawHex,
                    vehicleSupported: true,
                    favourite: false,
                    pollingEnabled: true,
                    history: history,
                    sourceLabel:
                        "\(module.name) · \(item.sourceText) · \(module.addressText)",
                    qualityNote: item.provenance))
                included.insert(item.id)
            }
        }
        return parameters
    }

    private func offlineMercedesProtocolName(_ protocolValue: UInt) -> String {
        switch protocolValue {
        case 0: return "UDS"
        case 1: return "KWP2000"
        default: return "Mercedes diagnostic protocol"
        }
    }

    private func offlineModuleName(
        tx: UInt32,
        rx: UInt32,
        extended: Bool,
        kind: Int,
        protocolValue: UInt,
        identityText: String?,
        partNumber: String?,
        softwareNumber: String?,
        hardwareNumber: String?
    ) -> String {
        let resolved = controller.resolvedMercedesModuleName(
            requestCANIdentifier: tx,
            responseCANIdentifier: rx,
            extendedID: extended,
            protocol: protocolValue,
            identityText: identityText,
            partNumber: partNumber,
            softwareNumber: softwareNumber,
            hardwareNumber: hardwareNumber)
        if !resolved.isEmpty && resolved != "Mercedes ECU" {
            return resolved
        }

        // Do not reuse an old coarse numeric kind when the authoritative
        // identity resolver cannot classify this route. Old profiles may have
        // carried broad/body/ABS guesses that were never ECU identity.
        if extended {
            return String(format: "Unknown Mercedes ECU 0x%08X", tx)
        }
        return String(format: "Unknown Mercedes ECU 0x%03X", tx)
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

                let identityText = saved["identity"] as? String
                let partNumber = saved["sparePart"] as? String
                let softwareNumber = saved["software"] as? String
                let hardwareNumber = saved["hardware"] as? String
                let protocolValue =
                    (saved["protocol"] as? NSNumber)?.uintValue ?? 0
                let offlineName = offlineModuleName(
                    tx: tx,
                    rx: rx,
                    extended: extended,
                    kind: kind,
                    protocolValue: protocolValue,
                    identityText: identityText,
                    partNumber: partNumber,
                    softwareNumber: softwareNumber,
                    hardwareNumber: hardwareNumber)
                modules.append(DiagnosticModule(
                    id: moduleID,
                    name: offlineName,
                    designation: "Saved vehicle controller",
                    network: "Saved VIN profile",
                    kind: offlineName.lowercased(),
                    protocolName: offlineMercedesProtocolName(protocolValue),
                    requestCANIdentifier: tx,
                    responseCANIdentifier: rx,
                    extendedID: extended,
                    identityText: identityText,
                    partNumber: partNumber,
                    softwareNumber: softwareNumber,
                    hardwareNumber: hardwareNumber,
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
                    tx: tx,
                    rx: rx,
                    extended: extended,
                    kind: 0,
                    protocolValue: 0,
                    identityText: nil,
                    partNumber: nil,
                    softwareNumber: nil,
                    hardwareNumber: nil),
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
            ensureControllerScopedStandardSelections()
            pidConfigurationSourceText =
                "Current vehicle · controller-owned OBD-II + Mercedes catalogue"
            return
        }

        let saved = loadSavedPIDConfiguration()
        pidSupportByModule = saved.support
        pidConfigurationModules = saved.modules
        ensureControllerScopedStandardSelections()
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
                      mblink_obd2_pid_definition(0x01, pid) != nil
                else { continue }
                if pid == 0x01 { continue }
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

    override func productDashboardParameters(
        standard: [LinkDiagnosticParameter]
    ) -> [LinkDiagnosticParameter] {
        // PID Setup is the sole display-membership authority in MBLINK.
        // Ignore LINK's generic dashboard preference store completely.
        enabledDisplayParameters
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

        /*
         * A live connection deliberately suppresses offline selections until
         * its VIN is known. Re-apply that VIN's complete saved selection once
         * LINK has finished building the real live scheduler. This is a
         * one-shot per selection fingerprint, so delegate refreshes cannot
         * recurse, but changing an 84-PID setup to an 85-PID setup re-arms the
         * scheduler immediately as well.
         */
        if isActive, isReady, let vin = activeVehicleVIN {
            let signature = livePollingSelectionSignature(vin: vin)
            if signature != livePollingReadyRearmSignature {
                livePollingReadyRearmSignature = signature
                appliedPollingConfigurationKey = nil
                applyConfiguredPollingIfNeeded(force: true)
            }
        } else {
            livePollingReadyRearmSignature = nil
        }
        applyConfiguredPollingIfNeeded()
#if MBLINK_CI_SIMULATED_FLOW
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1",
           isSimulationActive {
            let liveVIN = controller.mercedesVINText ?? ""
            let selectionVIN = effectivePIDConfigurationVIN ?? ""
            let standardSelectionPairs = pidConfigurationModules.flatMap {
                module in moduleStandardSelectionSet(moduleID: module.id).map {
                    "\(module.id)=\($0)"
                }
            }.sorted()
            let standardSelectionKeys =
                aggregateStandardPollingKeys().sorted()
            let standardSelectionCount = pidConfigurationModules.reduce(0) {
                $0 + moduleStandardSelectionSet(moduleID: $1.id).count
            }
            let standardSelectionScoped = selectionVIN.count == 17 &&
                controllerScopedStandardMigrationComplete(vin: selectionVIN)
            let transmissionCatalogueCount = manufacturerPIDCatalogueItems(
                moduleID: Self.ciTransmissionModuleID).count
            let espCatalogueCount = manufacturerPIDCatalogueItems(
                moduleID: Self.ciESPModuleID).count
            let orcCatalogueCount = manufacturerPIDCatalogueItems(
                moduleID: Self.ciORCModuleID).count
            let udsServiceCatalogueCount = udsServiceCatalogue.count
            let udsDTCReportCatalogueCount = udsDTCReportCatalogue.count
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
                "standard_selection_keys=\(standardSelectionKeys.joined(separator: ","))\n" +
                "standard_selection_pairs=\(standardSelectionPairs.joined(separator: ","))\n" +
                "standard_selection_scoped=\(standardSelectionScoped)\n" +
                "module_count=\(diagnosticModules.count)\n" +
                "transmission_catalogue_count=\(transmissionCatalogueCount)\n" +
                "esp_catalogue_count=\(espCatalogueCount)\n" +
                "orc_catalogue_count=\(orcCatalogueCount)\n" +
                "uds_service_catalogue_count=\(udsServiceCatalogueCount)\n" +
                "uds_dtc_report_catalogue_count=\(udsDTCReportCatalogueCount)\n" +
                "recorded_samples=\(recordedSampleCount)\n"
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

#if MBLINK_CI_SIMULATED_FLOW
    private static let ciTransmissionModuleID = "11:000007E1:000007E9"
    private static let ciESPModuleID = "11:00000632:00000486"
    private static let ciORCModuleID = "11:0000064A:00000489"

    private func verifySingleDisplaySelection() -> Bool {
        guard !isActive, effectivePIDConfigurationVIN != nil else {
            return false
        }
        let oldStandard = pidConfigurationModules.map {
            ($0.id, moduleStandardSelectionSet(moduleID: $0.id))
        }
        let oldManufacturer = pidConfigurationModules.map {
            ($0.id, manufacturerSelectionSet(moduleID: $0.id))
        }
        defer {
            for (moduleID, keys) in oldStandard {
                storeModuleStandardSelection(keys, moduleID: moduleID)
            }
            for (moduleID, keys) in oldManufacturer {
                storeManufacturerSelection(keys, moduleID: moduleID)
            }
            applyConfiguredPollingIfNeeded(force: true)
            refreshStandardState()
        }

        resetPIDSelectionsForCurrentVehicle()
        guard enabledDisplayParameters.isEmpty else { return false }
        guard let standardModule = pidConfigurationModules.first(where: {
            !standardPIDCatalogueItems(moduleID: $0.id).isEmpty
        }) else { return false }
        let standard = Array(
            standardPIDCatalogueItems(moduleID: standardModule.id)
                .prefix(2).map(\.selectionKey))
        let transmission = [
            "mercedes.transmission.oil_temperature",
            "mercedes.transmission.actual_gear"
        ]
        guard standard.count == 2 else { return false }

        for key in standard {
            setStandardPIDSelection(
                true, moduleID: standardModule.id, stableKey: key)
        }
        for key in transmission {
            setManufacturerPIDSelection(
                true,
                moduleID: Self.ciTransmissionModuleID,
                stableKey: key)
        }

        let expected = Set(
            standard.map {
                scopedChannelID(
                    moduleID: standardModule.id,
                    source: .standard,
                    selectionKey: $0)
            } +
            transmission.map {
                scopedChannelID(
                    moduleID: Self.ciTransmissionModuleID,
                    source: .manufacturer,
                    selectionKey: $0)
            })
        /*
         * During the cold-launch regression this helper runs from init before
         * Combine has published the next inherited diagnosticParameters value.
         * Verify the exact product parameter set directly; refreshStandardState
         * publishes this same set immediately afterwards in normal UI use.
         */
        let visible = loadPrimaryDiagnosticParameters().filter(\.pollingEnabled)
        guard Set(visible.map(\.id)) == expected else { return false }

        for key in standard {
            setStandardPIDSelection(
                false, moduleID: standardModule.id, stableKey: key)
        }
        for key in transmission {
            setManufacturerPIDSelection(
                false,
                moduleID: Self.ciTransmissionModuleID,
                stableKey: key)
        }
        return loadPrimaryDiagnosticParameters()
            .filter(\.pollingEnabled).isEmpty
    }

    private func writeSavedPIDCatalogueRegressionMarker() {
        let transmissionCount = manufacturerPIDCatalogueItems(
            moduleID: Self.ciTransmissionModuleID).count
        let espCount = manufacturerPIDCatalogueItems(
            moduleID: Self.ciESPModuleID).count
        let orcCount = manufacturerPIDCatalogueItems(
            moduleID: Self.ciORCModuleID).count
        let displaySelectionVerified = verifySingleDisplaySelection()
        let ready = !isActive && selectedVehicleVIN?.count == 17 &&
            pidConfigurationModules.count >= 4 && transmissionCount > 0 &&
            displaySelectionVerified
        let marker = "state=\(ready ? "ready" : "failed")\n" +
            "active=\(isActive)\n" +
            "selected_vin=\(selectedVehicleVIN ?? "")\n" +
            "profile_module_count=\(pidConfigurationModules.count)\n" +
            "profile_source=\(pidConfigurationSourceText)\n" +
            "transmission_catalogue_count=\(transmissionCount)\n" +
            "esp_catalogue_count=\(espCount)\n" +
            "orc_catalogue_count=\(orcCount)\n" +
            "display_selection_verified=\(displaySelectionVerified)\n"
        if let directory = FileManager.default.urls(
                for: .documentDirectory, in: .userDomainMask).first {
            try? FileManager.default.createDirectory(
                at: directory, withIntermediateDirectories: true)
            try? marker.write(
                to: directory.appendingPathComponent(
                    "mblink-ci-saved-pid-profile.ok"),
                atomically: true, encoding: .utf8)
        }
    }
#endif
}
