// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file probe.c
 * @brief Mercedes evidence/profile adapter over LINK's generic ECU probe.
 */
#include "mblink/mercedes_probe.h"

#include <string.h>

static const uint16_t mercedes_probe_identity_dids[
    MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT] = {
    0xF18CU, 0xF187U, 0xF188U, 0xF189U, 0xF191U, 0xF197U
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

static const uint16_t mercedes_probe_crd3_dids[
    MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT] = {
    0xF100U, 0xF154U, 0xF196U, 0x1001U, 0x1002U
};

static const char *const mercedes_probe_crd3_names[
    MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT] = {
    "CRD3 session / variant",
    "CRD3 supplier identifier",
    "CRD3 EROTAN",
    "CRD3 full variant coding",
    "CRD3 partial variant coding"
};

static const LinkEcuProbeDidRequest mercedes_probe_requests[] = {
    {MBLINK_MERCEDES_PROBE_VIN_DID, "vehicle.vin", "Vehicle identification number"},
    {0xF18CU, "ecu.serial", "ECU serial number"},
    {0xF187U, "ecu.part-number", "vehicle manufacturer spare part number"},
    {0xF188U, "ecu.software-number", "vehicle manufacturer ECU software number"},
    {0xF189U, "ecu.software-version", "vehicle manufacturer ECU software version"},
    {0xF191U, "ecu.hardware-number", "vehicle manufacturer ECU hardware number"},
    {0xF197U, "ecu.system-name", "system name / engine type"},
    {0xF100U, "mercedes.crd3.session-variant", "CRD3 session / variant"},
    {0xF154U, "mercedes.crd3.supplier", "CRD3 supplier identifier"},
    {0xF196U, "mercedes.crd3.erotan", "CRD3 EROTAN"},
    {0x1001U, "mercedes.crd3.full-variant-coding", "CRD3 full variant coding"},
    {0x1002U, "mercedes.crd3.partial-variant-coding", "CRD3 partial variant coding"}
};

static bool mercedes_probe_channel_config(
    const MblinkMercedesEcuEndpointDefinition *endpoint,
    LinkElm327CanChannelConfig *config)
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
    if (value >= (uint8_t)'0' && value <= (uint8_t)'9') return true;
    if (value < (uint8_t)'A' || value > (uint8_t)'Z') return false;
    return value != (uint8_t)'I' && value != (uint8_t)'O' && value != (uint8_t)'Q';
}

static MblinkMercedesEcuProbeResult mercedes_probe_map_result(LinkEcuProbeResult result)
{
    switch (result) {
    case LINK_ECU_PROBE_RESULT_OK: return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
    case LINK_ECU_PROBE_RESULT_COMPLETE: return MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE;
    case LINK_ECU_PROBE_RESULT_INVALID_ARGUMENT: return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    case LINK_ECU_PROBE_RESULT_BUFFER_TOO_SMALL: return MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL;
    case LINK_ECU_PROBE_RESULT_CHANNEL_ERROR: return MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR;
    case LINK_ECU_PROBE_RESULT_PDU_ERROR: return MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR;
    case LINK_ECU_PROBE_RESULT_UDS_ERROR: return MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR;
    case LINK_ECU_PROBE_RESULT_BLOCKED_BY_POLICY:
    case LINK_ECU_PROBE_RESULT_FAILED_STATE:
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }
    return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
}

static MblinkMercedesEcuProbeVinResult mercedes_probe_map_vin_status(
    LinkEcuProbeReadStatus status)
{
    switch (status) {
    case LINK_ECU_PROBE_READ_NOT_ATTEMPTED:
        return MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED;
    case LINK_ECU_PROBE_READ_AVAILABLE:
        return MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE;
    case LINK_ECU_PROBE_READ_NO_RESPONSE:
        return MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE;
    case LINK_ECU_PROBE_READ_NEGATIVE_RESPONSE:
        return MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE;
    case LINK_ECU_PROBE_READ_INVALID_RESPONSE:
        return MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE;
    }
    return MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE;
}

