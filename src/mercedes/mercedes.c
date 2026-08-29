// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes.c
 * @brief Mercedes-Benz manufacturer-definition validation and lookup.
 */
#include "mblink/mercedes.h"
#include "mblink/mercedes_did_lab.h"
#include "mblink/mercedes_om651_api.h"

#include "infiltratr/core.h"

#include <math.h>
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
            /* A key or physical route must identify exactly one ECU.  Keeping
             * ambiguity out of the catalogue prevents a valid response from
             * being attributed to whichever duplicate happened to be first. */
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
    /* This route is deliberately only a candidate.  Generic 7E0/7E8 traffic
     * may establish identity, but it cannot prove a Mercedes ECU variant. */
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
            .name = "Transmission control module (TCM / VGS) EOBD candidate",
            .module = MBLINK_MERCEDES_MODULE_TRANSMISSION,
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
                "ISO 15765-4 assigns/recommends 0x7E1/0x7E9 for the TCM, W212/C207 service material identifies the VGS/EGS transmission-control family, and a 2026-08-26 C207/OM651 capture proved this physical responder. The exact VGS/EGS identity remains a candidate until returned ECU identity confirms it."
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
        .engine_family = "M271",
        .display_name = "Mercedes-Benz C207 · M271 petrol family",
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
        .engine_family = "M274",
        .display_name = "Mercedes-Benz C207 · M274 petrol family",
        .endpoints = &mercedes_generic_engine_endpoint,
        .endpoint_count = 1U,
        .definitions = NULL,
        .definition_count = 0U
    };
    return &profile;
}

const MblinkMercedesVehicleProfile *mblink_mercedes_profile_for_vin(
    const char *vin)
{
    MblinkMercedesVinDecode decoded;
    const MblinkMercedesBaumusterDefinition *definition;

    if (!mblink_mercedes_vin_decode(vin, &decoded))
        return mblink_mercedes_generic_profile();

    definition = decoded.baumuster_definition;
    if (definition == NULL) {
        if (decoded.baumuster_available &&
            strcmp(decoded.series_number, "207") == 0) {
            return mblink_mercedes_c207_generic_profile();
        }
        return mblink_mercedes_generic_profile();
    }

    if (strcmp(definition->engine_family, "OM651") == 0)
        return mblink_mercedes_c207_om651_profile();
    if (strcmp(definition->engine_family, "M271") == 0)
        return mblink_mercedes_c207_m271_profile();
    if (strcmp(definition->engine_family, "M274") == 0)
        return mblink_mercedes_c207_m274_profile();
    if (strcmp(definition->chassis_family, "C207") == 0)
        return mblink_mercedes_c207_generic_profile();
    return mblink_mercedes_generic_profile();
}

bool mblink_mercedes_profile_is_crd3_candidate(
    const MblinkMercedesVehicleProfile *profile)
{
    return mblink_mercedes_vehicle_profile_is_valid(profile) &&
           strcmp(profile->engine_family, "OM651") == 0;
}


/* ------------------------------------------------------------------------- */
/* Offline Mercedes / Delphi DID research catalogue and correlation engine.   */
/* ------------------------------------------------------------------------- */

