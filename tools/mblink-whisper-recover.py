#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Read-only forensic scanner/parser for Mercedes me Whisper configuration artifacts."""

from __future__ import annotations

import argparse
import binascii
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import zipfile

WHISPER_KEY_HEX = "704668747a2f62464670392c25366451"
MAX_READ = 4 * 1024 * 1024
MAX_ZIP_MEMBER = 8 * 1024 * 1024
MAX_ZIP_FILES = 2000

STRONG = {
    b"actionproviders": 12,
    b"deviceproviders": 8,
    b"vinmapping": 6,
    b"activeconfiguration": 6,
    b"_configs": 6,
    b"REQUESTID.": 12,
    b"RESULTID.": 12,
    b"DATAID.": 8,
    b"matching_response": 10,
    b"deviceprovider": 7,
    b"formula": 5,
}
TARGETS = {
    b"intakeManifoldPressure": 10,
    b"engineFuelRate": 10,
    b"actualFuelFlow": 10,
    b"particleFilter": 10,
    b"engineOilTemperature": 6,
    b"fuelPressureCan": 6,
    b"boostPressureCan": 4,
}
PATH_HINTS = {
    "/incoming/": 16,
    "/actionproviders/": 12,
    "/deviceproviders/": 8,
    "/vinmapping/": 6,
    "_configs": 10,
}
BASELINE_NAMES = {
    "MSA_VIN_cascade.properties",
    "VinMapping.properties",
    "Availability.properties",
    "ObdSupportInfo.properties",
    "TripEvents.properties",
}
TEXT_SUFFIXES = (".properties", ".cfg", ".txt", ".json", ".plain")


def read_prefix(path: Path, limit: int = MAX_READ) -> bytes:
    try:
        with path.open("rb") as handle:
            return handle.read(limit)
    except OSError:
        return b""


def looks_hex_ciphertext(data: bytes) -> bool:
    compact = b"".join(data.split())
    if len(compact) < 32 or len(compact) % 32:
        return False
    try:
        binascii.unhexlify(compact)
    except (binascii.Error, ValueError):
        return False
    return True


def decrypt_whisper_hex(data: bytes) -> bytes | None:
    if not looks_hex_ciphertext(data):
        return None
    openssl = shutil.which("openssl")
    if not openssl:
        return None
    try:
        raw = binascii.unhexlify(b"".join(data.split()))
        proc = subprocess.run(
            [
                openssl,
                "enc",
                "-d",
                "-aes-128-ecb",
                "-K",
                WHISPER_KEY_HEX,
                "-nosalt",
            ],
            input=raw,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            timeout=5,
            check=False,
        )
        return proc.stdout if proc.returncode == 0 and proc.stdout else None
    except (OSError, subprocess.SubprocessError, binascii.Error):
        return None


def score_blob(data: bytes) -> tuple[int, list[str]]:
    score = 0
    reasons: list[str] = []
    lowered = data.lower()
    for token, weight in {**STRONG, **TARGETS}.items():
        if token.lower() in lowered:
            score += weight
            reasons.append(token.decode("ascii", "replace"))
    return score, reasons


def normalise_baseline_name(name: str) -> str:
    leaf = Path(name).name
    if leaf.endswith(".plain"):
        leaf = leaf[:-6]
    return re.sub(r"\(\d+\)(?=\.[^.]+$)", "", leaf)


def is_baseline_source(source: str) -> bool:
    rendered = "/" + source.replace("!", "/").replace("\\", "/").lower().strip("/") + "/"
    if "/assets/whisper_parameterization/" in rendered:
        return True
    return normalise_baseline_name(source) in BASELINE_NAMES


def score_path(path: Path) -> tuple[int, list[str]]:
    rendered = "/" + path.as_posix().lower().strip("/") + "/"
    score = 0
    reasons: list[str] = []

    if "/assets/whisper_parameterization/" in rendered:
        score -= 60
        reasons.append("APK-bundled-baseline-path")

    for hint, weight in PATH_HINTS.items():
        if hint.lower() in rendered:
            score += weight
            reasons.append("path:" + hint.strip("/"))

    if normalise_baseline_name(path.name) in BASELINE_NAMES:
        score -= 100
        reasons.append("bundled-baseline-name")

    return score, reasons


def decode_property_blob(data: bytes) -> tuple[bytes, bool]:
    plaintext = decrypt_whisper_hex(data)
    if plaintext is not None:
        return plaintext, True
    return data, False


