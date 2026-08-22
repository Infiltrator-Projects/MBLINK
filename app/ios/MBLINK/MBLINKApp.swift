// SPDX-License-Identifier: GPL-3.0-or-later
import Charts
import Foundation
import SwiftUI

private enum MBBrand {
    static let background = Color(red: 0.018, green: 0.020, blue: 0.024)
    static let chrome = Color(red: 0.040, green: 0.044, blue: 0.050)
    static let panel = Color(red: 0.070, green: 0.076, blue: 0.084)
    static let panelRaised = Color(red: 0.095, green: 0.102, blue: 0.112)
    static let silver = Color(red: 0.72, green: 0.75, blue: 0.78)
    static let silverBright = Color(red: 0.91, green: 0.93, blue: 0.95)
    static let muted = Color(red: 0.48, green: 0.51, blue: 0.55)
    static let line = Color(red: 0.24, green: 0.26, blue: 0.29)
    static let success = Color(red: 0.31, green: 0.67, blue: 0.48)
    static let warning = Color(red: 0.82, green: 0.62, blue: 0.28)
    static let fault = Color(red: 0.78, green: 0.28, blue: 0.28)
}

private struct MBBackground: View {
    var body: some View {
        LinearGradient(
            stops: [
                .init(color: Color.black, location: 0.0),
                .init(color: MBBrand.background, location: 0.55),
                .init(color: MBBrand.chrome, location: 1.0)
            ],
            startPoint: .top,
            endPoint: .bottomTrailing
        )
        .ignoresSafeArea()
    }
}

private struct MBLogoMark: View {
    var size: CGFloat = 54

    var body: some View {
        Image("MBLINKEmblem")
            .resizable()
            .scaledToFit()
            .frame(width: size, height: size)
            .shadow(color: .black.opacity(0.7), radius: 10, y: 6)
            .accessibilityHidden(true)
    }
}

private struct MBPanel<Content: View>: View {
    let content: Content

    init(@ViewBuilder content: () -> Content) {
        self.content = content()
    }

    var body: some View {
        content
            .padding(16)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(
                RoundedRectangle(cornerRadius: 18, style: .continuous)
                    .fill(
                        LinearGradient(
                            colors: [MBBrand.panelRaised, MBBrand.panel],
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 18, style: .continuous)
                            .stroke(MBBrand.line.opacity(0.85), lineWidth: 1)
                    )
            )
            .shadow(color: .black.opacity(0.38), radius: 12, y: 6)
    }
}

private struct MBSectionHeader: View {
    let title: String
    var kicker: String? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 3) {
            if let kicker {
                Text(kicker.uppercased())
                    .font(.caption2.weight(.bold))
                    .tracking(1.7)
                    .foregroundStyle(MBBrand.muted)
            }
            Text(title)
                .font(.title3.weight(.semibold))
                .foregroundStyle(MBBrand.silverBright)
        }
    }
}

private struct MBStatusPill: View {
    let text: String
    let active: Bool

    var body: some View {
        HStack(spacing: 7) {
            Circle()
                .fill(active ? MBBrand.success : MBBrand.muted)
                .frame(width: 7, height: 7)
            Text(text.uppercased())
                .font(.caption2.weight(.bold))
                .tracking(1.0)
        }
        .foregroundStyle(active ? MBBrand.silverBright : MBBrand.silver)
        .padding(.horizontal, 10)
        .padding(.vertical, 7)
        .background(Capsule().fill(MBBrand.panelRaised))
        .overlay(Capsule().stroke(MBBrand.line, lineWidth: 1))
    }
}

private struct MBInfoRow: View {
    let label: String
    let value: String
    var monospaced = false

    var body: some View {
        HStack(alignment: .firstTextBaseline, spacing: 14) {
            Text(label)
                .font(.subheadline)
                .foregroundStyle(MBBrand.muted)
            Spacer(minLength: 16)
            Text(value)
                .font(monospaced ? .subheadline.monospaced() : .subheadline.weight(.medium))
                .foregroundStyle(MBBrand.silverBright)
                .multilineTextAlignment(.trailing)
        }
        .padding(.vertical, 4)
    }
}

private struct MBMetricTile: View {
    let parameter: DiagnosticParameter

    var body: some View {
        VStack(alignment: .leading, spacing: 9) {
            HStack {
                Text(parameter.shortName.uppercased())
                    .font(.caption2.monospaced().weight(.bold))
                    .tracking(0.7)
                    .foregroundStyle(MBBrand.silver)
                Spacer()
                Text(parameter.brandPidText)
                    .font(.caption2.monospaced())
                    .foregroundStyle(MBBrand.muted)
            }

            Text(parameter.formattedValue)
                .font(.system(size: 24, weight: .semibold, design: .rounded))
                .monospacedDigit()
                .foregroundStyle(parameter.isAvailable ? MBBrand.silverBright : MBBrand.muted)
                .minimumScaleFactor(0.65)
                .lineLimit(1)

            Text(parameter.title)
                .font(.caption)
                .foregroundStyle(MBBrand.muted)
                .lineLimit(2)
        }
        .frame(maxWidth: .infinity, minHeight: 112, alignment: .leading)
        .padding(14)
        .background(
            RoundedRectangle(cornerRadius: 15, style: .continuous)
                .fill(MBBrand.panel)
                .overlay(
                    RoundedRectangle(cornerRadius: 15, style: .continuous)
                        .stroke(parameter.isAvailable ? MBBrand.silver.opacity(0.38) : MBBrand.line, lineWidth: 1)
                )
        )
    }
}

private struct MBWorkspaceTile<Destination: View>: View {
    let title: String
    let subtitle: String
    let symbol: String
    let destination: () -> Destination

    init(
        _ title: String,
        _ subtitle: String,
        _ symbol: String,
        @ViewBuilder destination: @escaping () -> Destination
    ) {
        self.title = title
        self.subtitle = subtitle
        self.symbol = symbol
        self.destination = destination
    }

