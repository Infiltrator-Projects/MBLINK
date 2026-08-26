#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

version="__MBLINK_VERSION__"
mode="install"
prefix="/usr/local"
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
portable core on this machine, runs the complete test suite, and installs the
GTK4 application under /usr/local.

Options:
  --build-only        Compile and test without installing
  --output FILE       Native executable destination for --build-only
  --extract DIR       Extract the complete bundled source and stop
  --prefix DIR        Installation prefix (default: /usr/local)
  --jobs N            Parallel build jobs (default: detected CPU count)
  -h, --help          Show this help

Required build packages on Debian/Ubuntu:
  build-essential cmake pkg-config libgtk-4-dev libbluetooth-dev

If any of these prerequisites are missing, the installer uses apt-get (and
sudo when required) to install them automatically before continuing.
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
        --prefix)
            [[ $# -ge 2 ]] || { echo '--prefix requires a directory.' >&2; exit 2; }
            prefix="$2"; shift 2 ;;
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
    for command_name in cmake cc pkg-config glib-compile-resources; do
        command -v "$command_name" >/dev/null 2>&1 || return 1
    done
    pkg-config --atleast-version=4.6 gtk4 >/dev/null 2>&1 || return 1
    pkg-config --exists bluez >/dev/null 2>&1 || return 1
    return 0
}

install_build_dependencies()
{
    local -a elevate=()
    local -a packages=(build-essential cmake pkg-config libgtk-4-dev libbluetooth-dev)

    if ! command -v apt-get >/dev/null 2>&1; then
        echo 'Required build prerequisites are missing and apt-get is unavailable.' >&2
        echo 'Install: build-essential cmake pkg-config libgtk-4-dev libbluetooth-dev' >&2
        return 1
    fi

    if [[ $EUID -ne 0 ]]; then
        if ! command -v sudo >/dev/null 2>&1; then
            echo 'Build prerequisites are missing and installing them requires root access.' >&2
            echo 'Install: build-essential cmake pkg-config libgtk-4-dev libbluetooth-dev' >&2
            return 1
        fi
        echo 'MBLINK needs to install missing build prerequisites; sudo may ask for your password.'
        sudo -v
        elevate=(sudo)
    fi

    echo 'Installing MBLINK native-build prerequisites...'
    "${elevate[@]}" env DEBIAN_FRONTEND=noninteractive apt-get update
    "${elevate[@]}" env DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "${packages[@]}"
}

if ! prerequisites_ready; then
    echo 'One or more MBLINK native-build prerequisites are missing.'
    install_build_dependencies
fi

if ! prerequisites_ready; then
    echo 'MBLINK prerequisites are still incomplete after the installation attempt.' >&2
    echo 'Required: build-essential cmake pkg-config libgtk-4-dev libbluetooth-dev with GTK 4.6 or newer.' >&2
    exit 1
fi

work_directory="$(mktemp -d)"
cleanup()
{
    rm -rf -- "$work_directory"
}
trap cleanup EXIT

source_directory="$work_directory/source"
build_directory="$work_directory/build"
extract_payload "$source_directory"

cmake \
    -S "$source_directory" \
    -B "$build_directory" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$prefix" \
    -DMBLINK_BUILD_LINUX_APP=ON \
    -DBUILD_TESTING=ON
cmake --build "$build_directory" --parallel "$jobs"
ctest --test-dir "$build_directory" --output-on-failure --parallel "$jobs"

if [[ "$mode" == "build-only" ]]; then
    if [[ -z "$output_path" ]]; then
        output_path="$PWD/MBLINK-${version}-linux-native"
    fi
    install -Dm755 "$build_directory/mblink-linux" "$output_path"
    printf 'Native MBLINK executable created: %s\n' "$output_path"
    exit 0
fi

install_command=(cmake --install "$build_directory")
if [[ $EUID -eq 0 || -w "$prefix" || (! -e "$prefix" && -w "$(dirname "$prefix")") ]]; then
    "${install_command[@]}"
elif command -v sudo >/dev/null 2>&1; then
    sudo "${install_command[@]}"
else
    printf 'Installing to %s requires elevated access and sudo is unavailable.\n' "$prefix" >&2
    exit 1
fi

printf 'MBLINK %s was compiled natively, tested, and installed under %s.\n' "$version" "$prefix"
exit 0
__MBLINK_NATIVE_PAYLOAD_BELOW__
