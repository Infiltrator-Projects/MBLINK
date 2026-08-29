// SPDX-License-Identifier: GPL-3.0-or-later
import Charts
import Foundation
import SwiftUI
import UIKit

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

private let mbDashboardColumns = [
    GridItem(.flexible(), spacing: 14),
    GridItem(.flexible(), spacing: 14)
]

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
    var size: CGFloat = 52

    var body: some View {
        Image("MBLINKEmblem")
            .resizable()
            .scaledToFit()
            .frame(width: size, height: size)
            .shadow(color: .black.opacity(0.7), radius: 8, y: 5)
            .accessibilityHidden(true)
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
            Text(LocalizedStringKey(text)).textCase(.uppercase)
                .font(.caption2.weight(.bold))
                .tracking(0.8)
                .lineLimit(1)
        }
        .foregroundStyle(MBBrand.silverBright)
        .padding(.horizontal, 10)
        .padding(.vertical, 7)
        .background(Capsule().fill(MBBrand.panelRaised))
        .overlay(Capsule().stroke(MBBrand.line, lineWidth: 1))
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
                    .fill(MBBrand.panel)
            )
            .overlay(
                RoundedRectangle(cornerRadius: 18, style: .continuous)
                    .stroke(MBBrand.line.opacity(0.85), lineWidth: 1)
            )
    }
}

private struct MBSectionHeader: View {
    let title: String
    var kicker: String? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 3) {
            if let kicker {
                Text(LocalizedStringKey(kicker)).textCase(.uppercase)
                    .font(.caption2.weight(.bold))
                    .tracking(1.4)
                    .foregroundStyle(MBBrand.muted)
            }
            Text(LocalizedStringKey(title))
                .font(.title3.weight(.semibold))
                .foregroundStyle(MBBrand.silverBright)
        }
    }
}

private struct MBInfoRow: View {
    let label: String
    let value: String
    var monospaced = false

    var body: some View {
        HStack(alignment: .firstTextBaseline, spacing: 14) {
            Text(LocalizedStringKey(label))
                .font(.subheadline)
                .foregroundStyle(MBBrand.muted)
            Spacer(minLength: 16)
            Text(LocalizedStringKey(value))
                .font(monospaced ? .subheadline.monospaced() : .subheadline.weight(.medium))
                .foregroundStyle(MBBrand.silverBright)
                .multilineTextAlignment(.trailing)
        }
        .padding(.vertical, 4)
    }
}

private struct MBVehicleFact: Identifiable {
    let label: String
    let value: String
    var monospaced = false

    var id: String { label }
}

private struct MBVehicleFactTile: View {
    let fact: MBVehicleFact

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(LocalizedStringKey(fact.label)).textCase(.uppercase)
                .font(.caption2.weight(.bold))
                .tracking(0.8)
                .foregroundStyle(MBBrand.muted)
            Text(fact.value)
                .font(fact.monospaced ? .subheadline.monospaced() : .subheadline.weight(.semibold))
                .foregroundStyle(MBBrand.silverBright)
                .lineLimit(3)
                .minimumScaleFactor(0.8)
                .textSelection(.enabled)
        }
        .frame(maxWidth: .infinity, minHeight: 64, alignment: .topLeading)
        .padding(12)
        .background(
            RoundedRectangle(cornerRadius: 12, style: .continuous)
                .fill(MBBrand.panelRaised)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 12, style: .continuous)
                .stroke(MBBrand.line.opacity(0.75), lineWidth: 1)
        )
    }
}

private struct MBVehicleFactGrid: View {
    let facts: [MBVehicleFact]

    private let columns = [
        GridItem(.adaptive(minimum: 132, maximum: 260), spacing: 10)
    ]

    var body: some View {
        LazyVGrid(columns: columns, alignment: .leading, spacing: 10) {
            ForEach(facts) { fact in
                MBVehicleFactTile(fact: fact)
            }
        }
    }
}

private struct MBHomeTile<Destination: View>: View {
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
            MBTileFace(title: title, subtitle: subtitle, symbol: symbol)
        }
        .buttonStyle(.plain)
    }
}

private struct MBActionTile: View {
    let title: String
    let subtitle: String
    let symbol: String
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            MBTileFace(title: title, subtitle: subtitle, symbol: symbol)
        }
        .buttonStyle(.plain)
    }
}

private struct MBTileFace: View {
    let title: String
    let subtitle: String
    let symbol: String

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Image(systemName: symbol)
                .font(.system(size: 24, weight: .semibold))
                .foregroundStyle(MBBrand.silverBright)
                .frame(width: 30, height: 30, alignment: .leading)

            Text(LocalizedStringKey(title))
                .font(.headline.weight(.semibold))
                .foregroundStyle(MBBrand.silverBright)
                .lineLimit(1)

