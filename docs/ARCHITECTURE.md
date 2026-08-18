<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Architecture

MBLINK is a **C-first diagnostics platform**. The native iPhone application is the first front end, not the architectural centre of the project.

The governing rule is:

> Portable diagnostic behaviour belongs in C. Platform frameworks belong behind narrow interfaces at the edge.

This keeps vehicle protocol work reusable, testable and independent of one UI toolkit, operating system or adapter.

## High-level structure

```text
SwiftUI presentation
       |
       v
Thin Apple application bridge
       |
       v
libmblink (portable C11)
  |- project/session model
  |- ELM327 command and response engine
  |- standard OBD-II services
  |- PID capability discovery and decoding
  |- DTC decoding
  |- request scheduler
  |- logging/data model
  |- ISO-TP
  |- future UDS
  `- future manufacturer extensions
       |
       v
transport/provider boundaries
       |
       +-- Apple BLE provider (Objective-C + CoreBluetooth)
       +-- future raw CAN/native providers
       `-- deterministic test models
       |
       v
Adapter / CAN interface -> vehicle networks -> ECUs
```

## 1. Portable C core

`libmblink` owns diagnostic behaviour. It targets portable C11 and must not depend on Swift, SwiftUI, Objective-C runtime types or CoreBluetooth.

Core responsibilities include:

- adapter command construction and initialisation policy;
- ELM327-compatible response normalisation and parsing;
- standard SAE OBD-II services;
- supported-PID enumeration;
- PID formula decoding and typed values;
- DTC parsing;
- vehicle and ECU identification queries;
- diagnostic request scheduling;
- transport-independent logging/session data;
- ISO-TP segmentation, reassembly and flow control;
- future UDS request/response handling;
- manufacturer-specific definition lookup and decoding;
- explicit validation state for experimental manufacturer data.

If a protocol implementation can be written portably in C, it belongs here.

## 2. Public C ABI

Public headers live under `include/mblink/`.

The ABI uses ordinary C types and explicit ownership rules. Platform objects must never leak into public structures.

The ELM byte-stream transport boundary remains callback/interface based. ISO-TP additionally defines a transport-neutral Classical-CAN frame model so future native CAN or adapter-specific providers can exchange raw CAN frames without owning the ISO-TP state machine.

## 3. Infiltratr Common

MBLINK reuses the canonical shared C foundation from **Infiltratr Common**.

Current dependency:

- version: `1.5.0`
- tag: `v1.5.0`
- commit: `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`

The dependency is a pinned Git submodule at `src/infiltratr-common`.

MBLINK compiles the portable Common components `src/core.c` and `src/format.c`. Existing Common functionality is reused for bounded strings, deterministic parsing, checked/saturating arithmetic, formatting and project identity metadata rather than privately duplicated.

OBD-II, ELM327, ISO-TP, UDS and Mercedes-specific code stays in MBLINK unless a capability later satisfies Common's real-consumer rule.

## 4. Apple platform boundary

The Apple provider owns functionality that cannot be made platform-neutral, primarily CoreBluetooth integration.

Objective-C consumes the C transport ABI while interoperating with Apple frameworks. Swift remains outside protocol and transport internals.

Apple provider responsibilities include:

- CoreBluetooth scanning;
- peripheral connection/disconnection;
- GATT service and characteristic discovery;
- notification/indication subscription;
- BLE write sizing and queuing;
- byte-stream delivery to the C transport boundary;
- translation of Apple connection errors into platform-neutral status codes.

The Apple provider does **not** decode OBD data, implement ISO-TP, or know Mercedes PIDs/DIDs.

## 5. SwiftUI application

Swift is used for native presentation, navigation, state binding and iOS lifecycle integration. It consumes stable typed results from the C core through a thin bridge and must not contain alternate implementations of diagnostic logic.

Current/future UI areas include connection status, live dashboard, favourites, graphs/session history, DTCs, readiness/freeze-frame views and Mercedes-specific views.

## 6. Adapter layer

The first reference adapter is the **Vgate iCar Pro BLE 4.0**.

MBLINK does not assume all ELM327-compatible BLE products share UUIDs, firmware behaviour or buffering rules. Adapter-specific BLE details belong in platform profiles/providers.

The generic C diagnostic engine deals in logical adapter commands, responses and protocol objects rather than CoreBluetooth characteristics.

## 7. Standard OBD-II

