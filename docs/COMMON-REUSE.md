<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Infiltratr Common reuse in MBLINK

MBLINK consumes the released Infiltratr Common submodule as its generic portable C foundation. Shared code is reused when the existing contract genuinely matches; vehicle-diagnostics behaviour remains owned by MBLINK.

Current direct reuse includes project metadata, bounded string helpers, deterministic string comparison, strict parsing, checked/saturating arithmetic, array sizing and presentation-safe scalar formatting. The protocol-neutral diagnostic parameter layer uses Common's scalar formatter so precision, clamping and unavailable-value rendering are not reimplemented in Swift, Objective-C or MBLINK C.

Common's configuration parser is the preferred implementation when MBLINK adds portable `key=value` settings. The interval-due helper is useful for ordinary refresh policy, but exact ELM327, scheduler, ISO-TP and UDS timing/state semantics remain in MBLINK because those contracts are protocol-specific.

The POSIX provider is not part of the iPhone/core footprint. Linux may consume it only when a real Linux provider requirement matches its existing contract.

MBLINK does not modify the submodule. Common changes are made and released in the Common repository, then MBLINK deliberately advances its exact release pin.
