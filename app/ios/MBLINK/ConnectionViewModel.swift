// SPDX-License-Identifier: GPL-3.0-or-later
import Combine
import Foundation
import UIKit

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

    var displayText: String {
        "\(code) — \(title)"
    }
}

struct MercedesTargetSignal: Identifiable {
    let id: String
    let title: String
    let category: String
    let status: String
    let provenance: String
}

@MainActor
final class ConnectionViewModel: NSObject, ObservableObject, MBLinkDiagnosticsControllerDelegate {
    @Published private(set) var statusText = "Idle"
    @Published private(set) var peripheralName = "No adapter"
    @Published private(set) var adapterIdentifier = "Unknown"
    @Published private(set) var mercedesProbeStatusText = "Not attempted"
    @Published private(set) var mercedesProbeEndpointText = "Source-corroborated endpoint not selected"
    @Published private(set) var mercedesVINText = "Not captured"
    @Published private(set) var mercedesIdentitySummaryText = "Not attempted"
    @Published private(set) var mercedesIdentityResults = [String]()
    @Published private(set) var mercedesCrd3SummaryText = "Not attempted"
    @Published private(set) var mercedesUDSFaultStatusText = "Not scanned"
    @Published private(set) var mercedesUDSFaults = [String]()
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

    private let controller = MBLinkDiagnosticsController()
    private var simulationTimer: Timer?
    private var simulationStep = 0
    private var simulationHistory = [String: [Double]]()
    private var simulationFavourites: Set<String> = [
        "obd2.engine.rpm",
        "obd2.vehicle.speed",
        "obd2.engine.coolant",
        "obd2.diesel.rail_pressure"
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
        controller.delegate = self
        mercedesTargetSignals = loadMercedesTargetSignals()
        refresh()
    }

