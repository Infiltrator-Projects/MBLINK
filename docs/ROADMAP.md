<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK grows from the portable C core outward. Every milestone must leave the repository buildable, tested and reusable.

**Current source version: 0.7.26. Active feature milestone: 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics, with fault interpretation now treated as the first completion track.**

## Completion discipline

MBLINK development now follows one vertical slice to completion rather than accumulating unrelated half-finished features. The active slice is fault diagnosis:

```text
raw ECU fault
  → generic LINK interpretation
  → product/manufacturer interpretation
  → correct scan state
  → diagnostic context (freeze-frame/readiness)
  → user-facing investigation record
```

Additional gauges, visual polish or new module breadth do not replace completion of this path.

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
| 0.7.3 | ELM-managed ISO 15765 channel, Common 1.8 timing reuse and cross-build recovery hardening |
| 0.7.4 | Common 1.10 authoritative CMake/Xcode target integration and release-state synchronization |
| 0.7.5 | iPhone C207/OM651 read-only engine ECU TesterPresent probe integration |
| 0.7.6 | Visible iPhone author/copyright/About identity correction |
| 0.7.7 | Standardised About layout and physical test packaging |
| 0.7.8 | Exportable raw diagnostic evidence independent of live telemetry samples |
| 0.7.9 | Standard VIN plus bounded standardized ECU-identity evidence sweep after positive UDS endpoint response |
| 0.7.10 | Automatic OBD fault scan, chained PID discovery and standard diesel/DPF live diagnostics |
| 0.7.11 | Read-only CRD3/CDID3 engine-ECU fingerprint evidence for OM651 |
| 0.7.12 | Source-corroborated CRD3 target, read-only Mercedes UDS faults and full black/silver Mercedes-oriented interface overhaul |
| 0.7.13+ | Official emblem/interface consolidation, LINK family consolidation and main-only release policy hardening |
| current fault slice | LINK 0.10.0 generic DTC knowledge, MBLINK structured generic fault records and translated existing Faults presentation |

Module contracts and limitations are documented in the corresponding files under `docs/`; this roadmap does not duplicate those specifications.

## Fault-diagnosis completion track (active)

The previous implementation could retrieve standard OBD and Mercedes UDS fault records but was much stronger at acquisition than interpretation. `docs/FAULT_DIAGNOSTICS.md` is the normative specification.

### Implemented now

- LINK 0.10.0 owns a shared, presentation-neutral DTC knowledge API.
- Valid SAE-style codes are normalized and classified by system and generic/manufacturer origin.
- Known generic definitions carry title, category and source class without replacing the raw code.
- The first catalogue targets the high-value engine/diesel areas MBLINK and JAGLINK both need: fuel/rail/injection, boost, EGR, misfire, glow-plug, DPF/EGT/NOx and common network faults.
- Structured per-cylinder families are generated in LINK instead of copied into product UIs.
- ISO 14229 DTC status meaning is shared in LINK.
- Unknown manufacturer-specific codes remain explicit unknowns rather than guessed descriptions.
- MBLINK pins LINK 0.10.0 and exposes the knowledge through its compatibility OBD API.
- The iPhone model carries structured `DiagnosticFault` records and feeds translated `CODE — description` text to the existing Faults screen.
- LINK and MBLINK both contain regression tests for the shared knowledge path.

### Must be finished before this slice is considered complete

1. Refactor the Faults UI to consume structured records directly and distinguish **not scanned**, **scan failed**, **successful clean scan** and **faults present**. An empty array alone is never proof of a clean scan.
2. Integrate the existing OBD readiness decoder into the shared diagnostic flow and user-facing investigation data.
3. Integrate capability-driven Mode 02 freeze-frame context into the fault flow, clearly separated from current live data.
4. Build an evidence-backed Mercedes/CRD3/OM651 DTC knowledge catalogue in MBLINK, with module/applicability/provenance and no fabricated meanings.
5. Expand LINK's generic catalogue systematically after the first high-value set.
6. Ensure other LINK-family product faces, especially JAGLINK, inherit generic knowledge by updating their LINK pin rather than copying the implementation.

## 0.7 — UDS foundation in C (complete)

- request/response model;
- positive and negative responses;
- diagnostic-session handling;
- ECU identification and data-identifier abstraction;
- timing/state behaviour for verified services;
- complete reuse of ISO-TP PDUs without duplicated segmentation or CAN addressing.

**Exit condition met:** deterministic UDS exchanges execute through the reusable C engine without manufacturer interpretation or ISO-TP duplication in platform transport code.

## 0.7.1–0.7.4 — reusable data model, CAN and Common integration (complete)

These releases established the protocol-neutral parameter model and keyed history store, the 64-item scheduler, ELM-managed ISO 15765 channel, strict source-ownership CI, and authoritative Common integration.

No Mercedes-specific live-data DID was introduced in these foundation releases.

## 0.7.5 — physical Mercedes endpoint probe (complete test release)

- keep the C207/OM651 endpoint evidence-gated rather than pretending it is vehicle-verified;
- invoke the portable Mercedes endpoint probe from the iPhone connection workflow after generic OBD-II capability discovery;
- configure the ELM-managed CAN channel and issue read-only positive-response UDS TesterPresent;
- retain the raw command/response transcript;
- expose the endpoint and probe result in Vehicle and Modules;
- reset the ELM adapter before resuming normal OBD-II polling.

