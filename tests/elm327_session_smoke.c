// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/elm327_session.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    bool connected;
    bool fail_write;
    bool respond_during_write;
    char last_write[128];
    size_t last_write_size;
    MblinkTransportReceiveFn receiver;
    void *receiver_context;
} MockTransport;

typedef struct {
    MblinkElm327Session *session;
    MblinkElm327SessionOpResult nested_result;
    unsigned int calls;
} ReentrantEventContext;

static int failures = 0;
static unsigned int event_count = 0U;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static MblinkTransportStatus mock_connect(void *context)
{
    MockTransport *mock = context;
    mock->connected = true;
    return MBLINK_TRANSPORT_OK;
}

static void mock_disconnect(void *context)
{
    MockTransport *mock = context;
    mock->connected = false;
}

static bool mock_is_connected(void *context)
{
    const MockTransport *mock = context;
    return mock->connected;
}

static MblinkTransportStatus mock_write(void *context,
                                        const uint8_t *data,
                                        size_t size)
{
    MockTransport *mock = context;
    size_t copy_size = size;

    if (mock->fail_write) {
        return MBLINK_TRANSPORT_IO_ERROR;
    }
    if (copy_size >= sizeof(mock->last_write)) {
        copy_size = sizeof(mock->last_write) - 1U;
    }
    memcpy(mock->last_write, data, copy_size);
    mock->last_write[copy_size] = '\0';
    mock->last_write_size = size;

    if (mock->respond_during_write && mock->receiver != NULL) {
        static const uint8_t response[] = "010C\r41 0C 1A F8\r>";
        mock->receiver(mock->receiver_context, response,
                       sizeof(response) - 1U);
    }
    return MBLINK_TRANSPORT_OK;
}

static void mock_set_receiver(void *context,
                              MblinkTransportReceiveFn receiver,
                              void *receiver_context)
{
    MockTransport *mock = context;
    mock->receiver = receiver;
    mock->receiver_context = receiver_context;
}

static MblinkTransport mock_transport_interface(MockTransport *mock)
{
    MblinkTransport transport = {
        .struct_size = sizeof(MblinkTransport),
        .abi_version = MBLINK_TRANSPORT_ABI,
        .context = mock,
        .connect = mock_connect,
        .disconnect = mock_disconnect,
        .is_connected = mock_is_connected,
        .write = mock_write,
        .set_receiver = mock_set_receiver
    };
    return transport;
}

static void mock_emit(MockTransport *mock, const char *text)
{
    if (mock->receiver != NULL) {
        mock->receiver(mock->receiver_context,
                       (const uint8_t *)text, strlen(text));
    }
}

static void on_session_event(void *context,
                             const MblinkElm327Session *session)
{
    unsigned int *last_sequence = context;
    event_count++;
    *last_sequence = (unsigned int)session->sequence;
}

static void on_reentrant_event(void *context,
                               const MblinkElm327Session *session)
{
    ReentrantEventContext *event_context = context;
    (void)session;
    event_context->calls++;
    event_context->nested_result = mblink_elm327_session_begin(
        event_context->session, "010D", 1U, 100U);
}

static void test_fragmented_execution(void)
{
    MockTransport mock = {0};
    MblinkTransport transport = mock_transport_interface(&mock);
    MblinkElm327Session session;
    const MblinkElm327Response *response;
    unsigned int last_sequence = 0U;

    check(mblink_elm327_session_init(&session, &transport,
                                     on_session_event, &last_sequence),
          "session initialises");
    check(mblink_elm327_session_connect(&session) == MBLINK_TRANSPORT_OK,
          "mock transport connects");
    check(mblink_elm327_session_begin(&session, "010C", 1000U, 500U) ==
              MBLINK_ELM327_SESSION_OP_OK,
          "command begins");
    check(session.status == MBLINK_ELM327_SESSION_WAITING,
          "command remains waiting before response");
    check(mock.last_write_size == 5U &&
              memcmp(mock.last_write, "010C\r", 5U) == 0,
          "session writes exact command frame");
    check(mblink_elm327_session_begin(&session, "010D", 1001U, 500U) ==
              MBLINK_ELM327_SESSION_OP_BUSY,
          "second command rejected while first outstanding");

    mock_emit(&mock, "010C\r41 0C ");
    check(session.status == MBLINK_ELM327_SESSION_WAITING,
          "fragment does not complete without prompt");
    mock_emit(&mock, "1A F8\r>tail");
    check(session.status == MBLINK_ELM327_SESSION_COMPLETE,
          "prompt completes outstanding command");
    response = mblink_elm327_session_response(&session);
    check(response != NULL, "completed response available");
    check(response != NULL &&
              strcmp(response->text, "41 0C 1A F8") == 0,
          "fragmented response normalised");
    check(session.unexpected_input_bytes == 4U,
          "bytes after prompt are accounted for");
    check(event_count == 1U && last_sequence == 1U,
          "completion event delivered once");
}

static void test_synchronous_provider_callback(void)
{
    MockTransport mock = {0};
    MblinkTransport transport = mock_transport_interface(&mock);
    MblinkElm327Session session;

    mock.respond_during_write = true;
    check(mblink_elm327_session_init(&session, &transport, NULL, NULL),
          "sync callback session initialises");
    check(mblink_elm327_session_connect(&session) == MBLINK_TRANSPORT_OK,
          "sync callback transport connects");
    check(mblink_elm327_session_begin(&session, "010C", 0U, 1000U) ==
              MBLINK_ELM327_SESSION_OP_OK,
          "begin tolerates synchronous receive from write");
    check(session.status == MBLINK_ELM327_SESSION_COMPLETE,
          "synchronous receive completes safely");
}

