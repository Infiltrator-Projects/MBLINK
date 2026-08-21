// SPDX-License-Identifier: GPL-3.0-or-later
import SwiftUI

@main
struct MBLINKApp: App {
    @StateObject private var connection = ConnectionViewModel()
    @State private var showingAbout = false

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(connection)
                .safeAreaInset(edge: .bottom, spacing: 0) {
                    Button {
                        showingAbout = true
                    } label: {
                        HStack(spacing: 8) {
                            Text("Copyright © 2026 Shannon Smith")
                            Spacer()
                            Label("About", systemImage: "info.circle")
                        }
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .padding(.horizontal, 16)
                        .padding(.vertical, 8)
                        .frame(maxWidth: .infinity)
                        .background(.bar)
                    }
                    .buttonStyle(.plain)
                }
                .sheet(isPresented: $showingAbout) {
                    MBLINKAboutView {
                        showingAbout = false
                    }
                }
        }
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
        VStack(spacing: 0) {
            ScrollView {
                VStack(spacing: 14) {
                    Image(systemName: "car.side.fill")
                        .font(.system(size: 62, weight: .semibold))
                        .padding(.top, 28)

                    Text("MBLINK")
                        .font(.title.bold())

                    Text("Version \(version)")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)

                    Text("A C-first, open-source vehicle diagnostics platform authored by Shannon Smith.")
                        .font(.body)
                        .multilineTextAlignment(.center)
                        .padding(.horizontal, 28)
                        .padding(.top, 4)

                    Text("Copyright © 2026 Shannon Smith")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)
                        .padding(.top, 6)

                    Link("Website", destination: URL(string: "https://github.com/The-First-Infiltrator/MBLINK")!)
                        .font(.body)
                }
                .frame(maxWidth: .infinity)
            }

            Divider()

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
            }
            .frame(maxWidth: .infinity)
            .padding(.horizontal, 16)
            .padding(.vertical, 12)
            .background(.bar)
        }
        .presentationDetents([.medium, .large])
        .presentationDragIndicator(.visible)
        .sheet(item: $detail) { item in
            switch item {
            case .credits:
                NavigationStack {
                    List {
                        Section("Credits") {
                            Text("Shannon Smith — Author and project maintainer")
                        }
                    }
                    .navigationTitle("Credits")
                    .navigationBarTitleDisplayMode(.inline)
                    .toolbar {
                        ToolbarItem(placement: .confirmationAction) {
                            Button("Close") {
                                detail = nil
                            }
                        }
                    }
                }
            case .license:
                NavigationStack {
                    ScrollView {
                        Text(
                            "MBLINK is free software licensed under the GNU General Public License version 3 or, at your option, any later version (GPL-3.0-or-later).\n\nSee LICENSE in the source package for the complete licence text."
                        )
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .padding()
                    }
                    .navigationTitle("License")
                    .navigationBarTitleDisplayMode(.inline)
                    .toolbar {
                        ToolbarItem(placement: .confirmationAction) {
                            Button("Close") {
                                detail = nil
                            }
                        }
                    }
                }
            }
        }
    }
}
