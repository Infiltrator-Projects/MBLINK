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
                    workspaceLink("Faults", "Mercedes UDS and standard OBD-II fault memory", "exclamationmark.triangle.fill") {
                        FaultsView()
                    }
                    workspaceLink("Diesel", "OM651 targets, DPF, turbo, rail pressure and EGR", "engine.combustion.fill") {
                        DieselDiagnosticsView()
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
                    workspaceLink("Log", "Diagnostic evidence, telemetry and CSV export", "doc.text.magnifyingglass") {
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
                LabeledContent("Target platform", value: "Mercedes-Benz C207 E 250 CDI")
                LabeledContent("Engine family", value: "OM651")
                LabeledContent("ECU family", value: "Delphi CRD3.x")
                LabeledContent("Generic diagnostics", value: "OBD-II")
                LabeledContent("Advanced diagnostics", value: "UDS + CRD3 evidence")
                LabeledContent("Identity reads", value: "VIN + 6 standard + 5 CRD3")
                LabeledContent("Captured VIN", value: connection.mercedesVINText)
                LabeledContent("Engine candidate", value: connection.mercedesProbeEndpointText)
                LabeledContent("CRD3 identity", value: connection.mercedesCrd3SummaryText)
                LabeledContent("Mercedes probe", value: connection.mercedesProbeStatusText)
            }
            if !connection.mercedesIdentityResults.isEmpty {
                Section("ECU identity evidence") {
                    ForEach(connection.mercedesIdentityResults, id: \.self) { result in
                        Text(result)
                            .font(.subheadline.monospaced())
                    }
                }
            }
            Section {
                Text("The W207 E 250 CDI is independently catalogued with a Delphi CRD3.x engine ECU. MBLINK keeps the physical 7E0→7E8 endpoint candidate until it is vehicle-verified, but development of the CRD3 protocol, UDS faults and OM651 data model continues offline.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Vehicle")
    }
}

private struct ModulesView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("Available now") {
                Label("Generic OBD-II engine diagnostics", systemImage: "engine.combustion.fill")
                Label("Portable UDS protocol engine", systemImage: "point.3.connected.trianglepath.dotted")
                Label("Standard UDS + CRD3 read-only identity evidence", systemImage: "magnifyingglass.circle.fill")
                Label("Read-only UDS 0x19 Mercedes fault scan", systemImage: "exclamationmark.triangle.fill")
            }
            Section("Mercedes-Benz engine") {
                LabeledContent("Engine candidate", value: connection.mercedesProbeEndpointText)
                LabeledContent("Captured VIN", value: connection.mercedesVINText)
                LabeledContent("Identity summary", value: connection.mercedesIdentitySummaryText)
                LabeledContent("CRD3 summary", value: connection.mercedesCrd3SummaryText)
                LabeledContent("UDS faults", value: connection.mercedesUDSFaultStatusText)
                LabeledContent("Probe result", value: connection.mercedesProbeStatusText)
                if !connection.mercedesIdentityResults.isEmpty {
                    ForEach(connection.mercedesIdentityResults, id: \.self) { result in
                        Text(result)
                            .font(.caption.monospaced())
                    }
                }
                NavigationLink {
                    LogView()
                } label: {
                    Label("Export diagnostic evidence", systemImage: "square.and.arrow.up")
                }
            }
        }
        .navigationTitle("Modules")
    }
}

private struct FaultsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("Mercedes engine · UDS 0x19") {
                LabeledContent("Status", value: connection.mercedesUDSFaultStatusText)
                if connection.mercedesUDSFaults.isEmpty {
                    Text("No Mercedes UDS fault records captured")
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(connection.mercedesUDSFaults, id: \.self) { code in
                        Label(code, systemImage: "exclamationmark.triangle.fill")
                            .font(.body.monospaced().weight(.semibold))
                    }
                }
                Text("MBLINK reads the CRD3 engine fault memory with ISO 14229 ReadDTCInformation 19 02 FF while the Mercedes ECU channel is selected. This is a read-only request; the raw response is retained in the evidence transcript.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            Section("Standard OBD-II") {
                LabeledContent("Status", value: connection.faultScanStatusText)
                Text("Stored, pending and permanent emissions-related faults are also read using OBD-II services 03, 07 and 0A after the adapter is restored to its normal OBD channel.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
            dtcSection("Stored", codes: connection.storedDTCs)
            dtcSection("Pending", codes: connection.pendingDTCs)
            dtcSection("Permanent", codes: connection.permanentDTCs)
        }
        .navigationTitle("Faults")
    }

    @ViewBuilder
    private func dtcSection(_ title: String, codes: [String]) -> some View {
        Section(title) {
            if codes.isEmpty {
                Text("None reported")
                    .foregroundStyle(.secondary)
            } else {
                ForEach(codes, id: \.self) { code in
                    Label(code, systemImage: "exclamationmark.triangle")
                        .font(.body.monospaced().weight(.semibold))
                }
            }
        }
    }
}

