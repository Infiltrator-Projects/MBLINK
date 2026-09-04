// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/telemetry.h"

bool mblink_telemetry_store_record(
    MblinkTelemetryStore *store,
    uint64_t timestamp_ms,
    const MblinkObd2Sample *measurement)
{
    return link_telemetry_store_record_obd2_sample(
        store, timestamp_ms, measurement);
}

bool mblink_telemetry_store_record_transcript(
    MblinkTelemetryStore *store,
    uint64_t timestamp_ms,
    const char *command,
    const MblinkElm327Response *response)
{
    return link_telemetry_store_record_elm327_transcript(
        store, timestamp_ms, command, response);
}
