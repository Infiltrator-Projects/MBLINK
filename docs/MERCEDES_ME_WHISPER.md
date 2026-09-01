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

## Local database live-data availability model

The archived application database migrations add an independent, concrete
application-level live-data model.

Migration 13 creates:

- `LiveDataId(Id, IdKey)`;
- `VehicleLiveDataId(Id, LiveDataId, Vehicle)`;
- a per-vehicle `AvailabilityCheckDone` flag;
- `DashboardSettings` tied to a vehicle.

It then seeds the live-data catalogue with exactly:

```text
engineFuelRate
calculatedEngineLoad
relativeAcceleratorPedalPosition
throttlePosition
ambientAirTemperature
intakeAirTemperature
```

Migration 16 adds:

```text
engineCoolantTemperature
engineOilTemperature
```

This is stronger than a raw string occurrence. It proves that the production
Android application persisted a per-vehicle set of available live DataIDs and
that these eight DataIDs were first-class members of that live-data system.

The schema does **not** store the UDS/KWP request, CAN route, byte extraction or
formula. It appears to persist availability/presentation state after the
diagnostic/configuration layer has determined which DataIDs a particular
vehicle supports. This aligns with the native DiagLogic functions for live-data
availability and default DataID selection.

Migration 14 separately creates a per-vehicle `Malfunction` table keyed by a
Mercedes `DataId`, reinforcing that application-facing DataIDs are a durable
semantic boundary between the diagnostic engine and the UI/database layer.

The bundled `assets/data_export.zip` is therefore a high-priority artifact to
inspect. Its being protected does not by itself prove that it contains the
Whisper request catalogue, but the database schema makes an exported database
or vehicle-state fixture materially interesting: populated
`VehicleLiveDataId` rows could reveal which live DataIDs Mercedes considered
available for the exported vehicle, and configuration/version fields could
help identify the selected per-vehicle diagnostic package.

## GDK session-master-key bridge

Further native analysis narrows the password/key path substantially.

The exported C entry point `start` takes the Session Master Key callback as
its **second** argument. Thumb disassembly shows that argument being wrapped
directly in `jni::main::JniSessionMasterKeyProvider`, while the first
argument is converted to the string passed into the adapter start path and the
third argument is the boolean passed to `ObdAdapter::start(..., bool)`.

The provider constructor type is:

```text
const char *(*)(const char *, const char *)
```

The callback contract can now be identified more precisely from
`RetrieveSmkAction::run()`. The action:

1. reads `KEY_OBD_ADAPTER_ID` from the init-sequence state;
2. checks `SessionMasterKeyCache::get(adapterId)`;
3. on a cache miss, executes `ISlCommandGetPasskey`;
4. calls the configured Session Master Key provider with the adapter ID and
   the returned adapter passkey;
5. treats the callback result as a string;
6. rejects special/error responses beginning with `#`;
7. Base64-decodes the normal callback result;
8. constructs a `SessionMasterKey` from the decoded bytes; and
9. stores it back in the cache under the adapter ID.

The returned Session Master Key is therefore a Base64-encoded 32-byte key, not
the same object as the system encryption password.

The persistent `SessionMasterKeyCache` exposes:

- `getSmkEncryptionKey()`;
- `getPasswordHash(...)`;
- `getSmkKeyCacheHash()`;
- `readCache()` / `writeCache()`;
- `getFilename()` / `getCacheDir()`;
- `createCacheDir()` / `remove(...)`.

`getCacheDir()` is built from
`cc::common::System::getRuntimeDirectory()` plus the library's cache
directory constant.

Most importantly, `getSmkEncryptionKey()` first asks
`System::getEncryptionPassword()`. If the application has not supplied one,
the GDK contains a hard-coded fallback byte string:

```text
QRn39mNyX55E7OLFyGVEq9NnUl1MuUQEDogGNskX/us=
```

That exact 44-byte ASCII value is constructed in the function itself. It is
not the SMK. It is the fallback key material used by the SMK cache encryption
path.

The cache's `getPasswordHash()` is also reproducible from the native code:
it calls `Aes256Utils::encode(password, password)` and then Base64-encodes
the resulting 16-byte block. With the built-in fallback above this produces:

```text
jeZ7us5Yd8wpoLgKW46pyg==
```

The cache therefore has a deterministic fallback protection path even when the
application never calls `setSystemEncryptionPassword`.

This is separate from the Session Master Key itself. The stack has at least
two distinct key concepts:

1. an application/system encryption password, or the built-in fallback above,
   used by GDK cache protection; and
2. a 32-byte Session Master Key fetched using
   `(adapterId, adapterPasskey)`, Base64-decoded by GDK, cached per adapter
   and later used to derive the live adapter Session Key.

The GDK binary also exposes the Java-facing class name
`com/tsystems/cc/aftermarket/app/android/gdkbt/BtConnectionGdkImp` and the
native operation `removeCachedSessionMasterKey`. The highest-value remaining
bridge target is therefore the DEX/decompiled class that supplies the
`start` callback and implements the backend lookup for
`(adapterId, passkey) -> Base64 SMK`.


## Protected data export artifact

The archived APK also contains `assets/data_export.zip`. Inspection of the
central directory shows a single member:

```text
data_export.json
uncompressed: 7,415,862 bytes
compressed:     65,470 bytes
CRC32:       DE173F34
timestamp:   2020-10-21 18:29:02
```

