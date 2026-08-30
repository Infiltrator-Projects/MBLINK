<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK grows from the portable C core outward. Every milestone must leave the repository buildable, tested and reusable. The exact current source version is `VERSION`; the exact shared-engine revision is the `src/link` gitlink.

The active product milestone remains Mercedes-Benz C207 / OM651 diagnostics, with fault interpretation treated as the first completion track.

MBLINK is one manufacturer product family containing both the normal MBLINK diagnostic application and the specialist MBLINK Discover application. Discover is not a separate repository or future `MBLINK-Reader`; it is the existing branded ECU/module discovery and read-only evidence/dump target and should evolve in place.

## Completion discipline

MBLINK development follows one vertical slice to completion rather than accumulating unrelated half-finished features. The active main-application slice is fault diagnosis:

```text
raw ECU fault
  -> generic LINK interpretation
  -> product/manufacturer interpretation
  -> correct scan state
  -> diagnostic context (freeze-frame/readiness)
  -> user-facing investigation record
```

Additional gauges, visual polish or new module breadth do not replace completion of this path.

Discover follows a separate but complementary specialist path:

```text
passive network observation
  -> standards-based inventory
  -> Mercedes-aware module discovery
  -> ECU/module identification
  -> documented read-only information acquisition
  -> structured raw/evidence dump
```

Generic Discover mechanics belong in LINK. Mercedes topology, identities, probes and decoders belong in MBLINK.

## Completed foundations

| Milestone | Result |
| --- | --- |
| 0.1 | Portable C11 core, public C API/transport ABI, pinned Infiltratr Common, strict CI |
| 0.2 | ELM327 command/parser/init/session engine with deterministic mock tests |
| 0.3 | Standard OBD-II PID discovery/decoding, VIN, readiness, freeze-frame and DTC primitives |
| 0.4 | Objective-C CoreBluetooth provider and native iPhone shell |
| 0.5 | C-owned scheduler, telemetry/history, full-session recording and SwiftUI dashboard/export |
| 0.6 | Reusable Classical-CAN ISO-TP TX/RX state machines independent of UDS/Mercedes |
| 0.6.1–0.6.3 | Lifetime/overflow hardening, sanitizer CI, IPA path and trusted smoke validation |
| 0.7 | Portable UDS response/session/timing engine and caller-supplied DID definitions |
| 0.7.1–0.7.4 | Common integration, parameter/history model, scheduler, ELM ISO 15765 channel and release hardening |
| 0.7.5–0.7.9 | Read-only Mercedes endpoint probing, standardized identity sweep, About/branding and evidence export |
| 0.7.10–0.7.12 | OBD fault inventory, diesel diagnostics, CRD3 fingerprinting, source-corroborated target and Mercedes UDS fault evidence |
| later 0.7.x | VIN-keyed module profiles, cached route validation, wider read-only Mercedes module census, offline replay fixtures and asynchronous evidence export |
| current shared baseline | Exact pinned LINK release with generic DTC knowledge, CAN-FD/extended ISO-TP, complete 27-service UDS codecs, shared CoreBluetooth engine, native OpenPort and shared Discover engine |

Module contracts and limitations are documented in the corresponding files under `docs/`; this roadmap does not duplicate dependency version numbers that are already authoritative in the gitlinks.

## Current field-hardening baseline

Several problems seen during physical C207/Vgate testing are now represented by permanent architectural fixes rather than operator workarounds:

- LINK's Apple Bluetooth transport remembers the last successfully probed CoreBluetooth peripheral and reuses it before falling back to a scan;
- duplicate BLE advertisements remain enabled during discovery because dual-mode Vgate adapters can expose their usable local name only on a later advertisement;
- scan/recovery attempts are bounded rather than requiring a manual disconnect/reconnect cycle;
- CSV preparation snapshots recorder bytes and performs file I/O away from the Bluetooth/diagnostic execution path, so evidence export must not intentionally stop polling;
- one controller lifetime retains distinct closed connection attempts in the evidence stream rather than erasing the first failed attempt when a later attempt succeeds;
- live polling is explicit opt-in and persisted by stable parameter key, preventing the ELM/BLE link from being saturated by every supported PID by default.

These fixes have regression/CI coverage where they can be simulated. Physical adapter behaviour remains field validation rather than something CI can manufacture.

## Fault-diagnosis completion track

Implemented:

