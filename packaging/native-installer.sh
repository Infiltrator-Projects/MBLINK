#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

version="__MBLINK_VERSION__"
native_version="${version}+native1"
mode="install"
extract_directory=""
output_path=""
jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf '2')"

usage()
{
    cat <<EOF
MBLINK ${version} native Linux builder

Usage: MBLINK-${version}-linux-native.run [options]

Without options, this checks and installs the required Debian/Ubuntu build
packages when possible, extracts the bundled source, compiles MBLINK and its
portable core for this machine, runs the complete test suite, creates the
Debian-managed package mblink ${native_version}, and installs it through APT.

Options:
  --build-only        Build and verify the native Debian package without installing it
  --output FILE       Native .deb destination for --build-only
  --extract DIR       Extract the complete bundled source and stop
  --jobs N            Parallel build jobs (default: detected CPU count)
  -h, --help          Show this help

Required build packages on Debian/Ubuntu:
  build-essential cmake dpkg-dev pkg-config libgtk-4-dev libbluetooth-dev libusb-1.0-0-dev

A native install is package-managed. It replaces the generic mblink package
record with ${native_version}; the next newer repository release can upgrade it.
EOF
}

while (($#)); do
    case "$1" in
        --build-only) mode="build-only"; shift ;;
        --output)
            [[ $# -ge 2 ]] || { echo '--output requires a file path.' >&2; exit 2; }
            output_path="$2"; shift 2 ;;
        --extract)
            [[ $# -ge 2 ]] || { echo '--extract requires a directory.' >&2; exit 2; }
            mode="extract"; extract_directory="$2"; shift 2 ;;
        --jobs)
            [[ $# -ge 2 ]] || { echo '--jobs requires a positive integer.' >&2; exit 2; }
            jobs="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'Unknown option: %s\n\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done

[[ "$jobs" =~ ^[1-9][0-9]*$ ]] || { echo '--jobs must be a positive integer.' >&2; exit 2; }
if [[ -n "$output_path" && "$mode" != "build-only" ]]; then
    echo '--output is valid only with --build-only.' >&2
    exit 2
fi

payload_line="$(awk '/^__MBLINK_NATIVE_PAYLOAD_BELOW__$/ { print NR + 1; exit }' "$0")"
[[ -n "$payload_line" ]] || { echo 'The embedded MBLINK source payload is missing.' >&2; exit 1; }

extract_payload()
{
    local destination="$1"
    mkdir -p -- "$destination"
    tail -n +"$payload_line" "$0" | gzip -dc | tar -xf - -C "$destination"
    test -f "$destination/CMakeLists.txt"
    test -f "$destination/src/link/VERSION"
    test -f "$destination/src/link/src/infiltratr-common/VERSION"
}

if [[ "$mode" == "extract" ]]; then
    [[ -n "$extract_directory" ]] || { echo 'An extraction directory is required.' >&2; exit 2; }
    if [[ -e "$extract_directory" && -n "$(find "$extract_directory" -mindepth 1 -maxdepth 1 -print -quit 2>/dev/null)" ]]; then
        echo "Extraction directory is not empty: $extract_directory" >&2
        exit 1
    fi
    extract_payload "$extract_directory"
    printf 'MBLINK %s source extracted to %s\n' "$version" "$extract_directory"
    exit 0
fi

prerequisites_ready()
{
    local command_name
    for command_name in cmake cpack cc pkg-config glib-compile-resources dpkg dpkg-deb dpkg-query; do
        command -v "$command_name" >/dev/null 2>&1 || return 1
    done
    pkg-config --atleast-version=4.6 gtk4 >/dev/null 2>&1 || return 1
    pkg-config --exists bluez >/dev/null 2>&1 || return 1
    pkg-config --exists libusb-1.0 >/dev/null 2>&1 || return 1
    return 0
}

run_as_root()
{
    if [[ $EUID -eq 0 ]]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        echo 'Administrator access is required and sudo is unavailable.' >&2
        return 1
    fi
}

install_build_dependencies()
{
    local -a packages=(build-essential cmake dpkg-dev pkg-config libgtk-4-dev libbluetooth-dev libusb-1.0-0-dev)
    command -v apt-get >/dev/null 2>&1 || {
        echo 'Required build prerequisites are missing and apt-get is unavailable.' >&2
        return 1
    }
    echo 'Installing MBLINK native-build prerequisites...'
    run_as_root env DEBIAN_FRONTEND=noninteractive apt-get update
    run_as_root env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "${packages[@]}"
}

if ! prerequisites_ready; then
    echo 'One or more MBLINK native-build prerequisites are missing.'
    install_build_dependencies
fi
prerequisites_ready || {
    echo 'MBLINK prerequisites are still incomplete after the installation attempt.' >&2
    exit 1
}

work_directory="$(mktemp -d)"
cleanup() { rm -rf -- "$work_directory"; }
trap cleanup EXIT

source_directory="$work_directory/source"
build_directory="$work_directory/build"
package_directory="$work_directory/package"
extract_payload "$source_directory"

cmake \
    -S "$source_directory" \
    -B "$build_directory" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DMBLINK_BUILD_LINUX_APP=ON \
    -DMBLINK_BUILD_PROFILE=native \
    -DMBLINK_PACKAGE_VERSION="$native_version" \
    -DBUILD_TESTING=ON
cmake --build "$build_directory" --parallel "$jobs"
ctest --test-dir "$build_directory" --output-on-failure --parallel "$jobs"

mkdir -p "$package_directory"
cpack --config "$build_directory/CPackConfig.cmake" -G DEB -B "$package_directory"
package_path="$(find "$package_directory" -maxdepth 1 -type f -name '*.deb' -print -quit)"
[[ -n "$package_path" && -s "$package_path" ]] || { echo 'Native Debian package was not produced.' >&2; exit 1; }

[[ "$(dpkg-deb -f "$package_path" Package)" == "mblink" ]] || { echo 'Native package has the wrong package name.' >&2; exit 1; }
[[ "$(dpkg-deb -f "$package_path" Version)" == "$native_version" ]] || { echo 'Native package has the wrong version.' >&2; exit 1; }
[[ "$(dpkg-deb -f "$package_path" Architecture)" == "$(dpkg --print-architecture)" ]] || { echo 'Native package has the wrong architecture.' >&2; exit 1; }

if [[ "$mode" == "build-only" ]]; then
    if [[ -z "$output_path" ]]; then
        output_path="$PWD/mblink_${native_version}_$(dpkg --print-architecture).deb"
    fi
    mkdir -p "$(dirname "$output_path")"
    install -m 0644 "$package_path" "$output_path"
    printf 'Native MBLINK Debian package created: %s\n' "$output_path"
    exit 0
fi

(
    cd "$(dirname "$package_path")"
    run_as_root apt-get install -y "./$(basename "$package_path")"
)

# Remove files left by pre-0.7.61 unmanaged /usr/local native installers.
run_as_root rm -f \
    /usr/local/bin/mblink-linux \
    /usr/local/share/icons/hicolor/180x180/apps/mblink.png \
    /usr/local/share/pixmaps/mblink.png \
    /usr/local/share/applications/com.github.The-First-Infiltrator.MBLINK.desktop
run_as_root rm -rf /usr/local/share/doc/mblink

installed_version="$(dpkg-query -W -f='${Version}' mblink 2>/dev/null || true)"
[[ "$installed_version" == "$native_version" ]] || {
    printf 'Native package installation verification failed: expected %s, found %s\n' "$native_version" "$installed_version" >&2
    exit 1
}

printf 'MBLINK %s was compiled locally, tested, packaged and installed as mblink %s.\n' "$version" "$native_version"
printf 'APT now owns the native installation and will offer only a genuinely newer release.\n'
exit 0
__MBLINK_NATIVE_PAYLOAD_BELOW__
