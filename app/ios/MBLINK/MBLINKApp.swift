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


private enum MBTypography {
    static let uiRegularName = "MBCorpoSTitleWEB-Regular"
    static let uiBoldName = "MBCorpoSTitleWEB-Bold"
    static let displayName = "MBCorpoATitleCondWEB-Regular"

    static func regular(_ size: CGFloat, relativeTo style: Font.TextStyle) -> Font {
        .custom(uiRegularName, size: size, relativeTo: style)
    }

    static func bold(_ size: CGFloat, relativeTo style: Font.TextStyle) -> Font {
        .custom(uiBoldName, size: size, relativeTo: style)
    }

    static func display(_ size: CGFloat, relativeTo style: Font.TextStyle) -> Font {
        .custom(displayName, size: size, relativeTo: style)
    }

    static let body = regular(17, relativeTo: .body)
    static let bodyBold = bold(17, relativeTo: .body)
    static let subheadline = regular(15, relativeTo: .subheadline)
    static let subheadlineBold = bold(15, relativeTo: .subheadline)
    static let headline = bold(17, relativeTo: .headline)
    static let caption = regular(12, relativeTo: .caption)
    static let captionBold = bold(12, relativeTo: .caption)
    static let caption2 = regular(11, relativeTo: .caption2)
    static let caption2Bold = bold(11, relativeTo: .caption2)
    static let title3 = bold(20, relativeTo: .title3)
    static let title2 = bold(22, relativeTo: .title2)

    static func verifyBundledFonts() {
        precondition(
            UIFont(name: uiRegularName, size: 12) != nil &&
            UIFont(name: uiBoldName, size: 12) != nil &&
            UIFont(name: displayName, size: 12) != nil,
            "MBLINK bundled MB Corpo font set is missing or not registered"
        )
    }
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
                .font(MBTypography.caption2Bold)
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
                    .font(MBTypography.caption2Bold)
                    .tracking(1.4)
                    .foregroundStyle(MBBrand.muted)
            }
            Text(LocalizedStringKey(title))
                .font(MBTypography.title3)
                .foregroundStyle(MBBrand.silverBright)
        }
    }
}

private struct MBInfoRow: View {
    @Environment(\.horizontalSizeClass) private var horizontalSizeClass

    let label: String
    let value: String
    var monospaced = false

    private var valueText: some View {
        Text(LocalizedStringKey(value))
            .font(monospaced ? MBTypography.subheadline : MBTypography.subheadlineBold)
            .foregroundStyle(MBBrand.silverBright)
            .fixedSize(horizontal: false, vertical: true)
            .textSelection(.enabled)
    }

    private var compactLabel: some View {
        Text(LocalizedStringKey(label))
            .font(MBTypography.captionBold)
            .foregroundStyle(MBBrand.muted)
            .textCase(.uppercase)
            .tracking(0.45)
    }

