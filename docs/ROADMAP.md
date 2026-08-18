<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK grows from the portable C core outward. Every milestone must leave the repository buildable, tested and reusable.

**Current gate: 0.6.1 — pre-UDS quality hardening. Next feature milestone: 0.7 — UDS foundation in C.**

## Completed foundations

| Milestone | Result |
| --- | --- |
| 0.1 | Portable C11 core, public C API/transport ABI, pinned Infiltratr Common, strict CI |
| 0.2 | ELM327 command/parser/init/session engine with deterministic mock tests |
| 0.3 | Standard OBD-II PID discovery/decoding, VIN, readiness, freeze-frame and DTCs |
| 0.4 | Objective-C CoreBluetooth provider and native iPhone shell; physical hardware validation remains pending |
| 0.5 | C-owned scheduler, telemetry/history, full-session recording and SwiftUI dashboard/export |
| 0.6 | Reusable Classical-CAN ISO-TP TX/RX state machines independent of UDS/Mercedes |

Module contracts and limitations are documented in the corresponding files under `docs/`; this roadmap does not duplicate those specifications.

## 0.6.1 — quality and API hardening

No new diagnostic feature belongs in this milestone.

Targets:

- close lifetime, overflow and failure-state edge cases found in the pre-0.7 review;
- strengthen regression coverage and add sanitizer CI;
- keep build configuration deterministic;
- reduce redundant documentation/comments;
- clean small iPhone export/presentation issues;
- leave 0.7 with a sharper, more deliberate C contract.

**Exit condition:** the final hardening branch head passes Linux/macOS C tests, ASan+UBSan, and Debug/Release iOS Simulator builds.

## 0.7 — UDS foundation in C

- request/response model;
- positive and negative responses;
- diagnostic-session handling;
- ECU identification and data-identifier abstraction;
- timing/state behaviour for verified services;
- complete reuse of ISO-TP PDUs without duplicated segmentation or CAN addressing.

**Exit condition:** verified UDS exchanges execute through the reusable C engine without manufacturer interpretation in transport code.

## 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics

Validate useful engine data such as ECU identity, DPF state/pressure/temperature, turbo/boost, rail pressure, injector information, EGR and related diesel parameters.

Every undocumented Mercedes definition remains experimental until verified against real vehicle responses and regression fixtures.

**Exit condition:** verified information materially exceeds generic OBD-II capability on the development vehicle.

## 0.9 — additional Mercedes modules

Subject to real vehicle/network access: transmission, ABS/ESP, SRS, climate, instrument cluster and other discoverable ECUs. Start with identification, DTCs and selected reads before write-oriented functions.

## 0.10 — adapter portability

Formalise adapter capabilities/profiles, verified firmware quirks and additional ELM327-compatible transports. The Vgate reference adapter remains a validation target, not a `libmblink` dependency.

## 1.0 — release hardening

- stable documented ABI for supported functionality;
- provenance for verified fixtures/definitions;
- saved vehicle/session behaviour as appropriate;
- accessibility, performance and battery review;
- durable long-session storage;
- reproducible release process;
- security/privacy review before telemetry can leave the device.

Later work may include service functions, adaptations, additional manufacturers and non-iOS front ends. Firmware flashing and immobiliser/security programming remain outside the early roadmap.

## Development principle

Each feature begins at the lowest reusable layer that can correctly own it. UI convenience is never a reason to duplicate protocol logic outside the C core.