    var body: some View {
        NavigationLink {
            destination()
        } label: {
            VStack(alignment: .leading, spacing: 12) {
                ZStack {
                    Circle()
                        .fill(MBBrand.panelRaised)
                    Circle()
                        .stroke(MBBrand.line, lineWidth: 1)
                    Image(systemName: symbol)
                        .font(.system(size: 19, weight: .semibold))
                        .foregroundStyle(MBBrand.silverBright)
                }
                .frame(width: 42, height: 42)

                VStack(alignment: .leading, spacing: 4) {
                    Text(title)
                        .font(.headline)
                        .foregroundStyle(MBBrand.silverBright)
                    Text(subtitle)
                        .font(.caption)
                        .foregroundStyle(MBBrand.muted)
                        .lineLimit(2)
                }

                Spacer(minLength: 0)

                HStack {
                    Text("OPEN")
                        .font(.caption2.weight(.bold))
                        .tracking(1.2)
                    Spacer()
                    Image(systemName: "chevron.right")
                }
                .foregroundStyle(MBBrand.silver)
            }
            .frame(maxWidth: .infinity, minHeight: 156, alignment: .leading)
            .padding(15)
            .background(
                RoundedRectangle(cornerRadius: 17, style: .continuous)
                    .fill(
                        LinearGradient(
                            colors: [MBBrand.panelRaised, MBBrand.panel],
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 17, style: .continuous)
                            .stroke(MBBrand.line, lineWidth: 1)
                    )
            )
        }
        .buttonStyle(.plain)
    }
}

private enum MBParameterGroup: String, CaseIterable, Identifiable {
    case engine = "Engine"
    case air = "Air / turbo"
    case fuel = "Fuel / injection"
    case egr = "EGR"
    case aftertreatment = "DPF / exhaust"
    case electrical = "Electrical"

    var id: String { rawValue }

    var symbol: String {
        switch self {
        case .engine: return "engine.combustion.fill"
        case .air: return "wind"
        case .fuel: return "fuelpump.fill"
        case .egr: return "arrow.triangle.2.circlepath"
        case .aftertreatment: return "aqi.medium"
        case .electrical: return "bolt.fill"
        }
    }
}

private enum MBLiveScope: String, CaseIterable, Identifiable {
    case available = "Available"
    case favourites = "Favourites"
    case all = "All"

    var id: String { rawValue }
}

private extension DiagnosticParameter {
    var brandGroup: MBParameterGroup {
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

    var brandPidText: String {
        let value = String(parameterIdentifier, radix: 16, uppercase: true)
        return "0x" + (value.count < 2 ? "0\(value)" : value)
    }
}

private struct MBFault: Identifiable {
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

@main
struct MBLINKApp: App {
    @StateObject private var connection = ConnectionViewModel()
    @State private var showingAbout = false

    var body: some Scene {
        WindowGroup {
            MBCommandCentreView()
                .environmentObject(connection)
                .preferredColorScheme(.dark)
                .tint(MBBrand.silverBright)
                .safeAreaInset(edge: .bottom, spacing: 0) {
                    Button {
                        showingAbout = true
                    } label: {
                        HStack(spacing: 8) {
                            Text("MBLINK")
                                .fontWeight(.bold)
                                .tracking(1.0)
                            Text("© 2026 Shannon Smith")
                                .foregroundStyle(MBBrand.muted)
                            Spacer()
                            Label("About", systemImage: "info.circle")
                        }
                        .font(.caption)
                        .foregroundStyle(MBBrand.silver)
                        .padding(.horizontal, 16)
                        .padding(.vertical, 9)
                        .frame(maxWidth: .infinity)
                        .background(MBBrand.chrome)
                        .overlay(alignment: .top) {
                            Rectangle()
                                .fill(MBBrand.line)
                                .frame(height: 1)
                        }
                    }
                    .buttonStyle(.plain)
                }
                .sheet(isPresented: $showingAbout) {
                    MBLINKAboutView {
                        showingAbout = false
                    }
                    .preferredColorScheme(.dark)
                    .tint(MBBrand.silverBright)
                }
        }
    }
}

private struct MBCommandCentreView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private let metricKeys = [
        "obd2.engine.rpm",
        "obd2.vehicle.speed",
        "obd2.engine.coolant",
        "obd2.diesel.rail_pressure",
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    ]

    private var metrics: [DiagnosticParameter] {
        metricKeys.compactMap(connection.parameter(stableKey:))
    }

    private var totalFaultCount: Int {
        connection.mercedesUDSFaults.count +
        connection.storedDTCs.count +
        connection.pendingDTCs.count +
        connection.permanentDTCs.count
    }

    var body: some View {
        NavigationStack {
            ZStack {
                MBBackground()

                ScrollView {
                    VStack(alignment: .leading, spacing: 20) {
                        hero
                        connectionPanel
                        commandMetrics
                        evidencePanel
                        workspaceGrid
                    }
                    .padding(.horizontal, 16)
                    .padding(.top, 18)
                    .padding(.bottom, 30)
                }
            }
            .toolbar(.hidden, for: .navigationBar)
        }
    }

    private var hero: some View {
        ViewThatFits(in: .horizontal) {
            HStack(alignment: .center, spacing: 15) {
                brandIdentity
                Spacer(minLength: 8)
                MBStatusPill(text: connection.statusText, active: connection.isReady)
            }

            VStack(alignment: .leading, spacing: 12) {
                brandIdentity
                MBStatusPill(text: connection.statusText, active: connection.isReady)
            }
        }
    }

    private var brandIdentity: some View {
        HStack(alignment: .center, spacing: 15) {
            MBLogoMark(size: 62)

            VStack(alignment: .leading, spacing: 3) {
                Text("MBLINK")
                    .font(.system(size: 31, weight: .black, design: .rounded))
                    .tracking(1.6)
                    .foregroundStyle(MBBrand.silverBright)
                Text("MERCEDES DIAGNOSTIC COMMAND")
                    .font(.caption2.weight(.bold))
                    .tracking(1.8)
                    .foregroundStyle(MBBrand.silver)
                Text("C207 · OM651 · DELPHI CRD3.x")
                    .font(.caption.monospaced())
                    .foregroundStyle(MBBrand.muted)
            }
        }
    }

