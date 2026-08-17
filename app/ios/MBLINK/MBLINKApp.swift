// SPDX-License-Identifier: GPL-3.0-or-later
import SwiftUI

@main
struct MBLINKApp: App {
    @StateObject private var connection = ConnectionViewModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(connection)
        }
    }
}
