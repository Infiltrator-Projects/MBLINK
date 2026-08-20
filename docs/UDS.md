<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# UDS Foundation

MBLINK implements ISO 14229 Unified Diagnostic Services as a portable C layer above complete diagnostic PDUs. UDS does not own CAN frames, ISO-TP segmentation, BLE, adapter discovery or manufacturer-specific interpretation.

## Boundary

`include/mblink/uds.h` and `src/uds/uds.c` own generic UDS request/response semantics and client timing. Callers provide complete request and response PDUs after any transport framing has been handled.

The layer is allocation-free. Parsed response and DID payload pointers borrow storage from the caller-supplied PDU; callers must keep that PDU valid while using those spans.

## Implemented services

The 0.7 foundation includes:

- generic positive response validation (`request SID + 0x40`);
- standard three-byte negative responses (`0x7F`, request SID, NRC);
- common negative-response-code names while preserving unknown/OEM NRCs as opaque values;
- Diagnostic Session Control (`0x10`) request construction and positive-response decoding;
- standard P2 and P2* timing extraction from session responses;
- TesterPresent (`0x3E`) request/response handling for explicit session keepalive;
- single-DID ReadDataByIdentifier (`0x22`) request construction and response validation;
- caller-supplied DID definitions with stable key/name and bounded response-length contracts for future ECU-identification and manufacturer tables;
- a transport-neutral client state machine with P2 timeout, P2* timeout, NRC `0x78` Response Pending, completed negative responses and terminal protocol/timeout failure.

Tracked Diagnostic Session Control and TesterPresent deliberately reject the suppress-positive-response bit because a no-response transaction requires a different completion contract. Their one-shot request builders can still construct suppressed-response requests for callers that explicitly own that behaviour.

## Timing and failure

Client timing uses caller-supplied monotonic microseconds. Deadlines use saturating arithmetic from the pinned Infiltratr Common library.

A normal negative response completes the current transaction and permits another request. NRC `0x78` extends the active transaction using P2*. Repeated `0x78` responses restart the P2* deadline from the latest response. Malformed, mismatched or timed-out active exchanges enter a failed state; the caller must reset the UDS client before reuse.

A successful Diagnostic Session Control response updates the active session. When the ECU supplies P2/P2* timing values, those values replace the client's current response timeouts until reset or a later successful session transition supplies new timing.

## DID definitions

Generic UDS owns DID request/response structure but not manufacturer meaning. `MblinkUdsDidDefinition` lets a higher layer supply a stable key, display name and accepted response-length range for a DID. The UDS layer validates the DID echo and response bounds without containing Mercedes-Benz constants or decoding formulas.

This is the contract the C207/OM651 manufacturer layer will consume in 0.8.

The first 0.8 integration uses the existing positive-response TesterPresent
contract as a read-only endpoint probe after the ELM-managed physical CAN
channel is configured. That confirms only that the selected endpoint speaks
UDS; it does not establish ECU identity or validate manufacturer data. It does
not enter a session or write vehicle data, although a responding ECU may refresh
its diagnostic inactivity timer.

## Manufacturer policy

This layer contains no Mercedes-Benz ECU addresses, DIDs, scaling formulas or DTC interpretation. Mercedes definitions belong above UDS and remain explicitly unverified until supported by real vehicle responses and regression fixtures.

## Testing

`tests/test_uds.c` covers transactional decoding, session control, TesterPresent, DID definitions/length contracts, DID echo validation, negative-response reuse, repeated Response Pending timing, exact deadline expiry, terminal failure/reset and overflow-safe deadlines. The same `uds.c` source is compiled by CMake and by the iPhone `MBLINKCore` target.
