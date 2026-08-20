<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Apple BLE and iPhone Layer

The Apple side is a platform edge around the portable C core. Physical iPhone installation has been validated; live Vgate/vehicle communication remains pending and is separate from simulator/build status.

## Boundary

`MBLinkBLETransport` owns CoreBluetooth and implements `MblinkTransport`. `MBLinkDiagnosticsController` coordinates the provider with the C diagnostic APIs. SwiftUI presents typed state and user actions.

Objective-C/Swift do not own ELM327 parsing, OBD-II formulas, polling policy, canonical telemetry state, ISO-TP/UDS or Mercedes definitions.

## CoreBluetooth provider

The provider:

- waits for Bluetooth readiness before scanning;
- discovers services/characteristics at runtime;
- forms writable + notify/indicate candidates within one service;
- validates candidates with `ATI` framed and parsed by the portable C ELM327 engine;
- subscribes to notifications before application traffic;
- respects negotiated maximum write size and CoreBluetooth backpressure;
- serializes write-with-response traffic and bounds queued bytes;
- forwards received bytes to the C session without diagnostic interpretation.

No Vgate GATT UUID is hard-coded before it has been observed on physical hardware.

Scan, connect, discovery and candidate-probe operations have explicit deadlines. Generation tokens invalidate stale asynchronous callbacks. Unexpected connection/discovery failures recover while the user still wants the connection; explicit disconnect cancels recovery.

## C integration

`MBLinkBLETransportMakeCTransport()` exposes the provider through the C transport ABI. The diagnostics controller then runs the C ELM initialization, discovers standard OBD-II capability, asks the C scheduler what is due, decodes through C, records C telemetry and exposes final typed values/history/favourites/export data to SwiftUI.

The iPhone core also compiles the portable ELM-managed ISO 15765 CAN-channel implementation used by CMake. Selecting a manufacturer ECU endpoint and running that channel through the live controller remain 0.8 discovery/integration work; source inclusion alone does not claim a working Mercedes exchange.

CoreBluetooth and diagnostics delegate delivery are main-queue-bound. Objective-C exposes that contract with `NS_SWIFT_UI_ACTOR`; the Swift view model remains `@MainActor` rather than suppressing concurrency diagnostics.

The iOS target declares the normal Bluetooth privacy description but does not request Bluetooth background execution. Background diagnostic recording has not been designed or validated.

## Build versus hardware validation

CI builds the same portable C implementation used by CMake plus the iPhone target in Debug and Release simulator configurations. Simulator success proves source/framework integration only.

Physical validation still needs to confirm:

- iPhone discovery/connection to the Vgate iCar Pro BLE 4.0;
- actual service/write/notify characteristic UUIDs;
- reliable C-parsed `ATI` candidate selection;
- ELM327 initialization against the adapter/vehicle;
- Mode 01 PID `00` and plausible scheduled live values;
- sustained write/notification behaviour and realistic polling rates;
- scan cancel/timeout and unexpected disconnect/reconnect behaviour;
- any real adapter quirks, kept behind the provider/profile boundary.

Hardware findings should become focused fixes and deterministic regression fixtures where practical, not platform-specific protocol forks.