Generic OBD-II is the first stable cross-vehicle diagnostic capability. The implemented C engine covers supported PID enumeration, common Mode 01 data, stored/pending/permanent DTCs, explicitly gated clearing, VIN, readiness and freeze-frame information.

Polling is capability-driven: MBLINK queries supported PID ranges and does not blindly poll unsupported values.

## 8. ISO-TP

Milestone 0.6 implements ISO-TP in portable C independently of UDS or Mercedes definitions.

The 0.6 contract deliberately targets **Classical CAN**:

- 8-byte CAN data fields;
- 11-bit and 29-bit identifiers;
- normal, extended and mixed addressing model;
- physical and functional target classification;
- Single, First, Consecutive and Flow Control frames;
- transmit segmentation and receive reassembly;
- 12-bit First Frame lengths up to 4095 bytes;
- sequence validation and wrap;
- block-size Flow Control;
- CTS, WAIT and OVERFLOW handling;
- millisecond and 100–900 microsecond STmin values;
- explicit receive/Flow-Control timeouts;
- caller-owned bounded receive storage;
- deterministic failed states requiring reset.

CAN FD, CAN-FD Single Frame escape encoding and 32-bit extended First Frame lengths are not claimed by 0.6. Unsupported extended-length First Frames are rejected explicitly rather than guessed at.

The ISO-TP API uses caller-provided monotonic microsecond timestamps. It does not read an OS clock or allocate hidden reassembly buffers.

See [ISO-TP Foundation](ISOTP.md).

## 9. UDS

UDS is the next reusable protocol layer. It will consume complete ISO-TP PDUs instead of duplicating CAN framing or segmentation.

Planned UDS capabilities include:

- request/response modelling;
- positive and negative responses;
- ECU identification;
- diagnostic sessions;
- data-identifier requests;
- timing/state management for verified services.

## 10. Mercedes-Benz extension

The first deep-diagnostic target is the C207 E-Class diesel / OM651 platform.

Candidate areas include DPF pressure/temperatures/regeneration, turbo command/actual data, rail-pressure target/actual data, injector information, EGR state, ECU identity, transmission and chassis modules.

No undocumented identifier is accepted merely because it appears in an online list. Definitions carry validation state and are promoted only after verification against real vehicle responses and regression fixtures.

## 11. Request scheduler

The C scheduler prevents the UI from becoming the timing authority. It owns polling groups/rates/priorities, pause/resume for exclusive operations, timestamping and bounded delayed catch-up without burst polling.

## 12. Data model and logging

Decoded samples retain machine-usable identifiers, timestamps, decoded values and units. The C telemetry layer owns latest values, recent history, favourites and full-session recording. The same typed data drives dashboards, graphs and CSV/session export.

## 13. Write operations

The initial project is intentionally read-focused.

Operations that alter diagnostic state are separate from passive reads and require explicit user action. ECU coding, firmware flashing, immobiliser/security programming and similarly high-impact functions are outside the early implementation scope.

## 14. Testing

Portable C tests are mandatory for protocol behaviour.

Current regression areas include:

- ELM327 response cleanup/state handling;
- standard PID formulas and supported-PID bitmaps;
- DTC/VIN/readiness/freeze-frame decoding;
- scheduler and telemetry ownership;
- ISO-TP SF/FF/CF/FC framing;
- segmentation/reassembly and sequence wrap;
- block-size/STmin Flow Control;
- malformed frames, wrong sequence numbers, buffer overflow and protocol timeouts;
- failed-state persistence until explicit reset.

Future tests will add UDS positive/negative responses and Mercedes decoding from captured verified fixtures.

The portable suite runs on Ubuntu and macOS. The iPhone target builds the same portable `mblink.c` translation unit in both Debug and Release Xcode CI, so ISO-TP cannot silently diverge into an Apple-specific implementation.

## Design rules

1. If changing the iPhone UI requires changing PID parsing, the separation is wrong.
2. If supporting another adapter requires changing Mercedes decoding, the separation is wrong.
3. If a parser/protocol state machine exists in Swift/Objective-C and C, the C implementation is canonical and the duplicate should be removed.
4. If MBLINK implements an existing Infiltratr Common contract locally, the local implementation should be removed.
5. If UDS starts reimplementing ISO-TP segmentation or CAN addressing, the layer boundary is wrong.
6. If MBLINK needs a genuinely reusable helper another Infiltrator application also needs, evaluate it for Common rather than starting another private copy.
