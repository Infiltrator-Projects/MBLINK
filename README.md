<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

**MBLINK** is a C-first, open-source vehicle diagnostics platform with a native iPhone front end. Development begins with Mercedes-Benz and affordable ELM327-compatible Bluetooth Low Energy adapters, while the diagnostic engine is deliberately portable and independent of Apple frameworks.

The initial development vehicle is a Mercedes-Benz C207 E-Class diesel with the OM651 engine. The first hardware target is the **Vgate iCar Pro BLE 4.0**.

MBLINK is not designed as an iOS application with diagnostic logic buried in Swift. The permanent architecture is the opposite: **portable vehicle and protocol logic lives in C; platform code stays at the edge.**

## Design goals

- Keep the diagnostic engine in portable C11 wherever technically practical.
- Reuse **Infiltratr Common** instead of creating private duplicates of shared C functionality.
- Keep Swift limited primarily to the native SwiftUI presentation layer.
- Isolate Apple CoreBluetooth behind a small platform transport provider.
- Support standard OBD-II before adding manufacturer-specific extensions.
- Add Mercedes-Benz diagnostics as verified data definitions above reusable ISO-TP/UDS code.
- Make parsers, decoders and protocol state machines testable without a vehicle or Bluetooth adapter.
- Keep adapter quirks out of the generic diagnostic engine.
- Prefer capability discovery and verified behaviour over product-name assumptions.

## Architecture

```text
Native iPhone UI
    SwiftUI
       |
       v
Thin Apple application bridge
       |
       v
+-----------------------------+
|          libmblink          |
|       portable C11 core     |
|                             |
| ELM327 | OBD-II | ISO-TP    |
| UDS    | DTC    | logging   |
| Mercedes-Benz extensions    |
+-----------------------------+
       |
       v
C transport interface
       |
       v
Apple BLE provider
Objective-C / CoreBluetooth
       |
       v
Vgate iCar Pro BLE 4.0
       |
       v
Vehicle networks and ECUs
```

The C library must not depend on SwiftUI or CoreBluetooth. A future Linux, macOS, Windows or command-line front end should be able to reuse the same diagnostic engine.

See [Architecture](docs/ARCHITECTURE.md) for the detailed design.

## Shared C foundation

