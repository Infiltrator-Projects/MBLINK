<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Mercedes / Delphi DID Lab

The DID Lab is MBLINK's offline discovery layer for turning public Mercedes/Delphi definitions and real vehicle captures into defensible factory parameters.

Three states are deliberately separate: `corroborated-unmapped` means the value exists but its protocol mapping is unknown; `source-backed-candidate` means an independent source gives a DID/encoding but the development car has not verified it; `vehicle-verified` means the exact request, response shape, scale and meaning are captured in a reproducible fixture. Only vehicle-verified definitions may be automatically polled.

## First exact CRD3 candidate

`CRD3::DT_2007_IN_Battery_voltage` is documented as UDS DID `0x2007`, two-byte big-endian unsigned data, factor `0.0078125`, offset `0`, unit volts. The published response `62 20 07 05 FC` contains raw `0x05FC = 1532`, yielding `11.96875 V`.

It remains a source-backed candidate until a C207 capture verifies it.

## Research manifest

`data/mercedes/crd3-did-lab.json` is the machine-readable research manifest. It records stable keys, source state, known DIDs/scaling, units and the best OBD reference channel for correlation. Rows from a Delphi PDF can be imported here without changing the live scheduler.

## Correlation

The portable engine searches bounded time lag and calculates pair count, Pearson correlation, affine scale/offset, RMSE, normalized RMSE and an evidence score. The fitted relationship is:

`candidate = slope × reference + intercept`

A positive lag means the candidate arrives later than the reference. This can compare candidate factory speed with OBD speed, rail pressure with PID 0x23, battery voltage with PID 0x42, pedal channels with OBD D/E, and temperatures with their known references.

Correlation is evidence, not identity; similar signals can track each other strongly, so a correlation score never promotes a DID by itself.

## CLI

```bash
mblink-did-lab catalog
mblink-did-lab decode 2007 62200705FC
mblink-did-lab correlate reference.csv candidate.csv 2000 100 100
```

Correlation input is a two-column CSV, `timestamp_ms,value`, with an optional header.