    private var connectionPanel: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 15) {
                HStack {
                    MBSectionHeader(title: "Vehicle link", kicker: "Connection")
                    Spacer()
                    Image(systemName: connection.isReady ? "antenna.radiowaves.left.and.right" : "antenna.radiowaves.left.and.right.slash")
                        .font(.title2)
                        .foregroundStyle(connection.isReady ? MBBrand.success : MBBrand.muted)
                }

                HStack(alignment: .center, spacing: 14) {
                    VStack(alignment: .leading, spacing: 5) {
                        Text(connection.peripheralName)
                            .font(.headline)
                            .foregroundStyle(MBBrand.silverBright)
                        Text(connection.adapterIdentifier)
                            .font(.caption.monospaced())
                            .foregroundStyle(MBBrand.muted)
                            .lineLimit(1)
                    }

                    Spacer()

                    if connection.isActive {
                        Button(connection.isReady ? "DISCONNECT" : "CANCEL") {
                            connection.disconnect()
                        }
                        .font(.caption.weight(.bold))
                        .tracking(0.9)
                        .foregroundStyle(MBBrand.silverBright)
                        .padding(.horizontal, 14)
                        .padding(.vertical, 10)
                        .background(Capsule().fill(MBBrand.panelRaised))
                        .overlay(Capsule().stroke(MBBrand.line, lineWidth: 1))
                    } else {
                        Button("CONNECT") {
                            connection.connect()
                        }
                        .font(.caption.weight(.black))
                        .tracking(1.0)
                        .foregroundStyle(Color.black)
                        .padding(.horizontal, 17)
                        .padding(.vertical, 10)
                        .background(Capsule().fill(MBBrand.silverBright))
                    }
                }
            }
        }
    }

    private var commandMetrics: some View {
        VStack(alignment: .leading, spacing: 12) {
            MBSectionHeader(title: "Powertrain telemetry", kicker: "Live instruments")

            LazyVGrid(
                columns: [GridItem(.adaptive(minimum: 155), spacing: 11)],
                spacing: 11
            ) {
                ForEach(metrics) { parameter in
                    MBMetricTile(parameter: parameter)
                }
            }
        }
    }

    private var evidencePanel: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 13) {
                MBSectionHeader(title: "Vehicle intelligence", kicker: "Mercedes evidence")

                MBInfoRow(label: "Vehicle", value: "C207 E 250 CDI · OM651")
                Divider().overlay(MBBrand.line)
                MBInfoRow(label: "Engine ECU", value: "Delphi CRD3.x")
                MBInfoRow(label: "Endpoint", value: "0x7E0 → 0x7E8", monospaced: true)
                MBInfoRow(label: "CRD3 identity", value: connection.mercedesCrd3SummaryText)
                MBInfoRow(label: "VIN", value: connection.mercedesVINText, monospaced: true)
                MBInfoRow(label: "Fault records", value: "\(totalFaultCount)")
            }
        }
    }

    private var workspaceGrid: some View {
        VStack(alignment: .leading, spacing: 12) {
            MBSectionHeader(title: "Diagnostic workspaces", kicker: "Command centre")

            LazyVGrid(
                columns: [GridItem(.flexible(), spacing: 11), GridItem(.flexible(), spacing: 11)],
                spacing: 11
            ) {
                MBWorkspaceTile("Vehicle", "Identity, ECU and connection evidence", "car.side.fill") {
                    MBVehicleView()
                }
                MBWorkspaceTile("Modules", "CRD3 ECU identity and capabilities", "square.stack.3d.up.fill") {
                    MBModulesView()
                }
                MBWorkspaceTile("Faults", "Mercedes UDS and OBD fault memory", "exclamationmark.triangle.fill") {
                    MBFaultsView()
                }
                MBWorkspaceTile("Diesel", "DPF, rail, EGR and exhaust data", "engine.combustion.fill") {
                    MBDieselView()
                }
                MBWorkspaceTile("Live Data", "Filter and favourite all live values", "waveform.path.ecg") {
                    MBLiveDataView()
                }
                MBWorkspaceTile("Data Table", "Dense PID and parameter view", "tablecells") {
                    MBTableView()
                }
                MBWorkspaceTile("Dashboard", "Focused at-a-glance vehicle measurements", "gauge.with.dots.needle.67percent") {
                    MBDashboardView()
                }
                MBWorkspaceTile("Graphs", "Instrument history and trends", "chart.xyaxis.line") {
                    MBGraphsView()
                }
                MBWorkspaceTile("Evidence", "Raw session evidence and CSV export", "doc.text.magnifyingglass") {
                    MBLogView()
                }
                MBWorkspaceTile("Settings", "Display, adapter, build and architecture", "gearshape.fill") {
                    MBSettingsView()
                }
            }
        }
    }
}

private extension View {
    func mbDiagnosticScreen(_ title: String) -> some View {
        self
            .background(MBBrand.background.ignoresSafeArea())
            .navigationTitle(title)
            .navigationBarTitleDisplayMode(.inline)
            .toolbarBackground(MBBrand.chrome, for: .navigationBar)
            .toolbarBackground(.visible, for: .navigationBar)
            .toolbarColorScheme(.dark, for: .navigationBar)
    }
}

private struct MBVehicleView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var totalFaultCount: Int {
        connection.mercedesUDSFaults.count +
        connection.storedDTCs.count +
        connection.pendingDTCs.count +
        connection.permanentDTCs.count
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "C207 E 250 CDI", kicker: "Vehicle")

                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "VIN", value: connection.mercedesVINText, monospaced: true)
                            MBInfoRow(label: "Engine", value: "OM651")
                            MBInfoRow(label: "Engine ECU", value: "Delphi CRD3.x")
                            MBInfoRow(label: "Connection", value: connection.statusText)
                            MBInfoRow(label: "Fault records", value: "\(totalFaultCount)")
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 8) {
                            MBSectionHeader(title: "Mercedes evidence", kicker: "Read-only probe")
                            MBInfoRow(label: "Endpoint", value: connection.mercedesProbeEndpointText)
                            MBInfoRow(label: "CRD3 identity", value: connection.mercedesCrd3SummaryText)
                            MBInfoRow(label: "Identity sweep", value: connection.mercedesIdentitySummaryText)
                            MBInfoRow(label: "Probe result", value: connection.mercedesProbeStatusText)
                            MBInfoRow(label: "UDS faults", value: connection.mercedesUDSFaultStatusText)

                            if !connection.mercedesIdentityResults.isEmpty {
                                Divider().overlay(MBBrand.line)
                                ForEach(connection.mercedesIdentityResults, id: \.self) { result in
                                    Text(result)
                                        .font(.caption.monospaced())
                                        .foregroundStyle(MBBrand.silver)
                                        .frame(maxWidth: .infinity, alignment: .leading)
                                }
                            }
                        }
                    }

                    Text("The W207 E 250 CDI / OM651 / Delphi CRD3.x combination and 0x7E0 → 0x7E8 engine endpoint are source-corroborated. Vehicle capture remains the final verification step.")
                        .font(.footnote)
                        .foregroundStyle(MBBrand.muted)
                        .padding(.horizontal, 3)
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Vehicle")
    }
}