static MblinkMercedesEcuProbeDtcResult mercedes_probe_map_dtc_status(
    LinkEcuProbeReadStatus status)
{
    switch (status) {
    case LINK_ECU_PROBE_READ_NOT_ATTEMPTED:
        return MBLINK_MERCEDES_ECU_PROBE_DTC_NOT_ATTEMPTED;
    case LINK_ECU_PROBE_READ_AVAILABLE:
        return MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE;
    case LINK_ECU_PROBE_READ_NO_RESPONSE:
        return MBLINK_MERCEDES_ECU_PROBE_DTC_NO_RESPONSE;
    case LINK_ECU_PROBE_READ_NEGATIVE_RESPONSE:
        return MBLINK_MERCEDES_ECU_PROBE_DTC_NEGATIVE_RESPONSE;
    case LINK_ECU_PROBE_READ_INVALID_RESPONSE:
        return MBLINK_MERCEDES_ECU_PROBE_DTC_INVALID_RESPONSE;
    }
    return MBLINK_MERCEDES_ECU_PROBE_DTC_INVALID_RESPONSE;
}

static void mercedes_probe_sync_stage(MblinkMercedesEcuProbe *probe)
{
    if (probe == NULL) return;
    switch (probe->shared.stage) {
    case LINK_ECU_PROBE_STAGE_CONFIGURE_CHANNEL:
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL;
        break;
    case LINK_ECU_PROBE_STAGE_TESTER_PRESENT:
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT;
        break;
    case LINK_ECU_PROBE_STAGE_READ_DID:
        if (probe->shared.did_index == 0U) {
            probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN;
        } else if (probe->shared.did_index <= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
            probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY;
        } else {
            probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_CRD3_FINGERPRINT;
        }
        break;
    case LINK_ECU_PROBE_STAGE_READ_DTC_INFORMATION:
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION;
        break;
    case LINK_ECU_PROBE_STAGE_COMPLETE:
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE;
        break;
    case LINK_ECU_PROBE_STAGE_FAILED:
        probe->stage = MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED;
        break;
    }

    if (probe->shared.did_index == 0U) {
        probe->identity_index = 0U;
        probe->crd3_index = 0U;
    } else if (probe->shared.did_index <= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
        probe->identity_index = probe->shared.did_index - 1U;
        probe->crd3_index = 0U;
    } else {
        probe->identity_index = MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT;
        probe->crd3_index = probe->shared.did_index -
            (MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 1U);
        if (probe->crd3_index > MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT) {
            probe->crd3_index = MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT;
        }
    }
}

static void mercedes_probe_set_mask(
    LinkEcuProbeReadStatus status,
    uint32_t bit,
    uint32_t *positive,
    uint32_t *negative,
    uint32_t *no_response,
    uint32_t *invalid)
{
    switch (status) {
    case LINK_ECU_PROBE_READ_AVAILABLE: *positive |= bit; break;
    case LINK_ECU_PROBE_READ_NEGATIVE_RESPONSE: *negative |= bit; break;
    case LINK_ECU_PROBE_READ_NO_RESPONSE: *no_response |= bit; break;
    case LINK_ECU_PROBE_READ_INVALID_RESPONSE: *invalid |= bit; break;
    case LINK_ECU_PROBE_READ_NOT_ATTEMPTED: break;
    }
}

