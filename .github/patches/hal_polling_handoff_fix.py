from pathlib import Path

OLD_LINK = '63ab72b8a2dad1f901c8b9a598dcf731344554c0'
NEW_LINK = 'c25568552d75cb6833812cab354ff849ed3cc817'

# Swift: re-arm the exact VIN-scoped selection once the real scheduler is ready.
p = Path('app/ios/MBLINK/ConnectionViewModel.swift')
s = p.read_text()
anchor = '    private var appliedPollingConfigurationKey: String?\n'
replacement = anchor + '    private var livePollingReadyRearmSignature: String?\n'
if s.count(anchor) != 1:
    raise SystemExit(f'appliedPollingConfigurationKey anchor count={s.count(anchor)}')
s = s.replace(anchor, replacement)

apply_anchor = '''    private func applyConfiguredPollingIfNeeded(force: Bool = false) {
'''
helper = '''    private func livePollingSelectionSignature(vin: String) -> String {
        let standard = storedPollingKeys().sorted().joined(separator: ",")
        let manufacturer = pidConfigurationModules.map { module in
            let keys = manufacturerSelectionSet(moduleID: module.id)
                .sorted().joined(separator: ",")
            return "\\(module.id)=\\(keys)"
        }.sorted().joined(separator: "|")
        return "\\(vin)|standard=\\(standard)|manufacturer=\\(manufacturer)"
    }

''' + apply_anchor
if s.count(apply_anchor) != 1:
    raise SystemExit('applyConfiguredPollingIfNeeded anchor mismatch')
s = s.replace(apply_anchor, helper)

hook_anchor = '''        diagnosticModules = isActive ? loadDiagnosticModules() : []
        refreshPIDConfiguration()
        applyConfiguredPollingIfNeeded()
#if MBLINK_CI_SIMULATED_FLOW
'''
hook_replacement = '''        diagnosticModules = isActive ? loadDiagnosticModules() : []
        refreshPIDConfiguration()

        /*
         * A live connection deliberately suppresses offline selections until
         * its VIN is known. Re-apply that VIN's complete saved selection once
         * LINK has finished building the real live scheduler. This is a
         * one-shot per selection fingerprint, so delegate refreshes cannot
         * recurse, but changing an 84-PID setup to an 85-PID setup re-arms the
         * scheduler immediately as well.
         */
        if isActive, isReady, let vin = activeVehicleVIN {
            let signature = livePollingSelectionSignature(vin: vin)
            if signature != livePollingReadyRearmSignature {
                livePollingReadyRearmSignature = signature
                appliedPollingConfigurationKey = nil
                applyConfiguredPollingIfNeeded(force: true)
            }
        } else {
            livePollingReadyRearmSignature = nil
        }
        applyConfiguredPollingIfNeeded()
#if MBLINK_CI_SIMULATED_FLOW
'''
if s.count(hook_anchor) != 1:
    raise SystemExit(f'product refresh hook count={s.count(hook_anchor)}')
s = s.replace(hook_anchor, hook_replacement)
p.write_text(s)

# Objective-C: never let a Mercedes background job be overdue before standard
# live polling has reached readiness. Register it at/after ready instead.
cpath = Path('platform/apple/MBLinkDiagnosticsController.m')
c = cpath.read_text()
old_gate = '''- (void)updateScheduledManufacturerLiveJob
{
    if (!_shared.isActive) return;
'''
new_gate = '''- (void)updateScheduledManufacturerLiveJob
{
    /*
     * Do not reserve a recurring Mercedes slot during startup discovery.
     * Standard OBD must reach its real live scheduler first; otherwise an
     * already-overdue manufacturer job can become the first LIVE action and
     * hide a broken standard-polling handoff.
     */
    if (!_shared.isActive || !_shared.isReady) return;
'''
if c.count(old_gate) != 1:
    raise SystemExit('manufacturer ready gate anchor mismatch')
c = c.replace(old_gate, new_gate)

event_anchor = '''    (void)controller;
    if (event == NULL) return;
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE) {
'''
event_replacement = '''    (void)controller;
    if (event == NULL) return;
    if (event->became_ready) {
        [self updateScheduledManufacturerLiveJob];
    }
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE) {
'''
if c.count(event_anchor) != 1:
    raise SystemExit('flow event anchor mismatch')