static const MblinkMercedesDidLabDefinition mercedes_did_lab[] = {
    {
        "mercedes.om651.electrical.battery_voltage", "Battery voltage",
        "Delphi CRD3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_SOURCE_BACKED_CANDIDATE,
        true, UINT16_C(0x2007), 2U,
        MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0078125, 0.0, "V",
        "obd2.electrical.control_module_voltage",
        "CaesarSuite CRD3::DT_2007_IN_Battery_voltage documents UDS DID 0x2007, two-byte big-endian data, factor 0.0078125 and offset 0 V; vehicle verification pending.",
        "https://github.com/jglim/CaesarSuite/discussions/1"
    },
    {
        "mercedes.om651.engine.speed", "Engine speed",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "rpm", "obd2.engine.rpm",
        "Independent OM651 CDID3 actual-value catalogues expose engine speed. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.vehicle.speed", "Vehicle speed",
        "Mercedes diagnostic network", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "km/h", "obd2.vehicle.speed",
        "Mercedes diagnostic records expose vehicle-speed actual values; exact CRD3 source DID remains unmapped.",
        "Mercedes diagnostic actual-value records"
    },
    {
        "mercedes.om651.fuel.tank_level", "Fuel tank level",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "L", "obd2.fuel.tank_level",
        "Independent OM651 CDID3 actual-value catalogues expose fuel tank level in litres. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.fuel.injection_quantity", "Injection quantity",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "mg/stroke", NULL,
        "Independent OM651 CDID3 actual-value catalogues expose injection quantity in mg/stroke; priority candidate for factory fuel-consumption reconstruction.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.fuel.rail_pressure", "Fuel rail pressure",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "bar", "obd2.diesel.rail_pressure",
        "Independent OM651 CDID3 actual-value catalogues expose fuel rail pressure. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.driver.accelerator_pedal.sensor1",
        "Accelerator pedal position sensor 1",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "%", "obd2.driver.accelerator_pedal_d",
        "Independent OM651 CDID3 actual-value catalogues expose accelerator-pedal position. Redundant pedal channels remain separate discovery targets.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.driver.accelerator_pedal.sensor2",
        "Accelerator pedal position sensor 2",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "%", "obd2.driver.accelerator_pedal_e",
        "The OM651 catalogue retains the second pedal channel as a separate discovery target.",
        "MBLINK OM651 target catalogue"
    },
    {
        "mercedes.om651.air.throttle_valve", "Throttle valve",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "%", "obd2.engine.throttle",
        "Factory throttle-valve position remains separate from accelerator-pedal demand. DID and encoding remain unmapped.",
        "MBLINK OM651 target catalogue plus C207 field correlation"
    },
    {
        "mercedes.om651.engine.coolant_temperature", "Coolant temperature",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_SIGNED_BIG_ENDIAN,
        0.0, 0.0, "°C", "obd2.engine.coolant",
        "Independent OM651 CDID3 actual-value catalogues expose coolant temperature. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.engine.oil_temperature", "Oil temperature",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_SIGNED_BIG_ENDIAN,
        0.0, 0.0, "°C", "obd2.engine.oil_temperature",
        "Independent OM651 CDID3 actual-value catalogues expose engine-oil temperature. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.engine.intake_air_temperature",
        "Intake air temperature",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_SIGNED_BIG_ENDIAN,
        0.0, 0.0, "°C", "obd2.engine.intake_air",
        "Independent OM651 CDID3 actual-value catalogues expose intake-air temperature. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    },
    {
        "mercedes.om651.environment.ambient_temperature",
        "Ambient temperature",
        "Mercedes vehicle network / OM651 context", MBLINK_MERCEDES_MODULE_OTHER,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_SIGNED_BIG_ENDIAN,
        0.0, 0.0, "°C", "obd2.environment.ambient_air",
        "Mercedes exposes finer ambient-temperature presentation than SAE PID 0x46 on the development vehicle. Exact factory source remains unmapped.",
        "Vehicle display observation plus MBLINK field evidence"
    },
    {
        "mercedes.om651.environment.barometric_pressure",
        "Barometric pressure",
        "Delphi CDID3 / OM651", MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY,
        false, 0U, 0U, MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN,
        0.0, 0.0, "bar", "obd2.engine.barometric_pressure",
        "Independent OM651 CDID3 actual-value catalogues expose barometric pressure. DID and encoding remain unmapped.",
        "ScanDoc OM651 CDID3 actual-value catalogue"
    }
};

static bool mercedes_did_lab_definition_valid(
    const MblinkMercedesDidLabDefinition *definition)
{
    if (definition == NULL || !mercedes_text_valid(definition->stable_key) ||
        !mercedes_text_valid(definition->name) ||
        !mercedes_text_valid(definition->ecu_family) ||
        !mercedes_module_valid(definition->module) ||
        !mercedes_text_valid(definition->unit) ||
        !mercedes_text_valid(definition->provenance) ||
        !mercedes_text_valid(definition->source_locator)) return false;
    if (!definition->identifier_known) return definition->raw_length == 0U;
    return definition->raw_length >= 1U && definition->raw_length <= 4U &&
           definition->factor != 0.0;
}

const char *mblink_mercedes_did_lab_status_name(MblinkMercedesDidLabStatus status)
{
    switch (status) {
    case MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY:
        return "corroborated-unmapped";
    case MBLINK_MERCEDES_DID_LAB_SOURCE_BACKED_CANDIDATE:
        return "source-backed-candidate";
    case MBLINK_MERCEDES_DID_LAB_VEHICLE_VERIFIED:
        return "vehicle-verified";
    }
    return "unknown";
}

const char *mblink_mercedes_did_lab_decode_result_name(
    MblinkMercedesDidLabDecodeResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_DID_LAB_DECODE_OK: return "ok";
    case MBLINK_MERCEDES_DID_LAB_DECODE_INVALID_ARGUMENT:
        return "invalid-argument";
    case MBLINK_MERCEDES_DID_LAB_DECODE_UNMAPPED: return "unmapped";
    case MBLINK_MERCEDES_DID_LAB_DECODE_MALFORMED: return "malformed";
    case MBLINK_MERCEDES_DID_LAB_DECODE_UNEXPECTED_RESPONSE:
        return "unexpected-response";
    }
    return "unknown";
}

