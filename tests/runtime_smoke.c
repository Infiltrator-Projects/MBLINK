// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/scheduler.h"
#include "mblink/telemetry.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

typedef struct {
    char data[65536];
    size_t length;
} TextBuffer;

static bool text_sink(void *context, const char *bytes, size_t length)
{
    TextBuffer *buffer = context;
    if (buffer == NULL || bytes == NULL ||
        length > sizeof(buffer->data) - buffer->length - 1U) {
        return false;
    }
    memcpy(buffer->data + buffer->length, bytes, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
    return true;
}

static void set_supported(MblinkObd2PidSet *set, uint8_t pid)
{
    set->bits[pid / 8U] |= (uint8_t)(1U << (pid % 8U));
}

static int test_scheduler(void)
{
    MblinkScheduler scheduler;
    mblink_scheduler_init(&scheduler);
    CHECK(mblink_scheduler_add(&scheduler, 0x05U, 1000U,
                               MBLINK_SCHEDULER_PRIORITY_LOW, 100U) ==
          MBLINK_SCHEDULER_RESULT_OK);
    CHECK(mblink_scheduler_add(&scheduler, 0x0cU, 250U,
                               MBLINK_SCHEDULER_PRIORITY_CRITICAL, 100U) ==
          MBLINK_SCHEDULER_RESULT_OK);
    CHECK(mblink_scheduler_add(&scheduler, 0x0cU, 500U,
                               MBLINK_SCHEDULER_PRIORITY_HIGH, 100U) ==
          MBLINK_SCHEDULER_RESULT_DUPLICATE);

    MblinkSchedulerDispatch dispatch;
    CHECK(mblink_scheduler_next(&scheduler, 90U, &dispatch) ==
          MBLINK_SCHEDULER_NEXT_WAITING);
    CHECK(dispatch.wait_ms == 10U);
    CHECK(mblink_scheduler_next(&scheduler, 100U, &dispatch) ==
          MBLINK_SCHEDULER_NEXT_READY);
    CHECK(dispatch.pid == 0x0cU);
    CHECK(mblink_scheduler_mark_dispatched(
              &scheduler, dispatch.index, 100U) ==
          MBLINK_SCHEDULER_RESULT_OK);
    CHECK(scheduler.items[dispatch.index].next_due_ms == 350U);

    mblink_scheduler_set_paused(&scheduler, true, 200U);
    CHECK(mblink_scheduler_next(&scheduler, 500U, &dispatch) ==
          MBLINK_SCHEDULER_NEXT_PAUSED);
    mblink_scheduler_set_paused(&scheduler, false, 700U);
    CHECK(scheduler.items[0].next_due_ms == 600U);
    CHECK(scheduler.items[1].next_due_ms == 850U);

    CHECK(mblink_scheduler_next(&scheduler, 700U, &dispatch) ==
          MBLINK_SCHEDULER_NEXT_READY);
    CHECK(dispatch.pid == 0x05U);
    CHECK(mblink_scheduler_mark_dispatched(
              &scheduler, dispatch.index, 2700U) ==
          MBLINK_SCHEDULER_RESULT_OK);
    CHECK(scheduler.items[dispatch.index].next_due_ms == 3600U);

    MblinkObd2PidSet supported = { { 0 } };
    set_supported(&supported, 0x0cU);
    set_supported(&supported, 0x05U);
    set_supported(&supported, 0x0dU);
    CHECK(mblink_scheduler_configure_standard_obd2(
              &scheduler, &supported, 1000U) ==
          MBLINK_SCHEDULER_RESULT_OK);
    CHECK(scheduler.count == 3U);
    CHECK(scheduler.items[0].pid == 0x0cU);
    CHECK(scheduler.items[1].pid == 0x0dU);
    CHECK(scheduler.items[2].pid == 0x05U);
    return 0;
}

static int test_telemetry(void)
{
    MblinkTelemetryStore store;
    mblink_telemetry_store_init(&store);
    mblink_telemetry_store_set_favourite(&store, 0x0cU, true);

    for (size_t index = 0U;
         index < MBLINK_TELEMETRY_HISTORY_CAPACITY + 3U;
         ++index) {
        MblinkObd2Sample measurement = {
            .pid = (index % 2U == 0U) ? 0x0cU : 0x05U,
            .value = (double)index + 0.25,
            .unit = (index % 2U == 0U)
                ? MBLINK_OBD2_UNIT_RPM
                : MBLINK_OBD2_UNIT_CELSIUS
        };
        CHECK(mblink_telemetry_store_record(
                  &store, 1000U + (uint64_t)index, &measurement));
    }

    CHECK(mblink_telemetry_store_history_count(&store) ==
          MBLINK_TELEMETRY_HISTORY_CAPACITY);
    CHECK(mblink_telemetry_store_total_sample_count(&store) ==
          MBLINK_TELEMETRY_HISTORY_CAPACITY + 3U);
    MblinkTelemetrySample oldest;
    CHECK(mblink_telemetry_store_history_at(&store, 0U, &oldest));
    CHECK(oldest.sequence == 3U);

    MblinkTelemetrySample latest;
    CHECK(mblink_telemetry_store_latest(&store, 0x0cU, &latest));
    CHECK(latest.measurement.pid == 0x0cU);
    CHECK(mblink_telemetry_store_is_favourite(&store, 0x0cU));
    CHECK(!mblink_telemetry_store_is_favourite(&store, 0x05U));

    MblinkElm327Response response = {
        .result = MBLINK_ELM327_RESULT_OK,
        .length = 8U
    };
    (void)snprintf(response.text, sizeof(response.text), "41 0C 1A F8");
    CHECK(mblink_telemetry_store_record_transcript(
              &store, 2000U, "010C", &response));
    CHECK(mblink_telemetry_store_transcript_count(&store) == 1U);
    MblinkTelemetryTranscriptEntry transcript;
    CHECK(mblink_telemetry_store_transcript_at(&store, 0U, &transcript));
    CHECK(strcmp(transcript.command, "010C") == 0);
    CHECK(strcmp(transcript.response, "41 0C 1A F8") == 0);

    MblinkTelemetrySessionMetadata metadata;
    mblink_telemetry_session_metadata_init(
        &metadata, 1234U, "ELM327, test \"adapter\"", "C207");
    mblink_telemetry_session_metadata_finish(&metadata, 5678U);

    TextBuffer output = { { 0 }, 0U };
    CHECK(mblink_telemetry_export_csv(
              &store, &metadata, text_sink, &output));
    CHECK(strstr(output.data, "# mblink_csv_version,1\n") != NULL);
    CHECK(strstr(output.data,
                 "# adapter_identifier,\"ELM327, test \"\"adapter\"\"\"\n") != NULL);
    CHECK(strstr(output.data,
                 "sequence,timestamp_ms,pid,name,value,unit,favourite\n") != NULL);
    CHECK(strstr(output.data, ",0x0C,\"Engine speed\",") != NULL);
    CHECK(strstr(output.data, "# diagnostic_transcript\n") != NULL);
    CHECK(strstr(output.data,
                 "timestamp_ms,command,result,response\n") != NULL);
    CHECK(strstr(output.data,
                 "2000,\"010C\",\"ok\",\"41 0C 1A F8\"\n") != NULL);

    TextBuffer stream = { { 0 }, 0U };
    MblinkTelemetryRecorder recorder;
    mblink_telemetry_recorder_init(&recorder);
    CHECK(mblink_telemetry_recorder_begin(
              &recorder, &metadata, text_sink, &stream));
    CHECK(mblink_telemetry_recorder_record_sample(
              &recorder, &latest, true));
    CHECK(mblink_telemetry_recorder_record_response(
              &recorder, 2000U, "010C", &response));
    CHECK(mblink_telemetry_recorder_finish(&recorder, 9000U));
    CHECK(strstr(stream.data, "# mblink_session_stream_version,1\n") != NULL);
    CHECK(strstr(stream.data,
                 "record_type,sequence,timestamp_ms,pid,name,value,unit,favourite,command,result,response\n") != NULL);
    CHECK(strstr(stream.data, "sample,") != NULL);
    CHECK(strstr(stream.data, "transcript,,2000,") != NULL);
    CHECK(strstr(stream.data, "# session_ended_epoch_ms,9000\n") != NULL);
    return 0;
}

int main(void)
{
    if (test_scheduler() != 0) {
        return 1;
    }
    if (test_telemetry() != 0) {
        return 1;
    }
    return 0;
}
