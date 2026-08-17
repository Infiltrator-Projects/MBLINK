# MBLINK

**MBLINK** is an open-source native iPhone vehicle diagnostics project, beginning with Mercedes-Benz and designed around affordable ELM327-compatible Bluetooth Low Energy (BLE) adapters.

The initial development vehicle is a Mercedes-Benz C207 E-Class diesel with the OM651 engine, and the initial hardware target is the **Vgate iCar Pro BLE 4.0**. The architecture is deliberately not tied to either one car or one adapter: standard OBD-II functionality lives in a reusable core, adapter communication is isolated behind a transport interface, and manufacturer-specific diagnostics are added as separate vehicle modules.

The goal is to grow MBLINK from a useful live-data and fault-code app into a deeper Mercedes diagnostic platform without sacrificing the generic OBD-II foundation.

## Project goals

MBLINK is being built to provide:

- Native iPhone support using Swift, SwiftUI and CoreBluetooth.
- Direct BLE communication with ELM327-compatible adapters.
- Standard SAE OBD-II live data and diagnostics.
- Fault-code reading, freeze-frame information, readiness data and controlled clearing of DTCs.
- Configurable live dashboards and time-series graphs.
- Drive/session logging and CSV export.
- A reusable transport layer so different BLE adapters can be supported without changing the diagnostic engine.
- Mercedes-Benz-specific diagnostics layered above the generic OBD-II implementation.
- Initial deep Mercedes support focused on the C207 / OM651 platform.
- Later access to additional control modules such as transmission, ABS/ESP, SRS, climate and body systems where the vehicle and adapter permit it.

## Hardware target

The first adapter target is the **Vgate iCar Pro BLE 4.0**.

The project does not assume that every ELM327-compatible BLE adapter exposes the same GATT service or characteristic UUIDs. Adapter discovery and BLE framing belong in the transport layer. This keeps the OBD-II and Mercedes diagnostic code independent of the physical dongle.

Additional adapters can be added by implementing the same transport interface.

## Architecture

```text
iPhone / SwiftUI
       |
       v
Vehicle & presentation layer
       |
       v
MBLINKCore
  |- OBD-II commands and PID decoding
  |- response parsing
  |- diagnostic sessions
  |- ISO-TP / UDS (planned)
  `- Mercedes diagnostic definitions (planned)
       |
       v
OBDCommandTransport
       |
       +-- Vgate BLE transport
       +-- other ELM327 BLE transports
       `-- future transports
       |
       v
OBD-II diagnostic connector -> vehicle networks -> ECUs
```

The core diagnostic code has no dependency on CoreBluetooth. It sends commands through an abstract transport and receives adapter responses back through that interface. That separation allows the parser, PID decoder and diagnostic logic to be tested without a vehicle or Bluetooth device attached.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the detailed design.

## Development stages

The first working milestone is deliberately small and testable:

1. Discover and connect to the Vgate adapter from an iPhone.
2. Establish the ELM327 command channel.
3. Initialise the adapter and automatically select the vehicle protocol.
4. Enumerate supported standard OBD-II PIDs.
5. Read and display live RPM and coolant temperature.
6. Expand to speed, manifold pressure, MAF, intake temperature, throttle and engine load.
7. Add fault-code and vehicle-identification functions.
8. Add logging, graphs and configurable dashboards.
9. Implement ISO-TP/UDS and verified Mercedes-specific diagnostics.

See [docs/ROADMAP.md](docs/ROADMAP.md) for the longer development plan.

## Mercedes-Benz direction

Standard OBD-II is only the starting point. MBLINK is intended to progressively support deeper Mercedes-Benz diagnostics, including data not exposed by generic Mode 01 PIDs.

For the C207 / OM651 development platform this may include, where verified and available from the relevant control module:

- DPF differential pressure, temperature and regeneration information.
- Turbocharger and boost-control information.
- Fuel-rail pressure and injector-related data.
- EGR data.
- ECU identification and addressing.
- Transmission and chassis-module diagnostics.

Mercedes-specific requests and interpretations will be added only when they have been identified and validated. They will remain separate from the generic OBD-II implementation.

## Safety and scope

Early MBLINK releases are intentionally read-focused. Operations that change vehicle state, such as clearing diagnostic trouble codes, will require an explicit user action. Coding, flashing and security-sensitive ECU programming are outside the initial scope.

## Status

**Pre-alpha / foundation stage.**

The repository is being established before hardware testing begins. The first hardware target is the Vgate iCar Pro BLE 4.0 and the first application target is iPhone.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Roadmap](docs/ROADMAP.md)
- [Adapter strategy](docs/ADAPTERS.md)

## Licence

MBLINK is released under the **GNU General Public License v3.0 or later (GPL-3.0-or-later)**.

See [LICENSE](LICENSE) for the full licence text.