This was the first installable 0.8 validation slice. It did not add guessed Mercedes DIDs or promote the endpoint without physical evidence.

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

## 0.7.10 — faults and standard diesel diagnostics

This slice moved immediately useful feature areas forward without inventing undocumented OM651 DIDs.

- enumerate chained SAE OBD-II supported-PID blocks instead of stopping after the first `0100` response;
- automatically read stored, pending and permanent OBD-II trouble codes using read-only services `03`, `07` and `0A`;
- surface captured standard UDS VIN and standardized ECU-identity results;
- expand the shared C parameter catalog and scheduler with advertised standard diesel/aftertreatment values;
- add a dedicated Diesel workspace;
- keep injector corrections, soot load, regeneration state, ash load and other Mercedes-specific values explicitly unavailable until a reproducible OM651 capture identifies their real DIDs and encoding.

Retrieval alone is no longer treated as completion of fault support; the active fault-diagnosis completion track above now owns interpretation/context/UI completion.

## 0.7.11 — CRD3/CDID3 engine fingerprint (complete test slice)

After the positive UDS endpoint test, VIN and six standardized identity reads, MBLINK performs five additional read-only `ReadDataByIdentifier` probes associated with the open-source CaesarSuite `Simulated_CRD3` model: `F100`, `F154`, `F196`, `1001` and `1002`. Their positive, negative, silent and malformed outcomes are retained independently and also surfaced in the combined Mercedes evidence list.

These identifiers are treated as ECU-family/variant fingerprint evidence only. A response is not interpreted as soot load, regeneration state, injector correction, rail pressure, boost or EGR data. The purpose is to establish whether the physical C207/OM651 behaves like the CRD3/CDID3 family and preserve enough exact evidence to bind later proprietary definitions to the correct ECU/software variant.

## 0.7.12 — Mercedes command interface and deeper CRD3 evidence (completed)

Diagnostic work:

- decode the published CRD3 `F100` session/variant structure and `F154` supplier identifier without assigning unsupported live-data meanings;
- corroborate the OM651/CDID3/Delphi family signature from independent evidence;
- promote the C207/W207 E 250 CDI / OM651 / Delphi CRD3.x `0x7E0 → 0x7E8` engine endpoint from candidate to **source-corroborated**, while deliberately keeping vehicle-verified status gated on the development car;
- add a portable read-only UDS `ReadDTCInformation` codec and execute one Mercedes engine `19 02 FF` fault-memory read before adapter restore;
- expose Mercedes UDS faults, CRD3 identity and evidence status directly to the iPhone model and diagnostics screens;
- retain the complete raw transcript for physical fixture creation.

Interface work established the black/silver Mercedes diagnostic command centre, official emblem usage and common Vehicle, Modules, Faults, Diesel, Live Data, Data Table, Dashboard, Graphs, Evidence, Settings and About workspaces.

## 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics (active)

The active work is split into two completion tracks rather than one open-ended feature bucket.

### A. Fault diagnosis — first priority

Finish the fault-diagnosis completion track above. A connected diagnostic product should answer not merely “what bytes/code came back?” but, where evidence permits, “what fault is it, where is it, what state is it in, and what context was recorded?”

### B. Mercedes live-data mapping — after/alongside evidence collection

Validate useful engine data such as ECU identity, DPF state/pressure/temperature, turbo/boost, rail pressure, injector information, EGR and related diesel parameters.

Current manufacturer-specific state:

- the C207/OM651 profile carries one source-corroborated conventional 11-bit physical engine endpoint at `0x7E0 → 0x7E8` and still requires the development vehicle for vehicle-verified promotion;
- the iPhone performs complete standard OBD capability discovery, read-only UDS TesterPresent, standard VIN/identity evidence collection, a bounded CRD3 fingerprint pass and one read-only Mercedes UDS fault-memory request before restoring normal OBD-II;
- the CRD3 pass requests `F100`, `F154`, `F196`, `1001` and `1002`, decodes only corroborated identity fields and records every raw response without assigning unsupported physical meanings;
- captured VIN, CRD3 identity, per-DID outcomes and Mercedes UDS faults are visible and preserved in the evidence transcript;
- standard diesel/DPF values remain available where the vehicle advertises them;
- no successful physical Mercedes exchange is claimed until the development vehicle actually provides the capture;
- no manufacturer-specific soot-load, regeneration, injector-correction or other undocumented OM651 formula is claimed without vehicle evidence.

The next evidence step remains physical: install the current test IPA, connect to the C207/OM651 and export diagnostic evidence. A reproducible CRD3/ECU fingerprint can then become a fixture and unlock genuine OM651-specific DPF/injector definitions from observed vehicle behaviour rather than guesses.

Every undocumented Mercedes definition remains experimental until verified against real vehicle responses and regression fixtures.

**0.8 exit condition:** verified Mercedes information materially exceeds generic OBD-II capability on the development vehicle **and** the fault workflow satisfies `FAULT_DIAGNOSTICS.md` rather than stopping at raw codes.

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

Each feature begins at the lowest reusable layer that can correctly own it. UI convenience is never a reason to duplicate protocol logic outside the C core. A feature is not considered done while its documented completion criteria are still knowingly unmet.
