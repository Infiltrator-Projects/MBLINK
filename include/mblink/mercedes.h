// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes.h
 * @brief Mercedes-Benz manufacturer-definition and validation contracts.
 */
#ifndef MBLINK_MERCEDES_H
#define MBLINK_MERCEDES_H

#include "mblink/uds.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_DEFINITION_CANDIDATE = 0,
    MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED
} MblinkMercedesDefinitionStatus;

typedef enum {
    MBLINK_MERCEDES_MODULE_ENGINE = 0,
    MBLINK_MERCEDES_MODULE_TRANSMISSION,
    MBLINK_MERCEDES_MODULE_ABS_ESP,
    MBLINK_MERCEDES_MODULE_RESTRAINTS,
    MBLINK_MERCEDES_MODULE_CLIMATE,
    MBLINK_MERCEDES_MODULE_INSTRUMENT_CLUSTER,
    MBLINK_MERCEDES_MODULE_BODY,
    MBLINK_MERCEDES_MODULE_OTHER
} MblinkMercedesModuleKind;

typedef struct {
    MblinkUdsDidDefinition uds;
    MblinkMercedesModuleKind module;
    MblinkMercedesDefinitionStatus status;
    const char *provenance;
} MblinkMercedesDidDefinition;

typedef struct {
    const char *chassis_code;
    const char *engine_family;
    const char *display_name;
    const MblinkMercedesDidDefinition *definitions;
    size_t definition_count;
} MblinkMercedesVehicleProfile;

const char *mblink_mercedes_definition_status_name(
    MblinkMercedesDefinitionStatus status);

const char *mblink_mercedes_module_kind_name(MblinkMercedesModuleKind module);

bool mblink_mercedes_did_definition_is_valid(
    const MblinkMercedesDidDefinition *definition);

bool mblink_mercedes_did_definition_is_verified(
    const MblinkMercedesDidDefinition *definition);

bool mblink_mercedes_vehicle_profile_is_valid(
    const MblinkMercedesVehicleProfile *profile);

const MblinkMercedesDidDefinition *mblink_mercedes_profile_find_did(
    const MblinkMercedesVehicleProfile *profile,
    MblinkMercedesModuleKind module,
    uint16_t identifier);

MblinkUdsResult mblink_mercedes_decode_defined_did(
    const uint8_t *pdu,
    size_t pdu_length,
    const MblinkMercedesDidDefinition *definition,
    MblinkUdsDidValue *value);

/**
 * Return the C207 / OM651 development profile.
 *
 * The profile starts with no manufacturer DIDs: definitions are added only
 * after provenance is recorded and vehicle responses become regression fixtures.
 */
const MblinkMercedesVehicleProfile *mblink_mercedes_c207_om651_profile(void);

#ifdef __cplusplus
}
#endif

#endif
