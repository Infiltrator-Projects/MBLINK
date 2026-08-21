<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes-Benz diagnostics

Mercedes support sits above the generic ELM327, ISO-TP and UDS layers. The manufacturer layer owns vehicle profiles, ECU endpoint provenance and future Mercedes-specific definitions; it does not duplicate transport framing or UDS response validation. Endpoint profiles use the transport-neutral ISO-TP address contract rather than an adapter-specific configuration type.

## C207 / OM651 development profile

The profile contains one initial engine endpoint candidate:

| Key | Request ID | Response ID | Status |
| --- | ---: | ---: | --- |
| `c207-om651-engine-eobd-11bit` | `7E0` | `7E8` | candidate |

This pair comes from the conventional 11-bit physical-address example in the ELM327 documentation, which points to ISO 15765-4. It is useful as a first read-only probe target, but it is not by itself evidence that a C207/OM651 responds at that endpoint. The definition therefore remains candidate status until a reproducible physical vehicle capture is committed as a regression fixture.

## Read-only ECU evidence probe

`MblinkMercedesEcuProbe` now coordinates the existing layers in this order:

1. configure the ELM transmit header and receive address;
2. enable CAN automatic formatting and automatic flow control;
3. send UDS TesterPresent with a positive response requested (`3E00`);
4. if TesterPresent succeeds, request standardized VIN DID `F190` (`22F190`);
5. perform a bounded read-only standardized ECU-identification sweep;
6. retain every raw ELM327 command/response in the session evidence transcript;
7. restore the normal ELM/OBD-II configuration before live generic polling resumes.

The standardized identity sweep currently requests:

| DID | Standard identity purpose |
| --- | --- |
| `F18C` | ECU serial number |
| `F187` | Vehicle manufacturer spare-part number |
| `F188` | Vehicle manufacturer ECU software number |
| `F189` | Vehicle manufacturer ECU software version |
| `F191` | Vehicle manufacturer ECU hardware number |
| `F197` | System name / engine type |

These are identification probes, not Mercedes live-data definitions. A positive response is retained verbatim as evidence; MBLINK does not guess the manufacturer-specific encoding of the returned payload. Negative responses, `NO DATA`, malformed responses and positive responses are classified separately and the sweep continues so one unsupported identification DID does not discard the evidence from the rest.

The probe does not enter an extended diagnostic session, perform security access, clear faults, write data or invoke an ECU routine. TesterPresent may refresh a responding ECU's diagnostic inactivity timer. The optional identity reads occur only after a valid positive TesterPresent response has established that the selected endpoint is answering UDS.

The current ELM-managed probe supports normal physical ISO-TP addresses with matching 11-bit or matching 29-bit request/response identifier formats. Other valid ISO-TP addressing modes remain valid profile data but are rejected as unsupported by this particular adapter path.

## iPhone evidence path

The native iPhone workflow invokes this probe automatically after the standard OBD-II capability exchange. The Vehicle and Modules workspaces identify the candidate and current probe result. The Log workspace exports the complete diagnostic-evidence transcript even when no live telemetry samples were recorded.

For physical validation, connect to the development vehicle, allow the connection sequence to finish, then export the diagnostic evidence from Log. The capture should show the channel setup, `3E00`, `22F190`, and the six bounded identity requests before the ELM adapter is reset for ordinary OBD-II polling.

## Evidence and promotion

An endpoint or DID may move from candidate to vehicle-verified only when its provenance identifies the vehicle, ECU, adapter/transport conditions and a complete reproducible request/response capture. The raw capture must become a deterministic regression fixture before the profile status changes.

No Mercedes manufacturer live-data DID is currently present in the C207/OM651 profile. DPF state/pressure/temperature, boost, rail pressure, injector and EGR definitions remain pending physical vehicle evidence. Standardized identity responses can help establish exactly which ECU/software is present, but they are not used as permission to invent Mercedes-specific live-data addresses or formulas.
