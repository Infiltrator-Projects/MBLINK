#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="$(tr -d '[:space:]' < "$project_root/VERSION")"
template="$project_root/src/link/packaging/native-product-installer.sh.in"
output="${1:-$project_root/MBLINK-${version}-linux-native.run}"

[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || {
    echo "Invalid VERSION: $version" >&2
    exit 1
}
test -f "$template"
grep -qx '__LINK_NATIVE_PAYLOAD_BELOW__' "$template"
test -f "$project_root/src/link/VERSION"
test -f "$project_root/src/link/src/infiltratr-common/VERSION"

temporary="$(mktemp -d)"
cleanup() { rm -rf -- "$temporary"; }
trap cleanup EXIT
source_epoch="$(git -C "$project_root" log -1 --format=%ct 2>/dev/null || printf '0')"
payload="$temporary/source.tar.gz"
generated_header="$temporary/native-installer.sh"

python3 - "$template" "$generated_header" "$version" <<'PY'
from pathlib import Path
import sys
source = Path(sys.argv[1]).read_text(encoding="utf-8")
values = {
    "__LINK_NATIVE_PRODUCT_NAME__": "MBLINK",
    "__LINK_NATIVE_PRODUCT_SLUG__": "mblink",
    "__LINK_NATIVE_PACKAGE_NAME__": "mblink",
    "__LINK_NATIVE_VERSION__": sys.argv[3],
    "__LINK_NATIVE_CMAKE_ENABLE_OPTION__": "MBLINK_BUILD_LINUX_APP",
    "__LINK_NATIVE_CMAKE_PROFILE_OPTION__": "MBLINK_BUILD_PROFILE",
    "__LINK_NATIVE_CMAKE_PACKAGE_VERSION_OPTION__": "MBLINK_PACKAGE_VERSION",
    "__LINK_NATIVE_LEGACY_CLEANUP_PATHS__": ":".join([
        "/usr/local/bin/mblink-linux",
        "/usr/local/share/icons/hicolor/180x180/apps/mblink.png",
        "/usr/local/share/pixmaps/mblink.png",
        "/usr/local/share/applications/com.github.The-First-Infiltrator.MBLINK.desktop",
        "/usr/local/share/doc/mblink",
    ]),
}
for key, value in values.items():
    if key not in source:
        raise SystemExit(f"LINK native installer template missing {key}")
    source = source.replace(key, value)
if "__LINK_NATIVE_" in source.replace("__LINK_NATIVE_PAYLOAD_BELOW__", ""):
    raise SystemExit("Unresolved LINK native installer placeholder")
Path(sys.argv[2]).write_text(source, encoding="utf-8")
PY

tar \
    --directory "$project_root" \
    --sort=name \
    --mtime="@$source_epoch" \
    --owner=0 \
    --group=0 \
    --numeric-owner \
    --pax-option=delete=atime,delete=ctime \
    --exclude-vcs \
    --exclude='./build' \
    --exclude='./build-*' \
    --exclude='./MBLINK-*.deb' \
    --exclude='./MBLINK-*.ipa' \
    --exclude='./MBLINK-*.run' \
    -cf - . | gzip -n -9 > "$payload"

mkdir -p -- "$(dirname "$output")"
cp "$generated_header" "$output"
cat "$payload" >> "$output"
chmod 0755 "$output"
test -x "$output"
"$output" --help >/dev/null
printf 'Created %s\n' "$output"
