#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="$(tr -d '[:space:]' < "$project_root/VERSION")"
header="$project_root/packaging/native-installer.sh"
output="${1:-$project_root/MBLINK-${version}-linux-native.run}"

[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || {
    echo "Invalid VERSION: $version" >&2
    exit 1
}
grep -qx '__MBLINK_NATIVE_PAYLOAD_BELOW__' "$header"
grep -q '^version="__MBLINK_VERSION__"$' "$header"
test -f "$project_root/src/infiltratr-common/VERSION"

temporary="$(mktemp -d)"
cleanup()
{
    rm -rf -- "$temporary"
}
trap cleanup EXIT

source_epoch="$(git -C "$project_root" log -1 --format=%ct 2>/dev/null || printf '0')"
payload="$temporary/source.tar.gz"
generated_header="$temporary/native-installer.sh"

sed "s/^version=\"__MBLINK_VERSION__\"$/version=\"$version\"/" \
    "$header" > "$generated_header"
grep -q "^version=\"${version}\"$" "$generated_header"

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
