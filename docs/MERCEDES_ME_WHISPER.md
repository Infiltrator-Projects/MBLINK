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

## Backend-delivered Vehicle Interface configuration

The decompiled Android framework now proves the configuration-download path
that was previously only inferred from native symbols.

The application contains a dedicated Adapter Configuration Service (ACS)
component. `AdapterConfigurationComponentImpl.downloadConfiguration(...)`
builds a `GetConfigurationRequest` from an
`AdapterConfigurationDeviceDescriptor`. Before sending it, the component
requires all four descriptor identities to be present:

```text
productID
deviceID
thingID
groupID
```

If an installed configuration already exists, the request also carries its
`version` and `externalID`; a forced/no-installed request sends those as
empty strings. The request is POSTed to the configured ACS endpoint through
`ACSConnectorImpl`, with authentication headers supplied either by an
application `AuthHeaderProvider` or the toolkit `AuthManagement`
authenticator.

The ACS response contains at least:

```text
uri
version
externalID
```

An empty `uri` means that no update is available. Otherwise the application
downloads the returned URI as raw bytes, stores the payload in a temporary
file, and records the download as a `DOWNLOAD_FINISHED` event together with
the response version/external ID.

Installation is a separate step. The application reads those exact downloaded
bytes back from the temporary file and calls:

```text
VehicleInterfaceFacadeFactory.SINGLETON
  .getVehicleInterface()
  .configureVehicleInterface(fileBytes, progressMonitor)
```

After installation it reports the result back to ACS, records
`INSTALL_SUCCESSFUL` or `INSTALL_NOT_SUCCESSFUL`, and deletes the temporary
file.

This is direct source evidence that the production application can obtain a
device-specific Vehicle Interface configuration from a backend repository and
install it dynamically. Combined with the supplied
`libtsivehicleinterface.so` / Whisper evidence, this materially strengthens
the conclusion that the detailed post-VIN diagnostic mappings need not be
bundled in the APK.

The ACS endpoint itself is not hard-coded in the Java class. The default
configuration obtains it from the Android resource
`R.string.acc_endpoint_uri`. Its exact string value has not yet been
recovered from the supplied source bundle.

The next highest-value recovery targets are therefore:

1. `GetConfigurationRequest.java` and `GetConfigurationResponse.java` for
   the complete wire model;
2. `AdapterConfigurationDeviceDescriptor.java` and the code that constructs
   it, to determine how vehicle/VIN/adapter identity selects a package;
3. the Android resource value for `acc_endpoint_uri`;
4. `VehicleInterfaceFacadeFactory` and the concrete
   `configureVehicleInterface(byte[], ...)` bridge, to prove exactly how the
   downloaded payload enters `libtsivehicleinterface.so` / Whisper; and
5. any retained/cached downloaded configuration file from a historical app
   installation, which would be the most direct route to the missing
   DataID-to-request/result/formula mappings.

## Exact Vehicle Interface bundle installation bridge

The Java/JNA and native bridge for installing an ACS-delivered package is now
source-backed end to end.

The ACS request model is exactly:

```text
productID
groupID
deviceID
thingID
version
externalID
```

where `version` and `externalID` are empty for a forced/first download and
carry the currently installed package identity otherwise.

The ACS response model contains exactly:

```text
uri
version
externalID
```

The downloaded bytes are passed to
`VehicleInterfaceFacadeImpl.configureVehicleInterface(...)`, which obtains
the registered `IVehicleInterface` configurator, creates an
`IConfigurationBundle` directly from the byte array and calls
`configure(bundle, progressMonitor)`.

The concrete Android implementation loads the native library
`libtsivehicleinterface.so` through JNA. Its C ABI is:

```c
void configure(
    byte *data,
    int length,
    progressPerformedCallback,
    progressFinishedCallback);
```

Native disassembly of the supplied `libtsivehicleinterface.so` shows that
this function writes the caller-supplied byte buffer into an in-memory
stringstream, obtains
`whisper::configuration::ConfigurationFactory::getConfigurator()`, wraps
the stream as an input stream, wraps the Java callbacks as a Whisper progress
monitor, and invokes the Whisper configurator.

The supplied `libwhisper.so` closes the remaining format gap. Its exported
configuration symbols include:

```text
Configurator::configure(...)
Configurator::getFileCount(...)
Configurator::extractConfigurationFiles(...)
Configurator::createReport(...)
DecompressHandler
Poco::Zip::ZipLocalFileHeader
Poco::Zip::AutoDetectInputStream
```

and the same binary contains the configuration directory names
`deviceproviders`, `actionproviders`, `vinmapping` and `_configs`.

Therefore the ACS `uri` does not point to an opaque firmware blob: the
downloaded Vehicle Interface payload is a ZIP-format Whisper configuration
bundle that is streamed directly into the Whisper configuration installer.

