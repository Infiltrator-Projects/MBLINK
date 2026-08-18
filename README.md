<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

**MBLINK** is a C-first, open-source vehicle diagnostics platform with a native iPhone front end. The portable diagnostic engine is deliberately independent of Apple frameworks so the same protocol and vehicle logic can later be reused by other front ends.

The initial development focus is Mercedes-Benz, with the Vgate iCar Pro BLE 4.0 as the first reference BLE adapter. Adapter-specific behaviour stays at the platform edge; it is not built into the diagnostic core.

## Current status

**Pre-alpha / version 0.6.0.**

MBLINK currently provides a tested portable C11 diagnostic foundation, ELM327-compatible command/session handling, standard OBD-II decoding, C-owned scheduling/telemetry/session recording, a reusable Classical-CAN ISO-TP engine, and a native Apple BLE/iPhone integration.

Software CI builds and tests the portable C core on Ubuntu and macOS and builds the native iPhone target in both Debug and Release configurations for the iOS Simulator.

**Physical BLE/vehicle validation remains pending.** The reference Vgate adapter has not yet been exercised with this build, so the project does not claim verified Vgate GATT UUIDs or proven live data from the development vehicle. Hardware findings will be recorded when the adapter is available.

## Architecture

```text
SwiftUI presentation
       |
       v
Thin Objective-C application bridge
       |
       v
+------------------------------------+
|          libmblink / C11           |
|                                    |
| ELM327 | OBD-II | scheduler        |
| telemetry | ISO-TP | recorder      |
| future UDS / Mercedes extensions   |
+------------------------------------+
       |
       v
C transport ABI / future CAN provider
       |
       v
Objective-C / CoreBluetooth provider
       |
       v
BLE diagnostic adapter -> vehicle
```

The governing rule is simple: **portable diagnostic behaviour belongs in C; platform frameworks belong behind narrow interfaces at the edge.**

See [Architecture](docs/ARCHITECTURE.md).

## Implemented milestones

### 0.1 — C foundation

- Portable C11 `libmblink`.
- Public C API and transport ABI.
- Strict warning-as-error builds and regression tests.
- Pinned Infiltratr Common dependency.
- Linux/macOS CI.

### 0.2 — ELM327 engine

- Bounded command construction.
- Fragmented response parsing and prompt/echo handling.
- ELM error classification.
- Adapter initialisation state machine.
- One-command-at-a-time session ownership.
- Timeout, cancellation and re-synchronisation protection.
- Adapter/protocol capability probing.

See [ELM327 Engine](docs/ELM327.md).

### 0.3 — Standard OBD-II

- Supported-PID discovery.
- Typed Mode 01 values including RPM, speed, coolant, MAP, MAF, intake temperature, throttle and calculated load.
- Readiness information.
- Freeze-frame decoding.
- VIN extraction.
- Stored, pending and permanent DTC decoding.
- Explicitly gated Mode 04 clearing.
- Indexed long-response reassembly for ELM-compatible adapters.

See [Standard OBD-II Engine](docs/OBD2.md).

### 0.4 — Apple BLE and iPhone layer

- Objective-C CoreBluetooth provider implementing the C transport ABI.
- Runtime service/characteristic discovery and C-parsed `ATI` channel probing.
- Scan/connect/discovery/probe deadlines.
- Controlled reconnect behaviour.
- Bounded BLE queues and CoreBluetooth backpressure handling.
- Thin Objective-C diagnostics bridge.
- SwiftUI iPhone shell.
- Xcode Simulator CI.

Hardware validation of this path is still pending.

See [Apple BLE and iPhone Layer](docs/APPLE.md).

### 0.5 — Scheduler, dashboard and logging

- Portable C request scheduler with explicit priorities and rates.
- Capability-driven default schedule for eight standard live PIDs.
- Pause/resume semantics for exclusive diagnostic operations.
- Delayed-request catch-up without burst polling.
- Typed latest-value cache and bounded chronological dashboard history.
- C-owned parameter favourites.
- Full-session streaming recorder independent of the recent-history ring.
- Diagnostic command/response transcript capture.
- C-formatted CSV session data and metadata.
- SwiftUI dashboard, favourites, charts and iOS share/export path.

See [Telemetry, Scheduling and Logging](docs/TELEMETRY.md).

### 0.6 — ISO-TP foundation

