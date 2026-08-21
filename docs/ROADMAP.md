<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK grows from the portable C core outward. Every milestone must leave the repository buildable, tested and reusable.

**Completed through test release 0.7.6. Active feature milestone: 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics.**

## Completed foundations

| Milestone | Result |
| --- | --- |
| 0.1 | Portable C11 core, public C API/transport ABI, pinned Infiltratr Common, strict CI |
| 0.2 | ELM327 command/parser/init/session engine with deterministic mock tests |
| 0.3 | Standard OBD-II PID discovery/decoding, VIN, readiness, freeze-frame and DTCs |
| 0.4 | Objective-C CoreBluetooth provider and native iPhone shell; physical BLE/vehicle validation remains pending |
| 0.5 | C-owned scheduler, telemetry/history, full-session recording and SwiftUI dashboard/export |
| 0.6 | Reusable Classical-CAN ISO-TP TX/RX state machines independent of UDS/Mercedes |
| 0.6.1 | Lifetime, overflow and failure-state hardening; sanitizer CI and device-IPA build path |
| 0.6.2 | Scheduler fairness, transactional OBD samples, stricter VIN validation, Apple lifecycle and release-identity hardening |
| 0.6.3 | Trusted self-hosted Linux smoke validation for the main development branch |
| 0.7 | Portable UDS response/session/timing engine, TesterPresent and caller-supplied DID definitions |
| 0.7.1 | Common 1.7.0 adoption, iPhone Mercedes-source wiring and permanent IPA release assets |
| 0.7.2 | Shared C live-data catalog, full-key parameter store and UDS-capable 64-item scheduler |
| 0.7.3 | ELM-managed ISO 15765 CAN channel, Common 1.8 timing reuse and cross-build recovery hardening |
| 0.7.4 | Common 1.10 authoritative CMake/Xcode target integration and release-state synchronization |
| 0.7.5 | iPhone C207/OM651 read-only engine ECU probe integration for physical vehicle testing |
| 0.7.6 | Restore visible iPhone author/copyright/About identity while retaining the 0.7.5 Mercedes probe test path |

Module contracts and limitations are documented in the corresponding files under `docs/`; this roadmap does not duplicate those specifications.

## 0.7 — UDS foundation in C (complete)

- request/response model;
- positive and negative responses;
- diagnostic-session handling;
- ECU identification and data-identifier abstraction;
- timing/state behaviour for verified services;
- complete reuse of ISO-TP PDUs without duplicated segmentation or CAN addressing.

**Exit condition met:** deterministic UDS exchanges execute through the reusable C engine without manufacturer interpretation or ISO-TP duplication in platform transport code.

## 0.7.1 — pre-Mercedes parameter and release hardening (complete)

- pin the released Infiltratr Common 1.7.0 dependency;
- add one protocol-neutral C scalar parameter identity/metadata/formatting model for OBD-II and future UDS live values;
- compile the Mercedes and parameter layers in the iPhone core and guard that wiring in CI;
- attach the unsigned iPhone IPA directly to each new GitHub Release;
- keep manufacturer-specific DIDs empty until real vehicle evidence exists.

**Exit condition:** all portable, sanitizer, Linux, iOS Debug/Release and unsigned-device build gates pass on one exact 0.7.1 commit, and that exact release publishes its IPA as a Release asset.

## 0.7.2 — shared live-data model hardening (complete)

- drive iPhone Live Data, Table, Dashboard and Graphs from C-owned parameter names, units, stable keys and Common-backed formatting instead of a second Swift parameter table;
- add a bounded 256-definition / 1024-sample protocol-neutral parameter store with full `{protocol,module,identifier}` keys, favourites, latest values and chronological history;
- expand the scheduler to 64 items and give each item the same full parameter key, allowing future UDS DIDs from different modules without colliding;
- retain explicit OBD-II scheduler wrappers and the proven OBD telemetry/CSV path while the generic store is integrated incrementally;
- remove obsolete per-PID measurement publication properties from the Objective-C bridge.

No Mercedes manufacturer DID or ECU address is promoted by this release. Those remain 0.8 vehicle-validation work.

**Exit condition:** C regression coverage includes the generic parameter catalog, store and keyed scheduler, and the generic iPhone presentation builds without the removed per-PID publication surface.

## 0.7.3 — ELM-managed CAN and recovery hardening (complete)

