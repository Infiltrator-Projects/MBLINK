<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Telemetry, Scheduling and Logging

MBLINK 0.5 moves live polling, sample history and session recording into the portable C core. The Apple layer coordinates transport and presents results; it does not own polling policy or diagnostic data formatting.

## Scheduler ownership

`MblinkScheduler` is a bounded portable C scheduler with a maximum of 16 active items. Each item has:

- PID;
- polling interval;
- next due time;
- explicit priority;
- enabled state.

The default standard OBD-II plan is capability-driven and only schedules values advertised by the vehicle:

| PID | Value | Default interval | Priority |
| --- | --- | ---: | --- |
| `0C` | Engine RPM | 250 ms | Critical |
| `0D` | Vehicle speed | 500 ms | High |
| `0B` | Manifold absolute pressure | 500 ms | High |
| `11` | Throttle position | 500 ms | High |
| `04` | Calculated engine load | 750 ms | Normal |
| `10` | Mass air flow | 750 ms | Normal |
| `05` | Coolant temperature | 1500 ms | Low |
| `0F` | Intake-air temperature | 1500 ms | Low |

These are software defaults, not claims about a particular adapter or vehicle's achievable sample rate.

## Timing behaviour

Scheduler time is monotonic milliseconds. It is intentionally separate from wall-clock/Unix time.

When the diagnostic loop is delayed, `mblink_scheduler_mark_dispatched()` advances the next deadline beyond the current time instead of issuing every missed interval in a burst. This prevents an app resume, debugger pause or slow adapter from creating avoidable bus load.

Pause/resume is explicit. Resuming shifts outstanding deadlines by the paused duration, preserving relative schedule timing. This is intended for future exclusive operations such as DTC queries, ISO-TP/UDS exchanges or service functions that should temporarily suspend ordinary live polling.

Checked/saturating time arithmetic reuses Infiltratr Common.

## Typed telemetry store

`MblinkTelemetryStore` owns live sample state in portable C.

It provides:

- latest sample per PID;
- a chronological recent-history ring;
- total recorded sample count independent of ring retention;
- parameter favourites;
- a bounded recent command/response transcript.

The recent sample ring holds 512 samples. This ring exists for dashboards and graphs; it is **not** the complete drive recording.

The recent transcript cache holds 64 entries. Its response text is intentionally bounded because it is recent UI/debug state, not the archival source of truth.

## Full-session recorder

Complete session recording is separate from the recent rings.

`MblinkTelemetryRecorder` writes records through a caller-supplied text sink as they happen. This means the portable core does not require a filesystem, Objective-C object, SQLite database or a particular operating system.

The recorder receives the original normalized `MblinkElm327Response`, not the shortened recent-transcript cache. Long replies therefore remain complete in the streaming session record.

The current Apple bridge uses the streaming API and exposes the resulting session data for iOS sharing. A future file-backed or database-backed Apple sink can replace the current sink without changing scheduler, OBD-II or recorder contracts.

## Session timestamps

MBLINK uses two time domains deliberately:

- session start/end metadata uses Unix epoch milliseconds;
- sample and diagnostic transcript timestamps use monotonic elapsed milliseconds from the session start.

Elapsed timestamps remain stable if the device's wall clock changes during a drive, while epoch metadata still identifies when the session occurred.

## CSV formats

Two related C-generated formats exist.

The streaming session format begins with session metadata and then uses a single record table containing `sample` and `transcript` rows. It is designed to represent the complete running session as the sink receives records.

The snapshot exporter produces:

1. session metadata;
2. the currently retained recent sample history;
3. the currently retained recent diagnostic transcript.

Snapshot export is therefore useful for bounded in-memory inspection, while the streaming recorder is the complete-session mechanism.

CSV quoting is handled by the C core. Numeric formatting reuses Infiltratr Common and normalises decimal separators for machine-readable CSV output.

## Apple/iPhone integration

`MBLinkDiagnosticsController` asks the C scheduler which request is due, sends that request through the existing C ELM session, decodes the response with the C OBD-II engine and records the typed sample in the C telemetry store.

The Objective-C controller exposes typed current values, recent history, favourites and export data to Swift. SwiftUI then provides:

- eight standard live OBD-II rows;
- favourite rows;
- RPM and coolant history charts;
- recorded-sample count;
- CSV preparation and iOS sharing.

SwiftUI does not calculate PID values, choose polling intervals, maintain the canonical history ring or format the diagnostic CSV.

## CI contracts

Portable scheduler/telemetry behaviour is tested on Ubuntu and macOS under the same strict C11 warning-as-error policy as the rest of `libmblink`.

The iOS CI additionally verifies that the Apple controller continues to call the C scheduler, telemetry store and streaming recorder. Both Debug and Release iOS Simulator configurations are release gates from 0.5 onward.

## Hardware independence

No part of the 0.5 scheduler or telemetry implementation requires the physical BLE adapter. Real hardware may later justify rate adjustments or a narrow adapter-specific policy, but those observations must not move scheduling, OBD decoding or logging ownership out of the portable core.