    func connect() {
        clearPreparedExport()
        if isSimulationActive { return }

        let alert = UIAlertController(
            title: "Connection Test",
            message: "Choose a real Bluetooth adapter or run the built-in simulated ECU.",
            preferredStyle: .alert
        )
        alert.addAction(UIAlertAction(title: "Real Adapter", style: .default) { [weak self] _ in
            Task { @MainActor [weak self] in
                self?.controller.start()
            }
        })
        alert.addAction(UIAlertAction(title: "Simulated ECU", style: .default) { [weak self] _ in
            Task { @MainActor [weak self] in
                self?.startSimulation()
            }
        })
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel))

        guard let presenter = presentingViewController() else {
            controller.start()
            return
        }
        presenter.present(alert, animated: true)
    }

    func disconnect() {
        if isSimulationActive {
            stopSimulation()
        } else {
            controller.disconnect()
        }
    }

    func parameter(stableKey: String) -> DiagnosticParameter? {
        diagnosticParameters.first { $0.id == stableKey }
    }

    func mercedesSignals(category: String) -> [MercedesTargetSignal] {
        mercedesTargetSignals.filter { $0.category == category }
    }

    func toggleFavourite(stableKey: String) {
        if isSimulationActive {
            if simulationFavourites.contains(stableKey) {
                simulationFavourites.remove(stableKey)
            } else {
                simulationFavourites.insert(stableKey)
            }
            rebuildSimulationParameters()
            return
        }

        let pid: UInt8? = stableKey.withCString { key in
            guard let definition = mblink_parameter_obd2_definition_for_stable_key(key) else {
                return nil
            }
            return UInt8(exactly: definition.pointee.key.identifier)
        }
        guard let pid else { return }

        controller.setFavourite(!controller.favourite(forPID: pid), forPID: pid)
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
        let csv: String?
        if isSimulationActive {
            csv = simulationCSV()
        } else {
            csv = controller.csvSnapshot()
        }
        guard let csv, let data = csv.data(using: .utf8) else {
            clearPreparedExport()
            return
        }

        clearPreparedExport()
        let filename = "MBLINK-diagnostic-evidence-\(UUID().uuidString).csv"
        let url = FileManager.default.temporaryDirectory
            .appendingPathComponent(filename)
        do {
            try data.write(to: url, options: .atomic)
            csvExportURL = url
        } catch {
            csvExportURL = nil
        }
    }

    nonisolated func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {
        Task { @MainActor [weak self] in
            guard let self, !self.isSimulationActive else { return }
            self.refresh()
        }
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
        if let presented = controller.presentedViewController {
            return topViewController(presented)
        }
        if let navigation = controller as? UINavigationController,
           let visible = navigation.visibleViewController {
            return topViewController(visible)
        }
        if let tabs = controller as? UITabBarController,
           let selected = tabs.selectedViewController {
            return topViewController(selected)
        }
        return controller
    }

    private func startSimulation() {
        isSimulationActive = true
        if controller.isActive { controller.disconnect() }
        simulationTimer?.invalidate()
        simulationStep = 0
        simulationHistory.removeAll()

        isActive = true
        isReady = true
        statusText = "Simulated ECU"
        peripheralName = "MBLINK Demo Adapter"
        adapterIdentifier = "ELM327 SIM v1.0"
        mercedesProbeStatusText = "Complete · simulated CRD3 response"
        mercedesProbeEndpointText = "Engine ECU · 0x7E0 → 0x7E8 · SIMULATED"
        mercedesVINText = "WDD2073032F000001"
        mercedesIdentitySummaryText = "3 simulated identity records"
        mercedesIdentityResults = [
            "VIN · WDD2073032F000001 · SIMULATED",
            "ECU · Delphi CRD3.x · SIMULATED",
            "Software · MBLINK-DEMO-1 · SIMULATED"
        ]
        mercedesCrd3SummaryText = "Delphi CRD3.x · simulated"
        mercedesUDSFaultStatusText = "Complete · 2 simulated records"
        mercedesUDSFaults = [
            "13A200 · status 0x09",
            "17F100 · status 0x08"
        ]
        faultScanStatusText = "Complete · simulated vehicle response"

        storedFaults = resolveFaults(["P0401", "P0101"], state: "Stored")
        pendingFaults = resolveFaults(["P0299"], state: "Pending")
        permanentFaults = resolveFaults(["P2002"], state: "Permanent")
        storedDTCs = storedFaults.map(\.displayText)
        pendingDTCs = pendingFaults.map(\.displayText)
        permanentDTCs = permanentFaults.map(\.displayText)
        recordedSampleCount = 0
        rebuildSimulationParameters()

        let timer = Timer(
            timeInterval: 1.0,
            target: self,
            selector: #selector(simulationTimerFired),
            userInfo: nil,
            repeats: true
        )
        simulationTimer = timer
        RunLoop.main.add(timer, forMode: .common)
    }

    private func stopSimulation() {
        simulationTimer?.invalidate()
        simulationTimer = nil
        simulationHistory.removeAll()
        isSimulationActive = false
        refresh()
    }

    @objc private func simulationTimerFired() {
        guard isSimulationActive else { return }
        simulationStep += 1
        rebuildSimulationParameters()
        recordedSampleCount += diagnosticParameters.filter(\.isAvailable).count
    }

    private func simulatedValue(for stableKey: String) -> Double? {
        let wave = sin(Double(simulationStep) * 0.33)
        let slowWave = sin(Double(simulationStep) * 0.11)
        switch stableKey {
        case "obd2.engine.rpm": return 980.0 + (wave * 360.0)
        case "obd2.vehicle.speed": return 52.0 + (slowWave * 20.0)
        case "obd2.engine.map": return 118.0 + (wave * 20.0)
        case "obd2.engine.throttle": return 22.0 + (wave * 7.0)
        case "obd2.engine.load": return 41.0 + (wave * 13.0)
        case "obd2.engine.maf": return 18.5 + (wave * 4.8)
        case "obd2.engine.coolant": return 91.0 + (slowWave * 1.2)
        case "obd2.engine.intake_air": return 29.0 + (slowWave * 2.0)
        case "obd2.diesel.rail_pressure": return 34500.0 + (wave * 7200.0)
        case "obd2.diesel.egr_command": return 30.0 + (wave * 10.0)
        case "obd2.diesel.egr_error": return wave * 2.0
        case "obd2.engine.barometric_pressure": return 100.0
        case "obd2.aftertreatment.catalyst_temp_b1s1": return 382.0 + (wave * 25.0)
        case "obd2.electrical.control_module_voltage": return 14.21 + (wave * 0.07)
        case "obd2.environment.ambient_air": return 24.0
        case "obd2.engine.oil_temperature": return 96.0 + slowWave
        case "obd2.engine.fuel_rate": return 4.9 + (wave * 1.0)
        case "obd2.aftertreatment.egt_b1s1": return 430.0 + (wave * 40.0)
        case "obd2.dpf.bank1_delta_pressure": return 1.85 + (wave * 0.40)
        case "obd2.dpf.bank1_inlet_temperature": return 365.0 + (wave * 30.0)
        default: return nil
        }
    }

    private func rebuildSimulationParameters() {
        let count = mblink_parameter_obd2_definition_count()
        var result = [DiagnosticParameter]()
        result.reserveCapacity(count)

        for index in 0..<count {
            guard let definition = mblink_parameter_obd2_definition_at(index) else { continue }
            let metadata = definition.pointee
            let stableKey = string(from: metadata.stable_key)
            guard !stableKey.isEmpty else { continue }

            let value = simulatedValue(for: stableKey)
            if let value {
                var history = simulationHistory[stableKey, default: []]
                if history.last != value {
                    history.append(value)
                    if history.count > 60 { history.removeFirst(history.count - 60) }
                    simulationHistory[stableKey] = history
                }
            }

            result.append(DiagnosticParameter(
                id: stableKey,
                protocolName: string(from: mblink_parameter_protocol_name(metadata.key.protocol)),
                moduleIdentifier: metadata.key.module,
                parameterIdentifier: metadata.key.identifier,
                shortName: string(from: metadata.short_name),
                title: string(from: metadata.name),
                suffix: string(from: metadata.suffix),
                formattedValue: formattedValue(definition: definition, value: value),
                value: value,
                favourite: simulationFavourites.contains(stableKey),
                history: simulationHistory[stableKey] ?? []
            ))
        }
        diagnosticParameters = result
    }

    private func simulationCSV() -> String {
        var rows = ["timestamp_ms,source,type,key,value"]
        let timestamp = Int(Date().timeIntervalSince1970 * 1000.0)
        rows.append("\(timestamp),simulated,adapter,identity,ELM327 SIM v1.0")
        rows.append("\(timestamp),simulated,vehicle,vin,WDD2073032F000001")
        rows.append("\(timestamp),simulated,dtc,stored,P0401")
        rows.append("\(timestamp),simulated,dtc,stored,P0101")
        rows.append("\(timestamp),simulated,dtc,pending,P0299")
        rows.append("\(timestamp),simulated,dtc,permanent,P2002")
        for parameter in diagnosticParameters where parameter.value != nil {
            rows.append("\(timestamp),simulated,parameter,\(parameter.id),\(parameter.formattedValue)")
        }
        return rows.joined(separator: "\n") + "\n"
    }

    private func clearPreparedExport() {
        if let url = csvExportURL {
            try? FileManager.default.removeItem(at: url)
        }
        csvExportURL = nil
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
        let resolved = code.withCString { rawCode in
            link_dtc_resolve(rawCode, &knowledge)
        }

        guard resolved else {
            return DiagnosticFault(
                code: code,
                title: "Invalid diagnostic trouble code",
                system: "Unknown",
                category: "Unclassified",
                origin: "Unknown",
                source: "Invalid raw code",
                state: state,
                definitionKnown: false
            )
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
        let category = known
            ? stringFromFixedCString(knowledge.category)
            : "Unmapped"

        return DiagnosticFault(
            code: normalizedCode.isEmpty ? code : normalizedCode,
            title: title,
            system: system,
            category: category,
            origin: origin,
            source: source,
            state: state,
            definitionKnown: known
        )
    }

    private func resolveFaults(_ codes: [String], state: String) -> [DiagnosticFault] {
        codes.map { resolveFault($0, state: state) }
    }

    private func formattedValue(
        definition: UnsafePointer<MblinkParameterDefinition>,
        value: Double?
    ) -> String {
        var buffer = [CChar](repeating: 0, count: 96)
        let success = buffer.withUnsafeMutableBufferPointer { storage in
            mblink_parameter_format_value(
                definition,
                value != nil,
                value ?? 0.0,
                storage.baseAddress,
                storage.count
            )
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
            guard let definition = mblink_parameter_obd2_definition_at(index) else {
                continue
            }

            let metadata = definition.pointee
            guard let pid = UInt8(exactly: metadata.key.identifier) else {
                continue
            }

            let history = controller
                .recentValues(forPID: pid, limit: 60)
                .map(\.doubleValue)
            let value = history.last
            let stableKey = string(from: metadata.stable_key)
            guard !stableKey.isEmpty else { continue }

            result.append(
                DiagnosticParameter(
                    id: stableKey,
                    protocolName: string(
                        from: mblink_parameter_protocol_name(metadata.key.protocol)
                    ),
                    moduleIdentifier: metadata.key.module,
                    parameterIdentifier: metadata.key.identifier,
                    shortName: string(from: metadata.short_name),
                    title: string(from: metadata.name),
                    suffix: string(from: metadata.suffix),
                    formattedValue: formattedValue(
                        definition: definition,
                        value: value
                    ),
                    value: value,
                    favourite: controller.favourite(forPID: pid),
                    history: history
                )
            )
        }

        return result
    }

    private func loadMercedesTargetSignals() -> [MercedesTargetSignal] {
        let count = Int(mblink_mercedes_om651_catalog_count())
        guard count > 0 else { return [] }

        var result = [MercedesTargetSignal]()
        result.reserveCapacity(count)
        for index in 0..<count {
            guard let definition = mblink_mercedes_om651_catalog_at(index) else {
                continue
            }
            let metadata = definition.pointee
            let key = string(from: metadata.key)
            guard !key.isEmpty else { continue }
            result.append(
                MercedesTargetSignal(
                    id: key,
                    title: string(from: metadata.name),
                    category: string(
                        from: mblink_mercedes_om651_signal_category_name(metadata.category)
                    ),
                    status: string(
                        from: mblink_mercedes_om651_signal_status_name(metadata.status)
                    ),
                    provenance: string(from: metadata.provenance)
                )
            )
        }
        return result
    }

    private func refresh() {
        guard !isSimulationActive else { return }
        statusText = controller.statusText
        peripheralName = controller.peripheralName ?? "No adapter"
        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        mercedesProbeStatusText = controller.mercedesProbeStatusText
        mercedesProbeEndpointText = controller.mercedesProbeEndpointText ?? "Source-corroborated endpoint not selected"
        mercedesVINText = controller.mercedesVINText ?? "Not captured"
        mercedesIdentitySummaryText = controller.mercedesIdentitySummaryText
        mercedesIdentityResults = controller.mercedesIdentityResults
        mercedesCrd3SummaryText = controller.mercedesCrd3SummaryText
        mercedesUDSFaultStatusText = controller.mercedesUDSFaultStatusText
        mercedesUDSFaults = controller.mercedesUDSFaults
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
