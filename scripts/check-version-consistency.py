#!/usr/bin/env python3
"""Verify that MBLINK's release metadata has one consistent semantic version."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
version = (root / "VERSION").read_text(encoding="utf-8").strip()
if not re.fullmatch(r"\d+\.\d+\.\d+", version):
    raise SystemExit(f"invalid VERSION: {version!r}")

header = (root / "include/mblink/version.h").read_text(encoding="utf-8")
header_match = re.search(r'MBLINK_VERSION_STRING\s+"([^"]+)"', header)
if not header_match or header_match.group(1) != version:
    raise SystemExit("include/mblink/version.h does not match VERSION")

project = (root / "app/ios/MBLINK.xcodeproj/project.pbxproj").read_text(encoding="utf-8")
marketing = sorted(set(re.findall(r"MARKETING_VERSION = ([^;]+);", project)))
if marketing != [version]:
    raise SystemExit(f"iOS MARKETING_VERSION values do not match VERSION: {marketing}")

print(f"MBLINK release metadata is consistent: {version}")
