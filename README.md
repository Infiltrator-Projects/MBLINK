<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

MBLINK is a C-first, open-source vehicle diagnostics platform with native iPhone and Linux front ends. Portable diagnostic behaviour and the shared diagnostic workspace live in C; platform frameworks stay behind narrow interfaces at the edge.

The initial vehicle focus is Mercedes-Benz. The first reference BLE adapter is the Vgate iCar Pro BLE 4.0, but adapter-specific behaviour is not part of the diagnostic core.

## Status

**Pre-alpha. Current development release target: 0.7.13 — official cross-platform MBLINK emblem, consolidated iPhone command interface and functional portable-model GTK4 workspaces. Active feature milestone: 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics.**

Implemented today:

- portable C11 core and transport ABI;
- shared Vehicle / Modules / Faults / Live Data / Table / Dashboard / Graphs / Log / Settings workspace contract, with a dedicated Mercedes diesel workspace in the iPhone shell;
- ELM327 command, parser, initialisation and session engines, plus an ELM-managed ISO 15765 CAN-channel contract;
- standard OBD-II PID, readiness, VIN, freeze-frame and DTC handling;
- complete chained OBD-II supported-PID discovery;
- automatic read-only stored, pending and permanent OBD-II fault scans;
- standard diesel/aftertreatment live-data decoding where advertised by the vehicle;
- portable polling scheduler, telemetry history and session recording;
- Classical-CAN ISO-TP transmit/receive state machines;
- portable UDS positive/negative responses, diagnostic sessions, TesterPresent, P2/P2* timing and caller-supplied DID definitions;
- portable read-only UDS `ReadDTCInformation` codec plus live CRD3 engine fault-memory collection;
- protocol-neutral C diagnostic parameter descriptors with Infiltratr Common-backed scalar formatting;
- bounded protocol-neutral parameter state/history keyed by protocol, module and identifier;
- 64-item live-data scheduler with full parameter keys;
- Infiltratr Common 1.10-backed portable primitives and authoritative CMake/Xcode target integration;
- Mercedes-Benz definition/profile foundation with candidate, source-corroborated and vehicle-verified provenance states;
- source-corroborated C207/W207 E 250 CDI / OM651 / Delphi CRD3.x engine target at physical 11-bit CAN `0x7E0 → 0x7E8`, while still withholding vehicle-verified status until the development car confirms it;
- read-only Mercedes ECU endpoint probing over the ELM-managed CAN and UDS layers;
- automatic iPhone probe flow after generic OBD-II capability discovery;
- standardized UDS VIN DID `F190` evidence capture;
- bounded standardized ECU-identity reads for `F18C`, `F187`, `F188`, `F189`, `F191` and `F197`;
- a second bounded Mercedes CRD3 fingerprint pass using evidence-only reads `F100`, `F154`, `F196`, `1001` and `1002` from the open-source CaesarSuite CRD3 model;
- decoded CRD3 family evidence for session/variant and supplier identity, including a corroborated OM651/CDID3/Delphi family signature;
- independent positive/negative/no-response/invalid classification for CRD3 fingerprint reads, also exposed in the combined Mercedes identity evidence list;
- live read-only Mercedes UDS fault-memory display on iPhone plus standard OBD-II faults;
- complete raw diagnostic-evidence export from the iPhone Evidence workspace even when no live-data samples were recorded;
- a full black, charcoal and brushed-silver iPhone branding overhaul with a vehicle-command-centre home screen, premium diagnostic panels, instrument-style telemetry tiles, dedicated fault presentation and the official MBLINK AppIcon emblem;
- matching black-and-silver GTK4 Linux workspace presentation using the same official emblem and portable C catalogues;
- visible iPhone project identity with `Copyright © 2026 Shannon Smith`, About, author/version/licence information and matching iOS copyright metadata;
- Objective-C CoreBluetooth provider and thin Apple application bridge;
- native GTK4 Linux workspace shell consuming the same C model;
- Ubuntu/macOS C CI, GTK4 Linux build CI, Debug/Release iOS Simulator builds and unsigned physical-iPhone IPA builds.

The CRD3 fingerprint identifiers are used only to establish ECU-family/variant evidence. MBLINK does not label their payloads as DPF soot load, regeneration state, injector corrections, rail pressure, boost or EGR values. Those manufacturer-specific meanings remain gated on a reproducible physical C207/OM651 capture and regression fixture rather than guessed formulas.

## Architecture

```text
SwiftUI / Objective-C              GTK4 / Linux shell
          \                          /
           +---- shared libmblink C11 ----+
         workspace | parameters | ELM327 | OBD-II
          scheduler | telemetry | ISO-TP | UDS
                         |
                 transport/provider boundary
                         |
                  adapter -> vehicle
```

Detailed boundaries and invariants are in [Architecture](docs/ARCHITECTURE.md). Milestone history and future work are in [Roadmap](docs/ROADMAP.md).

## Build the portable core

