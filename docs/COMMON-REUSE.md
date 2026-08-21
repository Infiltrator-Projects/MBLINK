<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Infiltratr Common reuse in MBLINK

MBLINK consumes the released Infiltratr Common submodule as its generic portable C foundation. Shared code is reused when the existing contract genuinely matches; vehicle-diagnostics behaviour remains owned by MBLINK.

MBLINK 0.7.4 pins Infiltratr Common 1.10.0 at exact release commit `182e64cb8b8992879e443b941565058166fe0161`. MBLINK validates both the release version and gitlink commit so a mismatched Common checkout fails configuration rather than silently changing the shared contract.

Common 1.10 owns the membership and dependency graph of its consumer build targets. CMake therefore adds the Common subdirectory and links `InfiltratrCommon::Portable`; it no longer copies Common's internal `.c` source list into MBLINK. The iPhone project likewise references Common's `apple/InfiltratrCommon.xcodeproj` and links the `InfiltratrCommonPortable` product instead of compiling Common implementation files directly. This removes the integration failure mode exposed when Common 1.9 timing acquired an arithmetic dependency while MBLINK still enumerated Common sources itself.

Current direct reuse includes project metadata, bounded string helpers, deterministic string comparison, strict parsing, checked/saturating arithmetic, array sizing, periodic-deadline advancement and presentation-safe scalar formatting. The protocol-neutral diagnostic parameter layer uses Common's scalar formatter so precision, clamping and unavailable-value rendering are not reimplemented in Swift, Objective-C or MBLINK C. The scheduler uses Common's integer deadline helper to skip missed occurrences without cadence drift or overflow.

Common's configuration parser is the preferred implementation when MBLINK adds portable `key=value` settings. Generic arithmetic belongs in Common; exact ELM327, scheduler selection, ISO-TP and UDS timing/state semantics remain in MBLINK because those contracts are protocol-specific.

The POSIX provider is not part of the MBLINK portable/iPhone footprint. Linux may consume it only when a real Linux provider requirement matches its existing contract.

MBLINK does not modify the submodule. Common changes are made and released in the Common repository, then MBLINK deliberately advances its exact release pin.
