<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Apple BLE and iPhone Layer

MBLINK 0.4.0 introduces the Apple platform edge while preserving the C-first architecture. The software integration is released after passing portable C CI on Ubuntu and macOS plus an iOS Simulator Xcode build. **Physical Vgate/iPhone/vehicle validation remains pending and is documented separately from the software release status.**

## Layer boundary

The Apple side is deliberately split into three pieces:

1. `MBLinkBLETransport` owns CoreBluetooth and implements the platform side of `MblinkTransport`.
2. `MBLinkDiagnosticsController` coordinates that provider with the existing C ELM327 and OBD-II engines.
3. SwiftUI presents connection state and already-decoded values. It does not parse ELM327 responses or implement PID formulas.

```text
SwiftUI
   |
Objective-C application bridge
   |
libmblink C11
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
- validates each candidate with an `ATI` conversation handled by the **portable C ELM327 parser**;
- subscribes to notifications before normal diagnostic traffic;
- honours `maximumWriteValueLengthForType:`;
- obeys `canSendWriteWithoutResponse` and the CoreBluetooth readiness callback;
- serialises write-with-response traffic through its completion callback;
- caps queued application writes instead of allowing unbounded memory growth;
- forwards received application bytes without OBD interpretation to the C session engine.

The app does not invent or hard-code Vgate GATT UUIDs before they have been observed and validated on the actual adapter.

## Discovery and recovery deadlines

BLE operations are bounded. The provider has explicit deadlines for:

- adapter scanning;
- peripheral connection;
- service/characteristic discovery;
- each candidate GATT-channel probe.

A candidate probe deadline starts **before** waiting for write-without-response capacity, preventing a candidate from hanging indefinitely if CoreBluetooth never becomes writable.

Connection and discovery failures are treated as transient and cause a controlled disconnect followed by a fresh scan while the user still wants the connection. Unexpected peripheral disconnects use the same recovery path. Explicit user disconnect cancels all outstanding generations and prevents automatic reconnection.

A complete scan with no matching adapter, or exhaustion of all GATT candidates without a valid ELM327 response, enters a visible failed state rather than looping forever.

## Device discovery

The initial discovery policy gives highest priority to `IOS-Vlink` and also accepts names containing `vlink`, `vgate`, `icar`, `obd` or `elm`.

The name is only a discovery hint. A peripheral is not accepted as an ELM command channel merely because its name matches. A candidate must expose appropriate GATT properties and complete a valid C-parsed `ATI` response.

## GATT channel validation

A connected peripheral may expose several services and characteristics. MBLINK therefore:

1. discovers all services;
2. discovers all characteristics;
3. forms write + notify/indicate pairs within the same service;
4. ranks candidates deterministically;
5. enables notifications for one candidate;
6. frames `ATI` through `mblink_elm327_build_command()`;
7. feeds notification fragments into `mblink_elm327_parser_feed()`;
8. accepts the channel only when `mblink_elm327_parser_finish()` returns a valid non-empty identity response;
9. advances to the next candidate after an invalid, errored or timed-out probe.

This is an architectural boundary, not merely an implementation detail: Objective-C must not grow a second ELM327 text parser. CR-only, LF-only, fragmented, echoed and adapter-error responses belong to the existing C parser and its portable regression suite.

## C transport integration

`MBLinkBLETransportMakeCTransport()` returns the existing `MblinkTransport` ABI. The provider owns CoreBluetooth objects; the returned C structure only references that provider.

The Objective-C diagnostics controller then:

- creates `MblinkElm327Session` with the Apple transport;
- runs the C `ATZ` / `ATE0` / `ATL0` / `ATS0` / `ATH0` / `ATSP0` / `ATI` initialisation sequence;
- queries Mode 01 PID `00` through the C OBD-II builder;
- checks the C supported-PID set;
- polls standard RPM (`0C`) and coolant temperature (`05`) only when advertised;
- decodes those responses with `mblink_obd2_decode_live_pid()`;
- exposes only final typed values to SwiftUI.

## Swift concurrency boundary

CoreBluetooth is instantiated on the main queue, the diagnostics timer runs on the main queue, and application delegate delivery is main-queue-bound. The Objective-C diagnostics controller exposes that real contract to Swift with `NS_SWIFT_UI_ACTOR`; the Swift view model remains `@MainActor`.

The project does not use `@preconcurrency` or disable Swift concurrency checking to hide an isolation mismatch.

## SwiftUI scope

The initial iPhone shell shows:

- BLE/diagnostic connection state;
- discovered peripheral name;
- adapter identity;
- engine RPM when supported;
- coolant temperature when supported;
- connect/disconnect controls.

Dashboard configuration, graphing and logging remain milestone 0.5 work.

## Privacy and background behaviour

The iOS target supplies `NSBluetoothAlwaysUsageDescription` for CoreBluetooth access.

MBLINK 0.4.0 does **not** declare the `bluetooth-central` background execution mode. Background diagnostic logging has not yet been designed or validated, so the app does not request that capability prematurely.

## Build validation

The repository CI tests the portable C core on Ubuntu and macOS and builds the native iPhone target with `xcodebuild` for a generic iOS Simulator destination with code signing disabled.

The Xcode target:

- links a dedicated `MBLINKCore` static library;
- compiles the same MBLINK and Infiltratr Common C sources used by the portable build;
- compiles the Objective-C CoreBluetooth provider and application bridge;
- compiles the SwiftUI shell;
- treats Swift and C warnings as errors;
- keeps the Xcode marketing version synchronised with repository `VERSION`.

The 0.4.0 software release passed all three CI jobs before publication. Simulator success proves source and Apple-framework integration; it does **not** prove real BLE radio behaviour or vehicle communication.

## Portable probe regressions

The C ELM327 suite explicitly covers probe-shaped `ATI` responses using carriage-return line endings, including:

- echoed `ATI` plus a valid adapter identity;
- `ATI` followed by `?`;
- `ATI` followed by `ERROR`.

Error responses must be classified by C and must not be exposed to the Apple layer as adapter identity text.

## Physical hardware validation pending

The following observations remain to be completed when the adapter is available:

- confirm the iPhone discovers the Vgate iCar Pro BLE 4.0;
- record the actual GATT service and characteristic UUIDs;
- confirm the C-parsed `ATI` probe selects the correct channel repeatedly;
- verify writes and notifications under sustained command traffic;
- verify the C ELM327 initialisation completes on the physical adapter;
- receive Mode 01 PID `00` from the development Mercedes;
- confirm RPM and coolant values are stable and plausible;
- exercise scan timeout and explicit cancel behaviour on-device;
- exercise unexpected disconnect/reconnect behaviour;
- document any Vgate-specific quirks behind a narrow adapter boundary rather than leaking them into `libmblink`.

These are **validation items, not claims already satisfied by 0.4.0**. If real hardware exposes defects or quirks, they should be corrected as focused 0.4.x patches with captured evidence and regression coverage where practical.
