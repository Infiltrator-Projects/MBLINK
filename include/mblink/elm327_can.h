// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327_can.h
 * @brief MBLINK compatibility facade for LINK's ELM-managed CAN channel.
 */
#ifndef MBLINK_ELM327_CAN_H
#define MBLINK_ELM327_CAN_H

#include "link/elm327_can.h"
#include "mblink/elm327.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ELM327_CAN_MAX_REQUEST_PDU LINK_ELM327_CAN_MAX_REQUEST_PDU

#define MBLINK_ELM327_CAN_STAGE_SET_HEADER LINK_ELM327_CAN_STAGE_SET_HEADER
#define MBLINK_ELM327_CAN_STAGE_SET_RECEIVE_ADDRESS LINK_ELM327_CAN_STAGE_SET_RECEIVE_ADDRESS
#define MBLINK_ELM327_CAN_STAGE_ENABLE_AUTO_FORMATTING LINK_ELM327_CAN_STAGE_ENABLE_AUTO_FORMATTING
#define MBLINK_ELM327_CAN_STAGE_ENABLE_FLOW_CONTROL LINK_ELM327_CAN_STAGE_ENABLE_FLOW_CONTROL
#define MBLINK_ELM327_CAN_STAGE_COMPLETE LINK_ELM327_CAN_STAGE_COMPLETE
#define MBLINK_ELM327_CAN_STAGE_FAILED LINK_ELM327_CAN_STAGE_FAILED

#define MBLINK_ELM327_CAN_RESULT_OK LINK_ELM327_CAN_RESULT_OK
#define MBLINK_ELM327_CAN_RESULT_INVALID_ARGUMENT LINK_ELM327_CAN_RESULT_INVALID_ARGUMENT
#define MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL LINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL
#define MBLINK_ELM327_CAN_RESULT_PDU_TOO_LARGE LINK_ELM327_CAN_RESULT_PDU_TOO_LARGE
#define MBLINK_ELM327_CAN_RESULT_ELM_ERROR LINK_ELM327_CAN_RESULT_ELM_ERROR
#define MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE LINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE
#define MBLINK_ELM327_CAN_RESULT_UNEXPECTED_RESPONSE LINK_ELM327_CAN_RESULT_UNEXPECTED_RESPONSE
#define MBLINK_ELM327_CAN_RESULT_FAILED_STATE LINK_ELM327_CAN_RESULT_FAILED_STATE

typedef LinkElm327CanChannelConfig MblinkElm327CanChannelConfig;
typedef LinkElm327CanStage MblinkElm327CanStage;
typedef LinkElm327CanResult MblinkElm327CanResult;
typedef LinkElm327CanChannelState MblinkElm327CanChannelState;

const char *mblink_elm327_can_result_name(MblinkElm327CanResult result);
const char *mblink_elm327_can_stage_name(MblinkElm327CanStage stage);
bool mblink_elm327_can_channel_config_is_valid(
    const MblinkElm327CanChannelConfig *config);
MblinkElm327CanResult mblink_elm327_can_channel_begin(
    MblinkElm327CanChannelState *state,
    const MblinkElm327CanChannelConfig *config);
MblinkElm327CanResult mblink_elm327_can_channel_command(
    const MblinkElm327CanChannelState *state,
    char *buffer,
    size_t buffer_size);
MblinkElm327CanResult mblink_elm327_can_channel_accept(
    MblinkElm327CanChannelState *state,
    const MblinkElm327Response *response);
MblinkElm327CanResult mblink_elm327_can_build_pdu_command(
    const uint8_t *pdu,
    size_t pdu_length,
    char *buffer,
    size_t buffer_size,
    size_t *written);
MblinkElm327CanResult mblink_elm327_can_decode_pdu(
    const MblinkElm327Response *response,
    uint8_t *pdu,
    size_t pdu_size,
    size_t *pdu_length);

#ifdef __cplusplus
}
#endif

#endif
