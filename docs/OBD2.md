<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Standard OBD-II Engine

The portable C OBD-II layer consumes normalized `MblinkElm327Response` values and returns typed results. BLE, raw ISO-TP, UDS, manufacturer data, scheduling and UI remain outside this layer. The implementation is owned by LINK and exposed through MBLINK compatibility APIs.

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
| `11` | Absolute throttle valve position | `%` | `A × 100 / 255` |
| `23` | Fuel rail gauge pressure | `kPa` | `(256A + B) × 10` |
| `2C` | Commanded EGR | `%` | `A × 100 / 255` |
| `2D` | EGR error | `%` | `(A - 128) × 100 / 128` |
| `33` | Barometric pressure | `kPa` | `A` |
| `3C` | Catalyst temperature B1S1 | `°C` | `(256A + B) / 10 - 40` |
| `42` | Control-module voltage | `V` | `(256A + B) / 1000` |
| `45` | Relative throttle position | `%` | `A × 100 / 255` |
| `46` | Ambient air temperature | `°C` | `A - 40` |
| `47` | Absolute throttle position B | `%` | `A × 100 / 255` |
| `48` | Absolute throttle position C | `%` | `A × 100 / 255` |
| `49` | Accelerator pedal position D | `%` | `A × 100 / 255` |
| `4A` | Accelerator pedal position E | `%` | `A × 100 / 255` |
| `4B` | Accelerator pedal position F | `%` | `A × 100 / 255` |
| `4C` | Commanded throttle actuator | `%` | `A × 100 / 255` |
| `5C` | Engine oil temperature | `°C` | `A - 40` |
| `5E` | Engine fuel rate | `L/h` | `(256A + B) / 20` |
| `78` | Exhaust-gas temperature B1S1 | `°C` | support bit for B1S1, then `(256B + C) / 10 - 40` |
| `7A` | DPF bank-1 differential pressure | `kPa` | support bit for bank-1 differential pressure, then signed `BC / 100` |
| `7C` | DPF bank-1 inlet temperature | `°C` | support bit for bank-1 inlet temperature, then `(256B + C) / 10 - 40` |

The aftertreatment PIDs contain their own sensor/support flags. MBLINK returns `MBLINK_OBD2_RESULT_UNSUPPORTED_PID` rather than presenting a value when the PID exists but the selected sub-field is not advertised. Unknown typed PIDs also return `MBLINK_OBD2_RESULT_UNSUPPORTED_PID`; MBLINK does not invent formulas.

PID `11` is the throttle-valve measurement, not accelerator-pedal demand. Pedal demand is kept as the distinct standard `49`/`4A`/`4B` channels and is only polled when the vehicle advertises those PIDs. Fuel-rail pressure remains canonical `kPa` in decoded samples and evidence exports; human-facing parameter formatting auto-scales values of at least 1000 kPa to MPa (for example, `123400 kPa` is shown as `123.4 MPa`).

These are standard SAE OBD-II values. They are not manufacturer-specific Mercedes definitions. In particular, this layer does not claim OM651 soot load, regeneration state, ash load, injector corrections or other undocumented Mercedes values.

## Capability and readiness

Supported-PID discovery accepts the standard 32-PID block bases from `00` through `E0` and unions matching responses from multiple ECUs into one `MblinkObd2PidSet`. The iPhone connection flow follows each advertised continuation bit so values above PID `20`, including diesel aftertreatment PIDs, can actually enter the live scheduler.

PID-set unions, typed samples and DTC-list decodes commit caller-visible output only after the complete response validates. Mode 01 PID 01 preserves MIL/DTC information, spark/compression ignition selection, monitor masks and the original four bytes. Raw masks are retained because non-continuous monitor meanings differ by ignition type.

Readiness is diagnostic information, not merely protocol state. The user-facing diagnostic workflow must expose applicable monitor completion/support information and preserve the difference between a clean completed monitor and a monitor that has not completed.

## Responder-specific capability inventory

Mode 01 capability discovery is now responder-attributed before routine polling
starts. MBLINK temporarily enables CAN response headers for the `0100`,
`0120`, `0140`, ... capability exchange, lets LINK retain one supported-PID
bitmap per responding CAN ECU, and then restores the ordinary response format
before VIN and fault acquisition.

The PID selector must use those advertised capability maps. It must never infer
"supported by this ECU" from the set of PIDs that happened to have been polled
already; polling is opt-in, so sample history is necessarily incomplete.

The captured C207 masks currently prove:

- responder `0x7E8`: 29 advertised capability codes including the `0x20`
  and `0x40` continuation pages, therefore 27 non-continuation Mode 01 codes;
  24 of those are current scalar live values already decoded by LINK;
- responder `0x7E9`: 12 advertised capability codes including continuation
  pages, therefore 10 non-continuation Mode 01 codes; 9 are current scalar live
  values already decoded by LINK.

This is also why the standard OBD-II screen should not be padded with hundreds
of unsupported catalogue entries. SAE Mode 01 has a bounded PID address space
and a vehicle advertises only the subset it implements. The much larger
Mercedes actual-value inventory available through the same diagnostic socket
belongs to the manufacturer UDS/KWP layer and is kept separate from SAE
OBD-II.

## VIN and ELM long responses

