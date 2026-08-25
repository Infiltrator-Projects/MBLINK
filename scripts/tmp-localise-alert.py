from pathlib import Path

path = Path("app/ios/MBLINK/ConnectionViewModel.swift")
text = path.read_text()

class_marker = "@MainActor\nfinal class ConnectionViewModel: NSObject, ObservableObject, MBLinkDiagnosticsControllerDelegate {"
helper = '''private func mblinkLocalized(_ key: String) -> String {
    let language = UserDefaults.standard.string(forKey: "mblink.language") ?? "en"
    guard let path = Bundle.main.path(forResource: language, ofType: "lproj"),
          let bundle = Bundle(path: path) else {
        return key
    }
    return bundle.localizedString(forKey: key, value: key, table: nil)
}

@MainActor
final class ConnectionViewModel: NSObject, ObservableObject, MBLinkDiagnosticsControllerDelegate {'''
if text.count(class_marker) != 1:
    raise SystemExit("ConnectionViewModel class marker changed")
text = text.replace(class_marker, helper, 1)

replacements = [
    ('title: "Connection Test"', 'title: mblinkLocalized("Connection Test")'),
    ('message: "Real Adapter uses Bluetooth. Simulated ELM327 runs the same ELM, OBD, UDS, Mercedes probe, telemetry and evidence stack against an in-process byte-stream emulator."', 'message: mblinkLocalized("Real Adapter uses Bluetooth. Simulated ELM327 runs the same ELM, OBD, UDS, Mercedes probe, telemetry and evidence stack against an in-process byte-stream emulator.")'),
    ('UIAlertAction(title: "Real Adapter", style: .default)', 'UIAlertAction(title: mblinkLocalized("Real Adapter"), style: .default)'),
    ('UIAlertAction(title: "Simulated ELM327", style: .default)', 'UIAlertAction(title: mblinkLocalized("Simulated ELM327"), style: .default)'),
    ('UIAlertAction(title: "Cancel", style: .cancel)', 'UIAlertAction(title: mblinkLocalized("Cancel"), style: .cancel)'),
]
for old, new in replacements:
    if text.count(old) != 1:
        raise SystemExit(f"alert marker changed: {old}")
    text = text.replace(old, new, 1)

path.write_text(text)