            Text(LocalizedStringKey(subtitle))
                .font(.caption)
                .foregroundStyle(MBBrand.muted)
                .lineLimit(2)
                .fixedSize(horizontal: false, vertical: true)

            Spacer(minLength: 0)

            HStack {
                Spacer()
                Image(systemName: "chevron.right")
                    .font(.caption.weight(.bold))
                    .foregroundStyle(MBBrand.silver.opacity(0.72))
            }
        }
        .frame(maxWidth: .infinity, minHeight: 118, alignment: .leading)
        .padding(16)
        .background(
            RoundedRectangle(cornerRadius: 19, style: .continuous)
                .fill(
                    LinearGradient(
                        colors: [MBBrand.panelRaised, MBBrand.panel],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    )
                )
        )
        .overlay(
            RoundedRectangle(cornerRadius: 19, style: .continuous)
                .stroke(MBBrand.line, lineWidth: 1)
        )
    }
}

private struct MBMetricTile: View {
    let parameter: DiagnosticParameter

    var body: some View {
        VStack(alignment: .leading, spacing: 9) {
            HStack {
                Text(LocalizedStringKey(parameter.shortName)).textCase(.uppercase)
                    .font(.caption2.monospaced().weight(.bold))
                    .tracking(0.7)
                    .foregroundStyle(MBBrand.silver)
                Spacer()
                Text(parameter.brandPidText)
                    .font(.caption2.monospaced())
                    .foregroundStyle(MBBrand.muted)
            }

            Text(parameter.pollingEnabled ? parameter.formattedValue : "OFF")
                .font(.system(size: 24, weight: .semibold, design: .rounded))
                .monospacedDigit()
                .foregroundStyle(parameter.pollingEnabled && parameter.isAvailable ? MBBrand.silverBright : MBBrand.muted)
                .minimumScaleFactor(0.65)
                .lineLimit(1)

            Text(LocalizedStringKey(parameter.title))
                .font(.caption)
                .foregroundStyle(MBBrand.muted)
                .lineLimit(2)
        }
        .frame(maxWidth: .infinity, minHeight: 112, alignment: .leading)
        .padding(14)
        .background(
            RoundedRectangle(cornerRadius: 15, style: .continuous)
                .fill(MBBrand.panel)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 15, style: .continuous)
                .stroke(parameter.isAvailable ? MBBrand.silver.opacity(0.38) : MBBrand.line, lineWidth: 1)
        )
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
        if id.contains(".dpf.") || id.contains(".aftertreatment.") { return .aftertreatment }
        if id.contains(".diesel.egr") { return .egr }
        if id.contains(".diesel.rail_pressure") || id.contains(".fuel_rate") ||
            id.contains(".fuel.") { return .fuel }
        if id.contains(".electrical.") { return .electrical }
        if id.contains(".engine.map") || id.contains(".barometric_pressure") ||
            id.contains(".engine.maf") || id.contains(".intake_air") || id.contains(".environment.") {
            return .air
        }
        return .engine
    }

    var brandPidText: String {
        let value = String(parameterIdentifier, radix: 16, uppercase: true)
        return "0x" + (value.count < 2 ? "0\(value)" : value)
    }

    var brandSourceText: String {
        id.hasPrefix("obd2.") ? "SAE OBD-II · \(brandPidText)" :
            "\(protocolName.uppercased()) · \(brandPidText)"
    }
}

private struct MBInterfaceLanguage: Identifiable, Hashable {
    let id: String
    let nativeName: String

    static let all: [MBInterfaceLanguage] = {
        let count = Int(link_i18n_supported_locale_count())
        return (0..<count).compactMap { index in
            guard let locale = link_i18n_supported_locale(index),
                  let name = link_i18n_supported_locale_name(index) else { return nil }
            return MBInterfaceLanguage(id: String(cString: locale), nativeName: String(cString: name))
        }
    }()

    static func canonical(_ stored: String) -> String {
        switch stored {
        case "en": return "en-AU"
        case "de": return "de-DE"
        case "pl": return "pl-PL"
        default: return all.contains(where: { $0.id == stored }) ? stored : "en-AU"
        }
    }

    static func displayName(for stored: String) -> String {
        let code = canonical(stored)
        return all.first(where: { $0.id == code })?.nativeName ?? "English (Australia)"
    }
}

@main
struct MBLINKApp: App {
    @Environment(\.scenePhase) private var scenePhase
    @StateObject private var connection = ConnectionViewModel()
    @State private var showingAbout = false
    @AppStorage("mblink.language") private var language = "en-AU"

