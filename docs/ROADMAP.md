<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK grows from the portable C core outward. Every milestone must leave the repository buildable, tested and reusable.

**Completed through test release 0.7.10. Active feature milestone: 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics.**

## Completed foundations

| Milestone | Result |
| --- | --- |
| 0.1 | Portable C11 core, public C API/transport ABI, pinned Infiltratr Common, strict CI |
| 0.2 | ELM327 command/parser/init/session engine with deterministic mock tests |
| 0.3 | Standard OBD-II PID discovery/decoding, VIN, readiness, freeze-frame and DTCs |
| 0.4 | Objective-C CoreBluetooth provider and native iPhone shell |
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
| 0.7.5 | iPhone C207/OM651 read-only engine ECU TesterPresent probe integration |
| 0.7.6 | Visible iPhone author/copyright/About identity correction |
| 0.7.7 | Standardised About layout and physical test packaging |
| 0.7.8 | Exportable raw diagnostic evidence independent of live telemetry samples |
| 0.7.9 | Standard VIN plus bounded standardized ECU-identity evidence sweep after positive UDS endpoint response |
| 0.7.10 | Automatic OBD fault scan, chained PID discovery and standard diesel/DPF live diagnostics |
| 0.7.11 | Read-only CRD3/CDID3 engine-ECU fingerprint evidence for OM651 |

Module contracts and limitations are documented in the corresponding files under `docs/`; this roadmap does not duplicate those specifications.

## 0.7 — UDS foundation in C (complete)

- request/response model;
- positive and negative responses;
- diagnostic-session handling;
- ECU identification and data-identifier abstraction;
- timing/state behaviour for verified services;
- complete reuse of ISO-TP PDUs without duplicated segmentation or CAN addressing.

**Exit condition met:** deterministic UDS exchanges execute through the reusable C engine without manufacturer interpretation or ISO-TP duplication in platform transport code.

## 0.7.1–0.7.4 — reusable data model, CAN and Common integration (complete)

These releases established the protocol-neutral parameter model and keyed history store, the 64-item scheduler, ELM-managed ISO 15765 channel, strict source-ownership CI, and authoritative Common 1.10 CMake/Xcode targets. Common 1.10 remains pinned at exact commit `182e64cb8b8992879e443b941565058166fe0161`.

No Mercedes-specific live-data DID was introduced in these foundation releases.

## 0.7.5 — physical Mercedes endpoint probe (complete test release)

- keep the C207/OM651 endpoint at candidate provenance status rather than pretending it is vehicle-verified;
- invoke the portable Mercedes endpoint probe from the iPhone connection workflow after generic OBD-II capability discovery;
- configure the ELM-managed CAN channel and issue read-only positive-response UDS TesterPresent;
- retain the raw command/response transcript;
- expose the candidate and probe result in Vehicle and Modules;
- reset the ELM adapter before resuming normal OBD-II polling.

This was the first installable 0.8 validation slice. It did not add guessed Mercedes DIDs or promote the candidate without physical evidence.

## 0.7.6–0.7.7 — iPhone identity/About correction (complete)

- display `Copyright © 2026 Shannon Smith` in the iPhone interface and application metadata;
- provide program/version/author/licence/project identity in About;
- standardise the About layout with the application's established Credits / License / Close convention;
- retain the complete Mercedes endpoint test path.

## 0.7.8 — diagnostic evidence export (complete)

- make the session transcript exportable even when no live-data samples were recorded;
- identify the exported file as diagnostic evidence;
- expose a direct evidence path from Modules and Log;
- keep the raw ELM327 commands and responses needed for fixture promotion.

**Exit condition met:** the physical test build can return the complete endpoint probe evidence instead of only a UI status string.

## 0.7.9 — standardized ECU identity evidence sweep (complete test release)

After a positive read-only TesterPresent response, the portable Mercedes probe continues with standardized UDS identification requests before restoring generic OBD-II operation.

- request standardized VIN DID `F190`;
- validate and retain a 17-character VIN when returned;
- perform a bounded read-only identity sweep of `F18C`, `F187`, `F188`, `F189`, `F191` and `F197`;
- classify each identity request as positive, negative, no-response or invalid without treating optional identity failures as endpoint failure;
- keep every raw response in diagnostic evidence rather than guessing manufacturer-specific payload encoding;
- expose the expanded identity-sweep state in the iPhone Vehicle, Modules and Log workspaces;
- retain the evidence rule: these standardized requests do not create or verify Mercedes-specific live-data definitions by themselves.

**Exit condition met:** all seven required build gates completed and `v0.7.9` was published from the exact release commit with its unsigned iPhone IPA.

## 0.7.10 — faults and standard diesel diagnostics (test release)

This slice moved immediately useful feature areas forward without inventing undocumented OM651 DIDs.

- enumerate chained SAE OBD-II supported-PID blocks instead of stopping after the first `0100` response;
- automatically read stored, pending and permanent OBD-II trouble codes using read-only services `03`, `07` and `0A`;
- surface captured standard UDS VIN and standardized ECU-identity results;
- expand the shared C parameter catalog and scheduler with advertised standard diesel/aftertreatment values;
- add a dedicated Diesel workspace;
- keep injector corrections, soot load, regeneration state, ash load and other Mercedes-specific values explicitly unavailable until a reproducible OM651 capture identifies their real DIDs and encoding.

## 0.7.11 — CRD3/CDID3 engine fingerprint (current test slice)

This release puts development back on the manufacturer-specific 0.8 track without fabricating live-data formulas.

After the positive UDS endpoint test, VIN and six standardized identity reads, MBLINK now performs five additional read-only `ReadDataByIdentifier` probes associated with the open-source CaesarSuite CRD3 model: `F100`, `F154`, `F196`, `1001` and `1002`. Their positive, negative, silent and malformed outcomes are retained independently and also surfaced in the combined Mercedes evidence list.

These identifiers are treated as ECU-family/variant fingerprint evidence only. A response is not interpreted as soot load, regeneration state, injector correction, rail pressure, boost or EGR data. The purpose is to establish whether the physical C207/OM651 behaves like the CRD3/CDID3 family and preserve enough exact evidence to bind later proprietary definitions to the correct ECU/software variant.

**Exit condition:** all seven required build gates pass for one exact 0.7.11 commit and the matching unsigned iPhone IPA is published for physical C207/OM651 validation.

## 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics (active)

Validate useful engine data such as ECU identity, DPF state/pressure/temperature, turbo/boost, rail pressure, injector information, EGR and related diesel parameters.

Current state after 0.7.11:

- the C207/OM651 profile carries one conventional 11-bit physical engine-endpoint candidate with explicit source provenance and unverified status;
- the iPhone performs complete standard OBD capability discovery, read-only UDS TesterPresent, standard VIN/identity evidence collection and a bounded CRD3 fingerprint pass before restoring normal OBD-II;
- the CRD3 pass requests `F100`, `F154`, `F196`, `1001` and `1002` and records every raw response without assigning unsupported physical meanings;
- captured VIN and per-DID outcomes are visible and preserved in the evidence transcript;
- standard diesel/DPF values remain available where the vehicle advertises them;
- no successful physical Mercedes exchange is claimed until the development vehicle actually provides the capture;
- no manufacturer-specific soot-load, regeneration, injector-correction or other undocumented OM651 formula is claimed without vehicle evidence.

The next evidence step is physical: install 0.7.11, connect to the C207/OM651 and export the diagnostic evidence CSV. A reproducible CRD3/ECU fingerprint can then become a fixture and unlock the first genuine OM651-specific DPF/injector definitions from observed vehicle behaviour rather than guesses.

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
