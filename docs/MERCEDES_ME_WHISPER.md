<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Archived Mercedes me Whisper parameterisation evidence

This document records interoperability evidence recovered from an archived
Mercedes me Adapter application supplied for analysis. It preserves the exact
configuration vocabulary, active-file selection, source revision IDs, route
definitions and DataID metadata that can be reproduced from the application.
It does **not** promote an unmapped value to a vehicle-verified live parameter.

## Recovered configuration format

The archived application stores Whisper `.properties` payloads as
hex-encoded encrypted data. Analysis of the archived native Whisper library and
successful decryption of the supplied files established the content transform
used by this package:

- AES-128-ECB;
- PKCS#7 padding;
- ciphertext represented as hexadecimal ASCII;
- recovered 16-byte content key: `pFhtz/bFFp9,%6dQ`;
- key bytes in hex: `704668747A2F62464670392C25366451`.

The result is reproducible. Decrypting
`deviceproviders/config/config.properties` yields:

```text
activeconfiguration = MSA_VIN_cascade.properties
```

The companion `.eh` files are loaded by the Whisper configuration path as
verification material. Their precise signature/verification algorithm is not
claimed here; MBLINK only records that they accompany the encrypted
configuration and are part of the archived loader contract.

A reproducible extraction command for these archived files is:

```bash
KEY=704668747a2f62464670392c25366451
xxd -r -p MSA_VIN_cascade.properties |
  openssl enc -d -aes-128-ecb -K "$KEY" -nosalt
```

## Active Whisper configuration set

The decrypted selector files name these active configurations:

| Area | Active configuration |
| --- | --- |
| device providers | `MSA_VIN_cascade.properties` |
| VIN mapping | `VinMapping.properties` |
| OBD support info | `ObdSupportInfo.properties` |
| trip events | `TripEvents.properties` |
| availability | `Availability.properties` |

The decrypted source embeds these revision identities:

| Configuration fragment | Embedded revision |
| --- | --- |
| MSA common DataIDs | `a1ff42991b5018892bb610b74c215fd1bfe851c0` |
| MSA RawCAN common | `ff6186aadc336fd6bffaeb8ccdceb806d9b16253` |
| MSA VIN cascade | `ed4f6813c40c2993194a797b79ecfa991c3c41dc` |
| VIN mapping | `dac60f99ac76970f1a21350e91e840b07e824644` |
| OBD support info | `05fece0d37bc1e40e49a314facc0ff0e1a00747e` |
| Trip events | `4ff3dd87f8a3938335871aa951dd71c244d93429` |
| Availability | `b1f80361d459973150b2f9d9389f1627aad2429f` |

For evidence provenance, the recovered plaintext
`MSA_VIN_cascade.properties` used for this note is 19,468 bytes / 691 lines
with SHA-256
`9e5a74d1ae0540e7d6a469957ccecdeff0fbb36854188f04a1c2c485db45c9a1`.

## Mercedes DataID catalogue recovered from the app

The MSA file contains a real application DataID catalogue with datatype, unit,
children and/or read-interval metadata. The following entries are especially
important for factory live-data work:

| DataID | Datatype | Unit in archived source | Current MBLINK state |
| --- | --- | --- | --- |
| `engineOilTemperature` | `Long` | `°C` | corroborated-unmapped |
| `fuelPressure` | `Double` | `bar` | corroborated-unmapped |
| `engineReferenceTorque` | `Long` | `Nm` | corroborated-unmapped |
| `intakeManifoldPressure` | `Double` | `bar` | corroborated-unmapped |
| `engineFuelRate` | `Double` | `L/h` | corroborated-unmapped |
| `absoluteBarometricPressure` | `LONG` | `hPa` | corroborated-unmapped |
| `actualFuelFlow` | `Double` | `l/100km` | corroborated-unmapped |
| `fuelFlowSinceReset` | `Double` | `l/100km` | corroborated-unmapped |
| `fuelFlowSinceStart` | `Double` | `l/100km` | corroborated-unmapped |
| `tirePressureFrontLeft` | `Double` | `bar` | corroborated-unmapped |
| `tirePressureFrontRight` | `Double` | `bar` | corroborated-unmapped |
| `tirePressureRearLeft` | `Double` | `bar` | corroborated-unmapped |
| `tirePressureRearRight` | `Double` | `bar` | corroborated-unmapped |
| `boostPressureCan` | `Long` | `%` | corroborated-unmapped |
| `fuelPressureCan` | `Double` | `bar` | corroborated-unmapped |
| `particleFilter` | `Long` | no unit declared | corroborated-unmapped |
| `actualEngineTorque` | `Long` | `%` | corroborated-unmapped |
| `relativeAcceleratorPedalPosition` | `Double` | `%` | corroborated-unmapped |
| `engineRpm` | `Double` | `rpm` | corroborated-unmapped/app-model overlap |
| `vehicleSpeed` | `Double` | `km/h` | corroborated-unmapped/app-model overlap |
| `ambientAirTemperature` | `Double` | `°C` | corroborated-unmapped/app-model overlap |

