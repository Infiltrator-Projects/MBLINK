// SPDX-License-Identifier: GPL-3.0-or-later
#include "isotp_test_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_full_round_trip_and_sequence_wrap(void)
{
    MblinkIsoTpAddress address = isotp_test_address();
    MblinkIsoTpAddress peer_address = isotp_test_address();
    MblinkIsoTpOptions rx_options = isotp_test_options();
    MblinkIsoTpOptions tx_options = isotp_test_options();
    MblinkIsoTpSender sender;
    MblinkIsoTpReceiver receiver;
    MblinkCanFrame tx;
    MblinkCanFrame fc;
    uint8_t payload[140];
    const uint8_t *decoded;
    size_t decoded_length;
    size_t i;
    uint64_t now = 0U;
    MblinkIsoTpStepResult step;

    peer_address.tx = address.rx;
    peer_address.rx = address.tx;
    rx_options.receive_block_size = 4U;
    rx_options.receive_stmin = 0U;
    for (i = 0U; i < sizeof(payload); ++i) {
        payload[i] = (uint8_t)(i ^ 0x5AU);
    }
    isotp_test_require(mblink_isotp_sender_init(&sender, &address, &tx_options), "wrap sender init");
    isotp_test_require(mblink_isotp_receiver_init(&receiver, &peer_address, &rx_options), "wrap receiver init");
    isotp_test_require(mblink_isotp_sender_start(&sender, payload, sizeof(payload), now, &tx) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "wrap FF generated");
    isotp_test_require(mblink_isotp_receiver_feed(&receiver, &tx, now, &fc) ==
                     MBLINK_ISOTP_STEP_FRAME_READY,
                 "wrap receiver FC");
    isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &fc, now) ==
                     MBLINK_ISOTP_STEP_IN_PROGRESS,
                 "wrap sender accepts FC");

    while (sender.state != MBLINK_ISOTP_TX_COMPLETE) {
        step = mblink_isotp_sender_next(&sender, now, &tx);
        if (step == MBLINK_ISOTP_STEP_NOT_DUE) {
            now++;
            continue;
        }
        isotp_test_require(step == MBLINK_ISOTP_STEP_FRAME_READY, "wrap CF generated");
        step = mblink_isotp_receiver_feed(&receiver, &tx, now, &fc);
        if (step == MBLINK_ISOTP_STEP_FRAME_READY) {
            isotp_test_require(sender.state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL,
                         "receiver FC aligns with sender block wait");
            isotp_test_require(mblink_isotp_sender_accept_flow_control(&sender, &fc, now) ==
                             MBLINK_ISOTP_STEP_IN_PROGRESS,
                         "sender accepts repeated FC");
        } else {
            isotp_test_require(step == MBLINK_ISOTP_STEP_IN_PROGRESS ||
                             step == MBLINK_ISOTP_STEP_COMPLETE,
                         "receiver accepts CF");
        }
        now++;
    }
    isotp_test_require(receiver.state == MBLINK_ISOTP_RX_COMPLETE,
                 "wrap receiver complete");
    decoded = mblink_isotp_receiver_payload(&receiver, &decoded_length);
    isotp_test_require(decoded != NULL && decoded_length == sizeof(payload) &&
                     memcmp(decoded, payload, sizeof(payload)) == 0,
                 "sequence-wrap payload matches");
}

int main(void)
{
    test_full_round_trip_and_sequence_wrap();
    puts("MBLINK ISO-TP roundtrip tests passed.");
    return EXIT_SUCCESS;
}
