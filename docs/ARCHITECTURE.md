<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Architecture

MBLINK is C-first. iPhone and Linux are peer front ends over the same portable diagnostic engine and workspace model.

> Portable diagnostic behaviour belongs in C. Platform frameworks belong behind narrow interfaces at the edge.

## Layers

```text
SwiftUI presentation             GTK4 presentation
        |                               |
Objective-C application bridge     Linux application shell
        |                               |
        +----------- libmblink / portable C11 -----------+
                    shared workspace model
       parameters | ELM327 | OBD-II | scheduler | telemetry | ISO-TP | UDS
                                |
                    transport/provider boundary
                                |
           CoreBluetooth / Linux BLE, serial or CAN providers
                                |
                   adapter or CAN interface -> vehicle ECUs
```

### Portable C core

`libmblink` owns protocol and diagnostic behaviour: request construction, parsing, state machines, typed values, scheduling, telemetry/session data, shared diagnostic workspace definitions and manufacturer decoding.

Public headers live under `include/mblink/`. Public types use ordinary C data and explicit ownership/lifetime rules. Platform framework objects must never enter the C ABI.

ELM327 uses a byte-stream transport contract. ISO-TP uses a transport-neutral Classical-CAN frame model. UDS consumes complete ISO-TP PDUs instead of reimplementing segmentation or CAN addressing. Protocol-neutral parameter descriptors give OBD-II and manufacturer UDS scalar data one shared identity, metadata and formatting model for Live Data, Table, Dashboard and Graphs.

### Shared diagnostic workspace

The stable top-level workspace is defined in C as Vehicle, Modules, Faults, Live Data, Table, Dashboard, Graphs, Log and Settings. Platform shells render those sections natively; they do not invent different diagnostic taxonomies or duplicate protocol state.

The workspace contract is intentionally small. It defines shared navigation and application meaning without forcing SwiftUI and GTK4 to share rendering code.

### Shared C foundation

MBLINK consumes the pinned `src/infiltratr-common` submodule for generic cross-project primitives such as bounded strings, parsing, checked/saturating arithmetic, formatting and project metadata.

Protocol-specific behaviour remains in MBLINK. Common is reused when an existing contract matches; it is not modified from this repository and does not receive speculative vehicle-diagnostics helpers. The shared diagnostic parameter layer deliberately uses Common's scalar formatter rather than carrying another numeric precision/clamping/unavailable-value implementation.

The authoritative Common version and commit pin are enforced by CMake and the submodule itself rather than duplicated throughout the documentation.

### Apple boundary

Objective-C owns the Apple-specific edge: CoreBluetooth lifecycle, runtime GATT discovery, notification subscription, BLE write sizing/backpressure and translation into the C transport contract.

Objective-C does not decode OBD-II, implement ISO-TP/UDS or contain Mercedes definitions. The portable C ELM parser is also used while probing BLE command channels.

Swift/SwiftUI owns iPhone presentation, navigation and user interaction. It consumes typed results from the bridge and does not contain protocol formulas, canonical polling policy or alternate parser/state-machine implementations.

### Linux boundary

The native Linux shell uses GTK4 from C. It consumes the same `libmblink` workspace and diagnostic contracts as the iPhone application. Linux-specific BLE, serial and SocketCAN providers belong behind the same transport/provider boundary; they do not own manufacturer decoding or protocol policy.

GTK4 is an optional build dependency. Building the portable core never requires a desktop toolkit.

### Adapter policy

Vgate iCar Pro BLE 4.0 is the first hardware validation target, not an architectural dependency.

MBLINK discovers GATT services and characteristics at runtime and validates candidate command channels rather than assuming all ELM327-compatible BLE products share UUIDs or buffering behaviour. Observed hardware behaviour takes precedence over marketing labels. Adapter quirks stay in provider/profile code.

### Manufacturer extensions

Generic OBD-II, ISO-TP and UDS remain manufacturer-neutral. Mercedes-Benz definitions sit above those reusable layers and carry explicit validation status where identifiers are not yet proven against real vehicle responses.

Manufacturer UDS values can project into the protocol-neutral parameter model only after their decoding/provenance contract is defined; the parameter layer is not a shortcut around manufacturer verification.

## Timing, ownership and failure

Portable engines do not read OS clocks directly. Callers provide the documented monotonic time domain for scheduler/session/ISO-TP/UDS operations.

Buffers and provider/session lifetimes are explicit. Where a state machine enters a terminal error state, reuse requires the documented reset/resynchronisation operation rather than heuristic recovery.

The project remains read-focused. Operations that change diagnostic state are separated from passive reads and require explicit application-level user action.

## Testing boundary

Protocol behaviour must be testable without a vehicle. Portable C tests run on Linux and macOS, with an additional sanitizer gate. CI also compiles the GTK4 Linux shell and builds the iPhone target in Debug, Release and unsigned physical-device configurations.

Real hardware captures are validation evidence, not a replacement for deterministic regression tests.

## Design rules

1. If a portable parser or protocol state machine appears in Swift/Objective-C/GTK and C, the duplicate is wrong.
2. If supporting another adapter requires changing Mercedes decoding, the boundary is wrong.
3. If UDS starts owning ISO-TP framing, the boundary is wrong.
4. If MBLINK duplicates an existing Infiltratr Common contract, reuse Common instead.
5. If an undocumented manufacturer definition lacks real-response evidence, it remains experimental.
6. If iPhone and Linux expose different diagnostic concepts for the same core capability, the shared application boundary is incomplete.
