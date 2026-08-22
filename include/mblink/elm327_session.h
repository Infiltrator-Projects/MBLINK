// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327_session.h
 * @brief MBLINK compatibility facade for LINK's ELM327 session engine.
 *
 * The session state machine and transport interaction are implemented once in
 * LINK.  MBLINK keeps its established type and symbol names as a thin ABI
 * facade for existing Mercedes-specific callers.
 */
#ifndef MBLINK_ELM327_SESSION_H
#define MBLINK_ELM327_SESSION_H

#include "link/elm327_session.h"
#include "mblink/elm327.h"
#include "mblink/transport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ELM327_SESSION_IDLE LINK_ELM327_SESSION_IDLE
#define MBLINK_ELM327_SESSION_WAITING LINK_ELM327_SESSION_WAITING
#define MBLINK_ELM327_SESSION_COMPLETE LINK_ELM327_SESSION_COMPLETE
#define MBLINK_ELM327_SESSION_TIMED_OUT LINK_ELM327_SESSION_TIMED_OUT
#define MBLINK_ELM327_SESSION_CANCELLED LINK_ELM327_SESSION_CANCELLED
#define MBLINK_ELM327_SESSION_FAILED LINK_ELM327_SESSION_FAILED

#define MBLINK_ELM327_SESSION_OP_OK LINK_ELM327_SESSION_OP_OK
#define MBLINK_ELM327_SESSION_OP_INVALID_ARGUMENT LINK_ELM327_SESSION_OP_INVALID_ARGUMENT
#define MBLINK_ELM327_SESSION_OP_BUSY LINK_ELM327_SESSION_OP_BUSY
#define MBLINK_ELM327_SESSION_OP_NOT_CONNECTED LINK_ELM327_SESSION_OP_NOT_CONNECTED
#define MBLINK_ELM327_SESSION_OP_NEEDS_RESYNC LINK_ELM327_SESSION_OP_NEEDS_RESYNC
#define MBLINK_ELM327_SESSION_OP_DEADLINE_OVERFLOW LINK_ELM327_SESSION_OP_DEADLINE_OVERFLOW
#define MBLINK_ELM327_SESSION_OP_TRANSPORT_ERROR LINK_ELM327_SESSION_OP_TRANSPORT_ERROR

typedef LinkElm327SessionStatus MblinkElm327SessionStatus;
typedef LinkElm327SessionOpResult MblinkElm327SessionOpResult;
typedef LinkElm327Session MblinkElm327Session;
typedef LinkElm327SessionEventFn MblinkElm327SessionEventFn;

bool mblink_elm327_session_init(MblinkElm327Session *session,
                                const MblinkTransport *transport,
                                MblinkElm327SessionEventFn event,
                                void *event_context);
void mblink_elm327_session_deinit(MblinkElm327Session *session);
MblinkTransportStatus mblink_elm327_session_connect(MblinkElm327Session *session);
void mblink_elm327_session_disconnect(MblinkElm327Session *session);
bool mblink_elm327_session_is_connected(const MblinkElm327Session *session);
MblinkElm327SessionOpResult mblink_elm327_session_begin(
    MblinkElm327Session *session,
    const char *command,
    uint64_t now_ms,
    uint64_t timeout_ms);
MblinkElm327SessionStatus mblink_elm327_session_tick(
    MblinkElm327Session *session,
    uint64_t now_ms);
bool mblink_elm327_session_cancel(MblinkElm327Session *session);
void mblink_elm327_session_mark_resynchronized(MblinkElm327Session *session);
const MblinkElm327Response *mblink_elm327_session_response(
    const MblinkElm327Session *session);
const char *mblink_elm327_session_op_result_name(
    MblinkElm327SessionOpResult result);

#ifdef __cplusplus
}
#endif

#endif
