<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Telemetry, Scheduling and Logging

Scheduling, canonical sample state and session formatting live in portable C. Apple code coordinates transport/presentation only.

## Default OBD-II schedule

`MblinkScheduler` is bounded to 64 items and uses caller-supplied monotonic milliseconds.

| PID | Value | Interval | Priority |
| --- | --- | ---: | --- |
| `0C` | RPM | 500 ms | Critical |
| `0D` | Speed | 750 ms | High |
| `0B` | MAP | 1000 ms | High |
| `23` | Fuel rail gauge pressure | 1000 ms | High |
| `2F` | Fuel tank level | 5000 ms | Low |
| `49` | Accelerator pedal D | 1000 ms | High |
| `4A` | Accelerator pedal E | 1000 ms | High |
| `4C` | Commanded throttle actuator | 1000 ms | High |
| `7A` | DPF differential pressure | 1500 ms | High |
| `7C` | DPF inlet temperature | 2000 ms | High |
| `11` | Absolute throttle valve position | 1000 ms | High |
| `04` | Load | 1500 ms | Normal |
| `10` | MAF | 1500 ms | Normal |
| `2C` | Commanded EGR | 1500 ms | Normal |
| `2D` | EGR error | 2000 ms | Normal |
| `45` | Relative throttle position | 1500 ms | Normal |
| `47` | Absolute throttle B | 1500 ms | Normal |
| `48` | Absolute throttle C | 1500 ms | Normal |
| `4B` | Accelerator pedal F | 1500 ms | Normal |
| `5E` | Engine fuel rate | 1500 ms | Normal |
| `78` | Exhaust-gas temperature | 2000 ms | Normal |
| `3C` | Catalyst temperature | 3000 ms | Normal |
| `05` | Coolant | 3000 ms | Low |
| `0F` | Intake air | 3000 ms | Low |
| `33` | Barometric pressure | 3000 ms | Low |
| `42` | Control-module voltage | 3000 ms | Low |
| `46` | Ambient air | 5000 ms | Low |
| `5C` | Engine oil temperature | 3000 ms | Low |

Only PIDs advertised by the vehicle are scheduled. These are software defaults, not guaranteed hardware sample rates.

Delayed dispatch advances the next deadline past `now` rather than replaying every missed poll. Pause/resume shifts outstanding deadlines by the pause duration, which allows future exclusive operations to suspend routine polling without a burst on resume.

## Telemetry store

`MblinkTelemetryStore` keeps:

- latest value per PID;
- a 512-sample chronological recent-history ring;
- total sample count independent of ring retention;
- favourites;
- a 64-entry bounded recent command/response transcript.

`mblink_telemetry_store_clear_samples()` clears samples/latest values/transcript and sequence counters while preserving favourites.

## Full-session recorder

`MblinkTelemetryRecorder` streams C-formatted records through a caller-provided text sink. The core therefore does not require a filesystem or Apple framework.

A sink/write failure is terminal for that recorder instance. Once `failed` is set, record/finish calls return false until `mblink_telemetry_recorder_init()` resets the recorder. This prevents continued output after a partial/corrupt row.

The stream records original normalized ELM responses rather than the shortened recent-transcript cache.

Streaming schema v2 adds `responder_can_id` and `responder_extended` columns.
When a functional Mode 01 request receives several replies, each decoded sample
is written once with the exact CAN source that supplied it. Legacy samples remain
valid with those columns empty, and transcript rows retain the full normalized
response. This makes exported evidence directly usable for per-module live-data
replay instead of requiring CAN addresses to be reconstructed from transcript
text.

## Time domains and CSV

Session start/end metadata uses Unix epoch milliseconds. Sample/transcript rows use monotonic elapsed milliseconds from the session start so wall-clock changes cannot distort drive-relative timing.

The streaming CSV is the complete session mechanism. Snapshot export contains only retained recent history/transcript. CSV quoting and numeric formatting are owned by C.

The current iPhone bridge keeps its stream in memory for sharing. Evidence begins before Bluetooth scanning, so adapter-not-found and other pre-ELM failures are retained. Manual disconnect/reconnect attempts append distinct, closed session boundaries to the same CSV stream for the lifetime of the controller, so a later successful connection does not erase an earlier failure. When adapter or VIN discovery completes, the current session header is rewritten with that identifier before export. Durable file-backed storage across application termination remains a later product hardening task, not a change to the portable recorder contract.


## Link-load control

The schedule table defines the maximum default cadence for a supported, enabled PID. It does not mean every advertised PID must run simultaneously. Product faces can disable individual PIDs at runtime; LINK then skips those scheduler items entirely. Re-enabling a PID resumes its configured cadence without requiring a vehicle reconnect.

This runtime policy is distinct from favourites, visibility and unit formatting. A favourite can be off, a non-favourite can be polled, and language/unit choices never change which diagnostic requests are transmitted.


## Evidence snapshots while live

CSV preparation must not change diagnostic state. Apple products take an immutable recorder-byte snapshot and perform file-system writing asynchronously. This keeps Bluetooth callbacks, ELM327 session timeouts and scheduler ticks responsive even when a long session has accumulated a large evidence file.


## C207 / Vgate field load evidence

A real iPhone/Vgate/C207 capture from MBLINK 0.7.79 recorded 3,412 completed live PID exchanges across about 247 seconds of live polling, or roughly 13.8 completed requests per second. The previous cadence set kept the serial/BLE ELM path effectively continuously occupied: even the nominal 250 ms RPM request achieved only about 2.6 Hz because other due PIDs were constantly competing for the same single-command channel.

LINK 0.14.25 therefore deliberately lowers the default live-data request budget. The scheduler remains capability-gated and each PID remains independently switchable; this change reduces background pressure rather than limiting what can be selected.

The same evidence snapshot successfully contained 6,865 total records without a BLE-disconnect record. This separates CSV preparation from the later live-request timeout and supports the asynchronous snapshot/export design.


## Persistent opt-in selection

Polling cadence and polling selection are separate. The scheduler contains the supported definitions and their maximum cadence, but MBLINK 0.7.81 starts a new user profile with every live PID disabled. The application persists the exact enabled stable-key set and reapplies it before the first live request after a later launch or reconnect.
