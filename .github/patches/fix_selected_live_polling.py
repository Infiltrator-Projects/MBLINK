from pathlib import Path

OLD_LINK = "e61aae48f784363932b5c8fe47c3d4b13974466f"
NEW_LINK = "50af397d8c4da5695635d38278af2e87f8ea0114"

p = Path("app/ios/MBLINK/ConnectionViewModel.swift")
s = p.read_text()

anchor = '''    private static let standardSelectionMigrationDefaultsKey =
        "mblink.standard.pidSelectionsGlobalMigrated.v1"
'''
addition = anchor + '''    private static let standardSelectionExplicitDefaultsKey =
        "mblink.standard.pidSelectionsExplicitByVehicle.v1"
'''
if s.count(anchor) != 1:
    raise SystemExit("selection key anchor mismatch")
s = s.replace(anchor, addition)

old_ci = '''#if MBLINK_CI_SIMULATED_FLOW
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1" {
            startSimulatedDiagnostics()
        }
#endif
'''
new_ci = '''#if MBLINK_CI_SIMULATED_FLOW
        if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_FLOW"] == "1" {
            if ProcessInfo.processInfo.environment["MBLINK_CI_SIMULATED_POLLING"] == "1" {
                let simulatedVIN = "WDD2073022F123456"
                pidSelectionStore.setStableKeys(
                    ["obd2.engine.rpm", "obd2.vehicle.speed"],
                    forVIN: simulatedVIN,
                    controllerIdentifier: Self.standardSelectionControllerIdentifier)
                markStandardSelectionExplicitlyEdited(vin: simulatedVIN)
                appliedPollingConfigurationKey = nil
            }
            startSimulatedDiagnostics()
        }
#endif
'''
if s.count(old_ci) != 1:
    raise SystemExit("simulated init anchor mismatch")
s = s.replace(old_ci, new_ci)

old_store = '''        storeStandardPollingKeys(enabledKeys)
        controller.setPollingEnabled(enabled, forPID: pid)
'''
new_store = '''        storeStandardPollingKeys(enabledKeys)
        if let vin = effectivePIDConfigurationVIN {
            markStandardSelectionExplicitlyEdited(vin: vin)
        }
        controller.setPollingEnabled(enabled, forPID: pid)
'''
if s.count(old_store) != 1:
    raise SystemExit("setPolling store anchor mismatch")
s = s.replace(old_store, new_store)

stored_anchor = '''    private func storedPollingKeys() -> Set<String> {
'''
helpers = '''    private func standardSelectionExplicitlyEdited(vin: String) -> Bool {
        let values = UserDefaults.standard.dictionary(
            forKey: Self.standardSelectionExplicitDefaultsKey) ?? [:]
        return (values[vin] as? NSNumber)?.boolValue ?? false
    }

    private func markStandardSelectionExplicitlyEdited(vin: String) {
        guard vin.count == 17 else { return }
        let defaults = UserDefaults.standard
        var values = defaults.dictionary(
            forKey: Self.standardSelectionExplicitDefaultsKey) ?? [:]
        values[vin] = true
        defaults.set(values, forKey: Self.standardSelectionExplicitDefaultsKey)
    }

    private func recoverLegacyStandardPollingKeys(vin: String) -> Set<String> {
        var recovered = Set<String>()
        for module in pidConfigurationModules {
            if pidSelectionStore.hasSelection(
                forVIN: vin,
                controllerIdentifier: module.id
            ) {
                recovered.formUnion(pidSelectionStore.stableKeys(
                    forVIN: vin,
                    controllerIdentifier: module.id))
            }
        }

        /* The pre-vehicle-wide selector was global. It is safe to recover a
         * non-empty explicit legacy choice only when this installation has at
         * most one saved vehicle; otherwise ownership is genuinely ambiguous. */
        if recovered.isEmpty && vehicleProfileStore.savedProfiles.count <= 1 {
            recovered.formUnion(legacyGlobalPollingKeys())
        }
        return recovered
    }

''' + stored_anchor
if s.count(stored_anchor) != 1:
    raise SystemExit("storedPollingKeys anchor mismatch")
s = s.replace(stored_anchor, helpers)

old_existing = '''            if pidSelectionStore.hasSelection(
                forVIN: vin,
                controllerIdentifier: controllerID
            ) {
                return Set(pidSelectionStore.stableKeys(
                    forVIN: vin,
                    controllerIdentifier: controllerID))
            }
'''
new_existing = '''            if pidSelectionStore.hasSelection(
                forVIN: vin,
                controllerIdentifier: controllerID
            ) {
                let existing = Set(pidSelectionStore.stableKeys(
                    forVIN: vin,
                    controllerIdentifier: controllerID))
                if !existing.isEmpty || standardSelectionExplicitlyEdited(vin: vin) {
                    return existing
                }

                /* v0.7.191 could materialise an empty vehicle-wide selection
                 * before it had recovered the older explicit per-module/global
                 * choices. Heal that empty migration once, but never override
                 * a deliberate all-off choice made in the current model. */
                let recovered = recoverLegacyStandardPollingKeys(vin: vin)
                if !recovered.isEmpty {
                    pidSelectionStore.setStableKeys(
                        Array(recovered).sorted(),
                        forVIN: vin,
                        controllerIdentifier: controllerID)
                    return recovered
                }
                return existing
            }
'''
if s.count(old_existing) != 1:
    raise SystemExit("existing standard selection anchor mismatch")
