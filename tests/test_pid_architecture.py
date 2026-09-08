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

print("MBLINK PID/module architecture verified")