    var body: some Scene {
        WindowGroup {
            MBCommandCentreView()
                .environmentObject(connection)
                .environment(\.locale, Locale(identifier: MBInterfaceLanguage.canonical(language)))
                .environment(\.layoutDirection, MBInterfaceLanguage.canonical(language).hasPrefix("ar") ? .rightToLeft : .leftToRight)
                .onAppear {
                    let canonical = MBInterfaceLanguage.canonical(language)
                    if language != canonical { language = canonical }
                    UIApplication.shared.isIdleTimerDisabled = true
                }
                .onChange(of: scenePhase) { _, phase in
                    UIApplication.shared.isIdleTimerDisabled = (phase == .active)
                }
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
                            Rectangle().fill(MBBrand.line).frame(height: 1)
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

    var body: some View {
        NavigationStack {
            ZStack {
                MBBackground()
                ScrollView {
                    VStack(alignment: .leading, spacing: 18) {
                        header
                        tileGrid
                    }
                    .padding(.horizontal, 20)
                    .padding(.top, 18)
                    .padding(.bottom, 30)
                }
            }
            .toolbar(.hidden, for: .navigationBar)
        }
    }

    private var header: some View {
        ViewThatFits(in: .horizontal) {
            HStack(alignment: .center, spacing: 14) {
                brandIdentity
                Spacer(minLength: 8)
                MBStatusPill(text: connection.statusText, active: connection.isReady)
            }

            VStack(alignment: .leading, spacing: 11) {
                brandIdentity
                MBStatusPill(text: connection.statusText, active: connection.isReady)
            }
        }
    }

    private var brandIdentity: some View {
        HStack(spacing: 14) {
            MBLogoMark(size: 54)
            VStack(alignment: .leading, spacing: 3) {
                Text("MBLINK")
                    .font(.system(size: 29, weight: .black, design: .rounded))
                    .tracking(1.6)
                    .foregroundStyle(MBBrand.silverBright)
                Text("MERCEDES DIAGNOSTICS")
                    .font(.caption2.weight(.bold))
                    .tracking(1.4)
                    .foregroundStyle(MBBrand.silver)
                Text("C207 · OM651 · DELPHI CRD3.x")
                    .font(.caption.monospaced())
                    .foregroundStyle(MBBrand.muted)
                    .lineLimit(1)
            }
        }
    }

    private var tileGrid: some View {
        LazyVGrid(columns: mbDashboardColumns, spacing: 14) {
            MBActionTile(
                title: "Connect",
                subtitle: connection.isActive ? "Disconnect vehicle link" : "Adapter and vehicle link",
                symbol: connection.isActive ? "cable.connector.slash" : "cable.connector"
            ) {
                connection.isActive ? connection.disconnect() : connection.connect()
            }

            MBHomeTile("Live Data", "Sensors and values", "waveform.path.ecg") {
                MBLiveDataView()
            }

            MBHomeTile("Faults", "Stored, pending, permanent", "exclamationmark.triangle.fill") {
                MBFaultsView()
            }

            MBHomeTile("Vehicle", "Identity and profile", "car.side.fill") {
                MBVehicleView()
            }

            MBHomeTile("Modules", "Networks and capabilities", "square.stack.3d.up.fill") {
                MBModulesView()
            }

            MBHomeTile("Dashboard", "At-a-glance measurements", "gauge.with.dots.needle.67percent") {
                MBDashboardView()
            }

            MBHomeTile("Evidence", "Session log and CSV", "doc.text.magnifyingglass") {
                MBEvidenceView()
            }

            MBHomeTile("Settings", "Adapter and app details", "gearshape.fill") {
                MBSettingsView()
            }
        }
    }
}

private extension View {
    func mbDiagnosticScreen(_ title: String) -> some View {
        self
            .background(MBBrand.background.ignoresSafeArea())
            .navigationTitle(LocalizedStringKey(title))
            .navigationBarTitleDisplayMode(.inline)
            .toolbarBackground(MBBrand.chrome, for: .navigationBar)
            .toolbarBackground(.visible, for: .navigationBar)
            .toolbarColorScheme(.dark, for: .navigationBar)
    }
}

private struct MBVehicleView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var totalFaultCount: Int {
        connection.mercedesUDSFaults.count + connection.storedDTCs.count +
            connection.pendingDTCs.count + connection.permanentDTCs.count
    }

    private var identityFacts: [MBVehicleFact] {
        guard let identity = connection.vehicleIdentity else { return [] }
        return [
            identity.chassis.map { MBVehicleFact(label: "Chassis", value: $0) },
            identity.bodyStyle.map { MBVehicleFact(label: "Body", value: $0) },
            identity.baumuster.map { MBVehicleFact(label: "Baumuster", value: formattedBaumuster($0), monospaced: true) },
            identity.productionYears.map { MBVehicleFact(label: "Production", value: $0) }
        ].compactMap { $0 }
    }

    private var engineFacts: [MBVehicleFact] {
        guard let identity = connection.vehicleIdentity else { return [] }
        return [
            identity.engineCode.map { MBVehicleFact(label: "Engine", value: $0, monospaced: true) },
            identity.engineFamily.map { MBVehicleFact(label: "Family", value: $0, monospaced: true) },
            identity.displacementCC.map { MBVehicleFact(label: "Capacity", value: formattedNumber($0) + " cc") },
            identity.ratedPowerKW.map { MBVehicleFact(label: "Factory output", value: "\($0) kW") },
            identity.fuel.map { MBVehicleFact(label: "Fuel", value: $0) }
        ].compactMap { $0 }
    }

    private var buildFacts: [MBVehicleFact] {
        guard let identity = connection.vehicleIdentity else { return [] }
        return [
            identity.plant.map { MBVehicleFact(label: "Assembly plant", value: $0) },
            identity.country.map { MBVehicleFact(label: "Country", value: $0) },
            identity.steering.map { MBVehicleFact(label: "Steering", value: $0) },
            identity.serialNumber.map { MBVehicleFact(label: "Production serial", value: $0, monospaced: true) }
        ].compactMap { $0 }
    }

    private func formattedBaumuster(_ value: String) -> String {
        guard value.count == 6 else { return value }
        return String(value.prefix(3)) + "." + String(value.suffix(3))
    }

    private func formattedNumber(_ value: UInt32) -> String {
        NumberFormatter.localizedString(from: NSNumber(value: value), number: .decimal)
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Vehicle identity", kicker: "Mercedes-Benz")
                    vehicleHero
                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            MBSectionHeader(title: "Vehicle", kicker: "Decoded VIN")
                            if identityFacts.isEmpty {
                                Text("Decoded Mercedes vehicle details will appear here after VIN identification.")
                                    .font(.subheadline)
                                    .foregroundStyle(MBBrand.muted)
                            } else {
                                MBVehicleFactGrid(facts: identityFacts)
                            }
                        }
                    }
                    if !engineFacts.isEmpty {
                        MBPanel {
                            VStack(alignment: .leading, spacing: 12) {
                                MBSectionHeader(title: "Powertrain", kicker: "Factory specification")
                                MBVehicleFactGrid(facts: engineFacts)
                            }
                        }
                    }
                    if !buildFacts.isEmpty {
                        MBPanel {
                            VStack(alignment: .leading, spacing: 12) {
                                MBSectionHeader(title: "Build", kicker: "Production identity")
                                MBVehicleFactGrid(facts: buildFacts)
                            }
                        }
                    }
                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "Connection", value: connection.statusText)
                            MBInfoRow(label: "Vehicle profile", value: connection.vehicleProfileStatusText)
                            MBInfoRow(label: "Fault records", value: "\(totalFaultCount)")
                            MBInfoRow(label: "Endpoint", value: connection.mercedesProbeEndpointText)
                            MBInfoRow(label: "CRD3 identity", value: connection.mercedesCrd3SummaryText)
                            MBInfoRow(label: "Identity sweep", value: connection.mercedesIdentitySummaryText)
                            MBInfoRow(label: "Probe", value: connection.mercedesProbeStatusText)
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Vehicle")
    }

    @ViewBuilder
    private var vehicleHero: some View {
        MBPanel {
            if let identity = connection.vehicleIdentity {
                HStack(alignment: .top, spacing: 14) {
                    Image(systemName: "car.side.fill")
                        .font(.system(size: 30, weight: .semibold))
                        .foregroundStyle(MBBrand.silverBright)
                        .frame(width: 42, height: 42)
                    VStack(alignment: .leading, spacing: 5) {
                        Text(identity.model ?? identity.manufacturer)
                            .font(.title2.weight(.bold))
                            .foregroundStyle(MBBrand.silverBright)
                        Text([identity.chassis, identity.bodyStyle, identity.engineFamily]
                            .compactMap { $0 }.joined(separator: " · "))
                            .font(.subheadline.weight(.medium))
                            .foregroundStyle(MBBrand.silver)
                        Text(identity.vin)
                            .font(.subheadline.monospaced().weight(.semibold))
                            .foregroundStyle(MBBrand.silverBright)
                            .textSelection(.enabled)
                    }
                }
            } else {
                VStack(alignment: .leading, spacing: 6) {
                    Text("Waiting for vehicle VIN")
                        .font(.headline)
                        .foregroundStyle(MBBrand.silverBright)
                    Text(connection.mercedesVINText)
                        .font(.subheadline.monospaced())
                        .foregroundStyle(MBBrand.muted)
                }
            }
        }
    }
}

