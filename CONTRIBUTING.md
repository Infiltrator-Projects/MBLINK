<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Contributing to MBLINK

MBLINK is intentionally C-first. Contributions should preserve the separation between portable diagnostic logic and platform presentation/transport code.

## Engineering rules

- Write portable diagnostic/protocol behaviour in C11 wherever practical.
- Do not duplicate protocol logic in Swift or Objective-C.
- Reuse Infiltratr Common when an existing shared API matches the requirement.
- Do not modify the pinned Common source inside MBLINK; changes to shared code belong in the Infiltrator-Libraries repository first.
- Keep adapter quirks in adapter/platform providers.
- Keep manufacturer definitions separate from generic OBD-II/ISO-TP/UDS engines.
- Treat undocumented Mercedes identifiers as experimental until verified against real vehicle responses.
- Add regression fixtures/tests with parser or decoder changes.
- Keep public C types platform-neutral and document ownership/lifetime rules.

## C style and quality

The portable build targets C11 with extensions disabled. New C code should compile cleanly under the project's warning policy, including `-Wall`, `-Wextra`, `-Wpedantic`, `-Wshadow`, `-Wformat=2`, `-Wstrict-prototypes` and `-Wmissing-prototypes` where supported.

Source files should carry:

```c
// SPDX-License-Identifier: GPL-3.0-or-later
```

Prefer explicit bounds, fixed-width integer types for protocol fields, checked arithmetic where overflow matters, and narrow interfaces between modules.

## Build and test

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Protocol changes should be testable without a vehicle. Real vehicle captures may be added as sanitised test fixtures when they do not expose sensitive identifiers unnecessarily.

## Layer ownership

| Layer | Preferred implementation | Owns |
| --- | --- | --- |
| `libmblink` | C11 | ELM327, OBD-II, DTCs, scheduler, ISO-TP, UDS, Mercedes decoding, logging model |
| Transport ABI | C11 | portable provider contract |
| Apple BLE provider | Objective-C | CoreBluetooth/GATT and Apple connection lifecycle |
| iPhone UI | Swift/SwiftUI | presentation, navigation, user interaction |
| Infiltratr Common | C11 shared repository | genuinely reusable cross-project primitives |

## Commit scope

Keep commits focused and explain architectural changes in the documentation when necessary. Avoid mixing unrelated formatting, protocol changes and platform changes into one patch.

## Licence

Contributions are accepted under the project's `GPL-3.0-or-later` licence unless explicitly agreed otherwise before submission.