MBLINK consumes **Infiltratr Common** from [`The-First-Infiltrator/Infiltrator-Libraries`](https://github.com/The-First-Infiltrator/Infiltrator-Libraries) as a pinned Git submodule.

The current pin is:

- Infiltratr Common: **1.5.0**
- tag: **v1.5.0**
- commit: **`a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`**

The portable `core.c` and `format.c` components are used first. Shared bounded strings, parsing, checked arithmetic, formatting and project metadata must be reused when their contracts fit MBLINK rather than reimplemented locally.

Application-specific vehicle protocols do **not** belong in Infiltratr Common merely because they are written in C. New Common APIs should follow that library's existing real-consumer reuse rule.

See [Dependencies](docs/DEPENDENCIES.md).

## Implemented in 0.2

The portable C ELM327-compatible layer provides bounded command framing, fragmented response parsing, prompt/echo handling, adapter initialisation, transport-backed command ownership, timeout/cancellation safeguards, re-synchronisation protection and capability probing.

See [ELM327 Engine](docs/ELM327.md).

## Implemented in 0.3

MBLINK includes a standard OBD-II diagnostic layer in portable C. The 0.3.x implementation provides:

- supported-PID discovery across standard 32-PID blocks;
- typed live calculated load, coolant temperature, MAP, RPM, road speed, intake-air temperature, MAF and throttle position;
- Mode 02 freeze-frame decoding through the same PID formula table;
- Mode 01 PID 01 MIL/DTC/readiness state;
- Mode 09 PID 02 VIN extraction;
- stored, pending and permanent DTC decoding;
- ELM-compatible indexed long-response reassembly for VIN and longer DTC replies;
- explicit Mode 04 clear-code gating with readiness-reset acknowledgement;
- strict hexadecimal validation and ELM-error propagation;
- deterministic tests that require no vehicle, BLE adapter or Apple framework.

Version **0.3.1** hardened the standard layer for long CAN replies displayed by ELM-compatible adapters as indexed `0:`, `1:`, `2:` chunks. This reassembly stays at the ELM presentation boundary; the future raw ISO-TP engine remains a separate reusable layer.

See [Standard OBD-II Engine](docs/OBD2.md).

## Implemented in 0.4

Version **0.4.0** adds the native Apple transport and iPhone application edge while preserving the C-first architecture:

- Objective-C CoreBluetooth provider implementing `MblinkTransport`;
- BLE scanning, connection, service and characteristic discovery;
- deterministic writable/notify channel probing through the existing C ELM327 parser;
- bounded scan, connection, discovery and probe deadlines;
- controlled reconnect/rescan behaviour after transient failures;
- bounded BLE write queues with CoreBluetooth backpressure handling;
- Objective-C diagnostics orchestration over the existing C ELM327 and OBD-II engines;
- SwiftUI connection status plus live RPM and coolant presentation;
- Swift/MainActor interoperability without moving diagnostic logic into Swift;
- CI validation on Ubuntu C11, macOS C11 and an iOS Simulator Xcode build.

**Hardware validation is still pending.** The Vgate iCar Pro BLE 4.0 has not yet been physically tested with this build, so 0.4.0 does not claim verified Vgate GATT UUIDs, vehicle connectivity or stable live data from the development Mercedes. Those observations will be recorded when hardware is available.

See [Apple BLE and iPhone Layer](docs/APPLE.md).

## Next target

Development now moves to **0.5 — scheduler, dashboard and logging**.

The next software stage will add a portable C request scheduler, typed sample flow, configurable dashboard, time-series display and session/CSV logging without moving those responsibilities into the BLE provider or SwiftUI.

Hardware validation of the 0.4 Apple/Vgate path remains an active validation task in parallel and may produce narrow adapter-specific fixes if the real device exposes quirks not visible in simulator CI.

The long-term Mercedes direction includes DPF, exhaust temperature, regeneration, turbo/boost, fuel-rail, injector and EGR information where those values can be identified and validated against the real vehicle.

See [Roadmap](docs/ROADMAP.md).

## Repository layout

```text
.github/workflows/     Continuous integration
include/mblink/        Public C API
src/core/              Portable MBLINK C foundation
src/elm327/            Portable ELM327 command/session/probe engine
src/obd2/              Portable standard OBD-II engine
src/infiltratr-common/ Pinned Infiltratr Common submodule
platform/apple/        Apple-specific transport/application bridge
app/ios/               Native iPhone presentation layer
tests/                 Portable C regression tests
docs/                  Architecture and engineering documentation
```

Vehicle/protocol logic should not migrate into platform/application directories for convenience.

## Build the portable core

Requirements:

- C11 compiler
- CMake 3.20 or later
- Git, when initialising the shared-library submodule

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

CMake verifies the expected Infiltratr Common version and, when Git metadata is available, its exact pinned commit.

## Development policy

The portable C core is the source of truth for diagnostic behaviour. In particular:

- PID formulas do not belong in Swift.
- ELM327 response parsing does not belong in Objective-C.
- ISO-TP/UDS state machines do not belong in the UI.
- Mercedes data definitions do not belong in the BLE provider.
- Adapter-specific GATT quirks do not belong in `libmblink` protocol code.

Every manufacturer-specific value must have an explicit validation status. Captured real responses should become regression fixtures before experimental definitions are promoted to stable support.

## Current status

**Pre-alpha / version 0.4.0.**

The portable C foundation, ELM327 engine and standard OBD-II engine are implemented and tested on Ubuntu and macOS. The Apple CoreBluetooth/Objective-C/SwiftUI integration also builds successfully as an iOS Simulator target under Xcode CI. Physical BLE-adapter and vehicle validation is pending and is not represented as complete.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)
- [ELM327 engine](docs/ELM327.md)
- [Standard OBD-II engine](docs/OBD2.md)
- [Apple BLE and iPhone layer](docs/APPLE.md)
- [Adapter strategy](docs/ADAPTERS.md)
- [Dependencies and shared-code policy](docs/DEPENDENCIES.md)
- [Contributing](CONTRIBUTING.md)

## Licence

Copyright (C) 2026 Shannon Smith.

MBLINK is free software licensed under the **GNU General Public License, version 3 or (at your option) any later version** (`GPL-3.0-or-later`). Source files use SPDX identifiers where practical. The complete GPLv3 licence text is in [LICENSE](LICENSE).
