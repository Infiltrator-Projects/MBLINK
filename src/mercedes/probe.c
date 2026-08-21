// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file probe.c
 * @brief Read-only Mercedes ECU endpoint probe implementation.
 */
#include "mblink/mercedes_probe.h"

#include <string.h>

#define MERCEDES_PROBE_PDU_CAPACITY (MBLINK_ELM327_MAX_RESPONSE / 2U)

static const uint16_t mercedes_probe_identity_dids[
    MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT] = {
    0xF18CU, /* ECU Serial Number */
    0xF187U, /* Vehicle Manufacturer Spare Part Number */
    0xF188U, /* Vehicle Manufacturer ECU Software Number */
    0xF189U, /* Vehicle Manufacturer ECU Software Version */
    0xF191U, /* Vehicle Manufacturer ECU Hardware Number */
    0xF197U  /* System Name / Engine Type */
};

static const char *const mercedes_probe_identity_names[
    MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT] = {
    "ECU serial number",
    "vehicle manufacturer spare part number",
    "vehicle manufacturer ECU software number",
    "vehicle manufacturer ECU software version",
    "vehicle manufacturer ECU hardware number",
    "system name / engine type"
};

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

static bool mercedes_probe_vin_character_is_valid(uint8_t value)
{
    if (value >= (uint8_t)'0' && value <= (uint8_t)'9') {
        return true;
    }
    if (value < (uint8_t)'A' || value > (uint8_t)'Z') {
        return false;
    }
    return value != (uint8_t)'I' &&
           value != (uint8_t)'O' &&
           value != (uint8_t)'Q';
}

static bool mercedes_probe_copy_vin(
    MblinkMercedesEcuProbe *probe,
    const uint8_t *data,
    size_t data_length)
{
    size_t index;

    if (probe == NULL || data == NULL ||
        data_length != MBLINK_MERCEDES_PROBE_VIN_LENGTH) {
        return false;
    }
    for (index = 0U; index < data_length; ++index) {
        if (!mercedes_probe_vin_character_is_valid(data[index])) {
            return false;
        }
    }
    memcpy(probe->vin, data, data_length);
    probe->vin[data_length] = '\0';
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

static MblinkMercedesEcuProbeResult mercedes_probe_complete(
    MblinkMercedesEcuProbe *probe)
{
    if (probe != NULL) {
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE;
    }
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE;
}

static MblinkMercedesEcuProbeResult mercedes_probe_advance_identity(
    MblinkMercedesEcuProbe *probe)
{
    if (probe == NULL) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    probe->identity_index++;
    if (probe->identity_index >= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
        return mercedes_probe_complete(probe);
    }
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
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
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN:
        return "read-standard-vin";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY:
        return "read-standard-identity";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

const char *mblink_mercedes_ecu_probe_vin_result_name(
    MblinkMercedesEcuProbeVinResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED:
        return "not-attempted";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE:
        return "available";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE:
        return "no-response";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE:
        return "negative-response";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE:
        return "invalid-response";
    }
    return "unknown";
}

size_t mblink_mercedes_ecu_probe_identity_did_count(void)
{
    return MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT;
}

uint16_t mblink_mercedes_ecu_probe_identity_did_at(size_t index)
{
    if (index >= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
        return 0U;
    }
    return mercedes_probe_identity_dids[index];
}

const char *mblink_mercedes_ecu_probe_identity_did_name(size_t index)
{
    if (index >= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
        return NULL;
    }
    return mercedes_probe_identity_names[index];
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
    probe->vin_result = MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED;
    probe->vin_elm_can_result = MBLINK_ELM327_CAN_RESULT_OK;
    probe->vin_elm_result = MBLINK_ELM327_RESULT_OK;
    probe->vin_uds_result = MBLINK_UDS_RESULT_OK;
    probe->vin_negative_response_code = 0U;
    probe->vin[0] = '\0';
    probe->identity_index = 0U;
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
}

static MblinkMercedesEcuProbeResult mercedes_probe_build_read_did_command(
    uint16_t did,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    uint8_t pdu[3];
    size_t pdu_length = 0U;
    MblinkUdsResult uds_result = mblink_uds_build_read_did_request(
        did, pdu, sizeof(pdu), &pdu_length);
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

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN) {
        return mercedes_probe_build_read_did_command(
            MBLINK_MERCEDES_PROBE_VIN_DID,
            buffer, buffer_size, written);
    }

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY) {
        if (probe->identity_index >= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
        }
        return mercedes_probe_build_read_did_command(
            mercedes_probe_identity_dids[probe->identity_index],
            buffer, buffer_size, written);
    }

    return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
}

