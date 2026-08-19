// SPDX-License-Identifier: GPL-3.0-or-later
import Charts
import Foundation
import SwiftUI

private struct DiagnosticMetric: Identifiable {
    let id: UInt8
    let shortName: String
    let title: String
    let value: String?
}

private struct HistoryPoint: Identifiable {
    let id: Int
    let value: Double
}

private extension ConnectionViewModel {
    var diagnosticMetrics: [DiagnosticMetric] {
        [
            DiagnosticMetric(id: 0x0c, shortName: "RPM", title: "Engine speed",
                             value: rpm.map { String(format: "%.0f rpm", $0) }),
            DiagnosticMetric(id: 0x0d, shortName: "SPEED", title: "Vehicle speed",
                             value: vehicleSpeedKmh.map { String(format: "%.0f km/h", $0) }),
            DiagnosticMetric(id: 0x0b, shortName: "MAP", title: "Manifold pressure",
                             value: manifoldPressureKPa.map { String(format: "%.0f kPa", $0) }),
            DiagnosticMetric(id: 0x11, shortName: "THROTTLE", title: "Throttle position",
                             value: throttlePositionPercent.map { String(format: "%.0f%%", $0) }),
            DiagnosticMetric(id: 0x04, shortName: "LOAD", title: "Calculated engine load",
                             value: engineLoadPercent.map { String(format: "%.0f%%", $0) }),
            DiagnosticMetric(id: 0x10, shortName: "MAF", title: "Mass air flow",
                             value: massAirFlowGramsPerSecond.map { String(format: "%.1f g/s", $0) }),
            DiagnosticMetric(id: 0x05, shortName: "ECT", title: "Coolant temperature",
                             value: coolantTemperatureCelsius.map { String(format: "%.0f °C", $0) }),
            DiagnosticMetric(id: 0x0f, shortName: "IAT", title: "Intake air temperature",
                             value: intakeAirTemperatureCelsius.map { String(format: "%.0f °C", $0) })
        ]
    }
}

struct ContentView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        NavigationStack {
            List {
                Section {
                    connectionCard
                }

                Section("Diagnostics") {
                    workspaceLink("Vehicle", "Vehicle identity and connection information", "car.fill") {
                        VehicleView()
                    }
                    workspaceLink("Modules", "Control units and ECU identification", "square.stack.3d.up.fill") {
                        ModulesView()
                    }
                    workspaceLink("Faults", "Diagnostic trouble codes by module", "exclamationmark.triangle.fill") {
                        FaultsView()
                    }
                    workspaceLink("Live Data", "Select and favourite diagnostic parameters", "waveform.path.ecg") {
                        LiveDataView()
                    }
                    workspaceLink("Table", "Dense live parameter values", "tablecells") {
                        TableDataView()
                    }
                    workspaceLink("Dashboard", "At-a-glance live measurements", "gauge.with.dots.needle.67percent") {
                        DashboardView()
                    }
                    workspaceLink("Graphs", "Live parameter history", "chart.xyaxis.line") {
                        GraphsView()
                    }
                }

                Section("Session") {
                    workspaceLink("Log", "Recorded telemetry and CSV export", "doc.text.magnifyingglass") {
                        LogView()
                    }
                    workspaceLink("Settings", "Adapter and application information", "gearshape.fill") {
                        SettingsView()
                    }
                }
            }
            .navigationTitle("MBLINK")
            .navigationBarTitleDisplayMode(.large)
        }
    }

    private var connectionCard: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack(alignment: .center, spacing: 12) {
                Image(systemName: "car.side.fill")
                    .font(.system(size: 34, weight: .semibold))
                    .foregroundStyle(connection.isReady ? .green : .secondary)
                    .frame(width: 46)

                VStack(alignment: .leading, spacing: 2) {
                    Text("Mercedes-Benz Diagnostics")
                        .font(.headline)
                    Text(connection.statusText)
                        .font(.subheadline)
                        .foregroundStyle(connection.isReady ? .green : .secondary)
                }
                Spacer()
            }

            HStack {
                VStack(alignment: .leading, spacing: 2) {
                    Text(connection.peripheralName)
                        .font(.subheadline.weight(.medium))
                    Text(connection.adapterIdentifier)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .lineLimit(1)
                }
                Spacer()
                if connection.isActive {
                    Button(connection.isReady ? "Disconnect" : "Cancel", role: .destructive) {
                        connection.disconnect()
                    }
                    .buttonStyle(.bordered)
                } else {
                    Button("Connect") {
                        connection.connect()
                    }
                    .buttonStyle(.borderedProminent)
                }
            }
        }
        .padding(.vertical, 6)
    }

    private func workspaceLink<Destination: View>(
        _ title: String,
        _ subtitle: String,
        _ systemImage: String,
        @ViewBuilder destination: () -> Destination
    ) -> some View {
        NavigationLink(destination: destination()) {
            Label {
                VStack(alignment: .leading, spacing: 2) {
                    Text(title)
                        .font(.body.weight(.medium))
                    Text(subtitle)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            } icon: {
                Image(systemName: systemImage)
                    .frame(width: 28)
            }
            .padding(.vertical, 3)
        }
    }
}

