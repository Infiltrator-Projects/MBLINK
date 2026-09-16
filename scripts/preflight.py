#!/usr/bin/env python3
"""Fast repository checks that should fail before expensive platform CI starts.

This intentionally checks stable repository/release invariants, not source layout.
Functional behaviour belongs in compiled/unit/simulator tests.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SEMVER = re.compile(r"^\d+\.\d+\.\d+$")


def fail(message: str) -> None:
    raise SystemExit(f"preflight: {message}")


def run(*args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    proc = subprocess.run(
        args,
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if check and proc.returncode != 0:
        detail = (proc.stderr or proc.stdout).strip()
        fail(f"{' '.join(args)} failed" + (f": {detail}" if detail else ""))
    return proc


def read_version(path: Path, label: str) -> str:
    try:
        value = path.read_text(encoding="utf-8").strip()
    except OSError as exc:
        fail(f"cannot read {label}: {exc}")
    if not SEMVER.fullmatch(value):
        fail(f"{label} is not semantic major.minor.patch: {value!r}")
    return value


def check_versions() -> str:
    version = read_version(ROOT / "VERSION", "VERSION")
    read_version(ROOT / "src/link/VERSION", "LINK VERSION")
    read_version(
        ROOT / "src/link/src/infiltratr-common/VERSION",
        "Infiltratr Common VERSION",
    )

    header = (ROOT / "include/mblink/version.h").read_text(encoding="utf-8")
    match = re.search(r'MBLINK_VERSION_STRING\s+"([^"]+)"', header)
    if not match or match.group(1) != version:
        fail("include/mblink/version.h does not match VERSION")

    project = (ROOT / "app/ios/MBLINK.xcodeproj/project.pbxproj").read_text(
        encoding="utf-8"
    )
    marketing = sorted(set(re.findall(r"MARKETING_VERSION = ([^;]+);", project)))
    if marketing != [version]:
        fail(f"iOS MARKETING_VERSION values do not match VERSION: {marketing}")
    return version


def check_gitlinks() -> None:
    proc = run("git", "ls-files", "--stage")
    gitlinks = sorted(
        line.split(maxsplit=3)[3]
        for line in proc.stdout.splitlines()
        if line.startswith("160000 ")
    )
    if gitlinks != ["src/link"]:
        fail(f"expected exactly one top-level gitlink at src/link; found {gitlinks}")

    status = run("git", "submodule", "status", "--recursive").stdout.splitlines()
    bad = [line for line in status if line[:1] in {"+", "-", "U"}]
    if bad:
        fail("recursive submodule checkout does not match committed gitlinks:\n" + "\n".join(bad))

    expected = run("git", "ls-tree", "HEAD", "src/link").stdout.split()
    if len(expected) < 3:
        fail("unable to resolve committed LINK gitlink")
    expected_sha = expected[2]
    actual_sha = run("git", "-C", "src/link", "rev-parse", "HEAD").stdout.strip()
    if actual_sha != expected_sha:
        fail(f"LINK checkout mismatch: expected {expected_sha}, found {actual_sha}")
    project = (ROOT / "app/ios/MBLINK.xcodeproj/project.pbxproj").read_text()
    revisions = [re.search(r"[0-9a-f]{40}", line) for line in project.splitlines()
                 if "LINK_SOURCE_REVISION=" in line]
    if len(revisions) != 2 or any(match is None or match.group() != expected_sha
                                  for match in revisions):
        fail("every iOS configuration must record the exact LINK gitlink")


def check_release_subject(version: str) -> bool:
    subject = os.environ.get("MBLINK_COMMIT_SUBJECT")
    if subject is None:
        subject = run("git", "log", "-1", "--format=%s", "HEAD").stdout.strip()
    is_release = subject.startswith("Release ")
    if is_release and not (
        subject == f"Release {version}" or subject.startswith(f"Release {version}:")
    ):
        fail(
            f"release commit subject must begin with 'Release {version}', got {subject!r}"
        )
    return is_release


def check_whitespace() -> None:
    parent = run("git", "rev-parse", "HEAD^", check=False)
    if parent.returncode == 0:
        proc = run("git", "diff", "--check", "HEAD^", "HEAD", check=False)
        if proc.returncode != 0:
            fail("git diff --check failed:\n" + (proc.stdout + proc.stderr).strip())


def check_stable_architecture_contract() -> None:
    contract = (ROOT / "docs/PID_ARCHITECTURE.md").read_text(encoding="utf-8")
    flattened = " ".join(contract.split())
    required = [
        "Standard OBD choices use one VIN-scoped selection",
        "Mercedes choices remain VIN-and-module scoped",
        "Unknown or unresolved modules may still appear in the module list.",
        "They must not be assigned invented semantics.",
    ]
    missing = [text for text in required if text not in flattened]
    if missing:
        fail("PID architecture contract is missing: " + "; ".join(missing))

    controller_header = (ROOT / "platform/apple/MBLinkDiagnosticsController.h").read_text(
        encoding="utf-8"
    )
    if "@interface MBLinkDiagnosticsController : LinkProductDiagnosticsController" not in controller_header:
        fail("Apple product controller must continue to extend LINK's product controller")

    core = (ROOT / "src/core/mblink.c").read_text(encoding="utf-8")
    if '#include "../link/src/' in core:
        fail("MBLINK core must not directly include LINK implementation sources")


def main() -> int:
    version = check_versions()
    check_gitlinks()
    check_whitespace()
    check_stable_architecture_contract()
    is_release = check_release_subject(version)
    print(f"MBLINK preflight OK: version={version} release={'true' if is_release else 'false'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