    var body: some View {
        Group {
            /*
             * Do not let long Mercedes evidence fight a two-column layout on
             * iPhone.  Compact-width screens always give the value the full
             * panel width; regular-width screens retain the denser row form.
             */
            if horizontalSizeClass == .compact {
                VStack(alignment: .leading, spacing: 5) {
                    compactLabel
                    valueText
                        .multilineTextAlignment(.leading)
                        .frame(maxWidth: .infinity, alignment: .leading)
                }
            } else {
                HStack(alignment: .firstTextBaseline, spacing: 14) {
                    Text(LocalizedStringKey(label))
                        .font(MBTypography.subheadline)
                        .foregroundStyle(MBBrand.muted)
                        .fixedSize(horizontal: true, vertical: false)
                    Spacer(minLength: 16)
                    valueText
                        .multilineTextAlignment(.trailing)
                        .frame(maxWidth: 420, alignment: .trailing)
                }
            }
        }
        .padding(.vertical, 6)
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
                .font(MBTypography.caption2Bold)
                .tracking(0.8)
                .foregroundStyle(MBBrand.muted)
            Text(fact.value)
                .font(fact.monospaced ? MBTypography.subheadline : MBTypography.subheadlineBold)
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

private struct MBCompactLink<Destination: View>: View {
    let title: String
    let subtitle: String
    let symbol: String
    let destination: () -> Destination

    init(_ title: String, _ subtitle: String, _ symbol: String,
         @ViewBuilder destination: @escaping () -> Destination) {
        self.title = title
        self.subtitle = subtitle
        self.symbol = symbol
        self.destination = destination
    }

    var body: some View {
        NavigationLink { destination() } label: {
            HStack(spacing: 12) {
                Image(systemName: symbol)
                    .font(MBTypography.title3)
                    .foregroundStyle(MBBrand.silverBright)
                    .frame(width: 28)
                VStack(alignment: .leading, spacing: 2) {
                    Text(LocalizedStringKey(title))
                        .font(MBTypography.subheadlineBold)
                        .foregroundStyle(MBBrand.silverBright)
                    Text(LocalizedStringKey(subtitle))
                        .font(MBTypography.caption)
                        .foregroundStyle(MBBrand.muted)
                        .lineLimit(1)
                }
                Spacer(minLength: 10)
                Image(systemName: "chevron.right")
                    .font(MBTypography.captionBold)
                    .foregroundStyle(MBBrand.muted)
            }
            .contentShape(Rectangle())
            .padding(.vertical, 6)
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
                .font(MBTypography.bold(24, relativeTo: .title2))
                .foregroundStyle(MBBrand.silverBright)
                .frame(width: 30, height: 30, alignment: .leading)

            Text(LocalizedStringKey(title))
                .font(MBTypography.headline)
                .foregroundStyle(MBBrand.silverBright)
                .lineLimit(1)

            Text(LocalizedStringKey(subtitle))
                .font(MBTypography.caption)
                .foregroundStyle(MBBrand.muted)
                .lineLimit(2)
                .fixedSize(horizontal: false, vertical: true)

            Spacer(minLength: 0)

            HStack {
                Spacer()
                Image(systemName: "chevron.right")
                    .font(MBTypography.captionBold)
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
                    .font(MBTypography.caption2Bold)
                    .tracking(0.7)
                    .foregroundStyle(MBBrand.silver)
                Spacer()
                Text(parameter.brandPidText)
                    .font(MBTypography.caption2)
                    .foregroundStyle(MBBrand.muted)
            }

            Text(parameter.presentationValue)
                .font(MBTypography.bold(24, relativeTo: .title2))
                .monospacedDigit()
                .foregroundStyle(parameter.hasLiveValue ? MBBrand.silverBright : MBBrand.muted)
                .minimumScaleFactor(0.65)
                .lineLimit(1)

            Text(LocalizedStringKey(parameter.title))
                .font(MBTypography.caption)
                .foregroundStyle(MBBrand.muted)
                .lineLimit(2)
            if let source = parameter.sourceLabel {
                Label(source, systemImage: "cpu")
                    .font(MBTypography.caption2Bold)
                    .foregroundStyle(MBBrand.silver)
                    .lineLimit(2)
            }
            if let qualityNote = parameter.qualityNote {
                Text(qualityNote)
                    .font(MBTypography.caption2)
                    .foregroundStyle(MBBrand.warning)
                    .lineLimit(2)
            }
        }
        .frame(maxWidth: .infinity,
               minHeight: parameter.sourceLabel == nil ? 112 : 142,
               alignment: .leading)
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
        protocolName.lowercased() == "obd2" ? "SAE OBD-II · \(brandPidText)" :
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

    init() {
        MBTypography.verifyBundledFonts()
    }

    var body: some Scene {
        WindowGroup {
            MBCommandCentreView()
                .environmentObject(connection)
                .environment(\.font, MBTypography.body)
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
                                .font(MBTypography.captionBold)
                                .tracking(1.0)
                            Text("© 2026 Shannon Smith")
                                .foregroundStyle(MBBrand.muted)
                            Spacer()
                            Label("About", systemImage: "info.circle")
                        }
                        .font(MBTypography.caption)
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
                        if connection.isActive && !connection.isReady { connectionProgress }
                        connectionCard
                        MBSectionHeader(title: "Diagnostics", kicker: "Vehicle")
                        primaryGrid
                        supportingTools
                    }
                    .padding(.horizontal, 20)
                    .padding(.top, 18)
                    .padding(.bottom, 30)
                }
            }
            .toolbar(.hidden, for: .navigationBar)
            .alert("Adapter transport unavailable",
                   isPresented: Binding(
                       get: { connection.connectionAlertText != nil },
                       set: { visible in if !visible { connection.dismissConnectionAlert() } })) {
                Button("OK") { connection.dismissConnectionAlert() }
            } message: {
                Text(connection.connectionAlertText ?? "")
            }
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
                    .font(MBTypography.display(29, relativeTo: .title))
                    .tracking(1.6)
                    .foregroundStyle(MBBrand.silverBright)
                Text("MERCEDES-BENZ DIAGNOSTICS")
                    .font(MBTypography.caption2Bold)
                    .tracking(1.4)
                    .foregroundStyle(MBBrand.silver)
            }
        }
    }

    private var connectionCard: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 12) {
                HStack(alignment: .firstTextBaseline) {
                    VStack(alignment: .leading, spacing: 3) {
                        Text(connection.isActive ? "Diagnostic session" : "Vehicle connection")
                            .font(MBTypography.headline)
                            .foregroundStyle(MBBrand.silverBright)
                        Text(connection.statusText)
                            .font(MBTypography.caption)
                            .foregroundStyle(MBBrand.muted)
                            .fixedSize(horizontal: false, vertical: true)
                    }
                    Spacer(minLength: 12)
                    Image(systemName: connection.isReady
                          ? "checkmark.circle.fill"
                          : connection.isActive ? "dot.radiowaves.left.and.right" : "cable.connector")
                        .foregroundStyle(connection.isReady ? MBBrand.success : MBBrand.silver)
                }

                Button {
                    connection.isActive ? connection.disconnect() : connection.connect()
                } label: {
                    Label(connection.isActive ? "Disconnect" : "Connect to vehicle",
                          systemImage: connection.isActive ? "cable.connector.slash" : "cable.connector")
                        .font(MBTypography.subheadlineBold)
                        .foregroundStyle(MBBrand.background)
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 11)
                        .background(RoundedRectangle(cornerRadius: 12, style: .continuous)
                            .fill(MBBrand.silverBright))
                }
                .buttonStyle(.plain)

                if let identity = connection.vehicleIdentity {
                    HStack(spacing: 9) {
                        Image(systemName: "car.side.fill").foregroundStyle(MBBrand.silver)
                        VStack(alignment: .leading, spacing: 2) {
                            Text(identity.model ?? identity.manufacturer)
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
                            Text(identity.vin)
                                .font(MBTypography.caption2)
                                .foregroundStyle(MBBrand.muted)
                                .lineLimit(1)
                        }
                    }
                } else if !connection.isActive {
                    Text("Connect once to identify the vehicle, faults, modules and supported live data.")
                        .font(MBTypography.caption)
                        .foregroundStyle(MBBrand.muted)
                }
            }
        }
    }

    private var primaryGrid: some View {
        LazyVGrid(columns: mbDashboardColumns, spacing: 14) {
            MBHomeTile("Faults", "Stored and active diagnostic faults", "exclamationmark.triangle.fill") { MBFaultsView() }
            MBHomeTile("Live Data", "Sensors and measurements", "waveform.path.ecg") { MBLiveDataView() }
            MBHomeTile("Vehicle", "VIN and decoded identity", "car.side.fill") { MBVehicleView() }
            MBHomeTile("Modules", "Control units and capabilities", "square.stack.3d.up.fill") { MBModulesView() }
        }
    }

    private var supportingTools: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 7) {
                MBSectionHeader(title: "Tools", kicker: "Secondary")
                MBCompactLink("Dashboard", "At-a-glance live measurements", "gauge.with.dots.needle.67percent") { MBDashboardView() }
                Divider().overlay(MBBrand.line)
                MBCompactLink("Evidence", "Session log and CSV export", "doc.text.magnifyingglass") { MBEvidenceView() }
                Divider().overlay(MBBrand.line)
                MBCompactLink("Settings", "Adapter, units and app preferences", "gearshape.fill") { MBSettingsView() }
            }
        }
    }

    private var connectionProgress: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 7) {
                Label(connection.connectionPhaseTitle, systemImage: "dot.radiowaves.left.and.right")
                    .font(MBTypography.headline)
                    .foregroundStyle(MBBrand.silverBright)
                Text(connection.statusText)
                    .font(MBTypography.subheadline)
                    .foregroundStyle(MBBrand.silver)
                    .fixedSize(horizontal: false, vertical: true)
                if !connection.diagnosticModules.isEmpty {
                    Text("\(connection.diagnosticModules.count) responding control units retained so far")
                        .font(MBTypography.caption)
                        .foregroundStyle(MBBrand.muted)
                }
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
    @State private var technicalDetailsExpanded = false

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
                    vehicleHero
                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            MBSectionHeader(title: "Vehicle", kicker: "Decoded VIN")
                            if identityFacts.isEmpty {
                                Text("Decoded Mercedes vehicle details will appear here after VIN identification.")
                                    .font(MBTypography.subheadline)
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
                        DisclosureGroup(isExpanded: $technicalDetailsExpanded) {
                            VStack(spacing: 4) {
                                MBInfoRow(label: "Connection", value: connection.statusText)
                                MBInfoRow(label: "Vehicle profile", value: connection.vehicleProfileStatusText)
                                MBInfoRow(label: "Fault records", value: "\(totalFaultCount)")
                                MBInfoRow(label: "Endpoint", value: connection.mercedesProbeEndpointText)
                                MBInfoRow(label: "CRD3 identity", value: connection.mercedesCrd3SummaryText)
                                MBInfoRow(label: "Identity sweep", value: connection.mercedesIdentitySummaryText)
                                MBInfoRow(label: "Probe", value: connection.mercedesProbeStatusText)
                            }
                            .padding(.top, 8)
                        } label: {
                            Label("Diagnostic details", systemImage: "wrench.and.screwdriver")
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
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
                        .font(MBTypography.bold(30, relativeTo: .title))
                        .foregroundStyle(MBBrand.silverBright)
                        .frame(width: 42, height: 42)
                    VStack(alignment: .leading, spacing: 5) {
                        Text(identity.model ?? identity.manufacturer)
                            .font(MBTypography.title2)
                            .foregroundStyle(MBBrand.silverBright)
                        Text([identity.chassis, identity.bodyStyle, identity.engineFamily]
                            .compactMap { $0 }.joined(separator: " · "))
                            .font(MBTypography.subheadlineBold)
                            .foregroundStyle(MBBrand.silver)
                        Text(identity.vin)
                            .font(MBTypography.subheadlineBold)
                            .foregroundStyle(MBBrand.silverBright)
                            .textSelection(.enabled)
                    }
                }
            } else {
                VStack(alignment: .leading, spacing: 6) {
                    Text("Waiting for vehicle VIN")
                        .font(MBTypography.headline)
                        .foregroundStyle(MBBrand.silverBright)
                    Text(connection.mercedesVINText)
                        .font(MBTypography.subheadline)
                        .foregroundStyle(MBBrand.muted)
                }
            }
        }
    }
}

