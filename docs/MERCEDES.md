<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes-Benz diagnostics

Mercedes support sits above the generic ELM327, ISO-TP and UDS layers. The manufacturer layer owns vehicle profiles, ECU endpoint provenance and Mercedes-specific evidence/definitions; it does not duplicate transport framing or UDS response validation. Endpoint profiles use the transport-neutral ISO-TP address contract rather than an adapter-specific configuration type.

## C207 / OM651 development profile

The profile contains one initial engine endpoint candidate:

| Key | Request ID | Response ID | Status |
| --- | ---: | ---: | --- |
| `c207-om651-engine-eobd-11bit` | `7E0` | `7E8` | candidate |

This pair comes from the conventional 11-bit physical-address example in the ELM327 documentation, which points to ISO 15765-4. It is useful as a first read-only probe target, but it is not by itself evidence that a C207/OM651 responds at that endpoint. The definition therefore remains candidate status until a reproducible physical vehicle capture is committed as a regression fixture.

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
| `F100` | Session / variant | raw ECU-family evidence only |
| `F154` | Supplier identifier | raw ECU-family evidence only |
| `F196` | EROTAN | raw ECU-family evidence only |
| `1001` | Full variant coding | raw ECU-family evidence only |
| `1002` | Partial variant coding | raw ECU-family evidence only |

The CRD3 identifiers are not being treated as DPF, rail-pressure, injector, EGR or turbo parameters. Their purpose is to establish that the responding engine ECU behaves like the CRD3/CDID3 family used with OM651, and to preserve exact raw responses for the vehicle fixture. MBLINK does not infer a live-data formula from a positive fingerprint response.

Negative responses, `NO DATA`, malformed responses and positive responses are classified separately and the sweep continues so one unsupported identifier does not discard evidence from the rest.

The probe does not enter an extended diagnostic session, perform security access, clear faults, write data or invoke an ECU routine. TesterPresent may refresh a responding ECU's diagnostic inactivity timer. Optional identification/fingerprint reads occur only after a valid positive TesterPresent response has established that the selected endpoint is answering UDS.

The current ELM-managed probe supports normal physical ISO-TP addresses with matching 11-bit or matching 29-bit request/response identifier formats. Other valid ISO-TP addressing modes remain valid profile data but are rejected as unsupported by this particular adapter path.

## Why the CRD3 fingerprint comes before proprietary DPF formulas

Public Mercedes engine live-data mappings for OM651 are substantially weaker than the standardized UDS identity material. Open-source Mercedes tooling confirms that `CDID3` on OM651 maps to the `CRD3` diagnostic family, and CaesarSuite includes a CRD3 simulator with the five identifiers above, but it does not publish trustworthy DPF soot-load, regeneration-state, injector-correction or equivalent CRD3 live-data formulas.

MBLINK therefore uses those five requests only to fingerprint the real ECU. This is deliberately different from guessing a Mercedes DID and showing a plausible-looking number. The next physical C207 capture should tell us which CRD3 fingerprint requests really respond and gives us the exact ECU/software identity needed to bind later DPF definitions to the correct variant.

## iPhone evidence path

The native iPhone workflow invokes this probe automatically after the standard OBD-II capability exchange. The Vehicle and Modules workspaces identify the candidate and current probe result. The Log workspace exports the complete diagnostic-evidence transcript even when no live telemetry samples were recorded.

For physical validation, connect to the development vehicle, allow the connection sequence to finish, then export the diagnostic evidence from Log. The capture should show the channel setup, `3E00`, `22F190`, the six standardized identity requests, and then `22F100`, `22F154`, `22F196`, `221001` and `221002` before the ELM adapter is reset for ordinary OBD-II polling.

## Evidence and promotion

An endpoint or DID may move from candidate to vehicle-verified only when its provenance identifies the vehicle, ECU, adapter/transport conditions and a complete reproducible request/response capture. The raw capture must become a deterministic regression fixture before the profile status changes.

No unverified Mercedes manufacturer live-data formula is currently promoted into the C207/OM651 profile. DPF soot/ash/regeneration state, boost, rail pressure, injector corrections and EGR manufacturer definitions remain pending physical vehicle evidence. The CRD3 fingerprint is the bridge from generic OBD-II to that actual ECU-specific work: it tells us which engine-control family and variant we are really talking to before we attach proprietary interpretations.
