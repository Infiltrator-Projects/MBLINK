<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

MBLINK is a C-first, open-source vehicle diagnostics platform with native iPhone and Linux front ends. Portable diagnostic behaviour and the shared diagnostic workspace live in C; platform frameworks stay behind narrow interfaces at the edge.

The initial vehicle focus is Mercedes-Benz. The first reference BLE adapter is the Vgate iCar Pro BLE 4.0, but adapter-specific behaviour is not part of the diagnostic core.

## Status

**Pre-alpha. Current release gate: 0.6.3 — trusted-runner integration before UDS.**

Implemented today:

- portable C11 core and transport ABI;
- shared Vehicle / Modules / Faults / Live Data / Table / Dashboard / Graphs / Log / Settings workspace contract;
- ELM327 command, parser, initialisation and session engines;
- standard OBD-II PID, readiness, VIN, freeze-frame and DTC handling;
- portable polling scheduler, telemetry history and session recording;
- Classical-CAN ISO-TP transmit/receive state machines;
- Objective-C CoreBluetooth provider and thin Apple application bridge;
- SwiftUI diagnostic workspace with live data, table, dashboard, graphs and CSV export;
- native GTK4 Linux workspace shell consuming the same C model;
- Ubuntu/macOS C CI, GTK4 Linux build CI, Debug/Release iOS Simulator builds and unsigned physical-iPhone IPA builds.

Physical iPhone installation has been validated. Live Vgate/vehicle communication and Mercedes-Benz ECU discovery remain pending; simulator/device build success does not prove vehicle protocol behaviour.

## Architecture

```text
SwiftUI / Objective-C              GTK4 / Linux shell
          \                          /
           +---- shared libmblink C11 ----+
             workspace | ELM327 | OBD-II
             scheduler | telemetry | ISO-TP
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

The native project is `app/ios/MBLINK.xcodeproj`. It builds the same portable C core used by CMake; Swift and Objective-C do not contain alternate ELM327, OBD-II or ISO-TP implementations.

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
- PID formulas, polling policy and canonical telemetry state do not belong in Swift or Linux presentation code.
- UDS must consume ISO-TP PDUs rather than duplicate segmentation or CAN addressing.
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
- [Apple](docs/APPLE.md) — CoreBluetooth/iPhone boundary and hardware validation
- [Contributing](CONTRIBUTING.md) — build and contribution rules

## Licence

Copyright (C) 2026 Shannon Smith.

MBLINK is licensed under `GPL-3.0-or-later`. See [LICENSE](LICENSE).