Requirements: a C11 compiler, CMake 3.20 or later, and Git for the pinned Infiltratr Common submodule.

```sh
git clone --recurse-submodules https://github.com/The-First-Infiltrator/MBLINK.git
cd MBLINK
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

If the repository was cloned without submodules:

```sh
git submodule update --init --recursive
```

CMake validates the required Common version and, when Git metadata is available, its exact pinned commit. Common 1.10 owns its portable target membership; MBLINK links the exported Common target rather than enumerating Common implementation files.

For an AddressSanitizer + UndefinedBehaviorSanitizer development build:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DMBLINK_ENABLE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

## iPhone target

The native project is `app/ios/MBLINK.xcodeproj`. It builds the same portable C core used by CMake; Swift and Objective-C do not contain alternate ELM327, OBD-II, ISO-TP, UDS or Mercedes protocol implementations. The project links Common's authoritative `InfiltratrCommonPortable` Xcode product rather than duplicating Common's internal source list.

The iPhone shell uses the official MBLINK AppIcon artwork as its emblem throughout the command centre, Settings and About. Its visual language is deep black/charcoal, brushed silver, restrained status colours, compact technical typography and instrument-style live telemetry. The home screen is a vehicle command centre rather than a generic SwiftUI list.

For a physical 0.7.13 test, connect to the C207/OM651 vehicle and let the connection sequence complete. After ordinary capability discovery MBLINK probes the source-corroborated Mercedes engine endpoint, reads VIN/standard ECU identity, issues the CRD3 fingerprint reads `22F100`, `22F154`, `22F196`, `221001` and `221002`, then reads Mercedes UDS fault information before restoring normal OBD-II, scanning standard faults and beginning live polling. Export Evidence → diagnostic evidence; that raw capture is what converts the next OM651 work from research evidence into vehicle-bound definitions.

## Linux target

The Linux shell is written in C with GTK4. On a system with GTK4 development files installed:

```sh
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release -DMBLINK_BUILD_LINUX_APP=ON
cmake --build build-linux --target mblink-linux
./build-linux/mblink-linux
```

The Linux shell renders the shared workspace, Mercedes profile and portable parameter catalogues and installs the official MBLINK emblem as its desktop icon. A Linux BLE, serial or SocketCAN provider is not implemented yet, so the interface reports transport-dependent values as unavailable rather than presenting simulated vehicle data. The desktop toolkit is optional: building only `libmblink` does not require GTK4.

Each published release includes two Linux artefacts alongside the unsigned iPhone IPA:

- `MBLINK-X.Y.Z-linux-amd64.deb` is the prebuilt Debian/Ubuntu package.
- `MBLINK-X.Y.Z-linux-native.run` contains the complete source tree, including the pinned Common library, and compiles and tests MBLINK natively before installation.

Install the Debian package with `sudo apt install ./MBLINK-X.Y.Z-linux-amd64.deb`. To compile natively, make the `.run` executable and launch it; `--help`, `--build-only` and `--extract DIR` are also available.

## Engineering rules

- Portable protocol and shared diagnostic workspace behaviour belongs in C.
- ELM327 parsing does not belong in Objective-C or GTK code.
- PID/DID formulas, polling policy and canonical telemetry state do not belong in Swift or Linux presentation code.
- UDS consumes complete ISO-TP PDUs and must not duplicate segmentation or CAN addressing.
- Mercedes definitions do not belong in BLE or desktop providers.
- Adapter quirks stay at the provider/profile boundary.
- Undocumented manufacturer data remains experimental until verified against real responses and regression fixtures.
- Reuse Infiltratr Common when its existing contract matches; do not modify the pinned submodule from this repository.

## Documentation

- [Architecture](docs/ARCHITECTURE.md) — layer ownership and invariants
- [Roadmap](docs/ROADMAP.md) — milestone history and future work
- [ELM327](docs/ELM327.md) — adapter/session contracts
- [Standard OBD-II](docs/OBD2.md) — supported generic diagnostics
- [Telemetry](docs/TELEMETRY.md) — scheduling, history and recording
- [ISO-TP](docs/ISOTP.md) — transport-layer scope and state machines
- [UDS](docs/UDS.md) — response, session, timing and DID contracts
- [Mercedes-Benz diagnostics](docs/MERCEDES.md) — C207/OM651 endpoint provenance, CRD3 fingerprinting and verification policy
- [Diagnostic parameters](docs/PARAMETERS.md) — shared OBD/UDS live-value identity and formatting contract
- [Common reuse](docs/COMMON-REUSE.md) — shared-library ownership and reuse boundary
- [Apple](docs/APPLE.md) — CoreBluetooth/iPhone boundary and hardware validation
- [Contributing](.github/CONTRIBUTING.md) — build and contribution rules

## Licence

Copyright (C) 2026 Shannon Smith.

MBLINK is licensed under `GPL-3.0-or-later`. See [LICENSE](LICENSE).