size_t mblink_mercedes_did_lab_count(void)
{
    return sizeof(mercedes_did_lab) / sizeof(mercedes_did_lab[0]);
}

const MblinkMercedesDidLabDefinition *mblink_mercedes_did_lab_at(size_t index)
{
    return index < mblink_mercedes_did_lab_count()
        ? &mercedes_did_lab[index] : NULL;
}

const MblinkMercedesDidLabDefinition *mblink_mercedes_did_lab_find_key(
    const char *stable_key)
{
    size_t index;
    if (!mercedes_text_valid(stable_key)) return NULL;
    for (index = 0U; index < mblink_mercedes_did_lab_count(); ++index) {
        if (strcmp(mercedes_did_lab[index].stable_key, stable_key) == 0)
            return &mercedes_did_lab[index];
    }
    return NULL;
}

const MblinkMercedesDidLabDefinition *mblink_mercedes_did_lab_find_identifier(
    uint16_t identifier)
{
    size_t index;
    for (index = 0U; index < mblink_mercedes_did_lab_count(); ++index) {
        if (mercedes_did_lab[index].identifier_known &&
            mercedes_did_lab[index].identifier == identifier)
            return &mercedes_did_lab[index];
    }
    return NULL;
}

bool mblink_mercedes_did_lab_can_auto_poll(
    const MblinkMercedesDidLabDefinition *definition)
{
    return mercedes_did_lab_definition_valid(definition) &&
           definition->identifier_known &&
           definition->status == MBLINK_MERCEDES_DID_LAB_VEHICLE_VERIFIED;
}

static uint32_t mercedes_did_lab_read_unsigned(
    const uint8_t *data, size_t length, bool little_endian)
{
    uint32_t raw = 0U;
    size_t index;
    if (little_endian) {
        for (index = 0U; index < length; ++index)
            raw |= ((uint32_t)data[index]) << (8U * index);
    } else {
        for (index = 0U; index < length; ++index)
            raw = (raw << 8U) | data[index];
    }
    return raw;
}

