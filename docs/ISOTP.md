<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# ISO-TP Foundation

MBLINK 0.6 introduces a portable C implementation of ISO 15765-2 transport behaviour for Classical CAN. It is intentionally independent of Mercedes-Benz definitions, UDS services, CoreBluetooth and ELM327 text framing.

## Scope

The first implementation covers the protocol mechanics needed by later UDS-on-CAN work:

- 11-bit and 29-bit CAN identifiers;
- normal, extended and mixed addressing metadata;
- physical and functional target classification;
- Single Frame (SF) encoding and decoding;
- First Frame (FF) encoding and decoding;
- Consecutive Frame (CF) segmentation and reassembly;
- Flow Control (FC) Continue/Wait/Overflow handling;
- block-size enforcement;
- STmin values from 0-127 ms and 100-900 microseconds;
- four-bit sequence-number validation and wrap-around;
- monotonic receive and flow-control deadlines;
- optional transmitted-frame padding;
- deterministic malformed-frame and wrong-address behaviour;
- bounded 4095-byte PDUs for the Classical CAN 12-bit FF length form.

The implementation does not dynamically allocate memory. Sender and receiver objects own bounded PDU buffers, making ownership and failure behaviour deterministic.

## Address model

`MblinkIsoTpAddress` keeps bus addressing separate from transport state. Each connection records a transmit CAN identifier, receive CAN identifier, identifier width, ISO-TP addressing format, target type and the optional transmit/receive address-extension bytes.

Normal addressing places the PCI byte at payload byte zero. Extended and mixed addressing place one address-extension byte before the PCI. At the ISO-TP framing layer, extended and mixed addressing therefore share the same byte-offset mechanics; their different network-address semantics remain explicit metadata for higher layers and future transport providers.

Frames that do not match the configured CAN identifier or address-extension byte are ignored rather than poisoning an active transfer.

## Receive state machine

A receiver begins in `MBLINK_ISOTP_RX_IDLE`.

- A valid SF completes immediately.
- A valid FF records the declared PDU length, captures the initial payload, enters `RECEIVING`, starts a monotonic deadline and returns an FC Continue frame.
- Each CF must contain the expected four-bit sequence number. The number wraps from `0xF` to `0x0`.
- A configured non-zero receive block size causes another FC Continue after each block.
- Completion exposes the reassembled PDU through `mblink_isotp_receiver_payload()`.
- Missing CFs time out through `mblink_isotp_receiver_tick()`, and a late correctly addressed CF is rejected even if the caller did not tick immediately before feeding it.

Correctly addressed malformed or out-of-sequence frames move the receiver into an explicit error state. A reset is required before starting another PDU.

## Send state machine

A sender copies the caller's PDU into bounded owned storage.

- Payloads that fit in one Classical CAN SF are emitted immediately and complete.
- Larger physical-addressed payloads emit an FF and enter `WAIT_FLOW_CONTROL`.
- FC Continue supplies block size and STmin, after which `mblink_isotp_sender_next()` emits CFs when due.
- A non-zero block size returns the sender to `WAIT_FLOW_CONTROL` at the correct boundary.
- FC Wait frames are bounded by `max_wait_frames`.
- FC Overflow fails the transmission explicitly.
- Missing FC frames time out through `mblink_isotp_sender_tick()`, and a late FC is rejected even if the caller did not tick immediately before feeding it.

Functional multi-frame **transmission** is rejected in this foundation because one-to-many flow-control ownership is not well-defined for the ordinary ISO-TP exchange. Functional Single Frames remain supported. A receiver is still allowed to reassemble a multi-frame response on its configured physical receive address after a functional request.

## Timing

Callers supply a monotonic microsecond clock. MBLINK does not read an operating-system clock inside the protocol engine.

STmin conversion follows the ISO-TP values used by the Linux CAN implementation:

- `0x00` through `0x7F`: 0 through 127 milliseconds;
- `0xF1` through `0xF9`: 100 through 900 microseconds.

Reserved STmin encodings are rejected rather than guessed.

Deadline arithmetic reuses Infiltratr Common saturating unsigned arithmetic so timer calculations cannot wrap.

## Link-layer boundary

`MblinkCanFrame` is deliberately transport-neutral. The 0.6 engine accepts Classical CAN frames only and rejects CAN FD frames explicitly. The frame structure reserves enough data storage for a future CAN FD extension without making the current implementation pretend that CAN FD PCI/length rules are already supported.

This distinction is important: later ELM/native-CAN providers may produce raw CAN frames differently, but ISO-TP framing and state remain in `libmblink`.

## Tests

The ISO-TP regression executables under `tests/` exercise:

- SF encode/decode;
- extended addressing byte placement;
- representative multi-frame diagnostic wire fixtures;
- FC generation;
- sequence mismatch rejection;
- receive and flow-control timeouts;
- BS and STmin behaviour;
- WAIT-frame limits and FC Overflow;
- a 140-byte end-to-end transfer that crosses the sequence-number wrap;
- functional multi-frame rejection;
- transmitted padding;
- wrong-CAN-ID filtering;
- malformed SF rejection.

The fixtures are protocol-level wire fixtures and are not represented as captures from the development Mercedes. Real vehicle captures will be added when hardware is available and can then be replayed through the same engine.

## Deliberate 0.6 limits

0.6 does not yet implement:

- CAN FD Single Frame escape-length encoding;
- the extended First Frame length form for PDUs larger than 4095 bytes;
- a raw CAN hardware/provider API;
- UDS services;
- Mercedes-Benz identifiers or addressing maps.

Those limits are explicit so future work extends a tested transport core rather than mixing unverified protocol variants into the first implementation.
