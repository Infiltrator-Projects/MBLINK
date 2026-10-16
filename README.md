<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLink

**Project copyright:** © 1993-2026 Shannon Smith

[![MBLink CI](https://github.com/Infiltrator-Projects/MBLink/actions/workflows/ci.yml/badge.svg)](https://github.com/Infiltrator-Projects/MBLink/actions/workflows/ci.yml)

MBLink is the Mercedes-Benz product face built on the shared Link vehicle-diagnostics engine. Its scope is Mercedes-Benz vehicles broadly. The C207 E 250 CDI / OM651 with Delphi CRD3.x engine management is the current physical development/evidence vehicle, not the product boundary.

**Current source version:** see [`VERSION`](VERSION)
**Shared engine:** exact Link gitlink at `src/link`; Link owns the nested Libraries pin
**Platforms:** Linux, iPhone/iOS and Windows Discover
**Licence:** GPL-3.0-or-later

## Engineering ethos

What can a Mercedes diagnostic tool built from first principles actually know, and how can it prove that knowledge without guessing? MBLink is where Mercedes-specific vehicle evidence becomes product behaviour on top of the shared Link diagnostic engine.

Link owns generic standards, transports, sequencing and safety policy; MBLink owns Mercedes identity, topology, definitions and genuinely manufacturer-specific behaviour. Public standards, captured traffic, service information and mature tools are evidence to reconcile, not code or assumptions to depend on blindly. Unknown values remain unknown until there is evidence strong enough to name or interpret them.

The project keeps diagnostic authority inside the reviewed source and pinned dependency tree. A newer technique, database or protocol interpretation is adopted only when it improves fidelity, coverage, safety or explainability. Read-only discovery remains deny-by-default, and adding knowledge never silently grants permission to transmit a more dangerous request.

## Role in the project family

```text
Libraries
        ↓
       Link
        ↓
      MBLink
 Mercedes face
```

MBLink pins one exact Link commit at `src/link`. Link owns Common beneath it, so MBLink carries no second top-level Common submodule and cannot silently choose a different Common revision.

Shared workspace, Classical CAN/CAN-FD ISO-TP, transport, ELM327, standard OBD-II, generic DTC knowledge, the complete product-neutral UDS codec catalogue, scheduler/telemetry, portable diagnostic sequencing, Discover safety/evidence and the Windows OpenPort/J2534 shell belong in Link. Mercedes identity, definitions and genuinely Mercedes-specific behaviour remain in MBLink.

## Applications in this repository

MBLink is one manufacturer product family with more than one application target. It is intentionally **not** split into separate `MBLink` and `MBLink-Reader` repositories.

```text
MBLink repository
  |- MBLink
  |    normal Mercedes diagnostic application
  |
  `- MBLink Discover
       specialist Mercedes ECU/module discovery,
       identification, read-only inventory and evidence/dump tool
```

The main application is the normal driver/technician diagnostic experience. Discover is the deeper engineering-oriented reader used to determine what control modules are present, identify them, read documented information and preserve raw evidence.

Windows Discover now combines passive CAN capture and the bounded read-only standard OBD inventory with an explicit FULL SWEEP mode for deeper engineering work. FULL SWEEP searches the broader Mercedes diagnostic address space only when the operator requests it; the normal diagnostic path remains deliberately fast and bounded.

Generic Discover mechanics belong in Link. Mercedes-specific network topology, module identities, endpoints, read-only probes and decoders belong here.

## Diagnostic priority

Fault diagnosis is a first-class product function. Acquiring and printing a raw `Pxxxx`/`Uxxxx` or Mercedes UDS value is not considered complete fault support.

MBLink consumes Link's shared generic DTC knowledge layer. Standard OBD fault records can be translated into a human-readable definition, system/category and generic/manufacturer classification while preserving the raw code. The iPhone model carries structured `DiagnosticFault` records and the existing Faults presentation receives translated `CODE — description` text for known definitions. Unknown manufacturer-specific codes remain explicitly unmapped rather than receiving invented meanings.

The iPhone and Linux fault-investigation surfaces distinguish not-scanned or
in-progress, failed/incomplete, verified-clean and faults-present results instead
of treating an empty list as proof of no faults. Standards-defined readiness and
capability-gated Mode 02 frame-zero freeze-frame context are now integrated into
the shared diagnostic flow and shown separately from current live data. Remaining
fault-diagnostic work is evidence-backed expansion of the Mercedes/CRD3/OM651
manufacturer DTC knowledge layer. See `docs/FAULT_DIAGNOSTICS.md`.

## Capabilities

- Shared ELM327/OBD-II/UDS diagnostics through Link, including explicit legacy J1850 PWM/VPW, ISO 9141-2, ISO 14230-4 and ISO 15765-4 protocol selection.
- Correct J1979 Service 05 TID+O2-sensor framing and structured CAN Service 06 OBDMID/TID/UASID decoding from Link 0.14.61; current target J1979DA_202607 is kept separate from the compiled public semantic baseline.
- Raw-preserving Mode 05/06/09 standard identifier access and the J1979-2_202604 OBDonUDS profile inherited from Link.
- Shared ISO 13400 DoIP diagnostic framing for modern OBDonUDS/WWH-OBD transport foundations.
- Shared Classical CAN and CAN-FD ISO-TP, including 64-byte CAN-FD payloads and extended ISO-TP lengths.
- Product-prefixed access to Link's complete 27-service UDS catalogue and codecs without duplicating implementations.
- Shared generic DTC interpretation through Link, including high-value engine/diesel and network definitions plus ISO 14229 status semantics.
- Mercedes-specific diagnostic extension hook and ECU probing.
- Responder-attributed iPhone module cards: each observed ECU keeps its CAN route, identity, fault evidence and live Mode 01 values separate from other responders.
- Mercedes-wide DID and signal research layer with source-state tracking, documented scaling decode and time-aligned signal correlation; chassis/engine-specific labels are used only when the evidence is actually specific.
- Archived Mercedes me Whisper parameterisation evidence with reproducible decryption, manufacturer-owned VIN routes/request contracts and a source-corroborated factory DataID inventory; live request/formula mappings remain evidence-gated.
- Native C/GTK4 Linux application.
- Native iPhone application using SwiftUI/Objective-C only at the Apple presentation/interoperability edge.
- Explicit FULL SWEEP controls on Linux and Windows for read-only forensic module discovery across the wider 11-bit diagnostic range and ISO 15765 normal-fixed 29-bit targets, with F197 identity evidence and address-preserving unknown-module labels.
- MBLink Discover: the branded specialist ECU/module reader on Windows with passive J2534 capture, bounded normal inventory, optional FULL SWEEP and evidence export.
- Shared portable diagnostic-flow state machine across platforms.
- Canonical Mercedes three-pointed-star branding reused across supported targets.

Manufacturer-specific data remains evidence-gated until documentation or reproducible vehicle captures establish its meaning. First-party Mercedes backend semantics are treated as Mercedes-wide research vocabulary with explicit applicability uncertainty, never as C207-only data and never as proof that every Mercedes supports the signal.

Heavy unknown-vehicle research is a Linux/Windows responsibility: MBLink Discover uses the shared Link research engine for passive capture, module census, read-only data harvest and evidence analysis, while iPhone remains the normal diagnostic instrument. See `docs/VEHICLE_RESEARCH.md`.

Mercedes me application semantics recovered from the archived official app (DataIds, DiagLogic and Whisper vocabulary/policies) are maintained in MBLink; Link owns only the reusable adapter transport/native framing boundary. See `docs/MERCEDES-ME-DATA-IDS.md`.

## Architecture

Portable diagnostic behaviour is C11. C++ is used only where it materially improves a design. Platform-required languages remain narrow presentation/interop edges and must not become alternate protocol implementations.

The Linux shell is C/GTK4. The iPhone shell consumes the same portable C model. Windows Discover is the MBLink face of Link's shared read-only OpenPort/J2534 scanner shell and the current platform implementation of the specialist ECU/module reader.

The Windows executable uses the canonical MBLink app icon from the iPhone asset catalogue, embeds product/version metadata and links the static MSVC runtime. CI launches the built EXE, verifies the `MBLink Discover` window stays alive and checks that no matching dynamic Visual C++ runtime is required.

As Discover gains manufacturer-aware depth, the boundary stays the same: reusable interrogation, transport, safety and evidence behaviour goes into Link; Mercedes-specific definitions and read-only requests remain in MBLink.

## Build and test

```bash
git clone --recurse-submodules https://github.com/Infiltrator-Projects/MBLink.git
cd MBLink
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Linux application:

```bash
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release -DMBLINK_BUILD_LINUX_APP=ON
cmake --build build-linux --target mblink-linux
./build-linux/mblink-linux
```

Linux adapter discovery is owned by Link. MBLink therefore supports the same
native choices without product-specific transport code: tty/RFCOMM ELM327,
Vgate-style BLE/GATT and Bluetooth Classic/SPP, plus direct USB Tactrix
OpenPort 2.0. A connected Tactrix appears as
`OP2:Tactrix OpenPort 2.0`; Linux talks to it through Link/libusb and does not
need the Windows J2534 DLL. The current native OpenPort path covers the
CAN/ISO-15765 modes used by the C207 diagnostics (11/29-bit, 500/250 kbit/s).

Offline C207/Vgate replay:

```bash
./build-linux/mblink-linux --replay-c207
```

Replay mode auto-connects a deterministic ELM327 v2.3/Vgate transport. It reuses the captured C207/OM651 PID-support, CRD3, response-pending, negative-UDS and deep-scan NO-DATA shapes, with the real VIN serial replaced by a synthetic suffix. A clearly labelled synthetic ORC restraint-controller response is injected early in the Linux forensic sweep so the Vehicle and Faults views must visibly surface an airbag/seatbelt test fault while the rest of the sweep continues. Replay evidence is never presented as a fault read from a live vehicle.

Linux `SAVE SESSION` now preserves one complete diagnostic investigation across repeated `Link DOWN` / `Link UP` cycles. Each physical connection is numbered as a separate attempt, with its own start/end markers, elapsed timing, adapter identity, outcome, diagnostic stage, retry/failure state, product-specific Mercedes state and every ELM command/raw response. Reconnecting no longer erases the previous attempt; restarting the application begins a new investigation.

The native iPhone project is `app/ios/MBLINK.xcodeproj`.

The iPhone target bundles the three canonical MB Corpo faces. At startup MBLink registers the bundled TTF files with CoreText, resolves their actual PostScript names from the font data and uses those faces throughout SwiftUI. Font registration is never process-fatal: a genuinely unavailable face falls back safely while the build-time SHA-256 checks continue to enforce the exact bundled font assets.

GitHub CI validates repository invariants, the portable core, sanitizer coverage, Linux application behaviour, the Windows Discover build and launch, and the Apple/iOS simulator diagnostic flow. CI never edits source, creates tags or publishes releases. The separate `MBLink Release` workflow rebuilds and verifies the release packages from one immutable source revision before publication.

## Release assets

A successful numbered release is atomic across supported targets and publishes:

| File | Purpose |
| --- | --- |
| `MBLink-<version>-unsigned.ipa` | Unsigned physical-device iPhone package. |
| `MBLink-<version>-linux-amd64.deb` | Generic Linux amd64 Debian package. |
| `MBLink-<version>-linux-native.run` | Native local Linux build/install program. |
| `MBLink-<version>-windows-discover.exe` | MBLink Discover specialist read-only ECU/module scanner and evidence application. |
| `MBLink-<version>-source.zip` | Complete recursive source snapshot, including the pinned Link and Common trees. |
| `SHA256SUMS.txt` | SHA-256 checksums for all project-owned release artifacts. |

GitHub also provides its automatic Source code (zip) and Source code (tar.gz) archives for every tagged release. The project-owned source ZIP is the release workflow's verified recursive snapshot and therefore includes the pinned dependency trees rather than only top-level gitlinks.

The `.run` contains the complete dependency tree, including Link and Link's pinned Common checkout, and builds/tests MBLink natively before installation.

Release notes derive the Link and Common versions from that exact dependency tree rather than maintaining separate hard-coded version strings.

## Repository and release policy

Development stays on `main`. `VERSION` identifies release intent. Complete CI qualifies the exact source and distributables once; the publisher consumes those artifacts and byte-verifies the draft before public release. Published tags and assets remain immutable. See [Validation](docs/VALIDATION.md) for the qualification boundaries.

## Engineering rules

- Broadly reusable non-automotive primitives belong in Libraries.
- Shared automotive behaviour and generic diagnostic knowledge belong in Link.
- Mercedes-only definitions and behaviour stay in MBLink.
- A second MBLink application target does not require a second repository; Discover remains part of this manufacturer product family.
- Public APIs document ownership, lifetime, failure behaviour and invariants.
- Comments explain rationale and non-obvious state-machine constraints rather than obvious syntax.
- Unknown diagnostic definitions remain explicit and evidence-preserving; unknown or unsafe diagnostic services are denied before transport transmission.

## Documentation

Use the smallest document that owns the question:

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) owns repository boundaries and engineering rules.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) owns completion status and priorities; it does not redefine dependency versions.
- [`docs/FAULT_DIAGNOSTICS.md`](docs/FAULT_DIAGNOSTICS.md) owns the end-to-end fault requirement.
- [`docs/VEHICLE_PROFILES.md`](docs/VEHICLE_PROFILES.md) owns the complete iPhone Connect, adapter-selection, VIN/profile and bounded module-census flow.
- [`docs/PID_ARCHITECTURE.md`](docs/PID_ARCHITECTURE.md) owns the later PID Setup hierarchy, catalogue separation, selection and polling rules.
- [`docs/MERCEDES.md`](docs/MERCEDES.md), [`docs/MERCEDES_ME_WHISPER.md`](docs/MERCEDES_ME_WHISPER.md), [`docs/DID_LAB.md`](docs/DID_LAB.md), [`docs/DISCOVER.md`](docs/DISCOVER.md) and [`docs/APPLE.md`](docs/APPLE.md) are scoped implementation/evidence notes. They do not override the authoritative contracts above or Link's shared contracts.

The committed gitlinks and `VERSION` files remain the sole authorities for dependency and source versions.

## Licence

Copyright © 1993-2026 Shannon Smith.

MBLink is free software licensed under the GNU General Public License version 3 or, at your option, any later version (`GPL-3.0-or-later`).
