#!/usr/bin/env python3
"""Guard the product-level PID/module architecture implemented by Swift.

The hosted Apple job still compiles and launches the application.  This test
protects the hierarchy and ownership decisions that a compile-only check cannot
see: one vehicle-wide SAE catalogue first, followed by Mercedes catalogues for
the discovered/saved module map.
"""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
APP = (ROOT / "app/ios/MBLINK/MBLINKApp.swift").read_text(encoding="utf-8")
MODEL = (ROOT / "app/ios/MBLINK/ConnectionViewModel.swift").read_text(
    encoding="utf-8"
)
CONTROLLER = (ROOT / "platform/apple/MBLinkDiagnosticsController.m").read_text(
    encoding="utf-8"
)
CONTRACT = (ROOT / "docs/PID_ARCHITECTURE.md").read_text(encoding="utf-8")
CONTRACT_WORDS = " ".join(CONTRACT.split())


def section(source: str, start: str, end: str) -> str:
    start_index = source.index(start)
    end_index = source.index(end, start_index + len(start))
    return source[start_index:end_index]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


setup = section(APP, "private struct MBPIDSetupView", "private struct MBPIDCatalogueSection")
standard_catalogue = section(
    MODEL, "func standardPIDCatalogueItems()", "func manufacturerPIDCatalogueItems"
)
manufacturer_catalogue = section(
    MODEL, "func manufacturerPIDCatalogueItems", "func setStandardPIDSelection"
)
module_detail = section(
    APP, "private struct MBModuleDetailView", "private struct MBFaultsView"
)
manufacturer_extension = section(
    (ROOT / "platform/apple/MBLinkDiagnosticsController.m").read_text(
        encoding="utf-8"
    ),
    "- (void)linkDiagnosticsControllerBeginManufacturerExtension:",
    "- (void)linkDiagnosticsController:(LinkDiagnosticsController *)controller\n  beginScheduledManufacturerJob:",
)

standard_position = setup.index("items: connection.standardPIDCatalogueItems()")
module_loop_position = setup.index("ForEach(connection.pidConfigurationModules)")
manufacturer_position = setup.index("connection.manufacturerPIDCatalogueItems(")

require(
    standard_position < module_loop_position < manufacturer_position,
    "PID Setup must show one Standard OBD catalogue before Mercedes module sections",
)
require(
    'addressText: "Legislated OBD · vehicle-wide"' in setup,
    "the Standard OBD catalogue must remain explicitly vehicle-wide",
)
require(
    "mblink_obd2_pid_definition_count()" in standard_catalogue,
    "the Standard OBD section must enumerate LINK's complete compiled catalogue",
)
require(
    "pidSupportByModule.values.flatMap" in standard_catalogue,
    "responder capability must remain metadata on the vehicle-wide SAE catalogue",
)
require(
    "controller.documentedDataDefinitions(" in manufacturer_catalogue,
    "Mercedes module sections must use documented manufacturer definitions",
)
require(
    "modulePIDSelectionSet" not in MODEL and "pidConfigurationItems(moduleID" not in MODEL,
    "the superseded responder-scoped SAE configuration model must not return",
)
require(
    'standardSelectionControllerIdentifier = "standard-obd"' in MODEL,
    "Standard OBD selection must remain one VIN-scoped selection",
)
require(
    "setManufacturerPIDSelection(" in APP and "setStandardPIDSelection(" in APP,
    "standard and manufacturer toggles must remain separate",
)
require(
    "connection.standardLiveRows" in APP
    and "connection.standardLiveValueRows" not in APP,
    "the iOS OBD screen must consume the view model's published live rows",
)
require(
    "discoverManufacturerData" not in setup and "rescanManufacturerData" not in setup,
    "opening PID Setup must not launch manufacturer discovery",
)
require(
    "setStandardPIDSelection" not in module_detail
    and "setManufacturerPIDSelection" not in module_detail
    and "Poll live factory data" not in module_detail,
    "PID Setup must remain the only polling-configuration surface",
)
require(
    "[self beginMercedesModuleScan];" in manufacturer_extension
    and "[self beginMercedesProbe];" not in manufacturer_extension,
    "normal Connect must identify modules without a preceding engine fingerprint sweep",
)
require(
    "Standard OBD choices use one VIN-scoped selection" in CONTRACT_WORDS,
    "the canonical architecture contract must state Standard OBD selection scope",
)
require(
    "Mercedes choices remain VIN-and-module scoped" in CONTRACT_WORDS,
    "the canonical architecture contract must state Mercedes selection scope",
)

require(
    "controller.mercedesModuleSnapshots.map" in MODEL,
    "every retained Mercedes/OBD module snapshot must remain visible to the Swift model",
)
require(
    "for (responderKey, pids) in responderPIDs" in MODEL
    and "where !seenResponderKeys.contains(responderKey)" in MODEL,
    "saved VIN profiles must retain standards responders even before Mercedes identity exists",
)
require(
    "for (uint32_t responseID = UINT32_C(0x7e8);" in CONTROLLER
    and "responseID <= UINT32_C(0x7ef);" in CONTROLLER
    and "Live responder observed · module fault state not established" in CONTROLLER,
    "live 0x7E8-0x7EF OBD responders must remain in the module map when manufacturer probing is quiet",
)
require(
    "controller.isActive ? activeVehicleVIN : selectedVehicleVIN" in MODEL,
    "live VIN must remain authoritative over a previously selected offline profile",
)
require(
    "let modules = vehicles[vin] as? [String: Any]" in MODEL
    and "modules[moduleID] = Array(selection).sorted()" in MODEL
    and "vehicles[vin] = modules" in MODEL,
    "Mercedes polling selections must remain scoped by VIN and module",
)
require(
    "Unknown or unresolved modules may still appear in the module list." in CONTRACT_WORDS
    and "They must not be assigned invented semantics." in CONTRACT_WORDS,
    "unknown modules must remain visible without invented PID meanings",
)
require(
    "SAE PIDs actually advertised by each controller" not in APP
    and "vehicle-wide SAE PIDs and documented Mercedes data" in APP,
    "module-screen PID Setup wording must match the vehicle-wide SAE architecture",
)
require(
    "Live polling will begin when the read-only module and fault census finishes." not in APP
    and "only for measurements enabled in PID Setup" in APP,
    "dashboard empty-state copy must not imply polling starts automatically",
)

print("MBLINK PID/module architecture verified")
