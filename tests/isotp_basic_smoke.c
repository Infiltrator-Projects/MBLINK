// SPDX-License-Identifier: GPL-3.0-or-later
#include "isotp_test_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_stmin(void)
{
    uint64_t us = 999U;
    isotp_test_require(mblink_isotp_stmin_to_us(0x00U, &us) && us == 0U,
                 "STmin zero");
    isotp_test_require(mblink_isotp_stmin_to_us(0x7FU, &us) && us == 127000U,
                 "STmin milliseconds");
    isotp_test_require(mblink_isotp_stmin_to_us(0xF1U, &us) && us == 100U,
                 "STmin 100us");
    isotp_test_require(mblink_isotp_stmin_to_us(0xF9U, &us) && us == 900U,
                 "STmin 900us");
    isotp_test_require(!mblink_isotp_stmin_to_us(0x80U, &us), "reserved STmin rejected");
}

static void test_single_frame_round_trip(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpSender sender;
    MblinkIsoTpReceiver receiver;
    MblinkCanFrame tx;
    MblinkCanFrame incoming;
    const uint8_t payload[] = {0x22U, 0xF1U, 0x90U};
    const uint8_t *decoded;
    size_t decoded_length;

    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "sender init");
    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options), "receiver init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 10U, &tx) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "single frame generated");
    isotp_test_require(sender.state == MBLINK_ISOTP_TX_COMPLETE, "single tx complete");
    isotp_test_require(tx.can_id == 0x7E0U && tx.data_length == 4U,
                 "single frame address and length");
    isotp_test_require(tx.data[0] == 0x03U && memcmp(tx.data + 1U, payload, sizeof(payload)) == 0,
                 "single frame PCI/payload");

    incoming = tx;
    incoming.can_id = 0x7E8U;
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &incoming, 20U, NULL) ==
                     MBLINK_ISOTP_STEP_COMPLETE,
                 "single rx complete");
    decoded = mblink_isotp_receiver_payload(&receiver, &decoded_length);
    isotp_test_require(decoded != NULL && decoded_length == sizeof(payload) &&
                     memcmp(decoded, payload, sizeof(payload)) == 0,
                 "single payload decoded");
}

static void test_extended_addressing(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpSender sender;
    MblinkCanFrame frame;
    const uint8_t payload[] = {0x10U, 0x03U};

    address.format = MBLINK_ISOTP_ADDRESS_EXTENDED;
    address.tx_address_extension = 0xDAU;
    address.rx_address_extension = 0xF1U;
    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &options), "extended sender init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), 0U, &frame) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "extended single generated");
    isotp_test_require(frame.data_length == 4U && frame.data[0] == 0xDAU &&
                     frame.data[1] == 0x02U && frame.data[2] == 0x10U &&
                     frame.data[3] == 0x03U,
                 "extended address byte precedes PCI");
}

static void test_captured_style_reassembly(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpReceiver receiver;
    MblinkCanFrame frame;
    MblinkCanFrame reply;
    const uint8_t ff[] = {0x10U, 0x14U, 0x62U, 0xF1U, 0x90U, 0x57U, 0x44U, 0x44U};
    const uint8_t cf1[] = {0x21U, 0x32U, 0x30U, 0x37U, 0x30U, 0x30U, 0x30U, 0x30U};
    const uint8_t cf2[] = {0x22U, 0x30U, 0x30U, 0x30U, 0x30U, 0x30U, 0x30U, 0x30U};
    const uint8_t expected[] = {
        0x62U,0xF1U,0x90U,0x57U,0x44U,0x44U,
        0x32U,0x30U,0x37U,0x30U,0x30U,0x30U,0x30U,
        0x30U,0x30U,0x30U,0x30U,0x30U,0x30U,0x30U
    };
    const uint8_t *decoded;
    size_t decoded_length;

    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options), "fixture receiver init");
    frame = isotp_test_rx_frame(ff, sizeof(ff));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 1000U, &reply) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "FF requests flow control");
    isotp_test_require(reply.can_id == 0x7E0U && reply.data[0] == 0x30U &&
                     reply.data[1] == 0x00U && reply.data[2] == 0x00U,
                 "CTS flow control built");
    frame = isotp_test_rx_frame(cf1, sizeof(cf1));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 2000U, &reply) ==
                     MBLINK_ISOTP_STEP_IN_PROGRESS,
                 "CF1 accepted");
    frame = isotp_test_rx_frame(cf2, sizeof(cf2));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 3000U, &reply) ==
                     MBLINK_ISOTP_STEP_COMPLETE,
                 "CF2 completes");
    decoded = mblink_isotp_receiver_payload(&receiver, &decoded_length);
    isotp_test_require(decoded != NULL && decoded_length == sizeof(expected) &&
                     memcmp(decoded, expected, sizeof(expected)) == 0,
                 "fixture payload reassembled");
}

static void test_address_filter_and_malformed_sf(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpOptions options = isotp_test_options();
    MblinkIsoTpReceiver receiver;
    MblinkCanFrame frame;
    const uint8_t other_data[] = {0x01U,0x00U};
    const uint8_t bad_data[] = {0x07U,0x01U,0x02U};

    isotp_test_require(mblink_isotp_receiver_init(&receiver, &address, &options), "filter receiver init");
    frame = isotp_test_rx_frame(other_data, sizeof(other_data));
    frame.can_id = 0x7E9U;
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 0U, NULL) ==
                     MBLINK_ISOTP_STEP_IGNORED &&
                     receiver.state == MBLINK_ISOTP_RX_IDLE,
                 "unmatched CAN ID ignored");

    frame = isotp_test_rx_frame(bad_data, sizeof(bad_data));
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &frame, 0U, NULL) ==
                     MBLINK_ISOTP_STEP_ERROR &&
                     receiver.error == MBLINK_ISOTP_ERROR_INVALID_LENGTH,
                 "malformed SF rejected");
}

int main(void)
{
    test_stmin();
    test_single_frame_round_trip();
    test_extended_addressing();
    test_captured_style_reassembly();
    test_address_filter_and_malformed_sf();
    puts("MBLINK ISO-TP basic tests passed.");
    return EXIT_SUCCESS;
}