MblinkMercedesDidLabDecodeResult mblink_mercedes_did_lab_decode_response(
    const MblinkMercedesDidLabDefinition *definition,
    const uint8_t *pdu, size_t pdu_length, double *value)
{
    bool little_endian, signed_value;
    uint32_t raw;
    if (!mercedes_did_lab_definition_valid(definition) ||
        pdu == NULL || value == NULL)
        return MBLINK_MERCEDES_DID_LAB_DECODE_INVALID_ARGUMENT;
    if (!definition->identifier_known)
        return MBLINK_MERCEDES_DID_LAB_DECODE_UNMAPPED;
    if (pdu_length < 3U + definition->raw_length)
        return MBLINK_MERCEDES_DID_LAB_DECODE_MALFORMED;
    if (pdu[0] != 0x62U ||
        pdu[1] != (uint8_t)(definition->identifier >> 8U) ||
        pdu[2] != (uint8_t)(definition->identifier & 0xffU))
        return MBLINK_MERCEDES_DID_LAB_DECODE_UNEXPECTED_RESPONSE;

    little_endian =
        definition->encoding == MBLINK_MERCEDES_DID_LAB_UNSIGNED_LITTLE_ENDIAN ||
        definition->encoding == MBLINK_MERCEDES_DID_LAB_SIGNED_LITTLE_ENDIAN;
    signed_value =
        definition->encoding == MBLINK_MERCEDES_DID_LAB_SIGNED_BIG_ENDIAN ||
        definition->encoding == MBLINK_MERCEDES_DID_LAB_SIGNED_LITTLE_ENDIAN;
    raw = mercedes_did_lab_read_unsigned(
        &pdu[3], definition->raw_length, little_endian);
    if (signed_value) {
        const unsigned int bits = (unsigned int)(definition->raw_length * 8U);
        int64_t signed_raw;
        if ((raw & (UINT32_C(1) << (bits - 1U))) != 0U) {
            const uint64_t full = UINT64_C(1) << bits;
            signed_raw = (int64_t)((uint64_t)raw - full);
        } else signed_raw = (int64_t)raw;
        *value = (double)signed_raw * definition->factor + definition->offset;
    } else {
        *value = (double)raw * definition->factor + definition->offset;
    }
    return MBLINK_MERCEDES_DID_LAB_DECODE_OK;
}

static bool signal_series_valid(const MblinkSignalPoint *points, size_t count)
{
    size_t index;
    if (points == NULL || count < 2U) return false;
    for (index = 0U; index < count; ++index) {
        if (!isfinite(points[index].value)) return false;
        if (index != 0U &&
            points[index].timestamp_ms < points[index - 1U].timestamp_ms)
            return false;
    }
    return true;
}

static uint64_t signal_time_distance(uint64_t left, uint64_t right)
{
    return left >= right ? left - right : right - left;
}

static bool signal_target_time(uint64_t candidate_time, int64_t lag_ms,
                               uint64_t *target)
{
    if (target == NULL) return false;
    if (lag_ms >= 0) {
        const uint64_t lag = (uint64_t)lag_ms;
        if (candidate_time < lag) return false;
        *target = candidate_time - lag;
        return true;
    }
    {
        const uint64_t lag = (uint64_t)(-(lag_ms + 1)) + UINT64_C(1);
        if (candidate_time > UINT64_MAX - lag) return false;
        *target = candidate_time + lag;
        return true;
    }
}

static bool signal_correlation_for_lag(
    const MblinkSignalPoint *reference, size_t reference_count,
    const MblinkSignalPoint *candidate, size_t candidate_count,
    int64_t lag_ms, uint64_t pair_tolerance_ms,
    MblinkSignalCorrelationResult *result)
{
    size_t ci, ri = 0U, pairs = 0U;
    double sx = 0.0, sy = 0.0, sxx = 0.0, syy = 0.0, sxy = 0.0;
    double min_y = 0.0, max_y = 0.0;
    double n, var_x, var_y, covariance, denominator, slope, intercept, sse;
    MblinkSignalCorrelationResult computed;

    if (result == NULL) return false;
    for (ci = 0U; ci < candidate_count; ++ci) {
        uint64_t target, distance;
        size_t chosen;
        double x, y;
        if (!signal_target_time(candidate[ci].timestamp_ms, lag_ms, &target))
            continue;
        while (ri + 1U < reference_count &&
               reference[ri + 1U].timestamp_ms <= target) ++ri;
        chosen = ri;
        distance = signal_time_distance(reference[chosen].timestamp_ms, target);
        if (chosen + 1U < reference_count) {
            const uint64_t next_distance =
                signal_time_distance(reference[chosen + 1U].timestamp_ms, target);
            if (next_distance < distance) {
                ++chosen; distance = next_distance;
            }
        }
        if (distance > pair_tolerance_ms) continue;
        x = reference[chosen].value; y = candidate[ci].value;
        if (pairs == 0U) min_y = max_y = y;
        else { if (y < min_y) min_y = y; if (y > max_y) max_y = y; }
        ++pairs;
        sx += x; sy += y; sxx += x * x; syy += y * y; sxy += x * y;
    }
    if (pairs < 3U) return false;
    n = (double)pairs;
    var_x = n * sxx - sx * sx;
    var_y = n * syy - sy * sy;
    covariance = n * sxy - sx * sy;
    if (var_x <= 1.0e-12 || var_y <= 1.0e-12) return false;
    denominator = sqrt(var_x * var_y);
    if (denominator <= 1.0e-12) return false;
    computed.pearson_r = covariance / denominator;
    if (computed.pearson_r > 1.0) computed.pearson_r = 1.0;
    if (computed.pearson_r < -1.0) computed.pearson_r = -1.0;
    slope = covariance / var_x;
    intercept = (sy - slope * sx) / n;
    sse = syy + slope * slope * sxx + n * intercept * intercept -
          2.0 * slope * sxy - 2.0 * intercept * sy +
          2.0 * slope * intercept * sx;
    if (sse < 0.0 && sse > -1.0e-8) sse = 0.0;
    if (sse < 0.0) return false;

    computed.pair_count = pairs;
    computed.lag_ms = lag_ms;
    computed.slope = slope;
    computed.intercept = intercept;
    computed.rmse = sqrt(sse / n);
    {
        const double range_y = max_y - min_y;
        const double coverage = pairs >= 20U ? 1.0 : (double)pairs / 20.0;
        double fit_penalty;
        computed.normalized_rmse =
            range_y > 1.0e-12 ? computed.rmse / range_y : computed.rmse;
        fit_penalty = 1.0 - computed.normalized_rmse;
        if (fit_penalty < 0.0) fit_penalty = 0.0;
        if (fit_penalty > 1.0) fit_penalty = 1.0;
        computed.score = fabs(computed.pearson_r) * coverage * fit_penalty;
    }
    *result = computed;
    return true;
}