Mode 09 PID 02 decoding requires exactly 17 VIN characters: digits or uppercase `A`–`Z`, excluding `I`, `O` and `Q`. Output is transactional: a failed decode leaves the caller-visible VIN empty rather than partially updated.

ELM-compatible adapters may display a long CAN reply as indexed text lines (`0:`, `1:`, ...), preceded by the extracted three-hex-digit total data length. MBLINK validates both index continuity and the final assembled byte count before decoding VIN/DTC data. This is ELM presentation reassembly, not the generic raw ISO-TP engine.

Malformed hexadecimal input, broken index sequences, truncated declared lengths and incomplete VINs are rejected.

## DTCs and diagnostic knowledge

Mode `03`/`43`, `07`/`47` and `0A`/`4A` cover stored, pending and permanent DTCs. Zero padding is ignored, duplicates are removed and the bounded list reports overflow instead of truncating silently. The iPhone connection path performs those three read-only scans automatically after restoring the generic OBD-II adapter channel, exposes the results in Faults and records every exchange as diagnostic evidence.

Formatting the returned bytes as `Pxxxx`, `Cxxxx`, `Bxxxx` or `Uxxxx` is not the completion condition for OBD fault support.

MBLINK consumes the generic `dtc_knowledge` API from its exact pinned LINK revision. A valid code is normalized and classified by system and generic/manufacturer origin. When the shared catalogue contains the definition, the result also carries a human-readable title, diagnostic category and standards-backed source class. Structured cylinder families such as injector circuits, contribution/balance, misfires and glow-plug circuits are generated by the shared layer rather than duplicated as hundreds of UI strings.

A valid manufacturer-specific or not-yet-catalogued code remains an explicit unknown record with its raw code intact. No layer may synthesize a plausible description merely to avoid an unknown result.

The iPhone `ConnectionViewModel` exposes structured `DiagnosticFault` records and maintains compatibility display strings in the form `CODE — description` so the existing Faults screen immediately translates known generic DTCs while the richer card presentation is completed.

The full product requirement is defined in `FAULT_DIAGNOSTICS.md`. Generic SAE/ISO diagnostic knowledge belongs in LINK so every LINK-family product uses the same definitions and semantics. Manufacturer-specific meanings remain in the product layer that owns that manufacturer.

## Freeze-frame integration requirement

The portable API supports Mode 02 freeze-frame PID request construction and decoding using the same typed PID formulas as live data. After a fault is found, higher-level diagnostic flow should collect capability-supported freeze-frame context where available and associate it with the relevant fault investigation. Useful context can include RPM, vehicle speed, load, coolant temperature, intake/manifold data and other supported standard PIDs. Unsupported or unavailable values remain unavailable.

A diagnostic UI must clearly distinguish current live data from freeze-frame values captured when the ECU recorded a fault.

## Scan-state requirement

A successful scan that returns zero DTCs, a scan that has not run and a scan that failed are distinct outcomes. The portable/application state preserves a successful three-stage inventory explicitly as `Complete · …`, with timeout/adapter errors represented separately. The Faults presentation must consume that state directly rather than inferring `no faults` merely because an array is empty.

## Clear-code gate

Mode 04 clearing requires both `confirmed` and `acknowledge_readiness_reset` in `MblinkObd2ClearAuthorization`. Higher layers must still require an explicit user action immediately before execution. MBLINK does not automatically clear faults.

## Error policy

The layer distinguishes invalid arguments, ELM errors, malformed payloads, unexpected service/PID responses, unsupported typed PIDs/sub-fields, bounded-buffer failure, DTC overflow and denied destructive requests. Adapter errors are never reinterpreted as OBD payload data.


## Runtime polling policy

Capability discovery and runtime polling are intentionally separate. A vehicle may advertise dozens of standard PIDs without MBLINK continuously requesting every one. The application keeps every supported definition visible, but each PID has an independent polling enable state backed by LINK's scheduler. Disabling a PID removes it from routine live dispatch rather than merely hiding its value.

MBLINK's first-run mobile/Linux core set is deliberately bounded to RPM, speed, coolant temperature, fuel-rail pressure, absolute throttle-valve position, accelerator-pedal positions D/E and ambient temperature. Unsupported entries simply never enter the schedule. Users can then enable additional advertised PIDs individually.

PID `46` ambient temperature is displayed to one decimal place for visual consistency with the vehicle display. SAE PID `46` itself has whole-degree-Celsius source resolution, so a value such as `18.0 °C` does not claim a measured tenth. A future verified Mercedes factory ambient-temperature mapping can provide its own native precision.


## Fuel Level Input (PID 0x2F)

PID `0x2F` is decoded as one byte using `A × 100 / 255` percent and is scheduled at a deliberately low cadence because tank level changes slowly. It is independent of fuel-rate PID `0x5E`. A vehicle may support fuel level while not supporting standard engine fuel rate, as seen in the C207 capture.


## C207 field validation of pedal and throttle semantics

A real C207 drive capture showed accelerator-pedal positions D and E tracking one another closely while PID `11` absolute throttle-valve position followed a distinctly different trajectory. This is expected on the OM651 diesel and is treated as field evidence that the three values must remain separate rather than presenting PID `11` as accelerator demand.
