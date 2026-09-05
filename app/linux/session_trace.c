// SPDX-License-Identifier: GPL-3.0-or-later
#include "session_trace.h"

#include "link/diagnostic_request.h"

#include <string.h>

static void ensure_graph_configuration(MblinkLinuxSessionTrace *trace)
{
    size_t graph_count = 0U;
    const uint8_t *graph_pids = link_session_trace_default_graph_pids(&graph_count);
    if (trace == NULL || graph_pids == NULL ||
        graph_count > LINK_SESSION_TRACE_MAX_GRAPHS) return;
    if (trace->graph_count == graph_count &&
        memcmp(trace->graph_pids, graph_pids, graph_count) == 0) return;
    memcpy(trace->graph_pids, graph_pids, graph_count);
    trace->graph_count = graph_count;
}

size_t mblink_linux_graph_trace_index(uint8_t pid)
{
    size_t graph_count = 0U;
    const uint8_t *graph_pids = link_session_trace_default_graph_pids(&graph_count);
    size_t index;
    if (graph_pids == NULL) return graph_count;
    for (index = 0U; index < graph_count; ++index) {
        if (graph_pids[index] == pid) return index;
    }
    return graph_count;
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
