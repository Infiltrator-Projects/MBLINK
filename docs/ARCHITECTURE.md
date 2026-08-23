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

LINK owns vehicle-diagnostics/application behaviour shared by MBLINK and JAGLINK. That includes the workspace model, Classical-CAN ISO-TP, transport contracts, ELM327 framing/parser/session behaviour, standard OBD-II, generic DTC knowledge, generic UDS, parameter definitions/store/history, scheduling, telemetry/CSV, portable diagnostic sequencing, Discover safety/evidence and the shared Windows OpenPort/J2534 scanner.

MBLINK owns Mercedes identity, the C207/OM651/CRD3 profile, Mercedes endpoint/definition provenance, Mercedes DTC knowledge and genuinely Mercedes-specific diagnostic behaviour. Product-prefixed files that delegate to LINK are compatibility adaptors, not independent implementations.

The ownership rule applies to diagnostic knowledge as well as protocol code. Standards-defined generic DTC descriptions/classification, OBD freeze-frame/readiness semantics and generic UDS DTC status interpretation belong in LINK. Mercedes-Benz/CRD3/OM651-specific DTC definitions, module associations and manufacturer diagnostic metadata belong in MBLINK. See `FAULT_DIAGNOSTICS.md` for the normative product requirement.

MBLINK currently pins LINK 0.10.0. Its shared `dtc_knowledge` API is the source of truth for generic fault-code classification/translation and ISO 14229 status semantics. MBLINK's compatibility OBD API exposes that shared model, and the iOS OBD bridge compiles the exact pinned LINK implementation rather than a copied table.

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

The iPhone model now carries structured generic `DiagnosticFault` values resolved through LINK. The existing Faults screen receives compatibility `CODE — description` strings from those records while the richer card UI is completed. This compatibility surface is temporary presentation glue; the structured model remains authoritative.

## Native iPhone dependency detail

The Xcode project references Infiltratr Common through LINK's nested checkout (`src/link/src/infiltratr-common`) because LINK does not yet expose an Apple project that wraps every native-iOS source. This is a build-system detail, not a second top-level product dependency: `.gitmodules` contains only LINK, and LINK owns the Common pin.

The existing iOS target lists MBLINK's OBD bridge source. That bridge includes LINK's exact OBD-II and DTC-knowledge C sources from the pinned submodule on iOS; CMake consumers instead link `LINK::Core` directly. This preserves one implementation across build systems.

## Timing, ownership and failure

Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery.

Manufacturer definitions carry evidence/provenance status. Undocumented identifiers, DTC meanings or formulas are not promoted to factual vehicle definitions without source evidence or reproducible capture fixtures.

A failed or unperformed fault scan must remain distinguishable from a successful clean scan all the way through the portable model to the user interface. The Apple controller already preserves this distinction with explicit waiting/error strings and a successful `Complete · …` result; the Faults presentation must consume that state rather than infer success from empty arrays.

## Diagnostic product rule

Acquiring a raw DTC is only the first stage of fault diagnosis. Where trustworthy knowledge exists, the portable model must carry the translated description, classification, module/system information, state and available diagnostic context while preserving the raw code/status for evidence. Live dashboards are supporting diagnostic views and do not substitute for this path.

LINK 0.10.0 completes the first shared interpretation step for generic DTCs. The next shared-flow step is readiness/freeze-frame context; the next manufacturer step is the evidence-backed Mercedes/CRD3/OM651 DTC catalogue.

The complete requirement is defined in `FAULT_DIAGNOSTICS.md` and is considered part of the architecture, not optional UI polish.

## Documentation standard

Public C APIs describe contracts, ownership, lifetime, failure behaviour and important invariants. Source comments explain rationale, state-machine subtleties and non-obvious constraints. Comments do not merely translate straightforward code into English.
