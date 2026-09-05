// SPDX-License-Identifier: GPL-3.0-or-later
#include "session_trace.h"

#include "link/diagnostic_request.h"

#include <string.h>

const uint8_t mblink_linux_graph_pids[MBLINK_LINUX_GRAPH_TRACE_COUNT] = {
    UINT8_C(0x0c), UINT8_C(0x0d), UINT8_C(0x05), UINT8_C(0x23),
    UINT8_C(0x2f), UINT8_C(0x11), UINT8_C(0x46), UINT8_C(0x49)
};

static void ensure_graph_configuration(MblinkLinuxSessionTrace *trace)
{
    if (trace == NULL) return;
    if (trace->graph_count == MBLINK_LINUX_GRAPH_TRACE_COUNT &&
        memcmp(trace->graph_pids, mblink_linux_graph_pids,
               MBLINK_LINUX_GRAPH_TRACE_COUNT) == 0) {
        return;
    }
    memcpy(trace->graph_pids, mblink_linux_graph_pids,
           MBLINK_LINUX_GRAPH_TRACE_COUNT);
    trace->graph_count = MBLINK_LINUX_GRAPH_TRACE_COUNT;
}

size_t mblink_linux_graph_trace_index(uint8_t pid)
{
    size_t index;
    for (index = 0U; index < MBLINK_LINUX_GRAPH_TRACE_COUNT; ++index) {
        if (mblink_linux_graph_pids[index] == pid) return index;
    }
    return MBLINK_LINUX_GRAPH_TRACE_COUNT;
}

void mblink_linux_trace_reset_graph(MblinkLinuxSessionTrace *trace)
{
    ensure_graph_configuration(trace);
    link_session_trace_reset_graph(trace);
}

void mblink_linux_trace_record_graph(
    MblinkLinuxSessionTrace *trace, uint8_t pid, double value)
{
    ensure_graph_configuration(trace);
    link_session_trace_record_graph(trace, pid, value);
}

void mblink_linux_trace_format_sparkline(
    const double *history, size_t count, size_t next,
    char *output, size_t output_size)
{
    link_session_trace_format_sparkline(
        history, count, next, output, output_size);
}

void mblink_linux_trace_clear_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms)
{
    link_session_trace_clear_log(trace, now_ms);
}

void mblink_linux_trace_append_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms, const char *message)
{
    link_session_trace_append_log(trace, now_ms, message);
}

size_t mblink_linux_trace_log_ordered_slot(
    const MblinkLinuxSessionTrace *trace, size_t ordered_index)
{
    return link_session_trace_log_ordered_slot(trace, ordered_index);
}

const char *mblink_linux_trace_event_text(LinkDiagnosticFlowEventKind kind)
{
    return link_diagnostic_flow_event_text(kind);
}

bool mblink_linux_trace_prefer_responder(
    uint32_t candidate,
    bool candidate_extended,
    bool current_valid,
    uint32_t current,
    bool current_extended)
{
    return link_diagnostic_response_route_preferred(
        candidate,
        candidate_extended,
        current_valid,
        current,
        current_extended,
        UINT32_C(0x7e8),
        false);
}
