<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# MBLINK Roadmap

MBLINK is developed from the portable C core outward. Each milestone must leave the repository buildable, tested and architecturally reusable.

**Current development target: 0.7 — UDS foundation in C.**

## 0.1 — C foundation and dependency discipline

**Status: complete.**

- Portable C11 `libmblink`.
- Public headers under `include/mblink/`.
- Platform-neutral C transport ABI.
- Infiltratr Common 1.5.0 pinned at `a0e75ffbe4e038c74c8f1e3d589f2dae87b2b7bb`.
- Strict warning-as-error builds and portable smoke tests.
- Ubuntu/macOS CI.

**Exit condition:** a real portable C library builds and tests with its shared dependency pinned and verified. **Satisfied.**

## 0.2 — ELM327 command engine in C

**Status: complete.**

- Bounded command construction and response normalisation.
- Prompt/echo/error handling.
- Adapter initialisation and capability/protocol probing.
- One-command-at-a-time transport-backed execution.
- Monotonic timeouts, cancellation and re-synchronisation protection.
- Deterministic mock-transport tests.

See [ELM327 Engine](ELM327.md).

**Exit condition:** ELM327 conversations are testable without BLE hardware. **Satisfied.**

## 0.3 — Standard OBD-II C engine

**Status: complete.**

- Supported-PID enumeration.
- Typed common Mode 01 values.
- Mode 09 VIN extraction.
- Stored, pending and permanent DTC decoding.
- Freeze-frame and readiness decoding.
- Explicitly gated Mode 04 clearing.
- Strict malformed-input/error handling.
- Indexed long-response reassembly.

See [Standard OBD-II Engine](OBD2.md).

**Exit condition:** `libmblink` is useful as a generic standard OBD-II engine independent of iOS. **Satisfied.**

## 0.4 — Apple BLE provider and iPhone shell

**Status: software implementation complete; physical hardware validation pending.**

Primary reference adapter: **Vgate iCar Pro BLE 4.0**.

- Objective-C CoreBluetooth provider implementing the C transport boundary.
- Runtime GATT discovery and C-parsed `ATI` probing.
- Explicit scan/connect/discovery/probe deadlines.
- Controlled reconnect/rescan behaviour.
- Bounded write queues and CoreBluetooth backpressure handling.
- Thin Objective-C bridge and native SwiftUI shell.
- Debug/Release Xcode Simulator CI.

See [Apple BLE and iPhone Layer](APPLE.md).

**Software exit condition:** Apple integration compiles under Xcode CI while preserving the C ownership boundary. **Satisfied.**

**Outstanding physical validation:** discover/connect to the reference adapter on an iPhone, record actual GATT UUIDs, complete C ELM initialisation against the vehicle, verify Mode 01 PID `00`, exercise live values and reconnect behaviour, and document real adapter quirks.

## 0.5 — Scheduler, dashboard and logging

**Status: complete.**

- Portable C request scheduler with rates/priorities.
- Capability-driven eight-PID default schedule.
- Pause/resume and delayed catch-up without burst polling.
- Typed latest-value cache and bounded recent history.
- C-owned favourites.
- Full-session streaming recorder independent of recent history.
- Complete diagnostic command/response transcript in the streaming recorder.
- C-formatted CSV/session metadata using Infiltratr Common.
- SwiftUI dashboard, favourites, charts and iOS share/export.
- Ubuntu/macOS C tests and Debug/Release Xcode gates.

See [Telemetry, Scheduling and Logging](TELEMETRY.md).

**Exit condition:** scheduling/logging remain owned by the portable core rather than SwiftUI/CoreBluetooth. **Satisfied.**

## 0.6 — ISO-TP foundation in C

**Status: complete.**

- Portable ISO-TP transmit and receive state machines.
- Classical CAN with 8-byte data frames and a 4095-byte PDU ceiling.
- 11-bit and 29-bit CAN identifiers.
- Normal, extended and mixed addressing model.
- Physical and functional target classification.
- Single Frame, First Frame, Consecutive Frame and Flow Control handling.
- Segmentation and caller-owned bounded reassembly.
- Four-bit sequence validation including `0xF -> 0x0` wrap.
- Receiver block-size Flow Control.
- CTS, WAIT and OVERFLOW handling.
- Configurable WAIT-frame limit.
- STmin support for `0x00..0x7F` and sub-millisecond `0xF1..0xF9` values.
- Explicit receive and Flow Control timeouts using a caller-supplied monotonic microsecond clock.
- Saturating deadline arithmetic via Infiltratr Common.
- Deterministic failed states requiring explicit reset.
- Explicit rejection of unsupported extended-length First Frames and functional multi-frame transmission.
- Portable regression tests including end-to-end segmentation/reassembly and sequence wrap.
- Ubuntu/macOS C tests and Debug/Release Xcode compilation of the same C implementation.

See [ISO-TP Foundation](ISOTP.md).

**Exit condition:** transport-layer ISO-TP tests pass without Mercedes-specific or UDS logic. **Satisfied.**

## 0.7 — UDS foundation in C

**Status: current target.**

- Request/response model.
- Positive and negative response handling.
- ECU identification.
- Diagnostic-session management.
- Data-identifier abstraction.
- Timing/state handling for verified services.
- Reuse ISO-TP PDUs without duplicating segmentation or CAN addressing.

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