The source also declares AdBlue remaining-distance values, tank/battery/gas
range, maintenance values, brake/fluid warnings and many lamp/body-state
DataIDs.

Two interpretation rules matter:

1. `intakeManifoldPressure` is explicitly a `Double` in `bar`. It is
   therefore a high-value candidate for the Mercedes factory pressure value
   that may avoid the one-byte SAE PID 0x0B ceiling, but the request and payload
   mapping are not present in this file and must not be guessed.
2. `boostPressureCan` is explicitly declared with unit `%`, not pressure.
   MBLINK must preserve that source fact and must not relabel it as bar/kPa.

The archived source contains some metadata that looks counter-intuitive, for
example `engineReferenceThrottle.unit = Nm`. MBLINK records such fields
verbatim until a second source or the actual request/response mapping explains
them; it does not silently repair the vendor data.

## Device-provider routes recovered from the VIN cascade

The active MSA configuration defines these 500 kbit/s providers:

| Provider | Type | Transmit | Receive |
| --- | --- | ---: | ---: |
| `OBDII_CAR` | `ObdTransceive` | adapter/OBD managed | adapter/OBD managed |
| `Motor` | `PduTransceive` | `0x7E0` | `0x7E8` |
| `Ecu4e0` | `PduTransceive` | `0x4E0` | `0x5FF` |
| `Ecu602` | `PduTransceive` | `0x602` | `0x480` |
| `Ecu607` | `PduTransceive` | `0x607` | `0x587` |
| `Ecu612` | `PduTransceive` | `0x612` | `0x482` |

The same configuration also defines receive-only RawCAN VIN providers at
`0x071` and `0x204`, with byte-mask filtering and ASCII extraction.

These route definitions are manufacturer-owned source evidence. They establish
a real production diagnostic bootstrap route, not the semantic identity of
every ECU behind that route. MBLINK therefore keeps the `0x602`, `0x607`
and `0x4E0` families unclassified until additional identity evidence exists.

## Exact VIN cascade requests

The archived configuration contains exact request/result contracts:

| Request ID | Request PDU | Positive prefix | Timeout |
| --- | --- | --- | ---: |
| `REQ_VIN_OBD` | `09 02` | `49 02` | 1050 ms |
| `REQ_VIN_UDS_22F1A0` | `22 F1 A0` | `62 F1 A0` | 1050 ms |
| `REQ_VIN_KWP2000_1A90` | `1A 90` | `5A 90` | 1050 ms |
| `REQ_VIN_KWP2000_2105` | `21 05` | `61 05` | 1050 ms |
| `REQ_VIN_WAKEUP_OBD` | `01 00` | `41 00` | 1050 ms |

The cascade binds those requests as follows:

- standard OBD first;
- `Motor`: UDS `22 F1 A0`, then KWP `1A 90`;
- `Ecu4e0`: KWP `21 05`, UDS `22 F1 A0`, then KWP `1A 90`;
- `Ecu602`: UDS `22 F1 A0`;
- `Ecu612`: UDS `22 F1 A0`, then KWP `21 05`;
- `Ecu607`: UDS `22 F1 A0`.

The result definitions extract 17 ASCII VIN bytes at offset 3 for OBD/UDS and
offset 2 for the KWP positive responses. This is direct evidence that Whisper
carries request bytes, response matching and payload-extraction rules in its
configuration rather than only in native code.

## DiagLogic value transport and live-data orchestration

The decompiled `Values.java` supplied from the archived application is generated
from `Values.proto`. It is not the live request catalogue; it defines the
higher-level diagnostic value transport between the native diagnostic engine
and the Android application.

