<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK is developed from the portable C core outward. Each milestone must leave the repository buildable, tested and architecturally reusable.

**Current development target: 0.6 — ISO-TP foundation in C.**

## 0.1 — C foundation and dependency discipline

**Status: complete.**

- Portable C11 `libmblink`.
- Public headers under `include/mblink/`.
- Platform-neutral C transport ABI.
- Infiltratr Common 1.5.0 pinned at `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`.
- Portable Common `core.c` and `format.c` integrated.
- Strict warning-as-error builds and C smoke tests.
- Ubuntu/macOS CI.

**Exit condition:** a real portable C library builds and tests with its shared dependency pinned and verified. **Satisfied.**

## 0.2 — ELM327 command engine in C

**Status: complete.**

- Bounded command model and construction.
- Response framing, normalisation and prompt detection.
- Echo handling and adapter error classification.
- Adapter reset/initialisation state machine.
- Capability and protocol probing.
- One-command-at-a-time transport-backed session execution.
- Monotonic timeout handling.
- Cancellation and re-synchronisation protection.
- Re-entrant completion protection.
- Deterministic mock-transport tests.

See [ELM327 Engine](ELM327.md).

**Exit condition:** ELM327 conversations are testable without BLE hardware, including fragmentation, ownership, timeouts, probing and error paths. **Satisfied.**

## 0.3 — Standard OBD-II C engine

**Status: complete.**

- Supported-PID enumeration.
- Typed RPM, coolant, speed, MAP, MAF, intake temperature, throttle and calculated load.
- Mode 09 VIN extraction.
- Stored, pending and permanent DTC decoding.
- Mode 02 freeze-frame decoding.
- Mode 01 PID 01 readiness decoding.
- Explicitly gated Mode 04 clearing.
- Strict malformed-hex and ELM-error handling.
- Indexed long-response reassembly for ELM-compatible adapters.
- Deterministic portable regression tests.

See [Standard OBD-II Engine](OBD2.md).

**Exit condition:** `libmblink` is useful as a generic standard OBD-II engine independent of iOS. **Satisfied.**

## 0.4 — Apple BLE provider and iPhone shell

**Status: software implementation complete; physical hardware validation pending.**

Primary reference adapter: **Vgate iCar Pro BLE 4.0**.

- Objective-C CoreBluetooth provider implementing the C transport boundary.
- Peripheral scan/connect/disconnect.
- Runtime service/characteristic discovery.
- C-parsed `ATI` candidate-channel probing.
- Notification subscription and negotiated write sizing.
- Explicit scan/connect/discovery/probe deadlines.
- Controlled reconnect/rescan behaviour.
- Bounded write queues and CoreBluetooth backpressure handling.
- Thin Objective-C bridge to Swift.
- Native SwiftUI connection/live-data shell.
- Xcode Simulator CI.

No diagnostic parser or PID formula is duplicated in Swift/Objective-C.

See [Apple BLE and iPhone Layer](APPLE.md).

**Software exit condition:** Apple integration compiles under Xcode CI while preserving the C ownership boundary. **Satisfied.**

**Outstanding physical validation:** discover/connect to the reference adapter on an iPhone, record actual GATT UUIDs, complete C ELM initialisation against the vehicle, verify Mode 01 PID `00`, exercise live values and reconnect behaviour, and document any real adapter quirks. This validation remains active in parallel and is not represented as already complete.

## 0.5 — Scheduler, dashboard and logging

**Status: complete.**

- Portable C request scheduler with parameter rates and priorities.
- Fast polling priority for rapidly changing values.
- Lower rates for temperatures and slower state.
- Pause/resume semantics for future exclusive diagnostic operations.
- Delayed-request catch-up without burst polling.
- Capability-driven standard eight-PID schedule.
- Typed latest-value cache.
- Bounded chronological recent-history ring for dashboard/graphs.
- Total sample count independent of recent-history retention.
- C-owned parameter favourites.
- Full-session streaming recorder independent of the recent-history ring.
- Full diagnostic command/response transcript in the streaming recorder.
- Bounded recent transcript cache for UI/debug history.
- Session start/end metadata.
- C-formatted CSV output using Infiltratr Common formatting/string contracts.
- SwiftUI dashboard for the eight standard live values.
- Favourites presentation.
- RPM and coolant time-series charts.
- iOS share/export path for the C-generated session data.
- Ubuntu/macOS C tests and Debug/Release Xcode Simulator release gates.

See [Telemetry, Scheduling and Logging](TELEMETRY.md).

**Exit condition:** live polling, recent history, dashboard state and complete session recording operate without moving scheduling/logging policy into SwiftUI or CoreBluetooth. **Satisfied in software CI.**

## 0.6 — ISO-TP foundation in C

**Status: current target.**

- CAN addressing model.
- Single-frame handling.
- Multi-frame first/consecutive frame handling.
- Segmentation and reassembly.
- Sequence-number validation.
- Flow-control handling where required by the transport model.
- Protocol timeouts and explicit state transitions.
- Bounded buffers and malformed-frame handling.
- Captured-frame regression fixtures.

**Exit condition:** transport-layer ISO-TP tests pass without Mercedes-specific logic.

## 0.7 — UDS foundation in C

- Request/response model.
- Positive and negative response handling.
- ECU identification.
- Diagnostic-session management.
- Data-identifier abstraction.
- Timing/state handling for verified services.

**Exit condition:** verified UDS exchanges execute through the reusable C engine without manufacturer interpretation in the transport.

## 0.8 — Mercedes-Benz C207 / OM651 engine diagnostics

Investigate and validate:

- ECU identity/software information;
- DPF differential pressure;
- DPF and exhaust temperatures;
- regeneration state/counters where exposed;
- turbo target/actual information;
- boost beyond generic Mode 01 values;
- fuel-rail target/actual pressure;
- injector-related values where exposed;
- EGR command/actual information;
- additional diesel-specific state and temperature values.

Every Mercedes parameter carries a validation status. Experimental definitions remain separate from verified definitions.

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

Initial scope is module identification, DTCs and selected live data before write-oriented services are considered.

## 0.10 — Adapter portability

- Formal adapter capability/profile model.
- Additional ELM327-compatible BLE adapters.
- Known firmware/command quirks.
- Capability probing rather than name assumptions.
- Optional future Wi-Fi/native transports.

The reference Vgate adapter is a validation target, not a permanent dependency of `libmblink`.

## 1.0 — Release hardening

- Stable documented C ABI for supported functionality.
- Public API documentation.
- Complete fixture provenance/validation metadata.
- Saved vehicle profiles and reconnect behaviour.
- Accessibility review of the iPhone front end.
- Performance and battery-use profiling.
- File-backed/long-session storage hardening as appropriate.
- Reproducible release process.
- Contributor/build documentation.
- Security/privacy review before any telemetry leaves the device.

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