private struct MBModulesView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var observedModules: [String] {
        connection.mercedesIdentityResults.filter { $0.hasPrefix("MODULE ·") }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Delphi CRD3.x", kicker: "Modules and capabilities")
                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "VIN profile", value: connection.vehicleProfileStatusText)
                            MBInfoRow(label: "Module state", value: connection.mercedesProbeStatusText)
                        }
                    }
                    if !observedModules.isEmpty {
                        MBPanel {
                            VStack(alignment: .leading, spacing: 9) {
                                MBSectionHeader(title: "Observed modules", kicker: "Saved + live verification")
                                ForEach(observedModules, id: \.self) { module in
                                    Text(module)
                                        .font(.caption.monospaced())
                                        .foregroundStyle(MBBrand.silverBright)
                                        .textSelection(.enabled)
                                }
                            }
                        }
                    }
                    MBPanel {
                        VStack(alignment: .leading, spacing: 11) {
                            capability("Standard OBD-II engine diagnostics", "waveform.path.ecg")
                            capability("UDS / ISO-TP diagnostic engine", "point.3.connected.trianglepath.dotted")
                            capability("CRD3 ECU identity and fingerprinting", "checkmark.seal.fill")
                            capability("Read-only UDS fault memory", "exclamationmark.triangle.fill")
                        }
                    }
                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            MBSectionHeader(title: "Mercedes target map", kicker: "OM651")
                            Text("\(connection.mercedesTargetSignals.count) evidence-backed manufacturer targets")
                                .font(.subheadline)
                                .foregroundStyle(MBBrand.silver)
                            NavigationLink {
                                MBDieselView()
                            } label: {
                                HStack {
                                    Label("Diesel / DPF diagnostics", systemImage: "engine.combustion.fill")
                                    Spacer()
                                    Image(systemName: "chevron.right")
                                }
                                .font(.subheadline.weight(.semibold))
                                .foregroundStyle(MBBrand.silverBright)
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

    private var total: Int {
        connection.mercedesUDSFaults.count + connection.storedFaults.count +
            connection.pendingFaults.count + connection.permanentFaults.count
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    HStack {
                        MBSectionHeader(title: "Fault memory", kicker: "Mercedes + OBD")
                        Spacer()
                        Text("\(total)")
                            .font(.system(size: 30, weight: .bold, design: .rounded))
                            .foregroundStyle(total == 0 ? MBBrand.success : MBBrand.fault)
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 10) {
                            MBSectionHeader(title: "Mercedes modules", kicker: "UDS")
                            Text(connection.mercedesUDSFaultStatusText)
                                .font(.caption)
                                .foregroundStyle(MBBrand.muted)
                            if connection.mercedesUDSFaults.isEmpty {
                                emptyFaults("No Mercedes module UDS fault records captured")
                            } else {
                                ForEach(connection.mercedesUDSFaults, id: \.self) { fault in
                                    Text(fault)
                                        .font(.body.monospaced().weight(.semibold))
                                        .foregroundStyle(MBBrand.silverBright)
                                }
                            }
                        }
                    }

                    faultPanel("Stored", faults: connection.storedFaults)
                    faultPanel("Pending", faults: connection.pendingFaults)
                    faultPanel("Permanent", faults: connection.permanentFaults)
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Faults")
    }

    private func faultPanel(_ title: String, faults: [DiagnosticFault]) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 10) {
                HStack {
                    MBSectionHeader(title: title, kicker: "Standard OBD-II")
                    Spacer()
                    Text("\(faults.count)")
                        .font(.title2.monospacedDigit().weight(.bold))
                        .foregroundStyle(faults.isEmpty ? MBBrand.success : MBBrand.fault)
                }
                if faults.isEmpty {
                    emptyFaults("None reported")
                } else {
                    ForEach(faults) { fault in
                        VStack(alignment: .leading, spacing: 3) {
                            Text(fault.code)
                                .font(.body.monospaced().weight(.bold))
                                .foregroundStyle(MBBrand.silverBright)
                            Text(fault.title)
                                .font(.subheadline)
                                .foregroundStyle(MBBrand.silver)
                            Text("\(fault.system) · \(fault.category) · \(fault.origin)")
                                .font(.caption)
                                .foregroundStyle(MBBrand.muted)
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

private struct MBLiveDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var scope: MBLiveScope = .available
    @State private var searchText = ""

    private var filtered: [DiagnosticParameter] {
        connection.diagnosticParameters.filter { parameter in
            let scopeMatches: Bool
            switch scope {
            case .available: scopeMatches = parameter.isAvailable
            case .favourites: scopeMatches = parameter.favourite
            case .all: scopeMatches = true
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

                    MBPanel {
                        HStack(spacing: 18) {
                            NavigationLink {
                                MBDataTableView()
                            } label: {
                                Label("Table", systemImage: "tablecells")
                            }
                            NavigationLink {
                                MBGraphsView()
                            } label: {
                                Label("Graphs", systemImage: "chart.xyaxis.line")
                            }
                            NavigationLink {
                                MBDieselView()
                            } label: {
                                Label("Diesel", systemImage: "engine.combustion.fill")
                            }
                        }
                        .font(.subheadline.weight(.semibold))
                        .foregroundStyle(MBBrand.silverBright)
                    }

                    Picker("Show", selection: $scope) {
                        ForEach(MBLiveScope.allCases) { item in
                            Text(LocalizedStringKey(item.rawValue)).tag(item)
                        }
                    }
                    .pickerStyle(.segmented)

                    ForEach(MBParameterGroup.allCases) { group in
                        let parameters = filtered.filter { $0.brandGroup == group }
                        if !parameters.isEmpty {
                            MBPanel {
                                VStack(alignment: .leading, spacing: 6) {
                                    Label(LocalizedStringKey(group.rawValue), systemImage: group.symbol)
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
                            Text(scope == .available
                                 ? "Connect to the vehicle to populate live values."
                                 : "No matching parameters.")
                                .font(.subheadline)
                                .foregroundStyle(MBBrand.silver)
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
                Text(LocalizedStringKey(parameter.title))
                    .font(.subheadline)
                    .foregroundStyle(MBBrand.silverBright)
                Text("\(parameter.shortName) · \(parameter.brandSourceText)")
                    .font(.caption.monospaced())
                    .foregroundStyle(MBBrand.muted)
            }
            Spacer()
            Text(parameter.pollingEnabled ? parameter.formattedValue : "OFF")
                .font(.subheadline.monospacedDigit().weight(.semibold))
                .foregroundStyle(parameter.pollingEnabled && parameter.isAvailable ? MBBrand.silverBright : MBBrand.muted)
            Text("Poll")
                .font(.caption2.weight(.bold))
                .foregroundStyle(MBBrand.muted)
            Toggle("", isOn: Binding(
                get: { parameter.pollingEnabled },
                set: { connection.setPolling($0, stableKey: parameter.id) }
            ))
            .labelsHidden()
            .tint(MBBrand.silverBright)
            Button {
                connection.toggleFavourite(stableKey: parameter.id)
            } label: {
                Image(systemName: parameter.favourite ? "star.fill" : "star")
                    .foregroundStyle(parameter.favourite ? MBBrand.silverBright : MBBrand.muted)
            }
            .buttonStyle(.plain)
        }
        .padding(.vertical, 6)
    }
}

private struct MBDataTableView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.showUnavailableParameters") private var showUnavailableParameters = true

    private var sorted: [DiagnosticParameter] {
        connection.diagnosticParameters
            .filter { showUnavailableParameters || $0.isAvailable }
            .sorted { $0.title < $1.title }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                MBPanel {
                    VStack(spacing: 0) {
                        ForEach(sorted) { parameter in
                            HStack(spacing: 10) {
                                Text(parameter.brandPidText)
                                    .font(.caption.monospaced().weight(.bold))
                                    .foregroundStyle(MBBrand.silver)
                                    .frame(width: 48, alignment: .leading)
                                Text(LocalizedStringKey(parameter.title))
                                    .font(.subheadline)
                                    .foregroundStyle(MBBrand.silverBright)
                                Spacer()
                                Text(parameter.pollingEnabled ? parameter.formattedValue : "OFF")
                                    .font(.subheadline.monospacedDigit().weight(.semibold))
                                    .foregroundStyle(parameter.pollingEnabled && parameter.isAvailable ? MBBrand.silverBright : MBBrand.muted)
                            }
                            .padding(.vertical, 9)
                            if parameter.id != sorted.last?.id { Divider().overlay(MBBrand.line) }
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
        "obd2.engine.rpm", "obd2.vehicle.speed", "obd2.engine.coolant",
        "obd2.diesel.rail_pressure", "obd2.fuel.tank_level",
        "obd2.dpf.bank1_delta_pressure", "obd2.aftertreatment.egt_b1s1"
    ]

    private var displayed: [DiagnosticParameter] {
        let available = connection.diagnosticParameters.filter { $0.pollingEnabled && $0.isAvailable }
        let favourites = available.filter(\.favourite)
        if preferFavouriteSignals && !favourites.isEmpty { return Array(favourites.prefix(8)) }
        let preferred = defaultKeys.compactMap { key in available.first { $0.id == key } }
        return preferred.isEmpty ? Array(available.prefix(6)) : preferred
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    if displayed.isEmpty {
                        MBPanel {
                            Text("Connect to the vehicle to populate dashboard measurements.")
                                .font(.subheadline)
                                .foregroundStyle(MBBrand.silver)
                        }
                    } else {
                        LazyVGrid(columns: mbDashboardColumns, spacing: 12) {
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

private struct MBDieselView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "OM651 powertrain", kicker: "Diesel diagnostics")
                    ForEach(MBParameterGroup.allCases) { group in
                        let parameters = connection.diagnosticParameters.filter { $0.brandGroup == group }
                        if !parameters.isEmpty {
                            VStack(alignment: .leading, spacing: 10) {
                                Label(LocalizedStringKey(group.rawValue), systemImage: group.symbol)
                                    .font(.headline)
                                    .foregroundStyle(MBBrand.silverBright)
                                LazyVGrid(columns: mbDashboardColumns, spacing: 10) {
                                    ForEach(parameters) { parameter in
                                        MBMetricTile(parameter: parameter)
                                    }
                                }
                            }
                        }
                    }
                    MBPanel {
                        VStack(alignment: .leading, spacing: 10) {
                            MBSectionHeader(title: "Manufacturer targets", kicker: "Evidence-gated")
                            Text("Verified Mercedes / Delphi factory values take priority over equivalent SAE OBD-II values. Unmapped factory targets remain visible but are never polled using guessed DIDs or scaling.")
                                .font(.caption)
                                .foregroundStyle(MBBrand.muted)
                            ForEach(connection.mercedesTargetSignals) { signal in
                                VStack(alignment: .leading, spacing: 3) {
                                    Text(signal.title)
                                        .font(.subheadline.weight(.semibold))
                                        .foregroundStyle(MBBrand.silverBright)
                                    Text("Mercedes / Delphi · \(signal.category.uppercased())")
                                        .font(.caption2.monospaced().weight(.bold))
                                        .foregroundStyle(MBBrand.silver)
                                    Text(signal.status.uppercased())
                                        .font(.caption2.monospaced().weight(.bold))
                                        .foregroundStyle(MBBrand.warning)
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

private struct MBGraphsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var graphed: [DiagnosticParameter] {
        let withHistory = connection.diagnosticParameters.filter { $0.pollingEnabled && !$0.history.isEmpty }
        let favourites = withHistory.filter(\.favourite)
        return Array((favourites.isEmpty ? withHistory : favourites).prefix(4))
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    if graphed.isEmpty {
                        MBPanel {
                            Text("Connect to the vehicle and collect samples to populate graphs.")
                                .font(.subheadline)
                                .foregroundStyle(MBBrand.silver)
                        }
                    } else {
                        ForEach(graphed) { parameter in
                            MBPanel {
                                VStack(alignment: .leading, spacing: 10) {
                                    HStack {
                                        Text(LocalizedStringKey(parameter.title))
                                            .font(.headline)
                                            .foregroundStyle(MBBrand.silverBright)
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
                                        .frame(height: 180)
                                    }
                                }
                            }
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Graphs")
    }
}

private struct MBEvidenceView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "Engine endpoint", value: connection.mercedesProbeEndpointText)
                            MBInfoRow(label: "Mercedes probe", value: connection.mercedesProbeStatusText)
                            MBInfoRow(label: "VIN", value: connection.mercedesVINText, monospaced: true)
                            MBInfoRow(label: "Identity", value: connection.mercedesIdentitySummaryText)
                            MBInfoRow(label: "CRD3", value: connection.mercedesCrd3SummaryText)
                            MBInfoRow(label: "UDS faults", value: connection.mercedesUDSFaultStatusText)
                            MBInfoRow(label: "OBD fault scan", value: connection.faultScanStatusText)
                            MBInfoRow(label: "Recorded samples", value: "\(connection.recordedSampleCount)")
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            Button {
                                connection.prepareCSVExport()
                            } label: {
                                HStack(spacing: 8) {
                                    if connection.isPreparingCSV {
                                        ProgressView()
                                            .controlSize(.small)
                                    }
                                    Label("Prepare diagnostic evidence CSV",
                                          systemImage: "doc.badge.gearshape")
                                }
                                .font(.subheadline.weight(.semibold))
                                .foregroundStyle(MBBrand.silverBright)
                            }
                            .disabled(connection.isPreparingCSV)
                            if let exportURL = connection.csvExportURL {
                                ShareLink(item: exportURL) {
                                    Label("Share diagnostic evidence", systemImage: "square.and.arrow.up")
                                        .font(.subheadline.weight(.semibold))
                                        .foregroundStyle(MBBrand.silverBright)
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
    @AppStorage("mblink.language") private var language = "en"
    @AppStorage("mblink.units") private var units = MBLINKUnitProfile.metric.rawValue

    private var version: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "Unknown"
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "Adapter", value: connection.peripheralName)
                            MBInfoRow(label: "Identity", value: connection.adapterIdentifier, monospaced: true)
                            MBInfoRow(label: "Status", value: connection.statusText)
                            MBInfoRow(label: "Version", value: version, monospaced: true)
                            MBInfoRow(label: "Bundle", value: Bundle.main.bundleIdentifier ?? "Unknown", monospaced: true)
                        }
                    }
                    MBPanel {
                        NavigationLink {
                            MBLanguageSelectionView(selection: $language)
                        } label: {
                            HStack(spacing: 12) {
                                Image(systemName: "globe")
                                    .font(.title3)
                                    .foregroundStyle(MBBrand.silverBright)
                                Text("Language")
                                    .font(.headline)
                                    .foregroundStyle(MBBrand.silverBright)
                                Spacer()
                                Text(MBInterfaceLanguage.displayName(for: language))
                                    .font(.subheadline)
                                    .foregroundStyle(MBBrand.silver)
                                Image(systemName: "chevron.right")
                                    .font(.caption.weight(.bold))
                                    .foregroundStyle(MBBrand.muted)
                            }
                            .contentShape(Rectangle())
                        }
                        .buttonStyle(.plain)
                    }
                    MBPanel {
                        HStack(spacing: 12) {
                            Image(systemName: "ruler")
                                .font(.title3)
                                .foregroundStyle(MBBrand.silverBright)
                            Text("Unit system")
                                .font(.headline)
                                .foregroundStyle(MBBrand.silverBright)
                            Spacer()
                            Picker("Unit system", selection: $units) {
                                ForEach(MBLINKUnitProfile.allCases) { profile in
                                    Text(LocalizedStringKey(profile.displayName)).tag(profile.rawValue)
                                }
                            }
                            .pickerStyle(.menu)
                            .tint(MBBrand.silverBright)
                        }
                    }
                    MBPanel {
                        VStack(alignment: .leading, spacing: 14) {
                            Toggle("Prefer favourites on Dashboard and Graphs", isOn: $preferFavouriteSignals)
                                .tint(MBBrand.silverBright)
                                .foregroundStyle(MBBrand.silverBright)
                            Divider().overlay(MBBrand.line)
                            Toggle("Show unavailable values in Data Table", isOn: $showUnavailableParameters)
                                .tint(MBBrand.silverBright)
                                .foregroundStyle(MBBrand.silverBright)
                        }
                    }
                }
                .padding(16)
            }
        }
        .onChange(of: units) { _, _ in connection.refreshPresentation() }
        .mbDiagnosticScreen("Settings")
    }
}

private struct MBLanguageSelectionView: View {
    @Binding var selection: String

    var body: some View {
        ZStack {
            MBBackground()
            List {
                ForEach(MBInterfaceLanguage.all) { item in
                    Button {
                        selection = item.id
                    } label: {
                        HStack(spacing: 12) {
                            Text(item.nativeName)
                                .font(.body.weight(.medium))
                                .foregroundStyle(MBBrand.silverBright)
                            Spacer()
                            if MBInterfaceLanguage.canonical(selection) == item.id {
                                Image(systemName: "checkmark")
                                    .font(.body.weight(.bold))
                                    .foregroundStyle(MBBrand.silverBright)
                            }
                        }
                        .contentShape(Rectangle())
                    }
                    .buttonStyle(.plain)
                    .listRowBackground(MBBrand.panel)
                }
            }
            .scrollContentBackground(.hidden)
        }
        .mbDiagnosticScreen("Language")
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
                            Text("MERCEDES DIAGNOSTICS")
                                .font(.caption2.weight(.bold))
                                .tracking(1.7)
                                .foregroundStyle(MBBrand.silver)
                        }
                        Text("Version \(version)")
                            .font(.subheadline.monospaced())
                            .foregroundStyle(MBBrand.muted)
                        Text("A C-first, open-source Mercedes vehicle diagnostics platform authored by Shannon Smith.")
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
                    Button("Credits") { detail = .credits }
                        .buttonStyle(.bordered)
                    Button("License") { detail = .license }
                        .buttonStyle(.bordered)
                    Button("Close") { onClose() }
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
            NavigationStack {
                ZStack {
                    MBBackground()
                    MBPanel {
                        Text(item == .credits
                             ? "Shannon Smith — Author and project maintainer"
                             : "MBLINK is free software licensed under GPL-3.0-or-later. See LICENSE in the source package for the complete licence text.")
                            .foregroundStyle(MBBrand.silverBright)
                    }
                    .padding(16)
                }
                .navigationTitle(item == .credits ? "Credits" : "License")
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
