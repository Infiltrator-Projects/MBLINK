// SPDX-License-Identifier: GPL-3.0-or-later
#include "session_trace.h"

#include <stdio.h>
#include <string.h>

#include "link/diagnostic_response.h"

void mblink_linux_trace_init(MblinkLinuxSessionTrace *trace)
{
    if (trace == NULL) return;
    memset(trace, 0, sizeof(*trace));
}

void mblink_linux_trace_set_started_ms(
    MblinkLinuxSessionTrace *trace, uint64_t started_ms)
{
    if (trace == NULL) return;
    trace->session_log_started_ms = started_ms;
}

void mblink_linux_trace_record_graph(
    MblinkLinuxSessionTrace *trace, uint8_t pid, double value)
{
    size_t slot;
    size_t index;
    if (trace == NULL) return;

    for (index = 0U; index < trace->graph_pid_count; ++index) {
        if (trace->graph_pids[index] == pid) break;
    }
    if (index == trace->graph_pid_count) {
        if (trace->graph_pid_count >= MBLINK_LINUX_GRAPH_PID_CAPACITY)
            return;
        trace->graph_pids[index] = pid;
        ++trace->graph_pid_count;
    }

    slot = trace->graph_history_next[index];
    trace->graph_history[index][slot] = value;
    trace->graph_history_next[index] = (uint8_t)(
        (slot + 1U) % MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY);
    if (trace->graph_history_count[index] <
        MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY) {
        ++trace->graph_history_count[index];
    }
}

void mblink_linux_trace_reset_graph(MblinkLinuxSessionTrace *trace)
{
    if (trace == NULL) return;
    memset(trace->graph_pids, 0, sizeof(trace->graph_pids));
    memset(trace->graph_history, 0, sizeof(trace->graph_history));
    memset(trace->graph_history_count, 0,
           sizeof(trace->graph_history_count));
    memset(trace->graph_history_next, 0,
           sizeof(trace->graph_history_next));
    trace->graph_pid_count = 0U;
}

bool mblink_linux_trace_latest_graph(
    const MblinkLinuxSessionTrace *trace,
    uint8_t pid,
    double *value)
{
    size_t index;
    size_t slot;
    if (trace == NULL || value == NULL) return false;
    for (index = 0U; index < trace->graph_pid_count; ++index) {
        if (trace->graph_pids[index] == pid) break;
    }
    if (index == trace->graph_pid_count ||
        trace->graph_history_count[index] == 0U) {
        return false;
    }
    slot = (trace->graph_history_next[index] +
            MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY - 1U) %
           MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY;
    *value = trace->graph_history[index][slot];
    return true;
}

size_t mblink_linux_trace_graph_ordered_slot(
    const MblinkLinuxSessionTrace *trace,
    size_t graph_index,
    size_t ordered_index)
{
    size_t start;
    if (trace == NULL || graph_index >= trace->graph_pid_count ||
        ordered_index >= trace->graph_history_count[graph_index]) {
        return MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY;
    }
    start = trace->graph_history_count[graph_index] <
            MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY
        ? 0U : trace->graph_history_next[graph_index];
    return (start + ordered_index) % MBLINK_LINUX_GRAPH_SAMPLE_CAPACITY;
}

void mblink_linux_trace_append_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms, const char *message)
{
    size_t slot;
    size_t length;
    if (trace == NULL || message == NULL || message[0] == '\0') return;

    slot = trace->session_log_next;
    length = strlen(message);
    if (length >= MBLINK_LINUX_SESSION_LOG_MESSAGE_CAPACITY)
        length = MBLINK_LINUX_SESSION_LOG_MESSAGE_CAPACITY - 1U;
    memcpy(trace->session_log[slot], message, length);
    trace->session_log[slot][length] = '\0';
    trace->session_log_time_ms[slot] =
        now_ms >= trace->session_log_started_ms
            ? now_ms - trace->session_log_started_ms : 0U;
    trace->session_log_next = (uint8_t)(
        (slot + 1U) % MBLINK_LINUX_SESSION_LOG_CAPACITY);
    if (trace->session_log_count < MBLINK_LINUX_SESSION_LOG_CAPACITY)
        ++trace->session_log_count;
}

size_t mblink_linux_trace_log_ordered_slot(
    const MblinkLinuxSessionTrace *trace, size_t ordered_index)
{
    size_t start;
    if (trace == NULL || ordered_index >= trace->session_log_count)
        return MBLINK_LINUX_SESSION_LOG_CAPACITY;
    start = trace->session_log_count < MBLINK_LINUX_SESSION_LOG_CAPACITY
        ? 0U : trace->session_log_next;
    return (start + ordered_index) % MBLINK_LINUX_SESSION_LOG_CAPACITY;
}

const char *mblink_linux_trace_event_text(LinkDiagnosticFlowEventKind kind)
{
    switch (kind) {
    case LINK_DIAGNOSTIC_FLOW_EVENT_ADAPTER_IDENTIFIED:
        return "Adapter identified";
    case LINK_DIAGNOSTIC_FLOW_EVENT_PROTOCOL_IDENTIFIED:
        return "OBD protocol identified";
    case LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE:
        return "Standard PID discovery complete";
    case LINK_DIAGNOSTIC_FLOW_EVENT_STANDARD_VIN:
        return "Standard VIN read complete";
    case LINK_DIAGNOSTIC_FLOW_EVENT_DTC_LIST:
        return "Standard DTC inventory updated";
    case LINK_DIAGNOSTIC_FLOW_EVENT_READINESS:
        return "Readiness monitors captured";
    case LINK_DIAGNOSTIC_FLOW_EVENT_FREEZE_FRAME_SAMPLE:
        return "Freeze-frame sample captured";
    case LINK_DIAGNOSTIC_FLOW_EVENT_DIAGNOSTIC_CONTEXT_COMPLETE:
        return "Diagnostic context complete";
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_NO_DATA:
        return "Live PID returned no data";
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_UNSUPPORTED:
        return "Live PID reported unsupported";
    case LINK_DIAGNOSTIC_FLOW_EVENT_NONE:
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE:
    case LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_STRUCTURED:
        return NULL;
    }
    return NULL;
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
