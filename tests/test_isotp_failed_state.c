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

static MblinkIsoTpAddress address(void)
{
    const MblinkIsoTpAddress value = {
        .tx_can_id = 0x7e0U,
        .rx_can_id = 0x7e8U,
        .tx_extended_id = false,
        .rx_extended_id = false,
        .addressing_mode = MBLINK_ISOTP_ADDRESSING_NORMAL,
        .target_type = MBLINK_ISOTP_TARGET_PHYSICAL,
        .tx_address_extension = 0U,
        .rx_address_extension = 0U
    };
    return value;
}

static void test_rx_failed_state(void)
{
    uint8_t buffer[32];
    MblinkIsoTpRx receiver;
    MblinkIsoTpCanFrame fc = {0};
    bool fc_ready = false;
    const MblinkIsoTpRxConfig config = {
        .address = address(),
        .block_size = 0U,
        .stmin = 0U,
        .consecutive_timeout_us = 1000U
    };
    const MblinkIsoTpCanFrame first = {
        .can_id = 0x7e8U,
        .extended_id = false,
        .length = 8U,
        .data = {0x10U, 0x0aU, 1U, 2U, 3U, 4U, 5U, 6U}
    };
    const MblinkIsoTpCanFrame wrong = {
        .can_id = 0x7e8U,
        .extended_id = false,
        .length = 5U,
        .data = {0x22U, 7U, 8U, 9U, 10U}
    };

    require(mblink_isotp_rx_init(&receiver, &config,
                                 buffer, sizeof(buffer)) ==
            MBLINK_ISOTP_RESULT_OK, "RX init");
    require(mblink_isotp_rx_feed(&receiver, &first, 0U,
                                 &fc, &fc_ready) ==
            MBLINK_ISOTP_RESULT_OK, "RX first frame");
    require(mblink_isotp_rx_feed(&receiver, &wrong, 10U,
                                 &fc, &fc_ready) ==
            MBLINK_ISOTP_RESULT_WRONG_SEQUENCE,
            "RX wrong sequence");
    require(receiver.state == MBLINK_ISOTP_RX_FAILED &&
            receiver.failure == MBLINK_ISOTP_RESULT_WRONG_SEQUENCE,
            "RX latches original failure");
    require(mblink_isotp_rx_tick(&receiver, 20U) ==
            MBLINK_ISOTP_RESULT_WRONG_SEQUENCE,
            "RX tick preserves failure cause");
    require(mblink_isotp_rx_feed(&receiver, &first, 20U,
                                 &fc, &fc_ready) ==
            MBLINK_ISOTP_RESULT_WRONG_SEQUENCE,
            "RX feed preserves failure cause");
    mblink_isotp_rx_reset(&receiver);
    require(receiver.state == MBLINK_ISOTP_RX_IDLE &&
            receiver.failure == MBLINK_ISOTP_RESULT_OK,
            "RX reset clears failed state and cause");
}

static void test_tx_failed_state(void)
{
    uint8_t payload[20] = {0};
    MblinkIsoTpTx transmitter;
    MblinkIsoTpCanFrame first;
    MblinkIsoTpCanFrame next;
    const MblinkIsoTpTxConfig config = {
        .address = address(),
        .flow_control_timeout_us = 1000U,
        .max_wait_frames = 0U
    };
    const MblinkIsoTpCanFrame overflow = {
        .can_id = 0x7e8U,
        .extended_id = false,
        .length = 3U,
        .data = {0x32U, 0U, 0U}
    };

    require(mblink_isotp_tx_init(&transmitter, &config,
                                 payload, sizeof(payload)) ==
            MBLINK_ISOTP_RESULT_OK, "TX init");
    require(mblink_isotp_tx_start(&transmitter, 0U, &first) ==
            MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL,
            "TX first frame");
    require(mblink_isotp_tx_accept_flow_control(&transmitter,
                                                &overflow, 10U) ==
            MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW,
            "TX overflow failure");
    require(transmitter.state == MBLINK_ISOTP_TX_FAILED &&
            transmitter.failure == MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW,
            "TX latches original failure");
    require(mblink_isotp_tx_tick(&transmitter, 20U) ==
            MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW,
            "TX tick preserves failure cause");
    require(mblink_isotp_tx_next(&transmitter, 20U, &next) ==
            MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW,
            "TX next preserves failure cause");
    mblink_isotp_tx_reset(&transmitter);
    require(transmitter.state == MBLINK_ISOTP_TX_IDLE &&
            transmitter.failure == MBLINK_ISOTP_RESULT_OK,
            "TX reset clears failed state and cause");
}

