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

typedef enum {
    MBLINK_MERCEDES_DTC_EVIDENCE_COMMUNITY_OBSERVATION = 0,
    MBLINK_MERCEDES_DTC_EVIDENCE_SPECIALIST_CORROBORATED,
    MBLINK_MERCEDES_DTC_EVIDENCE_PRIMARY_DOCUMENTED,
    MBLINK_MERCEDES_DTC_EVIDENCE_REPAIR_VERIFIED
} MblinkMercedesDtcEvidenceTier;

typedef struct {
    const char *label;
    const char *reference;
    MblinkMercedesDtcEvidenceTier tier;
    /** False when the source proves occurrence/applicability but not meaning. */
    bool supports_meaning;
} MblinkMercedesDtcEvidenceSource;

typedef struct {
    const char *protocol;
    const char *module_family;
    const char *ecu_family;
    const char *vehicle_family;
    const char *engine_family;
} MblinkMercedesDtcApplicability;

typedef struct {
    const char *label;
    const char *url;
    /** Lookup starting points never define a proprietary code by themselves. */
    bool authoritative_for_meaning;
} MblinkMercedesDtcLookupReference;

/**
 * A source-scoped five-character Mercedes reference DTC. Unlike the
 * module-scoped KWP/UDS definitions below, this catalogue deliberately retains
 * duplicate and conflicting meanings because the supplied lists often omit the
 * ECU family or vehicle generation needed to disambiguate them.
 */
typedef struct {
    const char *code;
    /** Optional Mercedes subcode such as "01", "02" or "004". */
    const char *subcode;
    const char *description;
    const char *area;
    const char *applicability;
    const char *source_label;
    const char *source_reference;
    MblinkMercedesDtcEvidenceTier evidence_tier;
} MblinkMercedesReferenceDtcDefinition;

#define MBLINK_MERCEDES_REFERENCE_DTC_CODE_LENGTH 6U
#define MBLINK_MERCEDES_REFERENCE_DTC_TITLE_LENGTH 192U
#define MBLINK_MERCEDES_REFERENCE_DTC_AREA_LENGTH 48U
#define MBLINK_MERCEDES_REFERENCE_DTC_SOURCE_LENGTH 112U
#define MBLINK_MERCEDES_REFERENCE_DTC_APPLICABILITY_LENGTH 192U

typedef struct {
    bool ambiguous;
    size_t match_count;
    char code[MBLINK_MERCEDES_REFERENCE_DTC_CODE_LENGTH];
    char title[MBLINK_MERCEDES_REFERENCE_DTC_TITLE_LENGTH];
    char area[MBLINK_MERCEDES_REFERENCE_DTC_AREA_LENGTH];
    char source[MBLINK_MERCEDES_REFERENCE_DTC_SOURCE_LENGTH];
    char applicability[MBLINK_MERCEDES_REFERENCE_DTC_APPLICABILITY_LENGTH];
} MblinkMercedesReferenceDtcKnowledge;

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
    MblinkMercedesDtcApplicability applicability_details;
    const MblinkMercedesDtcEvidenceSource *sources;
    size_t source_count;
} MblinkMercedesKwpDtcDefinition;

#define MBLINK_MERCEDES_DTC_TEXT_LENGTH 320U

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

const char *mblink_mercedes_dtc_evidence_tier_name(
    MblinkMercedesDtcEvidenceTier tier);
size_t mblink_mercedes_dtc_lookup_reference_count(void);
const MblinkMercedesDtcLookupReference *
mblink_mercedes_dtc_lookup_reference_at(size_t index);

/** Source-scoped Mercedes reference catalogue supplied to the project. */
size_t mblink_mercedes_reference_dtc_count(void);
const MblinkMercedesReferenceDtcDefinition *
mblink_mercedes_reference_dtc_at(size_t index);

/**
 * Count/find exact reference rows. With subcode == NULL or empty, only base
 * rows without a subcode match; subcoded variants are never silently applied.
 */
size_t mblink_mercedes_reference_dtc_match_count(
    const char *code,
    const char *subcode);
const MblinkMercedesReferenceDtcDefinition *
mblink_mercedes_reference_dtc_match_at(
    const char *code,
    const char *subcode,
    size_t match_index);

/**
 * Resolve a base Mercedes reference code. Returns false when the catalogue has
 * no base rows. The ambiguous flag is true when the supplied sources retain
 * more than one distinct textual meaning and module/subcode context is needed.
 */
bool mblink_mercedes_reference_dtc_resolve(
    const char *code,
    MblinkMercedesReferenceDtcKnowledge *knowledge);

bool mblink_mercedes_kwp_dtc_format(
    const char *module_key,
    uint16_t code,
    uint8_t raw_status,
    char *buffer,
    size_t capacity);

bool mblink_mercedes_uds_dtc_format(
    const char *module_key,
    uint32_t code,
    uint8_t raw_status,
    char *buffer,
    size_t capacity);

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
