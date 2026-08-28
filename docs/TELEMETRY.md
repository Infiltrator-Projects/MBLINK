<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Telemetry, Scheduling and Logging

Scheduling, canonical sample state and session formatting live in portable C. Apple code coordinates transport/presentation only.

## Default OBD-II schedule

`MblinkScheduler` is bounded to 64 items and uses caller-supplied monotonic milliseconds.

| PID | Value | Interval | Priority |
| --- | --- | ---: | --- |
| `0C` | RPM | 250 ms | Critical |
| `0D` | Speed | 500 ms | High |
| `0B` | MAP | 500 ms | High |
| `11` | Throttle | 500 ms | High |
| `04` | Load | 750 ms | Normal |
| `10` | MAF | 750 ms | Normal |
| `05` | Coolant | 1500 ms | Low |
| `0F` | Intake air | 1500 ms | Low |

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

## Time domains and CSV

Session start/end metadata uses Unix epoch milliseconds. Sample/transcript rows use monotonic elapsed milliseconds from the session start so wall-clock changes cannot distort drive-relative timing.

The streaming CSV is the complete session mechanism. Snapshot export contains only retained recent history/transcript. CSV quoting and numeric formatting are owned by C.

The current iPhone bridge keeps its stream in memory for sharing. Evidence begins before Bluetooth scanning, so adapter-not-found and other pre-ELM failures are retained. Manual disconnect/reconnect attempts append distinct, closed session boundaries to the same CSV stream for the lifetime of the controller, so a later successful connection does not erase an earlier failure. When adapter or VIN discovery completes, the current session header is rewritten with that identifier before export. Durable file-backed storage across application termination remains a later product hardening task, not a change to the portable recorder contract.
