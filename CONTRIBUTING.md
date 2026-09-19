<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Contributing to MBLINK

MBLINK is the Mercedes-Benz product layer over LINK. Contributions must preserve the dependency hierarchy **Common → LINK → MBLINK** and keep genuinely Mercedes-specific behaviour here.

## Ownership rules

- Product-neutral automotive transports, OBD, ISO-TP, UDS, diagnostic sequencing, evidence and shared application behaviour belong in LINK.
- Broadly reusable non-automotive primitives belong in Common through LINK's pinned dependency.
- Mercedes identity, network/module knowledge, profiles, manufacturer definitions and verified Mercedes-specific behaviour belong in MBLINK.
- Do not duplicate LINK protocol/application logic in C, Objective-C, Swift or platform-specific shells.
- Treat undocumented Mercedes identifiers and requests as experimental until supported by traceable evidence.
- Preserve deny-by-default transmit policy: adding a decoder or definition does not grant request permission.

## Languages and boundaries

Use C/C++ for first-party portable/native implementation where suitable; neither is preferred over the other by language policy. Swift/Objective-C remain legitimate Apple platform boundaries. Platform code owns transport/toolkit mechanics, not duplicate diagnostic semantics.

## Build and test

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Use the sanitizer-enabled configuration for protocol/parser changes. Real captures may be retained when useful, sanitised appropriately and legally distributable.

## Evidence discipline

Vehicle-specific claims should identify whether they come from a public standard/source, captured traffic, physical-vehicle observation or bounded inference. Unknown remains unknown. Replay evidence proves the captured path; it does not automatically prove every ECU/software/vehicle variant.

## Documentation

Use `docs/README.md` as the documentation map. Update `docs/ARCHITECTURE.md`, `docs/DESIGN.md`, `docs/DECISIONS.md`, `docs/ROADMAP.md` and `docs/VALIDATION.md` when their contracts change. Specialist Mercedes research remains in the domain-specific documents.

## Repository policy

Development and release authority are on `main`. Published tags/releases are immutable exact-source identities. Keep commits focused and dependency pins explicit.

Participation standards remain in [.github/CODE_OF_CONDUCT.md](.github/CODE_OF_CONDUCT.md).