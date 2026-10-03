#!/usr/bin/env python3
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
LINK_COMMIT = "c7009e913e8bd074542b54737cc49cd937a54696"
OLD_VERSION = "0.7.286"
NEW_VERSION = "0.7.287"


def replace_exact(path: Path, old: str, new: str, expected: int = 1) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != expected:
        raise SystemExit(f"expected {expected} match(es) in {path}: {old!r}, found {count}")
    path.write_text(text.replace(old, new), encoding="utf-8")


def replace_at_least_one(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count < 1:
        raise SystemExit(f"expected at least one match in {path}: {old!r}")
    path.write_text(text.replace(old, new), encoding="utf-8")


# Advance LINK as the single dependency edge; LINK owns the Common pin.
subprocess.run(["git", "-C", str(ROOT / "src/link"), "fetch", "origin", LINK_COMMIT], check=True)
subprocess.run(["git", "-C", str(ROOT / "src/link"), "checkout", "--detach", LINK_COMMIT], check=True)
subprocess.run(["git", "-C", str(ROOT / "src/link"), "submodule", "sync", "--recursive"], check=True)
subprocess.run(["git", "-C", str(ROOT / "src/link"), "submodule", "update", "--init", "--recursive"], check=True)
if (ROOT / "src/link/VERSION").read_text(encoding="utf-8").strip() != "0.15.91":
    raise SystemExit("LINK pin did not resolve to 0.15.91")
if (ROOT / "src/link/src/infiltratr-common/VERSION").read_text(encoding="utf-8").strip() != "1.19.38":
    raise SystemExit("Common pin did not resolve to 1.19.38")

# Release identity.
replace_exact(ROOT / "VERSION", OLD_VERSION + "\n", NEW_VERSION + "\n")
replace_exact(
    ROOT / "include/mblink/version.h",
    f'#define MBLINK_VERSION_STRING "{OLD_VERSION}"',
    f'#define MBLINK_VERSION_STRING "{NEW_VERSION}"')
replace_at_least_one(
    ROOT / "app/ios/MBLINK.xcodeproj/project.pbxproj",
    f"MARKETING_VERSION = {OLD_VERSION};",
    f"MARKETING_VERSION = {NEW_VERSION};")

# Coalesce high-frequency controller callbacks before rebuilding the complete
# presentation model. This keeps expensive profile/module/PID work off the
# per-sample callback rate while retaining a responsive 10 Hz presentation cap.
vm = ROOT / "app/ios/MBLINK/ConnectionViewModel.swift"
replace_exact(
    vm,
    "    private var livePollingReadyRearmSignature: String?\n",
    "    private var livePollingReadyRearmSignature: String?\n"
    "    private var controllerRefreshScheduled = false\n")
replace_exact(
    vm,
    "    nonisolated func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {\n"
    "        Task { @MainActor [weak self] in self?.refreshStandardState() }\n"
    "    }",
    "    private func scheduleControllerRefresh() {\n"
    "        guard !controllerRefreshScheduled else { return }\n"
    "        controllerRefreshScheduled = true\n"
    "        Task { @MainActor [weak self] in\n"
    "            try? await Task.sleep(nanoseconds: 100_000_000)\n"
    "            guard let self else { return }\n"
    "            self.controllerRefreshScheduled = false\n"
    "            self.refreshStandardState()\n"
    "        }\n"
    "    }\n\n"
    "    nonisolated func diagnosticsControllerDidUpdate(_ controller: MBLinkDiagnosticsController) {\n"
    "        Task { @MainActor [weak self] in self?.scheduleControllerRefresh() }\n"
    "    }")

# Yield shared LINK geometry/typography to LINK/Common and derive MBLINK's local
# cockpit surfaces from the canonical Infiltrator palette instead of frozen
# previous-iteration Night literals.
style = ROOT / "app/linux/style.c"
replace_exact(
    style,
    "    g_string_append(\n"
    "        css,\n"
    "        \"entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-size: 13px; }\");\n\n"
    "    /*\n"
    "     * These cockpit/trace shades are deliberately MBLINK-specific composition,\n"
    "     * not duplicate Common semantic roles. Preserve them exactly.\n"
    "     */\n"
    "    g_string_append(\n"
    "        css,\n"
    "        \".mblink-cockpit-card { background: linear-gradient(155deg,#20262c,#11161b 58%,#080a0d); border-color: #4a525b; }\");",
    "    /* Product composition stays local; semantic colours come from Common. */\n"
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".mblink-cockpit-card { background: linear-gradient(155deg,#%06x,#%06x 58%%,#%06x); border-color: #%06x; }\",\n"
    "        css_rgb(palette->card_rgb), css_rgb(palette->surface_rgb),\n"
    "        css_rgb(palette->background_rgb), css_rgb(palette->border_rgb));")
replace_exact(
    style,
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".mblink-cockpit-gauge { background: linear-gradient(155deg,#171c21,#0b0e11); border: 1px solid #394149; border-radius: %upx; padding: %upx %upx %upx %upx; }\",\n"
    "        (unsigned int)metrics->card_radius,",
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".mblink-cockpit-gauge { background: linear-gradient(155deg,#%06x,#%06x); border: 1px solid #%06x; border-radius: %upx; padding: %upx %upx %upx %upx; }\",\n"
    "        css_rgb(palette->card_rgb), css_rgb(palette->surface_rgb),\n"
    "        css_rgb(palette->border_rgb),\n"
    "        (unsigned int)metrics->card_radius,")
replace_exact(
    style,
    "    g_string_append(css, \".mblink-gauge-pid { color: #7f8991; font-size: 10px; }\");",
    "    g_string_append_printf(\n"
    "        css, \".mblink-gauge-pid { color: #%06x; font-size: 10px; }\",\n"
    "        css_rgb(palette->subtle_rgb));")
replace_exact(
    style,
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".mblink-gauge-title { color: #dce2e6; font-size: 13px; font-weight: %u; }\",\n"
    "        (unsigned int)type->ui_bold_weight);",
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".mblink-gauge-title { color: #%06x; font-size: 13px; font-weight: %u; }\",\n"
    "        css_rgb(palette->title_rgb), (unsigned int)type->ui_bold_weight);")
replace_exact(
    style,
    "    g_string_append(\n"
    "        css,\n"
    "        \".mblink-trace-card { background: linear-gradient(155deg,#171c21,#0b0e11); border-color: #3e464f; min-width: 300px; }\");",
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".mblink-trace-card { background: linear-gradient(155deg,#%06x,#%06x); border-color: #%06x; min-width: 300px; }\",\n"
    "        css_rgb(palette->card_rgb), css_rgb(palette->surface_rgb),\n"
    "        css_rgb(palette->border_rgb));")
replace_exact(
    style,
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".link-about-dialog label, .link-about-dialog textview, .link-about-dialog textview text { font-family: \\\"%s\\\"; font-size: 14px; }\",\n"
    "        type->ui_family);",
    "    g_string_append_printf(\n"
    "        css,\n"
    "        \".link-about-dialog label, .link-about-dialog textview, .link-about-dialog textview text { font-family: \\\"%s\\\"; }\",\n"
    "        type->ui_family);")

# Make the dependency document a release fact instead of a stale 'current' pin.
doc = ROOT / "docs/COMMON-REUSE.md"
replace_exact(
    doc,
    "MBLINK currently reaches Common 1.19.20 through LINK 0.15.33.",
    "Release 0.7.287 reaches Common 1.19.38 through LINK 0.15.91.")
replace_exact(
    doc,
    "Common 1.19.20 also owns the complete product-neutral Linux MBLINK Night design roles.",
    "Common 1.19.38 also owns the product-neutral Infiltrator design roles consumed by Linux MBLINK.")
replace_exact(
    doc,
    "Only MBLINK-specific cockpit/trace gradients and Mercedes presentation details remain local.",
    "Only MBLINK-specific cockpit/trace composition and Mercedes presentation details remain local; its semantic colours resolve through Common rather than frozen local Night literals.")

changelog = ROOT / "CHANGELOG.md"
text = changelog.read_text(encoding="utf-8")
needle = "# Changelog\n\n"
entry = (
    "## 0.7.287 — 2026-10-03\n\n"
    "- Coalesce high-frequency iPhone diagnostics callbacks so expensive vehicle/profile/module/PID presentation rebuilds run at no more than 10 Hz instead of once per controller update.\n"
    "- Advance the single canonical dependency edge to LINK 0.15.91 and Infiltratr Common 1.19.38; keep Common owned by LINK rather than adding a competing MBLINK pin.\n"
    "- Bring the Linux face back onto the InfiltratorOS design contract by deriving cockpit/trace semantic colours from Common and dropping local font-size overrides on shared LINK widgets.\n"
    "- Retain intentional compatibility migrations, C207 replay verification and recovered Mercedes-adapter evidence while removing stale dependency documentation rather than deleting active compatibility paths as presumed legacy code.\n\n"
)
if not text.startswith(needle):
    raise SystemExit("unexpected CHANGELOG header")
changelog.write_text(needle + entry + text[len(needle):], encoding="utf-8")

# One-shot scaffolding must not enter the product commit.
(ROOT / "scripts/hal-maintenance-patch.py").unlink()
(ROOT / ".github/workflows/hal-maintenance.yml").unlink()