private struct MBModulesView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var technicalDetailsExpanded = false

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Control units",
                                    kicker: "\(connection.diagnosticModules.count) responding")

                    if connection.diagnosticModules.isEmpty {
                        MBPanel {
                            Text(connection.isActive
                                 ? "Mercedes module census is in progress. Responding control units will appear here and remain attached to the VIN profile."
                                 : "Connect to the vehicle to discover its control units.")
                                .font(MBTypography.subheadline)
                                .foregroundStyle(MBBrand.silver)
                        }
                    } else {
                        ForEach(connection.diagnosticModules) { module in
                            NavigationLink { MBModuleDetailView(moduleID: module.id) } label: { moduleCard(module) }
                                .buttonStyle(.plain)
                        }
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 12) {
                            MBSectionHeader(title: "Factory data", kicker: "OM651 target catalogue")
                            Text("\(connection.mercedesTargetSignals.count) evidence-backed manufacturer value identities")
                                .font(MBTypography.subheadline)
                                .foregroundStyle(MBBrand.silver)
                            NavigationLink { MBDieselView() } label: {
                                HStack {
                                    Label("Open factory-data targets", systemImage: "engine.combustion.fill")
                                    Spacer()
                                    Image(systemName: "chevron.right")
                                }
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
                            }
                        }
                    }

                    MBPanel {
                        DisclosureGroup(isExpanded: $technicalDetailsExpanded) {
                            VStack(alignment: .leading, spacing: 10) {
                                MBInfoRow(label: "VIN profile", value: connection.vehicleProfileStatusText)
                                MBInfoRow(label: "Module state", value: connection.mercedesProbeStatusText)
                                Divider().overlay(MBBrand.line)
                                capability("Standard OBD-II engine diagnostics", "waveform.path.ecg")
                                capability("UDS / ISO-TP diagnostic engine", "point.3.connected.trianglepath.dotted")
                                capability("CRD3 ECU identity and fingerprinting", "checkmark.seal.fill")
                                capability("Read-only UDS fault memory", "exclamationmark.triangle.fill")
                            }
                            .padding(.top, 8)
                        } label: {
                            Label("Scan and capability details", systemImage: "info.circle")
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Modules")
    }

    private func moduleCard(_ module: DiagnosticModule) -> some View {
        HStack(alignment: .top, spacing: 13) {
            Image(systemName: module.symbol).font(MBTypography.title2).foregroundStyle(MBBrand.silverBright).frame(width: 34, height: 34)
            VStack(alignment: .leading, spacing: 5) {
                Text(module.name).font(MBTypography.headline).foregroundStyle(MBBrand.silverBright).multilineTextAlignment(.leading)
                if !module.designation.isEmpty {
                    Text(module.designation).font(MBTypography.captionBold).foregroundStyle(MBBrand.silver)
                }
                Text("\(module.addressText) · \(module.protocolName)").font(MBTypography.caption2).foregroundStyle(MBBrand.muted)
                HStack(spacing: 8) {
                    Label("\(module.livePIDCount) selectable OBD values", systemImage: "waveform.path.ecg")
                    Label(module.faultCountLabel, systemImage: "exclamationmark.triangle")
                }
                .font(MBTypography.caption2Bold)
                .foregroundStyle(MBBrand.silver)
            }
            Spacer(minLength: 4)
            Image(systemName: "chevron.right").foregroundStyle(MBBrand.muted).padding(.top, 8)
        }
        .padding(15)
        .background(RoundedRectangle(cornerRadius: 17, style: .continuous).fill(MBBrand.panel))
        .overlay(RoundedRectangle(cornerRadius: 17, style: .continuous).stroke(MBBrand.line, lineWidth: 1))
    }

    private func capability(_ text: String, _ symbol: String) -> some View {
        HStack(spacing: 11) {
            Image(systemName: symbol).frame(width: 24).foregroundStyle(MBBrand.silverBright)
            Text(text).font(MBTypography.subheadline).foregroundStyle(MBBrand.silver)
            Spacer()
        }
    }
}

private extension DiagnosticModule {
    var symbol: String {
        let value = kind.lowercased()
        if value.contains("engine") { return "engine.combustion.fill" }
        if value.contains("transmission") { return "gearshape.2.fill" }
        if value.contains("abs") || value.contains("esp") { return "car.side.rear.and.collision.and.car.side.front" }
        if value.contains("restraint") { return "airbag.fill" }
        if value.contains("climate") { return "fan.fill" }
        if value.contains("instrument") { return "gauge.with.dots.needle.67percent" }
        if value.contains("body") { return "car.fill" }
        return "cpu.fill"
    }
}

private struct MBModuleDetailView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    let moduleID: String

    private var module: DiagnosticModule? {
        connection.diagnosticModule(id: moduleID)
    }

    private var parameters: [DiagnosticParameter] {
        connection.moduleParameters(moduleID: moduleID).filter(\.isAvailable)
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                if let module {
                    VStack(alignment: .leading, spacing: 15) {
                        MBSectionHeader(title: module.name, kicker: module.kind.uppercased())

                        MBPanel {
                            VStack(spacing: 4) {
                                MBInfoRow(label: "CAN route", value: module.addressText)
                                MBInfoRow(label: "Protocol", value: module.protocolName)
                                if !module.designation.isEmpty {
                                    MBInfoRow(label: "Designation", value: module.designation)
                                }
                                if !module.network.isEmpty {
                                    MBInfoRow(label: "Network", value: module.network)
                                }
                                if let identity = module.identityText {
                                    MBInfoRow(label: "Identity", value: identity)
                                }
                                if let part = module.partNumber {
                                    MBInfoRow(label: "Part number", value: part)
                                }
                                if let software = module.softwareNumber {
                                    MBInfoRow(label: "Software", value: software)
                                }
                                if let hardware = module.hardwareNumber {
                                    MBInfoRow(label: "Hardware", value: hardware)
                                }
                            }
                        }

                        if !module.evidenceDetails.isEmpty {
                            MBSectionHeader(
                                title: "ECU evidence",
                                kicker: "Captured read-only identity")
                            MBPanel {
                                VStack(alignment: .leading, spacing: 8) {
                                    ForEach(module.evidenceDetails, id: \.self) { detail in
                                        Text(detail)
                                            .font(MBTypography.caption)
                                            .foregroundStyle(MBBrand.silverBright)
                                            .textSelection(.enabled)
                                    }
                                }
                            }
                        }

                        MBSectionHeader(
                            title: "Live data",
                            kicker: "\(parameters.count) values from response \(module.responseAddressText)")
                        if parameters.isEmpty {
                            MBPanel {
                                Text("This control unit is responding, but no source-attributed live parameter has been captured from it yet. Identity and fault evidence remain available without assigning guessed DIDs.")
                                    .font(MBTypography.subheadline)
                                    .foregroundStyle(MBBrand.silver)
                            }
                        } else {
                            ForEach(MBParameterGroup.allCases) { group in
                                let grouped = parameters.filter { $0.brandGroup == group }
                                if !grouped.isEmpty {
                                    VStack(alignment: .leading, spacing: 10) {
                                        Label(LocalizedStringKey(group.rawValue), systemImage: group.symbol)
                                            .font(MBTypography.headline)
                                            .foregroundStyle(MBBrand.silverBright)
                                        LazyVGrid(columns: mbDashboardColumns, spacing: 10) {
                                            ForEach(grouped) { parameter in
                                                MBMetricTile(parameter: parameter)
                                            }
                                        }
                                    }
                                }
                            }
                            MBPanel {
                                Text("These are standard Mode 01 values returned by this exact CAN responder. They are kept separate from the same PID returned by another module.")
                                    .font(MBTypography.caption)
                                    .foregroundStyle(MBBrand.muted)
                            }
                            MBPanel {
                                Text("State-dependent values are shown exactly as returned. MBLINK does not smooth, substitute or silently correct shutdown and stale ECU readings.")
                                    .font(MBTypography.caption)
                                    .foregroundStyle(MBBrand.muted)
                            }
                        }

                        MBSectionHeader(title: "Fault memory", kicker: module.faultStatus)
                        MBPanel {
                            if module.faults.isEmpty {
                                Text(module.faultStatus)
                                    .font(MBTypography.subheadline)
                                    .foregroundStyle(MBBrand.silver)
                            } else {
                                VStack(alignment: .leading, spacing: 8) {
                                    ForEach(module.faults, id: \.self) { fault in
                                        Text(fault)
                                            .font(MBTypography.caption)
                                            .foregroundStyle(MBBrand.silverBright)
                                            .textSelection(.enabled)
                                    }
                                }
                            }
                        }
                    }
                    .padding(16)
                } else {
                    MBPanel {
                        Text("The module is no longer present in the active vehicle profile.")
                            .foregroundStyle(MBBrand.silver)
                    }
                    .padding(16)
                }
            }
        }
        .mbDiagnosticScreen(module?.name ?? "Module")
    }
}

private extension DiagnosticModule {
    var responseAddressText: String {
        extendedID
            ? String(format: "0x%08X", responseCANIdentifier)
            : String(format: "0x%03X", responseCANIdentifier)
    }
}

