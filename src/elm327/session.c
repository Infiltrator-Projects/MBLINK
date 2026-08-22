// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file session.c
 * @brief MBLINK ABI wrappers for LINK's transport-backed ELM327 session.
 */
#include "mblink/elm327_session.h"

bool mblink_elm327_session_init(MblinkElm327Session *session,
                                const MblinkTransport *transport,
                                MblinkElm327SessionEventFn event,
                                void *event_context)
{
    return link_elm327_session_init(session, transport, event, event_context);
}

void mblink_elm327_session_deinit(MblinkElm327Session *session)
{
    link_elm327_session_deinit(session);
}

MblinkTransportStatus mblink_elm327_session_connect(MblinkElm327Session *session)
{
    return link_elm327_session_connect(session);
}

void mblink_elm327_session_disconnect(MblinkElm327Session *session)
{
    link_elm327_session_disconnect(session);
}

bool mblink_elm327_session_is_connected(const MblinkElm327Session *session)
{
    return link_elm327_session_is_connected(session);
}

MblinkElm327SessionOpResult mblink_elm327_session_begin(
    MblinkElm327Session *session,
    const char *command,
    uint64_t now_ms,
    uint64_t timeout_ms)
{
    return link_elm327_session_begin(session, command, now_ms, timeout_ms);
}

MblinkElm327SessionStatus mblink_elm327_session_tick(
    MblinkElm327Session *session,
    uint64_t now_ms)
{
    return link_elm327_session_tick(session, now_ms);
}

bool mblink_elm327_session_cancel(MblinkElm327Session *session)
{
    return link_elm327_session_cancel(session);
}

void mblink_elm327_session_mark_resynchronized(MblinkElm327Session *session)
{
    link_elm327_session_mark_resynchronized(session);
}

const MblinkElm327Response *mblink_elm327_session_response(
    const MblinkElm327Session *session)
{
    return link_elm327_session_response(session);
}

const char *mblink_elm327_session_op_result_name(
    MblinkElm327SessionOpResult result)
{
    return link_elm327_session_op_result_name(result);
}
