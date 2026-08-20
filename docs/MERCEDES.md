<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes-Benz diagnostics

Mercedes support sits above the generic ELM327, ISO-TP and UDS layers. The
manufacturer layer owns vehicle profiles, ECU endpoint provenance and future
Mercedes-specific definitions; it does not duplicate transport framing or UDS
response validation. Endpoint profiles use the transport-neutral ISO-TP address
contract rather than an adapter-specific configuration type.

## C207 / OM651 development profile

The profile contains one initial engine endpoint candidate:

| Key | Request ID | Response ID | Status |
| --- | ---: | ---: | --- |
| `c207-om651-engine-eobd-11bit` | `7E0` | `7E8` | candidate |

This pair comes from the conventional 11-bit physical-address example in the
ELM327DSH documentation, which points to ISO 15765-4. It is useful as a first
read-only probe target, but it is not evidence that the C207/OM651 responds at
that endpoint. The definition therefore carries candidate status and cannot be
reported as vehicle-verified.

## Read-only ECU probe

`MblinkMercedesEcuProbe` coordinates existing layers in this order:

1. configure the ELM transmit header and receive address;
2. enable CAN automatic formatting and automatic flow control;
3. send UDS TesterPresent with a positive response requested;
4. decode the complete ELM-managed PDU and validate the UDS `7E 00` response.

The probe does not enter an extended session, perform security access, clear
faults, write data or invoke an ECU routine. Completion means only that the
selected endpoint answered a standard UDS request. Failures preserve whether
the ELM channel, PDU decoder or UDS validator rejected the exchange, including
the underlying ELM result and any UDS negative-response code. TesterPresent may
refresh a responding ECU's diagnostic inactivity timer. The ELM channel remains
configured for the selected endpoint, so callers must configure the next target
or deliberately restore their generic OBD adapter setup afterward.

The current ELM-managed probe supports normal physical ISO-TP addresses with
matching 11-bit or matching 29-bit request/response identifier formats. Other
valid ISO-TP addressing modes remain valid profile data but are rejected as
unsupported by this particular adapter path.

## Evidence and promotion

An endpoint or DID may move from candidate to vehicle-verified only when its
provenance identifies the vehicle, ECU, adapter/transport conditions and a
complete reproducible request/response capture. The raw capture must become a
deterministic regression fixture before the profile status changes.

No manufacturer DID is currently present in the C207/OM651 profile. DPF,
boost, rail-pressure, injector and EGR definitions remain pending physical
vehicle evidence. The iPhone connection workflow does not invoke this probe yet.