This is the strongest evidence so far for where the missing Mercedes
DataID-to-provider/request/result/formula mappings live. A recovered historical
ACS bundle would be directly inspectable using the already recovered Whisper
configuration/decryption tooling.

The exact semantics of `productID`, `groupID`, `deviceID` and
`thingID` are still not established by the classes currently recovered.
Likewise, the default ACS base endpoint remains referenced indirectly via the
Android resource `R.string.acc_endpoint_uri`.

## ACS selection identity and Whisper VIN mapping

The recovered Java narrows the ACS selection model further.

The Adapter Configuration tracking database does **not** key installed/downloaded
state on all four descriptor fields. Its lookup predicate is specifically:

```text
event type
+ productID
+ thingID
```

and its persisted tracking row stores `productID`, `thingID`, `version`,
`externalID` and the temporary downloaded-file URL. `deviceID` and
`groupID` are still present in the ACS request model, but are not used by this
local package-history lookup.

This makes `thingID` a particularly important package-selection identity, but
the recovered classes still do not establish its semantic meaning. It must not
yet be labelled as VIN, adapter serial number or any other specific identifier.

Separately, native Whisper symbols prove that VIN-dependent configuration
selection exists *inside the vehicle-interface configuration layer*. The
supplied `libwhisper.so` exports:

```text
VinMappingRegistry::getMapForVin(...)
VinMappingRegistry::determineKey(...)
VinMappingRegistry::countMatchingCharacters(...)
VinMappingRegistry::writeVinConfigurations(...)
VinMappingRegistry::removeVinConfigurations(...)
VinMappingRegistry::CONFIGS_POSTFIX()
DpConfigurationController::getActiveConfiguration()
DpConfigurationController::setActiveConfiguration(...)
DeviceProviderFactory::getActiveConfigurationName()
```

and contains the configuration namespace literals `vinmapping`,
`deviceproviders`, `actionproviders` and `_configs`.

Therefore two selection stages must be kept distinct:

1. ACS chooses which Vehicle Interface ZIP to deliver from the generic
   descriptor/request identities; and
2. once installed, Whisper has its own VIN-mapping registry capable of selecting
   configuration content for a VIN and activating the corresponding device
   provider configuration.

This is stronger evidence for the route to a C207/OM651 mapping than assuming
that the ACS `thingID` itself is a VIN. The remaining Java target is the
application code that constructs `AdapterConfigurationDeviceDescriptor`;
the remaining artifact target is any historical downloaded ACS bundle or
installed Whisper `_configs` content.

## Exact ACS descriptor construction in the Mercedes app

The application-layer caller has now been recovered from the main DEX.

Class `y2.c` constructs every
`AdapterConfigurationDeviceDescriptor` in one helper. Its fields are set as:

```text
productID = Android string resource 2131887000
groupID   = Android string resource 2131887002
deviceID  = "8497e115-32ef-4738-b048-9f206ee43b10"
thingID   = caller-supplied string
```

The same descriptor is used for both ACS download and installation.

This materially narrows the remaining package-selection problem. `deviceID`
is a fixed application constant in this archived build, while `thingID` is
the only descriptor field populated dynamically by the immediate caller.

The caller has now been traced into `TripManagerImpl`. Its injected
`adapterConfigManager` is the `y2.a` interface implemented by this
descriptor builder, and after a trip stops the application calls:

```java
this.f6181s.c(tripVO.v());
```

Within the same decompiled class, `TripVO.v()` is consistently treated as
the vehicle VIN; multiple methods explicitly validate their corresponding
string argument with Kotlin's generated parameter name `"vin"`. Therefore,
for this production call path:

```text
thingID = VIN
```

This is source-backed, not inferred.

The class also performs a weekly refresh policy: it checks the timestamps of
the most recent `DOWNLOAD_FINISHED`, `DOWNLOAD_FAILED` and
`DOWNLOAD_STARTED` events for the same descriptor and forces a new download
when the newest event is older than 604800000 ms (7 days).

The merged Android resource table resolves the descriptor resources and their
literal values:

```text
0x7f120398 / 2131887000 = envFrameworkProductId      = "Daimler"
0x7f120399 / 2131887001 = envFrameworkServerTimeout = "10000"
0x7f12039a / 2131887002 = envFrameworkTenantId      = "d6d7c781-932f-42ca-9ee7-280a72480f37"
```

Therefore the application-layer descriptor construction can be expressed
exactly for this archived build as:

```text
productID = "Daimler"
groupID   = "d6d7c781-932f-42ca-9ee7-280a72480f37"
deviceID  = "8497e115-32ef-4738-b048-9f206ee43b10"
thingID   = VIN
```

The same tenant UUID is reused by the application's diagnosis/backend
configuration as both `tenantId` and `xClientId`, while the associated
server timeout is 10000 ms.

The ACS endpoint resource has also been resolved through the merged Android
resource table. The library-facing Java symbol

