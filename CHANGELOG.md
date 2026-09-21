# Changelog

This file records user-visible, compatibility, diagnostic-knowledge and validation changes for MBLINK.

## Unreleased

- No unreleased changes.

## 0.7.213 — 2026-09-21

- Advanced the exact shared engine to LINK 0.15.34 while retaining Infiltratr Common 1.19.20 unchanged.
- Reused Common's canonical monotonic clock in the native Linux MBLINK face instead of retaining a second GLib time wrapper.
- Removed the remaining locale-sensitive ctype use from the C207 replay command normaliser and DID-lab byte parser in favour of Common's deterministic ASCII contracts.
- Removed an obsolete ctype include from Mercedes data-scan code; Mercedes diagnostic semantics and evidence remain MBLINK-owned.
- No code was added to Common.

## 0.7.212 — 2026-09-21

- Advanced the exact shared engine to LINK 0.15.33 and therefore Infiltratr Common 1.19.20.
- Removed Mercedes-local ASCII case-folding/classification helpers where Common now provides the exact locale-independent contract for VIN, ECU identity, CRD3 family and DTC-reference text.
- Moved Linux preference path resolution and directory creation onto Common's XDG/POSIX helpers while retaining the MBLINK-owned INI schema.
- Publishes Linux preference bytes through Common's durable atomic-file writer instead of GTK/GLib filesystem policy; existing file permissions are preserved.
- Kept Mercedes diagnostic semantics, manufacturer evidence and MBLINK presentation policy local; no code was added to Common.

## 0.7.211 — 2026-09-20

- Advanced the exact shared engine to LINK 0.15.32, retaining the Vehicle Research implementation and Infiltratr Common 1.19.10.
- Published the repository-wide 2000-2026 copyright normalization already present on main.
- No diagnostic protocol, transport, safety, Mercedes evidence, polling or user-facing research behaviour changed from 0.7.210.

## 0.7.210 — 2026-09-20

- Advanced the exact shared automotive engine to LINK 0.15.31 while retaining Infiltratr Common 1.19.10.
- Pulled LINK's portable Vehicle Research session state and typed research evidence timeline into MBLINK without duplicating the generic implementation.
- Upgraded MBLINK Discover on Windows with shared passive-capture, standards-inventory and Mercedes FULL SWEEP phase tracking, operator event markers and Vehicle Research JSONL export summaries.
- Preserved the existing Mercedes-owned sweep plan and deny-by-default write/control boundary; the research workflow remains read-oriented.

## 0.7.209 — 2026-09-20

- Advanced the exact shared automotive engine to LINK 0.15.30 and therefore Infiltratr Common 1.19.10.
- Replaced the duplicated Linux Night palette, shared typography names/weights and matching design metrics in MBLINK's GTK face with Common 1.19.10's canonical native design contract while preserving the exact visible Night appearance.
- Kept only genuinely MBLINK-specific cockpit/trace gradients and Mercedes presentation details local; Common now supplies the product-neutral titlebar, connection, heading, summary, kicker, detail, note, status, accent and state-border roles.
- Reused Common's canonical MB Corpo CMake provenance for archive/file names and hashes instead of maintaining a second CMake copy of that metadata.
- Preserved the distinct iPhone and Windows product presentation contracts rather than mechanically replacing non-equivalent styling.

## 0.7.208 — 2026-09-19

- Advanced the exact shared engine to LINK 0.15.29 while retaining Infiltratr Common 1.19.8.
- Completed the MBLINK-side forensic reuse pass: Linux replay parsing, canonical array sizing, bounded string copies, checked timestamp arithmetic, Mercedes UDS endian access and embedded array sizing now use the corresponding Common contracts where those contracts exactly match the existing behaviour.
- Switched the Apple standard OBD-II scalar text path to LINK's shared deterministic formatter so iPhone and Linux no longer reconstruct the same unit/precision policy independently.
- Left Mercedes-specific variable-width decoding, CAN identifier composition, protocol state and evidence logic in MBLINK where Common/LINK do not provide a semantically equivalent contract.

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
