// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mblink.h"
#include "mblink/transport.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MblinkTransportStatus mock_connect(void *context)
{
    (void)context;
    return MBLINK_TRANSPORT_OK;
}

static void mock_disconnect(void *context)
{
    (void)context;
}

static bool mock_is_connected(void *context)
{
    (void)context;
    return true;
}

static MblinkTransportStatus mock_write(void *context,
                                        const uint8_t *data,
                                        size_t size)
{
    (void)context;
    return (data != NULL && size > 0U) ? MBLINK_TRANSPORT_OK
                                       : MBLINK_TRANSPORT_INVALID_ARGUMENT;
}

static void mock_set_receiver(void *context,
                              MblinkTransportReceiveFn receiver,
                              void *receiver_context)
{
    (void)context;
    (void)receiver;
    (void)receiver_context;
}

static bool check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "mblink-core-smoke: %s\n", message);
    }
    return condition;
}

int main(void)
{
    bool passed = true;
    MblinkTransport transport = MBLINK_TRANSPORT_INIT;
    static const uint8_t probe[] = { 'A', 'T', 'I', '\r' };

    if (!check(strcmp(mblink_version(), "0.1.0") == 0,
               "unexpected MBLINK version")) {
        passed = false;
    }
    if (!check(mblink_self_check(), "project identity validation failed")) {
        passed = false;
    }
    if (!check(!mblink_transport_is_valid(&transport),
               "empty transport should be invalid")) {
        passed = false;
    }

    transport.connect = mock_connect;
    transport.disconnect = mock_disconnect;
    transport.is_connected = mock_is_connected;
    transport.write = mock_write;
    transport.set_receiver = mock_set_receiver;

    if (!check(mblink_transport_is_valid(&transport),
               "complete transport should be valid")) {
        passed = false;
    }

    transport.abi_version = MBLINK_TRANSPORT_ABI + 1U;
    if (!check(!mblink_transport_is_valid(&transport),
               "unknown transport ABI should be rejected")) {
        passed = false;
    }
    transport.abi_version = MBLINK_TRANSPORT_ABI;

    if (!check(transport.connect(transport.context) == MBLINK_TRANSPORT_OK,
               "mock connect failed")) {
        passed = false;
    }
    if (!check(transport.is_connected(transport.context),
               "mock transport did not report connected")) {
        passed = false;
    }
    if (!check(transport.write(transport.context, probe, sizeof(probe)) ==
                   MBLINK_TRANSPORT_OK,
               "mock write failed")) {
        passed = false;
    }

    transport.disconnect(transport.context);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