private struct MBFaultsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var contextExpanded = false

    private var standardTotal: Int {
        connection.storedFaults.count + connection.pendingFaults.count + connection.permanentFaults.count
    }
    private var moduleFaultTotal: Int {
        connection.diagnosticModules.reduce(0) { $0 + $1.faultCount }
    }
    private var total: Int { moduleFaultTotal + standardTotal }
    private var sortedModules: [DiagnosticModule] {
        connection.diagnosticModules.sorted {
            if $0.faultCount != $1.faultCount { return $0.faultCount > $1.faultCount }
            if $0.requestCANIdentifier != $1.requestCANIdentifier {
                return $0.requestCANIdentifier < $1.requestCANIdentifier
            }
            return $0.name < $1.name
        }
    }
    private var allScansComplete: Bool {
        connection.obdFaultScanComplete && connection.mercedesFaultScanComplete
    }
    private var headlineColour: Color {
        if total > 0 { return MBBrand.fault }
        return allScansComplete ? MBBrand.success : MBBrand.warning
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    HStack {
                        MBSectionHeader(title: "Faults", kicker: "Mercedes-Benz diagnostic memory")
                        Spacer()
                        Text("\(total)")
                            .font(MBTypography.bold(30, relativeTo: .title))
                            .foregroundStyle(headlineColour)
                    }
                    scanSummaryPanel

                    moduleFaultSection

                    if standardTotal == 0 {
                        if connection.obdFaultScanComplete {
                            MBPanel { statusRow("No standard OBD-II faults reported", symbol: "checkmark.circle.fill", colour: MBBrand.success) }
                        } else if connection.obdFaultScanFailed {
                            MBPanel { statusRow("Standard OBD-II scan failed — no clean result inferred", symbol: "xmark.octagon.fill", colour: MBBrand.fault) }
                        } else {
                            MBPanel { statusRow("Standard OBD-II fault scan has not completed", symbol: "clock.fill", colour: MBBrand.muted) }
                        }
                    } else {
                        if !connection.storedFaults.isEmpty { faultPanel("Stored", faults: connection.storedFaults) }
                        if !connection.pendingFaults.isEmpty { faultPanel("Pending", faults: connection.pendingFaults) }
                        if !connection.permanentFaults.isEmpty { faultPanel("Permanent", faults: connection.permanentFaults) }
                    }
                    diagnosticContextPanel
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Faults")
    }

    private var scanSummaryPanel: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 12) {
                MBSectionHeader(title: "Scan status", kicker: "Evidence state")
                scanSummaryRow(title: "Mercedes modules",
                               detail: connection.mercedesUDSFaultStatusText,
                               count: "\(connection.diagnosticModules.count) modules · \(moduleFaultTotal) faults",
                               complete: connection.mercedesFaultScanComplete,
                               failed: connection.mercedesFaultScanFailed)
                Divider().overlay(MBBrand.line)
                scanSummaryRow(title: "Standard OBD-II",
                               detail: connection.faultScanStatusText,
                               count: "\(connection.storedFaults.count) stored · \(connection.pendingFaults.count) pending · \(connection.permanentFaults.count) permanent",
                               complete: connection.obdFaultScanComplete,
                               failed: connection.obdFaultScanFailed)
            }
        }
    }

    private func scanSummaryRow(title: String, detail: String, count: String, complete: Bool, failed: Bool) -> some View {
        HStack(alignment: .top, spacing: 11) {
            Image(systemName: failed ? "xmark.octagon.fill" : complete ? "checkmark.circle.fill" : "clock.fill")
                .foregroundStyle(failed ? MBBrand.fault : complete ? MBBrand.success : MBBrand.warning)
                .frame(width: 22)
            VStack(alignment: .leading, spacing: 3) {
                Text(title).font(MBTypography.subheadlineBold).foregroundStyle(MBBrand.silverBright)
                Text(count).font(MBTypography.caption).foregroundStyle(MBBrand.silver)
                Text(detail).font(MBTypography.caption).foregroundStyle(MBBrand.muted).fixedSize(horizontal: false, vertical: true)
            }
            Spacer(minLength: 0)
        }
    }

    @ViewBuilder
    private var moduleFaultSection: some View {
        if sortedModules.isEmpty {
            MBPanel {
                statusRow(
                    connection.isActive
                        ? "Mercedes control-unit census is still in progress"
                        : "Connect to the vehicle to read control-unit fault memory",
                    symbol: connection.isActive ? "clock.fill" : "cable.connector",
                    colour: MBBrand.muted)
            }
        } else {
            VStack(alignment: .leading, spacing: 10) {
                MBSectionHeader(
                    title: "Control-unit fault memory",
                    kicker: "Every responding ECU kept separate")
                ForEach(sortedModules) { module in
                    moduleFaultCard(module)
                }
            }
        }
    }

    private func moduleFaultCard(_ module: DiagnosticModule) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 9) {
                HStack(alignment: .top, spacing: 10) {
                    Image(systemName: module.symbol)
                        .font(MBTypography.title3)
                        .foregroundStyle(moduleFaultColour(module))
                        .frame(width: 26)
                    VStack(alignment: .leading, spacing: 3) {
                        Text(module.name)
                            .font(MBTypography.headline)
                            .foregroundStyle(MBBrand.silverBright)
                        if !module.designation.isEmpty {
                            Text(module.designation)
                                .font(MBTypography.captionBold)
                                .foregroundStyle(MBBrand.silver)
                        }
                        Text("\(module.addressText) · \(module.protocolName)")
                            .font(MBTypography.caption2)
                            .foregroundStyle(MBBrand.muted)
                    }
                    Spacer(minLength: 8)
                    Text(module.faultCountLabel.uppercased())
                        .font(MBTypography.caption2Bold)
                        .foregroundStyle(moduleFaultColour(module))
                }

                Divider().overlay(MBBrand.line)

                if module.faults.isEmpty {
                    Label(module.faultStatus,
                          systemImage: module.faultStatus == "Checked · no faults"
                              ? "checkmark.circle.fill" : "questionmark.circle.fill")
                        .font(MBTypography.subheadline)
                        .foregroundStyle(moduleFaultColour(module))
                        .fixedSize(horizontal: false, vertical: true)
                } else {
                    ForEach(module.faults, id: \.self) { fault in
                        Text(conciseModuleFault(fault, module: module))
                            .font(MBTypography.subheadlineBold)
                            .foregroundStyle(MBBrand.silverBright)
                            .fixedSize(horizontal: false, vertical: true)
                            .textSelection(.enabled)
                    }
                }

                NavigationLink {
                    MBModuleDetailView(moduleID: module.id)
                } label: {
                    HStack {
                        Text("Open \(module.name)")
                        Spacer()
                        Image(systemName: "chevron.right")
                    }
                    .font(MBTypography.captionBold)
                    .foregroundStyle(MBBrand.silver)
                }
                .buttonStyle(.plain)
            }
        }
    }

    private func conciseModuleFault(_ fault: String, module: DiagnosticModule) -> String {
        let prefix = "\(module.name) · \(module.addressText) · "
        guard fault.hasPrefix(prefix) else { return fault }
        return String(fault.dropFirst(prefix.count))
    }

    private func moduleFaultColour(_ module: DiagnosticModule) -> Color {
        if module.faultCount > 0 { return MBBrand.fault }
        if module.faultStatus == "Checked · no faults" { return MBBrand.success }
        return MBBrand.warning
    }

    private var diagnosticContextPanel: some View {
        MBPanel {
            DisclosureGroup(isExpanded: $contextExpanded) {
                VStack(alignment: .leading, spacing: 12) {
                    VStack(alignment: .leading, spacing: 6) {
                        Label("Emissions readiness", systemImage: "checklist").font(MBTypography.subheadlineBold).foregroundStyle(MBBrand.silverBright)
                        Text(connection.readinessStatusText).font(MBTypography.caption).foregroundStyle(MBBrand.silver)
                        ForEach(connection.readinessMonitorStatus, id: \.self) { monitor in
                            Text(monitor).font(MBTypography.caption).foregroundStyle(MBBrand.muted)
                        }
                    }
                    Divider().overlay(MBBrand.line)
                    VStack(alignment: .leading, spacing: 6) {
                        Label("Stored-fault freeze-frame", systemImage: "camera.metering.center.weighted")
                            .font(MBTypography.subheadlineBold).foregroundStyle(MBBrand.silverBright)
                        Text("Mode 02 frame 0 · captured fault context, not current live data").font(MBTypography.caption).foregroundStyle(MBBrand.muted)
                        if !connection.freezeFrameContext.isEmpty {
                            ForEach(connection.freezeFrameContext, id: \.self) { item in
                                Text(item).font(MBTypography.caption).foregroundStyle(MBBrand.silver).textSelection(.enabled)
                            }
                        } else if connection.obdFaultScanComplete && connection.storedFaults.isEmpty {
                            statusRow("Not required — no stored OBD fault was reported", symbol: "checkmark.circle", colour: MBBrand.muted)
                        } else if connection.obdFaultScanComplete {
                            statusRow("No Mode 02 freeze-frame values were available", symbol: "minus.circle", colour: MBBrand.warning)
                        } else {
                            statusRow("Freeze-frame context has not been collected", symbol: "clock", colour: MBBrand.muted)
                        }
                    }
                }
                .padding(.top, 8)
            } label: {
                Label("Readiness and freeze-frame context", systemImage: "waveform.path.ecg.rectangle")
                    .font(MBTypography.subheadlineBold).foregroundStyle(MBBrand.silverBright)
            }
        }
    }

    private func faultPanel(_ title: String, faults: [DiagnosticFault]) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 10) {
                HStack {
                    MBSectionHeader(title: title, kicker: "Standard OBD-II")
                    Spacer()
                    Text("\(faults.count)").font(MBTypography.title2).foregroundStyle(MBBrand.fault)
                }
                ForEach(faults) { fault in
                    diagnosticFaultCard(fault)
                    if fault.id != faults.last?.id { Divider().overlay(MBBrand.line) }
                }
            }
        }
    }

    private func diagnosticFaultCard(_ fault: DiagnosticFault) -> some View {
        VStack(alignment: .leading, spacing: 7) {
            HStack(alignment: .firstTextBaseline) {
                Text(fault.code).font(MBTypography.title3).foregroundStyle(MBBrand.silverBright).textSelection(.enabled)
                Spacer()
                Text(fault.state.uppercased()).font(MBTypography.caption2Bold).foregroundStyle(MBBrand.muted)
            }
            Text(fault.title).font(MBTypography.subheadlineBold).foregroundStyle(MBBrand.silver)
            Text("\(fault.system) · \(fault.category)").font(MBTypography.caption).foregroundStyle(MBBrand.muted)
            Text("\(fault.origin) · \(fault.source)").font(MBTypography.caption).foregroundStyle(MBBrand.muted)
            Label(fault.definitionKnown ? "Definition resolved; raw code preserved" : "Definition unknown; raw code preserved",
                  systemImage: fault.definitionKnown ? "checkmark.seal.fill" : "questionmark.diamond.fill")
                .font(MBTypography.caption).foregroundStyle(fault.definitionKnown ? MBBrand.success : MBBrand.warning)
        }
        .padding(.vertical, 3)
    }

    private func statusRow(_ text: String, symbol: String, colour: Color) -> some View {
        HStack(spacing: 9) {
            Image(systemName: symbol).foregroundStyle(colour)
            Text(text).font(MBTypography.subheadline).foregroundStyle(MBBrand.silver)
            Spacer()
        }
    }
}

