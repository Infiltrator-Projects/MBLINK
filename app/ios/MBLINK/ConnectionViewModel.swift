// SPDX-License-Identifier: GPL-3.0-or-later
import Combine
import Foundation

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
    @Published private(set) var mercedesProbeEndpointText = "Candidate not selected"
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
    @Published private(set) var isActive = false
    @Published private(set) var isReady = false

    @Published private(set) var diagnosticParameters = [DiagnosticParameter]()
    @Published private(set) var mercedesTargetSignals = [MercedesTargetSignal]()
    @Published private(set) var recordedSampleCount = 0
    @Published private(set) var csvExportURL: URL?

    private let controller = MBLinkDiagnosticsController()

    override init() {
        super.init()
        controller.delegate = self
        mercedesTargetSignals = loadMercedesTargetSignals()
        refresh()
    }

    func connect() {
        clearPreparedExport()
        controller.start()
    }

    func disconnect() {
        controller.disconnect()
    }

    func parameter(stableKey: String) -> DiagnosticParameter? {
        diagnosticParameters.first { $0.id == stableKey }
    }

    func mercedesSignals(category: String) -> [MercedesTargetSignal] {
        mercedesTargetSignals.filter { $0.category == category }
    }

    func toggleFavourite(stableKey: String) {
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

    func prepareCSVExport() {
        guard let csv = controller.csvSnapshot(),
              let data = csv.data(using: .utf8) else {
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
            self?.refresh()
        }
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
        statusText = controller.statusText
        peripheralName = controller.peripheralName ?? "No adapter"
        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        mercedesProbeStatusText = controller.mercedesProbeStatusText
        mercedesProbeEndpointText = controller.mercedesProbeEndpointText ?? "Candidate not selected"
        mercedesVINText = controller.mercedesVINText ?? "Not captured"
        mercedesIdentitySummaryText = controller.mercedesIdentitySummaryText
        mercedesIdentityResults = controller.mercedesIdentityResults
        mercedesCrd3SummaryText = controller.mercedesCrd3SummaryText
        mercedesUDSFaultStatusText = controller.mercedesUDSFaultStatusText
        mercedesUDSFaults = controller.mercedesUDSFaults
        faultScanStatusText = controller.faultScanStatusText
        storedDTCs = controller.storedDTCs
        pendingDTCs = controller.pendingDTCs
        permanentDTCs = controller.permanentDTCs
        isActive = controller.isActive
        isReady = controller.isReady
        diagnosticParameters = loadDiagnosticParameters()
        recordedSampleCount = Int(clamping: controller.recordedSampleCount)
    }
}
