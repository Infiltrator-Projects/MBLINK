<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Architecture

MBLINK is C-first. The iPhone app is one front end, not the centre of the system.

> Portable diagnostic behaviour belongs in C. Platform frameworks belong behind narrow interfaces at the edge.

## Layers

```text
SwiftUI presentation
        |
Objective-C application bridge
        |
libmblink / portable C11
  ELM327 | OBD-II | scheduler | telemetry | ISO-TP | future UDS
        |
transport/provider boundary
        |
Apple CoreBluetooth provider / future native providers
        |
adapter or CAN interface -> vehicle ECUs
```

### Portable C core

`libmblink` owns protocol and diagnostic behaviour: request construction, parsing, state machines, typed values, scheduling, telemetry/session data and manufacturer decoding.

Public headers live under `include/mblink/`. Public types use ordinary C data and explicit ownership/lifetime rules. Platform framework objects must never enter the C ABI.

ELM327 uses a byte-stream transport contract. ISO-TP uses a transport-neutral Classical-CAN frame model. UDS must consume complete ISO-TP PDUs instead of reimplementing segmentation or CAN addressing.

### Shared C foundation

MBLINK consumes the pinned `src/infiltratr-common` submodule for generic cross-project primitives such as bounded strings, parsing, checked/saturating arithmetic, formatting and project metadata.

Protocol-specific behaviour remains in MBLINK. Common is reused when an existing contract matches; it is not modified from this repository and does not receive speculative vehicle-diagnostics helpers.

The authoritative Common version and commit pin are enforced by CMake and the submodule itself rather than duplicated throughout the documentation.

### Apple boundary

Objective-C owns the Apple-specific edge: CoreBluetooth lifecycle, runtime GATT discovery, notification subscription, BLE write sizing/backpressure and translation into the C transport contract.

Objective-C does not decode OBD-II, implement ISO-TP/UDS or contain Mercedes definitions. The portable C ELM parser is also used while probing BLE command channels.

Swift/SwiftUI owns presentation, navigation and user interaction. It consumes typed results from the bridge and does not contain protocol formulas, canonical polling policy or alternate parser/state-machine implementations.

### Adapter policy

Vgate iCar Pro BLE 4.0 is the first hardware validation target, not an architectural dependency.

MBLINK discovers GATT services and characteristics at runtime and validates candidate command channels rather than assuming all ELM327-compatible BLE products share UUIDs or buffering behaviour. Observed hardware behaviour takes precedence over marketing labels. Adapter quirks stay in provider/profile code.

### Manufacturer extensions

Generic OBD-II, ISO-TP and UDS remain manufacturer-neutral. Mercedes-Benz definitions sit above those reusable layers and carry explicit validation status where identifiers are not yet proven against real vehicle responses.

## Timing, ownership and failure

Portable engines do not read OS clocks directly. Callers provide the documented monotonic time domain for scheduler/session/ISO-TP operations.

Buffers and provider/session lifetimes are explicit. Where a state machine enters a terminal error state, reuse requires the documented reset/resynchronisation operation rather than heuristic recovery.

The project remains read-focused. Operations that change diagnostic state are separated from passive reads and require explicit application-level user action.

## Testing boundary

Protocol behaviour must be testable without a vehicle. Portable C tests run on Linux and macOS, with an additional sanitizer gate. The iPhone target builds the same C implementation in Debug and Release simulator CI.

Real hardware captures are validation evidence, not a replacement for deterministic regression tests.

## Design rules

1. If a portable parser or protocol state machine appears in Swift/Objective-C and C, the duplicate is wrong.
2. If supporting another adapter requires changing Mercedes decoding, the boundary is wrong.
3. If UDS starts owning ISO-TP framing, the boundary is wrong.
4. If MBLINK duplicates an existing Infiltratr Common contract, reuse Common instead.
5. If an undocumented manufacturer definition lacks real-response evidence, it remains experimental.
