<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Fault diagnostics and diagnostic knowledge

Fault diagnosis is a first-class MBLINK product function. A scan is **not complete** merely because the ECU returned a DTC and MBLINK displayed its hexadecimal or SAE code.

The product requirement is to answer, as far as defensible data permits:

1. **What code did the ECU report?**
2. **What does that code mean?**
3. **Which module/system/component does it concern?**
4. **What state is the fault in?**
5. **What diagnostic context was captured with it?**
6. **What related vehicle data helps an operator investigate it?**

Live dashboards support diagnosis; they do not replace fault diagnosis.

## Non-negotiable behaviour

A user-facing fault result must not stop at `Pxxxx`, `Cxxxx`, `Bxxxx`, `Uxxxx` or a six-digit UDS DTC value when a trustworthy definition is available.

The diagnostic model must support, where applicable:

- human-readable DTC title/description;
- generic SAE/ISO versus manufacturer-specific classification;
- originating ECU/module;
- system/subsystem/component category;
- stored, pending, permanent and/or UDS status semantics;
- MIL/warning-indicator relevance where exposed by the protocol;
- standards-defined freeze-frame information associated with a fault;
- OBD readiness/monitor information relevant to diagnosis;
- raw code and raw status bytes for evidence/auditability;
- provenance for manufacturer-specific definitions;
- related measurements that help investigate the fault without pretending that correlation is a proven cause;
- diagnostic notes or likely causes only where they come from defensible documentation/data, never from invented heuristics presented as fact.

A successful scan with zero faults, a scan that has not run, and a scan that failed are three different states. The UI must never present an unperformed or failed scan as a reassuring green `0`/`None reported` result.

## Ownership

### LINK owns shared diagnostic knowledge

Anything that is manufacturer-neutral belongs in LINK so MBLINK and JAGLINK cannot diverge. LINK is the source of truth for:

- standards-defined OBD-II DTC decoding and lookup metadata;
- generic DTC classification across powertrain/chassis/body/network code families where the standard defines it;
- shared DTC structures capable of carrying code, description, category and provenance/source class;
- OBD stored/pending/permanent semantics;
- OBD freeze-frame request/decoding and diagnostic-context structures;
- OBD readiness/monitor decoding and presentation-neutral meaning;
- UDS DTC status-bit interpretation;
- generic fault correlation structures and portable APIs;
- tests proving that raw ECU data is translated transactionally and deterministically.

The portable layer must preserve the raw code even when no description is known. An unknown code is a valid diagnostic result, not permission to fabricate a description.

### MBLINK owns Mercedes diagnostic knowledge

Mercedes-specific information belongs in MBLINK. This includes:

- Mercedes-Benz manufacturer DTC definitions;
- Delphi CRD3/CDID3 and OM651-specific DTC definitions;
- Mercedes module/ECU associations;
- Mercedes-specific component/system descriptions;
- Mercedes-specific subcodes/status interpretation when supported by evidence;
- vehicle/profile applicability such as C207/W207 + OM651 + CRD3.x;
- source/provenance and vehicle-verification status for each manufacturer definition;
- Mercedes-specific diagnostic relationships between fault records and verified live parameters.

MBLINK must not copy generic SAE/ISO tables out of LINK. LINK must not contain Mercedes-only definitions.

## Current implementation state

The generic diagnostic-knowledge layer is implemented in the exact LINK revision pinned by MBLINK's `src/link` gitlink. This is materially beyond the former raw-code-only path:

- standard OBD services 03/07/0A retrieve stored, pending and permanent DTCs without altering the vehicle;
- raw DTC bytes are preserved and decoded to the five-character SAE-style code;
- LINK resolves valid generic codes into a presentation-neutral `LinkDtcKnowledge` record containing normalized code, known/unknown state, system, generic/manufacturer origin, source class, title and category;
- the shared catalogue covers high-value engine/diesel and network diagnostic areas and is maintained in LINK rather than copied into MBLINK;
- structured per-cylinder injector, misfire and glow-plug families are generated deterministically;
- valid but unmapped manufacturer-specific codes remain explicit unknown diagnostic records rather than receiving fabricated meanings;
- ISO 14229 status-byte translation is shared in LINK rather than being a manufacturer-specific concept;
- MBLINK exposes resolved `DiagnosticFault` objects to the iPhone model and feeds `CODE — description` into the existing Faults presentation for compatibility;
- MBLINK has product-level regression coverage proving the shared LINK knowledge and compatibility APIs reach the Mercedes product face;
- MBLINK has a module-scoped Mercedes KWP2000 definition table with provenance; the vehicle-captured ORC `9B51` record resolves to the driver seat-belt buckle circuit description while preserving raw status `E0`;
- MBLINK also contains a separate **299-row source-scoped Mercedes reference catalogue** reconstructed from the diagnostic material supplied to the project: 78 BenzWorld body/chassis rows, 104 Mercedes P1xxx reference rows, all 38 TaT teaser rows including explicit subcodes/duplicates, and 79 supplied SprinterManual reference rows. Duplicate meanings are retained deliberately instead of one source silently overwriting another;
- standard OBD manufacturer-specific codes that LINK correctly classifies as unknown can now be enriched on both iPhone and Linux from that Mercedes reference catalogue when the meaning is unambiguous. Conflicting codes such as `B1000` or `P2004` are shown as ambiguous and require module/subcode context rather than receiving an arbitrary meaning;
- the broad reference catalogue does **not** override standards-defined LINK meanings and does not promote source-scoped Sprinter/legacy rows into C207-specific KWP/UDS definitions;
- Mercedes UDS `19 02 FF` retrieves raw 24-bit DTC values plus their status byte;
- OBD freeze-frame request/decoding and readiness decoding primitives exist in LINK.

The broad fault-investigation vertical slice is now functionally complete. Ongoing work is manufacturer knowledge depth rather than missing scan-state/readiness/freeze-frame plumbing:

1. the Faults UI distinguishes scan-not-run/in-progress, failed/incomplete, verified-clean and faults-present states, and standard OBD fault cards consume structured diagnostic knowledge. A regression-tested presentation-state classifier prevents an empty unrun or failed scan from becoming a clean result;
2. readiness and capability-driven Mode 02 frame-zero freeze-frame context are integrated into the shared fault workflow and surfaced separately from current live data;
3. Mercedes knowledge is deliberately split: exact KWP/UDS definitions remain module-scoped and evidence-gated, while the supplied five-character reference corpus is searchable with source/applicability metadata and explicit ambiguity. Additional CRD3/OM651 wire-level mappings remain evidence-gated rather than being inferred from a generic reference row;
4. standards-backed generic diagnostic knowledge continues to be maintained in LINK.

Additional manufacturer definitions remain important, but they are catalogue expansion rather than a missing end-to-end fault workflow.

## Required standard OBD fault workflow

For each successful standard OBD fault scan, MBLINK should be able to construct a presentation-neutral record equivalent to:

```text
Code:        P0401
Title:       Exhaust Gas Recirculation Flow Insufficient Detected
Class:       SAE/ISO generic powertrain fault
State:       Confirmed / stored
System:      Engine -> emissions -> EGR
Module:      Standard OBD powertrain responder
Raw code:    preserved
Context:     freeze-frame/readiness values when available
Source:      standards-backed definition
```

The pinned LINK layer provides the generic code/title/system/category/origin/source portion of this shape. State comes from the scan kind. Readiness and capability-gated Mode 02 frame-zero context are integrated into the shared diagnostic flow and surfaced separately from current live data.

## Required Mercedes fault workflow

For each Mercedes UDS DTC, MBLINK should retain the raw 24-bit value and status while resolving all information supported by the Mercedes knowledge base:

```text
DTC:         raw Mercedes/UDS value
Title:       Mercedes fault description when known
Module:      CDI / CRD3.x engine control (or the actual reporting ECU)
Vehicle:     applicable Mercedes profile(s)
Status:      decoded ISO 14229 state bits
System:      verified Mercedes component/subsystem category
Context:     associated diagnostic measurements when available
Provenance:  source-corroborated / vehicle-verified definition evidence
```

A positive UDS DTC response without a known manufacturer definition must still be shown honestly as an unknown Mercedes DTC with raw evidence and decoded status.

### Evidence and applicability contract

Mercedes definitions are module scoped and retain protocol, module/ECU family,
vehicle family and engine-family applicability as structured fields. Evidence
sources are retained individually and classified as `community-observation`,
`specialist-corroborated`, `primary-documented` or `repair-verified`. Each
source also states whether it supports the textual meaning or proves only that
the raw record occurred. A vehicle/community capture therefore cannot promote
its own guessed description.

