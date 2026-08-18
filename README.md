<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

MBLINK is a C-first, open-source vehicle diagnostics platform with a native iPhone front end. Portable diagnostic behaviour lives in C; platform frameworks stay behind narrow interfaces at the edge.

The initial vehicle focus is Mercedes-Benz. The first reference BLE adapter is the Vgate iCar Pro BLE 4.0, but adapter-specific behaviour is not part of the diagnostic core.

## Status

**Pre-alpha. Latest completed feature milestone: 0.6 — ISO-TP foundation.**

Implemented today:

- portable C11 core and transport ABI;
- ELM327 command, parser, initialisation and session engines;
- standard OBD-II PID, readiness, VIN, freeze-frame and DTC handling;
- portable polling scheduler, telemetry history and session recording;
- Classical-CAN ISO-TP transmit/receive state machines;
- Objective-C CoreBluetooth provider and thin Apple application bridge;
- SwiftUI dashboard, favourites, charts and CSV sharing;
- Ubuntu/macOS C CI plus Debug/Release iOS Simulator builds.

Physical Vgate/iPhone/vehicle validation is still pending. Simulator CI proves source/framework integration, not BLE radio behaviour or live vehicle communication.

## Architecture

```text
SwiftUI presentation
        |
Objective-C application bridge
        |
libmblink / C11
  ELM327 | OBD-II | scheduler | telemetry | ISO-TP
        |
C transport/provider boundary
        |
Objective-C CoreBluetooth provider / future native providers
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

## Engineering rules

- Portable protocol behaviour belongs in C.
- ELM327 parsing does not belong in Objective-C.
- PID formulas, polling policy and canonical telemetry state do not belong in Swift.
- UDS must consume ISO-TP PDUs rather than duplicate segmentation or CAN addressing.
- Mercedes definitions do not belong in BLE providers.
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