- generic DTC knowledge is owned by LINK and reaches MBLINK as structured records;
- valid SAE-style codes are normalized and classified by system and generic/manufacturer origin;
- known generic definitions carry title, category and source class without replacing the raw code;
- ISO 14229 DTC status meaning is shared in LINK;
- unknown manufacturer-specific codes remain explicit unknowns;
- MBLINK exposes the knowledge through compatibility APIs and the iPhone model;
- standard OBD stored/pending/permanent inventory and Mercedes UDS fault evidence are captured;
- scan state is preserved separately in the controller instead of deriving success from an empty list;
- product facades expose LINK's CAN-FD and complete generic UDS service contracts without duplicating the implementation.

Current completion state:

1. **Implemented:** the iPhone and Linux Faults presentations distinguish not-scanned/in-progress, failed/incomplete, verified-clean and faults-present states. Standard OBD rows consume structured LINK diagnostic knowledge and preserve raw codes; unknown definitions remain explicit.
2. **Implemented:** standards-defined OBD readiness is now part of LINK's standard fault-investigation flow, retained in the investigation model and shown by MBLINK rather than remaining a dormant decoder.
3. **Implemented:** capability-gated Mode 02 frame-zero freeze-frame acquisition follows stored SAE fault inventory. Captured values are retained as diagnostic context and are explicitly separated from current Live Data. Unsupported/missing values remain unavailable rather than being inferred.
4. **Implemented baseline, expansion remains:** MBLINK's module-scoped Mercedes KWP table carries definition status and provenance; the vehicle-captured ORC `9B51` record resolves through that table while preserving raw status. Additional CRD3/OM651/module definitions still require defensible documentation or reproducible physical evidence before promotion.
5. Generic catalogue maintenance continues in LINK rather than being copied into MBLINK.

The remaining work in this slice is therefore evidence-backed expansion of Mercedes/CRD3/OM651 manufacturer knowledge and further physical module fixtures, not the standard scan-state/readiness/freeze-frame plumbing.

## MBLINK Discover and module-discovery completion track

Implemented baseline:

- shared LINK Windows OpenPort/J2534 shell;
- passive 500 kbit/s CAN capture;
- bounded read-only standard OBD inventory;
- deny-by-default request classification;
- structured evidence export and operator annotations;
- MBLINK branding and Mercedes product identity;
- C207/OM651 conventional engine route represented at `0x7E0 -> 0x7E8` and vehicle-verified on the development car;
- portable Mercedes engine fingerprint scan followed by a bounded module scan;
- mobile first-VIN census capable of learning responding routes, including wider 11/29-bit read-only discovery;
- VIN-keyed module profiles persisted on iPhone and validated on later connections instead of repeating full discovery every time;
- invalid or changed saved profiles are discarded and rebuilt rather than silently trusted;
- Linux normal census plus explicit `DEEP RESCAN` path;
- captured-trace and offline C207 replay regression coverage, including a replayed ORC seatbelt/airbag fault becoming visible through the shared product path.
- structured iPhone module cards and detail screens, with live Mode 01 values attributed to their exact response CAN identifier instead of being collapsed into one generic OBD value.

Next work:

1. Continue moving generic multi-network module-discovery/result behaviour into LINK while keeping Mercedes topology in MBLINK.
2. Expand evidence-backed module identities and read-only requests beyond the routes already observed or corroborated.
3. Turn each additional physical response into a sanitised regression fixture before promoting it to vehicle-verified.
4. Add bounded identity/DID acquisition for documented or reproducibly verified Mercedes modules.
5. Continue enriching structured dumps with raw requests/responses, module identity, network path, result status, timestamps and product/profile provenance.
6. Keep reset, security access, routines, DTC clearing, coding, programming and firmware-write operations outside the Discover allowlist unless a separately reviewed product capability explicitly requires them.

Discover remains part of this repository. A separate MBLINK Reader repository would duplicate the existing product boundary and is not part of the roadmap.

## Shared-engine integration rule

New generic automotive functionality is implemented in LINK first, released there, and then consumed by advancing MBLINK's single LINK gitlink. MBLINK does not pin Common independently. CMake and CI verify the checked-out dependency tree from the committed gitlinks instead of keeping duplicate commit/version constants.

The same rule applies to Discover. Generic module discovery, interrogation, safety, evidence and platform-shell behaviour belongs in LINK; Mercedes-specific definitions, supported read-only requests and decoders belong here.

