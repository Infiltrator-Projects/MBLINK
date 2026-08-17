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
    @Published private(set) var rpm: Double?
    @Published private(set) var coolantTemperatureCelsius: Double?

    private let controller = MBLinkDiagnosticsController()

    override init() {
        super.init()
        controller.delegate = self
        refresh()
    }

    func connect() {
        controller.start()
    }

    func disconnect() {
        controller.disconnect()
    }

    func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {
        refresh()
    }

    private func refresh() {
        statusText = controller.statusText
        peripheralName = controller.peripheralName ?? "No adapter"
        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        isActive = controller.isActive
        isReady = controller.isReady
        rpm = controller.hasRPM ? controller.rpm : nil
        coolantTemperatureCelsius = controller.hasCoolantTemperature
            ? controller.coolantTemperatureCelsius
            : nil
    }
}
