#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Read-only forensic scanner for Mercedes me Whisper configuration artifacts."""

from __future__ import annotations

import argparse
import binascii
import hashlib
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

    baseline_name = path.name[:-6] if path.name.endswith(".plain") else path.name
    baseline_name = re.sub(r"\(\d+\)(?=\.[^.]+$)", "", baseline_name)
    if baseline_name in BASELINE_NAMES:
        score -= 100
        reasons.append("bundled-baseline-name")

    return score, reasons


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

            for name in names[:2000]:
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

            # Only inspect small, unencrypted text/config members.
            for info in archive.infolist()[:500]:
                if info.flag_bits & 1 or info.file_size > MAX_ZIP_MEMBER:
                    continue
                if not info.filename.lower().endswith(
                    (".properties", ".cfg", ".txt", ".json")
                ):
                    continue
                try:
                    data = archive.read(info)[:MAX_READ]
                except (OSError, RuntimeError, zipfile.BadZipFile):
                    continue
                plaintext = decrypt_whisper_hex(data)
                part_score, part_reasons = score_blob(
                    plaintext if plaintext is not None else data
                )
                score += part_score
                reasons += [
                    f"{info.filename}:{item}" for item in part_reasons
                ]
    except (OSError, zipfile.BadZipFile, zipfile.LargeZipFile):
        return 0, []

    return score, reasons


def inspect_file(path: Path) -> tuple[int, list[str]]:
    path_score, path_reasons = score_path(path)
    data = read_prefix(path)

    if data.startswith(b"PK\x03\x04") or path.suffix.lower() == ".zip":
        zip_score, zip_reasons = inspect_zip(path)
        return path_score + zip_score, path_reasons + zip_reasons

    # Do not report native binaries merely because they embed schema strings.
    if path.suffix.lower() not in {
        ".properties",
        ".cfg",
        ".json",
        ".xml",
        ".plain",
    }:
        return path_score, path_reasons

    plaintext = decrypt_whisper_hex(data)
    if plaintext is not None:
        blob_score, blob_reasons = score_blob(plaintext)
        return (
            path_score + blob_score + 4,
            path_reasons + ["Whisper-AES-properties"] + blob_reasons,
        )

    blob_score, blob_reasons = score_blob(data)
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


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Find retained Mercedes me Whisper/ACS configuration artifacts "
            "without modifying the source tree."
        )
    )
    parser.add_argument("roots", nargs="+", help="directory/file roots to scan")
    parser.add_argument(
        "--min-score",
        type=int,
        default=18,
        help="minimum candidate score (default: 18)",
    )
    parser.add_argument(
        "--copy-out",
        metavar="DIR",
        help="copy candidates scoring >= 30 to DIR",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=50,
        help="maximum results printed",
    )
    args = parser.parse_args()

    results = []
    seen = set()

    for root_string in args.roots:
        root = Path(root_string).expanduser()
        if not root.exists():
            print(f"WARN missing: {root}", file=sys.stderr)
            continue

        for path in iter_files(root):
            key = str(path.resolve())
            if key in seen:
                continue
            seen.add(key)
            score, reasons = inspect_file(path)
            if score >= args.min_score:
                results.append((score, path, reasons))

    results.sort(key=lambda item: (-item[0], str(item[1])))
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
        print(
            "No retained dynamic Whisper/ACS configuration artifact "
            "found in the supplied roots."
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
