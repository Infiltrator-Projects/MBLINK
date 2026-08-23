<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes-Benz diagnostics

Mercedes support sits above the generic ELM327, ISO-TP and UDS layers. The manufacturer layer owns vehicle profiles, ECU endpoint provenance and Mercedes-specific evidence/definitions; it does not duplicate transport framing or UDS response validation. Endpoint profiles use the transport-neutral ISO-TP address contract rather than an adapter-specific configuration type.

## C207 E 250 CDI / OM651 / Delphi CRD3.x profile

The development profile contains one source-corroborated engine endpoint:

| Key | Request ID | Response ID | Status |
| --- | ---: | ---: | --- |
| `c207-om651-engine-eobd-11bit` | `7E0` | `7E8` | source-corroborated |

The ECU-family side has direct fitment evidence: the public `autodiag2/database` W207 E 250 2200 CDI record identifies Delphi `CRD3.x` for the 148 kW and 150 kW configurations. Independent public Mercedes CRD3 diagnostic material also corroborates the conventional engine request/response pair. That grounds MBLINK's C207/OM651 work in the correct ECU family and justifies source-corroborated status while still keeping vehicle verification gated on a reproducible C207 capture.

Physical hardware is a validation gate, not a development gate. MBLINK continues to build the Mercedes protocol model, fixture replay, ECU-family decoding, fault knowledge and safe read-only requests offline. When the development adapter becomes available, the real C207 capture promotes only what it actually proves rather than beginning the implementation from scratch.

## Read-only ECU evidence probe

`MblinkMercedesEcuProbe` owns the complete bounded engine-evidence sequence:

1. configure the ELM transmit header and receive address;
2. enable CAN automatic formatting and automatic flow control;
3. send UDS TesterPresent with a positive response requested (`3E00`);
4. if TesterPresent succeeds, request standardized VIN DID `F190` (`22F190`);
5. perform a bounded read-only standardized ECU-identification sweep;
6. perform a second bounded read-only CRD3 fingerprint sweep using publicly documented identifiers from the open-source CaesarSuite `Simulated_CRD3` model;
7. issue read-only UDS `ReadDTCInformation` `19 02 FF` while the Mercedes physical ECU channel is still selected;
8. retain every raw ELM327 command/response in the session evidence transcript;
9. restore the normal ELM/OBD-II configuration before generic polling resumes.

The standardized identity sweep currently requests:

| DID | Standard identity purpose |
| --- | --- |
| `F18C` | ECU serial number |
| `F187` | Vehicle manufacturer spare-part number |
| `F188` | Vehicle manufacturer ECU software number |
| `F189` | Vehicle manufacturer ECU software version |
| `F191` | Vehicle manufacturer ECU hardware number |
| `F197` | System name / engine type |

The CRD3 fingerprint sweep requests these evidence-only identifiers:

| DID | CaesarSuite CRD3 label | MBLINK treatment |
| --- | --- | --- |
| `F100` | Session / variant | ECU-family evidence |
| `F154` | Supplier identifier | ECU-family evidence |
| `F196` | EROTAN | raw ECU-family evidence |
| `1001` | Full variant coding | raw ECU-family evidence |
| `1002` | Partial variant coding | raw ECU-family evidence |

The CRD3 identifiers are not treated as DPF, rail-pressure, injector, EGR or turbo parameters. Their purpose is to identify the responding engine-control family and preserve exact raw responses for later fixture promotion. MBLINK does not infer a live-data formula from a positive fingerprint response.

## OM651 / CDID3 family signature

The portable CRD3 decoder understands the payload shape published by the CaesarSuite CRD3 simulator for `F100` and `F154`. `F100` carries gateway mode, a big-endian 16-bit ECU variant and the active session; `F154` carries a one-byte supplier identifier.

Independent OM651/CDID3 diagnostic material corroborates the signature `02 21 31` with Delphi as the supplier. MBLINK therefore recognises gateway mode `02`, variant `2131`, supplier `64`/Delphi as an **OM651/CDID3-family signature**. The live probe decodes those fields instead of merely recording positive-response masks. That remains family evidence, not automatic C207 vehicle verification.

## Mercedes UDS fault reading

`uds_dtc.h` provides the portable read-only ISO 14229 `ReadDTCInformation` codec. The request is `19 02 FF`: report DTCs by status mask with every status bit requested. Positive `59 02` responses are decoded into 24-bit UDS DTC values plus their ISO 14229 status byte; negative responses retain their NRC.

This request is part of the same `MblinkMercedesEcuProbe` used by the native iPhone connection path. It executes after the CRD3 fingerprint and before MBLINK resets the ELM adapter back to generic OBD-II. The iPhone Faults screen therefore has a distinct Mercedes UDS section in addition to the standard stored/pending/permanent OBD-II sections.

No-response, negative-response and malformed Mercedes fault replies are classified explicitly. MBLINK must not turn any of those states into a misleading empty-fault result. It does not clear faults, alter DTC settings, enter an extended session, perform security access, write data or invoke routines.

`MblinkMercedesEngineScan` remains as a compatibility/high-level wrapper around the same portable probe. It no longer issues a duplicate second DTC request.

## Mercedes fault knowledge is mandatory

