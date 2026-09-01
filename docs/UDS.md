<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# UDS Foundation

MBLINK exposes LINK's ISO 14229 Unified Diagnostic Services layer above complete diagnostic PDUs. UDS does not own CAN framing, BLE, adapter discovery or Mercedes-specific interpretation.

## Boundary

`include/mblink/uds.h` preserves the product-prefixed core UDS client/session/DID API. `include/mblink/uds_services.h` exposes LINK's complete product-neutral 27-service catalogue and bounded codec API. `src/uds/uds.c` is the native iOS build bridge and compiles the exact pinned LINK core UDS and service-codec implementations rather than maintaining product copies.

The layer is allocation-free. Parsed response and DID payload pointers borrow storage from the caller-supplied PDU; callers must keep that PDU valid while using those spans.

Codec availability is not permission to transmit a service. Discover remains independently deny-by-default and continues to allow only its deliberately approved read-only inventory requests.

## Core UDS client

The typed core includes:

- generic positive response validation (`request SID + 0x40`);
- standard three-byte negative responses (`0x7F`, request SID, NRC);
- common negative-response-code names while preserving unknown/OEM NRCs as opaque values;
- Diagnostic Session Control (`0x10`) request construction and response decoding;
- standard P2 and P2* timing extraction from session responses;
- TesterPresent (`0x3E`) request/response handling;
- ReadDataByIdentifier (`0x22`) request construction and response validation;
- caller-supplied DID definitions with stable key/name and bounded response-length contracts;
- a transport-neutral client state machine with P2 timeout, P2* timeout, NRC `0x78` Response Pending, completed negative responses and terminal protocol/timeout failure.

## Complete service catalogue

The shared service facade exposes exactly 27 registered standard service identifiers, including the original core services and the additional read, control, security and programming service codecs owned by LINK. The public metadata classifies each service as read-only, session-control, state-changing, security or programming.

Named builders are available for the additional service families such as memory reads/writes, scaling/periodic data, dynamic DIDs, routines, DTC clearing/control, timing/link control, secured data and transfer/programming records. Positive-response helpers validate service/subfunction/DID/routine/transfer echoes without assigning manufacturer-specific meaning to raw records.

State-changing, security and programming codecs exist so LINK has a complete standards-shaped serialization layer; they are not automatically enabled by any MBLINK transport or discovery path.

### Complete ReadDTCInformation surface

The 27 registered UDS service identifiers and the ReadDTCInformation report
types are separate catalogues. LINK now also exposes all 27 requested
`ReadDTCInformation (0x19)` report types: `0x01..0x19` plus WWH-OBD
`0x42` and `0x55`. The legacy mirror/emissions report types withdrawn by
ISO 14229-1:2020 remain available for older ECUs.

Every report type has a bounded request encoder and a validated positive-response
envelope. Fixed DTC/count/severity records are structurally validated.
Snapshot and extended-data records whose lengths depend on DID or manufacturer
definitions remain explicit raw spans until the higher layer supplies those
definitions; the generic codec does not guess record boundaries.

This completes issue #32 at the shared standards layer without changing
Discover's read-only allowlist.

## Timing and failure

Client timing uses caller-supplied monotonic microseconds. Deadlines use saturating arithmetic from the pinned Infiltratr Common library.

A normal negative response completes the current transaction and permits another request. NRC `0x78` extends the active transaction using P2*. Repeated `0x78` responses restart the P2* deadline from the latest response. Malformed, mismatched or timed-out active exchanges enter a failed state; the caller must reset the UDS client before reuse.

A successful Diagnostic Session Control response updates the active session. When the ECU supplies P2/P2* timing values, those values replace the client's current response timeouts until reset or a later successful session transition supplies new timing.

## DID and manufacturer policy

Generic UDS owns DID request/response structure but not manufacturer meaning. `MblinkUdsDidDefinition` lets a higher layer supply a stable key, display name and accepted response-length range for a DID. The UDS layer validates the DID echo and response bounds without containing Mercedes-Benz constants or decoding formulas.

Mercedes definitions belong above UDS and remain evidence-gated until supported by documentation or reproducible vehicle captures. A generic codec must never be interpreted as proof that a particular Mercedes ECU supports that service or record format.

## Testing

LINK owns the authoritative codec tests, including all 27 catalogue entries, bounded request builders, echo validation, memory-width validation and Discover safety regression coverage. MBLINK adds product-level facade coverage for the 27-service catalogue and verifies that the iOS bridge compiles LINK's service implementation.