private struct MBLiveDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    private var modules: [DiagnosticModule] {
        connection.diagnosticModules.sorted {
            if ($0.livePIDCount > 0) != ($1.livePIDCount > 0) {
                return $0.livePIDCount > 0
            }
            if $0.livePIDCount != $1.livePIDCount {
                return $0.livePIDCount > $1.livePIDCount
            }
            if $0.requestCANIdentifier != $1.requestCANIdentifier {
                return $0.requestCANIdentifier < $1.requestCANIdentifier
            }
            return $0.name < $1.name
        }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(
                        title: "Live data",
                        kicker: "Choose a control unit")

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
                                Label("Factory", systemImage: "engine.combustion.fill")
                            }
                        }
                        .font(MBTypography.subheadlineBold)
                        .foregroundStyle(MBBrand.silverBright)
                        .lineLimit(1)
                        .minimumScaleFactor(0.78)
                    }

                    if modules.isEmpty {
                        MBPanel {
                            Text(connection.isActive
                                 ? "Control-unit discovery is still in progress. Each responding ECU will appear here with its own supported PID list."
                                 : "Connect to the vehicle to discover control units and their supported PIDs.")
                                .font(MBTypography.subheadline)
                                .foregroundStyle(MBBrand.silver)
                                .fixedSize(horizontal: false, vertical: true)
                        }
                    } else {
                        ForEach(modules) { module in
                            NavigationLink {
                                MBModuleLiveDataView(moduleID: module.id)
                            } label: {
                                moduleCard(module)
                            }
                            .buttonStyle(.plain)
                        }

                        MBPanel {
                            Text("PID availability is kept per responding ECU. If two modules answer the same SAE PID, MBLINK keeps their values separate instead of collapsing them into one device.")
                                .font(MBTypography.caption)
                                .foregroundStyle(MBBrand.muted)
                                .fixedSize(horizontal: false, vertical: true)
                        }
                    }
                }
                .padding(16)
            }
        }
        .mbDiagnosticScreen("Live Data")
    }

    private func moduleCard(_ module: DiagnosticModule) -> some View {
        HStack(alignment: .top, spacing: 13) {
            Image(systemName: module.symbol)
                .font(MBTypography.title2)
                .foregroundStyle(MBBrand.silverBright)
                .frame(width: 34, height: 34)

            VStack(alignment: .leading, spacing: 5) {
                Text(module.name)
                    .font(MBTypography.headline)
                    .foregroundStyle(MBBrand.silverBright)
                    .multilineTextAlignment(.leading)

                if !module.designation.isEmpty {
                    Text(module.designation)
                        .font(MBTypography.captionBold)
                        .foregroundStyle(MBBrand.silver)
                }

                Text("\(module.addressText) · \(module.protocolName)")
                    .font(MBTypography.caption2)
                    .foregroundStyle(MBBrand.muted)
                    .fixedSize(horizontal: false, vertical: true)

                let mercedesCount =
                    connection.manufacturerData(moduleID: module.id).count
                HStack(spacing: 8) {
                    Label(
                        mercedesCount > 0
                            ? "\(mercedesCount) Mercedes data ID\(mercedesCount == 1 ? "" : "s")"
                            : "Mercedes data ready to scan",
                        systemImage: "wrench.and.screwdriver")
                    if module.livePIDCount > 0 {
                        Text("·")
                        Text("\(module.livePIDCount) standard live value\(module.livePIDCount == 1 ? "" : "s")")
                    }
                }
                .font(MBTypography.caption2Bold)
                .foregroundStyle(MBBrand.silver)

                if module.obdAdvertisedPIDCount > 0 {
                    Text("\(module.obdAdvertisedPIDCount) Mode 01 capability code\(module.obdAdvertisedPIDCount == 1 ? "" : "s") advertised by this responder")
                        .font(MBTypography.caption2)
                        .foregroundStyle(MBBrand.muted)
                        .fixedSize(horizontal: false, vertical: true)
                }
            }

            Spacer(minLength: 6)
            Image(systemName: "chevron.right")
                .font(MBTypography.captionBold)
                .foregroundStyle(MBBrand.muted)
                .padding(.top, 8)
        }
        .padding(15)
        .background(
            RoundedRectangle(cornerRadius: 17, style: .continuous)
                .fill(MBBrand.panel))
        .overlay(
            RoundedRectangle(cornerRadius: 17, style: .continuous)
                .stroke(MBBrand.line, lineWidth: 1))
    }
}

