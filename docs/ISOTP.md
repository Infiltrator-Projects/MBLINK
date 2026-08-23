<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# ISO-TP Foundation

MBLINK exposes LINK's portable ISO-TP (ISO 15765-2) engine below UDS and Mercedes-specific diagnostics.

## Supported transport contract

The current shared engine supports both Classical CAN and CAN FD:

- Classical CAN with 8-byte data fields;
- CAN FD with canonical data lengths through 64 bytes;
- 11-bit and 29-bit CAN identifiers;
- normal, extended and mixed addressing;
- physical and functional request classification;
- Single, First, Consecutive and Flow Control frames;
- CAN-FD escaped Single Frame length encoding;
- conventional 12-bit First Frame lengths;
- 32-bit extended First Frame lengths for PDUs above 4095 bytes;
- transmit segmentation and receive reassembly;
- sequence validation, block-size flow control and bounded WAIT handling;
- STmin timing using caller-supplied monotonic microseconds;
- caller-owned bounded receive storage.

`MblinkIsoTpCanFrame`, RX/TX configuration and state are compatibility aliases over LINK's single implementation. `MBLINK_ISOTP_CAN_FD_MAX_DATA_LENGTH` exposes the shared 64-byte limit and `mblink_isotp_can_data_length_is_valid()` exposes the shared Classical-CAN/CAN-FD data-length validator.

A zero configured `data_length` preserves compatibility defaults: 8 bytes for Classical CAN and 64 bytes for CAN FD.

## Addressing and frame validation

`MblinkIsoTpAddress` supplies TX/RX identifiers, identifier width, addressing mode, target type and optional address-extension bytes. Payload capacity depends on addressing overhead and configured link-layer data length rather than being hard-coded to Classical CAN.

CAN FD output is rounded/padded to canonical CAN-FD data lengths. Incoming frames are validated against the configured bus mode and maximum data length.

Functional requests may be transmitted only when they fit in one Single Frame. A responding ECU may return a physical multi-frame response; generated Flow Control frames are directed to that responder's physical request identifier rather than the functional broadcast identifier.

## Receive and transmit state

```text
IDLE -- SF ----------------------> COMPLETE
  |
  +-- FF --> RECEIVING -- CFs --> COMPLETE
                  |
                  +-- error/timeout --> FAILED
```

The caller supplies reassembly storage during initialization. Declared lengths and capacity are validated before receiving begins. Oversize transfers fail explicitly rather than truncating.

```text
IDLE -- SF --------------------------> COMPLETE
  |
  +-- FF --> WAIT_FLOW_CONTROL -- CTS --> SENDING -- CFs --> COMPLETE
                         ^                         |
                         +------ block boundary --+
```

The payload remains caller-owned for the transmitter lifetime. WAIT, OVERFLOW, timeout and malformed flow-control traffic fail deterministically according to the LINK contract.

## Compatibility and testing

Legacy zero-initialized MBLINK configuration remains Classical CAN. Product regression tests verify that the product-prefixed facade exposes CAN-FD support, while LINK's authoritative suite covers Classical CAN, CAN-FD single-frame and multi-frame traffic, canonical frame sizes and extended-length PDUs.

## Error policy

`MblinkIsoTpResult` distinguishes normal progress from malformed or unexpected frames, wrong sequence, buffer or payload limits, timeout, flow-control overflow, WAIT limit and unsupported framing. Terminal failures remain latched until explicit reset.
