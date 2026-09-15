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

core = (ROOT / "src/core/mblink.c").read_text(encoding="utf-8")
require(
    '#include "../link/src/' not in core,
    "MBLINK core must not include LINK implementation sources directly",
)

print("MBLINK stable PID/module architecture contract verified")
