// SPDX-License-Identifier: GPL-3.0-or-later
import SwiftUI

struct ContentView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        NavigationStack {
            List {
                Section("Connection") {
                    LabeledContent("Status", value: connection.statusText)
                    LabeledContent("Adapter", value: connection.peripheralName)
                    LabeledContent("Identity", value: connection.adapterIdentifier)

                    if connection.isActive {
                        Button(connection.isReady ? "Disconnect" : "Cancel", role: .destructive) {
                            connection.disconnect()
                        }
                    } else {
                        Button("Connect to OBD adapter") {
                            connection.connect()
                        }
                    }
                }

                Section("Live OBD-II") {
                    measurementRow(
                        title: "Engine speed",
                        value: connection.rpm.map { String(format: "%.0f rpm", $0) }
                    )
                    measurementRow(
                        title: "Coolant temperature",
                        value: connection.coolantTemperatureCelsius.map {
                            String(format: "%.0f °C", $0)
                        }
                    )
                }

                Section {
                    Text("Diagnostic parsing and PID formulas are produced by the portable C core. CoreBluetooth remains an Apple transport provider only.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("MBLINK")
        }
    }

    @ViewBuilder
    private func measurementRow(title: String, value: String?) -> some View {
        LabeledContent(title) {
            Text(value ?? "—")
                .monospacedDigit()
        }
    }
}