static void mercedes_probe_sync_evidence(MblinkMercedesEcuProbe *probe)
{
    const LinkEcuProbeDidResult *vin;
    size_t index;

    if (probe == NULL) return;
    probe->channel = probe->shared.channel;
    probe->failure = mercedes_probe_map_result(probe->shared.failure);
    probe->elm_can_failure = probe->shared.elm_can_failure;
    probe->elm_failure = probe->shared.elm_failure;
    probe->uds_failure = probe->shared.uds_failure;
    probe->uds_negative_response_code = probe->shared.uds_negative_response_code;

    probe->identity_positive_mask = 0U;
    probe->identity_negative_mask = 0U;
    probe->identity_no_response_mask = 0U;
    probe->identity_invalid_mask = 0U;
    probe->crd3_positive_mask = 0U;
    probe->crd3_negative_mask = 0U;
    probe->crd3_no_response_mask = 0U;
    probe->crd3_invalid_mask = 0U;

    vin = link_ecu_probe_did_result_at(&probe->shared, 0U);
    probe->vin[0] = '\0';
    if (vin != NULL) {
        size_t vin_index;
        probe->vin_result = mercedes_probe_map_vin_status(vin->status);
        probe->vin_elm_can_result = vin->elm_can_result;
        probe->vin_elm_result = vin->elm_result;
        probe->vin_uds_result = vin->uds_result;
        probe->vin_negative_response_code = vin->negative_response_code;
        if (vin->status == LINK_ECU_PROBE_READ_AVAILABLE) {
            bool valid = vin->data_length == MBLINK_MERCEDES_PROBE_VIN_LENGTH;
            for (vin_index = 0U; valid && vin_index < vin->data_length; ++vin_index) {
                valid = mercedes_probe_vin_character_is_valid(vin->data[vin_index]);
            }
            if (valid) {
                memcpy(probe->vin, vin->data, MBLINK_MERCEDES_PROBE_VIN_LENGTH);
                probe->vin[MBLINK_MERCEDES_PROBE_VIN_LENGTH] = '\0';
            } else {
                probe->vin_result = MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE;
            }
        }
    }

    for (index = 1U; index < sizeof(mercedes_probe_requests) / sizeof(mercedes_probe_requests[0]); ++index) {
        const LinkEcuProbeDidResult *result = link_ecu_probe_did_result_at(&probe->shared, index);
        const uint32_t evidence_bit = UINT32_C(1) << (index - 1U);
        if (result == NULL) continue;
        mercedes_probe_set_mask(result->status, evidence_bit,
                                &probe->identity_positive_mask,
                                &probe->identity_negative_mask,
                                &probe->identity_no_response_mask,
                                &probe->identity_invalid_mask);
        if (index > MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) {
            const size_t crd3_index = index - (MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 1U);
            const uint32_t crd3_bit = UINT32_C(1) << crd3_index;
            mercedes_probe_set_mask(result->status, crd3_bit,
                                    &probe->crd3_positive_mask,
                                    &probe->crd3_negative_mask,
                                    &probe->crd3_no_response_mask,
                                    &probe->crd3_invalid_mask);
        }
    }

    probe->crd3_session_variant_available = false;
    probe->crd3_supplier_available = false;
    {
        const LinkEcuProbeDidResult *result = link_ecu_probe_did_result_at(
            &probe->shared, MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 1U);
        if (result != NULL && result->status == LINK_ECU_PROBE_READ_AVAILABLE) {
            probe->crd3_session_variant_available =
                mblink_mercedes_crd3_decode_session_variant(
                    result->data, result->data_length, &probe->crd3_session_variant);
        }
    }
    {
        const LinkEcuProbeDidResult *result = link_ecu_probe_did_result_at(
            &probe->shared, MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + 2U);
        if (result != NULL && result->status == LINK_ECU_PROBE_READ_AVAILABLE) {
            probe->crd3_supplier_available =
                mblink_mercedes_crd3_decode_supplier(
                    result->data, result->data_length, &probe->crd3_supplier);
        }
    }

    probe->dtc_result = mercedes_probe_map_dtc_status(probe->shared.dtc_status);
    probe->dtc_elm_can_result = probe->shared.dtc_elm_can_result;
    probe->dtc_elm_result = probe->shared.dtc_elm_result;
    probe->dtc_uds_result = probe->shared.dtc_uds_result;
    probe->dtc_negative_response_code = probe->shared.dtc_negative_response_code;
    probe->dtcs = probe->shared.dtcs;
    mercedes_probe_sync_stage(probe);
}

const char *mblink_mercedes_ecu_probe_result_name(MblinkMercedesEcuProbeResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_OK: return "ok";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE: return "complete";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT: return "invalid-argument";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT: return "unsupported-endpoint";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL: return "buffer-too-small";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR: return "channel-error";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR: return "pdu-error";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR: return "uds-error";
    case MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE: return "failed-state";
    }
    return "unknown";
}

