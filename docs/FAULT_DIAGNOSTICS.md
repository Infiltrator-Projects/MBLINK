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

## Current implementation gap (0.7.26)

As of source version 0.7.26, the low-level acquisition path is materially ahead of the diagnostic-knowledge layer:

- standard OBD services 03/07/0A retrieve stored, pending and permanent DTCs;
- the generic decoder converts raw DTC bytes to the five-character SAE-style code;
- Mercedes UDS `19 02 FF` retrieves 24-bit DTC values and their ISO 14229 status byte;
- the iPhone UI interprets Mercedes status bits such as pending/confirmed/test-failed;
- OBD freeze-frame request/decoding primitives exist in LINK;
- OBD readiness decoding exists in LINK.

However, the current product does **not yet provide a comprehensive DTC description/knowledge database**, and the live diagnostic flow does not yet make full freeze-frame/readiness context part of each fault investigation. This is a major incomplete product area, not an optional polish item.

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

The example illustrates the required shape; the actual database must be sourced and tested rather than filled from guesses.

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

- LINK contains a tested standards-backed generic DTC knowledge mechanism rather than code formatting alone;
- MBLINK contains a tested Mercedes-specific DTC knowledge mechanism with provenance;
- fault records can carry resolved description/category/module information without losing raw data;
- the app distinguishes not-scanned, failed, clean and faults-present states;
- stored, pending, permanent and Mercedes UDS fault records are translated for the user when definitions are known;
- freeze-frame and readiness information are integrated into the diagnostic workflow and visible to the user;
- unknown codes remain explicit and evidence-preserving;
- the same diagnostic model can be consumed by all applicable platform faces without reimplementing lookup logic in SwiftUI, GTK or Win32.

Until these conditions are met, additional dashboard polish or additional live gauges must not be treated as a substitute for completing the diagnostic knowledge path.
