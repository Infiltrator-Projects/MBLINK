<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Infiltratr Common reuse in MBLINK

MBLINK consumes the released Infiltratr Common submodule as its generic portable C foundation. Shared code is reused when the existing contract genuinely matches; vehicle-diagnostics behaviour remains owned by MBLINK.

The 0.7.3 baseline pins Infiltratr Common 1.8.0 at exact release commit `318b1babc7343403ae5e222ea01235a0fc84d752`. CMake validates both the release version and gitlink commit so a mismatched Common checkout fails configuration rather than silently changing the shared contract.

Current direct reuse includes project metadata, bounded string helpers, deterministic string comparison, strict parsing, checked/saturating arithmetic, array sizing, periodic-deadline advancement and presentation-safe scalar formatting. The protocol-neutral diagnostic parameter layer uses Common's scalar formatter so precision, clamping and unavailable-value rendering are not reimplemented in Swift, Objective-C or MBLINK C. The scheduler uses Common's integer deadline helper to skip missed occurrences without cadence drift or overflow.

Common's configuration parser is the preferred implementation when MBLINK adds portable `key=value` settings. Generic arithmetic belongs in Common; exact ELM327, scheduler selection, ISO-TP and UDS timing/state semantics remain in MBLINK because those contracts are protocol-specific.

The POSIX provider is not part of the iPhone/core footprint. Linux may consume it only when a real Linux provider requirement matches its existing contract.

MBLINK does not modify the submodule. Common changes are made and released in the Common repository, then MBLINK deliberately advances its exact release pin.
