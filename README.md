<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK

MBLINK is the Mercedes-Benz product face built on the shared LINK vehicle-diagnostics engine. The current development vehicle is the C207 E 250 CDI / OM651 with Delphi CRD3.x engine management.

**Current release: 0.7.25.**

## Dependency hierarchy

```text
Infiltratr Common
        ↓
       LINK
        ↓
      MBLINK
 Mercedes face
```

MBLINK pins LINK 0.9.1 at `src/link`. LINK owns and pins Infiltratr Common 1.11.0 beneath it; MBLINK carries no second top-level Common submodule.

LINK is the source of truth for the shared workspace, ISO-TP, byte-stream transport ABI, ELM327 framing/parser/initialisation, ELM-managed CAN, ELM session/probe, parameter/store/scheduler/telemetry runtime, standard OBD-II, UDS, the portable diagnostic-flow controller, Discover safety/evidence, and the native Windows OpenPort 2.0/J2534 scanner shell. MBLINK retains small compatibility façades for those APIs plus Mercedes identity, Mercedes definitions and manufacturer-specific diagnostic behaviour.

## C-first architecture

Portable diagnostic behaviour is C11. C++ is preferred where it materially improves a design; platform-required languages remain narrow presentation/interop edges and must not become alternate protocol implementations.

The native Linux shell is C/GTK4. The iPhone shell uses SwiftUI and Objective-C only at the Apple boundary while consuming the same portable C diagnostic model.

## Build

```sh
git clone --recurse-submodules https://github.com/The-First-Infiltrator/MBLINK.git
cd MBLINK
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

### Linux

```sh
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release -DMBLINK_BUILD_LINUX_APP=ON
cmake --build build-linux --target mblink-linux
./build-linux/mblink-linux
```

The Linux shell uses the Mercedes three-pointed-star MBLINK emblem and includes the standard About dialog with version, author, website and GPL information.

### iPhone

The native project is `app/ios/MBLINK.xcodeproj`. It uses the MBLINK emblem throughout the app and carries `Copyright © 2026 Shannon Smith` in both the visible About experience and bundle metadata.

### Windows Discover

`mblink-discover` is the Mercedes face of LINK's shared read-only OpenPort/J2534 scanner. LINK 0.9.1 supplies a modern native Windows shell with Common Controls v6 styling, a File/Help menu, a native Task Dialog About screen, DPI-aware layout, responsive evidence controls and UTF-8-safe status rendering. The executable is linked with the static MSVC runtime so it can start on a clean Windows machine without a matching Visual C++ redistributable.

The Windows executable uses the exact `AppIcon-60@3x.png` from the iPhone asset catalogue as its canonical product image. LINK wraps those PNG bytes into the Windows icon resource and embeds product/version metadata at build time; no independent MBLINK `.ico` is maintained.

Windows CI does not stop at compilation: it starts the built EXE, waits for the `MBLINK Discover` main window, verifies that it remains alive, checks that no dynamic Visual C++ runtime DLL is required, closes it cleanly, and fails the release if that smoke test does not pass.

## Release assets

A successful release is atomic across all supported targets and is required to publish these top-level assets before the release job can pass:

- `MBLINK-X.Y.Z-unsigned.ipa`
- `MBLINK-X.Y.Z-linux-amd64.deb`
- `MBLINK-X.Y.Z-linux-native.run`
- `MBLINK-X.Y.Z-windows-discover.exe`
- `MBLINK-X.Y.Z-source.zip`
- `SHA256SUMS.txt`

The native `.run` contains the complete dependency tree, including LINK and LINK's pinned Common checkout, and builds/tests MBLINK natively before installation. There is one release publisher only; it fails if any required asset is absent.

## Engineering rules

- Shared automotive behaviour belongs in LINK.
- Broadly reusable non-automotive primitives belong in Infiltratr Common.
- Mercedes-only definitions and behaviour stay in MBLINK.
- C is the default implementation language; C++ is used where it genuinely improves the design.
- Public APIs document ownership, lifetime, failure behaviour and invariants; comments explain rationale rather than obvious syntax.
- Manufacturer-specific data remains evidence-gated until documentation or reproducible vehicle captures establish its meaning.

See `docs/ARCHITECTURE.md`, `docs/MERCEDES.md`, `docs/APPLE.md` and `docs/ROADMAP.md`.

## Licence

Copyright © 2026 Shannon Smith.

MBLINK is licensed under `GPL-3.0-or-later`.