private struct MBModulesView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Delphi CRD3.x", kicker: "Engine control unit")

                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "Engine", value: "OM651")
                            MBInfoRow(label: "Endpoint", value: "0x7E0 → 0x7E8", monospaced: true)
                            MBInfoRow(label: "Evidence", value: "Source-corroborated")
                            MBInfoRow(label: "VIN", value: connection.mercedesVINText, monospaced: true)
                            MBInfoRow(label: "CRD3 identity", value: connection.mercedesCrd3SummaryText)
                            MBInfoRow(label: "Mercedes faults", value: connection.mercedesUDSFaultStatusText)
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            MBSectionHeader(title: "Capabilities", kicker: "Diagnostic stack")
                            capability("Standard OBD-II engine diagnostics", "waveform.path.ecg")
                            capability("UDS / ISO-TP diagnostic engine", "point.3.connected.trianglepath.dotted")
                            capability("CRD3 ECU identity and fingerprinting", "checkmark.seal.fill")
                            capability("Read-only UDS 0x19 fault memory", "exclamationmark.triangle.fill")
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 10) {
                            MBSectionHeader(title: "Probe evidence", kicker: "Technical")
                            MBInfoRow(label: "Selected endpoint", value: connection.mercedesProbeEndpointText)
                            MBInfoRow(label: "Identity summary", value: connection.mercedesIdentitySummaryText)
                            MBInfoRow(label: "Probe result", value: connection.mercedesProbeStatusText)
                            ForEach(connection.mercedesIdentityResults, id: \.self) { result in
                                Text(result)
                                    .font(.caption.monospaced())
                                    .foregroundStyle(MBBrand.silver)
                            }

                            NavigationLink {
                                MBLogView()
                            } label: {
                                Label("EXPORT DIAGNOSTIC EVIDENCE", systemImage: "square.and.arrow.up")
                                    .font(.caption.weight(.bold))
                                    .tracking(0.8)
                                    .foregroundStyle(MBBrand.silverBright)
                                    .padding(.top, 6)
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Modules")
    }

    private func capability(_ text: String, _ symbol: String) -> some View {
        HStack(spacing: 11) {
            Image(systemName: symbol)
                .frame(width: 24)
                .foregroundStyle(MBBrand.silverBright)
            Text(text)
                .font(.subheadline)
                .foregroundStyle(MBBrand.silver)
            Spacer()
        }
    }
}

private struct MBFaultsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    HStack {
                        MBSectionHeader(title: "Fault memory", kicker: "Mercedes + OBD")
                        Spacer()
                        let total = connection.mercedesUDSFaults.count +
                            connection.storedDTCs.count +
                            connection.pendingDTCs.count +
                            connection.permanentDTCs.count
                        Text("\(total)")
                            .font(.system(size: 30, weight: .bold, design: .rounded))
                            .foregroundStyle(total == 0 ? MBBrand.success : MBBrand.fault)
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            HStack {
                                MBSectionHeader(title: "Mercedes engine", kicker: "UDS 0x19")
                                Spacer()
                                Text(connection.mercedesUDSFaultStatusText)
                                    .font(.caption)
                                    .foregroundStyle(MBBrand.muted)
                            }

                            if connection.mercedesUDSFaults.isEmpty {
                                emptyFaults("No Mercedes UDS fault records captured")
                            } else {
                                ForEach(connection.mercedesUDSFaults.map(MBFault.init(raw:))) { fault in
                                    mercedesFaultRow(fault)
                                }
                            }

                            Text("Read-only ISO 14229 ReadDTCInformation 19 02 FF. Raw responses remain in the evidence log.")
                                .font(.caption)
                                .foregroundStyle(MBBrand.muted)
                        }
                    }

                    obdFaultPanel("Stored", codes: connection.storedDTCs)
                    obdFaultPanel("Pending", codes: connection.pendingDTCs)
                    obdFaultPanel("Permanent", codes: connection.permanentDTCs)

                    Text("Standard emissions-related fault memory is read separately with OBD-II services 03, 07 and 0A.")
                        .font(.footnote)
                        .foregroundStyle(MBBrand.muted)
                        .padding(.horizontal, 3)
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Faults")
    }

    private func mercedesFaultRow(_ fault: MBFault) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Image(systemName: "exclamationmark.triangle.fill")
                    .foregroundStyle(MBBrand.fault)
                Text(fault.code)
                    .font(.body.monospaced().weight(.bold))
                    .foregroundStyle(MBBrand.silverBright)
                Spacer()
                Text(fault.statusHex)
                    .font(.caption.monospaced())
                    .foregroundStyle(MBBrand.silver)
            }
            if !fault.badges.isEmpty {
                Text(fault.badges.joined(separator: " · "))
                    .font(.caption)
                    .foregroundStyle(MBBrand.warning)
            }
            Divider().overlay(MBBrand.line)
        }
    }

    private func obdFaultPanel(_ title: String, codes: [String]) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 10) {
                HStack {
                    MBSectionHeader(title: title, kicker: "Standard OBD-II")
                    Spacer()
                    Text("\(codes.count)")
                        .font(.title2.monospacedDigit().weight(.bold))
                        .foregroundStyle(codes.isEmpty ? MBBrand.success : MBBrand.fault)
                }

                if codes.isEmpty {
                    emptyFaults("None reported")
                } else {
                    ForEach(codes, id: \.self) { code in
                        HStack {
                            Image(systemName: "exclamationmark.triangle")
                                .foregroundStyle(MBBrand.warning)
                            Text(code)
                                .font(.body.monospaced().weight(.semibold))
                                .foregroundStyle(MBBrand.silverBright)
                            Spacer()
                        }
                    }
                }
            }
        }
    }

    private func emptyFaults(_ text: String) -> some View {
        HStack(spacing: 9) {
            Image(systemName: "checkmark.circle.fill")
                .foregroundStyle(MBBrand.success)
            Text(text)
                .font(.subheadline)
                .foregroundStyle(MBBrand.silver)
            Spacer()
        }
    }
}