static MblinkMercedesEcuProbeResult mercedes_probe_begin_identity(
    MblinkMercedesEcuProbe *probe)
{
    if (probe == NULL) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    probe->identity_index = 0U;
    probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY;
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
}

static MblinkMercedesEcuProbeResult mercedes_probe_accept_vin(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MERCEDES_PROBE_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkElm327CanResult elm_result;
    MblinkUdsDidRecord record;
    MblinkUdsResult uds_result;

    elm_result = mblink_elm327_can_decode_pdu(
        response, pdu, sizeof(pdu), &pdu_length);
    probe->vin_elm_can_result = elm_result;
    probe->vin_elm_result = response->result;

    if (elm_result != MBLINK_ELM327_CAN_RESULT_OK) {
        probe->vin_result = response->result == MBLINK_ELM327_RESULT_NO_DATA
            ? MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE
            : MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE;
        return mercedes_probe_begin_identity(probe);
    }

    uds_result = mblink_uds_decode_read_did_response(
        pdu, pdu_length, MBLINK_MERCEDES_PROBE_VIN_DID, &record);
    probe->vin_uds_result = uds_result;

    if (uds_result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
        MblinkUdsResponse decoded;
        if (mblink_uds_decode_response(
                MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
                pdu, pdu_length, &decoded) ==
            MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
            probe->vin_negative_response_code = decoded.negative_response_code;
        }
        probe->vin_result = MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE;
        return mercedes_probe_begin_identity(probe);
    }

    if (uds_result != MBLINK_UDS_RESULT_OK ||
        !mercedes_probe_copy_vin(probe, record.data, record.data_length)) {
        probe->vin_result = MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE;
        return mercedes_probe_begin_identity(probe);
    }

    probe->vin_result = MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE;
    return mercedes_probe_begin_identity(probe);
}

static MblinkMercedesEcuProbeResult mercedes_probe_accept_identity(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MERCEDES_PROBE_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkElm327CanResult elm_result;
    MblinkUdsDidRecord record;
    MblinkUdsResult uds_result;
    uint16_t did;
    uint32_t bit;

    if (probe->identity_index >= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }

    did = mercedes_probe_identity_dids[probe->identity_index];
    bit = (uint32_t)1U << probe->identity_index;
    elm_result = mblink_elm327_can_decode_pdu(
        response, pdu, sizeof(pdu), &pdu_length);

    if (elm_result != MBLINK_ELM327_CAN_RESULT_OK) {
        if (response->result == MBLINK_ELM327_RESULT_NO_DATA) {
            probe->identity_no_response_mask |= bit;
        } else {
            probe->identity_invalid_mask |= bit;
        }
        return mercedes_probe_advance_identity(probe);
    }

    uds_result = mblink_uds_decode_read_did_response(
        pdu, pdu_length, did, &record);
    if (uds_result == MBLINK_UDS_RESULT_OK) {
        probe->identity_positive_mask |= bit;
    } else if (uds_result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
        probe->identity_negative_mask |= bit;
    } else {
        probe->identity_invalid_mask |= bit;
    }
    return mercedes_probe_advance_identity(probe);
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

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN) {
        return mercedes_probe_accept_vin(probe, response);
    }

    if (probe->stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY) {
        return mercedes_probe_accept_identity(probe, response);
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

    probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN;
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
}
