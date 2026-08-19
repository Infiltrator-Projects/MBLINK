<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Contributing to MBLINK

MBLINK is C-first. Contributions must preserve the separation between portable diagnostic logic and platform transport/presentation code.

## Engineering rules

- Write portable diagnostic/protocol behaviour in C11 wherever practical.
- Do not duplicate protocol logic in Swift or Objective-C.
- Reuse Infiltratr Common when an existing shared API matches the requirement; do not modify the pinned submodule from this repository.
- Keep adapter quirks in providers/profiles and manufacturer definitions above generic OBD-II/ISO-TP/UDS layers.
- Treat undocumented manufacturer identifiers as experimental until verified against real vehicle responses.
- Add deterministic regression coverage with parser, decoder or state-machine changes.
- Keep public C types platform-neutral and make ownership/lifetime contracts explicit.

## C style and quality

C11 extensions are disabled. Code must compile cleanly under the repository warning policy (`-Wall`, `-Wextra`, `-Wpedantic`, `-Wshadow`, `-Wformat=2`, `-Wstrict-prototypes`, `-Wmissing-prototypes` where supported).

Source files carry `// SPDX-License-Identifier: GPL-3.0-or-later`.

Prefer explicit bounds, fixed-width protocol fields, checked/saturating arithmetic where overflow matters, and narrow interfaces. Comments explain ownership, invariants, protocol quirks, safety or non-obvious decisions; do not narrate obvious syntax.

## Build and test

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

For sanitizer validation:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DMBLINK_ENABLE_SANITIZERS=ON
cmake --build build-sanitized
ctest --test-dir build-sanitized --output-on-failure
```

Protocol behaviour should be testable without a vehicle. Sanitised real captures may be added when useful and when they do not expose sensitive identifiers unnecessarily.

## Layer ownership

| Layer | Implementation | Owns |
| --- | --- | --- |
| `libmblink` | C11 | ELM327, OBD-II, scheduler, telemetry, ISO-TP, UDS, manufacturer decoding |
| Transport ABI | C11 | portable provider contract |
| Apple provider/bridge | Objective-C | CoreBluetooth and Apple application boundary |
| iPhone UI | Swift/SwiftUI | presentation and user interaction |
| Infiltratr Common | shared C11 repository | genuinely reusable cross-project primitives |

## Commits

Keep commits focused. Do not mix unrelated formatting, protocol changes and platform changes. Update documentation only when it owns a contract affected by the change.

## Licence

Contributions are accepted under `GPL-3.0-or-later` unless explicitly agreed otherwise beforehand.
