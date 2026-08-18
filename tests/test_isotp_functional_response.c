// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/isotp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static MblinkIsoTpAddress response_address(void)
{
    MblinkIsoTpAddress address = {
        .tx_can_id = 0x7e0U,
        .rx_can_id = 0x7e8U,
        .tx_extended_id = false,
        .rx_extended_id = false,
        .addressing_mode = MBLINK_ISOTP_ADDRESSING_NORMAL,
        .target_type = MBLINK_ISOTP_TARGET_FUNCTIONAL,
        .tx_address_extension = 0U,
        .rx_address_extension = 0U
    };
    return address;
}

static MblinkIsoTpCanFrame make_frame(
    uint32_t can_id, const uint8_t *bytes, size_t length)
{
    MblinkIsoTpCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    require(length <= sizeof(frame.data), "frame length");
    frame.can_id = can_id;
    frame.length = (uint8_t)length;
    memcpy(frame.data, bytes, length);
    return frame;
}

int main(void)
{
    static const uint8_t expected[] = {
        1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U
    };
    static const uint8_t ff_bytes[] = {
        0x10U, 0x0aU, 1U, 2U, 3U, 4U, 5U, 6U
    };
    static const uint8_t cf_bytes[] = {
        0x21U, 7U, 8U, 9U, 10U
    };
    uint8_t buffer[sizeof(expected)];
    MblinkIsoTpRx receiver;
    const MblinkIsoTpRxConfig config = {
        .address = response_address(),
        .block_size = 0U,
        .stmin = 0U,
        .consecutive_timeout_us = 100000U
    };
    MblinkIsoTpCanFrame frame;
    MblinkIsoTpCanFrame flow_control;
    bool flow_control_ready = false;
    size_t payload_length = 0U;
    const uint8_t *payload;

    require(mblink_isotp_rx_init(&receiver, &config,
                                 buffer, sizeof(buffer)) ==
            MBLINK_ISOTP_RESULT_OK,
            "functional-response receiver init");

    frame = make_frame(0x7e8U, ff_bytes, sizeof(ff_bytes));
    require(mblink_isotp_rx_feed(&receiver, &frame, 0U,
                                 &flow_control,
                                 &flow_control_ready) ==
            MBLINK_ISOTP_RESULT_OK,
            "functional request may receive multi-frame response");
    require(receiver.state == MBLINK_ISOTP_RX_RECEIVING,
            "receiver enters receiving state");
    require(flow_control_ready,
            "multi-frame response produces flow control");
    require(flow_control.can_id == 0x7e0U,
            "flow control uses physical ECU request id");
    require(flow_control.length >= 3U &&
            flow_control.data[0] == 0x30U,
            "flow control is CTS");

    flow_control_ready = false;
    frame = make_frame(0x7e8U, cf_bytes, sizeof(cf_bytes));
    require(mblink_isotp_rx_feed(&receiver, &frame, 100U,
                                 &flow_control,
                                 &flow_control_ready) ==
            MBLINK_ISOTP_RESULT_COMPLETE,
            "functional response reassembly completes");
    require(!flow_control_ready,
            "completed response needs no extra flow control");

    payload = mblink_isotp_rx_payload(&receiver, &payload_length);
    require(payload != NULL &&
            payload_length == sizeof(expected) &&
            memcmp(payload, expected, sizeof(expected)) == 0,
            "functional response payload exact");

    puts("ISO-TP functional-response tests passed.");
    return EXIT_SUCCESS;
}
