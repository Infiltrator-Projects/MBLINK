// SPDX-License-Identifier: GPL-3.0-or-later
#include "session_trace.h"

#include <stdio.h>
#include <string.h>

const uint8_t mblink_linux_graph_pids[MBLINK_LINUX_GRAPH_TRACE_COUNT] = {
    UINT8_C(0x0c), UINT8_C(0x0d), UINT8_C(0x05), UINT8_C(0x23),
    UINT8_C(0x2f), UINT8_C(0x11), UINT8_C(0x46), UINT8_C(0x49)
};

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
    if (trace == NULL) return;
    memset(trace->graph_history, 0, sizeof(trace->graph_history));
    memset(trace->graph_history_count, 0, sizeof(trace->graph_history_count));
    memset(trace->graph_history_next, 0, sizeof(trace->graph_history_next));
}

void mblink_linux_trace_record_graph(
    MblinkLinuxSessionTrace *trace, uint8_t pid, double value)
{
    const size_t graph = mblink_linux_graph_trace_index(pid);
    uint8_t slot;
    if (trace == NULL || graph >= MBLINK_LINUX_GRAPH_TRACE_COUNT) return;

    slot = trace->graph_history_next[graph];
    trace->graph_history[graph][slot] = value;
    trace->graph_history_next[graph] = (uint8_t)(
        (slot + 1U) % MBLINK_LINUX_GRAPH_HISTORY_CAPACITY);
    if (trace->graph_history_count[graph] <
        MBLINK_LINUX_GRAPH_HISTORY_CAPACITY) {
        ++trace->graph_history_count[graph];
    }
}

static size_t bounded_length(const char *text, size_t maximum)
{
    size_t length = 0U;
    if (text == NULL) return 0U;
    while (length < maximum && text[length] != '\0') ++length;
    return length;
}

static void append_text(char *output, size_t output_size, const char *text)
{
    const size_t used =
        output != NULL ? bounded_length(output, output_size) : output_size;
    if (output == NULL || output_size == 0U || used >= output_size) return;
    (void)snprintf(output + used, output_size - used, "%s", text);
}

void mblink_linux_trace_format_sparkline(
    const double *history, size_t count, size_t next,
    char *output, size_t output_size)
{
    static const char *const blocks[] = {
        "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"
    };
    double minimum;
    double maximum;
    size_t start;
    size_t index;

    if (output == NULL || output_size == 0U) return;
    output[0] = '\0';
    if (history == NULL || count == 0U) return;
    if (count > MBLINK_LINUX_GRAPH_HISTORY_CAPACITY)
        count = MBLINK_LINUX_GRAPH_HISTORY_CAPACITY;

    start = count < MBLINK_LINUX_GRAPH_HISTORY_CAPACITY ? 0U : next;
    minimum = history[start];
    maximum = history[start];
    for (index = 1U; index < count; ++index) {
        const double value = history[
            (start + index) % MBLINK_LINUX_GRAPH_HISTORY_CAPACITY];
        if (value < minimum) minimum = value;
        if (value > maximum) maximum = value;
    }

    for (index = 0U; index < count; ++index) {
        const double value = history[
            (start + index) % MBLINK_LINUX_GRAPH_HISTORY_CAPACITY];
        unsigned int level = 3U;
        if (maximum > minimum) {
            const double scaled =
                ((value - minimum) / (maximum - minimum)) * 7.0;
            level = (unsigned int)(scaled + 0.5);
            if (level > 7U) level = 7U;
        }
        append_text(output, output_size, blocks[level]);
    }
}

void mblink_linux_trace_clear_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms)
{
    if (trace == NULL) return;
    memset(trace->session_log, 0, sizeof(trace->session_log));
    memset(trace->session_log_time_ms, 0, sizeof(trace->session_log_time_ms));
    trace->session_log_count = 0U;
    trace->session_log_next = 0U;
    trace->session_log_started_ms = now_ms;
}

void mblink_linux_trace_append_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms, const char *message)
{
    uint8_t slot;
    size_t length;
    if (trace == NULL || message == NULL || message[0] == '\0') return;

    slot = trace->session_log_next;
    if (trace->session_log_started_ms == 0U)
        trace->session_log_started_ms = now_ms;

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

/*
 * Generic Linux table/graph caches use one deterministic physical responder
 * per PID. Never let arrival order alternate a PID between 7E8 and 7E9.
 * Prefer the legislated engine responder, then 11-bit over 29-bit, then the
 * lowest CAN identifier. A sample from the already-selected responder always
 * refreshes its own value.
 */
bool mblink_linux_trace_prefer_responder(
    uint32_t candidate,
    bool candidate_extended,
    bool current_valid,
    uint32_t current,
    bool current_extended)
{
    if (!current_valid) return true;
    if (candidate == current && candidate_extended == current_extended)
        return true;

    const bool candidate_engine =
        !candidate_extended && candidate == UINT32_C(0x7e8);
    const bool current_engine =
        !current_extended && current == UINT32_C(0x7e8);
    if (candidate_engine != current_engine) return candidate_engine;

    if (candidate_extended != current_extended)
        return !candidate_extended;

    return candidate < current;
}
