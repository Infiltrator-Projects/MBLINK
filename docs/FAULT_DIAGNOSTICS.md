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
- MBLINK now has a module-scoped Mercedes KWP2000 definition table with provenance; the vehicle-captured ORC `9B51` record resolves to the driver seat-belt buckle circuit description while preserving raw status `E0`;
- Mercedes UDS `19 02 FF` retrieves raw 24-bit DTC values plus their status byte;
- OBD freeze-frame request/decoding and readiness decoding primitives exist in LINK.

The remaining major gaps in this vertical slice are explicit:

1. complete the Faults UI refactor so scan-not-run, failed, clean and faults-present states are visually distinct and rich fault cards consume the structured model directly;
2. integrate capability-driven freeze-frame and readiness context into the fault workflow instead of leaving those decoders as dormant primitives;
3. expand the initial evidence-backed Mercedes manufacturer table beyond the verified ORC record, including CRD3/OM651 definitions, without inventing proprietary meanings;
4. continue maintaining and expanding standards-backed generic diagnostic knowledge in LINK.

This remains a completion track, not optional polish. Additional dashboard work is not a substitute for these remaining items.

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

The pinned LINK layer provides the generic code/title/system/category/origin/source portion of this shape. State comes from the scan kind. Freeze-frame/readiness association remains the next shared-flow step.

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

## Freeze-frame and readiness are diagnostic features

The existing OBD freeze-frame and readiness decoders must not remain dormant library features.

The fault workflow should collect and expose standards-defined freeze-frame values when the ECU supports them, including useful values such as RPM, vehicle speed, load, coolant temperature, MAP/MAF and other supported PIDs. Readiness state should be visible as diagnostic information rather than hidden protocol state.

The exact set must be capability-driven: unsupported values remain unavailable rather than being invented.

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
- MBLINK contains a tested Mercedes-specific DTC knowledge mechanism with provenance — **implemented for the initial ORC definition; catalogue expansion continues**;
- fault records can carry resolved description/category/module information without losing raw data — **generic structured records implemented; manufacturer/module enrichment continues**;
- the app distinguishes not-scanned, failed, clean and faults-present states — **controller state exists; Faults UI correction remains**;
- stored, pending, permanent and Mercedes module fault records are translated for the user when definitions are known — **generic OBD and the initial module-scoped KWP definition are implemented; Mercedes coverage remains incomplete**;
- freeze-frame and readiness information are integrated into the diagnostic workflow and visible to the user — **not yet complete**;
- unknown codes remain explicit and evidence-preserving — **implemented for the generic LINK path**;
- the same diagnostic model can be consumed by all applicable platform faces without reimplementing lookup logic in SwiftUI, GTK or Win32 — **shared LINK API implemented; additional platform consumers remain**.

Until these conditions are met, additional dashboard polish or additional live gauges must not be treated as a substitute for completing the diagnostic knowledge path.
