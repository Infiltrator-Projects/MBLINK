// SPDX-License-Identifier: GPL-3.0-or-later
import Charts
import Foundation
import SwiftUI

private struct HistoryPoint: Identifiable {
    let id: Int
    let value: Double
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
                LabeledContent("Engine family", value: "OM651")
                LabeledContent("Generic diagnostics", value: "OBD-II")
                LabeledContent("Advanced diagnostics", value: "UDS foundation ready")
                LabeledContent("Mercedes ECU discovery", value: "Hardware validation pending")
            }
            Section {
                Text("The portable UDS and Mercedes definition layers are present, but MBLINK does not invent control units or manufacturer data. ECU discovery and Mercedes live values will appear here only after they are observed and verified against the vehicle.")
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
                Label("Portable UDS protocol engine", systemImage: "point.3.connected.trianglepath.dotted")
            }
            Section("Mercedes-Benz discovery") {
                ContentUnavailableView(
                    "ECU discovery awaiting vehicle validation",
                    systemImage: "square.stack.3d.up.slash",
                    description: Text("The Mercedes layer carries explicit candidate and vehicle-verified provenance. Control units will populate this workspace only after real C207 / OM651 responses are captured and validated.")
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
        List(connection.diagnosticParameters) { parameter in
            HStack(spacing: 12) {
                VStack(alignment: .leading, spacing: 2) {
                    HStack(spacing: 6) {
                        Text(parameter.shortName)
                            .font(.caption.monospaced().weight(.semibold))
                        Text(parameter.protocolName.uppercased())
                            .font(.caption2.monospaced())
                            .foregroundStyle(.secondary)
                    }
                    Text(parameter.title)
                }
                Spacer()
                Text(parameter.formattedValue)
                    .monospacedDigit()
                    .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
                Button {
                    connection.toggleFavourite(stableKey: parameter.id)
                } label: {
                    Image(systemName: parameter.favourite ? "star.fill" : "star")
                }
                .buttonStyle(.borderless)
                .accessibilityLabel(parameter.favourite ? "Remove favourite" : "Add favourite")
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
                ForEach(connection.diagnosticParameters) { parameter in
                    LabeledContent {
                        Text(parameter.formattedValue)
                            .monospacedDigit()
                            .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
                    } label: {
                        VStack(alignment: .leading, spacing: 1) {
                            Text(parameter.title)
                            Text("\(parameter.protocolName.uppercased()) · \(parameter.shortName)")
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

    private var displayedParameters: [DiagnosticParameter] {
        let favourites = connection.diagnosticParameters.filter(\.favourite)
        return favourites.isEmpty ? connection.diagnosticParameters : favourites
    }

    var body: some View {
        ScrollView {
            LazyVGrid(columns: [GridItem(.adaptive(minimum: 155), spacing: 12)], spacing: 12) {
                ForEach(displayedParameters) { parameter in
                    VStack(alignment: .leading, spacing: 8) {
                        HStack {
                            Text(parameter.shortName)
                                .font(.caption.monospaced().weight(.bold))
                            Spacer()
                            Text(parameter.protocolName.uppercased())
                                .font(.caption2.monospaced())
                                .foregroundStyle(.secondary)
                        }
                        Text(parameter.formattedValue)
                            .font(.title2.monospacedDigit().weight(.semibold))
                            .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
                            .minimumScaleFactor(0.7)
                        Text(parameter.title)
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

    private var graphedParameters: [DiagnosticParameter] {
        let withHistory = connection.diagnosticParameters.filter { !$0.history.isEmpty }
        let favourites = withHistory.filter(\.favourite)
        let selected = favourites.isEmpty ? withHistory : favourites
        return Array(selected.prefix(4))
    }

    var body: some View {
        ScrollView {
            if graphedParameters.isEmpty {
                ContentUnavailableView(
                    "Waiting for live samples",
                    systemImage: "chart.xyaxis.line",
                    description: Text("Connect to the vehicle and MBLINK will graph recent history for available parameters. Favourite parameters are prioritised automatically.")
                )
                .padding(.top, 60)
            } else {
                VStack(spacing: 20) {
                    ForEach(graphedParameters) { parameter in
                        graphCard(parameter)
                    }
                }
                .padding()
            }
        }
        .navigationTitle("Graphs")
    }

    private func graphCard(_ parameter: DiagnosticParameter) -> some View {
        let unit = parameter.suffix.trimmingCharacters(in: .whitespaces)

        return VStack(alignment: .leading, spacing: 10) {
            HStack {
                VStack(alignment: .leading, spacing: 2) {
                    Text(parameter.title).font(.headline)
                    Text("\(parameter.protocolName.uppercased()) · \(parameter.shortName)")
                        .font(.caption.monospaced())
                        .foregroundStyle(.secondary)
                }
                Spacer()
                Text(parameter.formattedValue)
                    .monospacedDigit()
                    .foregroundStyle(.secondary)
            }
            if parameter.history.count > 1 {
                Chart(historyPoints(parameter.history)) { point in
                    LineMark(
                        x: .value("Sample", point.id),
                        y: .value(parameter.title, point.value)
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
                Text("SwiftUI renders the shared C diagnostic parameter catalog. Portable diagnostics, parameter metadata/formatting, scheduling, telemetry and protocol behaviour remain owned by libmblink and Infiltratr Common rather than being duplicated in the iPhone UI.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Settings")
    }
}
