# Changelog

## 0.7.224 — 2026-09-22

- Advance the exact LINK dependency to 0.15.51, including the STM32 tester/server role clarification, issue #37 DTC-clear regression coverage and exact issue-27 Cube-main CI qualification.
- Complete MBLINK issue #63's generic product facade by re-exporting LINK's typed ISO 14229-1:2020 Authentication codec for all nine 0x29 tasks and adding MBLINK-level builder/decoder regression coverage.
- Fix the publication-infrastructure regression identified against issue #24: central APT refresh is asynchronous, so verification now permits 35 minutes instead of racing the repository/Pages propagation boundary at 20 minutes.
- Keep Authentication trust, certificates, private keys and Mercedes/OEM policy evidence-gated; no security bypass or unproven manufacturer algorithm is introduced.

## 0.7.223 — 2026-09-22

- Advance the exact LINK dependency to 0.15.50, which in turn pins Infiltratr Common 1.19.22, keeping the vehicle stack on one tested shared dependency chain.
- Synchronise the iOS embedded LINK provenance with the exact `src/link` gitlink so release builds report the code they actually contain.

## 0.7.222 — 2026-09-22

- Expose LINK 0.15.49's fail-closed UDS OTA/bootloader core directly through an MBLINK facade for issue #56 instead of leaving the capability hidden in the submodule.
- Add a product-level regression proving the default configuration cannot arm programming and requires security, quiesce, authenticity and secure-boot validation.
- Document the STM32F767 integration boundary: existing LINK bxCAN transport plus target-owned inactive-slot flash, integrity/authenticity/HSM, secure-boot and protected anti-rollback storage callbacks.

## 0.7.221 — 2026-09-22

- Complete MBLINK issue #59's generic DTC snapshot/stored/extended-data surface by advancing the exact LINK dependency to 0.15.49.
- Re-export LINK's typed variable-record APIs: DID-length resolver, DID values, snapshot/stored-data record views and extended-data record views.
- Add product-level regression coverage that decodes snapshot and stored-data DID/value sequences, extended-data records and the unknown-DID rejection path rather than leaving the newer LINK capability untested behind the facade.
- Inherit LINK's fail-closed UDS OTA/bootloader core for issue #56 while keeping all active programming disabled unless an explicit target backend is provided and armed.

## 0.7.220 — 2026-09-21

- Complete issue #29's current evidence tranche without inventing Mercedes meanings: all supplied C207 fault evidence remains regression represented and D18100/50 stays explicitly unknown.
- Add a typed module-scoped 24-bit UDS DTC definition contract parallel to the existing KWP contract so future proven raw UDS meanings have a safe destination.
- Add eight primary-documented CDID3/OM651 records from Mercedes-Benz/Daimler XENTRY bulletins, preserving CRD3/CRD3NFZ controller and vehicle-family applicability rather than promoting Sprinter, W212 hybrid or 117/176/246 meanings into C207.
- Keep the existing 299-row supplied reference corpus unchanged and separate from automatic wire-level resolution.
- Advance the shared LINK dependency to 0.15.42 with generic AES-CMAC and algorithm-neutral SecurityAccess support; no Mercedes seed/key algorithm is inferred or enabled.

## 0.7.219 — 2026-09-21

- Complete CI diagnostics issue #24 by restoring one canonical 0.7.219 version across VERSION, C header metadata and all iOS MARKETING_VERSION configurations.
- Pin LINK 0.15.41 exactly and record the matching LINK source revision in both iOS core configurations.
- Consume LINK issue #25's product-neutral UDS server policy metadata and contextual physical/functional dispatcher without adding any Mercedes-specific policy guesses.
- Requalify the complete Linux, Windows, sanitizer, Apple/iOS and publication pipeline from a clean preflight state.

## 0.7.218 — 2026-09-21

- Advance to LINK 0.15.40 so MBLINK's Linux, Windows where applicable, and iOS About surfaces share the completed suite-wide About contract.
- Retain the previously separated build identity and product-specific credits while inheriting the corrected shared Windows and standard SwiftUI metadata handling.


## 0.7.217 — 2026-09-21

- Standardise Linux and iOS About presentation on the suite-wide System Monitor contract through LINK 0.15.38.
- Separate the canonical build label from descriptive text so About exposes the same product/version/description/build hierarchy as the rest of the suite.
- Remove product-tagline duplication from About while retaining MBLINK branding in the application shell.


This file records user-visible, compatibility, diagnostic-knowledge and validation changes for MBLINK.

## Unreleased

- No unreleased changes.

## 0.7.216 — 2026-09-21

- Exposed LINK's forensic 27-service UDS catalogue and complete requested ReadDTCInformation report catalogue through MBLINK without duplicating generic protocol ownership.
- Advanced the exact LINK dependency to 0.15.38, including request-aware ReadDTCInformation response validation for MemorySelection, record-number, functional-group and applicable DTC echoes.
- Re-exported the request-aware ReadDTC decoder through the MBLINK compatibility facade and retained typed fixed-record views while leaving OEM-sized snapshot and extended-data tails raw.
- Preserved deny-by-default vehicle policy: ClearDiagnosticInformation 0x14 remains state-changing and unavailable to automatic discovery; Authentication 0x29 remains security-gated and has no inferred Mercedes procedure.
- Added forensic issue documentation for the supplied STM32 server deviations that caused misleading 0x19 differential results.

## 0.7.215 — 2026-09-21

- Completed a direct Common 1.19.20 forensic reuse pass in MBLINK without changing Common and without changing MBLINK's LINK dependency.
- Replaced the embedded STM32 console's private ASCII lowercase helper with Common's deterministic ASCII case conversion while preserving the console's deliberately narrow space/tab trimming grammar.
- Replaced manual guarded unsigned timestamp subtraction in Mercedes signal-correlation lag handling with Common's checked uint64 subtraction contract.
- Deliberately retained Mercedes-specific printable-text validation, variable-width decoding, RGB presentation extraction and other local code where Common does not provide an exact semantic improvement.

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
