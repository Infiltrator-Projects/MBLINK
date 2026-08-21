<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Standard OBD-II Engine

The portable C OBD-II layer consumes normalized `MblinkElm327Response` values and returns typed results. BLE, raw ISO-TP, UDS, manufacturer data, scheduling and UI remain outside this layer.

## Supported live/freeze-frame values

| PID | Parameter | Unit | Formula / selection |
| --- | --- | --- | --- |
| `04` | Calculated engine load | `%` | `A × 100 / 255` |
| `05` | Engine coolant temperature | `°C` | `A - 40` |
| `0B` | Intake manifold absolute pressure | `kPa` | `A` |
| `0C` | Engine speed | `rpm` | `(256A + B) / 4` |
| `0D` | Vehicle speed | `km/h` | `A` |
| `0F` | Intake air temperature | `°C` | `A - 40` |
| `10` | Mass air flow rate | `g/s` | `(256A + B) / 100` |
| `11` | Throttle position | `%` | `A × 100 / 255` |
| `23` | Fuel rail gauge pressure | `kPa` | `(256A + B) × 10` |
| `2C` | Commanded EGR | `%` | `A × 100 / 255` |
| `2D` | EGR error | `%` | `(A - 128) × 100 / 128` |
| `33` | Barometric pressure | `kPa` | `A` |
| `3C` | Catalyst temperature B1S1 | `°C` | `(256A + B) / 10 - 40` |
| `42` | Control-module voltage | `V` | `(256A + B) / 1000` |
| `46` | Ambient air temperature | `°C` | `A - 40` |
| `5C` | Engine oil temperature | `°C` | `A - 40` |
| `5E` | Engine fuel rate | `L/h` | `(256A + B) / 20` |
| `78` | Exhaust-gas temperature B1S1 | `°C` | support bit for B1S1, then `(256B + C) / 10 - 40` |
| `7A` | DPF bank-1 differential pressure | `kPa` | support bit for bank-1 differential pressure, then signed `BC / 100` |
| `7C` | DPF bank-1 inlet temperature | `°C` | support bit for bank-1 inlet temperature, then `(256B + C) / 10 - 40` |

The aftertreatment PIDs contain their own sensor/support flags. MBLINK returns `MBLINK_OBD2_RESULT_UNSUPPORTED_PID` rather than presenting a value when the PID exists but the selected sub-field is not advertised. Unknown typed PIDs also return `MBLINK_OBD2_RESULT_UNSUPPORTED_PID`; MBLINK does not invent formulas.

These are standard SAE OBD-II values. They are not manufacturer-specific Mercedes definitions. In particular, this layer does not claim OM651 soot load, regeneration state, ash load, injector corrections or other undocumented Mercedes values.

## Capability and readiness

Supported-PID discovery accepts the standard 32-PID block bases from `00` through `E0` and unions matching responses from multiple ECUs into one `MblinkObd2PidSet`. The iPhone connection flow follows each advertised continuation bit so values above PID `20`, including diesel aftertreatment PIDs, can actually enter the live scheduler.

PID-set unions, typed samples and DTC-list decodes commit caller-visible output only after the complete response validates.

Mode 01 PID 01 preserves MIL/DTC information, spark/compression ignition selection, monitor masks and the original four bytes. Raw masks are retained because non-continuous monitor meanings differ by ignition type.

## VIN and ELM long responses

Mode 09 PID 02 decoding requires exactly 17 VIN characters: digits or uppercase `A`–`Z`, excluding `I`, `O` and `Q`. Output is transactional: a failed decode leaves the caller-visible VIN empty rather than partially updated.

ELM-compatible adapters may display a long CAN reply as indexed text lines (`0:`, `1:`, ...), preceded by the extracted three-hex-digit total data length. MBLINK validates both index continuity and the final assembled byte count before decoding VIN/DTC data. This is ELM presentation reassembly, not the generic raw ISO-TP engine.

Malformed hexadecimal input, broken index sequences, truncated declared lengths and incomplete VINs are rejected.

## DTCs and clear-code gate

Mode `03`/`43`, `07`/`47` and `0A`/`4A` cover stored, pending and permanent DTCs. Zero padding is ignored, duplicates are removed and the bounded list reports overflow instead of truncating silently. The iPhone connection path performs those three read-only scans automatically after restoring the generic OBD-II adapter channel, exposes the results in Faults and records every exchange as diagnostic evidence.

Mode 04 clearing requires both `confirmed` and `acknowledge_readiness_reset` in `MblinkObd2ClearAuthorization`. Higher layers must still require an explicit user action immediately before execution. MBLINK 0.7.10 does not automatically clear faults.

## Error policy

The layer distinguishes invalid arguments, ELM errors, malformed payloads, unexpected service/PID responses, unsupported typed PIDs/sub-fields, bounded-buffer failure, DTC overflow and denied destructive requests. Adapter errors are never reinterpreted as OBD payload data.
