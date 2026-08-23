<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

[![MBLINK CI](https://github.com/The-First-Infiltrator/MBLINK/actions/workflows/ci.yml/badge.svg)](https://github.com/The-First-Infiltrator/MBLINK/actions/workflows/ci.yml)

MBLINK is the Mercedes-Benz product face built on the shared LINK vehicle-diagnostics engine. The current development target is the C207 E 250 CDI / OM651 with Delphi CRD3.x engine management.

**Current source version:** 0.7.26  
**Shared engine:** LINK 0.10.0 → Infiltratr Common 1.11.0  
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

MBLINK pins LINK at `src/link`. LINK owns the Common dependency beneath it, so MBLINK carries no second top-level Common submodule.

Shared workspace, ISO-TP, transport, ELM327, standard OBD-II, generic DTC knowledge, UDS, scheduler/telemetry, portable diagnostic sequencing, Discover safety/evidence and the Windows OpenPort/J2534 shell belong in LINK. Mercedes identity, definitions and genuinely Mercedes-specific behaviour remain in MBLINK.

## Diagnostic priority

Fault diagnosis is a first-class product function. Acquiring and printing a raw `Pxxxx`/`Uxxxx` or Mercedes UDS value is not considered complete fault support.

MBLINK now consumes LINK 0.10.0's shared generic DTC knowledge layer. Standard OBD fault records can be translated into a human-readable definition, system/category and generic/manufacturer classification while preserving the raw code. The iPhone model carries structured `DiagnosticFault` records and the existing Faults presentation receives translated `CODE — description` text for known definitions. Unknown manufacturer-specific codes remain explicitly unmapped rather than receiving invented meanings.

The remaining fault-diagnostic completion work is deliberately ahead of additional dashboard polish: rich fault-card presentation with correct not-scanned/failed/clean states, freeze-frame/readiness context integration, and the evidence-backed Mercedes/CRD3/OM651 DTC knowledge layer. See `docs/FAULT_DIAGNOSTICS.md`.

## Capabilities

- Shared ELM327/OBD-II/UDS diagnostics through LINK.
- Shared generic DTC interpretation through LINK 0.10.0, including high-value engine/diesel and network definitions plus ISO 14229 status semantics.
- Mercedes-specific diagnostic extension hook and ECU probing.
- Native C/GTK4 Linux application.
- Native iPhone application using SwiftUI/Objective-C only at the Apple presentation/interoperability edge.
- Read-only Windows OpenPort 2.0/J2534 Discover scanner with evidence export.
- Shared portable diagnostic-flow state machine across platforms.
- Canonical Mercedes three-pointed-star branding reused across supported targets.

Manufacturer-specific data remains evidence-gated until documentation or reproducible vehicle captures establish its meaning.

## Architecture

Portable diagnostic behaviour is C11. C++ is used only where it materially improves a design. Platform-required languages remain narrow presentation/interop edges and must not become alternate protocol implementations.

The Linux shell is C/GTK4. The iPhone shell consumes the same portable C model. Windows Discover is the MBLINK face of LINK's shared read-only OpenPort/J2534 scanner shell.

The Windows executable uses the canonical MBLINK app icon from the iPhone asset catalogue, embeds product/version metadata and links the static MSVC runtime. CI launches the built EXE, verifies the `MBLINK Discover` window stays alive and checks that no matching dynamic Visual C++ runtime is required.

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

GitHub CI verifies the portable core, sanitizer coverage, Linux application/package path, Windows Discover executable and launch smoke test, Apple/iOS build and unsigned physical-device IPA before any release job can run. The portable test suite now also verifies that LINK's shared DTC knowledge reaches the MBLINK compatibility/product layer.

## Release assets

A successful numbered release is atomic across supported targets and publishes:

| File | Purpose |
| --- | --- |
| `MBLINK-<version>-unsigned.ipa` | Unsigned physical-device iPhone package. |
| `MBLINK-<version>-linux-amd64.deb` | Generic Linux amd64 Debian package. |
| `MBLINK-<version>-linux-native.run` | Native local Linux build/install program. |
| `MBLINK-<version>-windows-discover.exe` | Read-only Windows OpenPort/J2534 Discover application. |
| `MBLINK-<version>-source.zip` | Exact tested source archive including the pinned dependency tree. |
| `SHA256SUMS.txt` | SHA-256 checksums for all project-owned release artifacts. |

The `.run` contains the complete dependency tree, including LINK and LINK's pinned Common checkout, and builds/tests MBLINK natively before installation.

## Repository and release policy

This repository uses `main` as its working branch. Development changes are made directly on `main`; the normal project workflow does not depend on PR, feature or release branches.

Every push to `main` runs MBLINK CI. Ordinary commits do not publish. A commit is release-eligible only when its subject begins with the exact source version as `Release <version>` and every required CI job succeeds.

The release job re-checks that the tested SHA is still the exact current `main`, verifies the complete artifact set, creates the version tag and publishes the atomic release. Existing version tags and published releases are immutable and are never moved, replaced or edited in place.

Manually runnable build/smoke workflows are diagnostic helpers only and are not release-approval mechanisms.

## Engineering rules

- Broadly reusable non-automotive primitives belong in Infiltratr Common.
- Shared automotive behaviour and generic diagnostic knowledge belong in LINK.
- Mercedes-only definitions and behaviour stay in MBLINK.
- Public APIs document ownership, lifetime, failure behaviour and invariants.
- Comments explain rationale and non-obvious state-machine constraints rather than obvious syntax.
- Unknown diagnostic definitions remain explicit and evidence-preserving; unknown or unsafe diagnostic services are denied before transport transmission.

## Documentation

See `docs/FAULT_DIAGNOSTICS.md`, `docs/ARCHITECTURE.md`, `docs/MERCEDES.md`, `docs/APPLE.md` and `docs/ROADMAP.md`.

## Licence

Copyright © 2026 Shannon Smith.

MBLINK is free software licensed under the GNU General Public License version 3 or, at your option, any later version (`GPL-3.0-or-later`).
