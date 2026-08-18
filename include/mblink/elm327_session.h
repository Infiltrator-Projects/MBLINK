// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327_session.h
 * @brief Transport-backed ELM327 command session engine.
 */
#ifndef MBLINK_ELM327_SESSION_H
#define MBLINK_ELM327_SESSION_H

#include "mblink/elm327.h"
#include "mblink/transport.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_ELM327_SESSION_IDLE = 0,
    MBLINK_ELM327_SESSION_WAITING,
    MBLINK_ELM327_SESSION_COMPLETE,
    MBLINK_ELM327_SESSION_TIMED_OUT,
    MBLINK_ELM327_SESSION_CANCELLED,
    MBLINK_ELM327_SESSION_FAILED
} MblinkElm327SessionStatus;

typedef enum {
    MBLINK_ELM327_SESSION_OP_OK = 0,
    MBLINK_ELM327_SESSION_OP_INVALID_ARGUMENT,
    MBLINK_ELM327_SESSION_OP_BUSY,
    MBLINK_ELM327_SESSION_OP_NOT_CONNECTED,
    MBLINK_ELM327_SESSION_OP_NEEDS_RESYNC,
    MBLINK_ELM327_SESSION_OP_DEADLINE_OVERFLOW,
    MBLINK_ELM327_SESSION_OP_TRANSPORT_ERROR
} MblinkElm327SessionOpResult;

struct MblinkElm327Session;
typedef struct MblinkElm327Session MblinkElm327Session;

typedef void (*MblinkElm327SessionEventFn)(
    void *context,
    const MblinkElm327Session *session);

struct MblinkElm327Session {
    MblinkTransport transport;
    MblinkElm327Parser parser;
    MblinkElm327Response response;
    MblinkElm327SessionStatus status;
    MblinkElm327Result elm_result;
    MblinkTransportStatus transport_status;
    uint64_t deadline_ms;
    uint64_t sequence;
    size_t unexpected_input_bytes;
    bool needs_resync;
    bool callback_active;
    MblinkElm327SessionEventFn event;
    void *event_context;
};

/**
 * Initialise a session and install its transport receiver callback.
 *
 * The transport object is copied, but its context and provider-owned resources
 * must remain valid until `mblink_elm327_session_deinit()` is called.
 */
bool mblink_elm327_session_init(MblinkElm327Session *session,
                                const MblinkTransport *transport,
                                MblinkElm327SessionEventFn event,
                                void *event_context);

/**
 * Detach the transport receiver callback and invalidate the session.
 *
 * Call this before the session storage or transport provider is destroyed and
 * only after any synchronous session event callback has returned.
 */
void mblink_elm327_session_deinit(MblinkElm327Session *session);

/** Connect the underlying transport. A successful reconnect re-synchronises. */
MblinkTransportStatus mblink_elm327_session_connect(
    MblinkElm327Session *session);

/** Disconnect the underlying transport and cancel any outstanding command. */
void mblink_elm327_session_disconnect(MblinkElm327Session *session);

/** Return whether the underlying provider currently reports connected. */
bool mblink_elm327_session_is_connected(
    const MblinkElm327Session *session);

/**
 * Begin one ELM327 command.
 *
 * Exactly one command may be outstanding. `now_ms` must use a monotonic
 * caller-owned clock. Timeout zero is invalid. The session is single-threaded;
 * callers must serialize access, and a completion callback may observe state
 * but must defer starting the next command until the callback returns.
 */
MblinkElm327SessionOpResult mblink_elm327_session_begin(
    MblinkElm327Session *session,
    const char *command,
    uint64_t now_ms,
    uint64_t timeout_ms);

/** Apply timeout processing using the same monotonic clock supplied to begin. */
MblinkElm327SessionStatus mblink_elm327_session_tick(
    MblinkElm327Session *session,
    uint64_t now_ms);

/**
 * Cancel an outstanding command without sending an adapter-side abort byte.
 *
 * Cancellation requires transport re-synchronisation before another command,
 * preventing late bytes from a cancelled request contaminating the next one.
 */
bool mblink_elm327_session_cancel(MblinkElm327Session *session);

/**
 * Explicitly clear the re-synchronisation guard after the caller has drained
 * to a known prompt or otherwise restored the ELM command boundary.
 */
void mblink_elm327_session_mark_resynchronized(
    MblinkElm327Session *session);

/** Return the completed response, or NULL unless status is COMPLETE. */
const MblinkElm327Response *mblink_elm327_session_response(
    const MblinkElm327Session *session);

/** Return a stable string for an operation result. */
const char *mblink_elm327_session_op_result_name(
    MblinkElm327SessionOpResult result);

#ifdef __cplusplus
}
#endif

#endif
