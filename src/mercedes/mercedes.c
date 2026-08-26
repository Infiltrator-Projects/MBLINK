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
           status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED ||
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
    case MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED:
        return "source-corroborated";
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

static const MblinkMercedesEcuEndpointDefinition mercedes_generic_engine_endpoint = {
    .key = "mercedes-primary-engine-eobd-11bit",
    .name = "Primary engine ECU",
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
        "Generic read-only Mercedes/SAE physical engine-ECU discovery candidate. It is used only to acquire VIN, standard ECU identity and fault evidence before a manufacturer/engine family is selected."
};

const MblinkMercedesEcuEndpointDefinition *
mblink_mercedes_generic_engine_endpoint(void)
{
    return &mercedes_generic_engine_endpoint;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_generic_profile(void)
{
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "Mercedes-Benz",
        .engine_family = "Unidentified",
        .display_name = "Mercedes-Benz automatic vehicle identification",
        .endpoints = &mercedes_generic_engine_endpoint,
        .endpoint_count = 1U,
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_c207_generic_profile(void)
{
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "Unidentified",
        .display_name = "Mercedes-Benz C207 · engine family not yet identified",
        .endpoints = &mercedes_generic_engine_endpoint,
        .endpoint_count = 1U,
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_c207_om651_profile(void)
{
    static const MblinkMercedesEcuEndpointDefinition endpoints[] = {
        {
            .key = "c207-om651-engine-eobd-11bit",
            .name = "OM651 engine ECU · Delphi CRD3.x family candidate",
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
            .status = MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            .provenance =
                "C207 OM651 family sources corroborate Delphi CRD3.x and 0x7E0/0x7E8. A 2026-08-26 vehicle capture verified this endpoint on one C207/OM651 member; that single capture is evidence for the family but is not promoted as proof for every C207 OM651 variant."
        },
        {
            .key = "c207-om651-secondary-eobd-11bit",
            .name = "Secondary EOBD powertrain ECU",
            .module = MBLINK_MERCEDES_MODULE_OTHER,
            .address = {
                .tx_can_id = UINT32_C(0x7e1),
                .rx_can_id = UINT32_C(0x7e9),
                .tx_extended_id = false,
                .rx_extended_id = false,
                .addressing_mode = MBLINK_ISOTP_ADDRESSING_NORMAL,
                .target_type = MBLINK_ISOTP_TARGET_PHYSICAL,
                .tx_address_extension = 0U,
                .rx_address_extension = 0U
            },
            .status = MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            .provenance =
                "A 2026-08-26 C207/OM651 vehicle capture proved a UDS responder at 0x7E1/0x7E9. Its exact module identity is deliberately left unresolved and the evidence is not generalized to every C207."
        }
    };
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "OM651",
        .display_name = "Mercedes-Benz C207 · OM651 diesel family",
        .endpoints = endpoints,
        .endpoint_count = INFILTRATR_ARRAY_LENGTH(endpoints),
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_c207_m271_profile(void)
{
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "M271.860",
        .display_name = "Mercedes-Benz C207 · M271.860 petrol family",
        .endpoints = &mercedes_generic_engine_endpoint,
        .endpoint_count = 1U,
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_c207_m274_profile(void)
{
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "M274.920",
        .display_name = "Mercedes-Benz C207 · M274.920 petrol family",
        .endpoints = &mercedes_generic_engine_endpoint,
        .endpoint_count = 1U,
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}

static bool mercedes_vin_type_is(const char *vin, const char *type)
{
    return vin != NULL && type != NULL && strlen(vin) == 17U &&
           strlen(type) == 6U && memcmp(vin + 3U, type, 6U) == 0;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_profile_for_vin(
    const char *vin)
{
    static const char *const om651_types[] = {
        "207301", "207302", "207303", "207304"
    };
    static const char *const m271_types[] = {
        "207347", "207348"
    };
    static const char *const m274_types[] = {
        "207334", "207336"
    };
    size_t index;

    if (vin == NULL || strlen(vin) != 17U) {
        return mblink_mercedes_generic_profile();
    }
    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(om651_types); ++index) {
        if (mercedes_vin_type_is(vin, om651_types[index]))
            return mblink_mercedes_c207_om651_profile();
    }
    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(m271_types); ++index) {
        if (mercedes_vin_type_is(vin, m271_types[index]))
            return mblink_mercedes_c207_m271_profile();
    }
    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(m274_types); ++index) {
        if (mercedes_vin_type_is(vin, m274_types[index]))
            return mblink_mercedes_c207_m274_profile();
    }
    if (memcmp(vin + 3U, "207", 3U) == 0)
        return mblink_mercedes_c207_generic_profile();
    return mblink_mercedes_generic_profile();
}

bool mblink_mercedes_profile_is_crd3_candidate(
    const MblinkMercedesVehicleProfile *profile)
{
    return mblink_mercedes_vehicle_profile_is_valid(profile) &&
           strcmp(profile->engine_family, "OM651") == 0;
}