- retain the complete portable ELM-managed ISO 15765 CAN-channel implementation and its PDU encode/decode regression coverage;
- compile that implementation in both CMake and the iPhone portable core;
- adopt released Infiltratr Common 1.8.0 for generic periodic-deadline advancement without moving scheduler selection policy out of MBLINK;
- strengthen CI so dependency, source-ownership and ELM CAN coverage omissions fail before an iPhone or release build can be presented as healthy;
- align version metadata and documentation with the actual 64-item scheduler and the distinction between physical installation and live vehicle validation.

No ECU endpoint, Mercedes DID or successful Vgate/vehicle exchange is claimed by this maintenance release. Those remain evidence-driven 0.8 work.

**Exit condition:** all portable tests retain their ELM CAN PDU coverage, CMake and iPhone compile the same new C sources, and all portable, sanitizer, Linux, iOS and unsigned-device gates pass on one exact 0.7.3 commit.

## 0.7.4 — Common 1.10 build-package integration (complete)

- pin released Infiltratr Common 1.10.0 at exact commit `182e64cb8b8992879e443b941565058166fe0161`;
- consume Common's authoritative `InfiltratrCommon::Portable` CMake target instead of enumerating Common implementation files in MBLINK;
- link Common's authoritative `InfiltratrCommonPortable` Apple subproject product instead of duplicating Common implementation files in the MBLINK Xcode target;
- make CI reject regression to consumer-owned Common source lists while retaining exact version/gitlink verification;
- synchronize VERSION and all iPhone marketing-version metadata at 0.7.4 so the validated main commit can publish a matching GitHub Release and unsigned IPA.

The existing read-only Mercedes endpoint probe remains explicitly in-progress 0.8 work. This maintenance milestone does not claim live vehicle communication, a verified Mercedes endpoint or completion of the 0.8 exit condition.

**Exit condition:** the seven portable, sanitizer, Linux, iOS Debug/Release and unsigned-device gates pass on one exact 0.7.4 commit using Common-owned build targets, and that exact commit publishes the 0.7.4 release with its IPA.

## 0.7.5 — physical Mercedes probe test release

- keep the C207/OM651 endpoint at candidate provenance status rather than pretending it is vehicle-verified;
- invoke the portable Mercedes endpoint probe from the real iPhone connection workflow after the generic OBD-II capability exchange;
- configure the ELM-managed CAN channel, issue read-only UDS TesterPresent, decode the response and retain the raw command/response transcript;
- expose the selected candidate and probe result in the Vehicle and Modules workspaces so a physical test has visible evidence;
- reinitialise the ELM adapter after the Mercedes probe before resuming normal live OBD-II polling;
- publish an unsigned iPhone IPA specifically so the development vehicle can provide the first physical Mercedes response capture.

This release is a testable 0.8 slice, not completion of the 0.8 milestone. It does not add guessed Mercedes DIDs and does not promote the candidate endpoint without a reproducible physical capture.

**Exit condition:** all seven required gates pass for one exact 0.7.5 commit and the matching unsigned IPA is attached to the GitHub Release for physical vehicle testing.

## 0.7.6 — iPhone project identity correction

- retain the complete 0.7.5 Mercedes probe test path without changing endpoint provenance or adding guessed Mercedes DIDs;
- display `Copyright © 2026 Shannon Smith` directly in the iPhone interface;
- provide an About sheet containing program name, version, author, copyright, GPL-3.0-or-later licence and repository link;
- carry the same copyright into generated iOS application metadata;
- keep release identity synchronized across VERSION and all four Xcode marketing-version settings.

**Exit condition:** all seven required gates pass for one exact 0.7.6 commit and the matching unsigned IPA is attached to the GitHub Release.

## 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics

Validate useful engine data such as ECU identity, DPF state/pressure/temperature, turbo/boost, rail pressure, injector information, EGR and related diesel parameters.

In progress after 0.7.6:

- the C207/OM651 profile carries a conventional 11-bit physical engine-endpoint candidate with explicit source provenance and unverified status;
- a portable read-only ECU probe configures the ELM-managed CAN channel and validates a positive UDS TesterPresent response without entering a session or writing vehicle data;
- probe failures preserve whether configuration, ELM PDU decoding or UDS validation failed, including the underlying ELM result and UDS negative-response code;
- the iPhone connection workflow invokes that probe, displays its candidate/result and records the exchange before restoring standard OBD-II operation;
- no successful vehicle exchange or manufacturer DID is claimed until physical captures exist.

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
