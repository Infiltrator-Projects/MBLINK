// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file session.c
 * @brief Transport-backed ELM327 command execution.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#include "mblink/elm327_session.h"

#include "infiltratr/core.h"

#include <string.h>

static void elm327_session_add_unexpected(MblinkElm327Session *session,
                                          size_t amount)
{
    if (session == NULL || amount == 0U) {
        return;
    }
    if (SIZE_MAX - session->unexpected_input_bytes < amount) {
        session->unexpected_input_bytes = SIZE_MAX;
    } else {
        session->unexpected_input_bytes += amount;
    }
}

static void elm327_session_notify(MblinkElm327Session *session)
{
    if (session != NULL && session->event != NULL) {
        session->callback_active = true;
        session->event(session->event_context, session);
        session->callback_active = false;
    }
}

static void elm327_session_fail(MblinkElm327Session *session,
                                MblinkElm327Result elm_result,
                                MblinkTransportStatus transport_status,
                                bool needs_resync)
{
    session->status = MBLINK_ELM327_SESSION_FAILED;
    session->elm_result = elm_result;
    session->transport_status = transport_status;
    session->needs_resync = needs_resync;
    elm327_session_notify(session);
}

static void elm327_session_receive(void *context,
                                   const uint8_t *data,
                                   size_t size)
{
    MblinkElm327Session *session = context;
    size_t offset = 0U;

    if (session == NULL || (data == NULL && size != 0U)) {
        return;
    }

    if (session->status != MBLINK_ELM327_SESSION_WAITING) {
        elm327_session_add_unexpected(session, size);
        return;
    }

    while (offset < size &&
           session->status == MBLINK_ELM327_SESSION_WAITING) {
        size_t consumed = 0U;
        MblinkElm327Result result = mblink_elm327_parser_feed(
            &session->parser, data + offset, size - offset, &consumed);

        offset += consumed;
        if (result == MBLINK_ELM327_RESULT_MORE_DATA) {
            break;
        }
        if (result != MBLINK_ELM327_RESULT_OK) {
            elm327_session_add_unexpected(session, size - offset);
            elm327_session_fail(session, result, MBLINK_TRANSPORT_OK, true);
            return;
        }

        result = mblink_elm327_parser_finish(&session->parser,
                                             &session->response);
        session->elm_result = result;
        session->transport_status = MBLINK_TRANSPORT_OK;
        session->status = MBLINK_ELM327_SESSION_COMPLETE;
        elm327_session_add_unexpected(session, size - offset);
        elm327_session_notify(session);
    }
}

bool mblink_elm327_session_init(MblinkElm327Session *session,
                                const MblinkTransport *transport,
                                MblinkElm327SessionEventFn event,
                                void *event_context)
{
    if (session == NULL || !mblink_transport_is_valid(transport)) {
        return false;
    }

    memset(session, 0, sizeof(*session));
    session->transport = *transport;
    session->status = MBLINK_ELM327_SESSION_IDLE;
    session->elm_result = MBLINK_ELM327_RESULT_OK;
    session->transport_status = MBLINK_TRANSPORT_OK;
    session->event = event;
    session->event_context = event_context;
    session->transport.set_receiver(session->transport.context,
                                    elm327_session_receive, session);
    return true;
}

MblinkTransportStatus mblink_elm327_session_connect(
    MblinkElm327Session *session)
{
    MblinkTransportStatus result;

    if (session == NULL ||
        !mblink_transport_is_valid(&session->transport)) {
        return MBLINK_TRANSPORT_INVALID_ARGUMENT;
    }
    if (session->callback_active ||
        session->status == MBLINK_ELM327_SESSION_WAITING) {
        return MBLINK_TRANSPORT_BUSY;
    }

    result = session->transport.connect(session->transport.context);
    session->transport_status = result;
    if (result == MBLINK_TRANSPORT_OK) {
        session->status = MBLINK_ELM327_SESSION_IDLE;
        session->elm_result = MBLINK_ELM327_RESULT_OK;
        session->needs_resync = false;
        memset(&session->parser, 0, sizeof(session->parser));
        memset(&session->response, 0, sizeof(session->response));
    }
    return result;
}

void mblink_elm327_session_disconnect(MblinkElm327Session *session)
{
    if (session == NULL ||
        !mblink_transport_is_valid(&session->transport) ||
        session->callback_active) {
        return;
    }

    session->transport.disconnect(session->transport.context);
    if (session->status == MBLINK_ELM327_SESSION_WAITING) {
        session->status = MBLINK_ELM327_SESSION_CANCELLED;
        session->needs_resync = false;
        elm327_session_notify(session);
    } else {
        session->status = MBLINK_ELM327_SESSION_IDLE;
        session->needs_resync = false;
    }
    session->transport_status = MBLINK_TRANSPORT_NOT_CONNECTED;
}

bool mblink_elm327_session_is_connected(
    const MblinkElm327Session *session)
{
    if (session == NULL ||
        !mblink_transport_is_valid(&session->transport)) {
        return false;
    }
    return session->transport.is_connected(session->transport.context);
}

