// SPDX-License-Identifier: GPL-3.0-or-later
import Charts
import Foundation
import SwiftUI

private struct HistoryPoint: Identifiable {
    let id: Int
    let value: Double
}

private enum DiagnosticParameterGroup: String, CaseIterable, Identifiable {
    case engine = "Engine"
    case air = "Air / turbo"
    case fuel = "Fuel / injection"
    case egr = "EGR"
    case aftertreatment = "DPF / exhaust"
    case electrical = "Electrical"

    var id: String { rawValue }
}

private enum LiveDataScope: String, CaseIterable, Identifiable {
    case available = "Available"
    case favourites = "Favourites"
    case all = "All"

    var id: String { rawValue }
}

private extension DiagnosticParameter {
    var group: DiagnosticParameterGroup {
        if id.contains(".dpf.") || id.contains(".aftertreatment.") {
            return .aftertreatment
        }
        if id.contains(".diesel.egr") {
            return .egr
        }
        if id.contains(".diesel.rail_pressure") || id.contains(".fuel_rate") {
            return .fuel
        }
        if id.contains(".electrical.") {
            return .electrical
        }
        if id.contains(".engine.map") ||
            id.contains(".barometric_pressure") ||
            id.contains(".engine.maf") ||
            id.contains(".intake_air") ||
            id.contains(".environment.") {
            return .air
        }
        return .engine
    }

    var pidText: String {
        let value = String(parameterIdentifier, radix: 16, uppercase: true)
        return "0x" + (value.count < 2 ? "0\(value)" : value)
    }
}

private struct MercedesUdsFault: Identifiable {
    let raw: String

    var id: String { raw }

    var code: String {
        raw.components(separatedBy: " · ").first ?? raw
    }

    var status: UInt8? {
        guard let marker = raw.range(of: "status 0x") else { return nil }
        let suffix = raw[marker.upperBound...]
        return UInt8(String(suffix.prefix(2)), radix: 16)
    }

    var statusHex: String {
        guard let status else { return "Unknown status" }
        return String(format: "Status 0x%02X", status)
    }