private struct VehicleView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("Connection") {
                LabeledContent("Status", value: connection.statusText)
                LabeledContent("Adapter", value: connection.peripheralName)
                LabeledContent("Adapter identity", value: connection.adapterIdentifier)
            }
            Section("Vehicle") {
                LabeledContent("Target platform", value: "Mercedes-Benz C207")
                LabeledContent("Diagnostic layer", value: "Standard OBD-II")
                LabeledContent("Mercedes ECU discovery", value: "Not yet available")
            }
            Section {
                Text("UDS ECU identification and Mercedes-Benz module discovery will populate this screen as those portable C layers are implemented and verified against real vehicle responses.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Vehicle")
    }
}

private struct ModulesView: View {
    var body: some View {
        List {
            Section("Available now") {
                Label("Generic OBD-II engine diagnostics", systemImage: "engine.combustion.fill")
            }
            Section("Mercedes-Benz discovery") {
                ContentUnavailableView(
                    "ECU discovery not available yet",
                    systemImage: "square.stack.3d.up.slash",
                    description: Text("UDS and Mercedes-specific module discovery are the next diagnostic layers. No control units are invented or assumed before the vehicle reports them.")
                )
            }
        }
        .navigationTitle("Modules")
    }
}

private struct FaultsView: View {
    var body: some View {
        ContentUnavailableView(
            "No module fault scan yet",
            systemImage: "exclamationmark.triangle",
            description: Text("The portable core already decodes standard OBD-II DTC payloads. The app-wide module scan and Mercedes-specific fault catalogue will be connected after ECU discovery is implemented.")
        )
        .navigationTitle("Faults")
    }
}

private struct LiveDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List(connection.diagnosticMetrics) { metric in
            HStack(spacing: 12) {
                VStack(alignment: .leading, spacing: 2) {
                    Text(metric.shortName)
                        .font(.caption.monospaced().weight(.semibold))
                        .foregroundStyle(.secondary)
                    Text(metric.title)
                }
                Spacer()
                Text(metric.value ?? "—")
                    .monospacedDigit()
                Button {
                    connection.toggleFavourite(pid: metric.id)
                } label: {
                    Image(systemName: connection.favouritePIDs.contains(metric.id) ? "star.fill" : "star")
                }
                .buttonStyle(.borderless)
                .accessibilityLabel(connection.favouritePIDs.contains(metric.id) ? "Remove favourite" : "Add favourite")
            }
            .padding(.vertical, 3)
        }
        .navigationTitle("Live Data")
    }
}

