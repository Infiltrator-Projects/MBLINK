// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_MERCEDES_CONNECTION_LIFECYCLE_H
#define MBLINK_MERCEDES_CONNECTION_LIFECYCLE_H

#include "mblink/elm327.h"
#include "mblink/mercedes_module_scan_core.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_CONNECTION_SESSION_NONE = 0,
    MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH,
    MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN
} MblinkMercedesConnectionSessionMode;

typedef enum {
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE = 0,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_PROTOCOL,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_HEADER,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_RECEIVE,
    MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_CONTROL
} MblinkMercedesConnectionSessionStage;

typedef struct {
    MblinkMercedesConnectionSessionMode mode;
    MblinkMercedesConnectionSessionStage stage;
    size_t module_index;
    bool disconnect_requested;
} MblinkMercedesConnectionLifecycle;

void mblink_mercedes_connection_lifecycle_init(
    MblinkMercedesConnectionLifecycle *lifecycle);
void mblink_mercedes_connection_lifecycle_begin(
    MblinkMercedesConnectionLifecycle *lifecycle,
    MblinkMercedesConnectionSessionMode mode);
MblinkMercedesConnectionSessionMode mblink_mercedes_connection_lifecycle_finish(
    MblinkMercedesConnectionLifecycle *lifecycle);
bool mblink_mercedes_connection_lifecycle_active(
    const MblinkMercedesConnectionLifecycle *lifecycle);

/** Resolve one ECU pack's documented enter/quit session commands. */
bool mblink_mercedes_connection_session_commands(
    const MblinkMercedesModuleScanEntry *module,
    char enter_command[5],
    char quit_command[5]);

/** True only for a positively decoded DiagnosticSessionControl response. */
bool mblink_mercedes_connection_session_response_positive(
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
