// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_LINUX_SESSION_TRACE_H
#define MBLINK_LINUX_SESSION_TRACE_H

#include "link/diagnostic_flow.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MBLINK_LINUX_GRAPH_TRACE_COUNT 8U
#define MBLINK_LINUX_GRAPH_HISTORY_CAPACITY 48U
#define MBLINK_LINUX_SESSION_LOG_CAPACITY 24U
#define MBLINK_LINUX_SESSION_LOG_MESSAGE_CAPACITY 160U

typedef struct {
    double graph_history[MBLINK_LINUX_GRAPH_TRACE_COUNT][MBLINK_LINUX_GRAPH_HISTORY_CAPACITY];
    uint8_t graph_history_count[MBLINK_LINUX_GRAPH_TRACE_COUNT];
    uint8_t graph_history_next[MBLINK_LINUX_GRAPH_TRACE_COUNT];
    char session_log[MBLINK_LINUX_SESSION_LOG_CAPACITY][MBLINK_LINUX_SESSION_LOG_MESSAGE_CAPACITY];
    uint64_t session_log_time_ms[MBLINK_LINUX_SESSION_LOG_CAPACITY];
    uint8_t session_log_count;
    uint8_t session_log_next;
    uint64_t session_log_started_ms;
} MblinkLinuxSessionTrace;

extern const uint8_t
    mblink_linux_graph_pids[MBLINK_LINUX_GRAPH_TRACE_COUNT];

size_t mblink_linux_graph_trace_index(uint8_t pid);
void mblink_linux_trace_reset_graph(MblinkLinuxSessionTrace *trace);
void mblink_linux_trace_record_graph(
    MblinkLinuxSessionTrace *trace, uint8_t pid, double value);
void mblink_linux_trace_format_sparkline(
    const double *history, size_t count, size_t next,
    char *output, size_t output_size);

void mblink_linux_trace_clear_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms);
void mblink_linux_trace_append_log(
    MblinkLinuxSessionTrace *trace, uint64_t now_ms, const char *message);
size_t mblink_linux_trace_log_ordered_slot(
    const MblinkLinuxSessionTrace *trace, size_t ordered_index);

bool mblink_linux_trace_prefer_responder(
    uint32_t candidate, bool candidate_extended,
    bool current_valid, uint32_t current, bool current_extended);
const char *mblink_linux_trace_event_text(LinkDiagnosticFlowEventKind kind);

#endif