static void test_tx_reset_reuses_payload(void)
{
    static const uint8_t payload[] = {0x22U, 0xf1U, 0x90U};
    MblinkIsoTpTx transmitter;
    MblinkIsoTpCanFrame first;
    MblinkIsoTpCanFrame second;
    const MblinkIsoTpTxConfig config = {
        .address = address(),
        .flow_control_timeout_us = 1000U,
        .max_wait_frames = 0U
    };

    require(mblink_isotp_tx_init(&transmitter, &config,
                                 payload, sizeof(payload)) ==
            MBLINK_ISOTP_RESULT_OK, "retransmit TX init");
    require(mblink_isotp_tx_start(&transmitter, 0U, &first) ==
            MBLINK_ISOTP_RESULT_COMPLETE,
            "first single-frame transmit completes");

    mblink_isotp_tx_reset(&transmitter);
    require(transmitter.state == MBLINK_ISOTP_TX_IDLE &&
            transmitter.payload == payload &&
            transmitter.payload_length == sizeof(payload),
            "TX reset preserves borrowed payload");
    require(mblink_isotp_tx_start(&transmitter, 100U, &second) ==
            MBLINK_ISOTP_RESULT_COMPLETE,
            "reset transmitter can retransmit payload");
    require(first.length == second.length &&
            first.can_id == second.can_id &&
            memcmp(first.data, second.data, first.length) == 0,
            "retransmitted frame matches original");
}

static void test_unrelated_flow_control_is_non_terminal(void)
{
    uint8_t payload[20] = {0};
    MblinkIsoTpTx transmitter;
    MblinkIsoTpCanFrame first;
    const MblinkIsoTpTxConfig config = {
        .address = address(),
        .flow_control_timeout_us = 1000U,
        .max_wait_frames = 0U
    };
    const MblinkIsoTpCanFrame unrelated = {
        .can_id = 0x7e9U,
        .extended_id = false,
        .length = 3U,
        .data = {0x30U, 0U, 0U}
    };
    const MblinkIsoTpCanFrame expected = {
        .can_id = 0x7e8U,
        .extended_id = false,
        .length = 3U,
        .data = {0x30U, 0U, 0U}
    };

    require(mblink_isotp_tx_init(&transmitter, &config,
                                 payload, sizeof(payload)) ==
            MBLINK_ISOTP_RESULT_OK, "unrelated TX init");
    require(mblink_isotp_tx_start(&transmitter, 0U, &first) ==
            MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL,
            "unrelated TX waits for FC");
    require(mblink_isotp_tx_accept_flow_control(
                &transmitter, &unrelated, 10U) ==
            MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME,
            "unrelated FC is rejected");
    require(transmitter.state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL &&
            transmitter.failure == MBLINK_ISOTP_RESULT_OK,
            "unrelated FC does not poison transmitter");
    require(mblink_isotp_tx_accept_flow_control(
                &transmitter, &expected, 20U) ==
            MBLINK_ISOTP_RESULT_OK,
            "expected FC still accepted afterward");
    require(transmitter.state == MBLINK_ISOTP_TX_SENDING,
            "TX continues after unrelated bus traffic");
}

int main(void)
{
    test_rx_failed_state();
    test_tx_failed_state();
    test_tx_reset_reuses_payload();
    test_unrelated_flow_control_is_non_terminal();
    puts("ISO-TP failed-state tests passed.");
    return EXIT_SUCCESS;
}
