// SPDX-License-Identifier: GPL-3.0-or-later
#include "session_trace.h"

#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) {     (void)fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #x);     return 1; } } while (0)

int main(void)
{
    MblinkLinuxSessionTrace trace = {0};
    char sparkline[192];
    size_t index;

    CHECK(mblink_linux_graph_trace_index(UINT8_C(0x0c)) == 0U);
    CHECK(mblink_linux_graph_trace_index(UINT8_C(0xff)) ==
          MBLINK_LINUX_GRAPH_TRACE_COUNT);

    mblink_linux_trace_record_graph(&trace, UINT8_C(0x0c), 1000.0);
    mblink_linux_trace_record_graph(&trace, UINT8_C(0x0c), 1500.0);
    CHECK(trace.graph_history_count[0] == 2U);
    CHECK(trace.graph_history_next[0] == 2U);
    CHECK(trace.graph_history[0][0] == 1000.0);
    CHECK(trace.graph_history[0][1] == 1500.0);
    mblink_linux_trace_format_sparkline(
        trace.graph_history[0], trace.graph_history_count[0],
        trace.graph_history_next[0], sparkline, sizeof(sparkline));
    CHECK(sparkline[0] != '\0');

    mblink_linux_trace_reset_graph(&trace);
    CHECK(trace.graph_history_count[0] == 0U);
    CHECK(trace.graph_history_next[0] == 0U);
    CHECK(trace.graph_history[0][0] == 0.0);

    mblink_linux_trace_clear_log(&trace, UINT64_C(1000));
    mblink_linux_trace_append_log(&trace, UINT64_C(1100), "connected");
    mblink_linux_trace_append_log(&trace, UINT64_C(1250), "ready");
    CHECK(trace.session_log_count == 2U);
    CHECK(mblink_linux_trace_log_ordered_slot(&trace, 0U) == 0U);
    CHECK(mblink_linux_trace_log_ordered_slot(&trace, 1U) == 1U);
    CHECK(strcmp(trace.session_log[0], "connected") == 0);
    CHECK(trace.session_log_time_ms[0] == UINT64_C(100));
    CHECK(strcmp(trace.session_log[1], "ready") == 0);
    CHECK(trace.session_log_time_ms[1] == UINT64_C(250));

    mblink_linux_trace_clear_log(&trace, UINT64_C(2000));
    for (index = 0U; index < MBLINK_LINUX_SESSION_LOG_CAPACITY + 3U; ++index) {
        char message[32];
        (void)snprintf(message, sizeof(message), "event-%02zu", index);
        mblink_linux_trace_append_log(
            &trace, UINT64_C(2000) + (uint64_t)index, message);
    }
    CHECK(trace.session_log_count == MBLINK_LINUX_SESSION_LOG_CAPACITY);
    {
        const size_t first =
            mblink_linux_trace_log_ordered_slot(&trace, 0U);
        const size_t last =
            mblink_linux_trace_log_ordered_slot(
                &trace, MBLINK_LINUX_SESSION_LOG_CAPACITY - 1U);
        CHECK(first < MBLINK_LINUX_SESSION_LOG_CAPACITY);
        CHECK(last < MBLINK_LINUX_SESSION_LOG_CAPACITY);
        CHECK(strcmp(trace.session_log[first], "event-03") == 0);
        CHECK(strcmp(trace.session_log[last], "event-26") == 0);
    }

    (void)printf("MBLINK Linux session trace semantics verified\n");
    return 0;
}
