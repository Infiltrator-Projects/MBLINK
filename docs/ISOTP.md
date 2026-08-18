<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# ISO-TP Foundation

MBLINK implements ISO-TP (ISO 15765-2) in portable C below UDS and manufacturer-specific diagnostics. It does not depend on Apple frameworks, ELM327 text parsing or Mercedes definitions.

## Scope

The 0.6 contract deliberately targets Classical CAN with 8-byte data fields and the conventional 12-bit First Frame length, so one PDU is bounded to 4095 bytes.

Supported:

- 11-bit and 29-bit CAN identifiers;
- normal, extended and mixed addressing at the ISO-TP payload-offset level;
- physical and functional request classification;
- Single, First, Consecutive and Flow Control frames;
- transmit segmentation and receive reassembly;
- sequence validation including `0xF -> 0x0` wrap;
- block-size Flow Control;
- CTS, WAIT and OVERFLOW handling with bounded WAIT acceptance;
- STmin `0x00..0x7F` and `0xF1..0xF9` encodings;
- caller-supplied monotonic microsecond timing and explicit timeouts;
- caller-owned bounded receive storage.

Not claimed: CAN FD, CAN-FD Single Frame escape encoding, 32-bit extended First Frame lengths, padding-policy enforcement, a native CAN driver, UDS interpretation or manufacturer addressing/definitions. Unsupported extended-length First Frames are rejected rather than guessed at.

## Frame and addressing model

`MblinkIsoTpCanFrame` represents one Classical-CAN frame. `MblinkIsoTpAddress` supplies TX/RX identifiers, identifier width, addressing mode, target type and optional address-extension bytes.

| Addressing | Single Frame | First Frame initial data | Consecutive Frame data |
| --- | ---: | ---: | ---: |
| Normal | 7 bytes | 6 bytes | 7 bytes |
| Extended / mixed | 6 bytes | 5 bytes | 6 bytes |

Functional requests may be transmitted only when they fit in one Single Frame. A responding ECU may still return a physical multi-frame response. In that case the receive context uses the ECU response ID as `rx_can_id` and that ECU's physical request ID as `tx_can_id`, ensuring generated Flow Control frames go back to the physical ECU rather than the functional broadcast ID.

## Receive state

```text
IDLE -- SF ----------------------> COMPLETE
  |
  +-- FF --> RECEIVING -- CFs --> COMPLETE
                  |
                  +-- error/timeout --> FAILED
```

The caller supplies reassembly storage during init. First Frames validate declared length and capacity before the receiver enters `RECEIVING` and emits CTS. If the declared PDU exceeds caller capacity, MBLINK emits Overflow Flow Control and enters `FAILED`.

Consecutive Frames must carry the expected four-bit sequence number. A non-zero configured block size causes another CTS after each complete block. Completed or failed receive contexts require explicit reset before reuse.

## Transmit state

```text
IDLE -- SF --------------------------> COMPLETE
  |
  +-- FF --> WAIT_FLOW_CONTROL -- CTS --> SENDING -- CFs --> COMPLETE
                         ^                         |
                         +------ block boundary --+
```

The payload remains caller-owned for the transmitter lifetime. WAIT extends the Flow-Control deadline only within `max_wait_frames`; excess WAIT, OVERFLOW, timeout and malformed Flow Control fail deterministically.

Well-formed traffic for another CAN/address-extension endpoint is reported as unexpected but does not poison a transmitter waiting for its own Flow Control frame.

STmin is enforced using caller-provided monotonic microseconds so the 100–900 microsecond encodings are not rounded away. Deadline arithmetic saturates rather than wrapping into the past.

## Error policy

`MblinkIsoTpResult` distinguishes normal progress (`complete`, waiting for Flow Control/separation) from malformed/unexpected frames, wrong sequence, buffer/payload limits, timeout, Flow Control overflow, WAIT limit and unsupported framing.

The engine does not repair missing frames, infer sequence numbers or reinterpret malformed PCI. A terminal RX/TX failure latches its original result until explicit reset, so later calls do not degrade a timeout or sequence error into a generic state error.

## Regression focus

Tests cover identifier/STmin validation, SF/FF/CF/FC framing, extended addressing, end-to-end segmentation/reassembly, sequence wrap, block-size flow control, receive/transmit timeouts, wrong sequence, failure-cause retention, unrelated Flow Control traffic, buffer overflow, WAIT/OVERFLOW handling, functional multi-frame rejection and physical multi-frame response routing after a functional request.
