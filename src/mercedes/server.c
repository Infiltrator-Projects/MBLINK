// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_server.h"

#include <string.h>

static const MblinkMercedesEcuEndpointDefinition *find_module_endpoint(
    const MblinkMercedesVehicleProfile *profile,
    MblinkMercedesModuleKind module)
{
    size_t index;

    if (!mblink_mercedes_vehicle_profile_is_valid(profile)) return NULL;
    for (index = 0U; index < profile->endpoint_count; ++index) {
        if (profile->endpoints[index].module == module)
            return &profile->endpoints[index];
    }
    return NULL;
}

bool mblink_mercedes_server_init(
    MblinkMercedesServerState *state,
    const MblinkMercedesServerConfig *config)
{
    const MblinkMercedesVehicleProfile *profile;
    const MblinkMercedesEcuEndpointDefinition *endpoint;

    if (state == NULL || config == NULL || config->vin == NULL ||
        (config->dtc_count != 0U && config->dtcs == NULL) ||
        (config->dtc_detail_count != 0U && config->dtc_details == NULL) ||
        (config->wwh_dtc_format_identifier != UINT8_C(0x02) &&
         config->wwh_dtc_format_identifier != UINT8_C(0x04))) {
        return false;
    }

    memset(state, 0, sizeof(*state));
    if (!mblink_mercedes_vin_decode(config->vin, &state->decoded_vin))
        return false;

    profile = mblink_mercedes_profile_for_vin(config->vin);
    if (!mblink_mercedes_vehicle_profile_is_valid(profile))
        return false;

    if (config->endpoint_key != NULL) {
        endpoint = mblink_mercedes_profile_find_endpoint(
            profile, config->endpoint_key);
        if (endpoint == NULL || endpoint->module != config->module)
            return false;
    } else {
        endpoint = find_module_endpoint(profile, config->module);
    }
    if (endpoint == NULL) return false;

    memcpy(state->vin, state->decoded_vin.vin, sizeof(state->vin));
    state->profile = profile;
    state->endpoint = endpoint;
    state->module = config->module;
    state->dtc_store.records = config->dtcs;
    state->dtc_store.record_count = config->dtc_count;
    state->dtc_store.status_availability_mask = LINK_UDS_DTC_STATUS_MASK_ALL;
    state->dtc_store.severity_availability_mask = UINT8_C(0xff);
    state->dtc_store.dtc_format_identifier = UINT8_C(0x01);
    state->dtc_store.details = config->dtc_details;
    state->dtc_store.detail_count = config->dtc_detail_count;
    state->dtc_store.wwh_dtc_format_identifier =
        config->wwh_dtc_format_identifier;
    state->read_did = config->read_did;
    state->did_context = config->did_context;
    return true;
}

LinkUdsServerHandlerResult mblink_mercedes_server_read_did_handler(
    void *context,
    const LinkUdsServerRequest *request,
    uint8_t *response_data,
    size_t response_data_capacity)
{
    MblinkMercedesServerState *state =
        (MblinkMercedesServerState *)context;
    const MblinkMercedesDidDefinition *definition;
    uint16_t identifier;
    size_t data_length = 0U;

    if (state == NULL || request == NULL || request->pdu == NULL ||
        response_data == NULL) {
        return link_uds_server_handler_negative(LINK_UDS_NRC_GENERAL_REJECT);
    }
    if (request->service != LINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER ||
        request->pdu_length != 3U) {
        return link_uds_server_handler_negative(
            LINK_UDS_NRC_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT);
    }

    identifier = (uint16_t)(((uint16_t)request->pdu[1] << 8U) |
                            request->pdu[2]);
    if (identifier == MBLINK_MERCEDES_SERVER_VIN_DID) {
        if (response_data_capacity < MBLINK_MERCEDES_VIN_LENGTH + 2U) {
            return link_uds_server_handler_negative(
                LINK_UDS_NRC_RESPONSE_TOO_LONG);
        }
        response_data[0] = (uint8_t)(identifier >> 8U);
        response_data[1] = (uint8_t)identifier;
        memcpy(response_data + 2U, state->vin, MBLINK_MERCEDES_VIN_LENGTH);
        return link_uds_server_handler_positive(
            MBLINK_MERCEDES_VIN_LENGTH + 2U);
    }

    /*
     * MBLINK deliberately does not invent Mercedes DID values. A non-F190
     * DID must exist in the selected evidence-backed profile and the target
     * application must provide the live bytes.
     */
    definition = mblink_mercedes_profile_find_did(
        state->profile, state->module, identifier);
    if (definition == NULL || state->read_did == NULL)
        return link_uds_server_handler_negative(
            LINK_UDS_NRC_REQUEST_OUT_OF_RANGE);

    if (response_data_capacity < 2U)
        return link_uds_server_handler_negative(LINK_UDS_NRC_RESPONSE_TOO_LONG);

    if (!state->read_did(
            state->did_context, identifier, response_data + 2U,
            response_data_capacity - 2U, &data_length)) {
        return link_uds_server_handler_negative(
            LINK_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    if (data_length > response_data_capacity - 2U)
        return link_uds_server_handler_negative(LINK_UDS_NRC_RESPONSE_TOO_LONG);

    response_data[0] = (uint8_t)(identifier >> 8U);
    response_data[1] = (uint8_t)identifier;
    return link_uds_server_handler_positive(data_length + 2U);
}

LinkUdsServerHandlerResult mblink_mercedes_server_read_dtc_handler(
    void *context,
    const LinkUdsServerRequest *request,
    uint8_t *response_data,
    size_t response_data_capacity)
{
    MblinkMercedesServerState *state =
        (MblinkMercedesServerState *)context;

    if (state == NULL)
        return link_uds_server_handler_negative(LINK_UDS_NRC_GENERAL_REJECT);
    return link_uds_server_dtc_handler(
        &state->dtc_store, request, response_data, response_data_capacity);
}

bool mblink_mercedes_server_bind(
    LinkUdsServer *server,
    MblinkMercedesServerState *state)
{
    if (server == NULL || state == NULL) return false;
    return link_uds_server_set_handler(
               server, LINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
               mblink_mercedes_server_read_did_handler, state) &&
           link_uds_server_set_handler(
               server, LINK_UDS_SERVICE_READ_DTC_INFORMATION,
               mblink_mercedes_server_read_dtc_handler, state);
}