private struct TableDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section {
                ForEach(connection.diagnosticMetrics) { metric in
                    LabeledContent {
                        Text(metric.value ?? "—")
                            .monospacedDigit()
                    } label: {
                        VStack(alignment: .leading, spacing: 1) {
                            Text(metric.title)
                            Text(metric.shortName)
                                .font(.caption.monospaced())
                                .foregroundStyle(.secondary)
                        }
                    }
                }
            } header: {
                Text("Current values")
            }
        }
        .navigationTitle("Table")
    }
}

private struct DashboardView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var displayedMetrics: [DiagnosticMetric] {
        let favourites = connection.diagnosticMetrics.filter { connection.favouritePIDs.contains($0.id) }
        return favourites.isEmpty ? connection.diagnosticMetrics : favourites
    }

    var body: some View {
        ScrollView {
            LazyVGrid(columns: [GridItem(.adaptive(minimum: 155), spacing: 12)], spacing: 12) {
                ForEach(displayedMetrics) { metric in
                    VStack(alignment: .leading, spacing: 8) {
                        Text(metric.shortName)
                            .font(.caption.monospaced().weight(.bold))
                            .foregroundStyle(.secondary)
                        Text(metric.value ?? "—")
                            .font(.title2.monospacedDigit().weight(.semibold))
                            .minimumScaleFactor(0.7)
                        Text(metric.title)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                    .frame(maxWidth: .infinity, minHeight: 105, alignment: .leading)
                    .padding()
                    .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 16, style: .continuous))
                }
            }
            .padding()
        }
        .navigationTitle("Dashboard")
    }
}

private struct GraphsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        ScrollView {
            VStack(spacing: 20) {
                graphCard(title: "Engine speed", unit: "rpm", values: connection.recentRPM)
                graphCard(title: "Coolant temperature", unit: "°C", values: connection.recentCoolant)
            }
            .padding()
        }
        .navigationTitle("Graphs")
    }

    private func graphCard(title: String, unit: String, values: [Double]) -> some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                Text(title).font(.headline)
                Spacer()
                Text(values.last.map { String(format: "%.0f %@", $0, unit) } ?? "—")
                    .monospacedDigit()
                    .foregroundStyle(.secondary)
            }
            if values.count > 1 {
                Chart(historyPoints(values)) { point in
                    LineMark(
                        x: .value("Sample", point.id),
                        y: .value(title, point.value)
                    )
                }
                .frame(height: 180)
                .chartYAxisLabel(unit)
            } else {
                ContentUnavailableView("Waiting for samples", systemImage: "chart.xyaxis.line")
                    .frame(height: 180)
            }
        }
        .padding()
        .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 16, style: .continuous))
    }

    private func historyPoints(_ values: [Double]) -> [HistoryPoint] {
        values.enumerated().map { HistoryPoint(id: $0.offset, value: $0.element) }
    }
}

private struct LogView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("Session") {
                LabeledContent("Recorded samples", value: "\(connection.recordedSampleCount)")
                LabeledContent("Status", value: connection.statusText)
            }
            Section("Export") {
                Button("Prepare CSV export") {
                    connection.prepareCSVExport()
                }
                .disabled(connection.recordedSampleCount == 0)

                if let exportURL = connection.csvExportURL {
                    ShareLink(item: exportURL) {
                        Label("Share CSV", systemImage: "square.and.arrow.up")
                    }
                }
            }
        }
        .navigationTitle("Log")
    }
}

private struct SettingsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("Adapter") {
                LabeledContent("Name", value: connection.peripheralName)
                LabeledContent("Identity", value: connection.adapterIdentifier)
            }
            Section("Build") {
                LabeledContent("Version", value: Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "Unknown")
                LabeledContent("Bundle ID", value: Bundle.main.bundleIdentifier ?? "Unknown")
            }
            Section("Architecture") {
                Text("SwiftUI is presentation only. Portable diagnostics, scheduling, telemetry and protocol behaviour remain owned by the shared C core used by every platform front end.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Settings")
    }
}
