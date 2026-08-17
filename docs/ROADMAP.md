# MBLINK Roadmap

MBLINK will be developed in small, testable stages. Standard OBD-II comes first, followed by logging and diagnostics, then progressively deeper Mercedes-Benz support.

## 0.1 — BLE connection and standard live data

Primary target: iPhone + Vgate iCar Pro BLE 4.0.

- Native SwiftUI application shell.
- CoreBluetooth adapter discovery and connection.
- Runtime GATT service/characteristic discovery.
- ELM327-compatible command channel.
- Adapter reset and initialisation sequence.
- Automatic OBD protocol selection.
- Supported-PID enumeration.
- Live RPM.
- Coolant temperature.
- Vehicle speed.
- Manifold absolute pressure.
- Mass-air-flow rate.
- Intake-air temperature.
- Throttle position.
- Calculated engine load.
- Connection and error status UI.
- Unit tests for parser and standard PID decoding.

**Exit condition:** an iPhone can connect to the target Vgate adapter in the development Mercedes, identify supported standard PIDs and display stable live engine data.

## 0.2 — Standard diagnostics

- VIN and ECU identification where supported.
- Stored DTCs.
- Pending DTCs.
- Permanent DTCs where available.
- Human-readable standard DTC descriptions.
- Freeze-frame data.
- Readiness monitors.
- Clear DTC operation behind an explicit user action.
- Better timeout, retry and malformed-response handling.
- Session transcript/debug view for development.

**Exit condition:** MBLINK is useful as a conventional OBD-II diagnostic app even without Mercedes-specific extensions.

## 0.3 — Dashboard, graphs and logging

- Configurable live-data dashboard.
- Parameter selection and favourites.
- Multiple polling rates.
- Time-series graphs.
- Drive/session recording.
- Timestamped samples.
- CSV export.
- Session metadata such as VIN, connection time and selected parameters.
- Graceful handling of app background/foreground transitions.

**Exit condition:** live vehicle behaviour can be recorded and reviewed rather than merely watched on screen.

## 0.4 — Protocol foundation for manufacturer diagnostics

- CAN-header control through compatible ELM327 commands.
- ISO-TP segmentation and reassembly.
- UDS request/response support.
- ECU addressing model.
- Diagnostic-session management.
- Data-identifier abstraction.
- Captured-response fixtures for repeatable testing.

**Exit condition:** the core can communicate with verified non-generic diagnostic services without embedding Mercedes logic in the transport layer.

## 0.5 — Mercedes-Benz C207 / OM651 engine diagnostics

Initial areas to investigate and validate:

- ECU identity and software information.
- DPF differential pressure.
- DPF and exhaust temperature data.
- Regeneration state and related counters where exposed.
- Turbocharger command/actual information.
- Boost-related data beyond generic Mode 01 values.
- Fuel-rail target and actual pressure.
- Injector-related values where exposed.
- EGR command/actual information.
- Additional diesel-specific temperatures and status data.

Every Mercedes-specific parameter must be labelled according to its validation status. Experimental definitions should remain clearly separated from verified definitions.

**Exit condition:** MBLINK provides materially more useful information on the development Mercedes than a generic OBD-II application.

## 0.6 — Additional Mercedes control modules

Subject to adapter and vehicle-network access:

- Transmission.
- ABS / ESP.
- SRS / airbag.
- Climate control.
- Instrument cluster.
- Body modules.
- Other discoverable control units.

This stage includes module discovery, ECU identity, DTC access and selected live data before any write-oriented functionality is considered.

## 0.7 — Adapter portability

- Formal adapter-profile mechanism.
- Test additional ELM327-compatible BLE adapters.
- Record known quirks and capability differences.
- Capability probing rather than adapter-name assumptions.
- Optional support for other transports where useful.

The Vgate iCar Pro BLE 4.0 remains the first development target, not a permanent architectural dependency.

## 0.8 — User experience and release hardening

- Saved vehicle profiles.
- Automatic reconnect.
- Dashboard presets.
- Search/filter for parameters and DTCs.
- Clear presentation of raw versus verified manufacturer data.
- Better diagnostic-session history.
- Accessibility and larger-text review.
- Performance and battery-use optimisation.
- Crash/error telemetry only if implemented in a privacy-respecting, opt-in manner.
- Documentation for building and contributing.

## Later possibilities

These are deliberately outside the early scope and should only be considered once read-only diagnostics are mature:

- Service functions.
- Adaptations.
- Coding.
- Additional manufacturers.
- macOS or other front ends reusing `MBLINKCore`.

Firmware flashing, immobiliser/security programming and similarly high-impact functions are not part of the initial roadmap.

## Development principle

Each milestone should leave the repository in a useful, testable state. New Mercedes functionality should extend the generic platform rather than turning MBLINK into a collection of vehicle-specific shortcuts.
