<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK is developed from the portable C core outward. Each milestone must leave the repository buildable, testable and architecturally reusable.

**Current development target: 0.4 — Apple BLE provider and iPhone shell.**

## 0.1 — C foundation and dependency discipline

**Status: complete.**

- Establish `libmblink` as a portable C11 library.
- Publish stable public headers under `include/mblink/`.
- Define the platform-neutral C transport ABI.
- Pin Infiltratr Common 1.5.0 at commit `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`.
- Compile Common `core.c` and `format.c` into the portable foundation.
- Reuse Common project identity and shared primitives instead of private duplicates.
- Add strict compiler warnings and C smoke tests.
- Add Linux/macOS CI for the portable library.

**Exit condition:** the repository builds and tests a real portable C library with its shared dependency pinned and verified.

## 0.2 — ELM327 command engine in C

**Status: complete.**

- Command model and bounded command construction.
- Response framing/normalisation.
- Prompt detection and echo handling.
- Adapter reset/initialisation state machine.
- Capability probing.
- Automatic OBD protocol selection policy.
- One-command-at-a-time transport-backed session execution.
- Monotonic timeout handling with checked deadline arithmetic.
- Cancellation and explicit re-synchronisation protection.
- Re-entrant completion callback protection.
- Clean malformed-response and transport-error behaviour.
- Deterministic mock transport tests.

The implementation is documented in [ELM327 Engine](ELM327.md).

**Exit condition:** ELM327 conversations can be parsed and exercised completely without BLE hardware, including fragmented responses, command ownership, timeouts, capability probing and error paths.

## 0.3 — Standard OBD-II C engine

**Status: complete.**

- Supported-PID enumeration across standard 32-PID blocks.
- Typed live RPM.
- Coolant temperature.
- Vehicle speed.
- Manifold absolute pressure.
- Mass-air-flow rate.
- Intake-air temperature.
- Throttle position.
- Calculated engine load.
- Mode 09 PID 02 VIN extraction.
- Stored/pending/permanent DTC decoding.
- Mode 02 freeze-frame decoding using the same typed PID formulas.
- Mode 01 PID 01 readiness decoding for spark/compression metadata and monitor masks.
- Explicitly gated Mode 04 DTC clearing with readiness-reset acknowledgement.
- Strict malformed hexadecimal and ELM-error handling.
- Deterministic portable regression tests.

The implementation is documented in [Standard OBD-II Engine](OBD2.md).

**Exit condition:** `libmblink` is useful as a generic OBD-II diagnostic engine independent of iOS.

## 0.4 — Apple BLE provider and iPhone shell

Primary hardware target: **Vgate iCar Pro BLE 4.0**.

- Objective-C CoreBluetooth provider implementing the C transport boundary.
- Peripheral scan/connect/disconnect.
- Runtime service/characteristic discovery.
- Notification subscription and write-size handling.
- Adapter-profile/quirk boundary.
- Thin application bridge to Swift.
- Native SwiftUI connection and live-data screens.

No diagnostic parser or PID formula is duplicated in Swift/Objective-C.

**Exit condition:** an iPhone can connect to the target Vgate adapter in the development Mercedes and display stable values produced by the C core.

## 0.5 — Scheduler, dashboard and logging

- C request scheduler with parameter groups and rates.
- Priority for rapidly changing values.
- Controlled pauses for exclusive operations.
- Typed sample model.
- Configurable dashboard.
- Parameter favourites.
- Time-series graphs.
- Session recording.
- CSV export.
- Session metadata and diagnostic transcript support.

**Exit condition:** drives can be captured, reviewed and exported without coupling logging logic to the UI.

## 0.6 — ISO-TP foundation in C

- CAN addressing model.
- Single/multi-frame handling.
- Segmentation and reassembly.
- Flow-control handling where required by the adapter/transport model.
- Timeouts and sequence validation.
- Captured-frame regression fixtures.

**Exit condition:** transport-layer protocol tests pass without Mercedes-specific logic.

## 0.7 — UDS foundation in C

- Request/response model.
- Positive/negative response handling.
- ECU identification.
- Diagnostic-session management.
- Data-identifier abstraction.
- Timing/state handling required by verified services.

**Exit condition:** verified UDS exchanges can be executed through the generic C engine without embedding manufacturer interpretation in the transport.

## 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics

Investigate and validate:

- ECU identity/software information;
- DPF differential pressure;
- DPF and exhaust temperature data;
- regeneration state and counters where exposed;
- turbo target/actual information;
- boost data beyond generic Mode 01 values;
- fuel-rail target/actual pressure;
- injector-related values where exposed;
- EGR command/actual information;
- additional diesel-specific state/temperature values.

Every Mercedes parameter carries a validation status. Experimental definitions remain separated from verified definitions.

**Exit condition:** MBLINK provides materially deeper, verified information on the development Mercedes than a generic OBD-II application.

## 0.9 — Additional Mercedes modules

Subject to adapter and vehicle-network access:

- transmission;
- ABS/ESP;
- SRS/airbag;
- climate control;
- instrument cluster;
- body modules;
- other discoverable ECUs.

Initial scope is module identification, DTCs and selected live data before write-oriented service functions are considered.

## 0.10 — Adapter portability

- Formal adapter capability/profile model.
- Additional ELM327-compatible BLE adapters.
- Known firmware/command quirks.
- Capability probing rather than name assumptions.
- Optional future Wi-Fi/native transports.

The Vgate iCar Pro BLE 4.0 is the validation target, not a permanent dependency of `libmblink`.

## 1.0 — Release hardening

- Stable documented C ABI for supported functionality.
- Public API documentation.
- Complete test-fixture provenance/validation metadata.
- Saved vehicle profiles and reconnect behaviour.
- Accessibility review of the iPhone front end.
- Performance and battery-use profiling.
- Reproducible release process.
- Contributor/build documentation.
- Security and privacy review of any telemetry before such functionality is considered.

## Later possibilities

Only after read-focused diagnostics are mature:

- service functions;
- adaptations;
- coding;
- additional manufacturers;
- macOS/Linux/Windows front ends using the same C engine;
- command-line tooling for development and captured-session analysis.

Firmware flashing, immobiliser/security programming and similarly high-impact programming are outside the early roadmap.

## Development principle

Each feature begins at the lowest reusable layer that can correctly own it. UI convenience is never a reason to duplicate protocol logic outside the C core.
