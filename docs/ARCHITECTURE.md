<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLink Architecture

MBLink is a C-first Mercedes-Benz-wide product face over Link. Vehicle, engine and ECU-family profiles are evidence-scoped subsets inside that manufacturer layer; no single development vehicle defines the product boundary.

```text
Libraries
        ↓
       Link
        ↓
      MBLink
```

The MBLink repository owns one Mercedes product family with multiple application targets. The normal MBLink application and MBLink Discover are siblings over the same manufacturer layer and exact pinned Link engine.

```text
                 Link::Core
                    |
          +---------+---------+
          |                   |
      MBLink app        MBLink Discover
   normal diagnostics    ECU/module reader
          \                   /
           +-- Mercedes layer-+
```

Discover is not a separate repository and is not a proposed `MBLink-Reader` replacement. It is already the specialist application target intended to grow into deep Mercedes ECU/module discovery, identification, read-only acquisition and evidence/dump export.

## Ownership

Libraries owns portable primitives useful across unrelated programs.

Link owns product-neutral vehicle-diagnostics and application behaviour shared across the complete Link family. That includes the operator-task workspace and information architecture, common presentation behaviour, Classical CAN and CAN-FD ISO-TP, transport contracts, ELM327 framing/parser/session behaviour, standard OBD-II, generic DTC knowledge, generic UDS and the complete standard service codec catalogue, parameter definitions/store/history, scheduling, telemetry/CSV, portable diagnostic sequencing, diagnostic-generation classification, Discover safety/evidence, generic ECU/module interrogation machinery, shared Linux/Apple/Windows application infrastructure and the shared Windows OpenPort/J2534 scanner.

MBLink owns Mercedes-wide identity/catalogue logic, vehicle/engine/ECU-family profiles (including the current C207/OM651/CRD3 evidence profile), Mercedes endpoint/definition provenance, Mercedes DTC knowledge, Mercedes module topology, branding, manufacturer-specific presentation content and genuinely Mercedes-specific diagnostic behaviour. Product-prefixed files that delegate to Link are compatibility adaptors, not independent implementations.

The recovered Mercedes me application semantic layer is also MBLink-owned. Its DataIds catalogue, DiagLogic value/status schema and reference policies, and Whisper configuration vocabulary describe Mercedes application knowledge rather than a product-neutral adapter transport. Link retains only the reusable adapter connection, byte-stream/native command framing and transport-neutral diagnostic bridge.

The ownership rule applies to diagnostic knowledge as well as protocol code. Standards-defined generic DTC descriptions/classification, OBD freeze-frame/readiness semantics and generic UDS DTC status interpretation belong in Link. Mercedes-Benz/CRD3/OM651-specific DTC definitions, module associations and manufacturer diagnostic metadata belong in MBLink. The identity-first Mercedes module scanner follows the same boundary: its public headers expose types and declarations, while the state-machine implementation is compiled once from `src/mercedes/module_scan.c` rather than duplicated as header-only inline code. See `FAULT_DIAGNOSTICS.md` for the normative product requirement.

The same rule governs Discover. Generic scanning, transport, identification state, safety, evidence and dump formatting belong in Link. Mercedes-specific module identities, known endpoints, evidence-backed read-only requests and decoders belong in MBLink.

The same rule also governs the normal application interface. Protocols such as OBD-II, OBDonUDS, UDS and Mercedes legacy diagnostics are data sources beneath Link's shared operator-task model; they are not competing primary navigation destinations. MBLink may add Mercedes-specific parameters, module scans, tests, services and evidence-backed workflows without forking the shared task structure.

The normal iPhone manufacturer path is controller-first. After the authoritative VIN and fitted-module map exist, each identified controller resolves one ECU pack containing two acquisition sections: startup-once static/module-card facts and user-polling live data. The central controller orchestrates those packs; it must not contain a parallel engine-specific startup probe or vehicle-specific PID policy. Startup-once facts are automatic and never appear as selectable PIDs. Recurring manufacturer reads begin only from the user's per-VIN/per-controller selections.

Link supplies shared capabilities such as language selection, measurement conversion and the standard About surface. MBLink owns the composition and appearance of its Settings page and any genuinely Mercedes-specific settings; Link does not own a complete Settings screen.

The exact Link revision is the `src/link` gitlink. Link owns its nested exact Common revision. MBLink's CMake and CI validate that recursive dependency graph rather than maintaining duplicate expected-version constants.

## Application targets

The repository boundary is intentionally broader than one executable:

```text
MBLink repository
  |- main diagnostic application
  `- MBLink Discover
