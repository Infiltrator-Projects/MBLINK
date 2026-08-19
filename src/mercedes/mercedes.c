// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes.c
 * @brief Mercedes-Benz manufacturer-definition validation and lookup.
 */
#include "mblink/mercedes.h"

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

bool mblink_mercedes_vehicle_profile_is_valid(
    const MblinkMercedesVehicleProfile *profile)
{
    if (profile == NULL || !mercedes_text_valid(profile->chassis_code) ||
        !mercedes_text_valid(profile->engine_family) ||
        !mercedes_text_valid(profile->display_name) ||
        (profile->definition_count != 0U && profile->definitions == NULL)) {
        return false;
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

const MblinkMercedesVehicleProfile *mblink_mercedes_c207_om651_profile(void)
{
    static const MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "OM651",
        .display_name = "Mercedes-Benz C207 / OM651",
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}
