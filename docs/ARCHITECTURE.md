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

LINK owns vehicle-diagnostics/application behaviour shared by MBLINK and JAGLINK. In LINK 0.6.0 that includes the workspace model, Classical-CAN ISO-TP, parameter definitions/store/history, scheduling, telemetry/CSV, Discover safety/evidence and the shared Windows OpenPort/J2534 scanner.

MBLINK owns Mercedes identity, the C207/OM651/CRD3 profile, Mercedes endpoint/definition provenance and genuinely Mercedes-specific diagnostic behaviour. Product-prefixed files that delegate to LINK are compatibility adaptors, not independent implementations.

ELM327, standard OBD-II and UDS remain product-local migration candidates for the next LINK releases.

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

Objective-C owns Apple framework integration such as CoreBluetooth lifecycle and write/notification mechanics. SwiftUI owns iPhone presentation. Neither layer may carry an alternate ELM327/OBD/ISO-TP/UDS implementation.

The Linux shell is C/GTK4 and renders the same portable workspace and Mercedes profile. A future Linux BLE/serial/SocketCAN provider remains a transport-edge concern rather than a protocol or manufacturer layer.

## Native iPhone dependency detail

The Xcode project references Infiltratr Common through LINK's nested checkout (`src/link/src/infiltratr-common`) because LINK does not yet expose an Apple project that wraps every native-iOS source. This is a build-system detail, not a second top-level product dependency: `.gitmodules` contains only LINK, and LINK owns the Common pin.

## Timing, ownership and failure

Portable state machines use caller-supplied monotonic time rather than OS clocks. Buffer ownership and provider/session lifetimes are explicit. Terminal protocol errors require the documented reset/resynchronisation operation rather than heuristic recovery.

Manufacturer definitions carry evidence/provenance status. Undocumented identifiers or formulas are not promoted to factual vehicle definitions without source evidence or reproducible capture fixtures.

## Documentation standard

Public C APIs describe contracts, ownership, lifetime, failure behaviour and important invariants. Source comments explain rationale, state-machine subtleties and non-obvious constraints. Comments do not merely translate straightforward code into English.
