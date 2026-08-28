// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/telemetry.h"

static const char *mblink_link_unit_name(uint32_t unit)
{
    return mblink_obd2_unit_name((MblinkObd2Unit)unit);
}

static const char *mblink_link_result_name(uint32_t result)
{
    return mblink_elm327_result_name((MblinkElm327Result)result);
}

bool mblink_telemetry_recorder_begin(
    MblinkTelemetryRecorder *recorder,
    const MblinkTelemetrySessionMetadata *metadata,
    MblinkTelemetryTextSink sink,
    void *context)
{
    return link_telemetry_recorder_begin(
        recorder, metadata, "mblink", sink, context);
}

bool mblink_telemetry_recorder_continue(
    MblinkTelemetryRecorder *recorder,
    const MblinkTelemetrySessionMetadata *metadata,
    MblinkTelemetryTextSink sink,
    void *context)
{
    return link_telemetry_recorder_continue(
        recorder, metadata, "mblink", sink, context);
}

bool mblink_telemetry_recorder_record_sample(
    MblinkTelemetryRecorder *recorder,
    const MblinkTelemetrySample *sample,
    bool favourite)
{
    if (sample == NULL) {
        return false;
    }
    return link_telemetry_recorder_record_sample_named(
        recorder,
        sample,
        favourite,
        mblink_obd2_pid_name(sample->measurement.pid),
        mblink_obd2_unit_name((MblinkObd2Unit)sample->measurement.unit));
}

bool mblink_telemetry_recorder_record_response(
    MblinkTelemetryRecorder *recorder,
    uint64_t timestamp_ms,
    const char *command,
    const MblinkElm327Response *response)
{
    if (response == NULL) {
        return false;
    }
    return link_telemetry_recorder_record_response_named(
        recorder,
        timestamp_ms,
        command,
        mblink_elm327_result_name(response->result),
        response->text);
}

bool mblink_telemetry_export_csv(
    const MblinkTelemetryStore *store,
    const MblinkTelemetrySessionMetadata *metadata,
    MblinkTelemetryTextSink sink,
    void *context)
{
    return link_telemetry_export_csv_named(
        store,
        metadata,
        "mblink",
        mblink_obd2_pid_name,
        mblink_link_unit_name,
        mblink_link_result_name,
        sink,
        context);
}