def inspect_zip(path: Path) -> tuple[int, list[str]]:
    score = 0
    reasons = ["ZIP"]
    try:
        with zipfile.ZipFile(path) as archive:
            names = archive.namelist()
            low_names = [name.lower() for name in names]

            if (
                low_names
                and all(
                    "assets/whisper_parameterization/" in name or name.endswith("/")
                    for name in low_names
                )
                and not any(
                    "actionproviders/" in name or "_configs" in name
                    for name in low_names
                )
            ):
                score -= 50
                reasons.append("APK-bundled-baseline-members")

            joined = "\n".join(names).encode("utf-8", "replace")
            part_score, part_reasons = score_blob(joined)
            score += part_score
            reasons += ["member:" + item for item in part_reasons]

            for name in names[:MAX_ZIP_FILES]:
                low = name.lower()
                if "/incoming/" in "/" + low or low.startswith("incoming/"):
                    score += 16
                    reasons.append("member:incoming")
                if "actionproviders/" in low:
                    score += 14
                    reasons.append("member:actionproviders")
                if "deviceproviders/" in low:
                    score += 10
                    reasons.append("member:deviceproviders")
                if "vinmapping/" in low:
                    score += 8
                    reasons.append("member:vinmapping")
                if "_configs" in low:
                    score += 10
                    reasons.append("member:_configs")

            for info in archive.infolist()[:500]:
                if info.flag_bits & 1 or info.file_size > MAX_ZIP_MEMBER:
                    continue
                if not info.filename.lower().endswith(TEXT_SUFFIXES):
                    continue
                try:
                    data = archive.read(info)[:MAX_READ]
                except (OSError, RuntimeError, zipfile.BadZipFile):
                    continue
                data, _ = decode_property_blob(data)
                part_score, part_reasons = score_blob(data)
                score += part_score
                reasons += [f"{info.filename}:{item}" for item in part_reasons]
    except (OSError, zipfile.BadZipFile, zipfile.LargeZipFile):
        return 0, []

    return score, reasons


def inspect_file(path: Path) -> tuple[int, list[str]]:
    path_score, path_reasons = score_path(path)
    data = read_prefix(path)

    if data.startswith(b"PK\x03\x04") or path.suffix.lower() == ".zip":
        zip_score, zip_reasons = inspect_zip(path)
        return path_score + zip_score, path_reasons + zip_reasons

    if not path.name.lower().endswith(TEXT_SUFFIXES):
        return path_score, path_reasons

    data, decrypted = decode_property_blob(data)
    blob_score, blob_reasons = score_blob(data)
    if decrypted:
        return path_score + blob_score + 4, path_reasons + ["Whisper-AES-properties"] + blob_reasons
    return path_score + blob_score, path_reasons + blob_reasons


def iter_files(root: Path):
    if root.is_file():
        yield root
        return

    for base, dirs, files in os.walk(root, followlinks=False):
        dirs[:] = [
            name
            for name in dirs
            if name not in {".git", "node_modules", "proc", "sys", "dev"}
        ]
        for name in files:
            path = Path(base) / name
            try:
                if path.is_symlink() or path.stat().st_size > 512 * 1024 * 1024:
                    continue
            except OSError:
                continue
            yield path


def iter_property_sources(path: Path):
    data = read_prefix(path)
    if data.startswith(b"PK\x03\x04") or path.suffix.lower() == ".zip":
        try:
            with zipfile.ZipFile(path) as archive:
                for info in archive.infolist()[:MAX_ZIP_FILES]:
                    if info.is_dir() or info.flag_bits & 1 or info.file_size > MAX_ZIP_MEMBER:
                        continue
                    if not info.filename.lower().endswith(TEXT_SUFFIXES):
                        continue
                    try:
                        member = archive.read(info)[:MAX_READ]
                    except (OSError, RuntimeError, zipfile.BadZipFile):
                        continue
                    member, decrypted = decode_property_blob(member)
                    yield f"{path}!{info.filename}", member, decrypted
        except (OSError, zipfile.BadZipFile, zipfile.LargeZipFile):
            return
        return

    if path.name.lower().endswith(TEXT_SUFFIXES):
        data, decrypted = decode_property_blob(data)
        yield str(path), data, decrypted


def _logical_property_lines(text: str):
    pending = ""
    for physical in text.splitlines():
        line = pending + physical
        stripped = line.rstrip()
        slash_count = len(stripped) - len(stripped.rstrip("\\"))
        if slash_count % 2 == 1:
            pending = stripped[:-1]
            continue
        pending = ""
        yield line
    if pending:
        yield pending