c = c.replace(event_anchor, event_replacement)
cpath.write_text(c)

# Keep the runtime selected-polling proof in normal CI permanently.
wpath = Path('.github/workflows/ci.yml')
w = wpath.read_text()
launch = '''          launch_output="$(SIMCTL_CHILD_MBLINK_CI_SIMULATED_FLOW=1 xcrun simctl launch "$udid" com.github.The-First-Infiltrator.MBLINK)"
'''
launch2 = '''          launch_output="$(SIMCTL_CHILD_MBLINK_CI_SIMULATED_FLOW=1 SIMCTL_CHILD_MBLINK_CI_SIMULATED_POLLING=1 xcrun simctl launch "$udid" com.github.The-First-Infiltrator.MBLINK)"
'''
if w.count(launch) != 1:
    raise SystemExit(f'simulator launch anchor count={w.count(launch)}')
w = w.replace(launch, launch2)
old_gate = '''                 grep -Fq 'standard_selection_count=0' "$marker" &&
                 grep -Fq 'standard_selection_scoped=true' "$marker"; then
'''
new_gate = '''                 grep -Fq 'standard_selection_count=2' "$marker" &&
                 grep -Fq 'standard_selection_keys=obd2.engine.rpm,obd2.vehicle.speed' "$marker" &&
                 grep -Fq 'standard_selection_scoped=true' "$marker" &&
                 grep -Eq '^recorded_samples=[1-9][0-9]*$' "$marker"; then
'''
if w.count(old_gate) != 1:
    raise SystemExit(f'simulator marker gate count={w.count(old_gate)}')
wpath.write_text(w.replace(old_gate, new_gate))

# Architecture guards for the exact regression.
tpath = Path('tests/test_pid_architecture.py')
t = tpath.read_text()
anchor = '''require(
    "Reset PID selections for this VIN" in APP
    and "The saved vehicle and module discovery are retained" in APP,
    "PID Setup must expose a safe polling-selection reset without deleting vehicle evidence",
)

print("MBLINK PID/module architecture verified")
'''
addition = '''require(
    "Reset PID selections for this VIN" in APP
    and "The saved vehicle and module discovery are retained" in APP,
    "PID Setup must expose a safe polling-selection reset without deleting vehicle evidence",
)
require(
    "livePollingReadyRearmSignature" in MODEL
    and "if isActive, isReady, let vin = activeVehicleVIN" in MODEL
    and "applyConfiguredPollingIfNeeded(force: true)" in MODEL,
    "a live VIN selection must be re-applied after LINK builds the real scheduler",
)
require(
    "if (!_shared.isActive || !_shared.isReady) return;" in CONTROLLER
    and "if (event->became_ready)" in CONTROLLER
    and "[self updateScheduledManufacturerLiveJob];" in CONTROLLER,
    "Mercedes recurring jobs must not reserve the first live slot before standard readiness",
)

print("MBLINK PID/module architecture verified")
'''
if t.count(anchor) != 1:
    raise SystemExit('PID architecture tail mismatch')
tpath.write_text(t.replace(anchor, addition))

# Pin the exact released LINK candidate and bump MBLINK.
proj = Path('app/ios/MBLINK.xcodeproj/project.pbxproj')
x = proj.read_text()
if x.count(OLD_LINK) != 2:
    raise SystemExit(f'expected two Xcode LINK revision occurrences, got {x.count(OLD_LINK)}')
x = x.replace(OLD_LINK, NEW_LINK)
if x.count('MARKETING_VERSION = 0.7.192;') != 4:
    raise SystemExit(f'unexpected marketing version count={x.count("MARKETING_VERSION = 0.7.192;")}')
x = x.replace('MARKETING_VERSION = 0.7.192;', 'MARKETING_VERSION = 0.7.193;')
proj.write_text(x)

v = Path('VERSION')
if v.read_text().strip() != '0.7.192':
    raise SystemExit('unexpected MBLINK VERSION')
v.write_text('0.7.193\n')

h = Path('include/mblink/version.h')
hh = h.read_text()
needle = '#define MBLINK_VERSION_STRING "0.7.192"'
if hh.count(needle) != 1:
    raise SystemExit('unexpected MBLINK version header')
h.write_text(hh.replace(needle, '#define MBLINK_VERSION_STRING "0.7.193"'))
