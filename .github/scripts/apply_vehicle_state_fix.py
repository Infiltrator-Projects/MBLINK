# SPDX-License-Identifier: GPL-3.0-or-later
from pathlib import Path

path = Path("app/ios/MBLINK/ConnectionViewModel.swift")
text = path.read_text()


def replace_once(old: str, new: str) -> None:
    global text
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"expected one source block, found {count}: {old[:120]!r}")
    text = text.replace(old, new, 1)


replace_once(
'''    func selectSavedVehicle(vin: String) {
        guard savedVehicleProfiles.contains(where: { $0.vin == vin }) else { return }
        selectedVehicleVIN = vin
        UserDefaults.standard.set(vin, forKey: Self.selectedVehicleVINDefaultsKey)
        refreshPIDConfiguration()
        applyConfiguredPollingForSelectedVehicle()
        refreshPresentation()
    }
''',
'''    func selectSavedVehicle(vin: String) {
        // A live VIN is authoritative. Saved-profile selection is an offline
        // operation and must never override the physical car.
        guard !controller.isActive,
              savedVehicleProfiles.contains(where: { $0.vin == vin }) else { return }
        selectedVehicleVIN = vin
        UserDefaults.standard.set(vin, forKey: Self.selectedVehicleVINDefaultsKey)
        mercedesVINText = vin
        vehicleIdentity = decodeVehicleIdentity(vin: vin)
        vehicleProfileStatusText = "Saved vehicle profile loaded · offline"
        mercedesIdentitySummaryText = "Saved vehicle profile · offline"
        mercedesProbeStatusText = "Disconnected · saved vehicle profile"
        refreshPIDConfiguration()
        applyConfiguredPollingForSelectedVehicle()
        refreshPresentation()
    }
''')

replace_once(
'''    private var effectivePIDConfigurationVIN: String? {
        if let liveVIN = controller.mercedesVINText, liveVIN.count == 17 {
            return liveVIN
        }
        return selectedVehicleVIN
    }
''',
'''    private var effectivePIDConfigurationVIN: String? {
        // The controller deliberately retains its last VIN after disconnect.
        // It is authoritative only while a live diagnostic session is active.
        if controller.isActive,
           let liveVIN = controller.mercedesVINText,
           liveVIN.count == 17 {
            return liveVIN
        }
        return selectedVehicleVIN
    }
''')

replace_once(
'''        if let selectedVehicleVIN,
           savedVehicleProfiles.contains(where: { $0.vin == selectedVehicleVIN }) {
            return
        }
        if let newest = savedVehicleProfiles.first {
            selectedVehicleVIN = newest.vin
            UserDefaults.standard.set(
                newest.vin, forKey: Self.selectedVehicleVINDefaultsKey)
        } else {
            selectedVehicleVIN = nil
            UserDefaults.standard.removeObject(
                forKey: Self.selectedVehicleVINDefaultsKey)
        }
''',
'''        if let selectedVehicleVIN,
           savedVehicleProfiles.contains(where: { $0.vin == selectedVehicleVIN }) {
            return
        }

        // No remembered current vehicle means exactly that. Never manufacture
        // a current vehicle by picking the newest unrelated saved profile.
        selectedVehicleVIN = nil
        UserDefaults.standard.removeObject(
            forKey: Self.selectedVehicleVINDefaultsKey)
''')

replace_once(
'''        let liveVIN = controller.mercedesVINText
        var selectedVIN: String? =
            (liveVIN?.count == 17 ? liveVIN : selectedVehicleVIN)
        var selectedProfile: [String: Any]?

        if let vin = selectedVIN,
           let profile = profiles[vin] as? [String: Any] {
            selectedProfile = profile
        } else {
            for (vin, value) in profiles {
                guard let profile = value as? [String: Any] else { continue }
                let updated = (profile["updatedAt"] as? NSNumber)?.doubleValue ?? 0
                let selectedUpdated =
                    (selectedProfile?["updatedAt"] as? NSNumber)?.doubleValue ?? -1
                if selectedProfile == nil || updated > selectedUpdated {
                    selectedVIN = vin
                    selectedProfile = profile
                }
            }
        }

        guard let profile = selectedProfile else {
            return ([], [:],
                    "Connect once to learn which PIDs each controller supports")
        }
''',
'''        let liveVIN = controller.isActive ? controller.mercedesVINText : nil
        let selectedVIN: String? =
            (liveVIN?.count == 17 ? liveVIN : selectedVehicleVIN)

        // While connected, only the physical car's VIN may select a profile.
        // While offline, only the remembered/explicitly selected VIN may do so.
        guard let vin = selectedVIN,
              let profile = profiles[vin] as? [String: Any] else {
            let label = controller.isActive && liveVIN?.count == 17
                ? "New vehicle detected · learning controller map"
                : "No vehicle loaded · connect to a vehicle"
            return ([], [:], label)
        }
''')

replace_once(
'''        let live = diagnosticModules
        if !live.isEmpty {
''',
'''        let live = diagnosticModules
        if isActive && !live.isEmpty {
''')

