// SPDX-License-Identifier: GPL-3.0-or-later
import Charts
import Foundation
import SwiftUI

struct ContentView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private struct Metric {
        let pid: UInt8
        let title: String
        let value: String?
    }

    private struct HistoryPoint: Identifiable {
        let id: Int
        let value: Double
    }

    private var metrics: [Metric] {
        [
            Metric(
                pid: 0x0c,
                title: "Engine speed",
                value: connection.rpm.map { String(format: "%.0f rpm", $0) }
            ),
            Metric(
                pid: 0x0d,
                title: "Vehicle speed",
                value: connection.vehicleSpeedKmh.map {
                    String(format: "%.0f km/h", $0)
                }
            ),
            Metric(
                pid: 0x0b,
                title: "Manifold pressure",
                value: connection.manifoldPressureKPa.map {
                    String(format: "%.0f kPa", $0)
                }
            ),
            Metric(
                pid: 0x11,
                title: "Throttle position",
                value: connection.throttlePositionPercent.map {
                    String(format: "%.0f%%", $0)
                }
            ),
            Metric(
                pid: 0x04,
                title: "Calculated load",
                value: connection.engineLoadPercent.map {
                    String(format: "%.0f%%", $0)
                }
            ),
            Metric(
                pid: 0x10,
                title: "Mass air flow",
                value: connection.massAirFlowGramsPerSecond.map {
                    String(format: "%.1f g/s", $0)
                }
            ),
            Metric(
                pid: 0x05,
                title: "Coolant temperature",
                value: connection.coolantTemperatureCelsius.map {
                    String(format: "%.0f °C", $0)
                }
            ),
            Metric(
                pid: 0x0f,
                title: "Intake air temperature",
                value: connection.intakeAirTemperatureCelsius.map {
                    String(format: "%.0f °C", $0)
                }
            )
        ]
    }

    private var favouriteMetrics: [Metric] {
        metrics.filter { connection.favouritePIDs.contains($0.pid) }
    }

    var body: some View {
        NavigationStack {
            List {
                Section("Connection") {
                    LabeledContent("Status", value: connection.statusText)
                    LabeledContent("Adapter", value: connection.peripheralName)
                    LabeledContent("Identity", value: connection.adapterIdentifier)

                    if connection.isActive {
                        Button(connection.isReady ? "Disconnect" : "Cancel",
                               role: .destructive) {
                            connection.disconnect()
                        }
                    } else {
                        Button("Connect to OBD adapter") {
                            connection.connect()
                        }
                    }
                }

                if !favouriteMetrics.isEmpty {
                    Section("Favourites") {
                        ForEach(favouriteMetrics, id: \.pid) { metric in
                            measurementRow(metric)
                        }
                    }
                }

                Section("Live OBD-II dashboard") {
                    ForEach(metrics, id: \.pid) { metric in
                        measurementRow(metric)
                    }
                }

                if connection.recentRPM.count > 1 ||
                    connection.recentCoolant.count > 1 {
                    Section("Recent history") {
                        if connection.recentRPM.count > 1 {
                            VStack(alignment: .leading) {
                                Text("Engine speed")
                                    .font(.subheadline)
                                Chart(historyPoints(connection.recentRPM)) { point in
                                    LineMark(
                                        x: .value("Sample", point.id),
                                        y: .value("RPM", point.value)
                                    )
                                }
                                .frame(height: 140)
                                .chartYAxisLabel("rpm")
                            }
                        }

                        if connection.recentCoolant.count > 1 {
                            VStack(alignment: .leading) {
                                Text("Coolant temperature")
                                    .font(.subheadline)
                                Chart(historyPoints(connection.recentCoolant)) { point in
                                    LineMark(
                                        x: .value("Sample", point.id),
                                        y: .value("Temperature", point.value)
                                    )
                                }
                                .frame(height: 140)
                                .chartYAxisLabel("°C")
                            }
                        }
                    }
                }

                Section("Session recording") {
                    LabeledContent(
                        "Recorded samples",
                        value: "\(connection.recordedSampleCount)"
                    )

                    Button("Prepare CSV export") {
                        connection.prepareCSVExport()
                    }
                    .disabled(connection.recordedSampleCount == 0)

                    if let exportURL = connection.csvExportURL {
                        ShareLink(item: exportURL) {
                            Label("Share MBLINK-session.csv",
                                  systemImage: "square.and.arrow.up")
                        }
                    }
                }

                Section {
                    Text("Polling, sample history and CSV formatting are produced by the portable C core. SwiftUI only presents typed values and exports the generated session data.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            .navigationTitle("MBLINK")
        }
    }

    private func historyPoints(_ values: [Double]) -> [HistoryPoint] {
        values.enumerated().map { HistoryPoint(id: $0.offset, value: $0.element) }
    }

    @ViewBuilder
    private func measurementRow(_ metric: Metric) -> some View {
        HStack {
            Text(metric.title)
            Spacer()
            Text(metric.value ?? "—")
                .monospacedDigit()
                .foregroundStyle(metric.value == nil ? .secondary : .primary)
            Button {
                connection.toggleFavourite(pid: metric.pid)
            } label: {
                Image(
                    systemName: connection.favouritePIDs.contains(metric.pid)
                        ? "star.fill" : "star"
                )
                .accessibilityLabel(
                    connection.favouritePIDs.contains(metric.pid)
                        ? "Remove from favourites" : "Add to favourites"
                )
            }
            .buttonStyle(.borderless)
        }
    }
}