private struct MBDieselView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private let groups: [MBParameterGroup] = [
        .engine, .aftertreatment, .air, .fuel, .egr, .electrical
    ]

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 17) {
                    MBSectionHeader(title: "OM651 powertrain", kicker: "Diesel diagnostics")

                    MBPanel {
                        VStack(alignment: .leading, spacing: 10) {
                            HStack {
                                MBSectionHeader(title: "CRD3 manufacturer targets", kicker: "Evidence-gated")
                                Spacer()
                                Text("\(connection.mercedesTargetSignals.count)")
                                    .font(.title2.monospacedDigit().weight(.bold))
                                    .foregroundStyle(MBBrand.silverBright)
                            }

                            Text("Soot, ash, regeneration, injector, boost, rail and EGR manufacturer values remain locked until their raw requests and scaling meet MBLINK's provenance threshold.")
                                .font(.footnote)
                                .foregroundStyle(MBBrand.muted)

                            NavigationLink {
                                MBMercedesTargetsView()
                            } label: {
                                HStack {
                                    Text("VIEW MANUFACTURER TARGET MAP")
                                        .font(.caption.weight(.bold))
                                        .tracking(0.7)
                                    Spacer()
                                    Image(systemName: "chevron.right")
                                }
                                .foregroundStyle(MBBrand.silverBright)
                                .padding(.top, 4)
                            }
                        }
                    }

                    ForEach(groups) { group in
                        let parameters = connection.diagnosticParameters.filter { $0.brandGroup == group }
                        if !parameters.isEmpty {
                            VStack(alignment: .leading, spacing: 10) {
                                HStack {
                                    Label(group.rawValue, systemImage: group.symbol)
                                        .font(.headline)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Spacer()
                                    Text("\(parameters.filter(\.isAvailable).count)/\(parameters.count) LIVE")
                                        .font(.caption2.monospaced().weight(.bold))
                                        .foregroundStyle(MBBrand.muted)
                                }

                                LazyVGrid(
                                    columns: [GridItem(.adaptive(minimum: 155), spacing: 10)],
                                    spacing: 10
                                ) {
                                    ForEach(parameters) { parameter in
                                        MBMetricTile(parameter: parameter)
                                    }
                                }
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Diesel")
    }
}

private struct MBMercedesTargetsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    private let categories = ["dpf", "exhaust", "fuel", "air", "egr", "injector"]

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "OM651 target map", kicker: "Manufacturer diagnostics")

                    MBPanel {
                        Text("Corroborated · mapping pending means the value is independently evidenced, but its raw request or scaling has not yet met MBLINK's provenance threshold.")
                            .font(.footnote)
                            .foregroundStyle(MBBrand.silver)
                    }

                    ForEach(categories, id: \.self) { category in
                        let signals = connection.mercedesSignals(category: category)
                        if !signals.isEmpty {
                            MBPanel {
                                VStack(alignment: .leading, spacing: 12) {
                                    Text(category.uppercased())
                                        .font(.caption.weight(.black))
                                        .tracking(1.5)
                                        .foregroundStyle(MBBrand.silverBright)

                                    ForEach(signals) { signal in
                                        VStack(alignment: .leading, spacing: 4) {
                                            Text(signal.title)
                                                .font(.subheadline.weight(.semibold))
                                                .foregroundStyle(MBBrand.silverBright)
                                            Text(signal.status == "corroborated-unmapped"
                                                 ? "CORROBORATED · MAPPING PENDING"
                                                 : signal.status.uppercased())
                                                .font(.caption2.monospaced().weight(.bold))
                                                .foregroundStyle(MBBrand.warning)
                                        }
                                        if signal.id != signals.last?.id {
                                            Divider().overlay(MBBrand.line)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("OM651 Targets")
    }
}

private struct MBLiveDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var scope: MBLiveScope = .available
    @State private var searchText = ""

    private var filtered: [DiagnosticParameter] {
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
                parameter.brandPidText.localizedCaseInsensitiveContains(searchText)
        }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Live data", kicker: "Shared parameter catalogue")

                    Picker("Show", selection: $scope) {
                        ForEach(MBLiveScope.allCases) { item in
                            Text(item.rawValue).tag(item)
                        }
                    }
                    .pickerStyle(.segmented)
                    .tint(MBBrand.silver)

                    ForEach(MBParameterGroup.allCases) { group in
                        let parameters = filtered.filter { $0.brandGroup == group }
                        if !parameters.isEmpty {
                            MBPanel {
                                VStack(alignment: .leading, spacing: 6) {
                                    Label(group.rawValue, systemImage: group.symbol)
                                        .font(.headline)
                                        .foregroundStyle(MBBrand.silverBright)
                                        .padding(.bottom, 4)

                                    ForEach(parameters) { parameter in
                                        liveRow(parameter)
                                        if parameter.id != parameters.last?.id {
                                            Divider().overlay(MBBrand.line)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if filtered.isEmpty {
                        MBPanel {
                            HStack(spacing: 11) {
                                Image(systemName: "waveform.path.ecg")
                                    .font(.title2)
                                    .foregroundStyle(MBBrand.muted)
                                VStack(alignment: .leading, spacing: 3) {
                                    Text(scope == .available ? "No live values yet" : "No matching parameters")
                                        .font(.headline)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Text(scope == .available
                                         ? "Connect to the vehicle to populate values the ECU actually reports."
                                         : "Change the filter or search text to see more parameters.")
                                        .font(.caption)
                                        .foregroundStyle(MBBrand.muted)
                                }
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .searchable(text: $searchText, prompt: "Parameter, PID or name")
        .mbDiagnosticScreen("Live Data")
    }

    private func liveRow(_ parameter: DiagnosticParameter) -> some View {
        HStack(spacing: 12) {
            VStack(alignment: .leading, spacing: 3) {
                HStack(spacing: 7) {
                    Text(parameter.shortName)
                        .font(.caption.monospaced().weight(.bold))
                        .foregroundStyle(MBBrand.silver)
                    Text(parameter.brandPidText)
                        .font(.caption2.monospaced())
                        .foregroundStyle(MBBrand.muted)
                }
                Text(parameter.title)
                    .font(.subheadline)
                    .foregroundStyle(MBBrand.silverBright)
            }
            Spacer()
            Text(parameter.formattedValue)
                .font(.subheadline.monospacedDigit().weight(.semibold))
                .foregroundStyle(parameter.isAvailable ? MBBrand.silverBright : MBBrand.muted)
            Button {
                connection.toggleFavourite(stableKey: parameter.id)
            } label: {
                Image(systemName: parameter.favourite ? "star.fill" : "star")
                    .foregroundStyle(parameter.favourite ? MBBrand.silverBright : MBBrand.muted)
            }
            .buttonStyle(.plain)
            .accessibilityLabel(parameter.favourite ? "Remove favourite" : "Add favourite")
        }
        .padding(.vertical, 6)
    }
}

private struct MBTableView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.showUnavailableParameters") private var showUnavailableParameters = true

    private var sorted: [DiagnosticParameter] {
        connection.diagnosticParameters
            .filter { showUnavailableParameters || $0.isAvailable }
            .sorted { left, right in
                if left.isAvailable != right.isAvailable {
                    return left.isAvailable && !right.isAvailable
                }
                if left.brandGroup.rawValue != right.brandGroup.rawValue {
                    return left.brandGroup.rawValue < right.brandGroup.rawValue
                }
                return left.parameterIdentifier < right.parameterIdentifier
            }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 14) {
                    MBSectionHeader(title: "Parameter table", kicker: "PID · name · value")

                    if sorted.isEmpty {
                        MBPanel {
                            HStack(spacing: 12) {
                                Image(systemName: "tablecells")
                                    .font(.title2)
                                    .foregroundStyle(MBBrand.muted)
                                VStack(alignment: .leading, spacing: 3) {
                                    Text("No available values yet")
                                        .font(.headline)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Text("Connect to the vehicle, or enable unavailable values in Settings to inspect the full catalogue.")
                                        .font(.caption)
                                        .foregroundStyle(MBBrand.muted)
                                }
                            }
                        }
                    } else {
                        MBPanel {
                            VStack(spacing: 0) {
                                ForEach(sorted) { parameter in
                                    HStack(spacing: 10) {
                                        Text(parameter.brandPidText)
                                            .font(.caption.monospaced().weight(.bold))
                                            .foregroundStyle(MBBrand.silver)
                                            .frame(width: 48, alignment: .leading)

                                        VStack(alignment: .leading, spacing: 2) {
                                            Text(parameter.title)
                                                .font(.subheadline)
                                                .foregroundStyle(MBBrand.silverBright)
                                            Text(parameter.brandGroup.rawValue)
                                                .font(.caption2)
                                                .foregroundStyle(MBBrand.muted)
                                        }

                                        Spacer(minLength: 8)

                                        Text(parameter.formattedValue)
                                            .font(.subheadline.monospacedDigit().weight(.semibold))
                                            .foregroundStyle(parameter.isAvailable ? MBBrand.silverBright : MBBrand.muted)
                                            .multilineTextAlignment(.trailing)

                                        Circle()
                                            .fill(parameter.isAvailable ? MBBrand.success : MBBrand.line)
                                            .frame(width: 7, height: 7)
                                    }
                                    .padding(.vertical, 9)

                                    if parameter.id != sorted.last?.id {
                                        Divider().overlay(MBBrand.line)
                                    }
                                }
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Data Table")
    }
}

private struct MBDashboardView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.preferFavouriteSignals") private var preferFavouriteSignals = true

    private let defaultKeys = [
        "obd2.engine.rpm",
        "obd2.vehicle.speed",
        "obd2.engine.coolant",
        "obd2.diesel.rail_pressure",
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    ]

    private var displayed: [DiagnosticParameter] {
        let available = connection.diagnosticParameters.filter(\.isAvailable)
        let favourites = available.filter(\.favourite)
        if preferFavouriteSignals && !favourites.isEmpty {
            return Array(favourites.prefix(8))
        }
        let preferred = defaultKeys.compactMap { key in
            available.first { $0.id == key }
        }
        return preferred.isEmpty ? Array(available.prefix(6)) : preferred
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    MBSectionHeader(title: "Vehicle dashboard", kicker: "At-a-glance live data")

                    if displayed.isEmpty {
                        MBPanel {
                            HStack(spacing: 12) {
                                Image(systemName: "gauge.with.dots.needle.67percent")
                                    .font(.title2)
                                    .foregroundStyle(MBBrand.muted)
                                VStack(alignment: .leading, spacing: 3) {
                                    Text("Waiting for vehicle data")
                                        .font(.headline)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Text(preferFavouriteSignals
                                         ? "Connect to the vehicle. Favourite live parameters become the dashboard automatically."
                                         : "Connect to the vehicle. MBLINK will show its default powertrain instruments.")
                                        .font(.caption)
                                        .foregroundStyle(MBBrand.muted)
                                }
                            }
                        }
                    } else {
                        LazyVGrid(
                            columns: [GridItem(.adaptive(minimum: 155), spacing: 11)],
                            spacing: 11
                        ) {
                            ForEach(displayed) { parameter in
                                MBMetricTile(parameter: parameter)
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Dashboard")
    }
}

private struct MBGraphsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.preferFavouriteSignals") private var preferFavouriteSignals = true
    @State private var selectedKeys = Set<String>()
    @State private var showingSignalPicker = false

    private let defaultKeys = [
        "obd2.engine.rpm",
        "obd2.diesel.rail_pressure",
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    ]

    private var graphed: [DiagnosticParameter] {
        let withHistory = connection.diagnosticParameters.filter { !$0.history.isEmpty }
        if !selectedKeys.isEmpty {
            return Array(withHistory.filter { selectedKeys.contains($0.id) }.prefix(4))
        }
        let favourites = withHistory.filter(\.favourite)
        if preferFavouriteSignals && !favourites.isEmpty {
            return Array(favourites.prefix(4))
        }
        let preferred = defaultKeys.compactMap { key in
            withHistory.first { $0.id == key }
        }
        return preferred.isEmpty ? Array(withHistory.prefix(4)) : preferred
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    MBSectionHeader(title: "Signal history", kicker: "Instrument traces")

                    if graphed.isEmpty {
                        MBPanel {
                            HStack(spacing: 12) {
                                Image(systemName: "chart.xyaxis.line")
                                    .font(.title2)
                                    .foregroundStyle(MBBrand.muted)
                                VStack(alignment: .leading, spacing: 3) {
                                    Text("Waiting for live samples")
                                        .font(.headline)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Text(preferFavouriteSignals
                                         ? "Connect to the vehicle. Favourites are graphed first; otherwise MBLINK uses its powertrain defaults."
                                         : "Connect to the vehicle. MBLINK will graph its default powertrain signals.")
                                        .font(.caption)
                                        .foregroundStyle(MBBrand.muted)
                                }
                            }
                        }
                    } else {
                        ForEach(graphed) { parameter in
                            graphPanel(parameter)
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Graphs")
        .toolbar {
            ToolbarItem(placement: .topBarTrailing) {
                Button("Choose") {
                    showingSignalPicker = true
                }
            }
        }
        .sheet(isPresented: $showingSignalPicker) {
            MBGraphSignalPicker(
                parameters: connection.diagnosticParameters,
                selectedKeys: $selectedKeys
            )
            .preferredColorScheme(.dark)
            .tint(MBBrand.silverBright)
        }
    }

    private func graphPanel(_ parameter: DiagnosticParameter) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 11) {
                HStack {
                    VStack(alignment: .leading, spacing: 3) {
                        Text(parameter.title)
                            .font(.headline)
                            .foregroundStyle(MBBrand.silverBright)
                        Text("\(parameter.brandPidText) · \(parameter.shortName)")
                            .font(.caption.monospaced())
                            .foregroundStyle(MBBrand.muted)
                    }
                    Spacer()
                    Text(parameter.formattedValue)
                        .font(.headline.monospacedDigit())
                        .foregroundStyle(MBBrand.silverBright)
                }

                if parameter.history.count > 1 {
                    Chart(Array(parameter.history.enumerated()), id: \.offset) { point in
                        LineMark(
                            x: .value("Sample", point.offset),
                            y: .value(parameter.title, point.element)
                        )
                        .foregroundStyle(MBBrand.silverBright)
                    }
                    .chartXAxis {
                        AxisMarks(values: .automatic(desiredCount: 4)) {
                            AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5))
                                .foregroundStyle(MBBrand.line)
                            AxisValueLabel()
                                .foregroundStyle(MBBrand.muted)
                        }
                    }
                    .chartYAxis {
                        AxisMarks {
                            AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5))
                                .foregroundStyle(MBBrand.line)
                            AxisValueLabel()
                                .foregroundStyle(MBBrand.muted)
                        }
                    }
                    .frame(height: 190)
                } else {
                    Text("Waiting for more samples")
                        .font(.caption)
                        .foregroundStyle(MBBrand.muted)
                        .frame(maxWidth: .infinity, minHeight: 120)
                }
            }
        }
    }
}

private struct MBGraphSignalPicker: View {
    @Environment(\.dismiss) private var dismiss
    let parameters: [DiagnosticParameter]
    @Binding var selectedKeys: Set<String>

    var body: some View {
        NavigationStack {
            ZStack {
                MBBackground()
                ScrollView {
                    VStack(alignment: .leading, spacing: 15) {
                        MBSectionHeader(title: "Graph signals", kicker: "Choose up to four")

                        MBPanel {
                            Text("Clear the selection to return to automatic favourites and powertrain defaults.")
                                .font(.footnote)
                                .foregroundStyle(MBBrand.silver)
                        }

                        ForEach(MBParameterGroup.allCases) { group in
                            let groupParameters = parameters.filter { $0.brandGroup == group }
                            if !groupParameters.isEmpty {
                                MBPanel {
                                    VStack(alignment: .leading, spacing: 5) {
                                        Label(group.rawValue, systemImage: group.symbol)
                                            .font(.headline)
                                            .foregroundStyle(MBBrand.silverBright)
                                            .padding(.bottom, 4)

                                        ForEach(groupParameters) { parameter in
                                            signalRow(parameter)
                                            if parameter.id != groupParameters.last?.id {
                                                Divider().overlay(MBBrand.line)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    .padding(16)
                }
            }
            .navigationTitle("Graph Signals")
            .navigationBarTitleDisplayMode(.inline)
            .toolbarBackground(MBBrand.chrome, for: .navigationBar)
            .toolbarBackground(.visible, for: .navigationBar)
            .toolbarColorScheme(.dark, for: .navigationBar)
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

    private func signalRow(_ parameter: DiagnosticParameter) -> some View {
        let selected = selectedKeys.contains(parameter.id)
        let selectionLimitReached = !selected && selectedKeys.count >= 4

        return Button {
            if selected {
                selectedKeys.remove(parameter.id)
            } else if !selectionLimitReached {
                selectedKeys.insert(parameter.id)
            }
        } label: {
            HStack(spacing: 11) {
                VStack(alignment: .leading, spacing: 3) {
                    Text(parameter.title)
                        .font(.subheadline)
                        .foregroundStyle(selectionLimitReached ? MBBrand.muted : MBBrand.silverBright)
                    Text("\(parameter.brandPidText) · \(parameter.formattedValue)")
                        .font(.caption.monospaced())
                        .foregroundStyle(MBBrand.muted)
                }
                Spacer()
                Image(systemName: selected ? "checkmark.circle.fill" : "circle")
                    .foregroundStyle(selected ? MBBrand.silverBright : MBBrand.muted)
            }
            .contentShape(Rectangle())
            .padding(.vertical, 6)
        }
        .buttonStyle(.plain)
        .disabled(selectionLimitReached)
    }
}

private struct MBLogView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Diagnostic evidence", kicker: "Session recorder")

                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "Engine endpoint", value: connection.mercedesProbeEndpointText)
                            MBInfoRow(label: "Mercedes probe", value: connection.mercedesProbeStatusText)
                            MBInfoRow(label: "VIN", value: connection.mercedesVINText, monospaced: true)
                            MBInfoRow(label: "Identity", value: connection.mercedesIdentitySummaryText)
                            MBInfoRow(label: "CRD3", value: connection.mercedesCrd3SummaryText)
                            MBInfoRow(label: "Mercedes UDS faults", value: connection.mercedesUDSFaultStatusText)
                            MBInfoRow(label: "OBD-II fault scan", value: connection.faultScanStatusText)
                            MBInfoRow(label: "Recorded samples", value: "\(connection.recordedSampleCount)")
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            MBSectionHeader(title: "Export", kicker: "Raw transcript + telemetry")

                            Text("The recorder retains the raw ELM327 command/response transcript, CRD3 identity evidence, Mercedes UDS fault reads, standard OBD-II faults and live diesel/DPF requests.")
                                .font(.footnote)
                                .foregroundStyle(MBBrand.muted)

                            Button {
                                connection.prepareCSVExport()
                            } label: {
                                HStack {
                                    Label("PREPARE DIAGNOSTIC EVIDENCE CSV", systemImage: "doc.badge.gearshape")
                                    Spacer()
                                }
                                .font(.caption.weight(.bold))
                                .tracking(0.5)
                                .foregroundStyle(Color.black)
                                .padding(.horizontal, 14)
                                .padding(.vertical, 12)
                                .background(
                                    RoundedRectangle(cornerRadius: 11, style: .continuous)
                                        .fill(MBBrand.silverBright)
                                )
                            }
                            .buttonStyle(.plain)

                            if let exportURL = connection.csvExportURL {
                                ShareLink(item: exportURL) {
                                    HStack {
                                        Label("SHARE DIAGNOSTIC EVIDENCE", systemImage: "square.and.arrow.up")
                                        Spacer()
                                    }
                                    .font(.caption.weight(.bold))
                                    .tracking(0.5)
                                    .foregroundStyle(MBBrand.silverBright)
                                    .padding(.horizontal, 14)
                                    .padding(.vertical, 12)
                                    .background(
                                        RoundedRectangle(cornerRadius: 11, style: .continuous)
                                            .fill(MBBrand.panelRaised)
                                    )
                                    .overlay(
                                        RoundedRectangle(cornerRadius: 11, style: .continuous)
                                            .stroke(MBBrand.line, lineWidth: 1)
                                    )
                                }
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Evidence")
    }
}

private struct MBSettingsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.preferFavouriteSignals") private var preferFavouriteSignals = true
    @AppStorage("mblink.showUnavailableParameters") private var showUnavailableParameters = true

    private var version: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "Unknown"
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "System", kicker: "MBLINK")

                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "Adapter", value: connection.peripheralName)
                            MBInfoRow(label: "Identity", value: connection.adapterIdentifier, monospaced: true)
                            MBInfoRow(label: "Version", value: version, monospaced: true)
                            MBInfoRow(label: "Bundle ID", value: Bundle.main.bundleIdentifier ?? "Unknown", monospaced: true)
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 14) {
                            MBSectionHeader(title: "Display", kicker: "Preferences")

                            Toggle("Prefer favourites on Dashboard and Graphs", isOn: $preferFavouriteSignals)
                                .tint(MBBrand.silverBright)
                                .foregroundStyle(MBBrand.silverBright)

                            Divider().overlay(MBBrand.line)

                            Toggle("Show unavailable values in Data Table", isOn: $showUnavailableParameters)
                                .tint(MBBrand.silverBright)
                                .foregroundStyle(MBBrand.silverBright)
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 9) {
                            MBSectionHeader(title: "Architecture", kicker: "Portable core")
                            Text("The interface renders the shared C diagnostic parameter catalogue and evidence-backed OM651 target catalogue. Portable diagnostics, metadata, scheduling, telemetry and protocol behaviour remain owned by libmblink and Infiltratr Common rather than being duplicated in the iPhone layer.")
                                .font(.footnote)
                                .foregroundStyle(MBBrand.silver)
                        }
                    }

                    MBPanel {
                        HStack(spacing: 13) {
                            MBLogoMark(size: 46)
                            VStack(alignment: .leading, spacing: 3) {
                                Text("BLACK · SILVER · MERCEDES-ORIENTED")
                                    .font(.caption2.weight(.bold))
                                    .tracking(1.0)
                                    .foregroundStyle(MBBrand.silver)
                                Text("MBLINK diagnostic command interface")
                                    .font(.headline)
                                    .foregroundStyle(MBBrand.silverBright)
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Settings")
    }
}

private enum MBLINKAboutDetail: String, Identifiable {
    case credits
    case license

    var id: String { rawValue }
}

private struct MBLINKAboutView: View {
    let onClose: () -> Void
    @State private var detail: MBLINKAboutDetail?

    private var version: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "Unknown"
    }

    var body: some View {
        ZStack {
            MBBackground()

            VStack(spacing: 0) {
                ScrollView {
                    VStack(spacing: 17) {
                        MBLogoMark(size: 82)
                            .padding(.top, 30)

                        VStack(spacing: 4) {
                            Text("MBLINK")
                                .font(.system(size: 34, weight: .black, design: .rounded))
                                .tracking(2.0)
                                .foregroundStyle(MBBrand.silverBright)
                            Text("MERCEDES DIAGNOSTIC COMMAND")
                                .font(.caption2.weight(.bold))
                                .tracking(1.7)
                                .foregroundStyle(MBBrand.silver)
                        }

                        Text("Version \(version)")
                            .font(.subheadline.monospaced())
                            .foregroundStyle(MBBrand.muted)

                        Text("A C-first, open-source vehicle diagnostics platform authored by Shannon Smith.")
                            .font(.body)
                            .multilineTextAlignment(.center)
                            .foregroundStyle(MBBrand.silverBright)
                            .padding(.horizontal, 28)

                        Text("Copyright © 2026 Shannon Smith")
                            .font(.subheadline)
                            .foregroundStyle(MBBrand.muted)

                        Link(
                            "Project Website",
                            destination: URL(string: "https://github.com/The-First-Infiltrator/MBLINK")!
                        )
                        .font(.body.weight(.semibold))
                        .foregroundStyle(MBBrand.silverBright)
                    }
                    .frame(maxWidth: .infinity)
                }

                HStack(spacing: 10) {
                    Button("Credits") {
                        detail = .credits
                    }
                    .buttonStyle(.bordered)

                    Button("License") {
                        detail = .license
                    }
                    .buttonStyle(.bordered)

                    Button("Close") {
                        onClose()
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(MBBrand.silverBright)
                    .foregroundStyle(Color.black)
                }
                .frame(maxWidth: .infinity)
                .padding(.horizontal, 16)
                .padding(.vertical, 12)
                .background(MBBrand.chrome)
                .overlay(alignment: .top) {
                    Rectangle().fill(MBBrand.line).frame(height: 1)
                }
            }
        }
        .presentationDetents([.medium, .large])
        .presentationDragIndicator(.visible)
        .sheet(item: $detail) { item in
            switch item {
            case .credits:
                NavigationStack {
                    ZStack {
                        MBBackground()
                        MBPanel {
                            Text("Shannon Smith — Author and project maintainer")
                                .foregroundStyle(MBBrand.silverBright)
                        }
                        .padding(16)
                    }
                    .navigationTitle("Credits")
                    .navigationBarTitleDisplayMode(.inline)
                    .toolbarBackground(MBBrand.chrome, for: .navigationBar)
                    .toolbarBackground(.visible, for: .navigationBar)
                    .toolbar {
                        ToolbarItem(placement: .confirmationAction) {
                            Button("Close") { detail = nil }
                        }
                    }
                }
            case .license:
                NavigationStack {
                    ZStack {
                        MBBackground()
                        ScrollView {
                            MBPanel {
                                Text(
                                    "MBLINK is free software licensed under the GNU General Public License version 3 or, at your option, any later version (GPL-3.0-or-later).\n\nSee LICENSE in the source package for the complete licence text."
                                )
                                .foregroundStyle(MBBrand.silverBright)
                            }
                            .padding(16)
                        }
                    }
                    .navigationTitle("License")
                    .navigationBarTitleDisplayMode(.inline)
                    .toolbarBackground(MBBrand.chrome, for: .navigationBar)
                    .toolbarBackground(.visible, for: .navigationBar)
                    .toolbar {
                        ToolbarItem(placement: .confirmationAction) {
                            Button("Close") { detail = nil }
                        }
                    }
                }
            }
        }
    }
}