def parse_properties(data: bytes) -> dict[str, str]:
    props: dict[str, str] = {}
    text = data.decode("utf-8", "replace")
    for raw in _logical_property_lines(text):
        line = raw.strip()
        if not line or line.startswith(("#", "!")):
            continue
        if "=" in line:
            key, value = line.split("=", 1)
        elif ":" in line:
            key, value = line.split(":", 1)
        else:
            match = re.match(r"([^\s]+)\s+(.+)", line)
            if not match:
                continue
            key, value = match.group(1), match.group(2)
        key = key.strip()
        if not key:
            continue
        props[key] = value.strip()
    return props


def _prefix_fields(props: dict[str, str], prefix: str) -> dict[str, str]:
    return {key[len(prefix):]: value for key, value in props.items() if key.startswith(prefix)}


def extract_mappings(props: dict[str, str], source: str) -> list[dict[str, object]]:
    devices: dict[str, dict[str, str]] = {}
    bindings: list[tuple[str, str, str, str]] = []
    requests: dict[str, dict[str, str]] = {}
    dataids: dict[str, dict[str, str]] = {}

    for key, value in props.items():
        match = re.match(r"^DEV\.([^.]+)\.(.+)$", key)
        if match:
            device, tail = match.groups()
            devices.setdefault(device, {})[tail] = value
            bind = re.match(r"^([^.]+)\.([^.]+)\.requestid$", tail)
            if bind:
                data_id, link_id = bind.groups()
                bindings.append((device, data_id, link_id, value))
            continue

        match = re.match(r"^REQUESTID\.([^.]+)\.(.+)$", key)
        if match:
            request_id, field = match.groups()
            requests.setdefault(request_id, {})[field] = value
            continue

        match = re.match(r"^DATAID\.([^.]+)\.(.+)$", key)
        if match:
            data_id, field = match.groups()
            dataids.setdefault(data_id, {})[field] = value

    mappings: list[dict[str, object]] = []
    for device, data_id, link_id, request_id in bindings:
        request = requests.get(request_id, {})
        result_id = request.get("resultid", "")
        all_result = _prefix_fields(props, f"RESULTID.{result_id}.") if result_id else {}
        own_result = {
            field[len(data_id) + 1 :]: value
            for field, value in all_result.items()
            if field.startswith(data_id + ".")
        }
        device_fields = devices.get(device, {})
        channel = {
            field[len("channel.") :]: value
            for field, value in device_fields.items()
            if field.startswith("channel.")
        }
        record: dict[str, object] = {
            "source": source,
            "device": device,
            "provider_type": device_fields.get("type", ""),
            "channel": channel,
            "data_id": data_id,
            "data": dataids.get(data_id, {}),
            "link_id": link_id,
            "request_id": request_id,
            "request": request,
            "result_id": result_id,
            "result": own_result,
            "result_all": all_result,
            "evidence_state": "source-backed-candidate",
        }
        mappings.append(record)

    return mappings


def mapping_matches_targets(mapping: dict[str, object], targets: list[str]) -> bool:
    if not targets:
        return True
    data_id = str(mapping.get("data_id", "")).lower()
    return any(target.lower() in data_id for target in targets)


def collect_mappings(paths: list[Path], include_baseline: bool, targets: list[str]) -> list[dict[str, object]]:
    mappings: list[dict[str, object]] = []
    seen_sources: set[str] = set()
    for path in paths:
        for source, data, _decrypted in iter_property_sources(path):
            if source in seen_sources:
                continue
            seen_sources.add(source)
            if not include_baseline and is_baseline_source(source):
                continue
            props = parse_properties(data)
            if not props:
                continue
            for mapping in extract_mappings(props, source):
                if mapping_matches_targets(mapping, targets):
                    mappings.append(mapping)
    mappings.sort(key=lambda item: (str(item["data_id"]), str(item["device"]), str(item["source"])))
    return mappings


