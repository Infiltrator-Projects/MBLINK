// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes.c
 * @brief Mercedes-Benz manufacturer-definition validation and lookup.
 */
#include "mblink/mercedes.h"
#include "mblink/mercedes_om651_api.h"

#include "infiltratr/core.h"

#include <string.h>

static bool mercedes_text_valid(const char *text)
{
    return text != NULL && text[0] != '\0';
}

static bool mercedes_status_valid(MblinkMercedesDefinitionStatus status)
{
    return status == MBLINK_MERCEDES_DEFINITION_CANDIDATE ||
           status == MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED;
}

static bool mercedes_module_valid(MblinkMercedesModuleKind module)
{
    return module >= MBLINK_MERCEDES_MODULE_ENGINE &&
           module <= MBLINK_MERCEDES_MODULE_OTHER;
}

static bool mercedes_address_equal(const MblinkIsoTpAddress *left,
                                   const MblinkIsoTpAddress *right)
{
    return left != NULL && right != NULL &&
           left->tx_can_id == right->tx_can_id &&
           left->rx_can_id == right->rx_can_id &&
           left->tx_extended_id == right->tx_extended_id &&
           left->rx_extended_id == right->rx_extended_id &&
           left->addressing_mode == right->addressing_mode &&
           left->target_type == right->target_type &&
           left->tx_address_extension == right->tx_address_extension &&
           left->rx_address_extension == right->rx_address_extension;
}

const char *mblink_mercedes_definition_status_name(
    MblinkMercedesDefinitionStatus status)
{
    switch (status) {
    case MBLINK_MERCEDES_DEFINITION_CANDIDATE: return "candidate";
    case MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED:
        return "vehicle-verified";
    }
    return "unknown";
}

const char *mblink_mercedes_module_kind_name(MblinkMercedesModuleKind module)
{
    switch (module) {
    case MBLINK_MERCEDES_MODULE_ENGINE: return "engine";
    case MBLINK_MERCEDES_MODULE_TRANSMISSION: return "transmission";
    case MBLINK_MERCEDES_MODULE_ABS_ESP: return "abs-esp";
    case MBLINK_MERCEDES_MODULE_RESTRAINTS: return "restraints";
    case MBLINK_MERCEDES_MODULE_CLIMATE: return "climate";
    case MBLINK_MERCEDES_MODULE_INSTRUMENT_CLUSTER:
        return "instrument-cluster";
    case MBLINK_MERCEDES_MODULE_BODY: return "body";
    case MBLINK_MERCEDES_MODULE_OTHER: return "other";
    }
    return "unknown";
}

bool mblink_mercedes_did_definition_is_valid(
    const MblinkMercedesDidDefinition *definition)
{
    if (definition == NULL ||
        !mblink_uds_did_definition_is_valid(&definition->uds) ||
        !mercedes_module_valid(definition->module) ||
        !mercedes_status_valid(definition->status) ||
        !mercedes_text_valid(definition->provenance)) {
        return false;
    }
    return true;
}

bool mblink_mercedes_did_definition_is_verified(
    const MblinkMercedesDidDefinition *definition)
{
    return mblink_mercedes_did_definition_is_valid(definition) &&
           definition->status ==
               MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED;
}

bool mblink_mercedes_ecu_endpoint_is_valid(
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    return endpoint != NULL && mercedes_text_valid(endpoint->key) &&
           mercedes_text_valid(endpoint->name) &&
           mercedes_module_valid(endpoint->module) &&
           mblink_isotp_address_is_valid(&endpoint->address) &&
           endpoint->address.target_type == MBLINK_ISOTP_TARGET_PHYSICAL &&
           endpoint->address.tx_can_id != endpoint->address.rx_can_id &&
           mercedes_status_valid(endpoint->status) &&
           mercedes_text_valid(endpoint->provenance);
}

bool mblink_mercedes_ecu_endpoint_is_verified(
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    return mblink_mercedes_ecu_endpoint_is_valid(endpoint) &&
           endpoint->status ==
               MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED;
}

