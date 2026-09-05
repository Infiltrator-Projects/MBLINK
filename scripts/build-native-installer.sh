#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="$(tr -d '[:space:]' < "$project_root/VERSION")"
output="${1:-$project_root/MBLINK-${version}-linux-native.run}"

test -f "$project_root/src/link/VERSION"
test -f "$project_root/src/link/src/infiltratr-common/VERSION"
builder="$project_root/src/link/packaging/build-native-product-installer.sh"
test -x "$builder"

export LINK_NATIVE_PRODUCT_NAME='MBLINK'
export LINK_NATIVE_PRODUCT_SLUG='mblink'
export LINK_NATIVE_PACKAGE_NAME='mblink'
export LINK_NATIVE_CMAKE_ENABLE_OPTION='MBLINK_BUILD_LINUX_APP'
export LINK_NATIVE_CMAKE_PROFILE_OPTION='MBLINK_BUILD_PROFILE'
export LINK_NATIVE_CMAKE_PACKAGE_VERSION_OPTION='MBLINK_PACKAGE_VERSION'
export LINK_NATIVE_LEGACY_CLEANUP_PATHS='/usr/local/bin/mblink-linux:/usr/local/share/icons/hicolor/180x180/apps/mblink.png:/usr/local/share/pixmaps/mblink.png:/usr/local/share/applications/com.github.The-First-Infiltrator.MBLINK.desktop:/usr/local/share/doc/mblink'

exec "$builder" "$project_root" "$output"
