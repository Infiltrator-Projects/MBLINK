<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Apple BLE and iPhone Layer

MBLINK 0.4.0 introduced the Apple platform edge while preserving the C-first architecture. MBLINK 0.5.0 extends that iPhone front end with the portable C scheduler, telemetry store, session recorder, favourites, charts and CSV sharing.

**Physical Vgate/iPhone/vehicle validation remains pending and is separate from software release status.**

## Layer boundary

The Apple side remains deliberately split:

1. `MBLinkBLETransport` owns CoreBluetooth and implements the platform side of `MblinkTransport`.
2. `MBLinkDiagnosticsController` coordinates that provider with the portable C ELM327, OBD-II, scheduler and telemetry APIs.
3. SwiftUI presents typed state, charts and sharing controls. It does not parse ELM327 responses, calculate PID values, choose polling rates or format the diagnostic session data.

```text
SwiftUI presentation
       |
Objective-C application bridge
       |
libmblink C11
  ELM327 / OBD-II / scheduler / telemetry
       |
MblinkTransport C ABI
       |
Objective-C CoreBluetooth provider
       |
BLE adapter
```

## CoreBluetooth policy

The provider acts only as a Bluetooth central. It:

- creates `CBCentralManager` on the main queue;
- scans only after Bluetooth reports `poweredOn`;
- connects to likely OBD adapters;
- discovers services and characteristics at runtime;
- forms writable + notify/indicate candidate channels within one service;
- validates each candidate with an `ATI` conversation handled by the portable C ELM327 parser;
- subscribes to notifications before normal diagnostic traffic;
- honours `maximumWriteValueLengthForType:`;
- obeys `canSendWriteWithoutResponse` and the CoreBluetooth readiness callback;
- serialises write-with-response traffic;
- caps queued application writes;
- forwards received application bytes without OBD interpretation to the C session engine.

The app does not invent or hard-code Vgate GATT UUIDs before they have been observed on the physical adapter.

## Discovery and recovery deadlines

BLE operations have explicit deadlines for:

- adapter scanning;
- peripheral connection;
- service/characteristic discovery;
- each candidate GATT-channel probe.

A probe deadline begins before waiting for write-without-response capacity, so a candidate cannot hang indefinitely if CoreBluetooth never becomes writable.

Connection/discovery failures and unexpected disconnects use controlled recovery while the user still wants the connection. Explicit user disconnect cancels outstanding generations and prevents automatic reconnection.

## GATT channel validation

MBLINK:

1. discovers all services;
2. discovers all characteristics;
3. forms write + notify/indicate pairs within each service;
4. ranks candidates deterministically;
5. enables notifications for one candidate;
6. frames `ATI` through `mblink_elm327_build_command()`;
7. feeds notification fragments into `mblink_elm327_parser_feed()`;
8. accepts the channel only when `mblink_elm327_parser_finish()` returns a valid non-empty identity response;
9. advances after an invalid, errored or timed-out candidate.

Objective-C must not grow a second ELM327 text parser. CR-only, LF-only, fragmented, echoed and adapter-error responses belong to the portable C parser and regression suite.

## C diagnostic integration

`MBLinkBLETransportMakeCTransport()` returns the existing `MblinkTransport` ABI. CoreBluetooth objects remain owned by the Objective-C provider.

The diagnostics controller then:

- creates `MblinkElm327Session` with the Apple transport;
- runs the C adapter initialisation sequence;
- queries Mode 01 PID `00` through the C OBD-II builder;
- builds the standard live schedule from the C supported-PID set;
- asks `mblink_scheduler_next()` which PID is due;
- sends one C-built request at a time through the C ELM session;
- decodes responses with `mblink_obd2_decode_live_pid()`;
- records typed samples through `mblink_telemetry_store_record()`;
- streams samples and original command/response records through `MblinkTelemetryRecorder`;
- exposes final typed values/history/favourites/export data to SwiftUI.

The default 0.5 dashboard schedule covers advertised RPM, speed, MAP, throttle, load, MAF, coolant and intake-air temperature.

See [Telemetry, Scheduling and Logging](TELEMETRY.md).

## Swift concurrency boundary

CoreBluetooth and diagnostics-controller delegate delivery are main-queue-bound. The Objective-C controller exposes this contract to Swift with `NS_SWIFT_UI_ACTOR`; the Swift view model remains `@MainActor`.

The project does not use `@preconcurrency` or disable Swift concurrency checking to hide isolation mismatches.

## SwiftUI scope in 0.5

The iPhone front end now presents:

- BLE/diagnostic connection state;
- peripheral name and adapter identity;
- eight standard live OBD-II values when available;
- parameter favourites;
- recent RPM and coolant charts;
- total recorded-sample count;
- CSV preparation and iOS sharing.

These are presentation features over C-owned state. Poll rates, canonical history retention and CSV formatting remain outside Swift.

## Privacy and background behaviour

The iOS target supplies `NSBluetoothAlwaysUsageDescription` for CoreBluetooth access.

MBLINK does **not** declare the `bluetooth-central` background execution mode. Background diagnostic recording has not yet been designed or validated, so the app does not request that capability prematurely.

## Build validation

CI tests the portable C core on Ubuntu and macOS. From 0.5 onward, the native iPhone target is built in both **Debug and Release** configurations for a generic iOS Simulator destination with code signing disabled.

CI also verifies architectural boundaries by checking that:

- the Apple BLE provider still uses the C ELM parser;
- the diagnostics controller calls the C scheduler;
- samples go through the C telemetry store;
- full session sample/response records go through the C streaming recorder;
- all four Xcode marketing-version entries match repository `VERSION`.

Simulator success proves source/framework integration. It does not prove physical BLE radio behaviour or vehicle communication.

## Physical hardware validation pending

The following observations remain for the physical adapter:

- confirm iPhone discovery of the Vgate iCar Pro BLE 4.0;
- record actual GATT service/characteristic UUIDs;
- confirm the C-parsed `ATI` probe reliably selects the correct channel;
- verify writes/notifications under sustained scheduled traffic;
- verify C ELM327 initialisation on the physical adapter;
- receive Mode 01 PID `00` from the development vehicle;
- confirm scheduled live values are stable and plausible;
- exercise scan timeout/cancel behaviour on-device;
- exercise unexpected disconnect/reconnect behaviour;
- measure realistic sustainable polling rates;
- document adapter-specific quirks behind a narrow adapter boundary.

These remain validation items rather than claims satisfied by simulator CI. Real hardware defects should be corrected as focused patches with regression coverage where practical.
