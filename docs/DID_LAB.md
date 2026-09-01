<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes / Delphi DID Lab

The DID Lab is MBLINK's offline discovery layer for turning public Mercedes/Delphi definitions and real vehicle captures into defensible factory parameters.

Three states are deliberately separate: `corroborated-unmapped` means the value exists but its protocol mapping is unknown; `source-backed-candidate` means an independent source gives a DID/encoding but the development car has not verified it; `vehicle-verified` means the exact request, response shape, scale and meaning are captured in a reproducible fixture. Only vehicle-verified definitions may be automatically polled.

## First exact CRD3 candidate

`CRD3::DT_2007_IN_Battery_voltage` is documented as UDS DID `0x2007`, two-byte big-endian unsigned data, factor `0.0078125`, offset `0`, unit volts. The published response `62 20 07 05 FC` contains raw `0x05FC = 1532`, yielding `11.96875 V`.

It remains a source-backed candidate until a C207 capture verifies it.

## Archived Mercedes me Whisper catalogue

The archived Mercedes me Adapter Whisper parameterisation is now an independent
manufacturer source for the existence, datatype and unit of a substantial
factory-value inventory. The recovered active
`MSA_VIN_cascade.properties` declares, among others,
`engineOilTemperature` (°C), `intakeManifoldPressure` (bar),
`engineFuelRate` (L/h), `actualFuelFlow` / `fuelFlowSinceStart` /
`fuelFlowSinceReset` (l/100km), four individual tyre pressures (bar),
`fuelPressureCan` (bar), `particleFilter`, `actualEngineTorque` (%),
`engineReferenceTorque` (Nm) and AdBlue remaining-distance values.

These are **corroborated-unmapped**, not vehicle-verified live parameters. The
VIN-cascade file contains their application-model metadata but does not bind
those high-value DataIDs to the request/result mapping used to acquire them.
In particular, `intakeManifoldPressure` is a `Double` in bar and is the
strongest current source-backed candidate for a factory pressure value beyond
SAE PID 0x0B's one-byte ceiling, while `boostPressureCan` is explicitly a
percentage and must not be mislabeled as pressure.

The same archive proves the Whisper configuration architecture directly:
device provider, request PDU, matching positive response, timeout, result
extraction and encoding are configuration fields. Its VIN cascade supplies
real examples such as UDS `22 F1 A0 -> 62 F1 A0` and KWP
`21 05 -> 61 05` on manufacturer-owned routes.

See `MERCEDES_ME_WHISPER.md` for the encryption/extraction method, provenance,
full priority inventory and exact recovered routes. The next DID-Lab task is
to recover the live DataID -> provider/request/result/formula binding from the
archived diagnostic-logic/value layer and then validate each mapping on a
reproducible C207 capture.

## Research manifest

`data/mercedes/crd3-did-lab.json` is the machine-readable research manifest. It records stable keys, source state, known DIDs/scaling, units and the best OBD reference channel for correlation. Rows from a Delphi PDF can be imported here without changing the live scheduler.

The development C207 now also contributes two explicit transmission discovery targets: transmission-fluid temperature and actual selected gear. Both remain unmapped. The vehicle display supplies useful ground truth, but MBLINK does not assign a VGS/EGS endpoint, DID or scale until diagnostic evidence proves it. The observed secondary `0x7E1/0x7E9` EOBD responder is therefore an investigation target, not a claimed transmission identity.

## Correlation

The portable engine searches bounded time lag and calculates pair count, Pearson correlation, affine scale/offset, RMSE, normalized RMSE and an evidence score. The fitted relationship is:

`candidate = slope × reference + intercept`

A positive lag means the candidate arrives later than the reference. This can compare candidate factory speed with OBD speed, rail pressure with PID `0x23`, battery voltage with PID `0x42`, pedal channels with OBD D/E, and temperatures with their known references. Transmission temperature and actual gear can also use manually observed vehicle-display anchors until a machine-readable reference channel is available.

Correlation is evidence, not identity; similar signals can track each other strongly, so a correlation score never promotes a DID by itself.

## CLI

```bash
mblink-did-lab catalog
mblink-did-lab decode 2007 62200705FC
mblink-did-lab correlate reference.csv candidate.csv 2000 100 100
```

Correlation input is a two-column CSV, `timestamp_ms,value`, with an optional header.