private struct MBModuleLiveDataView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    let moduleID: String

    @State private var scope: MBLiveScope = .available
    @State private var searchText = ""

    private var module: DiagnosticModule? {
        connection.diagnosticModule(id: moduleID)
    }

    private var manufacturerValues: [MercedesModuleDataValue] {
        connection.manufacturerData(moduleID: moduleID)
    }

    private var scanningThisModule: Bool {
        connection.manufacturerDataScanActive &&
            connection.manufacturerDataScanModuleID == moduleID
    }

    private var allParameters: [DiagnosticParameter] {
        connection.moduleParameters(moduleID: moduleID)
    }

    private var supportedParameters: [DiagnosticParameter] {
        allParameters.filter(\.isSupported)
    }

    private var filteredStandardParameters: [DiagnosticParameter] {
        allParameters.filter { parameter in
            let scopeMatches: Bool
            switch scope {
            case .available:
                scopeMatches = parameter.isSupported
            case .favourites:
                scopeMatches = parameter.isSupported && parameter.favourite
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

    private var filteredManufacturerValues: [MercedesModuleDataValue] {
        guard !searchText.isEmpty else { return manufacturerValues }
        return manufacturerValues.filter {
            $0.title.localizedCaseInsensitiveContains(searchText) ||
            $0.codeText.localizedCaseInsensitiveContains(searchText) ||
            $0.rawHex.localizedCaseInsensitiveContains(searchText)
        }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                if let module {
                    VStack(alignment: .leading, spacing: 15) {
                        MBSectionHeader(
                            title: module.name,
                            kicker: "Mercedes control-unit data")

                        moduleSummary(module)

                        MBSectionHeader(
                            title: "Mercedes manufacturer data",
                            kicker: manufacturerKicker(module))

                        if scanningThisModule {
                            scanProgress
                        }

                        if filteredManufacturerValues.isEmpty {
                            manufacturerEmptyState(module)
                        } else {
                            manufacturerValuesPanel(module)
                        }

                        if !manufacturerValues.isEmpty && !scanningThisModule {
                            manufacturerRefreshControl(module)
                        }

                        if !supportedParameters.isEmpty {
                            standardOBDSection(module)
                        }

                        MBPanel {
                            Text("Mercedes manufacturer values are read directly from this ECU's physical diagnostic route. Positive UDS DIDs or KWP local identifiers are retained even when their meaning is not mapped yet; unknown data stays RAW rather than being mislabeled as OBD-II.")
                                .font(MBTypography.caption)
                                .foregroundStyle(MBBrand.muted)
                                .fixedSize(horizontal: false, vertical: true)
                        }
                    }
                    .padding(16)
                } else {
                    MBPanel {
                        Text("This control unit is no longer present in the active vehicle profile.")
                            .font(MBTypography.subheadline)
                            .foregroundStyle(MBBrand.silver)
                    }
                    .padding(16)
                }
            }
        }
        .searchable(text: $searchText, prompt: "Mercedes data ID, value or SAE PID")
        .mbDiagnosticScreen(module?.name ?? "Module Data")
    }

    private func moduleSummary(_ module: DiagnosticModule) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 8) {
                MBInfoRow(label: "CAN route", value: module.addressText)
                MBInfoRow(label: "Protocol", value: module.protocolName)
                if !module.designation.isEmpty {
                    MBInfoRow(label: "ECU", value: module.designation)
                }
                MBInfoRow(
                    label: "Mercedes data IDs",
                    value: manufacturerValues.isEmpty
                        ? (scanningThisModule ? "Scanning" : "Not scanned")
                        : "\(manufacturerValues.count) positive")
                if module.obdAdvertisedPIDCount > 0 {
                    MBInfoRow(
                        label: "OBD capability codes",
                        value: "\(module.obdAdvertisedPIDCount) advertised")
                    MBInfoRow(
                        label: "Decoded live values",
                        value: "\(supportedParameters.count) selectable")
                }
            }
        }
    }

    private func manufacturerKicker(_ module: DiagnosticModule) -> String {
        if module.protocolName.localizedCaseInsensitiveContains("KWP") {
            return "KWP2000 local identifiers · ECU \(module.responseAddressText)"
        }
        return "UDS ReadDataByIdentifier · ECU \(module.responseAddressText)"
    }

    private var scanProgress: some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 9) {
                HStack(spacing: 10) {
                    ProgressView()
                        .controlSize(.small)
                    Text("Reading module data")
                        .font(MBTypography.headline)
                        .foregroundStyle(MBBrand.silverBright)
                }
                Text(connection.manufacturerDataScanStatusText)
                    .font(MBTypography.caption)
                    .foregroundStyle(MBBrand.silver)
                    .fixedSize(horizontal: false, vertical: true)
                Text("This is a read-only manufacturer-data discovery pass, not the legislated OBD-II PID list.")
                    .font(MBTypography.caption)
                    .foregroundStyle(MBBrand.muted)
                    .fixedSize(horizontal: false, vertical: true)
            }
        }
    }

    @ViewBuilder
    private func manufacturerEmptyState(_ module: DiagnosticModule) -> some View {
        if !scanningThisModule {
            MBPanel {
                VStack(alignment: .leading, spacing: 10) {
                    Text(manufacturerValues.isEmpty
                         ? "No positive Mercedes manufacturer data IDs have been retained for this module yet."
                         : "No Mercedes manufacturer data matches the current search.")
                        .font(MBTypography.subheadline)
                        .foregroundStyle(MBBrand.silver)
                        .fixedSize(horizontal: false, vertical: true)

                    if manufacturerValues.isEmpty {
                        Button {
                            connection.discoverManufacturerData(moduleID: module.id)
                        } label: {
                            Label("Read Mercedes data from this ECU",
                                  systemImage: "dot.radiowaves.left.and.right")
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.background)
                                .frame(maxWidth: .infinity)
                                .padding(.vertical, 10)
                                .background(
                                    RoundedRectangle(
                                        cornerRadius: 11,
                                        style: .continuous)
                                        .fill(MBBrand.silverBright))
                        }
                        .buttonStyle(.plain)
                        .disabled(!connection.isActive)
                    }
                }
            }
        }
    }

    private func manufacturerRefreshControl(_ module: DiagnosticModule) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 9) {
                Button {
                    connection.discoverManufacturerData(moduleID: module.id)
                } label: {
                    Label("Refresh Mercedes values",
                          systemImage: "arrow.clockwise")
                        .font(MBTypography.subheadlineBold)
                        .foregroundStyle(MBBrand.background)
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 10)
                        .background(
                            RoundedRectangle(
                                cornerRadius: 11,
                                style: .continuous)
                                .fill(MBBrand.silverBright))
                }
                .buttonStyle(.plain)
                .disabled(!connection.isActive ||
                          connection.manufacturerDataScanActive)

                Text("Refresh re-reads only the \(manufacturerValues.count) identifiers that this ECU already proved positive, so it can capture changing values without repeating the full discovery sweep.")
                    .font(MBTypography.caption)
                    .foregroundStyle(MBBrand.muted)
                    .fixedSize(horizontal: false, vertical: true)
            }
        }
    }

    private func manufacturerValuesPanel(_ module: DiagnosticModule) -> some View {
        MBPanel {
            VStack(alignment: .leading, spacing: 0) {
                ForEach(filteredManufacturerValues) { value in
                    manufacturerRow(value, module: module)
                    if value.id != filteredManufacturerValues.last?.id {
                        Divider().overlay(MBBrand.line)
                    }
                }
            }
        }
    }

    private func manufacturerRow(
        _ value: MercedesModuleDataValue,
        module: DiagnosticModule
    ) -> some View {
        VStack(alignment: .leading, spacing: 7) {
            HStack(alignment: .firstTextBaseline, spacing: 10) {
                Text(value.title)
                    .font(MBTypography.subheadlineBold)
                    .foregroundStyle(MBBrand.silverBright)
                    .fixedSize(horizontal: false, vertical: true)
                Spacer(minLength: 8)
                Text(value.mapped ? "MAPPED" : "RAW")
                    .font(MBTypography.caption2Bold)
                    .foregroundStyle(value.mapped
                                     ? MBBrand.success : MBBrand.warning)
            }

            Text(value.formattedValue)
                .font(MBTypography.bold(20, relativeTo: .title3))
                .monospacedDigit()
                .foregroundStyle(MBBrand.silverBright)
                .fixedSize(horizontal: false, vertical: true)
                .textSelection(.enabled)

            Text("\(value.codeText) · \(value.serviceName)")
                .font(MBTypography.caption)
                .foregroundStyle(MBBrand.silver)
                .fixedSize(horizontal: false, vertical: true)
                .textSelection(.enabled)

            if !value.mapped {
                Text("Raw response payload · \(value.rawHex)")
                    .font(MBTypography.caption2)
                    .foregroundStyle(MBBrand.muted)
                    .fixedSize(horizontal: false, vertical: true)
                    .textSelection(.enabled)
            }
        }
        .padding(.vertical, 9)
    }

    @ViewBuilder
    private func standardOBDSection(_ module: DiagnosticModule) -> some View {
        MBSectionHeader(
            title: "Standard OBD-II",
            kicker: "Secondary · Mode 01 replies from \(module.responseAddressText)")

        Picker("Show", selection: $scope) {
            ForEach(MBLiveScope.allCases) { item in
                Text(LocalizedStringKey(item.rawValue)).tag(item)
            }
        }
        .pickerStyle(.segmented)

        ForEach(MBParameterGroup.allCases) { group in
            let parameters = filteredStandardParameters.filter {
                $0.brandGroup == group
            }
            if !parameters.isEmpty {
                MBPanel {
                    VStack(alignment: .leading, spacing: 6) {
                        Label(
                            LocalizedStringKey(group.rawValue),
                            systemImage: group.symbol)
                            .font(MBTypography.headline)
                            .foregroundStyle(MBBrand.silverBright)
                            .padding(.bottom, 4)

                        ForEach(parameters) { parameter in
                            standardLiveRow(parameter)
                            if parameter.id != parameters.last?.id {
                                Divider().overlay(MBBrand.line)
                            }
                        }
                    }
                }
            }
        }
    }

    private func standardLiveRow(_ parameter: DiagnosticParameter) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(LocalizedStringKey(parameter.title))
                .font(MBTypography.subheadlineBold)
                .foregroundStyle(MBBrand.silverBright)
                .fixedSize(horizontal: false, vertical: true)
                .frame(maxWidth: .infinity, alignment: .leading)

            HStack(alignment: .center, spacing: 12) {
                Text(parameter.presentationValue)
                    .font(MBTypography.bold(21, relativeTo: .title3))
                    .monospacedDigit()
                    .foregroundStyle(parameter.hasLiveValue
                                     ? MBBrand.silverBright : MBBrand.muted)
                    .lineLimit(1)
                    .minimumScaleFactor(0.68)
                    .layoutPriority(2)

                Spacer(minLength: 10)

                HStack(spacing: 7) {
                    Text("Poll")
                        .font(MBTypography.caption2Bold)
                        .foregroundStyle(MBBrand.muted)

                    Toggle("", isOn: Binding(
                        get: { parameter.pollingEnabled },
                        set: {
                            connection.setPolling(
                                $0, stableKey: parameter.id)
                        }
                    ))
                    .labelsHidden()
                    .tint(MBBrand.success)
                    .controlSize(.small)

                    Button {
                        connection.toggleFavourite(stableKey: parameter.id)
                    } label: {
                        Image(systemName: parameter.favourite
                              ? "star.fill" : "star")
                            .font(MBTypography.title3)
                            .foregroundStyle(parameter.favourite
                                             ? MBBrand.silverBright
                                             : MBBrand.muted)
                            .frame(width: 30, height: 30)
                    }
                    .buttonStyle(.plain)
                }
                .fixedSize(horizontal: true, vertical: false)
            }

            Text("\(parameter.shortName) · SAE OBD-II · \(parameter.brandPidText)")
                .font(MBTypography.caption)
                .foregroundStyle(MBBrand.muted)
                .fixedSize(horizontal: false, vertical: true)
        }
        .padding(.vertical, 9)
    }
}

