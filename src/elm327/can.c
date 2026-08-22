// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file can.c
 * @brief MBLINK ABI wrappers for LINK's ELM-managed CAN channel.
 */
#include "mblink/elm327_can.h"

const char *mblink_elm327_can_result_name(MblinkElm327CanResult result)
{
    return link_elm327_can_result_name(result);
}

const char *mblink_elm327_can_stage_name(MblinkElm327CanStage stage)
{
    return link_elm327_can_stage_name(stage);
}

bool mblink_elm327_can_channel_config_is_valid(
    const MblinkElm327CanChannelConfig *config)
{
    return link_elm327_can_channel_config_is_valid(config);
}

MblinkElm327CanResult mblink_elm327_can_channel_begin(
    MblinkElm327CanChannelState *state,
    const MblinkElm327CanChannelConfig *config)
{
    return link_elm327_can_channel_begin(state, config);
}

MblinkElm327CanResult mblink_elm327_can_channel_command(
    const MblinkElm327CanChannelState *state,
    char *buffer,
    size_t buffer_size)
{
    return link_elm327_can_channel_command(state, buffer, buffer_size);
}

MblinkElm327CanResult mblink_elm327_can_channel_accept(
    MblinkElm327CanChannelState *state,
    const MblinkElm327Response *response)
{
    return link_elm327_can_channel_accept(state, response);
}

MblinkElm327CanResult mblink_elm327_can_build_pdu_command(
    const uint8_t *pdu,
    size_t pdu_length,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    return link_elm327_can_build_pdu_command(
        pdu, pdu_length, buffer, buffer_size, written);
}

MblinkElm327CanResult mblink_elm327_can_decode_pdu(
    const MblinkElm327Response *response,
    uint8_t *pdu,
    size_t pdu_size,
    size_t *pdu_length)
{
    return link_elm327_can_decode_pdu(response, pdu, pdu_size, pdu_length);
}
