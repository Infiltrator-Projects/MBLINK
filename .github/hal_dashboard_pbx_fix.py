from pathlib import Path

pbx = Path("app/ios/MBLINK.xcodeproj/project.pbxproj")
text = pbx.read_text()

build_anchor = '\t\tA22100000000000000000001 /* link_workspace.c in Sources */ = {isa = PBXBuildFile; fileRef = A22000000000000000000001 /* link_workspace.c */; };\n'
build_line = '\t\tA22100000000000000000018 /* link_dashboard.c in Sources */ = {isa = PBXBuildFile; fileRef = A22000000000000000000018 /* link_dashboard.c */; };\n'
file_anchor = '\t\tA22000000000000000000001 /* link_workspace.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = "link_workspace.c"; path = "../../src/link/src/core/workspace.c"; sourceTree = SOURCE_ROOT; };\n'
file_line = '\t\tA22000000000000000000018 /* link_dashboard.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = "link_dashboard.c"; path = "../../src/link/src/core/dashboard.c"; sourceTree = SOURCE_ROOT; };\n'
group_anchor = '\t\t\t\tA22000000000000000000001 /* link_workspace.c */,\n'
group_line = '\t\t\t\tA22000000000000000000018 /* link_dashboard.c */,\n'
phase_anchor = '\t\t\t\tA22100000000000000000001 /* link_workspace.c in Sources */,\n'
phase_line = '\t\t\t\tA22100000000000000000018 /* link_dashboard.c in Sources */,\n'

if "../../src/link/src/core/dashboard.c" not in text:
    for anchor, addition, label in (
        (build_anchor, build_line, "PBXBuildFile"),
        (file_anchor, file_line, "PBXFileReference"),
        (group_anchor, group_line, "Portable Core group"),
        (phase_anchor, phase_line, "Sources build phase"),
    ):
        if text.count(anchor) != 1:
            raise SystemExit(f"expected exactly one {label} anchor")
        text = text.replace(anchor, anchor + addition, 1)

for required in (
    "link_dashboard.c in Sources",
    "../../src/link/src/core/dashboard.c",
):
    if required not in text:
        raise SystemExit(f"failed to install Apple dashboard source wiring: {required}")

pbx.write_text(text)
Path(__file__).unlink()