private struct MBDataTableView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.showUnsupportedParameters.v2") private var showUnsupportedParameters = false

    private var sorted: [DiagnosticParameter] {
        connection.diagnosticParameters
            .filter { showUnsupportedParameters || $0.isSupported || $0.isAvailable }
            .sorted { $0.title < $1.title }
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 12) {
                    MBPanel {
                        VStack(alignment: .leading, spacing: 9) {
                            Toggle("Show unsupported catalogue entries",
                                   isOn: $showUnsupportedParameters)
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
                            Text("Not polled = supported by the vehicle but disabled. Waiting for sample = polling is enabled but no value has arrived yet. Not advertised = the vehicle did not report support for that PID.")
                                .font(MBTypography.caption)
                                .foregroundStyle(MBBrand.muted)
                                .fixedSize(horizontal: false, vertical: true)
                        }
                    }

                    MBPanel {
                        VStack(spacing: 0) {
                        ForEach(sorted) { parameter in
                            HStack(spacing: 10) {
                                Text(parameter.brandPidText)
                                    .font(MBTypography.captionBold)
                                    .foregroundStyle(MBBrand.silver)
                                    .frame(width: 48, alignment: .leading)
                                Text(LocalizedStringKey(parameter.title))
                                    .font(MBTypography.subheadline)
                                    .foregroundStyle(MBBrand.silverBright)
                                Spacer()
                                Text(parameter.presentationValue)
                                    .font(MBTypography.subheadlineBold)
                                    .foregroundStyle(parameter.hasLiveValue ? MBBrand.silverBright : MBBrand.muted)
                            }
                            .padding(.vertical, 9)
                            if parameter.id != sorted.last?.id { Divider().overlay(MBBrand.line) }
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
        "obd2.engine.rpm", "obd2.vehicle.speed", "obd2.engine.coolant",
        "obd2.diesel.rail_pressure", "obd2.fuel.tank_level",
        "obd2.dpf.bank1_delta_pressure", "obd2.aftertreatment.egt_b1s1"
    ]

    private var displayed: [DiagnosticParameter] {
        let available = connection.dashboardParameters.filter { $0.pollingEnabled && $0.isAvailable }
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
                    if connection.isActive && !connection.isReady {
                        MBPanel {
                            VStack(alignment: .leading, spacing: 7) {
                                Label("Mercedes module census", systemImage: "dot.radiowaves.left.and.right")
                                    .font(MBTypography.headline)
                                    .foregroundStyle(MBBrand.silverBright)
                                Text(connection.mercedesProbeStatusText)
                                    .font(MBTypography.subheadline)
                                    .foregroundStyle(MBBrand.silver)
                                Text("\(connection.diagnosticModules.count) responding control units found")
                                    .font(MBTypography.caption)
                                    .foregroundStyle(MBBrand.muted)
                            }
                        }
                    }
                    if displayed.isEmpty {
                        MBPanel {
                            Text(connection.isActive
                                 ? "Live polling will begin when the read-only module and fault census finishes."
                                 : "Connect to the vehicle to populate dashboard measurements.")
                                .font(MBTypography.subheadline)
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
    private enum Scope: String, CaseIterable, Identifiable {
        case vehicle = "Vehicle targets"
        case mercedesMe = "Mercedes me IDs"
        var id: String { rawValue }
    }

    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var searchText = ""
    @State private var scope: Scope = .vehicle

    private var targetSignals: [MercedesTargetSignal] {
        guard scope == .vehicle else { return [] }
        guard !searchText.isEmpty else { return connection.mercedesTargetSignals }
        return connection.mercedesTargetSignals.filter {
            $0.title.localizedCaseInsensitiveContains(searchText) ||
            $0.category.localizedCaseInsensitiveContains(searchText) ||
            $0.status.localizedCaseInsensitiveContains(searchText)
        }
    }

    private var nativeIdentities: [MercedesNativeDataIdentity] {
        guard scope == .mercedesMe else { return [] }
        let values = connection.mercedesNativeDataIdentities
        guard !searchText.isEmpty else {
            return values.sorted { $0.symbol < $1.symbol }
        }
        return values.filter {
            $0.symbol.localizedCaseInsensitiveContains(searchText) ||
            $0.dataID.localizedCaseInsensitiveContains(searchText)
        }.sorted { $0.symbol < $1.symbol }
    }

    private var targetCategories: [String] {
        Array(Set(targetSignals.map(\.category))).sorted()
    }

    private var mappedCount: Int {
        connection.mercedesTargetSignals.filter {
            $0.status != "corroborated-unmapped"
        }.count
    }

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBSectionHeader(title: "Factory data", kicker: "Mercedes-Benz evidence catalogue")

                    MBPanel {
                        VStack(spacing: 4) {
                            MBInfoRow(label: "OM651 vehicle targets",
                                      value: "\(connection.mercedesTargetSignals.count)")
                            MBInfoRow(label: "Mercedes me identities",
                                      value: "\(connection.mercedesNativeDataIdentities.count)")
                            MBInfoRow(label: "Mapped / verified targets",
                                      value: "\(mappedCount)")
                        }
                    }

                    Picker("Factory data source", selection: $scope) {
                        ForEach(Scope.allCases) { item in
                            Text(item.rawValue).tag(item)
                        }
                    }
                    .pickerStyle(.segmented)

                    if scope == .vehicle {
                        vehicleTargets
                    } else {
                        mercedesMeIdentities
                    }

                    MBPanel {
                        Text(scope == .vehicle
                             ? "Vehicle targets are manufacturer values known to exist on the OM651/CDID3 family. MBLINK only polls a value after its request, response shape, scale and meaning are verified."
                             : "Mercedes me IDs are exact model identifiers recovered from the official diagnostic stack. They prove the factory framework knew the value; they do not by themselves prove a CAN address, UDS/KWP request, payload layout or scale.")
                            .font(MBTypography.caption)
                            .foregroundStyle(MBBrand.muted)
                    }
                }
                .padding(16)
            }
        }
        .searchable(
            text: $searchText,
            prompt: scope == .vehicle
                ? "Factory value or category"
                : "Mercedes me identity")
        .mbDiagnosticScreen("Factory Data")
    }

    @ViewBuilder
    private var vehicleTargets: some View {
        ForEach(targetCategories, id: \.self) { category in
            let signals = targetSignals.filter { $0.category == category }
            if !signals.isEmpty {
                MBPanel {
                    VStack(alignment: .leading, spacing: 9) {
                        Text(category.uppercased())
                            .font(MBTypography.caption2Bold)
                            .tracking(0.9)
                            .foregroundStyle(MBBrand.muted)

                        ForEach(signals) { signal in
                            VStack(alignment: .leading, spacing: 3) {
                                HStack(alignment: .firstTextBaseline) {
                                    Text(signal.title)
                                        .font(MBTypography.subheadlineBold)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Spacer(minLength: 8)
                                    Text(statusLabel(signal.status))
                                        .font(MBTypography.caption2Bold)
                                        .foregroundStyle(signal.status == "vehicle-verified"
                                                         ? MBBrand.success : MBBrand.warning)
                                }
                                if signal.status != "corroborated-unmapped" {
                                    Text(signal.provenance)
                                        .font(MBTypography.caption2)
                                        .foregroundStyle(MBBrand.muted)
                                        .fixedSize(horizontal: false, vertical: true)
                                }
                            }
                            .padding(.vertical, 5)

                            if signal.id != signals.last?.id {
                                Divider().overlay(MBBrand.line)
                            }
                        }
                    }
                }
            }
        }

        if targetSignals.isEmpty {
            MBPanel {
                Text("No vehicle factory-data targets match the current search.")
                    .font(MBTypography.subheadline)
                    .foregroundStyle(MBBrand.silver)
            }
        }
    }

    @ViewBuilder
    private var mercedesMeIdentities: some View {
        if nativeIdentities.isEmpty {
            MBPanel {
                Text("No Mercedes me data identities match the current search.")
                    .font(MBTypography.subheadline)
                    .foregroundStyle(MBBrand.silver)
            }
        } else {
            MBPanel {
                LazyVStack(alignment: .leading, spacing: 0) {
                    ForEach(nativeIdentities) { identity in
                        HStack(alignment: .top, spacing: 10) {
                            VStack(alignment: .leading, spacing: 3) {
                                Text(nativeTitle(identity.symbol))
                                    .font(MBTypography.subheadlineBold)
                                    .foregroundStyle(MBBrand.silverBright)
                                Text(identity.dataID)
                                    .font(MBTypography.caption)
                                    .foregroundStyle(MBBrand.silver)
                                    .textSelection(.enabled)
                                Text(identity.symbol)
                                    .font(MBTypography.caption2)
                                    .foregroundStyle(MBBrand.muted)
                                    .textSelection(.enabled)
                            }
                            Spacer(minLength: 8)
                            Text("KNOWN ID")
                                .font(MBTypography.caption2Bold)
                                .foregroundStyle(MBBrand.silver)
                        }
                        .padding(.vertical, 8)

                        if identity.id != nativeIdentities.last?.id {
                            Divider().overlay(MBBrand.line)
                        }
                    }
                }
            }
        }
    }

    private func nativeTitle(_ symbol: String) -> String {
        let acronyms: Set<String> = [
            "ABS", "BT", "CAN", "DCS", "ECU", "HIL", "ID",
            "MMC", "OBD", "RPM", "SAM", "TM", "VIN"
        ]
        return symbol.split(separator: "_").map { part in
            let word = String(part)
            if acronyms.contains(word) { return word }
            return word.prefix(1) + word.dropFirst().lowercased()
        }.joined(separator: " ")
    }

    private func statusLabel(_ status: String) -> String {
        switch status {
        case "vehicle-verified": return "VERIFIED"
        case "mapping-candidate": return "CANDIDATE"
        default: return "UNMAPPED"
        }
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
                                .font(MBTypography.subheadline)
                                .foregroundStyle(MBBrand.silver)
                        }
                    } else {
                        ForEach(graphed) { parameter in
                            MBPanel {
                                VStack(alignment: .leading, spacing: 10) {
                                    HStack {
                                        Text(LocalizedStringKey(parameter.title))
                                            .font(MBTypography.headline)
                                            .foregroundStyle(MBBrand.silverBright)
                                        Spacer()
                                        Text(parameter.formattedValue)
                                            .font(MBTypography.headline)
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
    @State private var showingTechnicalDetails = false

    var body: some View {
        ZStack {
            MBBackground()
            ScrollView {
                VStack(alignment: .leading, spacing: 15) {
                    MBPanel {
                        VStack(alignment: .leading, spacing: 4) {
                            MBSectionHeader(title: "Session summary", kicker: "Vehicle evidence")
                            MBInfoRow(label: "VIN", value: connection.mercedesVINText, monospaced: true)
                            Divider().overlay(MBBrand.line)
                            MBInfoRow(label: "Mercedes probe", value: connection.mercedesProbeStatusText)
                            Divider().overlay(MBBrand.line)
                            MBInfoRow(label: "UDS faults", value: connection.mercedesUDSFaultStatusText)
                            Divider().overlay(MBBrand.line)
                            MBInfoRow(label: "OBD fault scan", value: connection.faultScanStatusText)
                            Divider().overlay(MBBrand.line)
                            MBInfoRow(label: "Recorded samples", value: "\(connection.recordedSampleCount)")
                        }
                    }

                    MBPanel {
                        DisclosureGroup(isExpanded: $showingTechnicalDetails) {
                            VStack(alignment: .leading, spacing: 4) {
                                Divider().overlay(MBBrand.line)
                                    .padding(.vertical, 5)
                                MBInfoRow(label: "Engine endpoint", value: connection.mercedesProbeEndpointText)
                                Divider().overlay(MBBrand.line)
                                MBInfoRow(label: "Identity", value: connection.mercedesIdentitySummaryText)
                                Divider().overlay(MBBrand.line)
                                MBInfoRow(label: "CRD3", value: connection.mercedesCrd3SummaryText)
                            }
                        } label: {
                            Label("Technical details", systemImage: "wrench.and.screwdriver")
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
                        }
                        .tint(MBBrand.silver)
                    }

                    MBPanel {
                        VStack(alignment: .leading, spacing: 11) {
                            Text("Diagnostic evidence CSV")
                                .font(MBTypography.headline)
                                .foregroundStyle(MBBrand.silverBright)
                            Text("Creates a snapshot of the current session without stopping live polling.")
                                .font(MBTypography.caption)
                                .foregroundStyle(MBBrand.muted)
                                .fixedSize(horizontal: false, vertical: true)

                            Button {
                                connection.prepareCSVExport()
                            } label: {
                                HStack(spacing: 8) {
                                    if connection.isPreparingCSV {
                                        ProgressView()
                                            .controlSize(.small)
                                    }
                                    Label("Prepare evidence CSV",
                                          systemImage: "doc.badge.gearshape")
                                    Spacer()
                                }
                                .font(MBTypography.subheadlineBold)
                                .foregroundStyle(MBBrand.silverBright)
                                .padding(.vertical, 3)
                            }
                            .disabled(connection.isPreparingCSV)

                            if let exportURL = connection.csvExportURL {
                                Divider().overlay(MBBrand.line)
                                ShareLink(item: exportURL) {
                                    HStack(spacing: 8) {
                                        Label("Share evidence CSV", systemImage: "square.and.arrow.up")
                                        Spacer()
                                    }
                                    .font(MBTypography.subheadlineBold)
                                    .foregroundStyle(MBBrand.silverBright)
                                    .padding(.vertical, 3)
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
                                    .font(MBTypography.title3)
                                    .foregroundStyle(MBBrand.silverBright)
                                Text("Language")
                                    .font(MBTypography.headline)
                                    .foregroundStyle(MBBrand.silverBright)
                                Spacer()
                                Text(MBInterfaceLanguage.displayName(for: language))
                                    .font(MBTypography.subheadline)
                                    .foregroundStyle(MBBrand.silver)
                                Image(systemName: "chevron.right")
                                    .font(MBTypography.captionBold)
                                    .foregroundStyle(MBBrand.muted)
                            }
                            .contentShape(Rectangle())
                        }
                        .buttonStyle(.plain)
                    }
                    MBPanel {
                        HStack(spacing: 12) {
                            Image(systemName: "ruler")
                                .font(MBTypography.title3)
                                .foregroundStyle(MBBrand.silverBright)
                            Text("Unit system")
                                .font(MBTypography.headline)
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
                                .font(MBTypography.bodyBold)
                                .foregroundStyle(MBBrand.silverBright)
                            Spacer()
                            if MBInterfaceLanguage.canonical(selection) == item.id {
                                Image(systemName: "checkmark")
                                    .font(MBTypography.bodyBold)
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
                                .font(MBTypography.bold(34, relativeTo: .largeTitle))
                                .tracking(2.0)
                                .foregroundStyle(MBBrand.silverBright)
                            Text("MERCEDES DIAGNOSTICS")
                                .font(MBTypography.caption2Bold)
                                .tracking(1.7)
                                .foregroundStyle(MBBrand.silver)
                        }
                        Text("Version \(version)")
                            .font(MBTypography.subheadline)
                            .foregroundStyle(MBBrand.muted)
                        Text("A C-first, open-source Mercedes vehicle diagnostics platform authored by Shannon Smith.")
                            .font(MBTypography.body)
                            .multilineTextAlignment(.center)
                            .foregroundStyle(MBBrand.silverBright)
                            .padding(.horizontal, 28)
                        Text("Copyright © 2026 Shannon Smith")
                            .font(MBTypography.subheadline)
                            .foregroundStyle(MBBrand.muted)
                        Link(
                            "Project Website",
                            destination: URL(string: "https://github.com/Infiltrator-Projects/MBLINK")!
                        )
                        .font(MBTypography.bodyBold)
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
