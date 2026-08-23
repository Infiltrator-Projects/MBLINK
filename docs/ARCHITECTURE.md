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

## Ownership

Infiltratr Common owns portable primitives useful across unrelated programs.

LINK owns vehicle-diagnostics/application behaviour shared by MBLINK and JAGLINK. That includes the workspace model, Classical CAN and CAN-FD ISO-TP, transport contracts, ELM327 framing/parser/session behaviour, standard OBD-II, generic DTC knowledge, generic UDS and the complete standard service codec catalogue, parameter definitions/store/history, scheduling, telemetry/CSV, portable diagnostic sequencing, Discover safety/evidence and the shared Windows OpenPort/J2534 scanner.

MBLINK owns Mercedes identity, the C207/OM651/CRD3 profile, Mercedes endpoint/definition provenance, Mercedes DTC knowledge and genuinely Mercedes-specific diagnostic behaviour. Product-prefixed files that delegate to LINK are compatibility adaptors, not independent implementations.

The ownership rule applies to diagnostic knowledge as well as protocol code. Standards-defined generic DTC descriptions/classification, OBD freeze-frame/readiness semantics and generic UDS DTC status interpretation belong in LINK. Mercedes-Benz/CRD3/OM651-specific DTC definitions, module associations and manufacturer diagnostic metadata belong in MBLINK. See `FAULT_DIAGNOSTICS.md` for the normative product requirement.

The exact LINK revision is the `src/link` gitlink. LINK owns its nested exact Common revision. MBLINK's CMake and CI validate that recursive dependency graph rather than maintaining duplicate expected-version constants.

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

The Linux shell is C/GTK4 and renders the same portable workspace and Mercedes profile. A future Linux BLE/serial/SocketCAN provider remains a transport-edge concern rather than a protocol or manufacturer layer.

Platform UIs consume resolved diagnostic records from the portable/shared and manufacturer layers. SwiftUI, GTK and Win32 must not independently translate the same DTC into different descriptions or statuses.

The iPhone model carries structured generic `DiagnosticFault` values resolved through LINK. The existing Faults screen receives compatibility `CODE — description` strings from those records while the richer card UI is completed. This compatibility surface is temporary presentation glue; the structured model remains authoritative.

## Native iPhone dependency detail

The Xcode project references Infiltratr Common through LINK's nested checkout (`src/link/src/infiltratr-common`). This is a build-system detail, not a second top-level product dependency: `.gitmodules` contains only LINK, and LINK owns the Common pin.

The iOS target compiles product compatibility bridge files which include the exact shared LINK implementation from the pinned submodule. In particular, the UDS bridge includes both LINK's core UDS implementation and its compiled service-codec implementation. CMake consumers link `LINK::Core` directly. CI explicitly checks this Apple bridge so a new shared LINK implementation cannot disappear from iOS unnoticed.

## Timing, ownership and failure

Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery.

Manufacturer definitions carry evidence/provenance status. Undocumented identifiers, DTC meanings or formulas are not promoted to factual vehicle definitions without source evidence or reproducible capture fixtures.

A failed or unperformed fault scan must remain distinguishable from a successful clean scan all the way through the portable model to the user interface. The Apple controller preserves this distinction with explicit waiting/error strings and a successful `Complete · …` result; the Faults presentation must consume that state rather than infer success from empty arrays.

## Diagnostic product rule

Acquiring a raw DTC is only the first stage of fault diagnosis. Where trustworthy knowledge exists, the portable model must carry the translated description, classification, module/system information, state and available diagnostic context while preserving the raw code/status for evidence. Live dashboards are supporting diagnostic views and do not substitute for this path.

The pinned LINK layer is the single source of truth for generic DTC interpretation, CAN-FD/ISO-TP and generic UDS services. The next shared-flow work is readiness/freeze-frame context; the next manufacturer layer is the evidence-backed Mercedes/CRD3/OM651 DTC catalogue.

The complete requirement is defined in `FAULT_DIAGNOSTICS.md` and is considered part of the architecture, not optional UI polish.

## Documentation standard

Public C APIs describe contracts, ownership, lifetime, failure behaviour and important invariants. Source comments explain rationale, state-machine subtleties and non-obvious constraints. Comments do not merely translate straightforward code into English.
