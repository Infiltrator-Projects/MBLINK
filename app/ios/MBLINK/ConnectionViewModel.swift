// SPDX-License-Identifier: GPL-3.0-or-later
import Combine
import Foundation

@MainActor
final class ConnectionViewModel: NSObject, ObservableObject, MBLinkDiagnosticsControllerDelegate {
    @Published private(set) var statusText = "Idle"
    @Published private(set) var peripheralName = "No adapter"
    @Published private(set) var adapterIdentifier = "Unknown"
    @Published private(set) var isActive = false
    @Published private(set) var isReady = false

    @Published private(set) var engineLoadPercent: Double?
    @Published private(set) var coolantTemperatureCelsius: Double?
    @Published private(set) var manifoldPressureKPa: Double?
    @Published private(set) var rpm: Double?
    @Published private(set) var vehicleSpeedKmh: Double?
    @Published private(set) var intakeAirTemperatureCelsius: Double?
    @Published private(set) var massAirFlowGramsPerSecond: Double?
    @Published private(set) var throttlePositionPercent: Double?

    @Published private(set) var recordedSampleCount = 0
    @Published private(set) var favouritePIDs = Set<UInt8>()
    @Published private(set) var recentRPM = [Double]()
    @Published private(set) var recentCoolant = [Double]()
    @Published private(set) var csvExportURL: URL?

    private let controller = MBLinkDiagnosticsController()
    private let dashboardPIDs: [UInt8] = [
        0x0c, 0x0d, 0x0b, 0x11, 0x04, 0x10, 0x05, 0x0f
    ]

    override init() {
        super.init()
        controller.delegate = self
        refresh()
    }

    func connect() {
        clearPreparedExport()
        controller.start()
    }

    func disconnect() {
        controller.disconnect()
    }

    func toggleFavourite(pid: UInt8) {
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
        let filename = "MBLINK-session-\(UUID().uuidString).csv"
        let url = FileManager.default.temporaryDirectory
            .appendingPathComponent(filename)
        do {
            try data.write(to: url, options: .atomic)
            csvExportURL = url
        } catch {
            csvExportURL = nil
        }
    }

    func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {
        refresh()
    }

    private func clearPreparedExport() {
        if let url = csvExportURL {
            try? FileManager.default.removeItem(at: url)
        }
        csvExportURL = nil
    }

    private func refresh() {
        statusText = controller.statusText
        peripheralName = controller.peripheralName ?? "No adapter"
        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        isActive = controller.isActive
        isReady = controller.isReady

        engineLoadPercent = controller.hasEngineLoad
            ? controller.engineLoadPercent : nil
        coolantTemperatureCelsius = controller.hasCoolantTemperature
            ? controller.coolantTemperatureCelsius : nil
        manifoldPressureKPa = controller.hasManifoldPressure
            ? controller.manifoldPressureKPa : nil
        rpm = controller.hasRPM ? controller.rpm : nil
        vehicleSpeedKmh = controller.hasVehicleSpeed
            ? controller.vehicleSpeedKmh : nil
        intakeAirTemperatureCelsius = controller.hasIntakeAirTemperature
            ? controller.intakeAirTemperatureCelsius : nil
        massAirFlowGramsPerSecond = controller.hasMassAirFlow
            ? controller.massAirFlowGramsPerSecond : nil
        throttlePositionPercent = controller.hasThrottlePosition
            ? controller.throttlePositionPercent : nil

        recordedSampleCount = Int(clamping: controller.recordedSampleCount)
        favouritePIDs = Set(
            dashboardPIDs.filter { controller.favourite(forPID: $0) }
        )
        recentRPM = controller
            .recentValues(forPID: 0x0c, limit: 60)
            .map(\.doubleValue)
        recentCoolant = controller
            .recentValues(forPID: 0x05, limit: 60)
            .map(\.doubleValue)
    }
}
