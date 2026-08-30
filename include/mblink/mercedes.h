// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes.h
 * @brief Mercedes-Benz manufacturer-definition and validation contracts.
 */
#ifndef MBLINK_MERCEDES_H
#define MBLINK_MERCEDES_H

#include "mblink/isotp.h"
#include "mblink/mercedes_vin.h"
#include "mblink/uds.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /** Plausible definition with insufficient independent protocol evidence. */
    MBLINK_MERCEDES_DEFINITION_CANDIDATE = 0,
    /** Independent sources corroborate the definition; vehicle capture pending. */
    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
    /** Exact request/response behaviour is backed by a vehicle regression fixture. */
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

/**
 * A manufacturer fault definition whose numeric code is meaningful only in
 * the named Mercedes module/protocol namespace.  The raw ECU status byte is
 * deliberately not part of this table: it remains live evidence and must not
 * be replaced by a guessed textual interpretation.
 */
typedef struct {
    const char *module_key;
    uint16_t code;
    const char *description;
    /** Human-readable subsystem/component scope inside the named module. */
    const char *subsystem;
    /**
     * Explicit applicability boundary.  This is deliberately separate from
     * provenance: a source can corroborate a meaning without proving that the
     * definition applies outside the named ECU/protocol family.
     */
    const char *applicability;
    MblinkMercedesDefinitionStatus status;
    const char *provenance;
} MblinkMercedesKwpDtcDefinition;

typedef struct {
    const char *key;
    const char *name;
    MblinkMercedesModuleKind module;
    MblinkIsoTpAddress address;
    MblinkMercedesDefinitionStatus status;
    const char *provenance;
} MblinkMercedesEcuEndpointDefinition;

typedef struct {
    const char *chassis_code;
    const char *engine_family;
    const char *display_name;
    const MblinkMercedesEcuEndpointDefinition *endpoints;
    size_t endpoint_count;
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

bool mblink_mercedes_ecu_endpoint_is_valid(
    const MblinkMercedesEcuEndpointDefinition *endpoint);

bool mblink_mercedes_ecu_endpoint_is_verified(
    const MblinkMercedesEcuEndpointDefinition *endpoint);

bool mblink_mercedes_vehicle_profile_is_valid(
    const MblinkMercedesVehicleProfile *profile);

const MblinkMercedesEcuEndpointDefinition *
mblink_mercedes_profile_find_endpoint(
    const MblinkMercedesVehicleProfile *profile,
    const char *key);

const MblinkMercedesDidDefinition *mblink_mercedes_profile_find_did(
    const MblinkMercedesVehicleProfile *profile,
    MblinkMercedesModuleKind module,
    uint16_t identifier);

bool mblink_mercedes_kwp_dtc_definition_is_valid(
    const MblinkMercedesKwpDtcDefinition *definition);

/** Find a Mercedes KWP2000 fault by its module-scoped raw identifier. */
const MblinkMercedesKwpDtcDefinition *mblink_mercedes_kwp_dtc_find(
    const char *module_key,
    uint16_t code);

size_t mblink_mercedes_kwp_dtc_count(void);
const MblinkMercedesKwpDtcDefinition *mblink_mercedes_kwp_dtc_at(
    size_t index);

MblinkUdsResult mblink_mercedes_decode_defined_did(
    const uint8_t *pdu,
    size_t pdu_length,
    const MblinkMercedesDidDefinition *definition,
    MblinkUdsDidValue *value);

/**
 * Mercedes diagnostic profiles are selected from decoded VIN/Baumuster and
 * live ECU evidence. Exact model/engine suffixes belong to the VIN catalogue;
 * these profiles describe only diagnostic families and evidence status.
 */
const MblinkMercedesEcuEndpointDefinition *
mblink_mercedes_generic_engine_endpoint(void);

const MblinkMercedesVehicleProfile *mblink_mercedes_generic_profile(void);
const MblinkMercedesVehicleProfile *mblink_mercedes_c207_generic_profile(void);
const MblinkMercedesVehicleProfile *mblink_mercedes_c207_om651_profile(void);
const MblinkMercedesVehicleProfile *mblink_mercedes_c207_m271_profile(void);
const MblinkMercedesVehicleProfile *mblink_mercedes_c207_m274_profile(void);

/**
 * Select the narrowest profile justified by the VIN type code.
 * Unknown/unmapped VINs intentionally fall back to a generic Mercedes profile.
 */
const MblinkMercedesVehicleProfile *mblink_mercedes_profile_for_vin(
    const char *vin);

bool mblink_mercedes_profile_is_crd3_candidate(
    const MblinkMercedesVehicleProfile *profile);

#ifdef __cplusplus
}
#endif

#endif
