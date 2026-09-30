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
    "Standard OBD choices are VIN-and-controller scoped",
    "Mercedes choices remain VIN-and-controller scoped",
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
ecu_pack_api = (ROOT / "include/mblink/mercedes_ecu_pack.h").read_text(
    encoding="utf-8"
)
ecu_pack = (ROOT / "src/mercedes/ecu_pack.c").read_text(encoding="utf-8")
require(
    "mblink_mercedes_data_runtime_candidate_identifier_count_for_route" in manufacturer_api,
    "public Mercedes runtime-candidate API is missing",
)
require(
    "MblinkMercedesEcuPack" in ecu_pack_api
    and "mblink_mercedes_ecu_pack_resolve_module" in ecu_pack_api
    and "mblink_mercedes_ecu_pack_data_item_at" in ecu_pack_api,
    "Mercedes ECU identity, route, protocol and data metadata must have one public pack view",
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
    "MBDieselView" not in app
    and "Factory reference" not in app
    and "Factory Reference" not in app,
    "obsolete Factory Reference UI must not exist",
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

documented_defs_start = controller.index(
    "documentedDataDefinitionsForModuleIdentifier:"
)
documented_defs_end = controller.index(
    "loadSavedVehicleProfileForPIDConfiguration:", documented_defs_start
)
documented_defs = controller[documented_defs_start:documented_defs_end]
require(
    "mblink_mercedes_documented_route_read_" not in documented_defs
    and "mblink_mercedes_route_evidence_identifier_" not in documented_defs,
    "PID catalogue must come from the identified ECU profile, not route fallbacks",
)
require(
    "pollable" not in model
    and "pollable" not in app
    and "documentedDefinitions.map" in model
    and ".filter { selected.contains($0.selectionKey) }" in model,
    "iPhone PID Setup must be documentation-driven with no separate pollable gate",
)
require(
    "transmissionLiveValueSnapshots(" in model
    and "identifiers: Array(selected)" in model
    and ".filter { selected.contains($0.identifier) }" not in model,
    "Mercedes grouped records must receive logical selection before decoding rather than filtering a fully decoded record",
)
require(
    "readinessFieldSnapshots" in model
    and "readinessFieldSnapshots(" in model
    and "forResponderCANIdentifier:" in model
    and "setPollingFieldMask" in model
    and "readinessFieldMask(for: selection)" in model
    and "one shared 01 01 request" in model,
    "standard grouped PID fields must be independently selectable per responder while sharing one functional source request",
)
require(
    "documentedLiveIdentifiersForModuleIdentifier" not in controller
    and "documentedPIDCommandsForModuleIdentifier" in controller
    and "if (!definition.live) continue;" not in controller,
    "iPhone polling must attempt every selected documented read without a hidden live/pollable gate",
)
require(
    "MBLinkManufacturerCommandToken" in controller
    and "MBLinkDecodeManufacturerCommandToken" in controller
    and "mblink_mercedes_data_scan_begin_documented_commands" in controller
    and "mblink_mercedes_data_scan_begin_identifiers(" not in controller
    and "UInt32($0.service) << 16 | UInt32($0.identifier)" in model
    and "setManufacturerLivePollingCommands" in model,
    "manufacturer polling must preserve service plus identifier so KWP 1A and 21 reads cannot collapse to the wrong wire command",
)
require(
    "mblink_mercedes_documented_read_is_safe(" in ecu_pack
    and "entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED" in ecu_pack
    and "MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED" in ecu_pack
    and "item->advertised = item->live" in ecu_pack,
    "PID Setup must expose every safe source-backed exact-controller read while capture-only raw evidence stays out",
)
require(
    "mblink.manufacturer.pidCatalogueByVehicle.v4" in model
    and "mblink.manufacturer.pidCatalogueByVehicle.v3" not in model,
    "legacy pollable/read-only manufacturer PID catalogue caches must not survive the documented-PID policy change",
)
require(
    "let documentedDefinitions = controller.documentedDataDefinitions(" in model
    and "if !documentedDefinitions.isEmpty {" in model
    and "else if controller.isActive {" in model
    and "let sanitized = selected.intersection(documentedStableKeys)" in model,
    "the exact identified ECU's documented PID catalogue must be authoritative and stale selections must be pruned",
)
saved_marker_start = model.index("private func writeSavedPIDCatalogueRegressionMarker()")
saved_marker_end = model.index("#endif", saved_marker_start)
saved_marker = model[saved_marker_start:saved_marker_end]
require(
    "manufacturerPIDCatalogueItems(" in saved_marker
    and "controller.documentedDataDefinitions(" not in saved_marker,
    "saved-profile regression must consume the same cached documented PID catalogue as live PID Setup",
)
require(
    "runtimeCandidateIdentifiersForModule" not in controller
    and "mblink_mercedes_data_scan_begin_probe_identifiers" not in controller
    and "mblink_mercedes_data_scan_begin_probe_commands" not in controller
    and "mblink_mercedes_data_scan_begin(" not in controller,
    "Apple runtime must not probe or brute-force manufacturer PID candidates",
)
require(
    "mblink_mercedes_documented_route_read_" not in controller,
    "Apple manufacturer reads must not use route-wide documented unions",
)
require(
    "persistedManufacturerDataIdentifiersForModule" not in controller
    and "runtimeManufacturerDataIdentifiersForModule" not in controller
    and "targetedRefresh" not in controller,
    "Apple manufacturer reads must not reuse legacy discovered PID lists",
)
require(
    "Body control unit" not in model
    and "resolvedMercedesModuleName(" in model,
    "saved/offline modules must use the shared Mercedes ECU identity resolver",
)
require(
    "KWP2000 / SAE OBD-II" not in model
    and 'case 0: return "UDS"' in model
    and 'case 1: return "KWP2000"' in model,
    "Mercedes module protocol labels must stay separate from standard OBD-II",
)
require(
    "ONLINE SOURCE INCOMPLETE" not in app
    and "SOURCE PARTIAL" not in app
    and "READ ONLY" not in app
    and "manufacturerPIDCatalogueComplete" not in model
    and "online_catalogue_complete" not in ecu_pack_api,
    "iPhone PID Setup must not invent source-completeness or pollability states",
)
require(
    "item.sourceText" in app
    and "No OBD-II or documented Mercedes data is currently available for this controller." in app
    and "item.selectionKey" in app,
    "controller PID rows must preserve independent source-labelled OBD-II and documented Mercedes selections",
)
require(
    "Legislated OBD-II responders are intentionally not represented" in controller
    and "controllerPIDCatalogueItems" in model
    and "scopedChannelID" in model
    and "moduleStandardSelectionSet" in model
    and "aggregateStandardPollingKeys" in model,
    "Mercedes Modules must remain identity-driven while PID Setup attaches exact-responder OBD channels to their physical controller",
)
require(
    'title: "OBD / EOBD"' not in app
    and "controllerPIDCatalogueItems(" in app
    and "item.sourceText" in app,
    "PID Setup must be controller-first and show OBD-II/KWP2000/UDS source labels inside each controller",
)
require(
    "func standardPIDCatalogueItems(" in model
    and "pidSupportByModule[moduleID]" in model
    and "Advertised by this controller" in model,
    "controller OBD catalogue membership must come from that responder's advertised/cached capability map",
)
require(
    '"controllerFamily"' in controller
    and '"name"' in controller,
    "saved VIN profiles must persist resolved Mercedes ECU identity",
)

core = (ROOT / "src/core/mblink.c").read_text(encoding="utf-8")
require(
    '#include "../link/src/' not in core,
    "MBLINK core must not include LINK implementation sources directly",
)

print("MBLINK stable PID/module architecture contract verified")
