<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Standard OBD-II Engine

MBLINK 0.3 adds a portable C11 standard OBD-II layer above the ELM327 engine. The layer is independent of CoreBluetooth, Swift, the Vgate product profile and Mercedes-specific definitions.

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
- explicit gating of Mode 04 DTC clearing.

It does not own:

- BLE discovery or GATT details;
- adapter-specific UUIDs or quirks;
- ISO-TP or UDS;
- Mercedes-Benz enhanced data identifiers;
- dashboard scheduling;
- UI presentation.

Those remain separate roadmap layers.

## Supported live-data PIDs

The initial typed decoder supports:

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

The same formula table is used for Mode 02 freeze-frame samples, avoiding duplicate decoding logic.

Unknown PIDs are reported as `MBLINK_OBD2_RESULT_UNSUPPORTED_PID`; the library does not invent a formula.

## Supported-PID discovery

`mblink_obd2_build_supported_pid_request()` accepts the standard 32-PID block bases:

`00`, `20`, `40`, `60`, `80`, `A0`, `C0`, `E0`.

`mblink_obd2_accept_supported_pids()` merges matching ECU responses into a 256-bit `MblinkObd2PidSet`. The continuation PID at the end of each block determines whether the caller should request the next block.

Multiple ECU response lines are therefore treated as a union instead of silently discarding all but one ECU during capability discovery.

## Readiness

Mode 01 PID 01 is decoded into `MblinkObd2Readiness`.

The structure exposes:

- MIL state;
- confirmed DTC count;
- spark/compression ignition selection;
- continuous monitor support mask;
- continuous monitor incomplete mask;
- non-continuous monitor support mask;
- non-continuous monitor incomplete mask;
- the original four bytes.

The meaning of non-continuous monitor bits depends on spark versus compression ignition, so the raw masks are retained rather than being flattened into an incorrect universal monitor list.

## VIN

`mblink_obd2_decode_vin()` accepts Mode 09 PID 02 records and assembles exactly 17 printable VIN characters. Sequence/index bytes are excluded from the returned VIN.

Incomplete, malformed or non-printable responses are rejected.

## Diagnostic trouble codes

The DTC decoder supports:

- Mode `03` / response `43`: stored DTCs;
- Mode `07` / response `47`: pending DTCs;
- Mode `0A` / response `4A`: permanent DTCs.

Each two-byte code is converted to the conventional five-character form such as `P0133` or `U0123`. Zero padding entries are ignored and duplicate codes from multiple response lines are de-duplicated.

The list is bounded by `MBLINK_OBD2_MAX_DTCS`; overflow is reported instead of truncating silently.

## DTC clearing safety gate

Mode 04 can clear emissions-related diagnostic information and reset readiness state. MBLINK therefore does not expose it as an unguarded literal command.

`mblink_obd2_build_clear_dtc_request()` requires a `MblinkObd2ClearAuthorization` with both:

- `confirmed = true`;
- `acknowledge_readiness_reset = true`.

Without both acknowledgements, no `04` command is produced.

This is a construction-time safety boundary. Higher application layers should still require an explicit user action immediately before executing the command.

## Error handling

The standard OBD-II parser distinguishes:

- invalid API arguments;
- ELM-layer errors such as `NO DATA`;
- malformed hexadecimal data;
- responses for an unexpected service/PID;
- unsupported typed PIDs;
- bounded-buffer failures;
- excessive DTC lists;
- denied destructive requests.

An ELM error is never reinterpreted as valid OBD payload data.

## Testing

`tests/obd2_smoke.c` exercises:

- request construction;
- supported-PID continuation and accumulation;
- all initial live PID formulas;
- Mode 02 freeze-frame decoding;
- spark and compression readiness metadata;
- VIN extraction;
- stored/pending/permanent DTCs;
- DTC clear gating;
- malformed response rejection;
- ELM error propagation.

CI builds and runs the portable C tests on both Ubuntu and macOS.

## Hardware validation boundary

0.3 is intentionally hardware-independent. Its request/response contracts are validated with deterministic captured-style conversations. Real BLE transport and the development Vgate adapter are introduced in 0.4; vehicle captures can then be added as regression fixtures without changing the ownership of OBD-II formulas.
