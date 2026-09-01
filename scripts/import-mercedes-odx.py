#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Extract Mercedes DID research candidates from LINK-normalised ODX JSON."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def extract_candidates(document: dict[str, Any]) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    seen: set[tuple[str, int]] = set()
    for ecu in document.get("ecus", []):
        ecu_name = ecu.get("short_name") or "unknown"
        for service in ecu.get("services", []):
            prefix = service.get("request_coded_prefix")
            if not isinstance(prefix, str):
                continue
            compact = "".join(prefix.split()).upper()
            if len(compact) < 6 or not compact.startswith("22"):
                continue
            try:
                did = int(compact[2:6], 16)
            except ValueError:
                continue
            identity = (ecu_name, did)
            if identity in seen:
                continue
            seen.add(identity)
            candidates.append({
                "ecu_variant": ecu_name,
                "can_receive_id": ecu.get("can_receive_id"),
                "can_send_id": ecu.get("can_send_id"),
                "service": service.get("short_name"),
                "did": f"0x{did:04X}",
                "request_prefix": compact,
                "status": "source-backed-candidate",
                "automatic_polling": False,
                "decode_ready": False,
                "policy_note": (
                    "ODX proves a described request identity, not vehicle "
                    "support, response scaling or safe automatic polling."
                ),
            })
    return candidates


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Extract non-auto-polling Mercedes DID candidates from LINK ODX JSON")
    parser.add_argument("input", type=Path,
                        help="JSON produced by LINK scripts/import-odx.py")
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    document = json.loads(args.input.read_text(encoding="utf-8"))
    if document.get("schema_version") != 1 or "ecus" not in document:
        parser.error("input is not a LINK ODX schema_version 1 document")

    result = {
        "schema_version": 1,
        "source_file": document.get("source_file"),
        "source_odxtools_version": document.get("odxtools_version"),
        "policy": {
            "automatic_polling": False,
            "promotion_requires_vehicle_verification": True,
        },
        "candidates": extract_candidates(document),
    }
    encoded = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
