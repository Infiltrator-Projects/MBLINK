#!/usr/bin/env python3
"""Verify stable PID architecture contracts without pinning source layout.

Detailed behaviour is covered by the compiled C tests and the iOS simulated-flow
smoke.  This guard only checks public ownership/API contracts and documentation,
so implementation can be split or moved without creating false CI failures.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


contract = " ".join(
    (ROOT / "docs/PID_ARCHITECTURE.md").read_text(encoding="utf-8").split()
)
for statement in (
    "Standard OBD choices use one VIN-scoped selection",
    "Mercedes choices remain VIN-and-module scoped",
    "Unknown or unresolved modules may still appear in the module list.",
    "They must not be assigned invented semantics.",
):
    require(statement in contract, f"missing PID architecture contract: {statement}")

obd_api = (ROOT / "include/mblink/obd2.h").read_text(encoding="utf-8")
require(
    "mblink_obd2_pid_definition_count" in obd_api,
    "public MBLINK OBD catalogue API is missing",
)

manufacturer_api = (ROOT / "include/mblink/mercedes_data_scan.h").read_text(
    encoding="utf-8"
)
require(
    "mblink_mercedes_data_runtime_candidate_identifier_count_for_route" in manufacturer_api,
    "public Mercedes runtime-candidate API is missing",
)

apple_api = (ROOT / "platform/apple/MBLinkDiagnosticsController.h").read_text(
    encoding="utf-8"
)
require(
    "@interface MBLinkDiagnosticsController : LinkProductDiagnosticsController" in apple_api,
    "Apple MBLINK diagnostics must extend LINK's product diagnostics controller",
)

app = (ROOT / "app/ios/MBLINK/MBLINKApp.swift").read_text(encoding="utf-8")
model = (ROOT / "app/ios/MBLINK/ConnectionViewModel.swift").read_text(
    encoding="utf-8"
)
controller = (ROOT / "platform/apple/MBLinkDiagnosticsController.m").read_text(
    encoding="utf-8"
)
require(
    "enableStarterStandardPIDs" not in app
    and "enableStarterStandardPIDs" not in model
    and "Enable starter readings" not in app,
    "PID Setup must not have a second starter-reading enable surface",
)
require(
    app.count("MBPIDSetupView()") == 1
    and 'MBHomeTile("PID Setup"' in app,
    "the main-screen PID Setup tile must be the only route into PID selection",
)
require(
    "MBMeasurementStartPanel" not in app
    and "MBPIDModuleSetupView" not in app,
    "no secondary PID-selection helper view may exist",
)
require(
    "setManufacturerLivePollingEnabled" not in apple_api
    and "manufacturerLivePollingEnabledForModuleIdentifier" not in apple_api
    and "setManufacturerLivePollingEnabled" not in controller,
    "Apple controller must not expose a blanket manufacturer polling toggle",
)
require(
    "override func productDashboardParameters" in model
    and "enabledDisplayParameters" in model
    and "dashboardSelectionStore" not in model,
    "MBLINK display membership must come only from PID Setup selections",
)
factory_start = app.index("private struct MBFactoryReadingsView")
factory_end = app.index(".mbDiagnosticScreen(\"Factory Readings\")", factory_start)
factory_view = app[factory_start:factory_end]
require(
    "discoverManufacturerData" in factory_view
    and "setManufacturerPIDSelection" not in factory_view
    and "setStandardPIDSelection" not in factory_view,
    "Factory Readings must stay manual and must never change live PID selections",
)
vehicle_start = app.index("private struct MBVehicleView")
vehicle_end = app.index("private struct MBModulesView", vehicle_start)
vehicle_view = app[vehicle_start:vehicle_end]
require(
    "MBPIDSetupView" not in vehicle_view
    and "PID Setup & Saved Vehicle" not in vehicle_view,
    "Vehicle screen must not expose PID-selection controls",
)
require(
    "MBModulesView" not in vehicle_view
    and 'Text("Control units")' not in vehicle_view,
    "Vehicle screen must not duplicate the dedicated Modules screen",
)
require(
    "Diagnostic details" not in vehicle_view
    and 'MBInfoRow(label: "Connection"' not in vehicle_view
    and 'MBInfoRow(label: "Fault records"' not in vehicle_view,
    "Vehicle screen must remain vehicle/profile identity only",
)
evidence_start = app.index("private struct MBEvidenceView")
evidence_end = app.index("private struct MBTestsView", evidence_start)
evidence_view = app[evidence_start:evidence_end]
require(
    'MBInfoRow(label: "Connection"' in evidence_view
    and 'MBInfoRow(label: "Vehicle profile"' in evidence_view
    and 'MBInfoRow(label: "Fault records"' in evidence_view
    and 'MBInfoRow(label: "Endpoint"' in evidence_view
    and 'MBInfoRow(label: "Identity sweep"' in evidence_view,
    "Evidence screen must own diagnostic/session details",
)
modules_start = app.index("private struct MBModulesView")
modules_end = app.index("private struct MBModuleDetailView", modules_start)
modules_view = app[modules_start:modules_end]
require(
    "MBPIDSetupView" not in modules_view
    and 'title: "PID setup"' not in modules_view
    and "Open Saved Vehicles & PIDs" not in modules_view
    and "PID configuration" not in modules_view,
    "Modules screen must remain ECU inventory only and must not expose PID setup",
)
live_start = app.index("private struct MBLiveDataView")
live_end = app.index("private struct MBDataTableView", live_start)
live_view = app[live_start:live_end]
require(
    "MBPIDSetupView" not in live_view
    and "Choose PIDs" not in live_view,
    "Live Data must only display live data and must not expose PID selection",
)

core = (ROOT / "src/core/mblink.c").read_text(encoding="utf-8")
require(
    '#include "../link/src/' not in core,
    "MBLINK core must not include LINK implementation sources directly",
)

print("MBLINK stable PID/module architecture contract verified")