const char *mblink_mercedes_ecu_probe_stage_name(MblinkMercedesEcuProbeStage stage)
{
    switch (stage) {
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL: return "configure-channel";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT: return "tester-present";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN: return "read-standard-vin";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY: return "read-standard-identity";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_CRD3_FINGERPRINT: return "read-crd3-fingerprint";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION: return "read-dtc-information";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

const char *mblink_mercedes_ecu_probe_vin_result_name(MblinkMercedesEcuProbeVinResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED: return "not-attempted";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE: return "available";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE: return "no-response";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE: return "negative-response";
    case MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE: return "invalid-response";
    }
    return "unknown";
}

const char *mblink_mercedes_ecu_probe_dtc_result_name(MblinkMercedesEcuProbeDtcResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NOT_ATTEMPTED: return "not-attempted";
    case MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE: return "available";
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NO_RESPONSE: return "no-response";
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NEGATIVE_RESPONSE: return "negative-response";
    case MBLINK_MERCEDES_ECU_PROBE_DTC_INVALID_RESPONSE: return "invalid-response";
    }
    return "unknown";
}

size_t mblink_mercedes_ecu_probe_identity_did_count(void)
{
    return MBLINK_MERCEDES_PROBE_EVIDENCE_DID_COUNT;
}

uint16_t mblink_mercedes_ecu_probe_identity_did_at(size_t index)
{
    if (index < MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) return mercedes_probe_identity_dids[index];
    index -= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT;
    if (index < MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT) return mercedes_probe_crd3_dids[index];
    return 0U;
}

const char *mblink_mercedes_ecu_probe_identity_did_name(size_t index)
{
    if (index < MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT) return mercedes_probe_identity_names[index];
    index -= MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT;
    if (index < MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT) return mercedes_probe_crd3_names[index];
    return NULL;
}

size_t mblink_mercedes_ecu_probe_crd3_did_count(void)
{
    return MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT;
}

uint16_t mblink_mercedes_ecu_probe_crd3_did_at(size_t index)
{
    return index < MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT ? mercedes_probe_crd3_dids[index] : 0U;
}

const char *mblink_mercedes_ecu_probe_crd3_did_name(size_t index)
{
    return index < MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT ? mercedes_probe_crd3_names[index] : NULL;
}

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_begin(
    MblinkMercedesEcuProbe *probe,
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    LinkElm327CanChannelConfig channel;
    LinkEcuProbeProfile profile;
    LinkEcuProbeResult result;

    if (probe == NULL || !mblink_mercedes_ecu_endpoint_is_valid(endpoint)) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    if (!mercedes_probe_channel_config(endpoint, &channel)) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT;
    }

    memset(probe, 0, sizeof(*probe));
    probe->endpoint = endpoint;
    profile.channel = channel;
    profile.dids = mercedes_probe_requests;
    profile.did_count = sizeof(mercedes_probe_requests) / sizeof(mercedes_probe_requests[0]);
    profile.tester_present = true;
    profile.read_dtcs = true;
    result = link_ecu_probe_begin(&probe->shared, &profile);
    mercedes_probe_sync_evidence(probe);
    return mercedes_probe_map_result(result);
}

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_command(
    const MblinkMercedesEcuProbe *probe,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    if (probe == NULL || probe->stage < MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL ||
        probe->stage > MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED) {
        if (written != NULL) *written = 0U;
        if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }
    return mercedes_probe_map_result(
        link_ecu_probe_command(&probe->shared, buffer, buffer_size, written));
}

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_accept(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response)
{
    LinkEcuProbeResult result;

    if (probe == NULL || response == NULL) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    if (probe->stage < MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL ||
        probe->stage > MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }
    result = link_ecu_probe_accept(&probe->shared, response);
    mercedes_probe_sync_evidence(probe);
    return mercedes_probe_map_result(result);
}
