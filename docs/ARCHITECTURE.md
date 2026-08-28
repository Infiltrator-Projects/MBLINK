<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Architecture

MBLINK is a C-first Mercedes product face over LINK.

```text
Infiltratr Common
        ↓
       LINK
        ↓
      MBLINK
```

The MBLINK repository owns one Mercedes product family with multiple application targets. The normal MBLINK application and MBLINK Discover are siblings over the same manufacturer layer and exact pinned LINK engine.

```text
                 LINK::Core
                    |
          +---------+---------+
          |                   |
      MBLINK app        MBLINK Discover
   normal diagnostics    ECU/module reader
          \                   /
           +-- Mercedes layer-+
```

Discover is not a separate repository and is not a proposed `MBLINK-Reader` replacement. It is already the specialist application target intended to grow into deep Mercedes ECU/module discovery, identification, read-only acquisition and evidence/dump export.

## Ownership

Infiltratr Common owns portable primitives useful across unrelated programs.

LINK owns vehicle-diagnostics/application behaviour shared by MBLINK and JAGLINK. That includes the workspace model, Classical CAN and CAN-FD ISO-TP, transport contracts, ELM327 framing/parser/session behaviour, standard OBD-II, generic DTC knowledge, generic UDS and the complete standard service codec catalogue, parameter definitions/store/history, scheduling, telemetry/CSV, portable diagnostic sequencing, Discover safety/evidence, generic ECU/module interrogation machinery and the shared Windows OpenPort/J2534 scanner.

MBLINK owns Mercedes identity, the C207/OM651/CRD3 profile, Mercedes endpoint/definition provenance, Mercedes DTC knowledge, Mercedes module topology and genuinely Mercedes-specific diagnostic behaviour. Product-prefixed files that delegate to LINK are compatibility adaptors, not independent implementations.

The ownership rule applies to diagnostic knowledge as well as protocol code. Standards-defined generic DTC descriptions/classification, OBD freeze-frame/readiness semantics and generic UDS DTC status interpretation belong in LINK. Mercedes-Benz/CRD3/OM651-specific DTC definitions, module associations and manufacturer diagnostic metadata belong in MBLINK. See `FAULT_DIAGNOSTICS.md` for the normative product requirement.

The same rule governs Discover. Generic scanning, transport, identification state, safety, evidence and dump formatting belong in LINK. Mercedes-specific module identities, known endpoints, evidence-backed read-only requests and decoders belong in MBLINK.

The exact LINK revision is the `src/link` gitlink. LINK owns its nested exact Common revision. MBLINK's CMake and CI validate that recursive dependency graph rather than maintaining duplicate expected-version constants.

## Application targets

The repository boundary is intentionally broader than one executable:

```text
MBLINK repository
  |- main diagnostic application
  `- MBLINK Discover
```

A second executable does not justify a second repository when it shares the same manufacturer knowledge, release ownership, branding and LINK dependency.

The main application is the user-facing diagnostic product. Discover is the engineering-oriented reader/dumper that explores vehicle networks and modules more deeply while remaining read-oriented and evidence-preserving.

The current Windows Discover target provides passive CAN capture and bounded OBD inventory. Future manufacturer-aware module discovery should extend this target rather than introducing a parallel `MBLINK Reader` codebase.

## Platform boundaries

```text
SwiftUI / Objective-C                  GTK4 / C
          \                              /
           +---- MBLINK product face ----+
                        |
                    LINK::Core
                        |
               Infiltratr Common
                        |
             transport/provider edge
                        |
                adapter -> vehicle
```

Objective-C owns Apple framework integration such as CoreBluetooth lifecycle and write/notification mechanics. SwiftUI owns iPhone presentation. Neither layer may carry an alternate ELM327/OBD/ISO-TP/UDS implementation or a separate fault-code lookup database.

The Linux shell is C/GTK4 and renders the same portable workspace and Mercedes profile. Linux BLE/serial and native OpenPort 2.0 providers already live at LINK's transport edge; a future SocketCAN provider belongs at that same edge rather than in a protocol or manufacturer layer.

Windows Discover is a branded target over LINK's shared scanner shell. If Discover later gains Linux or Apple specialist shells, they should follow the same rule: presentation/provider code at the platform edge, manufacturer knowledge in MBLINK, generic scanner behaviour in LINK.

Platform UIs consume resolved diagnostic records from the portable/shared and manufacturer layers. SwiftUI, GTK and Win32 must not independently translate the same DTC or module identity into conflicting meanings.

The iPhone model carries structured generic `DiagnosticFault` values resolved through LINK. The existing Faults screen receives compatibility `CODE — description` strings from those records while the richer card UI is completed. This compatibility surface is temporary presentation glue; the structured model remains authoritative.

## Native iPhone dependency detail

The Xcode project references Infiltratr Common through LINK's nested checkout (`src/link/src/infiltratr-common`). This is a build-system detail, not a second top-level product dependency: `.gitmodules` contains only LINK, and LINK owns the Common pin.

The iOS target compiles product compatibility bridge files which include the exact shared LINK implementation from the pinned submodule. In particular, the UDS bridge includes both LINK's core UDS implementation and its compiled service-codec implementation. CMake consumers link `LINK::Core` directly. CI explicitly checks this Apple bridge so a new shared LINK implementation cannot disappear from iOS unnoticed.

## Timing, ownership and failure

Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery.

Manufacturer definitions carry evidence/provenance status. Undocumented identifiers, DTC meanings or formulas are not promoted to factual vehicle definitions without source evidence or reproducible capture fixtures.

A failed or unperformed fault scan must remain distinguishable from a successful clean scan all the way through the portable model to the user interface. The Apple controller preserves this distinction with explicit waiting/error strings and a successful `Complete · …` result; the Faults presentation must consume that state rather than infer success from empty arrays.

Discover must preserve the same evidence discipline. A module not responding, a request being blocked, an unsupported identifier and a successful empty/clean result are distinct outcomes and must not be collapsed into one state.

## Diagnostic product rule

Acquiring a raw DTC is only the first stage of fault diagnosis. Where trustworthy knowledge exists, the portable model must carry the translated description, classification, module/system information, state and available diagnostic context while preserving the raw code/status for evidence. Live dashboards are supporting diagnostic views and do not substitute for this path.

The pinned LINK layer is the single source of truth for generic DTC interpretation, CAN-FD/ISO-TP and generic UDS services. The next shared-flow work is readiness/freeze-frame context; the next manufacturer layer is the evidence-backed Mercedes/CRD3/OM651 DTC catalogue.

For Discover, a "dump" means structured read-only acquisition of available diagnostic information and raw responses. It does not grant reset, security access, coding, programming or firmware-write authority.

The complete fault requirement is defined in `FAULT_DIAGNOSTICS.md`; the Discover boundary is defined in `DISCOVER.md` and LINK's `docs/DISCOVER.md` / `docs/PRODUCT_FACES.md`.

## Documentation standard

Public C APIs describe contracts, ownership, lifetime, failure behaviour and important invariants. Source comments explain rationale, state-machine subtleties and non-obvious constraints. Comments do not merely translate straightforward code into English.