replace_once(
'''        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        mercedesProbeStatusText = controller.mercedesProbeStatusText
        mercedesProbeEndpointText = controller.mercedesProbeEndpointText ?? "Source-corroborated endpoint not selected"
        let capturedVIN = controller.mercedesVINText
        mercedesVINText = capturedVIN ?? "Not captured"
        vehicleIdentity = decodeVehicleIdentity(vin: capturedVIN)
        mercedesIdentitySummaryText = controller.mercedesIdentitySummaryText
        mercedesIdentityResults = controller.mercedesIdentityResults
        mercedesCrd3SummaryText = controller.mercedesCrd3SummaryText
        mercedesUDSFaultStatusText = controller.mercedesUDSFaultStatusText
        mercedesUDSFaults = controller.mercedesUDSFaults
        vehicleProfileStatusText = controller.vehicleProfileStatusText
        faultScanStatusText = controller.faultScanStatusText
''',
'''        adapterIdentifier = controller.adapterIdentifier ?? "Unknown"
        refreshSavedVehicleProfiles()

        let capturedVIN = controller.isActive ? controller.mercedesVINText : nil
        let currentVIN = capturedVIN?.count == 17
            ? capturedVIN : selectedVehicleVIN
        mercedesVINText = currentVIN ?? "Not captured"
        vehicleIdentity = decodeVehicleIdentity(vin: currentVIN)

        if controller.isActive {
            mercedesProbeStatusText = controller.mercedesProbeStatusText
            mercedesProbeEndpointText = controller.mercedesProbeEndpointText ?? "Source-corroborated endpoint not selected"
            mercedesIdentitySummaryText = controller.mercedesIdentitySummaryText
            mercedesIdentityResults = controller.mercedesIdentityResults
            mercedesCrd3SummaryText = controller.mercedesCrd3SummaryText
            mercedesUDSFaultStatusText = controller.mercedesUDSFaultStatusText
            mercedesUDSFaults = controller.mercedesUDSFaults
            vehicleProfileStatusText = controller.vehicleProfileStatusText
        } else if let vin = selectedVehicleVIN {
            let profiles = UserDefaults.standard.dictionary(
                forKey: Self.vehicleProfilesDefaultsKey) ?? [:]
            let profile = profiles[vin] as? [String: Any]
            let modules = profile?["modules"] as? [[String: Any]] ?? []
            mercedesProbeStatusText = "Disconnected · saved vehicle profile"
            mercedesProbeEndpointText =
                (profile?["probeEndpoint"] as? String) ?? "Saved profile · endpoint not recorded"
            mercedesIdentitySummaryText =
                "Saved vehicle profile · \(modules.count) controller\(modules.count == 1 ? "" : "s") · offline"
            mercedesIdentityResults = []
            mercedesCrd3SummaryText =
                (profile?["crd3Summary"] as? String) ?? "Saved profile · identity not recorded"
            mercedesUDSFaultStatusText = "Disconnected · saved fault state not refreshed"
            mercedesUDSFaults = []
            vehicleProfileStatusText = "Saved vehicle profile loaded · offline"
        } else {
            mercedesProbeStatusText = "Not connected"
            mercedesProbeEndpointText = "No vehicle loaded"
            mercedesIdentitySummaryText = "No vehicle loaded"
            mercedesIdentityResults = []
            mercedesCrd3SummaryText = "Not available"
            mercedesUDSFaultStatusText = "Not scanned"
            mercedesUDSFaults = []
            vehicleProfileStatusText = "No vehicle loaded · connect to a vehicle"
        }
        faultScanStatusText = controller.faultScanStatusText
''')

replace_once(
'''        isActive = controller.isActive
        isReady = controller.isReady
        refreshSavedVehicleProfiles()
        if let liveVIN = controller.mercedesVINText, liveVIN.count == 17,
           selectedVehicleVIN != liveVIN {
            selectedVehicleVIN = liveVIN
            UserDefaults.standard.set(
                liveVIN, forKey: Self.selectedVehicleVINDefaultsKey)
        }
        diagnosticModules = loadDiagnosticModules()
''',
'''        isActive = controller.isActive
        isReady = controller.isReady
        if controller.isActive,
           let liveVIN = controller.mercedesVINText,
           liveVIN.count == 17,
           selectedVehicleVIN != liveVIN {
            // Live VIN wins immediately. The controller either validates the
            // exact saved profile or learns a new one under this VIN.
            selectedVehicleVIN = liveVIN
            UserDefaults.standard.set(
                liveVIN, forKey: Self.selectedVehicleVINDefaultsKey)
        }
        diagnosticModules = controller.isActive ? loadDiagnosticModules() : []
''')

path.write_text(text)

ci_path = Path(".github/workflows/ci.yml")
ci = ci_path.read_text()
anchor = "          grep -Fq 'diagnosticCapabilityText = controller.diagnosticCapabilityText' app/ios/MBLINK/ConnectionViewModel.swift\n"
if ci.count(anchor) != 1:
    raise SystemExit("CI regression-guard anchor missing or duplicated")
guards = anchor + (
    "          grep -Fq 'Saved vehicle profile loaded · offline' app/ios/MBLINK/ConnectionViewModel.swift\n"
    "          grep -Fq 'No vehicle loaded · connect to a vehicle' app/ios/MBLINK/ConnectionViewModel.swift\n"
    "          grep -Fq 'if isActive && !live.isEmpty {' app/ios/MBLINK/ConnectionViewModel.swift\n"
    "          grep -Fq 'let liveVIN = controller.isActive ? controller.mercedesVINText : nil' app/ios/MBLINK/ConnectionViewModel.swift\n"
    "          ! grep -Fq 'if let newest = savedVehicleProfiles.first' app/ios/MBLINK/ConnectionViewModel.swift\n"
)
ci_path.write_text(ci.replace(anchor, guards, 1))

# One-shot implementation machinery: leave only the product source and its
# regression guards in the repository after this commit.
Path(".github/workflows/apply-vehicle-state-fix.yml").unlink()
Path(".github/scripts/apply_vehicle_state_fix.py").unlink()