bool mblink_signal_correlation_best_linear(
    const MblinkSignalPoint *reference, size_t reference_count,
    const MblinkSignalPoint *candidate, size_t candidate_count,
    uint64_t max_lag_ms, uint64_t lag_step_ms,
    uint64_t pair_tolerance_ms, MblinkSignalCorrelationResult *result)
{
    int64_t lag, max_lag;
    bool found = false;
    MblinkSignalCorrelationResult best;
    if (result == NULL ||
        !signal_series_valid(reference, reference_count) ||
        !signal_series_valid(candidate, candidate_count) ||
        lag_step_ms == 0U || max_lag_ms > (uint64_t)INT64_MAX ||
        lag_step_ms > (uint64_t)INT64_MAX) return false;

    max_lag = (int64_t)max_lag_ms;
    lag = -max_lag;
    for (;;) {
        MblinkSignalCorrelationResult current;
        if (signal_correlation_for_lag(
                reference, reference_count, candidate, candidate_count,
                lag, pair_tolerance_ms, &current)) {
            const uint64_t ca = current.lag_ms >= 0
                ? (uint64_t)current.lag_ms
                : (uint64_t)(-(current.lag_ms + 1)) + UINT64_C(1);
            uint64_t ba = 0U;
            if (found) ba = best.lag_ms >= 0
                ? (uint64_t)best.lag_ms
                : (uint64_t)(-(best.lag_ms + 1)) + UINT64_C(1);
            if (!found || current.score > best.score + 1.0e-12 ||
                (fabs(current.score - best.score) <= 1.0e-12 &&
                 (current.pair_count > best.pair_count ||
                  (current.pair_count == best.pair_count && ca < ba)))) {
                best = current; found = true;
            }
        }
        if (lag >= max_lag) break;
        if ((uint64_t)(max_lag - lag) <= lag_step_ms) lag = max_lag;
        else lag += (int64_t)lag_step_ms;
    }
    if (!found) return false;
    *result = best;
    return true;
}

const char *mblink_signal_correlation_strength(
    const MblinkSignalCorrelationResult *result)
{
    double correlation;
    if (result == NULL || result->pair_count < 3U) return "insufficient";
    correlation = fabs(result->pearson_r);
    if (result->pair_count >= 20U && correlation >= 0.995 &&
        result->normalized_rmse <= 0.05) return "very-strong";
    if (result->pair_count >= 10U && correlation >= 0.98 &&
        result->normalized_rmse <= 0.10) return "strong";
    if (correlation >= 0.90) return "moderate";
    return "weak";
}