```text
R.string.acc_endpoint_uri = 0x7f12007d
```

maps in the final application resource table to:

```text
0x7f12007d = string "acc.endpoint_uri"
```

The name change is a resource-merge/aapt normalization detail; it explains why
searching the decoded resources for the underscore form returned no value.

The decoded final string resource resolves the ACS endpoint to:

```text
https://ws41.caritc.de/services-dg/acs_core_api/acs
```

Therefore the archived application's ACS configuration request is sent to that
endpoint with the source-backed descriptor values:

```text
productID = "Daimler"
groupID   = "d6d7c781-932f-42ca-9ee7-280a72480f37"
deviceID  = "8497e115-32ef-4738-b048-9f206ee43b10"
thingID   = VIN
version   = installed version or ""
externalID = installed externalID or ""
```

Authentication headers are supplied separately by the app/toolkit
authentication layer and are not fabricated here.

## ACS authentication path from the production application

The production application does not use the toolkit's generic authentication
fallback for ACS. Application class `y3.b` installs its own
`AuthHeaderProvider` on the `AdapterConfigurationComponent` before starting
the component.

The provider obtains the current OpenShift/backend authentication headers from
the application's `AuthenticatorOpenShift` path:

```text
MBFAApplication.authManager
  -> AuthenticatorOpenShift
  -> auth header array
  -> AuthHeaderProvider map
  -> ACSConnectorImpl.getAuthHeaders()
```

The ACS provider rejects an empty authentication-header set and augments the
returned backend headers with the following fixed/client metadata:

```text
X-APP-VERSION = 4.7.61
X-APP-TARGET  = live
X-OS          = Android
X-OS-VERSION  = Build.VERSION.RELEASE
X-MANUFACTURER = Build.MANUFACTURER
X-PHONE-TYPE   = Build.MODEL
```

`ACSConnectorImpl` applies that same header map to the ACS configuration
request and to the subsequent configuration-file download URI. The toolkit
fallback through `AuthManagement.getAuthenticator(...).getAuthHeaders()`
therefore exists, but is not the primary application path in this build.

The same application class overrides the component configuration rather than
using only the library defaults. Its endpoint is assembled as:

```text
string resource 0x7f120389 + string resource 0x7f120385
```

and its encrypted local tracking-database password is derived from:

```text
PBKDF2WithHmacSHA1(
  password = Android secure android_id,
  salt     = app-specific s0.a() byte array,
  iterations = 1000,
  keyLength  = 256 bits
)
```

The database-password derivation is local storage protection and must not be
confused with ACS authorization, Whisper configuration encryption, or the
adapter Session Master Key.

The exact names/values of the OpenShift-supplied authentication headers still
need to be recovered from the `AuthenticatorOpenShift` implementation, and
resources `0x7f120389` and `0x7f120385` should be resolved to verify the
application-level ACS URL assembly against the already recovered merged
endpoint.

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

The Java/JNA bridge has now been recovered from the decompiled Android
application. `HwDevice.start(...)` wraps the application provider in
`SessionMasterKeyProviderCallback` and passes it directly to the native
`IHwDeviceLibrary.start(...)` entry point. The callback validates that both
arguments are non-blank and then delegates to
`ISessionMasterKeyProvider.loadSessionMasterKey(adapterId, passKey)`.

The production provider is `OpenShiftSessionMasterKeyProvider`. It builds a
JSON-style request object containing exactly one field, `passKey`, and POSTs
it to the relative backend endpoint:

```text
tenants/{tenantId}/obdAdapters/{obdAdapterId}/v2/smk
```

The tenant ID comes from `BackendAppApiConfiguration.getTenantId()`; the
adapter ID is the first callback argument. The response model contains a single
`smk` string, which is returned unchanged to GDK. Backend failures are encoded
for the native caller as `#<HTTP>` or `#<HTTP>:<application-error>`.

This closes the application-side SMK call chain:

```text
GDK asks adapter for passkey
  -> JNA SessionMasterKeyProviderCallback(adapterId, passKey)
  -> OpenShiftSessionMasterKeyProvider
  -> POST tenants/{tenantId}/obdAdapters/{obdAdapterId}/v2/smk
       body: { passKey: ... }
  -> response: { smk: ... }
  -> GDK Base64-decodes the returned 32-byte Session Master Key
  -> SessionMasterKeyCache / live session-key derivation
```

The backend base URL, authentication headers/token machinery and tenant
selection are defined elsewhere in the decompiled backend configuration layer
and are not claimed here yet.

This path is Mercedes-me-adapter authentication infrastructure, not the
vehicle-side live-data request catalogue. Recovering it is useful for complete
adapter interoperability, but the higher-value MBLINK live-data target remains
the downloaded/selected Whisper vehicle configuration that maps DataIDs to
providers, request PDUs, response extraction and formulas.


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
