#!/usr/bin/env python3
"""Fast repository checks that should fail before expensive platform CI starts.

This intentionally checks stable repository/release invariants, not source layout.
Functional behaviour belongs in compiled/unit/simulator tests.
"""
from __future__ import annotations

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
    read_version(ROOT / "src/link/VERSION", "Link VERSION")
    read_version(
        ROOT / "src/link/src/infiltratr-common/VERSION",
        "Libraries VERSION",
    )

    header = (ROOT / "include/mblink/version.h").read_text(encoding="utf-8")
    match = re.search(r'MBLINK_VERSION_STRING\s+"([^"]+)"', header)
    if not match or match.group(1) != version:
        fail("include/mblink/version.h does not match VERSION")

    project = (ROOT / "app/ios/MBLink.xcodeproj/project.pbxproj").read_text(
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
        fail("unable to resolve committed Link gitlink")
    expected_sha = expected[2]
    actual_sha = run("git", "-C", "src/link", "rev-parse", "HEAD").stdout.strip()
    if actual_sha != expected_sha:
        fail(f"Link checkout mismatch: expected {expected_sha}, found {actual_sha}")
    project = (ROOT / "app/ios/MBLink.xcodeproj/project.pbxproj").read_text()
    revisions = [
        re.search(r"[0-9a-f]{40}", line)
        for line in project.splitlines()
        if "LINK_SOURCE_REVISION=" in line
    ]
    if len(revisions) != 2 or any(
        match is None or match.group() != expected_sha for match in revisions
    ):
        fail("every iOS configuration must record the exact Link gitlink")


def check_release_structure() -> None:
    """Keep publication explicit and separate from ordinary validation.

    These are deliberately coarse, stable invariants. Detailed behaviour is
    exercised by CI/release jobs themselves; preflight only prevents a future
    edit from quietly re-coupling validation, tagging and publication.
    """
    workflows = ROOT / ".github/workflows"
    required = {
        "ci.yml": "ordinary CI",
        "release.yml": "qualified artifact publication",
        "apt-publication.yml": "downstream APT publication",
    }
    for filename, purpose in required.items():
        if not (workflows / filename).is_file():
            fail(f"missing {purpose} workflow: .github/workflows/{filename}")

    deprecated = [
        "auto-release.yml",
        "mblink-refactor-release-once.yml",
    ]
    present = [name for name in deprecated if (workflows / name).exists()]
    if present:
        fail("deprecated self-modifying release workflow present: " + ", ".join(present))

    ci = (workflows / "ci.yml").read_text(encoding="utf-8")
    forbidden_ci_tokens = [
        "head_commit.message",
        "is_release",
        "contents: write",
    ]
    leaked = [token for token in forbidden_ci_tokens if token in ci]
    if leaked:
        fail("ordinary CI must not infer or publish releases: " + ", ".join(leaked))

    release = (workflows / "release.yml").read_text(encoding="utf-8")
    if "workflow_run:" not in release or 'workflows: ["MBLink CI"]' not in release:
        fail("publication must follow the complete successful MBLink CI run")
    if "actions/download-artifact@" not in release or "github.event.workflow_run.id" not in release:
        fail("publication must consume artifacts from the exact qualified CI run")
    if any(token in release for token in ("cmake --build", "xcodebuild ", "cpack --config")):
        fail("publication must not rebuild qualified artifacts")

    apt = (workflows / "apt-publication.yml").read_text(encoding="utf-8")
    if "types: [published]" not in apt:
        fail("APT publication must remain downstream of a published GitHub release")


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
        "Standard OBD choices are VIN-and-controller scoped",
        "Mercedes choices remain VIN-and-controller scoped",
        "Unknown or unresolved modules may still appear in the module list.",
        "They must not be assigned invented semantics.",
    ]
    missing = [text for text in required if text not in flattened]
    if missing:
        fail("PID architecture contract is missing: " + "; ".join(missing))

    controller_header = (ROOT / "platform/apple/MBLinkDiagnosticsController.h").read_text(
        encoding="utf-8"
    )
    if (
        "@interface MBLinkDiagnosticsController : LinkProductDiagnosticsController"
        not in controller_header
    ):
        fail("Apple product controller must continue to extend Link's product controller")

    core = (ROOT / "src/core/mblink.c").read_text(encoding="utf-8")
    if '#include "../link/src/' in core:
        fail("MBLink core must not directly include Link implementation sources")


def main() -> int:
    version = check_versions()
    check_gitlinks()
    check_release_structure()
    check_whitespace()
    check_stable_architecture_contract()
    print(f"MBLink preflight OK: version={version}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
