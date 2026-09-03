// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_TEST_ELM_TRACE_REPLAY_H
#define MBLINK_TEST_ELM_TRACE_REPLAY_H

/*
 * Compatibility-only forwarding header.
 * The generic ELM trace replay helper is owned and implemented by LINK.
 */
#include "../../src/link/tests/support/elm_trace_replay.h"

typedef LinkTestElmTraceEntry MblinkTestElmTraceEntry;
typedef LinkTestElmTraceReplay MblinkTestElmTraceReplay;

#define mblink_test_elm_trace_replay_init link_test_elm_trace_replay_init
#define mblink_test_elm_trace_replay_next link_test_elm_trace_replay_next
#define mblink_test_elm_trace_replay_complete link_test_elm_trace_replay_complete

#endif