bool mblink_mercedes_vehicle_profile_is_valid(
    const MblinkMercedesVehicleProfile *profile)
{
    if (profile == NULL || !mercedes_text_valid(profile->chassis_code) ||
        !mercedes_text_valid(profile->engine_family) ||
        !mercedes_text_valid(profile->display_name) ||
        (profile->endpoint_count != 0U && profile->endpoints == NULL) ||
        (profile->definition_count != 0U && profile->definitions == NULL)) {
        return false;
    }

    for (size_t index = 0U; index < profile->endpoint_count; ++index) {
        const MblinkMercedesEcuEndpointDefinition *endpoint =
            &profile->endpoints[index];
        if (!mblink_mercedes_ecu_endpoint_is_valid(endpoint)) {
            return false;
        }
        for (size_t earlier = 0U; earlier < index; ++earlier) {
            const MblinkMercedesEcuEndpointDefinition *previous =
                &profile->endpoints[earlier];
            if (infiltratr_string_equal(previous->key, endpoint->key) ||
                mercedes_address_equal(&previous->address,
                                       &endpoint->address)) {
                return false;
            }
        }
    }

    for (size_t index = 0U; index < profile->definition_count; ++index) {
        const MblinkMercedesDidDefinition *definition =
            &profile->definitions[index];
        if (!mblink_mercedes_did_definition_is_valid(definition)) {
            return false;
        }
        for (size_t earlier = 0U; earlier < index; ++earlier) {
            const MblinkMercedesDidDefinition *previous =
                &profile->definitions[earlier];
            if (previous->module == definition->module &&
                previous->uds.identifier == definition->uds.identifier) {
                return false;
            }
        }
    }
    return true;
}

const MblinkMercedesEcuEndpointDefinition *
mblink_mercedes_profile_find_endpoint(
    const MblinkMercedesVehicleProfile *profile,
    const char *key)
{
    if (!mblink_mercedes_vehicle_profile_is_valid(profile) ||
        !mercedes_text_valid(key)) {
        return NULL;
    }

    for (size_t index = 0U; index < profile->endpoint_count; ++index) {
        const MblinkMercedesEcuEndpointDefinition *endpoint =
            &profile->endpoints[index];
        if (infiltratr_string_equal(endpoint->key, key)) {
            return endpoint;
        }
    }
    return NULL;
}

const MblinkMercedesDidDefinition *mblink_mercedes_profile_find_did(
    const MblinkMercedesVehicleProfile *profile,
    MblinkMercedesModuleKind module,
    uint16_t identifier)
{
    if (!mblink_mercedes_vehicle_profile_is_valid(profile) ||
        !mercedes_module_valid(module)) {
        return NULL;
    }

    for (size_t index = 0U; index < profile->definition_count; ++index) {
        const MblinkMercedesDidDefinition *definition =
            &profile->definitions[index];
        if (definition->module == module &&
            definition->uds.identifier == identifier) {
            return definition;
        }
    }
    return NULL;
}

MblinkUdsResult mblink_mercedes_decode_defined_did(
    const uint8_t *pdu,
    size_t pdu_length,
    const MblinkMercedesDidDefinition *definition,
    MblinkUdsDidValue *value)
{
    if (!mblink_mercedes_did_definition_is_valid(definition)) {
        return MBLINK_UDS_RESULT_INVALID_ARGUMENT;
    }
    return mblink_uds_decode_defined_did_response(
        pdu, pdu_length, &definition->uds, value);
}

size_t mblink_mercedes_om651_catalog_count(void)
{
    return mblink_mercedes_om651_signal_count();
}

const MblinkMercedesOm651SignalDefinition *
mblink_mercedes_om651_catalog_at(size_t index)
{
    return mblink_mercedes_om651_signal_at(index);
}

const MblinkMercedesVehicleProfile *mblink_mercedes_c207_om651_profile(void)
{
    static const MblinkMercedesEcuEndpointDefinition endpoints[] = {
        {
            .key = "c207-om651-engine-eobd-11bit",
            .name = "Delphi CRD3.x engine ECU candidate",
            .module = MBLINK_MERCEDES_MODULE_ENGINE,
            .address = {
                .tx_can_id = UINT32_C(0x7e0),
                .rx_can_id = UINT32_C(0x7e8),
                .tx_extended_id = false,
                .rx_extended_id = false,
                .addressing_mode = MBLINK_ISOTP_ADDRESSING_NORMAL,
                .target_type = MBLINK_ISOTP_TARGET_PHYSICAL,
                .tx_address_extension = 0U,
                .rx_address_extension = 0U
            },
            .status = MBLINK_MERCEDES_DEFINITION_CANDIDATE,
            .provenance =
                "autodiag2/database W207 E 250 2200 CDI (148/150 kW) -> Delphi CRD3.x fitment; 7E0/7E8 conventional EOBD physical endpoint remains pending C207 vehicle capture"
        }
    };
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "OM651",
        .display_name = "Mercedes-Benz C207 E 250 CDI / OM651 / Delphi CRD3.x",
        .endpoints = endpoints,
        .endpoint_count = INFILTRATR_ARRAY_LENGTH(endpoints),
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}
