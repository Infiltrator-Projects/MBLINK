# Changelog

This file records user-visible, compatibility, diagnostic-knowledge and validation changes for MBLINK.

## Unreleased

- No unreleased changes.

## 0.7.207 — 2026-09-19

- Upgraded the exact shared engine dependency to LINK 0.15.28 and Infiltratr Common 1.19.8.
- Replaced Linux product-local OBD-II value, decoded-PID and fuel-economy formatting with LINK's shared preference-aware presentation contracts.
- Reused Common's strict ranged integer parser, checked dynamic-array growth helper and canonical array-length primitive in MBLINK-owned code.
- Moved the Mercedes identity-first module scanner out of multi-thousand-line public inline headers into a normal MBLINK translation unit while retaining the existing API and state-machine behaviour.
- Kept the portable diagnostic core C11-first; no C++ runtime or duplicate OO layer was introduced.


## 0.7.206 — 2026-09-19

- Replaced the Linux Dashboard navigation artwork with a high-contrast four-panel glyph that remains legible at sidebar size.

## 0.7.205 — 2026-09-19

- Fixed DID-lab unsigned parsing so negative timestamps and tolerances are rejected, and made CSV floating-point parsing use Common's strict finite parser.
- Removed signed overflow from extreme signal-correlation lag stepping and added boundary regression coverage.
- Reused Common's alignment-safe endian readers in Mercedes transmission, DiagLogic and data-scan decoding.
- Upgraded the exact shared engine dependency to LINK 0.15.25, which in turn pins Infiltratr Common 1.19.3.
- Linux live-data profiles now start with every real-vehicle channel OFF and store standard selections per VIN rather than globally.
- Linux Dashboard, Graphs and Table now follow the same explicit selected-channel model; selected graph channels remain visible while awaiting their first sample.
- Removed the old fixed eight-graph product limit and use LINK's runtime-selected graph configuration.
- Reused LINK's shared ELM327 CAN address formatting in Mercedes scanners instead of maintaining parallel ATSH/ATCRA formatters.
- Canonical documentation baseline aligned with the Infiltrator project family.

## Policy

Record new supported diagnostic behaviour, changed safety/permission boundaries, corrected definitions, platform changes, dependency revisions that alter behaviour and material fixes. Raw research activity belongs in the relevant evidence document until it changes supported product behaviour.

## Historical identity

Git tags and GitHub Releases remain authoritative for exact historical source and dependency identity.
