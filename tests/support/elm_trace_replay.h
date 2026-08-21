// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_TEST_ELM_TRACE_REPLAY_H
#define MBLINK_TEST_ELM_TRACE_REPLAY_H

#include "mblink/elm327.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *command;
    MblinkElm327Result result;
    const char *response_text;
    bool ok_seen;
} MblinkTestElmTraceEntry;

typedef struct {
    const MblinkTestElmTraceEntry *entries;
    size_t count;
    size_t index;
    bool failed;
} MblinkTestElmTraceReplay;

static inline void mblink_test_elm_trace_replay_init(
    MblinkTestElmTraceReplay *replay,
    const MblinkTestElmTraceEntry *entries,
    size_t count)
{
    if (replay == NULL) {
        return;
    }
    replay->entries = entries;
    replay->count = count;
    replay->index = 0U;
    replay->failed = entries == NULL && count != 0U;
}

static inline bool mblink_test_elm_trace_replay_next(
    MblinkTestElmTraceReplay *replay,
    const char *command,
    MblinkElm327Response *response)
{
    const MblinkTestElmTraceEntry *entry;
    size_t length;

    if (replay == NULL || command == NULL || response == NULL ||
        replay->failed || replay->entries == NULL ||
        replay->index >= replay->count) {
        if (replay != NULL) {
            replay->failed = true;
        }
        return false;
    }

    entry = &replay->entries[replay->index];
    if (entry->command == NULL || strcmp(entry->command, command) != 0) {
        replay->failed = true;
        return false;
    }

    memset(response, 0, sizeof(*response));
    response->result = entry->result;
    response->ok_seen = entry->ok_seen;
    if (entry->response_text != NULL) {
        length = strlen(entry->response_text);
        if (length >= sizeof(response->text)) {
            replay->failed = true;
            return false;
        }
        memcpy(response->text, entry->response_text, length);
        response->text[length] = '\0';
        response->length = length;
    }

    replay->index++;
    return true;
}

static inline bool mblink_test_elm_trace_replay_complete(
    const MblinkTestElmTraceReplay *replay)
{
    return replay != NULL && !replay->failed && replay->index == replay->count;
}

#endif
