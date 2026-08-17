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
  |- UDS
  `- manufacturer extensions
       |
       v
mblink_transport C ABI
       |
       +-- Apple BLE provider (Objective-C + CoreBluetooth)
       +-- future native providers
       `-- test/mock transport
       |
       v
Adapter -> OBD connector -> vehicle networks -> ECUs
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
- timeouts and retry policy at the diagnostic layer;
- transport-independent logging/session data;
- ISO-TP segmentation/reassembly;
- UDS request/response handling;
- manufacturer-specific definition lookup and decoding;
- explicit validation state for experimental manufacturer data.

If a protocol implementation can be written portably in C, it belongs here.

## 2. Public C ABI

Public headers live under `include/mblink/`.

The ABI must use ordinary C types and explicit ownership rules. Platform objects must never leak into public structures.

The transport boundary is callback/interface based. `libmblink` should be able to communicate through a real BLE provider, a future desktop transport or a deterministic mock without changing the parser or protocol engine.

The initial `mblink_transport` contract is intentionally small. It will grow only as real transport requirements are proven.

## 3. Infiltratr Common

MBLINK reuses the canonical shared C foundation from **Infiltratr Common**.

Initial dependency:

- version: `1.5.0`
- tag: `v1.5.0`
- commit: `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`

The dependency is a pinned Git submodule at `src/infiltratr-common`.

MBLINK initially compiles the portable Common components:

- `src/core.c`
- `src/format.c`

The POSIX provider is not automatically included in the iOS core. Any platform provider must be selected deliberately after its contracts are validated for that platform.

Existing Common functionality should be used where appropriate for:

- bounded strings;
- deterministic string handling;
- strict integer and decimal parsing;
- checked/saturating arithmetic;
- range validation and clamping;
- generic scalar and temperature/percentage formatting;
- project identity metadata;
- compiler annotations.

MBLINK must not create local helpers that merely duplicate an existing Common contract.

Conversely, OBD-II, ELM327, ISO-TP, UDS and Mercedes-specific code stays in MBLINK. Shared-library promotion occurs only when a capability satisfies Infiltratr Common's real-consumer rule.

## 4. Apple platform boundary

The Apple provider owns functionality that cannot be made platform-neutral, primarily CoreBluetooth integration.

The intended provider implementation is Objective-C because it can consume the C transport ABI directly while interoperating naturally with Apple frameworks. This keeps Swift out of protocol and transport internals.

Apple provider responsibilities include:

- CoreBluetooth scanning;
- peripheral connection/disconnection;
- GATT service discovery;
- characteristic discovery;
- notification/indication subscription;
- BLE write sizing and queuing;
- byte-stream delivery to the C transport boundary;
- translation of Apple connection errors into platform-neutral status codes.

The Apple provider does **not** decode OBD data or know Mercedes PIDs/DIDs.

## 5. SwiftUI application

Swift is used where it provides genuine platform value: native presentation, navigation, state binding and iOS lifecycle integration.

The SwiftUI application consumes stable results from the C core through a thin bridge. It should not contain alternate implementations of diagnostic logic.

SwiftUI areas may include:

- connection and vehicle status;
- live dashboard;
- parameter browser;
- diagnostic trouble codes;
- readiness and freeze-frame views;
- graphs and session history;
- Mercedes-specific views;
- settings and adapter selection.

## 6. Adapter layer

The first target is the **Vgate iCar Pro BLE 4.0**.

MBLINK does not assume all ELM327-compatible BLE products share UUIDs, firmware behaviour or buffering rules. Adapter-specific BLE details belong in platform profiles/providers.

The C diagnostic engine deals in logical adapter commands and responses, not GATT characteristics.

## 7. Standard OBD-II

Generic OBD-II is the first stable cross-vehicle capability.

Initial work covers:

- Mode 01 current powertrain data;
- supported-PID enumeration;
- Mode 03 stored DTCs;
- Mode 04 DTC clearing behind explicit user action;
- Mode 07 pending DTCs;
- Mode 09 vehicle information where supported;
- permanent DTCs where available;
- freeze-frame and readiness information.

Polling is capability-driven: MBLINK queries supported PID ranges and does not blindly poll unsupported values.

## 8. ISO-TP and UDS

Deeper Mercedes diagnostics must sit on reusable protocol layers.

ISO-TP and UDS are implemented in C independently of Mercedes definitions. This gives MBLINK a reusable foundation for multiple ECUs and, later, other manufacturers.

Planned UDS capabilities include:

- ECU identification;
- diagnostic sessions;
- data-identifier requests;
- negative-response handling;
- timing/state management required by verified services.

## 9. Mercedes-Benz extension

The first deep-diagnostic target is the C207 E-Class diesel / OM651 platform.

Candidate areas include:

- DPF differential pressure;
- DPF/exhaust temperatures;
- regeneration state and counters;
- turbo command/actual values;
- rail-pressure target/actual values;
- injector-related data;
- EGR command/actual data;
- ECU software/identity information;
- transmission and chassis modules.

No undocumented identifier is accepted merely because it appears in an online list. Definitions carry a validation state and are promoted only after verification against real vehicle responses and regression fixtures.

## 10. Request scheduler

The C scheduler prevents the UI from becoming the timing authority.

It is responsible for:

- polling groups and rates;
- priority for fast-changing values;
- lower rates for temperatures/status data;
- pausing polling for exclusive diagnostic operations;
- request timeout/retry policy;
- timestamping samples close to receipt;
- avoiding unnecessary adapter/bus load.

## 11. Data model and logging

Decoded samples retain machine-usable information rather than only display strings:

- stable parameter identifier;
- timestamp;
- source ECU/module;
- raw payload where useful;
- decoded numeric/enumerated value;
- unit/scale metadata;
- validation state for manufacturer-specific data.

The same samples drive dashboards, graphs, CSV export and regression analysis.

## 12. Write operations

The initial project is intentionally read-focused.

Operations that alter diagnostic state are separate from passive reads and require explicit user action. ECU coding, firmware flashing, immobiliser/security programming and similarly high-impact functions are outside the early implementation scope.

## 13. Testing

Portable C tests are mandatory for protocol behaviour.

Coverage will include:

- ELM327 response cleanup;
- standard PID formulas;
- supported-PID bitmaps;
- DTC decoding;
- malformed/truncated responses;
- timeout/retry state;
- ISO-TP segmentation/reassembly;
- UDS negative responses;
- Mercedes decoding from captured verified fixtures.

A mock C transport must make these tests independent of Bluetooth hardware.

Apple/CoreBluetooth testing is a separate platform test surface.

## Design rules

1. If changing the iPhone UI requires changing PID parsing, the separation is wrong.
2. If supporting another adapter requires changing Mercedes decoding, the separation is wrong.
3. If a parser exists in both Swift and C, the C implementation is the canonical one and the duplicate should be removed.
4. If MBLINK implements an existing Infiltratr Common contract locally, the local implementation should be removed.
5. If MBLINK needs a genuinely reusable helper another Infiltrator application also needs, evaluate it for Common rather than starting another private copy.