The protobuf model is nevertheless important because it proves the exact shape
of a returned measured value:

- `MeasuredItem.dataId`: required application DataID string;
- `MeasuredItem.unit`: optional engineering unit string;
- `MeasuredItem.value`: typed value;
- `MeasuredItem.respondedDeviceAddress`: optional responding device address;
- `MeasuredItem.timeStamp`: optional timestamp.

The typed `Value` union supports `BOOLEAN`, `DOUBLE`, `LONG`, `STRING`
and `BYTEARRAY`. `MeasuredItemCollection` groups measured items under a
required `requestedDeviceId`. The DTC model separately carries
`troubleCode`, `SPORADIC` / `STATIC`, optional display text and
`respondedDeviceId`. `VehicleStatus` aggregates assigned VIN, errors, DTC
collections, measured items, vehicle configuration and adapter software
version. `DiagPreview` carries `cycleCompleted`, `pendingActionToken` and
`repeatable`.

This means the production stack preserves both the logical requested device
identity and the actual responding device address alongside each measured
value. MBLINK should preserve the same distinction when importing source-backed
factory values rather than collapsing manufacturer measurements into one
anonymous live-data stream.

Further symbol recovery from the supplied `libdiaglogic.so` confirms that the
native diagnostic layer contains a dedicated live-data engine, including
`LiveDataDiagLogic`, `LiveDataPointReadAction`,
`LiveDataStreamOpenAction`, `LiveDataStreamReadAction`,
`LiveDataStreamCloseAction` and `LiveDataStreamReader`. Critically,
`LiveDataStreamOpenAction::readDefaultDataIdsFromConfiguration()` exists as a
named exported function. The same binary exposes `DataPoint` objects with
`getDataId()`, `getDeviceId()`, read interval, throttle/repeat state and
last-read time, plus `DataPoints::readDataPoints()` and availability actions.

The binary also names live configuration keys such as `defaultLiveDataPoints`,
`liveDataPoints`, `liveDataFuelValues`, `liveDataStreamReadTimeout`,
`liveDataAvailabilityDataIdsToRead` and `dataPoints`.

Together with the Whisper configuration model, this establishes the production
chain as configuration-driven:

```text
configured DataID/device list
  -> DiagLogic DataPoint scheduler
  -> Whisper device/request/result definition
  -> vehicle response
  -> typed MeasuredItem(dataId, unit, value, respondedDeviceAddress, timestamp)
```

The native library also exposes configuration lifecycle code including
`ConfigUpdateCheckAction`, `ConfigDownloadTimestamps` and
`VehicleConfigurationDataUpdater::getActiveDeviceProviderConfiguration()`,
with accessors for configuration version, variant, delivery ID and revision
information. That is strong evidence that the VIN bootstrap configuration in
the APK is not necessarily the final per-vehicle live-data package: the active
device-provider configuration can be selected/updated after vehicle
identification.

## What this file does not yet give us

`MSA_VIN_cascade.properties` proves that Mercedes' application model contains
the factory-value names and units above, and it proves the VIN bootstrap
routes/requests. It does **not** bind the interesting live DataIDs such as
`intakeManifoldPressure`, `engineOilTemperature`, `engineFuelRate` or
`particleFilter` to a concrete provider/request/result mapping in this active
VIN-cascade file.

Accordingly those rows remain `corroborated-unmapped`. They are stronger than
a guessed gauge name but are not yet safe to auto-poll.

The next reverse-engineering target is the archived diagnostic-logic/value
layer, especially
`com/tsystems/cc/aftermarket/app/android/diaglogic/values/Values.java` and any
configuration/resources it references. The goal is to recover, for each
high-value DataID:

```text
DataID
  -> device/provider
  -> request ID / PDU
  -> matching response
  -> result extraction
  -> formula/scaling
  -> unit
```

Only after that mapping is source-backed should a C207 capture promote it to
`vehicle-verified`.

## Project ownership

The evidence is Mercedes-specific and belongs in MBLINK. Generic request,
ISO-TP, UDS/KWP, scheduling, transport and evidence machinery stays in LINK.
LINK may carry stable Mercedes-me application DataID literals where they are
needed for shared interoperability plumbing, but semantic request mappings and
Mercedes formulas remain manufacturer data owned by MBLINK.
