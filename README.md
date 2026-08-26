<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

[![MBLINK CI](https://github.com/The-First-Infiltrator/MBLINK/actions/workflows/ci.yml/badge.svg)](https://github.com/The-First-Infiltrator/MBLINK/actions/workflows/ci.yml)

MBLINK is the Mercedes-Benz product face built on the shared LINK vehicle-diagnostics engine. The current development target is the C207 E 250 CDI / OM651 with Delphi CRD3.x engine management.

**Current source version:** see [`VERSION`](VERSION)  
**Shared engine:** exact LINK gitlink at `src/link`; LINK owns the nested Infiltratr Common pin  
**Platforms:** Linux, iPhone/iOS and Windows Discover  
**Licence:** GPL-3.0-or-later

## Role in the project family

```text
Infiltratr Common
        ↓
       LINK
        ↓
      MBLINK
 Mercedes face
```

MBLINK pins one exact LINK commit at `src/link`. LINK owns Common beneath it, so MBLINK carries no second top-level Common submodule and cannot silently choose a different Common revision.

Shared workspace, Classical CAN/CAN-FD ISO-TP, transport, ELM327, standard OBD-II, generic DTC knowledge, the complete product-neutral UDS codec catalogue, scheduler/telemetry, portable diagnostic sequencing, Discover safety/evidence and the Windows OpenPort/J2534 shell belong in LINK. Mercedes identity, definitions and genuinely Mercedes-specific behaviour remain in MBLINK.

## Applications in this repository

MBLINK is one manufacturer product family with more than one application target. It is intentionally **not** split into separate `MBLINK` and `MBLINK-Reader` repositories.

```text
MBLINK repository
  |- MBLINK
  |    normal Mercedes diagnostic application
  |
  `- MBLINK Discover
       specialist Mercedes ECU/module discovery,
       identification, read-only inventory and evidence/dump tool
```

The main application is the normal driver/technician diagnostic experience. Discover is the deeper engineering-oriented reader used to determine what control modules are present, identify them, read documented information and preserve raw evidence.

Windows Discover now combines passive CAN capture and the bounded read-only standard OBD inventory with an explicit FULL SWEEP mode for deeper engineering work. FULL SWEEP searches the broader Mercedes diagnostic address space only when the operator requests it; the normal diagnostic path remains deliberately fast and bounded.

Generic Discover mechanics belong in LINK. Mercedes-specific network topology, module identities, endpoints, read-only probes and decoders belong here.

## Diagnostic priority

Fault diagnosis is a first-class product function. Acquiring and printing a raw `Pxxxx`/`Uxxxx` or Mercedes UDS value is not considered complete fault support.

MBLINK consumes LINK's shared generic DTC knowledge layer. Standard OBD fault records can be translated into a human-readable definition, system/category and generic/manufacturer classification while preserving the raw code. The iPhone model carries structured `DiagnosticFault` records and the existing Faults presentation receives translated `CODE — description` text for known definitions. Unknown manufacturer-specific codes remain explicitly unmapped rather than receiving invented meanings.

The remaining fault-diagnostic completion work is deliberately ahead of additional dashboard polish: rich fault-card presentation with correct not-scanned/failed/clean states, freeze-frame/readiness context integration, and the evidence-backed Mercedes/CRD3/OM651 DTC knowledge layer. See `docs/FAULT_DIAGNOSTICS.md`.

## Capabilities

- Shared ELM327/OBD-II/UDS diagnostics through LINK.
- Shared Classical CAN and CAN-FD ISO-TP, including 64-byte CAN-FD payloads and extended ISO-TP lengths.
- Product-prefixed access to LINK's complete 27-service UDS catalogue and codecs without duplicating implementations.
- Shared generic DTC interpretation through LINK, including high-value engine/diesel and network definitions plus ISO 14229 status semantics.
- Mercedes-specific diagnostic extension hook and ECU probing.
- Native C/GTK4 Linux application.
- Native iPhone application using SwiftUI/Objective-C only at the Apple presentation/interoperability edge.
- Explicit FULL SWEEP controls on Linux and Windows for read-only forensic module discovery across the wider 11-bit diagnostic range and ISO 15765 normal-fixed 29-bit targets, with F197 identity evidence and address-preserving unknown-module labels.
- MBLINK Discover: the branded specialist ECU/module reader on Windows with passive J2534 capture, bounded normal inventory, optional FULL SWEEP and evidence export.
- Shared portable diagnostic-flow state machine across platforms.
- Canonical Mercedes three-pointed-star branding reused across supported targets.

Manufacturer-specific data remains evidence-gated until documentation or reproducible vehicle captures establish its meaning.

## Architecture

Portable diagnostic behaviour is C11. C++ is used only where it materially improves a design. Platform-required languages remain narrow presentation/interop edges and must not become alternate protocol implementations.

The Linux shell is C/GTK4. The iPhone shell consumes the same portable C model. Windows Discover is the MBLINK face of LINK's shared read-only OpenPort/J2534 scanner shell and the current platform implementation of the specialist ECU/module reader.

The Windows executable uses the canonical MBLINK app icon from the iPhone asset catalogue, embeds product/version metadata and links the static MSVC runtime. CI launches the built EXE, verifies the `MBLINK Discover` window stays alive and checks that no matching dynamic Visual C++ runtime is required.

As Discover gains manufacturer-aware depth, the boundary stays the same: reusable interrogation, transport, safety and evidence behaviour goes into LINK; Mercedes-specific definitions and read-only requests remain in MBLINK.

## Build and test

```bash
git clone --recurse-submodules https://github.com/The-First-Infiltrator/MBLINK.git
cd MBLINK
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

The native iPhone project is `app/ios/MBLINK.xcodeproj`.

GitHub CI verifies the exact recursive dependency gitlinks, portable core, product-to-LINK facade, sanitizer coverage, Linux application/package path, Windows Discover executable and launch smoke test, Apple/iOS build and unsigned physical-device IPA before any release job can run.

## Release assets

A successful numbered release is atomic across supported targets and publishes:

| File | Purpose |
| --- | --- |
| `MBLINK-<version>-unsigned.ipa` | Unsigned physical-device iPhone package. |
| `MBLINK-<version>-linux-amd64.deb` | Generic Linux amd64 Debian package. |
| `MBLINK-<version>-linux-native.run` | Native local Linux build/install program. |
| `MBLINK-<version>-windows-discover.exe` | MBLINK Discover specialist read-only ECU/module scanner and evidence application. |
| `MBLINK-<version>-source.zip` | Exact tested source archive including the pinned dependency tree. |
| `SHA256SUMS.txt` | SHA-256 checksums for all project-owned release artifacts. |

The `.run` contains the complete dependency tree, including LINK and LINK's pinned Common checkout, and builds/tests MBLINK natively before installation.

Release notes derive the LINK and Common versions from that exact dependency tree rather than maintaining separate hard-coded version strings.

## Repository and release policy

This repository uses `main` as its working branch. Development changes are made directly on `main`; the normal project workflow does not depend on PR, feature or release branches.

Every push to `main` runs MBLINK CI. Ordinary commits do not publish. A commit is release-eligible only when its subject begins with the exact source version as `Release <version>` and every required CI job succeeds.

The release job re-checks that the tested SHA is still the exact current `main`, verifies the complete artifact set, creates the version tag and publishes the atomic release. Existing version tags and published releases are immutable and are never moved, replaced or edited in place.

Manually runnable build/smoke workflows are diagnostic helpers only and are not release-approval mechanisms.

## Engineering rules

- Broadly reusable non-automotive primitives belong in Infiltratr Common.
- Shared automotive behaviour and generic diagnostic knowledge belong in LINK.
- Mercedes-only definitions and behaviour stay in MBLINK.
- A second MBLINK application target does not require a second repository; Discover remains part of this manufacturer product family.
- Public APIs document ownership, lifetime, failure behaviour and invariants.
- Comments explain rationale and non-obvious state-machine constraints rather than obvious syntax.
- Unknown diagnostic definitions remain explicit and evidence-preserving; unknown or unsafe diagnostic services are denied before transport transmission.

## Documentation

See `docs/FAULT_DIAGNOSTICS.md`, `docs/ARCHITECTURE.md`, `docs/MERCEDES.md`, `docs/DISCOVER.md`, `docs/APPLE.md` and `docs/ROADMAP.md`.

## Licence

Copyright © 2026 Shannon Smith.

MBLINK is free software licensed under the GNU General Public License version 3 or, at your option, any later version (`GPL-3.0-or-later`).
