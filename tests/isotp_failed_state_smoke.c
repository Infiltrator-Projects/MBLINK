// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/isotp.h"

#include <stdio.h>
#include <stdlib.h>

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
    require(receiver.state == MBLINK_ISOTP_RX_FAILED,
            "RX enters failed state");
    require(mblink_isotp_rx_tick(&receiver, 20U) !=
            MBLINK_ISOTP_RESULT_OK,
            "RX failed state remains observable");
    mblink_isotp_rx_reset(&receiver);
    require(receiver.state == MBLINK_ISOTP_RX_IDLE,
            "RX reset clears failed state");
}

static void test_tx_failed_state(void)
{
    uint8_t payload[20] = {0};
    MblinkIsoTpTx transmitter;
    MblinkIsoTpCanFrame first;
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
    require(transmitter.state == MBLINK_ISOTP_TX_FAILED,
            "TX enters failed state");
    require(mblink_isotp_tx_tick(&transmitter, 20U) !=
            MBLINK_ISOTP_RESULT_OK,
            "TX failed state remains observable");
    mblink_isotp_tx_reset(&transmitter);
    require(transmitter.state == MBLINK_ISOTP_TX_IDLE,
            "TX reset clears failed state");
}

int main(void)
{
    test_rx_failed_state();
    test_tx_failed_state();
    puts("ISO-TP failed-state smoke tests passed.");
    return EXIT_SUCCESS;
}
