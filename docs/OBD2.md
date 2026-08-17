<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Standard OBD-II Engine

MBLINK 0.3 provides a portable C11 standard OBD-II layer above the ELM327 engine. The layer is independent of CoreBluetooth, Swift, the Vgate product profile and Mercedes-specific definitions.

The OBD-II engine consumes `MblinkElm327Response` values and returns typed C results. Transport concerns remain in the ELM327/session layer.

## Responsibilities

The 0.3 layer owns:

- standard Mode 01 live-data request construction;
- Mode 02 freeze-frame request construction;
- supported-PID block discovery and accumulation;
- hexadecimal response validation;
- standard formulas for the initial live-data PID set;
- Mode 01 PID 01 readiness decoding;
- Mode 09 PID 02 VIN extraction;
- stored, pending and permanent DTC decoding;
- ELM indexed long-response reassembly for standard OBD replies;
- explicit gating of Mode 04 DTC clearing.

It does not own BLE discovery/GATT details, adapter-specific UUIDs, raw ISO-TP PCI segmentation, UDS, Mercedes-Benz enhanced identifiers, dashboard scheduling or UI presentation.

## Supported live-data PIDs

| PID | Parameter | Unit | Formula |
| --- | --- | --- | --- |
| `04` | Calculated engine load | `%` | `A × 100 / 255` |
| `05` | Engine coolant temperature | `°C` | `A - 40` |
| `0B` | Intake manifold absolute pressure | `kPa` | `A` |
| `0C` | Engine speed | `rpm` | `(256A + B) / 4` |
| `0D` | Vehicle speed | `km/h` | `A` |
| `0F` | Intake air temperature | `°C` | `A - 40` |
| `10` | Mass air flow rate | `g/s` | `(256A + B) / 100` |
| `11` | Throttle position | `%` | `A × 100 / 255` |

The same formula table is used for Mode 02 freeze-frame samples. Unknown PIDs return `MBLINK_OBD2_RESULT_UNSUPPORTED_PID`; MBLINK does not invent a formula.

## Supported-PID discovery

`mblink_obd2_build_supported_pid_request()` accepts the standard 32-PID block bases `00`, `20`, `40`, `60`, `80`, `A0`, `C0` and `E0`.

`mblink_obd2_accept_supported_pids()` merges matching ECU responses into a 256-bit `MblinkObd2PidSet`. The continuation PID at the end of each block determines whether the caller should request the next block. Multiple ECU response lines are treated as a union rather than silently discarding all but one ECU.

## Readiness

Mode 01 PID 01 is decoded into `MblinkObd2Readiness`, exposing MIL state, confirmed-DTC count, spark/compression ignition selection, continuous monitor support/incomplete masks, non-continuous support/incomplete masks, and the original four data bytes.

The meaning of non-continuous monitor bits depends on spark versus compression ignition, so the raw masks are retained rather than flattened into an incorrect universal monitor list.

## VIN and long ELM responses

`mblink_obd2_decode_vin()` accepts Mode 09 PID 02 data and assembles exactly 17 printable VIN characters.

In addition to a single normalized hex line, 0.3.1 handles the indexed long-message display used by ELM-compatible CAN adapters, for example an optional hexadecimal length line followed by `0:`, `1:`, `2:` and later chunks. The C layer validates contiguous indices and reassembles the displayed payload before decoding the VIN.

This is deliberately an ELM presentation-layer reassembly only. MBLINK does not treat it as the project’s generic raw ISO-TP engine; raw ISO-TP segmentation/reassembly remains a later reusable protocol layer.

Incomplete sequences, missing indices, malformed hexadecimal data, non-printable VIN bytes and incomplete VINs are rejected.

## Diagnostic trouble codes

The DTC decoder supports Mode `03` / response `43` stored DTCs, Mode `07` / response `47` pending DTCs, and Mode `0A` / response `4A` permanent DTCs.

Each two-byte code is converted to the conventional five-character form such as `P0133` or `U0123`. Zero padding entries are ignored and duplicate codes are de-duplicated. Indexed ELM long-response blocks are reassembled before DTC-pair decoding, so a longer fault list is not limited to a single displayed CAN chunk.

The list is bounded by `MBLINK_OBD2_MAX_DTCS`; overflow is reported instead of silently truncating.

## DTC clearing safety gate

Mode 04 can clear emissions-related diagnostic information and reset readiness state. MBLINK therefore does not expose it as an unguarded literal command.

`mblink_obd2_build_clear_dtc_request()` requires a `MblinkObd2ClearAuthorization` with both `confirmed = true` and `acknowledge_readiness_reset = true`. Without both acknowledgements, no `04` command is produced. Higher application layers should still require an explicit user action immediately before execution.

## Error handling

The OBD-II layer distinguishes invalid API arguments, ELM-layer errors such as `NO DATA`, malformed hexadecimal data, unexpected service/PID responses, unsupported typed PIDs, bounded-buffer failures, excessive DTC lists and denied destructive requests. An ELM error is never reinterpreted as valid OBD payload data.

## Testing

`tests/obd2_smoke.c` exercises request construction, supported-PID continuation/accumulation, all initial live PID formulas, Mode 02 freeze-frame decoding, spark and compression readiness metadata, single-line and indexed VIN extraction, stored/pending/permanent DTCs, indexed long DTC replies, clear-code gating, malformed response rejection, missing indexed-frame rejection and ELM error propagation.

CI builds and runs the portable C tests on both Ubuntu and macOS.

## Hardware validation boundary

0.3 remains hardware-independent. Request/response contracts are validated with deterministic captured-style conversations, including ELM-compatible indexed long-message formatting. Real BLE transport and the development Vgate adapter are introduced in 0.4; real vehicle captures can then be added as regression fixtures without moving OBD-II formulas into Apple-specific code.