```

A second executable does not justify a second repository when it shares the same manufacturer knowledge, release ownership, branding and Link dependency.

The main application is the user-facing diagnostic product. Discover is the engineering-oriented reader/dumper that explores vehicle networks and modules more deeply while remaining read-oriented and evidence-preserving.

The current Windows Discover target provides passive CAN capture, bounded OBD inventory and the explicit Mercedes FULL SWEEP built on Link's shared deep read-only discovery-plan interface. Further manufacturer-aware research extends this target rather than introducing a parallel `MBLink Reader` codebase.

## Platform boundaries

```text
SwiftUI / Objective-C                  GTK4 / C
          \                              /
           +---- MBLink product face ----+
                        |
                    Link::Core
                        |
               Libraries
                        |
             transport/provider edge
                        |
                adapter -> vehicle
```

Objective-C owns Apple framework integration such as CoreBluetooth lifecycle and write/notification mechanics. SwiftUI remains the Apple presentation edge, but common operator-task information architecture, shared presentation contracts and reusable diagnostic controller behaviour belong in Link. MBLink supplies Mercedes identity, theme, wording and manufacturer-specific content rather than carrying an alternate ELM327/OBD/ISO-TP/UDS implementation or a separate fault-code lookup database.

The Linux shell is C/GTK4 and renders the same portable workspace and Mercedes profile. Link owns common shell geometry and shared application behaviour; MBLink supplies Mercedes theme, branding and manufacturer-specific page content. Linux BLE/serial and native OpenPort 2.0 providers already live at Link's transport edge; a future SocketCAN provider belongs at that same edge rather than in a protocol or manufacturer layer.

Windows Discover is a branded target over Link's shared scanner shell. If Discover later gains Linux or Apple specialist shells, they should follow the same rule: presentation/provider code at the platform edge, manufacturer knowledge in MBLink, generic scanner behaviour in Link.

Platform UIs consume resolved diagnostic records from the portable/shared and manufacturer layers. SwiftUI, GTK and Win32 must not independently translate the same DTC or module identity into conflicting meanings.

The iPhone model carries structured generic `DiagnosticFault` values resolved through Link. The existing Faults screen receives compatibility `CODE — description` strings from those records while the richer card UI is completed. This compatibility surface is temporary presentation glue; the structured model remains authoritative.

## Native iPhone dependency detail

The Xcode project references Libraries through Link's nested checkout (`src/link/src/infiltratr-common`). This is a build-system detail, not a second top-level product dependency: `.gitmodules` contains only Link, and Link owns the Common pin.

The iOS target compiles product compatibility bridge files which include the exact shared Link implementation from the pinned submodule. In particular, the UDS bridge includes both Link's core UDS implementation and its compiled service-codec implementation. CMake consumers link `Link::Core` directly. CI explicitly checks this Apple bridge so a new shared Link implementation cannot disappear from iOS unnoticed.

## Timing, ownership and failure

Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery. Mercedes connection-owned diagnostic-session mode/stage/index state and session-command policy live in the portable Mercedes connection lifecycle; the Apple controller performs platform I/O around that state rather than owning another private session state machine.

The iPhone presentation edge consumes typed diagnostic state. Connection/fault behaviour is derived from Link diagnostic-flow state plus the explicit Mercedes fault-scan state, never by searching human-readable status strings. Manufacturer PID persistence, generic UDS catalogue projection and DTC presentation enrichment are separate domain services rather than responsibilities of `ConnectionViewModel`.

Manufacturer definitions carry evidence/provenance status. Undocumented identifiers, DTC meanings or formulas are not promoted to factual vehicle definitions without source evidence or reproducible capture fixtures.

A failed or unperformed fault scan must remain distinguishable from a successful clean scan all the way through the portable model to the user interface. The Apple controller preserves this distinction with explicit waiting/error strings and a successful `Complete · …` result; the Faults presentation must consume that state rather than infer success from empty arrays.

Discover must preserve the same evidence discipline. A module not responding, a request being blocked, an unsupported identifier and a successful empty/clean result are distinct outcomes and must not be collapsed into one state.

## Diagnostic product rule

Acquiring a raw DTC is only the first stage of fault diagnosis. Where trustworthy knowledge exists, the portable model must carry the translated description, classification, module/system information, state and available diagnostic context while preserving the raw code/status for evidence. Live dashboards are supporting diagnostic views and do not substitute for this path.

The pinned Link layer is the single source of truth for generic DTC interpretation, CAN-FD/ISO-TP and generic UDS services. Manufacturer-specific expansion is Mercedes-wide; CRD3/OM651 is the current evidence-rich catalogue slice, not the boundary of the Mercedes layer.

For Discover, a "dump" means structured read-only acquisition of available diagnostic information and raw responses. It does not grant reset, security access, coding, programming or firmware-write authority.

The complete fault requirement is defined in `FAULT_DIAGNOSTICS.md`; the Discover boundary is defined in `DISCOVER.md` and Link's `docs/DISCOVER.md` / `docs/PRODUCT_FACES.md`.

## Documentation standard

Public C APIs describe contracts, ownership, lifetime, failure behaviour and important invariants. Source comments explain rationale, state-machine subtleties and non-obvious constraints. Comments do not merely translate straightforward code into English.