MblinkElm327SessionOpResult mblink_elm327_session_begin(
    MblinkElm327Session *session,
    const char *command,
    uint64_t now_ms,
    uint64_t timeout_ms)
{
    uint8_t frame[MBLINK_ELM327_MAX_COMMAND + 1U];
    size_t frame_size = 0U;
    uint64_t deadline;
    MblinkElm327Result parser_result;
    MblinkTransportStatus write_result;

    if (session == NULL || command == NULL || timeout_ms == 0U ||
        !mblink_transport_is_valid(&session->transport)) {
        return MBLINK_ELM327_SESSION_OP_INVALID_ARGUMENT;
    }
    if (session->callback_active ||
        session->status == MBLINK_ELM327_SESSION_WAITING) {
        return MBLINK_ELM327_SESSION_OP_BUSY;
    }
    if (session->needs_resync) {
        return MBLINK_ELM327_SESSION_OP_NEEDS_RESYNC;
    }
    if (!session->transport.is_connected(session->transport.context)) {
        return MBLINK_ELM327_SESSION_OP_NOT_CONNECTED;
    }
    if (!infiltratr_u64_add_checked(now_ms, timeout_ms, &deadline)) {
        return MBLINK_ELM327_SESSION_OP_DEADLINE_OVERFLOW;
    }

    parser_result = mblink_elm327_build_command(
        command, frame, sizeof(frame), &frame_size);
    if (parser_result != MBLINK_ELM327_RESULT_OK) {
        session->elm_result = parser_result;
        return MBLINK_ELM327_SESSION_OP_INVALID_ARGUMENT;
    }
    parser_result = mblink_elm327_parser_begin(&session->parser, command);
    if (parser_result != MBLINK_ELM327_RESULT_OK) {
        session->elm_result = parser_result;
        return MBLINK_ELM327_SESSION_OP_INVALID_ARGUMENT;
    }

    memset(&session->response, 0, sizeof(session->response));
    session->status = MBLINK_ELM327_SESSION_WAITING;
    session->elm_result = MBLINK_ELM327_RESULT_MORE_DATA;
    session->transport_status = MBLINK_TRANSPORT_OK;
    session->deadline_ms = deadline;
    session->sequence++;

    write_result = session->transport.write(
        session->transport.context, frame, frame_size);
    if (write_result != MBLINK_TRANSPORT_OK) {
        elm327_session_fail(session, MBLINK_ELM327_RESULT_MORE_DATA,
                            write_result, true);
        return MBLINK_ELM327_SESSION_OP_TRANSPORT_ERROR;
    }

    return MBLINK_ELM327_SESSION_OP_OK;
}

MblinkElm327SessionStatus mblink_elm327_session_tick(
    MblinkElm327Session *session,
    uint64_t now_ms)
{
    if (session == NULL) {
        return MBLINK_ELM327_SESSION_FAILED;
    }

    if (session->status == MBLINK_ELM327_SESSION_WAITING &&
        now_ms >= session->deadline_ms) {
        session->status = MBLINK_ELM327_SESSION_TIMED_OUT;
        session->transport_status = MBLINK_TRANSPORT_TIMEOUT;
        session->needs_resync = true;
        elm327_session_notify(session);
    }
    return session->status;
}

bool mblink_elm327_session_cancel(MblinkElm327Session *session)
{
    if (session == NULL ||
        session->status != MBLINK_ELM327_SESSION_WAITING) {
        return false;
    }

    session->status = MBLINK_ELM327_SESSION_CANCELLED;
    session->needs_resync = true;
    elm327_session_notify(session);
    return true;
}

void mblink_elm327_session_mark_resynchronized(
    MblinkElm327Session *session)
{
    if (session == NULL ||
        session->status == MBLINK_ELM327_SESSION_WAITING) {
        return;
    }

    session->needs_resync = false;
    session->status = MBLINK_ELM327_SESSION_IDLE;
    session->elm_result = MBLINK_ELM327_RESULT_OK;
    session->transport_status = MBLINK_TRANSPORT_OK;
    memset(&session->parser, 0, sizeof(session->parser));
    memset(&session->response, 0, sizeof(session->response));
}

const MblinkElm327Response *mblink_elm327_session_response(
    const MblinkElm327Session *session)
{
    if (session == NULL ||
        session->status != MBLINK_ELM327_SESSION_COMPLETE) {
        return NULL;
    }
    return &session->response;
}

const char *mblink_elm327_session_op_result_name(
    MblinkElm327SessionOpResult result)
{
    switch (result) {
    case MBLINK_ELM327_SESSION_OP_OK:
        return "ok";
    case MBLINK_ELM327_SESSION_OP_INVALID_ARGUMENT:
        return "invalid-argument";
    case MBLINK_ELM327_SESSION_OP_BUSY:
        return "busy";
    case MBLINK_ELM327_SESSION_OP_NOT_CONNECTED:
        return "not-connected";
    case MBLINK_ELM327_SESSION_OP_NEEDS_RESYNC:
        return "needs-resync";
    case MBLINK_ELM327_SESSION_OP_DEADLINE_OVERFLOW:
        return "deadline-overflow";
    case MBLINK_ELM327_SESSION_OP_TRANSPORT_ERROR:
        return "transport-error";
    }
    return "unknown";
}