Retrieving a 24-bit UDS DTC and displaying a six-digit hexadecimal value plus `status 0xNN` is only the acquisition layer. It is **not** considered complete Mercedes fault support.

MBLINK must maintain an evidence-backed Mercedes diagnostic knowledge layer able to resolve, where trustworthy information exists:

- human-readable Mercedes-Benz DTC title/description;
- originating ECU/module;
- Mercedes subsystem/component category;
- applicable vehicle/engine/ECU family, including CRD3/CDID3 and OM651 where relevant;
- decoded ISO 14229 status semantics;
- manufacturer-specific subcode/status interpretation when documented;
- provenance/source status for the definition;
- links to related verified diagnostic measurements where useful to investigation.

The raw 24-bit DTC and status byte remain part of the record even after a description is resolved. Unknown Mercedes codes must be shown as unknown with raw evidence preserved; MBLINK must never invent a likely-sounding Mercedes description.

Generic UDS status interpretation belongs in LINK. Mercedes-specific DTC definitions and module/component meanings belong in MBLINK. The full normative requirement is in `FAULT_DIAGNOSTICS.md`.

The Faults workspace is expected to present Mercedes faults as diagnostic records, not just hexadecimal rows. A useful record includes at least the raw DTC, resolved title when known, module, status, subsystem/category and source/provenance. A successful clean scan, an unperformed scan and a failed scan must remain visibly distinct.

## Offline trace replay

The test suite contains a deterministic ELM trace-replay harness. A fixture is an ordered list of expected adapter commands and responses. The replay fails if command order changes, an unexpected request appears or a response no longer decodes through the portable layers.

The current synthetic Mercedes fixture drives the entire read-only sequence through channel configuration, TesterPresent, VIN, six standardized identity DIDs, five CRD3 fingerprint DIDs and UDS `19 02 FF`. It also verifies decoded CRD3 variant/supplier evidence and the OM651/CDID3 Delphi signature. This lets development and regression testing continue before the physical adapter arrives. Synthetic fixtures prove deterministic software behaviour only; they never count as vehicle verification.

When real evidence is available, the same harness is the promotion path: sanitize the captured exchange, commit it as a deterministic fixture, then promote only the endpoint/definitions that the capture actually proves.

Mercedes DTC knowledge should follow the same evidence discipline: definitions are test data with provenance, not UI strings scattered through SwiftUI.

## OM651 manufacturer signal catalogue

`mercedes_om651.h` defines stable identities for manufacturer-level values independently shown by OM651/CDID3 diagnostic material. Current priority groups are:

- DPF differential pressure, fill level, ash, multiple soot estimates, regeneration state and distance since regeneration;
- exhaust temperatures before the turbocharger, before the catalyst and at the DPF inlet;
- fuel rail pressure;
- boost pressure;
- EGR command/rate;
- cylinder 1–4 smooth-running corrections and injector correction factors.

Every entry is currently `corroborated-unmapped`: the capability is known to exist on OM651/CDID3 diagnostic systems, but MBLINK deliberately leaves its DID, byte layout, scaling and units unbound until defensible protocol evidence exists. This gives the program a concrete Mercedes feature model now without inventing attractive-looking numbers.

## Why proprietary DPF formulas are still unbound

Public Mercedes engine live-data mappings for OM651 are substantially weaker than standardized UDS identity material. Independent diagnostic pages show that CRD3 exposes the DPF, injector, rail, EGR and boost values MBLINK wants, while the W207 fitment database grounds the E 250 CDI in Delphi CRD3.x. Neither source, by itself, supplies enough raw request/response/scaling evidence to bind a proprietary live-data formula safely.

MBLINK therefore separates **fitment evidence**, **capability evidence**, **protocol mapping evidence** and **vehicle verification**. A DID/formula is only bound when its request, raw response layout and scaling are supportable. That is deliberately different from guessing a Mercedes DID and showing a plausible-looking value.

The same principle applies to Mercedes DTC descriptions: MBLINK should be comprehensive, but comprehensiveness must come from sourced definitions rather than fabricated interpretations.

## iPhone evidence path

The native iPhone workflow performs the full portable Mercedes sequence automatically after the standard OBD-II capability exchange: VIN, standard identity, CRD3 fingerprint, decoded CRD3 family evidence and Mercedes UDS fault memory. Vehicle and Modules expose the CRD3 result; Faults exposes the separate UDS records; Log exports the complete transcript. The adapter is then reset and ordinary OBD-II polling/fault services resume.

SwiftUI remains a view layer. Mercedes protocol state, CRD3 decoding, UDS fault parsing and Mercedes fault lookup/knowledge belong below the UI so the same evidence rules, definitions and replay tests can be used on other platforms.

## Evidence and promotion

An endpoint, DID, DTC definition or scaling rule may move to a stronger evidence status only when its provenance is defensible. Vehicle-specific promotion requires a reproducible capture tied to the relevant vehicle/ECU/adapter conditions and regression fixture where the definition depends on observed protocol behaviour.

No unverified Mercedes manufacturer live-data formula is currently promoted into the C207/OM651 profile. The work can continue without hardware; hardware is required only for final promotion from corroborated/candidate protocol definitions to exact C207/OM651 vehicle-verified mappings.
