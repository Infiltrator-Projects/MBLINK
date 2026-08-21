<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes-Benz diagnostics

Mercedes support sits above the generic ELM327, ISO-TP and UDS layers. The manufacturer layer owns vehicle profiles, ECU endpoint provenance and Mercedes-specific evidence/definitions; it does not duplicate transport framing or UDS response validation. Endpoint profiles use the transport-neutral ISO-TP address contract rather than an adapter-specific configuration type.

## C207 / OM651 development profile

The profile contains one initial engine endpoint candidate:

| Key | Request ID | Response ID | Status |
| --- | ---: | ---: | --- |
| `c207-om651-engine-eobd-11bit` | `7E0` | `7E8` | candidate |

This pair comes from the conventional 11-bit physical-address example in the ELM327 documentation, which points to ISO 15765-4. It is useful as a first read-only probe target, but it is not by itself evidence that a C207/OM651 responds at that endpoint. The definition therefore remains candidate status until a reproducible physical vehicle capture is committed as a regression fixture.

Physical hardware is a validation gate, not a development gate. MBLINK continues to build the Mercedes protocol model, fixture replay, ECU-family decoding and safe read-only requests offline. When the development adapter becomes available, the real C207 capture replaces assumptions rather than beginning the implementation from scratch.

## Read-only ECU evidence probe

`MblinkMercedesEcuProbe` coordinates the existing layers in this order:

1. configure the ELM transmit header and receive address;
2. enable CAN automatic formatting and automatic flow control;
3. send UDS TesterPresent with a positive response requested (`3E00`);
4. if TesterPresent succeeds, request standardized VIN DID `F190` (`22F190`);
5. perform a bounded read-only standardized ECU-identification sweep;
6. perform a second bounded read-only CRD3 fingerprint sweep using publicly documented identifiers from the open-source CaesarSuite `Simulated_CRD3` model;
7. retain every raw ELM327 command/response in the session evidence transcript;
8. restore the normal ELM/OBD-II configuration before live generic polling resumes.

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

Independent OM651/CDID3 diagnostic catalogues for E 250 CDI and S 250 CDI systems corroborate the signature `02 21 31` with Delphi as the supplier. MBLINK therefore recognises gateway mode `02`, variant `2131`, supplier `64`/Delphi as an **OM651/CDID3-family signature**. That is useful offline corroboration, but it does not promote the C207 endpoint or any proprietary live-data definition to vehicle-verified status.

## Mercedes UDS fault reading

`uds_dtc.h` adds a portable read-only ISO 14229 `ReadDTCInformation` codec. The initial request is `19 02 FF`: report DTCs by status mask with every status bit requested. Positive `59 02` responses are decoded into 24-bit UDS DTC values plus their ISO 14229 status byte; negative responses remain visible with their NRC.

`MblinkMercedesEngineScan` composes the existing endpoint/identity probe with this UDS fault read while the engine ECU channel is still selected. It does not clear faults, alter DTC settings, enter an extended session, perform security access, write data or invoke routines. No response, negative response and malformed data are classified as evidence rather than being disguised as an empty fault list.

## Offline trace replay

The test suite contains a deterministic ELM trace-replay harness. A fixture is an ordered list of expected adapter commands and responses. The replay fails if command order changes, an unexpected request appears or a response no longer decodes through the portable layers.

The current synthetic Mercedes fixture drives the entire read-only sequence through channel configuration, TesterPresent, VIN, six standardized identity DIDs, five CRD3 fingerprint DIDs and UDS `19 02 FF`. This lets development and regression testing continue before the physical adapter arrives. Synthetic fixtures prove deterministic software behaviour only; they never count as vehicle verification.

When real evidence is available, the same harness is the promotion path: sanitize the captured exchange, commit it as a deterministic fixture, then promote only the endpoint/definitions that the capture actually proves.

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

Public Mercedes engine live-data mappings for OM651 are substantially weaker than the standardized UDS identity material. Open-source and independent diagnostic material strongly corroborates that OM651/CDID3 exposes the DPF, injector, rail, EGR and boost values MBLINK wants, but the material currently available to the project does not provide trustworthy raw request identifiers and scaling for those values.

MBLINK therefore separates **capability evidence** from **protocol mapping evidence**. The capability catalogue can advance immediately; a DID/formula is only bound when its request, raw response layout and scaling are supportable. That is deliberately different from guessing a Mercedes DID and showing a plausible-looking value.

## iPhone evidence path

The native iPhone workflow currently invokes the Mercedes identity/fingerprint probe automatically after the standard OBD-II capability exchange. Vehicle and Modules identify the candidate and current evidence result; Log exports the complete diagnostic-evidence transcript even when no live telemetry samples were recorded.

The newer composite Mercedes engine scan and manufacturer-level OM651 catalogue are portable C foundations first. They are intended to replace the remaining generic-only portions of the iPhone workflow as they are wired upward, rather than duplicating protocol logic in Swift or Objective-C.

## Evidence and promotion

An endpoint, DID or scaling rule may move from candidate to vehicle-verified only when its provenance identifies the vehicle, ECU, adapter/transport conditions and a complete reproducible request/response capture. The raw capture must become a deterministic regression fixture before the profile status changes.

No unverified Mercedes manufacturer live-data formula is currently promoted into the C207/OM651 profile. The work can continue without hardware; hardware is required only for the final promotion from corroborated/candidate definitions to the exact C207/OM651 vehicle-verified mappings.