Unknown KWP and UDS records retain the raw code and raw status and expose
external research starting points. Those links are explicitly non-authoritative
for meaning: the result remains **unknown** until applicable evidence supports
a module-scoped definition. The captured `0x602 -> 0x480` UDS record
`D18100 / 50` is the regression example for this behaviour; MBLINK assigns no
meaning or exact ECU identity to it.

## Freeze-frame and readiness are diagnostic features

The OBD freeze-frame and readiness decoders are now active diagnostic-flow features.

After standard stored/pending/permanent fault inventory, LINK attempts Mode 01 PID 01 readiness and retains the standards-defined monitor state. When a stored SAE fault exists, LINK then attempts a bounded capability-gated set of Mode 02 frame-zero values such as load, coolant temperature, MAP, RPM, speed, intake temperature, MAF and throttle when the ECU advertised the corresponding PID family. MBLINK presents those values as captured fault context, never as current Live Data.

Unsupported, malformed or absent values remain unavailable rather than being invented. Raw requests/responses remain in the evidence trace, while Linux investigation JSON also carries the structured readiness/freeze-frame snapshot.

## Archived official-app DTC model evidence

The archived Mercedes me Adapter application binary supplied for interoperability
analysis contains a concrete diagnostic object model in `libdiaglogic.so`.
Recovered symbol/protobuf identities include `diaglogic.api.Dtc`,
`diaglogic.api.DtcCollection`, DTC flags named `STATIC` and `SPORADIC`,
and the application data identifier `storedObdDtcs`. The same binary exposes
fault/warning state identifiers including `brakeControlWarnings`,
`irregularObdResponse` and numerous lamp/brake/tire fault states.

That is useful evidence that the original product distinguished DTC collections
and fault state instead of treating faults as plain text. It does **not** prove
how a Mercedes KWP/UDS raw status byte maps to `STATIC` or `SPORADIC`.
MBLINK therefore keeps raw status bytes authoritative and does not label
`0xE0`, or any other raw status value, as static/sporadic until a defensible
wire-level mapping is recovered.

## UI requirement

The Faults workspace is expected to become an investigation surface, not a raw-code list. At minimum each fault card/row should expose:

- code and human-readable title;
- source/module;
- stored/pending/permanent/UDS status;
- subsystem/category;
- captured context when available;
- an explicit unknown-definition state when no trustworthy lookup exists.

Raw evidence remains exportable separately.

The UI may correlate a fault with current/freeze-frame measurements, but must distinguish **observed data**, **documented diagnostic information** and **inference**.

## Completion criteria

Fault diagnostics are not considered feature-complete until all of the following are true:

- LINK contains a tested standards-backed generic DTC knowledge mechanism rather than code formatting alone — **implemented; catalogue maintenance continues**;
- MBLINK contains tested Mercedes-specific DTC knowledge with applicability and provenance — **implemented as both the exact module-scoped KWP/UDS layer and the 299-row source-scoped reference layer; additional ECU-specific wire mappings remain evidence-gated**;
- fault records can carry resolved description/category/module information without losing raw data — **generic structured records implemented; manufacturer/module enrichment continues**;
- the app distinguishes not-scanned, failed, clean and faults-present states — **implemented on iPhone and Linux fault-investigation surfaces, with product regression coverage proving that unrun/failed empty scans cannot become clean**;
- stored, pending, permanent and Mercedes module fault records are translated for the user when definitions are known — **generic OBD and the initial module-scoped KWP definition are implemented; Mercedes coverage remains incomplete**;
- freeze-frame and readiness information are integrated into the diagnostic workflow and visible to the user — **implemented in the shared flow and surfaced on iPhone/Linux; physical ECU availability remains capability-dependent**;
- unknown codes remain explicit and evidence-preserving — **implemented for the generic LINK path**;
- the same diagnostic model can be consumed by all applicable platform faces without reimplementing lookup logic in SwiftUI, GTK or Win32 — **shared LINK API implemented; additional platform consumers remain**.

Until these conditions are met, additional dashboard polish or additional live gauges must not be treated as a substitute for completing the diagnostic knowledge path.