The native iPhone target must compile the same LINK implementation as CMake. Product bridge files therefore include exact pinned LINK source where required, and CI explicitly verifies the shared UDS service implementation is present. Platform code must not grow alternate protocol engines.

## Mercedes-Benz C207 / OM651 diagnostics

Current manufacturer-specific state:

- the C207/OM651 profile carries one source-corroborated conventional 11-bit physical engine endpoint at `0x7E0 -> 0x7E8`; a 2026-08-26 capture verified that route on one development C207 without generalising it to every family member;
- the iPhone performs complete standard OBD capability discovery, read-only UDS TesterPresent, standard VIN/identity evidence collection, a bounded CRD3 fingerprint pass and read-only Mercedes fault/module work before restoring normal OBD-II;
- the CRD3 pass requests `F100`, `F154`, `F196`, `1001` and `1002`, decodes only corroborated identity fields and records every raw response without assigning unsupported physical meanings;
- captured VIN, CRD3 identity, per-DID outcomes, responding module routes and Mercedes fault records are visible and preserved in the evidence transcript;
- a new VIN can perform the wider read-only mobile census once, save the learned module topology against that VIN and use a bounded cached validation path on future connections;
- VIN profiles retain responder-specific Mode 01 capabilities, so the Modules workspace can keep separate engine (`0x7E8`) and transmission-candidate (`0x7E9`) live-data pages even when a later manufacturer probe is quiet;
- the 2026-08-30 C207 capture verifies the KWP routes `0x64A -> 0x489` (ORC, one raw DTC record) and `0x652 -> 0x48A` (head unit, valid clean inventory); the module-scoped Mercedes fault table resolves ORC `9B51` to the source-corroborated driver seat-belt buckle circuit description while retaining raw status `E0`; both exact response shapes are regression fixtures, while the truncated ESP DTC response remains explicitly incomplete;
- standard diesel/DPF values remain available where the vehicle advertises them;
- physical C207/Vgate captures have verified standard VIN, selected Mercedes UDS response shapes and the conventional engine route, while unobserved definitions remain explicitly unverified;
- captured live data distinguishes accelerator-pedal channels from absolute throttle-valve position rather than treating PID `0x11` as pedal demand;
- the development C207 advertises SAE PID `0x2F` fuel level but not SAE PID `0x5E` engine fuel rate, so MBLINK exposes measured tank percentage while refusing to invent an SAE fuel-flow value;
- no manufacturer-specific soot-load, regeneration, injector-correction, fuel-consumption or other undocumented OM651 formula is claimed without protocol and vehicle evidence.

The DID Lab remains the promotion boundary for factory values. `corroborated-unmapped`, `source-backed-candidate` and `vehicle-verified` are separate states; correlation can support a candidate but cannot invent its request address or promote it by itself.

The next evidence step is to turn each additional physical C207/OM651 result into a sanitised regression fixture. A reproducible CRD3/ECU/module fingerprint can then unlock genuine OM651-specific DPF, injector, fuel and transmission definitions from observed vehicle behaviour rather than guesses.

Every undocumented Mercedes definition remains experimental until verified against real vehicle responses and regression fixtures.

## Later milestones

### Additional Mercedes modules

Continue transmission, ABS/ESP, SRS, climate, instrument-cluster and other module work from the existing read-only census/profile framework. Promote a module or request only when documentation or reproducible physical evidence supports it; begin with identification/evidence and expose normal diagnostics only after the definition is trustworthy.

### Adapter portability

The shared LINK adapter model already covers ELM/Vgate, native Tactrix OpenPort 2.0, STM32 CAN/FDCAN and the independently reconstructed Mercedes me native command layer. Remaining work is physical interoperability/quirk validation and, for the Mercedes me Adapter, completion of the authentication/session-establishment ordering rather than duplicating vehicle definitions per adapter.

### 1.0 release hardening

- stable documented ABI for supported functionality;
- provenance for verified fixtures/definitions;
- VIN-keyed iPhone vehicle/module profiles are implemented; broaden session persistence only where it remains useful;
- accessibility, performance and battery review;
- durable long-session storage across application termination;
- reproducible release process;
- security/privacy review before telemetry can leave the device.

Later work may include service functions, adaptations, additional manufacturers and non-iOS front ends. Firmware flashing and immobiliser/security programming remain outside the early roadmap.

## Development principle

Each feature begins at the lowest reusable layer that can correctly own it. UI convenience is never a reason to duplicate protocol logic outside the C core. A feature is not considered done while its documented completion criteria are still knowingly unmet.
