<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# ISO-TP Foundation

MBLINK 0.6 establishes a reusable **ISO-TP (ISO 15765-2) transport layer in portable C11**. It sits below UDS and manufacturer-specific diagnostics and has no dependency on Swift, Objective-C, CoreBluetooth, ELM327 text parsing or Mercedes-Benz definitions.

## Scope of 0.6

The initial implementation deliberately targets **Classical CAN with an 8-byte CAN data field**. It supports the conventional 12-bit First Frame length and therefore bounds one ISO-TP PDU to **4095 bytes**.

Supported in 0.6:

- 11-bit and 29-bit CAN identifiers;
- normal addressing;
- extended addressing;
- mixed addressing at the ISO-TP payload-offset level;
- physical and functional target classification;
- Single Frame (SF);
- First Frame (FF);
- Consecutive Frame (CF);
- Flow Control (FC);
- transmit segmentation;
- receive reassembly;
- sequence-number checking and 0xF -> 0x0 wrap;
- receiver block-size flow control;
- Continue To Send, Wait and Overflow flow status;
- bounded WAIT-frame acceptance on transmit;
- STmin values `0x00..0x7F` (0..127 ms);
- STmin values `0xF1..0xF9` (100..900 microseconds);
- explicit receive and flow-control timeouts;
- caller-owned bounded receive storage;
- deterministic error/failure states.

Not claimed by 0.6:

- CAN FD link-layer framing;
- CAN-FD Single Frame escape format;
- 32-bit extended First Frame lengths;
- padding-policy enforcement;
- a native CAN driver/provider;
- UDS service interpretation;
- Mercedes-Benz addressing or DID definitions.

An extended-length First Frame whose 12-bit length field is zero is rejected as `unsupported` rather than guessed at. This keeps the Classical-CAN contract explicit until CAN FD / extended-length support is deliberately implemented and tested.

## Public model

The public API is in `include/mblink/isotp.h`.

`MblinkIsoTpCanFrame` is a transport-neutral Classical-CAN frame model containing:

- CAN identifier;
- standard/extended-ID flag;
- data length;
- up to eight data bytes.

`MblinkIsoTpAddress` contains the transmit and receive CAN identifiers, identifier widths, addressing mode, target type and optional address-extension bytes.

The ISO-TP engine does not open a CAN device. A future ELM/raw-CAN/native provider is responsible for converting between its own transport and `MblinkIsoTpCanFrame`.

## Addressing

Normal addressing places ISO-TP PCI at data byte zero.

Extended and mixed addressing reserve the first CAN data byte for the configured address-extension value, so PCI begins at data byte one. At this layer the payload offset is the same for extended and mixed addressing; the semantic choice remains explicit in the address configuration for higher/lower layers.

For Classical CAN, the resulting payload capacities are:

| Addressing | Single Frame | First Frame initial data | Consecutive Frame data |
| --- | ---: | ---: | ---: |
| Normal | 7 bytes | 6 bytes | 7 bytes |
| Extended / mixed | 6 bytes | 5 bytes | 6 bytes |

Functional multi-frame transmission is rejected in 0.6. Functional requests that fit in a Single Frame remain representable. The `target_type` classification governs transmitted request PDUs; it does not prohibit a responding ECU from returning a physical multi-frame response. For such a response, configure the receive context with the ECU response CAN ID as `rx_can_id` and that ECU's physical request CAN ID as `tx_can_id`, so generated Flow Control frames are returned to the physical ECU rather than broadcast on the functional request ID.

## Receive state machine

`MblinkIsoTpRx` uses explicit states:

```text
IDLE
  | SF
  +--------------------------> COMPLETE
  |
  | FF
  v
RECEIVING -- valid CFs -----> COMPLETE
  |
  +-- malformed/timeout/SN --> FAILED
```

The caller supplies the reassembly buffer and capacity during `mblink_isotp_rx_init()`. No hidden dynamic allocation occurs.

On an accepted First Frame the receiver:

1. validates the declared PDU length;
2. rejects unsupported extended-length framing;
3. verifies the caller buffer can hold the complete PDU;
4. copies the initial payload bytes;
5. enters `RECEIVING`;
6. emits a Continue-To-Send Flow Control frame using configured block size and STmin.

If the declared PDU is larger than the caller buffer, the receiver emits an **Overflow** Flow Control frame and enters `FAILED`.

Each Consecutive Frame must carry the expected four-bit sequence number. Sequence numbers wrap from `0xF` to `0x0`. A mismatch fails the receive context instead of attempting heuristic recovery.

When a non-zero receiver block size is configured, another Continue-To-Send Flow Control frame is emitted after each full block until the PDU completes.

A completed or failed receive context must be explicitly reset before it is reused. This prevents a late frame from silently becoming the beginning of a different PDU.

## Transmit state machine

`MblinkIsoTpTx` uses explicit states:

```text
IDLE
  | SF ----------------------> COMPLETE
  |
  | FF
  v
WAIT_FLOW_CONTROL
  | CTS
  v
SENDING -- CFs -------------> COMPLETE
  | block boundary
  +--------------------------> WAIT_FLOW_CONTROL

WAIT / overflow / timeout / malformed FC -> FAILED as appropriate
```

The caller-owned payload remains valid for the life of the transmit context.

For a multi-frame PDU, `mblink_isotp_tx_start()` emits the First Frame and moves to `WAIT_FLOW_CONTROL`. After a valid Continue-To-Send frame, `mblink_isotp_tx_next()` emits Consecutive Frames while enforcing the supplied block size and STmin.

A Flow Control `WAIT` frame extends the flow-control deadline only while the configured `max_wait_frames` budget remains. Exceeding that budget fails deterministically. `OVERFLOW` fails immediately.

## Timing

ISO-TP timing uses a caller-provided **monotonic microsecond clock**. The engine never reads an operating-system clock directly.

Microseconds are intentional: ISO-TP STmin can encode 100–900 microsecond spacing with values `0xF1..0xF9`. Rounding these values to milliseconds would lose protocol information.

Deadline arithmetic uses Infiltratr Common's saturating unsigned addition so a very large monotonic timestamp cannot wrap a timeout into the past.

The caller drives timeout observation with:

- `mblink_isotp_rx_tick()`;
- `mblink_isotp_tx_tick()`.

A context that has entered `FAILED` remains non-successful until explicitly reset.

## Error policy

The public `MblinkIsoTpResult` distinguishes normal progress from protocol failures, including:

- complete;
- waiting for flow control;
- waiting for STmin separation;
- Flow Control WAIT;
- malformed frame;
- unexpected frame;
- wrong sequence number;
- receive buffer too small;
- payload too large;
- timeout;
- Flow Control overflow;
- WAIT-frame limit exceeded;
- explicitly unsupported framing.

The engine does not silently repair sequence errors, infer missing frames or reinterpret malformed PCI.

## Regression coverage

`tests/isotp_smoke.c` exercises:

- STmin conversion including sub-millisecond encodings;
- 11-bit and 29-bit identifier validation;
- Single Frame transmit and receive;
- extended-address receive framing;
- end-to-end 130-byte segmentation/reassembly;
- sequence-number wrap;
- receiver block-size Flow Control;
- wrong-sequence rejection;
- receive timeout;
- receive-buffer overflow and Overflow FC generation;
- transmit WAIT handling and WAIT limit;
- Flow Control overflow;
- transmit flow-control timeout;
- functional multi-frame transmit rejection;
- physical multi-frame response reception after a functional request, including physical Flow Control routing.

The CTest target runs under the same strict warning-as-error policy as the rest of `libmblink` on Ubuntu and macOS. Because ISO-TP is included in the same `mblink.c` translation unit used by the Xcode static-library target, iOS Debug and Release builds compile the same implementation as an additional platform gate.

## Next layer

Milestone 0.7 builds **UDS in C** above this ISO-TP contract. UDS will consume complete ISO-TP PDUs and must not duplicate segmentation, flow-control or CAN-addressing logic.