The member is encrypted, but the ZIP metadata does not contain a WinZip AES
extra field and reports ZIP extraction compatibility version 2.0. That is
consistent with traditional PKZIP/ZipCrypto encryption rather than AES ZIP
encryption.

This distinction matters for evidence recovery: traditional ZipCrypto is weak
against known-plaintext recovery. Because the encrypted member is JSON, a
recoverable plaintext prefix or other known fragment may be enough to recover
the internal ZIP keys without discovering the original password. No recovered
plaintext is claimed until that attack is actually reproduced.

The archive's extreme compression ratio (about 7.4 MB down to about 65 KB)
also indicates highly repetitive structured content, making it a strong
candidate for synthetic/export fixture data rather than arbitrary binary
payload.

The APK additionally contains a very small `assets/android/appExport.zip`
artifact. Inspection shows that it is **not** encrypted the same way as
`data_export.zip`: its sole member `appExport.json` is 16 bytes and uses
WinZip AES metadata (`compression method 99`, extra field `0x9901`, vendor
`AE`, strength `3` = AES-256, actual compression method `8`/deflate).
Therefore it is not suitable as a direct ZipCrypto known-plaintext companion
for `data_export.zip`.

The native `libcommon.so` supplied from the same application exports
`setSystemEncryptionPassword`,
`cc::common::System::setEncryptionPassword()`,
`cc::common::System::getEncryptionPassword()` and AES-256 helper routines.

Thumb disassembly of `setSystemEncryptionPassword` shows that it accepts a
caller-supplied byte pointer plus length, copies those bytes into a vector and
passes that vector to `System::setEncryptionPassword()`. There is no fixed
password literal in that wrapper. Among the supplied native libraries,
`libgdk.so` is the only library with an undefined reference to
`System::getEncryptionPassword()`; the other supplied vehicle-stack
libraries do not import that getter directly. This narrows the next
reverse-engineering target to the application/JNA/JNI code that invokes the
C ABI setter and to GDK call sites that consume the stored password.

This gives a concrete next reverse-engineering target: identify the Java/JNI
call site that supplies the system encryption password and determine whether
the same application secret is reused by `appExport.zip`, backup/restore, or
other protected app-state artifacts. Password reuse is not assumed until
verified.

## Demo-mode application data

The archived `assets/demo_mode_data.json` is application/demo content rather
than a DiagLogic/Whisper request catalogue. It is still useful because it shows
the product-side semantic model Mercedes expected to present.

The supplied demo vehicle is VIN `WDD2130431A038256` and carries fields for
mileage, maintenance remaining time/distance, malfunctions, fuel level, fuel
range, aggregate fuel consumption, dealer, parking position, appointments,
refuelling history, trip history and saved/favourite locations. The example
aggregate fuel record is `kilometersDriven = 38002` and
`litersConsumed = 2964`; the example malfunction token is
`brakeLiningCriticalOccurredOnTrip`.

Trip/path data includes latitude, longitude, altitude and speed samples. This is
valuable for UI/schema fixtures and for keeping Mercedes' application-facing
terminology intact.

Crucially, this file does **not** expose DiagLogic `dataId` values,
`requestedDeviceId`, responding ECU addresses, UDS/KWP requests, DIDs,
payload extraction rules or formulas. It therefore does not advance a factory
live parameter from `corroborated-unmapped` to a protocol-mapped state.

## Demo cockpit live-data trace

The archived `assets/demo_mode_cockptit.json` is materially stronger than the
general demo dataset because it is a time-offset live cockpit trace keyed by the
same Mercedes DataID strings used by Whisper/DiagLogic.

The trace contains exactly these live channels:

- `tripScore`;
- `acceleration`;
- `engineOilTemperature`;
- `relativeAcceleratorPedalPosition`;
- `calculatedEngineLoad`;
- `intakeManifoldPressure`.

This directly corroborates that `engineOilTemperature`,
`relativeAcceleratorPedalPosition`, `calculatedEngineLoad` and
`intakeManifoldPressure` are not merely catalogue names: the archived
Mercedes application drives a live cockpit/demo stream using those exact
DataIDs.

Representative ranges in the supplied scripted trace are:

| DataID | Observed demo range |
| --- | ---: |
| `tripScore` | 74 .. 100 |
| `acceleration` | -3 .. +2 |
| `engineOilTemperature` | 8 .. 14 |
| `relativeAcceleratorPedalPosition` | 13 .. 62 |
| `calculatedEngineLoad` | 0 .. 82 |
| `intakeManifoldPressure` | 0.18 .. 1.09 |

For `intakeManifoldPressure`, the separate Whisper catalogue declares the
engineering unit as `bar`; the cockpit trace supplies floating-point values
in the 0.18..1.09 range. This is useful semantic corroboration, but the trace is
synthetic/demo data and does **not** establish whether the production value is
absolute manifold pressure, gauge boost pressure or some transformed display
quantity on every vehicle. MBLINK must not infer that distinction without the
actual request/result mapping and a physical capture.

Likewise, the trace contains no device/provider, request PDU, response prefix,
DID, byte offsets or scaling formula. All four vehicle-value DataIDs therefore
remain `corroborated-unmapped`; their evidence has become stronger, not
protocol-complete.

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
