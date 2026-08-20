<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

MBLINK is a C-first, open-source vehicle diagnostics platform with native iPhone and Linux front ends. Portable diagnostic behaviour and the shared diagnostic workspace live in C; platform frameworks stay behind narrow interfaces at the edge.

The initial vehicle focus is Mercedes-Benz. The first reference BLE adapter is the Vgate iCar Pro BLE 4.0, but adapter-specific behaviour is not part of the diagnostic core.

## Status

**Pre-alpha. Latest completed maintenance milestone: 0.7.3 — ELM-managed CAN and shared-timing recovery hardening. Next feature milestone: 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics.**

Implemented today:

- portable C11 core and transport ABI;
- shared Vehicle / Modules / Faults / Live Data / Table / Dashboard / Graphs / Log / Settings workspace contract;
- ELM327 command, parser, initialisation and session engines, plus an ELM-managed ISO 15765 CAN-channel contract;
- standard OBD-II PID, readiness, VIN, freeze-frame and DTC handling;
- portable polling scheduler, telemetry history and session recording;
- Classical-CAN ISO-TP transmit/receive state machines;
- portable UDS positive/negative responses, diagnostic sessions, TesterPresent, P2/P2* timing and caller-supplied DID definitions;
- protocol-neutral C diagnostic parameter descriptors with Infiltratr Common-backed scalar formatting for OBD-II and future manufacturer UDS live values;
- bounded protocol-neutral parameter state/history keyed by protocol, module and identifier;
- 64-item live-data scheduler with full parameter keys plus compatibility wrappers for the current OBD-II session loop;
- Infiltratr Common 1.9-backed portable primitives and periodic deadline advancement with identical CMake and iPhone source wiring;
- Mercedes-Benz definition/profile foundation with explicit candidate versus vehicle-verified provenance states;
- read-only Mercedes ECU endpoint probing over the ELM-managed CAN and UDS layers, with a provenance-labelled C207/OM651 engine-address candidate;
- Objective-C CoreBluetooth provider and thin Apple application bridge;
- SwiftUI diagnostic workspace whose live data, table, dashboard and graphs consume the shared C parameter catalog, plus CSV export;
- native GTK4 Linux workspace shell consuming the same C model;
- Ubuntu/macOS C CI, GTK4 Linux build CI, Debug/Release iOS Simulator builds, unsigned physical-iPhone IPA builds and trusted self-hosted Linux smoke validation.

Physical iPhone installation has been validated. Live Vgate/vehicle communication and Mercedes-Benz ECU discovery remain pending; simulator/device build success and deterministic UDS fixtures do not prove Mercedes vehicle behaviour.

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

CMake validates the required Common version and, when Git metadata is available, its exact pinned commit. Configuration never modifies the checkout to fetch a missing dependency for you.

For an AddressSanitizer + UndefinedBehaviorSanitizer development build:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DMBLINK_ENABLE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

## iPhone target

The native project is `app/ios/MBLINK.xcodeproj`. It builds the same portable C core used by CMake; Swift and Objective-C do not contain alternate ELM327, OBD-II, ISO-TP, UDS or Mercedes protocol implementations.

## Linux target

The Linux shell is written in C with GTK4. On a system with GTK4 development files installed:

```sh
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release -DMBLINK_BUILD_LINUX_APP=ON
cmake --build build-linux --target mblink-linux
./build-linux/mblink-linux
```

The desktop toolkit is optional: building only `libmblink` does not require GTK4.

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
- [Mercedes-Benz diagnostics](docs/MERCEDES.md) — C207/OM651 endpoint provenance, probing and verification policy
- [Diagnostic parameters](docs/PARAMETERS.md) — shared OBD/UDS live-value identity and formatting contract
- [Common reuse](docs/COMMON-REUSE.md) — shared-library ownership and reuse boundary
- [Apple](docs/APPLE.md) — CoreBluetooth/iPhone boundary and hardware validation
- [Contributing](.github/CONTRIBUTING.md) — build and contribution rules

## Licence

Copyright (C) 2026 Shannon Smith.

MBLINK is licensed under `GPL-3.0-or-later`. See [LICENSE](LICENSE).
