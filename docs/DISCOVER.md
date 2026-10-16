<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLink Discover

MBLink Discover is the Mercedes-branded specialist ECU/module discovery and read-only evidence/dump application built on the shared Link Discover subsystem.

It is not a separate repository and it is not a future `MBLink-Reader` product. It is already the second application target inside the MBLink repository, alongside the main MBLink diagnostic application.

```text
MBLink repository
  |- main MBLink application
  `- MBLink Discover
       Mercedes ECU/module reader and evidence tool
```

## Ownership

MBLink does not own a separate generic discovery engine or Windows OpenPort/J2534 scanner implementation. Shared discovery behaviour, transport handling, safety classification, evidence writing, standard OBD inventory and generic ECU/module interrogation machinery live in Link.

MBLink supplies the Mercedes-specific layer:

- Mercedes-wide network and ECU knowledge, with C207-specific entries retained only where the evidence is actually C207-specific;
- known module identities and endpoint definitions;
- evidence-backed Mercedes read-only probes and identifiers;
- Mercedes-specific decoders where meaning is documented or reproducibly verified;
- MBLink branding, iconography and presentation.

The MBLink compatibility API is exposed through `include/mblink/discover.h`, which aliases product-prefixed names to the shared `link_*` API without duplicating implementation.

## Current implementation

On Windows, `mblink-discover.exe` is built through Link's `link_add_windows_discover` constructor from the same shared implementation used for JAGLink.

The current Discover baseline provides:

- passive 500 kbit/s CAN capture;
- bounded read-only standard OBD inventory for routine use;
- an explicit operator-requested `FULL SWEEP` across the wider 11-bit diagnostic range and ISO 15765 normal-fixed 29-bit targets;
- read-only `F197` system-name probing for responding modules, with unknown identities kept explicit rather than guessed;
- deny-by-default request classification;
- JSON Lines evidence export and operator annotations;
- product-branded Windows presentation.

The routine path remains bounded and fast. On iPhone, a new VIN is learned with a compact Mercedes gateway census: 47 exact 11-bit request/response slots (`TX = 0x602 + 8×slot`, `RX = 0x480 + slot`, slots 0..46), the source-backed out-of-grid Daimler routes, and the eight legislated OBD physical slots. This keeps the BLE adapter on exact receive filters instead of opening the live CAN bus.

`FULL SWEEP` is deliberately separate and remains the workstation forensic path. It exhaustively searches the broader 11/29-bit diagnostic space and may temporarily learn an unknown 11-bit response identifier from a headered diagnostic reply. The current Mercedes plan contains exactly 760 targets: 505 11-bit targets (the 0x600..0x7F7 range plus the externally evidenced 0x4E0 route) and 255 ISO 15765 normal-fixed 29-bit targets. Every transmitted probe and received response remains evidence-recorded.

A completed 2026-09-03 C207 field capture, made with the older MBLink 0.7.153 / Link 0.14.81 build, independently observed proprietary responders `0x602 -> 0x480`, `0x612 -> 0x482`, `0x632 -> 0x486`, `0x64A -> 0x489` and `0x652 -> 0x48A`, plus the two EOBD responders `0x7E0 -> 0x7E8` and `0x7E1 -> 0x7E9`. No 29-bit responder was observed in that completed pass. The old software version is provenance, not a validation of the current scanner implementation; the raw ECU response shapes are retained as sanitised regression evidence and replayed against current code.

## Intended evolution

MBLink Discover should evolve in place into the deeper Mercedes engineering reader/dumper:

```text
passive network observation
    -> standards-based inventory
    -> Mercedes-aware module discovery
    -> ECU/module identification
    -> documented read-only information acquisition
    -> structured raw/evidence dump
```

The main MBLink application remains the normal diagnostic experience. Discover is the specialist tool for determining what control modules exist, identifying them, reading supported information and preserving raw evidence for analysis.

A "dump" here means bounded read-only acquisition of diagnostic information and raw responses. It does not imply unrestricted coding, flashing, reset, security-access or programming capability.

## Architecture rule

Any generic scanner, transport, safety, evidence or reader improvement belongs in Link first. Any Mercedes-only definition or evidence-backed request belongs in MBLink.

Do not create a separate MBLink Reader repository and do not fork Link's scanner to add Mercedes depth. Extend the shared Discover engine where the behaviour is generic and supply Mercedes knowledge from this product repository.

The full behavioural, safety and repository contract is documented in Link's `docs/DISCOVER.md` and `docs/PRODUCT_FACES.md`.
