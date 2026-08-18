// SPDX-License-Identifier: GPL-3.0-or-later
#include "isotp_test_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_sequence_mismatch_and_timeout(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpReceiver receiver;
    MblinkCanFrame frame;
    MblinkCanFrame reply;
    const uint8_t ff[] = {0x10U,0x0AU,1U,2U,3U,4U,5U,6U};
    const uint8_t wrong[] = {0x22U,7U,8U,9U,10U,0U,0U,0U};

    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options), "seq receiver init");
    frame = isotp_test_rx_frame(ff, sizeof(ff));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 100U, &reply) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "seq FF accepted");
    frame = isotp_test_rx_frame(wrong, sizeof(wrong));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 200U, &reply) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     receiver.error == MBLINK_ISOTP_ERROR_SEQUENCE,
                 "sequence mismatch rejected");

    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options), "timeout receiver init");
    frame = isotp_test_rx_frame(ff, sizeof(ff));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 500U, &reply) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "timeout FF accepted");
    isotp_test_require(mblink_isotp_receiver_tick(&receiver, 100500U) == MBLINK_ISOTP_STEP_ERROR &&
                     receiver.error == MBLINK_ISOTP_ERROR_TIMEOUT,
                 "rx timeout enforced");
}

static void test_sender_flow_control(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpSender sender;
    MblinkCanFrame frame;
    MblinkCanFrame fc;
    uint8_t payload[30];
    size_t i;
    const uint8_t fc_data[] = {0x30U,0x02U,0x05U};

    for (i = 0U; i < sizeof(payload); ++i) {
        payload[i] = (uint8_t)i;
    }
    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "flow sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 1000U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY &&
                     sender.state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL,
                 "FF sender waits for FC");
    isotp_test_require(frame.data[0] == 0x10U && frame.data[1] == 30U,
                 "FF length encoded");
    fc = isotp_test_rx_frame(fc_data, sizeof(fc_data));
    isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &fc, 2000U) ==
                     MBLINK_ISOTP_STEP_IN_PROGRESS &&
                     sender.state == MBLINK_ISOTP_TX_SENDING,
                 "CTS starts CF sending");
    isotp_test_require(sender.separation_time_us == 5000U && sender.block_size == 2U,
                 "FC BS/STmin applied");
    isotp_test_require(mblink_isotp_sender_next(&sender, 2000U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY && frame.data[0] == 0x21U,
                 "first CF immediate");
    isotp_test_require(mblink_isotp_sender_next(&sender, 6999U, &frame) ==
                     MBLINK_ISOTP_STEP_NOT_DUE,
                 "STmin blocks early CF");
    isotp_test_require(mblink_isotp_sender_next(&sender, 7000U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY && frame.data[0] == 0x22U &&
                     sender.state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL,
                 "block size pauses after two CFs");
}

static void test_wait_overflow_and_fc_timeout(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpSender sender;
    MblinkCanFrame frame;
    MblinkCanFrame fc;
    uint8_t payload[12] = {0};
    const uint8_t wait_data[] = {0x31U,0x00U,0x00U};
    const uint8_t ovfl_data[] = {0x32U,0x00U,0x00U};

    options.max_wait_frames = 1U;
    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "wait sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 0U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "wait sender FF");
    fc = isotp_test_rx_frame(wait_data, sizeof(wait_data));
    isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &fc, 10U) ==
                     MBLINK_ISOTP_STEP_IN_PROGRESS,
                 "first WAIT accepted");
    isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &fc, 20U) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     sender.error == MBLINK_ISOTP_ERROR_WAIT_FRAME_LIMIT,
                 "WAIT limit enforced");

    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "overflow sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 0U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "overflow sender FF");
    fc = isotp_test_rx_frame(ovfl_data, sizeof(ovfl_data));
    isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &fc, 10U) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     sender.error == MBLINK_ISOTP_ERROR_FLOW_CONTROL_OVERFLOW,
                 "overflow FC enforced");

    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "fc timeout sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 100U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "fc timeout sender FF");
    isotp_test_require(mblink_isotp_sender_tick(&sender, 100100U) == MBLINK_ISOTP_STEP_ERROR &&
                     sender.error == MBLINK_ISOTP_ERROR_TIMEOUT,
                 "flow control timeout enforced");
}

static void test_deadlines_without_explicit_tick(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpReceiver receiver;
    MblinkIsoTpSender sender;
    MblinkCanFrame frame;
    MblinkCanFrame reply;
    uint8_t payload[12] = {0};
    const uint8_t ff[] = {0x10U,0x0AU,1U,2U,3U,4U,5U,6U};
    const uint8_t cf[] = {0x21U,7U,8U,9U,10U,0U,0U,0U};
    const uint8_t fc_data[] = {0x30U,0x00U,0x00U};

    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options),
                 "late receiver init");
    frame = isotp_test_rx_frame(ff, sizeof(ff));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 100U, &reply) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "late FF accepted");
    frame = isotp_test_rx_frame(cf, sizeof(cf));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 100100U, &reply) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     receiver.error == MBLINK_ISOTP_ERROR_TIMEOUT,
                 "late CF rejected without explicit tick");

    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options),
                 "late sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 100U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "late sender FF");
    frame = isotp_test_rx_frame(fc_data, sizeof(fc_data));
    isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &frame, 100100U) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     sender.error == MBLINK_ISOTP_ERROR_TIMEOUT,
                 "late FC rejected without explicit tick");
}

static void test_functional_request_can_receive_multiframe_response(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpReceiver receiver;
    MblinkCanFrame frame;
    MblinkCanFrame reply;
    const uint8_t ff[] = {0x10U,0x0AU,1U,2U,3U,4U,5U,6U};

    address.target_type = MBLINK_ISOTP_TARGET_FUNCTIONAL;
    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options),
                 "functional response receiver init");
    frame = isotp_test_rx_frame(ff, sizeof(ff));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 0U, &reply) ==
                     MBLINK_ISOTP_STEP_FRAME_READY &&
                     receiver.state == MBLINK_ISOTP_RX_RECEIVING,
                 "physical multi-frame response remains receivable after functional request");
}

static void test_functional_multiframe_and_padding(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpSender sender;
    MblinkCanFrame frame;
    uint8_t payload[8] = {0};
    const uint8_t short_payload[] = {1U,2U};

    address.target_type = MBLINK_ISOTP_TARGET_FUNCTIONAL;
    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "functional sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 0U, &frame) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     sender.error == MBLINK_ISOTP_ERROR_FUNCTIONAL_MULTIFRAME,
                 "functional multi-frame rejected");

    address.target_type = MBLINK_ISOTP_TARGET_PHYSICAL;
    options.tx_padding_enabled = true;
    options.tx_padding_byte = 0xAAU;
    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "padding sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, short_payload, sizeof(short_payload), 0U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "padded SF generated");
    isotp_test_require(frame.data_length == 8U && frame.data[0] == 2U &&
                     frame.data[1] == 1U && frame.data[2] == 2U &&
                     frame.data[7] == 0xAAU,
                 "padding applied");
}

int main(void)
{
    test_sequence_mismatch_and_timeout();
    test_sender_flow_control();
    test_wait_overflow_and_fc_timeout();
    test_deadlines_without_explicit_tick();
    test_functional_request_can_receive_multiframe_response();
    test_functional_multiframe_and_padding();
    puts("MBLINK ISO-TP flow tests passed.");
    return EXIT_SUCCESS;
}
