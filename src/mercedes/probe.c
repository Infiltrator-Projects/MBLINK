// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file probe.c
 * @brief Read-only Mercedes ECU endpoint probe implementation.
 */
#include "mblink/mercedes_probe.h"

#include <string.h>

#define MERCEDES_PROBE_PDU_CAPACITY (MBLINK_ELM327_MAX_RESPONSE / 2U)

static bool mercedes_probe_channel_config(
    const MblinkMercedesEcuEndpointDefinition *endpoint,
    MblinkElm327CanChannelConfig *config)
{
    const MblinkIsoTpAddress *address;

    if (!mblink_mercedes_ecu_endpoint_is_valid(endpoint) || config == NULL) {
        return false;
    }

    address = &endpoint->address;
    if (address->addressing_mode != MBLINK_ISOTP_ADDRESSING_NORMAL ||
        address->target_type != MBLINK_ISOTP_TARGET_PHYSICAL ||
        address->tx_extended_id != address->rx_extended_id ||
        address->tx_address_extension != 0U ||
        address->rx_address_extension != 0U) {
        return false;
    }

    config->tx_can_id = address->tx_can_id;
    config->rx_can_id = address->rx_can_id;
    config->extended_id = address->tx_extended_id;
    return true;
}

static MblinkMercedesEcuProbeResult mercedes_probe_fail(
    MblinkMercedesEcuProbe *probe,
    MblinkMercedesEcuProbeResult failure)
{
    if (probe != NULL) {
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED;
        probe->failure = failure;
    }
    return failure;
}

const char *mblink_mercedes_ecu_probe_result_name(
    MblinkMercedesEcuProbeResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_OK: return "ok";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE: return "complete";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT:
        return "invalid-argument";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT:
        return "unsupported-endpoint";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL:
        return "buffer-too-small";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR:
        return "channel-error";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR: return "pdu-error";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR: return "uds-error";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE:
        return "failed-state";
    }
    return "unknown";
}

const char *mblink_mercedes_ecu_probe_stage_name(
    MblinkMercedesEcuProbeStage stage)
{
    switch (stage) {
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL:
        return "configure-channel";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT:
        return "tester-present";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_begin(
    MblinkMercedesEcuProbe *probe,
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    MblinkElm327CanChannelConfig channel_config;
    MblinkElm327CanResult result;

    if (probe == NULL || !mblink_mercedes_ecu_endpoint_is_valid(endpoint)) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    if (!mercedes_probe_channel_config(endpoint, &channel_config)) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT;
    }

    memset(probe, 0, sizeof(*probe));
    result = mblink_elm327_can_channel_begin(&probe->channel,
                                             &channel_config);
    if (result != MBLINK_ELM327_CAN_RESULT_OK) {
        probe->elm_can_failure = result;
        return mercedes_probe_fail(
            probe, MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR);
    }

    probe->endpoint = endpoint;
    probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL;
    probe->failure = MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
    probe->elm_can_failure = MBLINK_ELM327_CAN_RESULT_OK;
    probe->elm_failure = MBLINK_ELM327_RESULT_OK;
    probe->uds_failure = MBLINK_UDS_RESULT_OK;
    probe->uds_negative_response_code = 0U;
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
}

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_command(
    const MblinkMercedesEcuProbe *probe,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    if (written != NULL) *written = 0U;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (probe == NULL || buffer == NULL || buffer_size == 0U ||
        written == NULL ||
        !mblink_mercedes_ecu_endpoint_is_valid(probe->endpoint)) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL) {
        MblinkElm327CanResult result = mblink_elm327_can_channel_command(
            &probe->channel, buffer, buffer_size);
        if (result == MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL;
        }
        if (result != MBLINK_ELM327_CAN_RESULT_OK) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR;
        }
        *written = strlen(buffer);
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
    }

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT) {
        uint8_t pdu[2];
        size_t pdu_length = 0U;
        MblinkUdsResult uds_result = mblink_uds_build_tester_present_request(
            false, pdu, sizeof(pdu), &pdu_length);
        MblinkElm327CanResult elm_result;

        if (uds_result != MBLINK_UDS_RESULT_OK) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR;
        }
        elm_result = mblink_elm327_can_build_pdu_command(
            pdu, pdu_length, buffer, buffer_size, written);
        if (elm_result == MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL;
        }
        if (elm_result != MBLINK_ELM327_CAN_RESULT_OK) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR;
        }
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
    }

    return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
}

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_accept(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response)
{
    if (probe == NULL || response == NULL ||
        !mblink_mercedes_ecu_endpoint_is_valid(probe->endpoint)) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED ||
        probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL) {
        MblinkElm327CanResult result = mblink_elm327_can_channel_accept(
            &probe->channel, response);
        if (result != MBLINK_ELM327_CAN_RESULT_OK) {
            probe->elm_can_failure = result;
            probe->elm_failure = probe->channel.elm_failure;
            return mercedes_probe_fail(
                probe, MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR);
        }
        if (probe->channel.stage == MBLINK_ELM327_CAN_STAGE_COMPLETE) {
            probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT;
        }
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
    }

    if (probe->stage != MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }

    {
        uint8_t pdu[MERCEDES_PROBE_PDU_CAPACITY];
        size_t pdu_length = 0U;
        MblinkElm327CanResult elm_result = mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length);
        MblinkUdsResult uds_result;

        if (elm_result != MBLINK_ELM327_CAN_RESULT_OK) {
            probe->elm_can_failure = elm_result;
            probe->elm_failure = response->result;
            return mercedes_probe_fail(
                probe, MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR);
        }
        uds_result = mblink_uds_decode_tester_present_response(
            pdu, pdu_length);
        if (uds_result != MBLINK_UDS_RESULT_OK) {
            if (uds_result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
                MblinkUdsResponse decoded;
                if (mblink_uds_decode_response(
                        MBLINK_UDS_SERVICE_TESTER_PRESENT,
                        pdu, pdu_length, &decoded) ==
                    MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
                    probe->uds_negative_response_code =
                        decoded.negative_response_code;
                }
            }
            probe->uds_failure = uds_result;
            return mercedes_probe_fail(
                probe, MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR);
        }
    }

    probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE;
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE;
}
