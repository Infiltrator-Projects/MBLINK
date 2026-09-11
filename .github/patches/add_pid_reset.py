from pathlib import Path

model = Path("app/ios/MBLINK/ConnectionViewModel.swift")
s = model.read_text()
anchor = '''    var configuredPollingCount: Int {
        let standard = storedPollingKeys().count
        let manufacturer = pidConfigurationModules.reduce(0) {
            $0 + manufacturerSelectionSet(moduleID: $1.id).count
        }
        return standard + manufacturer
    }
'''
addition = anchor + '''
    var pidConfigurationVehicleVIN: String? {
        effectivePIDConfigurationVIN
    }

    func resetPIDSelectionsForCurrentVehicle() {
        guard let vin = effectivePIDConfigurationVIN else { return }

        // Reset only live-polling choices. Keep the VIN, module map, fault
        // evidence and discovered capabilities intact so the vehicle does not
        // have to be rediscovered merely to repair bad selection state.
        pidSelectionStore.setStableKeys(
            [],
            forVIN: vin,
            controllerIdentifier: Self.standardSelectionControllerIdentifier)
        markStandardSelectionExplicitlyEdited(vin: vin)

        // Remove the entire per-VIN Mercedes selection subtree, including
        // selections for modules that are no longer present in the current map.
        let defaults = UserDefaults.standard
        var vehicles = defaults.dictionary(
            forKey: Self.manufacturerSelectionDefaultsKey) ?? [:]
        vehicles.removeValue(forKey: vin)
        defaults.set(vehicles, forKey: Self.manufacturerSelectionDefaultsKey)

        appliedPollingConfigurationKey = nil
        applyConfiguredPollingIfNeeded(force: true)
        refreshPresentation()
        refreshStandardState()
    }
'''
if s.count(anchor) != 1:
    raise SystemExit("configured polling count anchor mismatch")
s = s.replace(anchor, addition)
model.write_text(s)

app = Path("app/ios/MBLINK/MBLINKApp.swift")
a = app.read_text()
state_anchor = '''private struct MBPIDSetupView: View {
    @EnvironmentObject private var connection: ConnectionViewModel

    var body: some View {
'''
state_replacement = '''private struct MBPIDSetupView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @State private var showingResetConfirmation = false

    var body: some View {
'''
if a.count(state_anchor) != 1:
    raise SystemExit("PID Setup state anchor mismatch")
a = a.replace(state_anchor, state_replacement)

panel_anchor = '''                    MBPIDCatalogueSection(
                        title: "OBD / EOBD",
'''
panel = '''                    MBPanel {
                        VStack(alignment: .leading, spacing: 10) {
                            HStack {
                                VStack(alignment: .leading, spacing: 3) {
                                    Text("Polling selections")
                                        .font(MBTypography.subheadlineBold)
                                        .foregroundStyle(MBBrand.silverBright)
                                    Text("\\(connection.configuredPollingCount) enabled choice\\(connection.configuredPollingCount == 1 ? "" : "s") for this VIN")
                                        .font(MBTypography.caption)
                                        .foregroundStyle(MBBrand.silver)
                                }
                                Spacer(minLength: 8)
                                Text("\\(connection.configuredPollingCount) ON")
                                    .font(MBTypography.caption2Bold)
                                    .foregroundStyle(connection.configuredPollingCount == 0
                                                     ? MBBrand.muted : MBBrand.success)
                            }

                            if let vin = connection.pidConfigurationVehicleVIN {
                                Text(vin)
                                    .font(MBTypography.caption2.monospaced())
                                    .foregroundStyle(MBBrand.muted)
                            }

                            Button(role: .destructive) {
                                showingResetConfirmation = true
                            } label: {
                                Label("Reset PID selections for this VIN",
                                      systemImage: "arrow.counterclockwise.circle")
                                    .font(MBTypography.subheadlineBold)
                                    .frame(maxWidth: .infinity)
                                    .padding(.vertical, 9)
                            }
                            .buttonStyle(.bordered)
                            .disabled(connection.pidConfigurationVehicleVIN == nil)

                            Text("This keeps the saved VIN, control-unit map and diagnostic evidence. It only turns every Standard OBD and Mercedes live-polling choice OFF so you can select a clean set again.")
                                .font(MBTypography.caption)
                                .foregroundStyle(MBBrand.muted)
                                .fixedSize(horizontal: false, vertical: true)
                        }
                    }

                    MBPIDCatalogueSection(
                        title: "OBD / EOBD",
'''
if a.count(panel_anchor) != 1:
    raise SystemExit("PID Setup catalogue anchor mismatch")
a = a.replace(panel_anchor, panel)

screen_anchor = '''        .mbDiagnosticScreen("PID Setup")
    }
}

private struct MBPIDCatalogueSection: View {
'''
screen_replacement = '''        .mbDiagnosticScreen("PID Setup")
        .confirmationDialog(
            "Reset PID selections for this VIN?",
            isPresented: $showingResetConfirmation,
            titleVisibility: .visible
        ) {
            Button("Reset all PID selections", role: .destructive) {
                connection.resetPIDSelectionsForCurrentVehicle()
            }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("The saved vehicle and module discovery are retained. All live-polling selections for this VIN are switched OFF.")
        }
    }
}

private struct MBPIDCatalogueSection: View {
'''
if a.count(screen_anchor) != 1:
    raise SystemExit("PID Setup screen anchor mismatch")
a = a.replace(screen_anchor, screen_replacement)
app.write_text(a)

test = Path("tests/test_pid_architecture.py")
t = test.read_text()
print_anchor = '''print("MBLINK PID/module architecture verified")
'''
checks = '''require(
    "func resetPIDSelectionsForCurrentVehicle()" in MODEL
    and "vehicles.removeValue(forKey: vin)" in MODEL
    and "markStandardSelectionExplicitlyEdited(vin: vin)" in MODEL
    and "applyConfiguredPollingIfNeeded(force: true)" in MODEL,
    "VIN-scoped PID reset must clear standard/manufacturer selections without deleting the vehicle profile",
)
require(
    "Reset PID selections for this VIN" in APP
    and "The saved vehicle and module discovery are retained" in APP,
    "PID Setup must expose a safe polling-selection reset without deleting vehicle evidence",
)

''' + print_anchor
if t.count(print_anchor) != 1:
    raise SystemExit("PID architecture print anchor mismatch")
test.write_text(t.replace(print_anchor, checks))