def format_mapping(mapping: dict[str, object]) -> list[str]:
    request = mapping.get("request", {})
    channel = mapping.get("channel", {})
    data = mapping.get("data", {})
    result = mapping.get("result", {})
    assert isinstance(request, dict)
    assert isinstance(channel, dict)
    assert isinstance(data, dict)
    assert isinstance(result, dict)

    route_parts = []
    if channel.get("transmit_address"):
        route_parts.append(f"tx={channel['transmit_address']}")
    if channel.get("receive_address"):
        route_parts.append(f"rx={channel['receive_address']}")
    if channel.get("baudrate"):
        route_parts.append(f"baud={channel['baudrate']}")

    request_parts = []
    if request.get("pdu"):
        request_parts.append(f"pdu={request['pdu']}")
    if request.get("matching_response"):
        request_parts.append(f"match={request['matching_response']}")
    if request.get("timeout"):
        request_parts.append(f"timeout={request['timeout']}")

    result_parts = []
    for key in ("responseparam", "param.0.extract", "param.0.encoding", "param.0.formula", "formula"):
        if result.get(key):
            result_parts.append(f"{key}={result[key]}")

    data_parts = []
    if data.get("datatype"):
        data_parts.append(f"datatype={data['datatype']}")
    if data.get("unit"):
        data_parts.append(f"unit={data['unit']}")

    header = (
        f"{mapping['data_id']}  device={mapping['device']} "
        f"link={mapping['link_id']} request={mapping['request_id']} "
        f"result={mapping['result_id']}"
    )
    lines = [header]
    if route_parts:
        lines.append("  route: " + " ".join(route_parts))
    if request_parts:
        lines.append("  request: " + " ".join(request_parts))
    if result_parts:
        lines.append("  result: " + " ".join(result_parts))
    if data_parts:
        lines.append("  data: " + " ".join(data_parts))
    lines.append(f"  source: {mapping['source']}")
    return lines


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Find retained Mercedes me Whisper/ACS configuration artifacts and "
            "optionally reconstruct source-backed DataID request mappings."
        )
    )
    parser.add_argument("roots", nargs="+", help="directory/file roots to scan")
    parser.add_argument("--min-score", type=int, default=18, help="minimum candidate score (default: 18)")
    parser.add_argument("--copy-out", metavar="DIR", help="copy candidates scoring >= 30 to DIR")
    parser.add_argument("--limit", type=int, default=50, help="maximum candidate results printed")
    parser.add_argument(
        "--mappings",
        action="store_true",
        help="parse Whisper properties and print exact device/request/result bindings",
    )
    parser.add_argument(
        "--target",
        action="append",
        default=[],
        help="only show mappings whose DataID contains this text (repeatable)",
    )
    parser.add_argument(
        "--include-baseline-mappings",
        action="store_true",
        help="also parse the APK-bundled VIN/bootstrap mappings",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="emit a machine-readable candidate/mapping report",
    )
    args = parser.parse_args()

    results = []
    seen = set()
    scanned_paths: list[Path] = []

    for root_string in args.roots:
        root = Path(root_string).expanduser()
        if not root.exists():
            if not args.json:
                print(f"WARN missing: {root}", file=sys.stderr)
            continue

        for path in iter_files(root):
            try:
                key = str(path.resolve())
            except OSError:
                key = str(path)
            if key in seen:
                continue
            seen.add(key)
            scanned_paths.append(path)
            score, reasons = inspect_file(path)
            if score >= args.min_score:
                results.append((score, path, reasons))

    results.sort(key=lambda item: (-item[0], str(item[1])))
    mappings = (
        collect_mappings(scanned_paths, args.include_baseline_mappings, args.target)
        if args.mappings or args.json
        else []
    )

    if args.json:
        report = {
            "candidate_count": len(results),
            "candidates": [
                {
                    "score": score,
                    "path": str(path),
                    "reasons": list(dict.fromkeys(reasons))[:20],
                }
                for score, path, reasons in results[: max(1, args.limit)]
            ],
            "mapping_count": len(mappings),
            "mappings": mappings,
        }
        print(json.dumps(report, indent=2, sort_keys=True))
        return 0

    print(f"Whisper recovery scan: {len(results)} candidate(s)")

    output_dir = Path(args.copy_out).expanduser() if args.copy_out else None
    copied = 0
    if output_dir:
        output_dir.mkdir(parents=True, exist_ok=True)

    for score, path, reasons in results[: max(1, args.limit)]:
        unique_reasons = list(dict.fromkeys(reasons))[:12]
        print(f"{score:3d}  {path}")
        print("     " + ", ".join(unique_reasons))

        if output_dir and score >= 30:
            try:
                digest = sha256(path)
                destination = output_dir / f"{digest[:12]}-{path.name}"
                if not destination.exists():
                    shutil.copy2(path, destination)
                copied += 1
            except OSError as exc:
                print(f"     copy failed: {exc}", file=sys.stderr)

    if output_dir:
        print(f"Copied {copied} high-value candidate(s) to {output_dir}")

    if not results:
        print("No retained dynamic Whisper/ACS configuration artifact found in the supplied roots.")

    if args.mappings:
        print(f"Whisper mapping extraction: {len(mappings)} binding(s)")
        for mapping in mappings:
            for line in format_mapping(mapping):
                print(line)
        if not mappings:
            print(
                "No post-VIN DataID -> device/request/result binding was found. "
                "APK bootstrap mappings are suppressed unless --include-baseline-mappings is used."
            )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