    var badges: [String] {
        guard let status else { return [] }
        var result = [String]()
        if status & 0x01 != 0 { result.append("Test failed") }
        if status & 0x02 != 0 { result.append("Failed this cycle") }
        if status & 0x04 != 0 { result.append("Pending") }
        if status & 0x08 != 0 { result.append("Confirmed") }
        if status & 0x10 != 0 { result.append("Not completed since clear") }
        if status & 0x20 != 0 { result.append("Failed since clear") }
        if status & 0x40 != 0 { result.append("Not completed this cycle") }
        if status & 0x80 != 0 { result.append("Warning requested") }
        return result
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
                    workspaceLink("Modules", "Mercedes ECU identification and technical evidence", "square.stack.3d.up.fill") {
                        ModulesView()
                    }
                    workspaceLink("Faults", "Mercedes UDS and standard OBD-II fault memory", "exclamationmark.triangle.fill") {
                        FaultsView()
                    }
                    workspaceLink("Diesel", "OM651 engine, DPF, rail pressure, EGR and temperatures", "engine.combustion.fill") {
                        DieselDiagnosticsView()
                    }
                    workspaceLink("Live Data", "Browse, filter and favourite diagnostic parameters", "waveform.path.ecg") {
                        LiveDataView()
                    }
                    workspaceLink("Table", "Dense PID, parameter and value view", "tablecells") {
                        TableDataView()
                    }
                    workspaceLink("Dashboard", "Focused at-a-glance vehicle measurements", "gauge.with.dots.needle.67percent") {
                        DashboardView()
                    }
                    workspaceLink("Graphs", "Choose up to four live signal histories", "chart.xyaxis.line") {
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

    private var totalFaultCount: Int {
        connection.mercedesUDSFaults.count +
        connection.storedDTCs.count +
        connection.pendingDTCs.count +
        connection.permanentDTCs.count
    }

    var body: some View {
        List {
            Section("Vehicle") {
                LabeledContent("Model", value: "C207 E 250 CDI")
                LabeledContent("VIN", value: connection.mercedesVINText)
                LabeledContent("Engine", value: "OM651")
                LabeledContent("Engine ECU", value: "Delphi CRD3.x")
                LabeledContent("Connection", value: connection.statusText)
                LabeledContent("Fault records", value: "\(totalFaultCount)")
            }

            Section("Adapter") {
                LabeledContent("Name", value: connection.peripheralName)
                LabeledContent("Identity", value: connection.adapterIdentifier)
            }

            Section("Mercedes diagnostics") {
                LabeledContent("CRD3 identity", value: connection.mercedesCrd3SummaryText)
                LabeledContent("UDS fault memory", value: connection.mercedesUDSFaultStatusText)
                DisclosureGroup("Diagnostic evidence") {
                    LabeledContent("Engine endpoint", value: connection.mercedesProbeEndpointText)
                    LabeledContent("Identity sweep", value: connection.mercedesIdentitySummaryText)
                    LabeledContent("Probe result", value: connection.mercedesProbeStatusText)
                    ForEach(connection.mercedesIdentityResults, id: \.self) { result in
                        Text(result)
                            .font(.caption.monospaced())
                    }
                }
            }

            Section {
                Text("The W207 E 250 CDI / OM651 / Delphi CRD3.x combination and its 0x7E0 → 0x7E8 engine diagnostic endpoint are source-corroborated. Vehicle capture remains the final verification step.")
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
            Section("Engine control unit") {
                Label("Delphi CRD3.x · OM651", systemImage: "engine.combustion.fill")
                LabeledContent("Endpoint", value: "0x7E0 → 0x7E8")
                LabeledContent("Evidence", value: "Source-corroborated")
                LabeledContent("Captured VIN", value: connection.mercedesVINText)
                LabeledContent("CRD3 identity", value: connection.mercedesCrd3SummaryText)
                LabeledContent("Mercedes faults", value: connection.mercedesUDSFaultStatusText)
            }

            Section("Capabilities") {
                Label("Standard OBD-II engine diagnostics", systemImage: "waveform.path.ecg")
                Label("UDS / ISO-TP diagnostic engine", systemImage: "point.3.connected.trianglepath.dotted")
                Label("CRD3 ECU identity and fingerprinting", systemImage: "checkmark.seal.fill")
                Label("Read-only UDS 0x19 fault memory", systemImage: "exclamationmark.triangle.fill")
            }

            Section("Technical evidence") {
                DisclosureGroup("Show identity and probe details") {
                    LabeledContent("Selected endpoint", value: connection.mercedesProbeEndpointText)
                    LabeledContent("Identity summary", value: connection.mercedesIdentitySummaryText)
                    LabeledContent("Probe result", value: connection.mercedesProbeStatusText)
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
                    ForEach(connection.mercedesUDSFaults.map(MercedesUdsFault.init(raw:))) { fault in
                        mercedesFaultRow(fault)
                    }
                }
                Text("Read-only ISO 14229 ReadDTCInformation 19 02 FF. Raw responses remain in the diagnostic evidence log.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            Section("Standard OBD-II") {
                LabeledContent("Status", value: connection.faultScanStatusText)
                Text("Emissions-related stored, pending and permanent faults are read separately with services 03, 07 and 0A.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
            dtcSection("Stored", codes: connection.storedDTCs)
            dtcSection("Pending", codes: connection.pendingDTCs)
            dtcSection("Permanent", codes: connection.permanentDTCs)
        }
        .navigationTitle("Faults")
    }

    private func mercedesFaultRow(_ fault: MercedesUdsFault) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Label(fault.code, systemImage: "exclamationmark.triangle.fill")
                    .font(.body.monospaced().weight(.semibold))
                Spacer()
                Text(fault.statusHex)
                    .font(.caption.monospaced())
                    .foregroundStyle(.secondary)
            }
            if !fault.badges.isEmpty {
                Text(fault.badges.joined(separator: " · "))
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
        .padding(.vertical, 2)
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

    private let groups: [DiagnosticParameterGroup] = [
        .engine, .aftertreatment, .air, .fuel, .egr, .electrical
    ]

    var body: some View {
        List {
            Section("OM651 / CRD3") {
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
                Text("Manufacturer-specific soot, ash, regeneration and injector values remain evidence-gated. Standard OBD-II values below populate when the vehicle advertises them.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }

            ForEach(groups) { group in
                Section(group.rawValue) {
                    let parameters = connection.diagnosticParameters.filter { $0.group == group }
                    if parameters.isEmpty {
                        Text("No parameters in this group")
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(parameters) { parameter in
                            parameterRow(parameter)
                        }
                    }
                }
            }
        }
        .navigationTitle("Diesel")
    }

    private func parameterRow(_ parameter: DiagnosticParameter) -> some View {
        LabeledContent {
            Text(parameter.formattedValue)
                .monospacedDigit()
                .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
        } label: {
            VStack(alignment: .leading, spacing: 1) {
                Text(parameter.title)
                Text("PID \(parameter.pidText)")
                    .font(.caption.monospaced())
                    .foregroundStyle(.secondary)
            }
        }
    }
}

private struct MercedesOm651TargetsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private let categories = ["dpf", "exhaust", "fuel", "air", "egr", "injector"]

    var body: some View {
        List {
            Section {
                Text("This is the manufacturer-level OM651 / CRD3 feature map. ‘Corroborated · mapping pending’ means the value is independently evidenced, but its raw request or scaling has not yet met MBLINK's provenance threshold.")
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
    @State private var scope: LiveDataScope = .available
    @State private var searchText = ""

    private var filteredParameters: [DiagnosticParameter] {
        connection.diagnosticParameters.filter { parameter in
            let scopeMatches: Bool
            switch scope {
            case .available:
                scopeMatches = parameter.isAvailable
            case .favourites:
                scopeMatches = parameter.favourite
            case .all:
                scopeMatches = true
            }
            guard scopeMatches else { return false }
            guard !searchText.isEmpty else { return true }
            return parameter.title.localizedCaseInsensitiveContains(searchText) ||
                parameter.shortName.localizedCaseInsensitiveContains(searchText) ||
                parameter.pidText.localizedCaseInsensitiveContains(searchText)
        }
    }

    var body: some View {
        List {
            Section {
                Picker("Show", selection: $scope) {
                    ForEach(LiveDataScope.allCases) { item in
                        Text(item.rawValue).tag(item)
                    }
                }
                .pickerStyle(.segmented)
            }

            ForEach(DiagnosticParameterGroup.allCases) { group in
                let parameters = filteredParameters.filter { $0.group == group }
                if !parameters.isEmpty {
                    Section(group.rawValue) {
                        ForEach(parameters) { parameter in
                            parameterRow(parameter)
                        }
                    }
                }
            }

            if filteredParameters.isEmpty {
                Section {
                    ContentUnavailableView(
                        scope == .available ? "No live values yet" : "No matching parameters",
                        systemImage: "waveform.path.ecg",
                        description: Text(scope == .available
                            ? "Connect to the vehicle to populate parameters the ECU actually reports."
                            : "Change the filter or search text to see more parameters.")
                    )
                }
            }
        }
        .navigationTitle("Live Data")
        .searchable(text: $searchText, prompt: "Parameter, PID or name")
    }

    private func parameterRow(_ parameter: DiagnosticParameter) -> some View {
        HStack(spacing: 12) {
            VStack(alignment: .leading, spacing: 2) {
                HStack(spacing: 6) {
                    Text(parameter.shortName)
                        .font(.caption.monospaced().weight(.semibold))
                    Text(parameter.pidText)
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
}

private struct TableDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var sortedParameters: [DiagnosticParameter] {
        connection.diagnosticParameters.sorted { left, right in
            if left.isAvailable != right.isAvailable {
                return left.isAvailable && !right.isAvailable
            }
            if left.group != right.group {
                return DiagnosticParameterGroup.allCases.firstIndex(of: left.group)! <
                    DiagnosticParameterGroup.allCases.firstIndex(of: right.group)!
            }
            return left.parameterIdentifier < right.parameterIdentifier
        }
    }

    var body: some View {
        List {
            Section {
                ForEach(sortedParameters) { parameter in
                    HStack(spacing: 10) {
                        Text(parameter.pidText)
                            .font(.caption.monospaced().weight(.semibold))
                            .frame(width: 48, alignment: .leading)
                        VStack(alignment: .leading, spacing: 1) {
                            Text(parameter.title)
                                .font(.subheadline)
                            Text(parameter.group.rawValue)
                                .font(.caption2)
                                .foregroundStyle(.secondary)
                        }
                        Spacer(minLength: 8)
                        Text(parameter.formattedValue)
                            .font(.subheadline.monospacedDigit())
                            .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
                            .multilineTextAlignment(.trailing)
                        Image(systemName: parameter.isAvailable ? "checkmark.circle.fill" : "minus.circle")
                            .font(.caption)
                            .foregroundStyle(parameter.isAvailable ? .primary : .secondary)
                    }
                    .padding(.vertical, 2)
                }
            } header: {
                Text("PID · Parameter · Value")
            }
        }
        .navigationTitle("Table")
    }
}

private struct DashboardView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private let defaultKeys = [
        "obd2.engine.rpm",
        "obd2.vehicle.speed",
        "obd2.engine.coolant",
        "obd2.diesel.rail_pressure",
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    ]

    private var displayedParameters: [DiagnosticParameter] {
        let available = connection.diagnosticParameters.filter(\.isAvailable)
        let favourites = available.filter(\.favourite)
        if !favourites.isEmpty {
            return Array(favourites.prefix(8))
        }
        let preferred = defaultKeys.compactMap { key in
            available.first { $0.id == key }
        }
        return preferred.isEmpty ? Array(available.prefix(6)) : preferred
    }

    var body: some View {
        ScrollView {
            if displayedParameters.isEmpty {
                ContentUnavailableView(
                    "Waiting for vehicle data",
                    systemImage: "gauge.with.dots.needle.67percent",
                    description: Text("The dashboard only shows values the connected vehicle actually reports. Favourite live-data parameters become the dashboard automatically.")
                )
                .padding(.top, 60)
            } else {
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 155), spacing: 12)], spacing: 12) {
                    ForEach(displayedParameters) { parameter in
                        VStack(alignment: .leading, spacing: 8) {
                            HStack {
                                Text(parameter.shortName)
                                    .font(.caption.monospaced().weight(.bold))
                                Spacer()
                                Text(parameter.pidText)
                                    .font(.caption2.monospaced())
                                    .foregroundStyle(.secondary)
                            }
                            Text(parameter.formattedValue)
                                .font(.title2.monospacedDigit().weight(.semibold))
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
        }
        .navigationTitle("Dashboard")
    }
}

private struct GraphsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var selectedKeys = Set<String>()
    @State private var showingSignalPicker = false

    private let defaultKeys = [
        "obd2.engine.rpm",
        "obd2.diesel.rail_pressure",
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    ]

    private var parametersWithHistory: [DiagnosticParameter] {
        connection.diagnosticParameters.filter { !$0.history.isEmpty }
    }

    private var graphedParameters: [DiagnosticParameter] {
        if !selectedKeys.isEmpty {
            return Array(parametersWithHistory.filter { selectedKeys.contains($0.id) }.prefix(4))
        }
        let favourites = parametersWithHistory.filter(\.favourite)
        if !favourites.isEmpty {
            return Array(favourites.prefix(4))
        }
        let preferred = defaultKeys.compactMap { key in
            parametersWithHistory.first { $0.id == key }
        }
        return preferred.isEmpty ? Array(parametersWithHistory.prefix(4)) : preferred
    }

    var body: some View {
        ScrollView {
            if graphedParameters.isEmpty {
                ContentUnavailableView(
                    "Waiting for live samples",
                    systemImage: "chart.xyaxis.line",
                    description: Text("Connect to the vehicle and choose up to four live signals to graph. Favourites are selected automatically when no custom selection is set.")
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
        .toolbar {
            ToolbarItem(placement: .topBarTrailing) {
                Button("Choose") {
                    showingSignalPicker = true
                }
            }
        }
        .sheet(isPresented: $showingSignalPicker) {
            GraphSignalPicker(
                parameters: connection.diagnosticParameters,
                selectedKeys: $selectedKeys
            )
        }
    }

    private func graphCard(_ parameter: DiagnosticParameter) -> some View {
        let unit = parameter.suffix.trimmingCharacters(in: .whitespaces)

        return VStack(alignment: .leading, spacing: 10) {
            HStack {
                VStack(alignment: .leading, spacing: 2) {
                    Text(parameter.title).font(.headline)
                    Text("\(parameter.pidText) · \(parameter.shortName)")
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

private struct GraphSignalPicker: View {
    @Environment(\.dismiss) private var dismiss
    let parameters: [DiagnosticParameter]
    @Binding var selectedKeys: Set<String>

    var body: some View {
        NavigationStack {
            List {
                Section {
                    Text("Choose up to four signals. Clear the selection to return to automatic favourites/default selection.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }

                ForEach(DiagnosticParameterGroup.allCases) { group in
                    let groupParameters = parameters.filter { $0.group == group }
                    if !groupParameters.isEmpty {
                        Section(group.rawValue) {
                            ForEach(groupParameters) { parameter in
                                let selected = selectedKeys.contains(parameter.id)
                                Button {
                                    if selected {
                                        selectedKeys.remove(parameter.id)
                                    } else if selectedKeys.count < 4 {
                                        selectedKeys.insert(parameter.id)
                                    }
                                } label: {
                                    HStack {
                                        VStack(alignment: .leading, spacing: 2) {
                                            Text(parameter.title)
                                                .foregroundStyle(.primary)
                                            Text("\(parameter.pidText) · \(parameter.formattedValue)")
                                                .font(.caption.monospaced())
                                                .foregroundStyle(.secondary)
                                        }
                                        Spacer()
                                        if selected {
                                            Image(systemName: "checkmark.circle.fill")
                                        }
                                    }
                                }
                                .disabled(!selected && selectedKeys.count >= 4)
                            }
                        }
                    }
                }
            }
            .navigationTitle("Graph Signals")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Clear") {
                        selectedKeys.removeAll()
                    }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") {
                        dismiss()
                    }
                }
            }
        }
    }
}

private struct LogView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        List {
            Section("Diagnostic evidence") {
                LabeledContent("Engine endpoint", value: connection.mercedesProbeEndpointText)
                LabeledContent("Mercedes probe", value: connection.mercedesProbeStatusText)
                LabeledContent("Captured VIN", value: connection.mercedesVINText)
                LabeledContent("Identity results", value: connection.mercedesIdentitySummaryText)
                LabeledContent("CRD3 identity", value: connection.mercedesCrd3SummaryText)
                LabeledContent("Mercedes UDS faults", value: connection.mercedesUDSFaultStatusText)
                LabeledContent("OBD-II fault scan", value: connection.faultScanStatusText)
                Text("The session recorder contains the raw ELM327 command/response transcript, including Mercedes UDS/CRD3 identity evidence, manufacturer fault-memory reads, standard OBD-II faults and live diesel/DPF requests.")
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
                Text("SwiftUI renders the shared C diagnostic parameter catalogue and the evidence-backed OM651 target catalogue. Portable diagnostics, parameter metadata, scheduling, telemetry and protocol behaviour remain owned by libmblink and Infiltratr Common rather than being duplicated in the iPhone UI.")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
        .navigationTitle("Settings")
    }
}
