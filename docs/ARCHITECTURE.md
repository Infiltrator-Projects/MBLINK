# MBLINK Architecture

MBLINK is deliberately split into layers so the diagnostic engine is not tied to one Bluetooth adapter, one vehicle, or one user interface.

The design principle is simple: **vehicle diagnostics belong in reusable, testable core code; Bluetooth belongs at the edge.**

## High-level structure

```text
SwiftUI application
       |
       v
Application / vehicle layer
       |
       v
MBLINKCore
  |- command model
  |- ELM327 response parsing
  |- standard OBD-II services
  |- PID decoding
  |- DTC decoding
  |- polling/session management
  |- ISO-TP (planned)
  |- UDS (planned)
  `- manufacturer extensions (planned)
       |
       v
OBDCommandTransport
       |
       +-- Vgate iCar Pro BLE transport
       +-- other BLE ELM327 transports
       `-- future transport types
       |
       v
Adapter -> OBD-II connector -> vehicle networks -> ECUs
```

## 1. User-interface layer

The iPhone application is native SwiftUI.

Its responsibilities are presentation and user interaction rather than protocol implementation. Screens can request live parameters, diagnostics, logging or module information from the application layer without knowing how a BLE packet is framed or how an OBD response is decoded.

Planned UI areas include:

- Connection and vehicle status.
- Live dashboard.
- Parameter browser.
- Diagnostic trouble codes.
- Freeze-frame and readiness information.
- Graphs and session logging.
- Mercedes-Benz-specific screens.
- Module discovery and module-specific diagnostics.

## 2. Application and vehicle layer

This layer turns generic diagnostic capabilities into useful vehicle-facing features.

A vehicle profile can declare known control modules, preferred parameters, verified manufacturer-specific identifiers and presentation metadata. The initial profile will focus on the Mercedes-Benz C207 / OM651 development vehicle, but generic OBD-II operation must remain available without any Mercedes profile loaded.

Vehicle profiles must not duplicate the protocol engine. They supply definitions and interpretation where manufacturer-specific knowledge is required.

## 3. MBLINKCore

`MBLINKCore` is the reusable diagnostic engine.

It must remain independent of SwiftUI and CoreBluetooth so it can be unit-tested on its own and potentially reused by other front ends later.

Core responsibilities include:

- ELM327 command construction.
- Adapter initialisation sequencing.
- Response normalisation and parsing.
- Standard SAE OBD-II services and PID discovery.
- PID value decoding and unit representation.
- DTC parsing.
- Vehicle identification queries.
- Diagnostic session and request scheduling.
- Timeouts, retries and transport error handling.
- Later ISO-TP segmentation/reassembly.
- Later UDS diagnostic services.
- A clean extension point for Mercedes-Benz definitions.

The core communicates through an abstract `OBDCommandTransport`. It must not know whether commands travel over Bluetooth LE, Wi-Fi or another future link.

## 4. Transport layer

The transport layer owns the physical adapter connection.

The first target is the **Vgate iCar Pro BLE 4.0** using Apple CoreBluetooth.

Responsibilities include:

- BLE scanning and adapter discovery.
- Connecting and disconnecting.
- GATT service and characteristic discovery.
- Selecting suitable write and notification characteristics.
- Converting command strings to BLE writes.
- Collecting incoming notifications into adapter responses.
- Respecting negotiated write sizes rather than assuming a fixed BLE payload.
- Reporting connection and transport errors to the core.

MBLINK will not assume that every ELM327-compatible BLE product exposes identical service or characteristic UUIDs. Adapter-specific knowledge belongs in transport implementations or adapter profiles, never in the diagnostic core.

## 5. Standard OBD-II layer

Standard OBD-II provides the first reliable cross-vehicle capability.

Initial support is centred on:

- Mode 01 current powertrain data.
- Supported-PID enumeration.
- Mode 03 stored DTCs.
- Mode 04 DTC clearing with explicit user action.
- Mode 07 pending DTCs.
- Mode 09 vehicle information such as VIN where supported.
- Permanent DTC support where available.
- Freeze-frame and readiness information.

PID support is capability-driven: MBLINK asks the ECU which PID ranges it supports and only polls valid parameters.

## 6. ISO-TP and UDS

Deeper Mercedes-Benz diagnostics require moving beyond generic OBD-II requests.

Planned protocol work includes ISO 15765-2 transport handling and Unified Diagnostic Services (UDS), including ECU identification, diagnostic sessions and data-identifier requests where applicable.

These layers will be implemented independently of Mercedes-specific definitions so they can be reused for other manufacturers and modules.

## 7. Mercedes-Benz extension layer

Mercedes-specific diagnostics sit above the generic transport and protocol code.

The initial development target is the C207 E-Class diesel / OM651 platform. Candidate areas include DPF state, exhaust temperatures, differential pressure, regeneration information, turbo control, fuel-rail pressure, injector-related information, EGR information and additional module diagnostics.

No manufacturer-specific identifier is treated as correct merely because it appears in an undocumented list. Definitions should record their source or test status and be validated against real vehicle responses before being promoted to stable support.

## 8. Request scheduling

Live-data polling must be controlled rather than flooding the adapter or vehicle bus.

The scheduler will support:

- Parameter groups with configurable rates.
- Priority for rapidly changing values such as RPM and boost.
- Slower polling for temperatures and status values.
- Pausing live polling while diagnostic operations require exclusive command access.
- Per-request timeout and retry policy.
- Timestamping values as close as practical to receipt.

## 9. Data model and logging

Decoded values should carry more than a display string. A live sample should retain a stable parameter identifier, timestamp, raw response where useful, decoded numeric value, unit and source ECU/module.

This allows the same data to drive dashboards, graphs, CSV export and later analysis without re-parsing display text.

## 10. Write operations and safety boundaries

The early architecture is read-focused.

Operations that alter diagnostic state, such as clearing DTCs, must be separated from ordinary reads and require an explicit user action. ECU coding, firmware flashing, immobiliser/security functions and other high-impact programming are outside the initial scope.

This separation also keeps the early test surface small while the transport and parsing layers are being validated against real hardware.

## 11. Testing strategy

Core functionality should be testable without an adapter or vehicle.

Tests will cover:

- Response cleanup and parsing.
- Standard PID formulas.
- DTC decoding.
- Supported-PID bitmaps.
- Timeout and malformed-response handling.
- ISO-TP segmentation/reassembly when implemented.
- Mercedes data decoding using captured, verified responses.

The BLE transport will be tested separately on an actual iPhone with the target adapter.

## Design rule

If adding support for a different adapter or vehicle requires rewriting generic OBD-II parsing, the abstraction is wrong. Adapter quirks belong in transports, manufacturer knowledge belongs in vehicle extensions, and reusable protocol behaviour belongs in `MBLINKCore`.