- Portable ISO-TP state machines in C11.
- Classical CAN, 8-byte data frames and PDUs up to 4095 bytes.
- 11-bit and 29-bit CAN identifiers.
- Normal, extended and mixed addressing model.
- Physical and functional target classification.
- Single, First, Consecutive and Flow Control frames.
- Segmentation and bounded caller-owned reassembly.
- Sequence-number validation including wrap from `0xF` to `0x0`.
- Receiver block-size Flow Control.
- CTS, WAIT and OVERFLOW handling.
- STmin support including 100–900 microsecond encodings.
- Explicit receive and Flow Control timeouts.
- Deterministic failed states that require reset before reuse.
- Regression coverage independent of Mercedes-Benz or UDS logic.

CAN FD and extended-length ISO-TP are deliberately not claimed by 0.6.

See [ISO-TP Foundation](docs/ISOTP.md).

## Shared C foundation

MBLINK consumes **Infiltratr Common** from `The-First-Infiltrator/Infiltrator-Libraries` as a pinned Git submodule.

Current pin:

- version: **1.5.0**
- tag: **v1.5.0**
- commit: `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`

MBLINK currently uses the portable Common `core.c` and `format.c` components. Existing Common contracts are reused for bounded strings, checked/saturating arithmetic, formatting and project metadata instead of being privately duplicated.

Protocol-specific code remains in MBLINK unless it later proves genuinely reusable by another consumer.

See [Dependencies](docs/DEPENDENCIES.md).

## Repository layout

```text
.github/workflows/     Continuous integration and release automation
include/mblink/        Public C API
src/core/              Portable project/scheduler/telemetry/ISO-TP foundation
src/elm327/            Portable ELM327 engine
src/obd2/              Portable standard OBD-II engine
src/infiltratr-common/ Pinned Infiltratr Common submodule
platform/apple/        Objective-C Apple transport/application bridge
app/ios/               Native SwiftUI iPhone front end
tests/                 Portable C regression tests
docs/                  Architecture and engineering documentation
```

## Build the portable core

Requirements:

- C11 compiler
- CMake 3.20 or later
- Git for the shared-library submodule

```sh
git clone --recurse-submodules https://github.com/The-First-Infiltrator/MBLINK.git
cd MBLINK
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

If cloned without submodules:

```sh
git submodule update --init --recursive
```

CMake verifies both the expected Infiltratr Common version and, when Git metadata is available, the exact pinned commit.

## iPhone target

The native iPhone project lives at:

```text
app/ios/MBLINK.xcodeproj
```

The application links a static `MBLINKCore` target containing the same portable MBLINK and Infiltratr Common C sources used by the CMake build. ISO-TP is included in that same portable C translation unit; SwiftUI does not contain alternate ELM/OBD/ISO-TP implementations or polling policy.

## Development policy

- PID formulas belong in C, not Swift.
- ELM327 response parsing belongs in C, not Objective-C.
- Poll scheduling and sample/session logic belong in C, not the UI.
- ISO-TP and UDS state machines belong in C.
- Mercedes definitions do not belong in the BLE provider.
- Adapter-specific GATT quirks do not belong in generic protocol code.
- Manufacturer-specific definitions require explicit validation status and real-response fixtures before being considered stable.

## Next target

Development now moves to **0.7 — UDS foundation in C**.

UDS will consume complete ISO-TP PDUs and add request/response modelling, positive and negative response handling, ECU identification, diagnostic-session management, data-identifier abstractions and verified timing/state behaviour without duplicating ISO-TP segmentation or CAN addressing.

The 0.4 physical Vgate/iPhone/vehicle validation remains an active parallel validation task and may produce narrow adapter-specific corrections when hardware becomes available.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)
- [ELM327 engine](docs/ELM327.md)
- [Standard OBD-II engine](docs/OBD2.md)
- [Apple BLE and iPhone layer](docs/APPLE.md)
- [Telemetry, scheduling and logging](docs/TELEMETRY.md)
- [ISO-TP foundation](docs/ISOTP.md)
- [Adapter strategy](docs/ADAPTERS.md)
- [Dependencies and shared-code policy](docs/DEPENDENCIES.md)
- [Contributing](CONTRIBUTING.md)

## Licence

Copyright (C) 2026 Shannon Smith.

MBLINK is free software licensed under the **GNU General Public License, version 3 or (at your option) any later version** (`GPL-3.0-or-later`). Source files use SPDX identifiers where practical. The complete GPLv3 licence text is in [LICENSE](LICENSE).