s = s.replace(old_existing, new_existing)

old_marker_vars = '''            let standardSelectionCount = storedPollingKeys().count
            let standardSelectionScoped = selectionVIN.count == 17 &&
'''
new_marker_vars = '''            let standardSelectionKeys = storedPollingKeys().sorted()
            let standardSelectionCount = standardSelectionKeys.count
            let standardSelectionScoped = selectionVIN.count == 17 &&
'''
if s.count(old_marker_vars) != 1:
    raise SystemExit("marker variable anchor mismatch")
s = s.replace(old_marker_vars, new_marker_vars)

old_marker = '''                "standard_selection_count=\\(standardSelectionCount)\\n" +
                "standard_selection_scoped=\\(standardSelectionScoped)\\n"
'''
new_marker = '''                "standard_selection_count=\\(standardSelectionCount)\\n" +
                "standard_selection_keys=\\(standardSelectionKeys.joined(separator: ","))\\n" +
                "standard_selection_scoped=\\(standardSelectionScoped)\\n" +
                "recorded_samples=\\(recordedSampleCount)\\n"
'''
if s.count(old_marker) != 1:
    raise SystemExit("marker text anchor mismatch")
s = s.replace(old_marker, new_marker)
p.write_text(s)

t = Path("tests/test_pid_architecture.py")
r = t.read_text()
end = '''require(
    "Live polling will begin when the read-only module and fault census finishes." not in APP
    and "only for measurements enabled in PID Setup" in APP,
    "dashboard empty-state copy must not imply polling starts automatically",
)

print("MBLINK PID/module architecture verified")
'''
replacement = '''require(
    "Live polling will begin when the read-only module and fault census finishes." not in APP
    and "only for measurements enabled in PID Setup" in APP,
    "dashboard empty-state copy must not imply polling starts automatically",
)
require(
    "standardSelectionExplicitlyEdited(vin:" in MODEL
    and "recoverLegacyStandardPollingKeys(vin:" in MODEL
    and "if !existing.isEmpty || standardSelectionExplicitlyEdited(vin: vin)" in MODEL,
    "an automatically materialised empty standard selection must not mask older explicit choices",
)
require(
    '"recorded_samples=\\\\(recordedSampleCount)\\\\n"' in MODEL
    and "MBLINK_CI_SIMULATED_POLLING" in MODEL,
    "Apple simulated readiness must expose completed live-polling samples",
)

print("MBLINK PID/module architecture verified")
'''
if r.count(end) != 1:
    raise SystemExit("PID architecture tail mismatch")
t.write_text(r.replace(end, replacement))

w = Path(".github/workflows/ci.yml")
c = w.read_text()
launch = '''          launch_output="$(SIMCTL_CHILD_MBLINK_CI_SIMULATED_FLOW=1 xcrun simctl launch "$udid" com.github.The-First-Infiltrator.MBLINK)"
'''
launch2 = '''          launch_output="$(SIMCTL_CHILD_MBLINK_CI_SIMULATED_FLOW=1 SIMCTL_CHILD_MBLINK_CI_SIMULATED_POLLING=1 xcrun simctl launch "$udid" com.github.The-First-Infiltrator.MBLINK)"
'''
if c.count(launch) != 1:
    raise SystemExit("simctl launch anchor mismatch")
c = c.replace(launch, launch2)

gate = '''                 grep -Fq 'standard_selection_vin=WDD2073022F123456' "$marker" &&
                 grep -Fq 'standard_selection_count=0' "$marker" &&
                 grep -Fq 'standard_selection_scoped=true' "$marker"; then
'''
gate2 = '''                 grep -Fq 'standard_selection_vin=WDD2073022F123456' "$marker" &&
                 grep -Fq 'standard_selection_count=2' "$marker" &&
                 grep -Fq 'standard_selection_keys=obd2.engine.rpm,obd2.vehicle.speed' "$marker" &&
                 grep -Fq 'standard_selection_scoped=true' "$marker" &&
                 grep -Eq '^recorded_samples=[1-9][0-9]*$' "$marker"; then
'''
if c.count(gate) != 1:
    raise SystemExit("runtime marker gate mismatch")
c = c.replace(gate, gate2)
w.write_text(c)

project = Path("app/ios/MBLINK.xcodeproj/project.pbxproj")
x = project.read_text()
if x.count(OLD_LINK) != 4:
    raise SystemExit(f"expected four Xcode LINK revision occurrences, got {x.count(OLD_LINK)}")
x = x.replace(OLD_LINK, NEW_LINK)
if x.count("MARKETING_VERSION = 0.7.191;") != 4:
    raise SystemExit("unexpected MARKETING_VERSION occurrence count")
x = x.replace("MARKETING_VERSION = 0.7.191;", "MARKETING_VERSION = 0.7.192;")
project.write_text(x)

v = Path("VERSION")
if v.read_text().strip() != "0.7.191":
    raise SystemExit("unexpected MBLINK VERSION")
v.write_text("0.7.192\n")

h = Path("include/mblink/version.h")
hh = h.read_text()
needle = '#define MBLINK_VERSION_STRING "0.7.191"'
if hh.count(needle) != 1:
    raise SystemExit("unexpected MBLINK version header")
h.write_text(hh.replace(needle, '#define MBLINK_VERSION_STRING "0.7.192"'))
