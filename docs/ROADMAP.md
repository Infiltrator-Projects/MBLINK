<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK grows from the portable C core outward. Every milestone must leave the repository buildable, tested and reusable. The exact current source version is `VERSION`; the exact shared-engine revision is the `src/link` gitlink.

The active product milestone remains Mercedes-Benz C207 / OM651 engine diagnostics, with fault interpretation treated as the first completion track.

## Completion discipline

MBLINK development follows one vertical slice to completion rather than accumulating unrelated half-finished features. The active slice is fault diagnosis:

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
| 0.6.1–0.6.3 | Lifetime/overflow hardening, sanitizer CI, IPA path and trusted smoke validation |
| 0.7 | Portable UDS response/session/timing engine and caller-supplied DID definitions |
| 0.7.1–0.7.4 | Common integration, parameter/history model, scheduler, ELM ISO 15765 channel and release hardening |
| 0.7.5–0.7.9 | Read-only Mercedes endpoint probing, standardized identity sweep, About/branding and evidence export |
| 0.7.10–0.7.12 | OBD fault inventory, diesel diagnostics, CRD3 fingerprinting, source-corroborated target and Mercedes UDS fault evidence |
| 0.7.13+ | Official emblem/interface consolidation, LINK family consolidation and main-only release policy hardening |
| current shared baseline | Exact pinned LINK release with generic DTC knowledge, CAN-FD/extended ISO-TP and complete 27-service UDS codecs |

Module contracts and limitations are documented in the corresponding files under `docs/`; this roadmap does not duplicate dependency version numbers that are already authoritative in the gitlinks.

## Fault-diagnosis completion track

Implemented:

- generic DTC knowledge is owned by LINK and reaches MBLINK as structured records;
- valid SAE-style codes are normalized and classified by system and generic/manufacturer origin;
- known generic definitions carry title, category and source class without replacing the raw code;
- ISO 14229 DTC status meaning is shared in LINK;
- unknown manufacturer-specific codes remain explicit unknowns;
- MBLINK exposes the knowledge through compatibility APIs and the iPhone model;
- standard OBD stored/pending/permanent inventory and Mercedes UDS fault evidence are captured;
- product facades expose LINK's CAN-FD and complete generic UDS service contracts without duplicating the implementation.

Must still be finished before this product slice is complete:

1. Refactor the Faults UI so **not scanned**, **scan failed**, **successful clean scan** and **faults present** are distinct states and rich cards consume structured records directly.
2. Integrate OBD readiness into the shared diagnostic flow and investigation record.
3. Integrate capability-driven Mode 02 freeze-frame context and clearly separate it from current live data.
4. Build an evidence-backed Mercedes/CRD3/OM651 DTC knowledge catalogue in MBLINK with applicability and provenance; do not fabricate proprietary meanings.
5. Continue evidence-led generic catalogue maintenance in LINK rather than copying definitions into MBLINK.

## Shared-engine integration rule

New generic automotive functionality is implemented in LINK first, released there, and then consumed by advancing MBLINK's single LINK gitlink. MBLINK does not pin Common independently. CMake and CI verify the checked-out dependency tree from the committed gitlinks instead of keeping duplicate commit/version constants.

The native iPhone target must compile the same LINK implementation as CMake. Product bridge files therefore include exact pinned LINK source where required, and CI explicitly verifies the shared UDS service implementation is present. Platform code must not grow alternate protocol engines.

## Mercedes-Benz C207 / OM651 diagnostics

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

## Later milestones

### Additional Mercedes modules

Subject to real vehicle/network access: transmission, ABS/ESP, SRS, climate, instrument cluster and other discoverable ECUs. Start with identification, DTCs and selected reads before write-oriented functions.

### Adapter portability

Formalise adapter capabilities/profiles, verified firmware quirks and additional ELM327-compatible transports. The Vgate reference adapter remains a validation target, not a `libmblink` dependency.

### 1.0 release hardening

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
