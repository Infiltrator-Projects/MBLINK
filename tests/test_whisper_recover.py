#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Regression test for the offline Whisper mapping-recovery helper."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
TOOL = Path(os.environ.get("MBLINK_WHISPER_TOOL", ROOT / "tools" / "mblink-whisper-recover.py"))

SYNTHETIC = """\
DEV.Motor.type = PduTransceive
DEV.Motor.channel.baudrate = 500k
DEV.Motor.channel.transmit_address = 0x7e0
DEV.Motor.channel.receive_address = 0x7e8
DEV.Motor.intakeManifoldPressure.default.requestid = REQ_IMP
REQUESTID.REQ_IMP.resultid = RES_IMP
REQUESTID.REQ_IMP.pdu = 22,20,01
REQUESTID.REQ_IMP.matching_response = 62,20,01
REQUESTID.REQ_IMP.timeout = 1050
RESULTID.RES_IMP.intakeManifoldPressure.responseparam = <PDU>
RESULTID.RES_IMP.intakeManifoldPressure.param.0.extract = BYTE[3,2]
RESULTID.RES_IMP.intakeManifoldPressure.param.0.encoding = UINT16
RESULTID.RES_IMP.intakeManifoldPressure.param.0.formula = x*0.01
DATAID.intakeManifoldPressure.datatype = Double
DATAID.intakeManifoldPressure.unit = bar
"""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    require(TOOL.is_file(), f"missing recovery tool: {TOOL}")
    with tempfile.TemporaryDirectory(prefix="mblink-whisper-test-") as directory:
        sample = Path(directory) / "vehicle_configs.properties"
        sample.write_text(SYNTHETIC, encoding="utf-8")
        proc = subprocess.run(
            [
                sys.executable,
                str(TOOL),
                str(sample),
                "--json",
                "--target",
                "intakeManifoldPressure",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        require(proc.returncode == 0, proc.stderr or proc.stdout)
        report = json.loads(proc.stdout)
        require(report["mapping_count"] == 1, "expected one reconstructed mapping")
        mapping = report["mappings"][0]
        require(mapping["evidence_state"] == "source-backed-candidate", "wrong evidence state")
        require(mapping["device"] == "Motor", "wrong device")
        require(mapping["provider_type"] == "PduTransceive", "wrong provider type")
        require(mapping["channel"]["transmit_address"] == "0x7e0", "wrong TX route")
        require(mapping["channel"]["receive_address"] == "0x7e8", "wrong RX route")
        require(mapping["request_id"] == "REQ_IMP", "wrong request id")
        require(mapping["request"]["pdu"] == "22,20,01", "wrong request PDU")
        require(mapping["request"]["matching_response"] == "62,20,01", "wrong response match")
        require(mapping["result_id"] == "RES_IMP", "wrong result id")
        require(mapping["result"]["param.0.extract"] == "BYTE[3,2]", "wrong extraction")
        require(mapping["result"]["param.0.formula"] == "x*0.01", "wrong formula")
        require(mapping["data"]["datatype"] == "Double", "wrong datatype")
        require(mapping["data"]["unit"] == "bar", "wrong unit")

        baseline = Path(directory) / "MSA_VIN_cascade.properties"
        baseline.write_text(SYNTHETIC, encoding="utf-8")
        baseline_proc = subprocess.run(
            [sys.executable, str(TOOL), str(baseline), "--json"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        require(baseline_proc.returncode == 0, baseline_proc.stderr or baseline_proc.stdout)
        baseline_report = json.loads(baseline_proc.stdout)
        require(baseline_report["mapping_count"] == 0, "baseline mapping must be suppressed")

    print("Whisper mapping recovery regression passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
