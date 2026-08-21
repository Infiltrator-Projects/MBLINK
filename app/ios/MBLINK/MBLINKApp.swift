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
                    NavigationStack {
                        List {
                            Section {
                                VStack(alignment: .center, spacing: 8) {
                                    Image(systemName: "car.side.fill")
                                        .font(.system(size: 48, weight: .semibold))
                                    Text("MBLINK")
                                        .font(.title.bold())
                                    Text("Open-source vehicle diagnostics platform")
                                        .font(.subheadline)
                                        .foregroundStyle(.secondary)
                                        .multilineTextAlignment(.center)
                                }
                                .frame(maxWidth: .infinity)
                                .padding(.vertical, 10)
                            }

                            Section("Application") {
                                LabeledContent(
                                    "Version",
                                    value: Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "Unknown"
                                )
                                LabeledContent("Author", value: "Shannon Smith")
                                LabeledContent("Copyright", value: "Copyright © 2026 Shannon Smith")
                                LabeledContent("Licence", value: "GPL-3.0-or-later")
                            }

                            Section("Project") {
                                Link(destination: URL(string: "https://github.com/The-First-Infiltrator/MBLINK")!) {
                                    Label("MBLINK on GitHub", systemImage: "link")
                                }
                                Text("Portable diagnostics are owned by libmblink and Infiltratr Common, with the native iPhone interface remaining a thin presentation and transport layer.")
                                    .font(.footnote)
                                    .foregroundStyle(.secondary)
                            }
                        }
                        .navigationTitle("About MBLINK")
                        .navigationBarTitleDisplayMode(.inline)
                        .toolbar {
                            ToolbarItem(placement: .confirmationAction) {
                                Button("Done") {
                                    showingAbout = false
                                }
                            }
                        }
                    }
                }
        }
    }
}