private struct DieselDiagnosticsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("OM651 / CDID3") {
                NavigationLink {
                    MercedesOm651TargetsView()
                } label: {
                    VStack(alignment: .leading, spacing: 2) {
                        Text("Mercedes manufacturer targets")
                            .font(.body.weight(.medium))
                        Text("\(connection.mercedesTargetSignals.count) corroborated values · protocol mapping in progress")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                }
                Text("These targets come from OM651/CDID3 diagnostic evidence. They are visible before an adapter is available, but MBLINK will not attach a guessed DID, scaling rule or live value to them.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            Section("DPF") {
                parameterRow("obd2.dpf.bank1_delta_pressure", fallback: "DPF differential pressure")
                parameterRow("obd2.dpf.bank1_inlet_temperature", fallback: "DPF inlet temperature")
                parameterRow("obd2.aftertreatment.egt_b1s1", fallback: "Exhaust gas temperature")
                parameterRow("obd2.aftertreatment.catalyst_temp_b1s1", fallback: "Catalyst temperature")
                Text("These are standard SAE OBD-II aftertreatment values and only populate when the vehicle advertises the corresponding PID and sub-field. Mercedes soot load, regeneration state and ash-load DIDs remain evidence-gated rather than guessed.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            Section("Turbo / air") {
                parameterRow("obd2.engine.map", fallback: "Manifold absolute pressure")
                parameterRow("obd2.engine.barometric_pressure", fallback: "Barometric pressure")
                parameterRow("obd2.engine.maf", fallback: "Mass air flow")
                parameterRow("obd2.engine.intake_air", fallback: "Intake air temperature")
            }

            Section("Fuel / injection") {
                parameterRow("obd2.diesel.rail_pressure", fallback: "Fuel rail pressure")
                parameterRow("obd2.engine.fuel_rate", fallback: "Fuel rate")
                LabeledContent("Injector corrections", value: "OM651 mapping in progress")
            }

            Section("EGR") {
                parameterRow("obd2.diesel.egr_command", fallback: "Commanded EGR")
                parameterRow("obd2.diesel.egr_error", fallback: "EGR error")
            }

            Section("Temperatures / electrical") {
                parameterRow("obd2.engine.coolant", fallback: "Coolant temperature")
                parameterRow("obd2.engine.oil_temperature", fallback: "Oil temperature")
                parameterRow("obd2.environment.ambient_air", fallback: "Ambient temperature")
                parameterRow("obd2.electrical.control_module_voltage", fallback: "Control module voltage")
            }
        }
        .navigationTitle("Diesel")
    }

    private func pidHex(_ identifier: UInt32) -> String {
        let text = String(identifier, radix: 16, uppercase: true)
        return text.count < 2 ? "0\(text)" : text
    }

    @ViewBuilder
    private func parameterRow(_ stableKey: String, fallback: String) -> some View {
        if let parameter = connection.parameter(stableKey: stableKey) {
            LabeledContent {
                Text(parameter.formattedValue)
                    .monospacedDigit()
                    .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
            } label: {
                VStack(alignment: .leading, spacing: 1) {
                    Text(parameter.title)
                    Text("PID 0x\(pidHex(parameter.parameterIdentifier))")
                        .font(.caption.monospaced())
                        .foregroundStyle(.secondary)
                }
            }
        } else {
            LabeledContent(fallback, value: "Unavailable")
        }
    }
}

private struct MercedesOm651TargetsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private let categories = ["dpf", "exhaust", "fuel", "air", "egr", "injector"]

    var body: some View {
        List {
            Section {
                Text("This is the manufacturer-level feature map MBLINK is implementing for OM651/CDID3. ‘Corroborated · mapping pending’ means the diagnostic value is independently evidenced, but its raw request/encoding has not yet met MBLINK's provenance threshold.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            ForEach(categories, id: \.self) { category in
                Section(category.uppercased()) {
                    ForEach(connection.mercedesSignals(category: category)) { signal in
                        VStack(alignment: .leading, spacing: 3) {
                            Text(signal.title)
                            Text(signal.status == "corroborated-unmapped"
                                 ? "Corroborated · mapping pending"
                                 : signal.status)
                                .font(.caption.monospaced())
                                .foregroundStyle(.secondary)
                        }
                        .padding(.vertical, 2)
                    }
                }
            }
        }
        .navigationTitle("OM651 Targets")
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
            Section("Diagnostic evidence") {
                LabeledContent("Engine candidate", value: connection.mercedesProbeEndpointText)
                LabeledContent("Mercedes probe", value: connection.mercedesProbeStatusText)
                LabeledContent("Captured VIN", value: connection.mercedesVINText)
                LabeledContent("Identity results", value: connection.mercedesIdentitySummaryText)
                LabeledContent("CRD3 identity", value: connection.mercedesCrd3SummaryText)
                LabeledContent("Mercedes UDS faults", value: connection.mercedesUDSFaultStatusText)
                LabeledContent("OBD-II fault scan", value: connection.faultScanStatusText)
                Text("The session recorder contains the raw ELM327 command/response transcript, including the Mercedes UDS/CRD3 identity evidence, the 19 02 FF manufacturer fault-memory request, all three OBD-II fault services and every live diesel/DPF request.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            Section("Session") {
                LabeledContent("Recorded samples", value: "\(connection.recordedSampleCount)")
                LabeledContent("Status", value: connection.statusText)
            }

            Section("Export") {
                Button("Prepare diagnostic evidence CSV") {
                    connection.prepareCSVExport()
                }

                if let exportURL = connection.csvExportURL {
                    ShareLink(item: exportURL) {
                        Label("Share diagnostic evidence", systemImage: "square.and.arrow.up")
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
                Text("SwiftUI renders the shared C diagnostic parameter catalog and the evidence-backed OM651 target catalogue. Portable diagnostics, parameter metadata/formatting, scheduling, telemetry and protocol behaviour remain owned by libmblink and Infiltratr Common rather than being duplicated in the iPhone UI.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Settings")
    }
}