static void test_completion_callback_cannot_reenter(void)
{
    MockTransport mock = {0};
    MblinkTransport transport = mock_transport_interface(&mock);
    MblinkElm327Session session;
    ReentrantEventContext event_context = {0};

    mock.respond_during_write = true;
    event_context.session = &session;
    check(mblink_elm327_session_init(&session, &transport,
                                     on_reentrant_event, &event_context),
          "reentrant session initialises");
    check(mblink_elm327_session_connect(&session) == MBLINK_TRANSPORT_OK,
          "reentrant transport connects");
    check(mblink_elm327_session_begin(&session, "010C", 0U, 100U) ==
              MBLINK_ELM327_SESSION_OP_OK,
          "outer command starts");
    check(event_context.calls == 1U,
          "completion callback invoked once");
    check(event_context.nested_result == MBLINK_ELM327_SESSION_OP_BUSY,
          "completion callback cannot start next command reentrantly");
    check(session.status == MBLINK_ELM327_SESSION_COMPLETE,
          "reentrant attempt does not disturb completed response");
}

static void test_timeout_and_resync_guard(void)
{
    MockTransport mock = {0};
    MblinkTransport transport = mock_transport_interface(&mock);
    MblinkElm327Session session;

    check(mblink_elm327_session_init(&session, &transport, NULL, NULL),
          "timeout session initialises");
    check(mblink_elm327_session_connect(&session) == MBLINK_TRANSPORT_OK,
          "timeout transport connects");
    check(mblink_elm327_session_begin(&session, "0100", 100U, 50U) ==
              MBLINK_ELM327_SESSION_OP_OK,
          "timeout test command begins");
    check(mblink_elm327_session_tick(&session, 149U) ==
              MBLINK_ELM327_SESSION_WAITING,
          "deadline not triggered early");
    check(mblink_elm327_session_tick(&session, 150U) ==
              MBLINK_ELM327_SESSION_TIMED_OUT,
          "deadline triggers exactly");
    check(session.needs_resync, "timeout marks session unsynchronised");
    check(mblink_elm327_session_begin(&session, "ATI", 151U, 50U) ==
              MBLINK_ELM327_SESSION_OP_NEEDS_RESYNC,
          "new command blocked until explicit resync");

    mblink_elm327_session_mark_resynchronized(&session);
    check(!session.needs_resync &&
              session.status == MBLINK_ELM327_SESSION_IDLE,
          "explicit resync restores idle state");
    check(mblink_elm327_session_begin(&session, "ATI", 151U, 50U) ==
              MBLINK_ELM327_SESSION_OP_OK,
          "command accepted after resync");
}

static void test_cancel_and_write_failure(void)
{
    MockTransport mock = {0};
    MblinkTransport transport = mock_transport_interface(&mock);
    MblinkElm327Session session;

    check(mblink_elm327_session_init(&session, &transport, NULL, NULL),
          "cancel session initialises");
    check(mblink_elm327_session_connect(&session) == MBLINK_TRANSPORT_OK,
          "cancel transport connects");
    check(mblink_elm327_session_begin(&session, "ATI", 0U, 100U) ==
              MBLINK_ELM327_SESSION_OP_OK,
          "cancel test command begins");
    check(mblink_elm327_session_cancel(&session), "outstanding command cancels");
    check(session.status == MBLINK_ELM327_SESSION_CANCELLED &&
              session.needs_resync,
          "cancel requires resync");

    mblink_elm327_session_mark_resynchronized(&session);
    mock.fail_write = true;
    check(mblink_elm327_session_begin(&session, "ATI", 0U, 100U) ==
              MBLINK_ELM327_SESSION_OP_TRANSPORT_ERROR,
          "write failure returned to caller");
    check(session.status == MBLINK_ELM327_SESSION_FAILED &&
              session.transport_status == MBLINK_TRANSPORT_IO_ERROR &&
              session.needs_resync,
          "write failure records transport state and resync requirement");
}

static void test_deadline_overflow(void)
{
    MockTransport mock = {0};
    MblinkTransport transport = mock_transport_interface(&mock);
    MblinkElm327Session session;

    check(mblink_elm327_session_init(&session, &transport, NULL, NULL),
          "overflow session initialises");
    check(mblink_elm327_session_connect(&session) == MBLINK_TRANSPORT_OK,
          "overflow transport connects");
    check(mblink_elm327_session_begin(&session, "ATI", UINT64_MAX - 2U, 10U) ==
              MBLINK_ELM327_SESSION_OP_DEADLINE_OVERFLOW,
          "deadline arithmetic never wraps");
}

int main(void)
{
    test_fragmented_execution();
    test_synchronous_provider_callback();
    test_completion_callback_cannot_reenter();
    test_timeout_and_resync_guard();
    test_cancel_and_write_failure();
    test_deadline_overflow();

    if (failures != 0) {
        fprintf(stderr, "%d ELM327 session smoke test(s) failed\n", failures);
        return 1;
    }
    puts("ELM327 session smoke tests passed");
    return 0;
}
