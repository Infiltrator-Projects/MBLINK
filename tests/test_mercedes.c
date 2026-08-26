// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static MblinkMercedesDidDefinition make_definition(
    uint16_t identifier,
    MblinkMercedesModuleKind module,
    MblinkMercedesDefinitionStatus status,
    const char *provenance)
{
    MblinkMercedesDidDefinition definition = {
        .uds = {
            .identifier = identifier,
            .key = "test-did",
            .name = "Test DID",
            .minimum_length = 2U,
            .maximum_length = 4U
        },
        .module = module,
        .status = status,
        .provenance = provenance
    };
    return definition;
}

static int test_development_profile(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();

    CHECK(profile != NULL);
    CHECK(strcmp(profile->chassis_code, "C207") == 0);
    CHECK(strcmp(profile->engine_family, "OM651") == 0);
    CHECK(strcmp(profile->display_name,
                 "Mercedes-Benz C207 E 250 CDI / OM651 / Delphi CRD3.x") == 0);
    CHECK(profile->endpoints != NULL);
    CHECK(profile->endpoint_count == 2U);
    CHECK(strcmp(profile->endpoints[0].key,
                 "c207-om651-engine-eobd-11bit") == 0);
    CHECK(strcmp(profile->endpoints[0].name,
                 "Delphi CRD3.x engine ECU") == 0);
    CHECK(profile->endpoints[0].status ==
          MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED);
    CHECK(strcmp(mblink_mercedes_definition_status_name(
                     profile->endpoints[0].status),
                 "vehicle-verified") == 0);
    CHECK(strstr(profile->endpoints[0].provenance,
                 "2026-08-26") != NULL);
    CHECK(strstr(profile->endpoints[0].provenance,
                 "0x7E0/0x7E8") != NULL);
    CHECK(profile->endpoints[0].module == MBLINK_MERCEDES_MODULE_ENGINE);
    CHECK(profile->endpoints[0].address.tx_can_id == 0x7e0U);
    CHECK(profile->endpoints[0].address.rx_can_id == 0x7e8U);
    CHECK(!profile->endpoints[0].address.tx_extended_id);
    CHECK(!profile->endpoints[0].address.rx_extended_id);
    CHECK(profile->endpoints[0].address.addressing_mode ==
          MBLINK_ISOTP_ADDRESSING_NORMAL);
    CHECK(mblink_mercedes_ecu_endpoint_is_verified(&profile->endpoints[0]));
    CHECK(mblink_mercedes_profile_find_endpoint(
              profile, "c207-om651-engine-eobd-11bit") ==
          &profile->endpoints[0]);
    CHECK(strcmp(profile->endpoints[1].key,
                 "c207-secondary-powertrain-eobd-11bit") == 0);
    CHECK(strcmp(profile->endpoints[1].name,
                 "Secondary EOBD powertrain ECU") == 0);
    CHECK(profile->endpoints[1].module == MBLINK_MERCEDES_MODULE_OTHER);
    CHECK(profile->endpoints[1].address.tx_can_id == 0x7e1U);
    CHECK(profile->endpoints[1].address.rx_can_id == 0x7e9U);
    CHECK(profile->endpoints[1].status ==
          MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED);
    CHECK(mblink_mercedes_ecu_endpoint_is_verified(&profile->endpoints[1]));
    CHECK(mblink_mercedes_profile_find_endpoint(
              profile, "c207-secondary-powertrain-eobd-11bit") ==
          &profile->endpoints[1]);
    CHECK(mblink_mercedes_profile_find_endpoint(profile, "missing") == NULL);
    CHECK(profile->definitions == NULL);
    CHECK(profile->definition_count == 0U);
    CHECK(mblink_mercedes_vehicle_profile_is_valid(profile));
    CHECK(mblink_mercedes_profile_find_did(
              profile, MBLINK_MERCEDES_MODULE_ENGINE, 0x1234U) == NULL);
    return 0;
}

static MblinkMercedesEcuEndpointDefinition make_endpoint(
    const char *key,
    uint32_t tx_can_id,
    uint32_t rx_can_id)
{
    MblinkMercedesEcuEndpointDefinition endpoint = {
        .key = key,
        .name = "Test endpoint",
        .module = MBLINK_MERCEDES_MODULE_ENGINE,
        .address = {
            .tx_can_id = tx_can_id,
            .rx_can_id = rx_can_id,
            .tx_extended_id = false,
            .rx_extended_id = false,
            .addressing_mode = MBLINK_ISOTP_ADDRESSING_NORMAL,
            .target_type = MBLINK_ISOTP_TARGET_PHYSICAL,
            .tx_address_extension = 0U,
            .rx_address_extension = 0U
        },
        .status = MBLINK_MERCEDES_DEFINITION_CANDIDATE,
        .provenance = "test fixture"
    };
    return endpoint;
}

static int test_endpoint_validation(void)
{
    MblinkMercedesEcuEndpointDefinition endpoints[2] = {
        make_endpoint("engine-a", 0x7e0U, 0x7e8U),
        make_endpoint("engine-b", 0x7e1U, 0x7e9U)
    };
    MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "OM651",
        .display_name = "test profile",
        .endpoints = endpoints,
        .endpoint_count = 2U,
        .definitions = NULL,
        .definition_count = 0U
    };

    CHECK(mblink_mercedes_ecu_endpoint_is_valid(&endpoints[0]));
    CHECK(!mblink_mercedes_ecu_endpoint_is_verified(&endpoints[0]));
    endpoints[0].status = MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED;
    CHECK(mblink_mercedes_ecu_endpoint_is_verified(&endpoints[0]));
    CHECK(mblink_mercedes_vehicle_profile_is_valid(&profile));

    endpoints[1].key = "engine-a";
    CHECK(!mblink_mercedes_vehicle_profile_is_valid(&profile));
    endpoints[1] = make_endpoint("engine-b", 0x7e0U, 0x7e8U);
    CHECK(!mblink_mercedes_vehicle_profile_is_valid(&profile));
    endpoints[1] = make_endpoint("engine-b", 0x7e1U, 0x7e9U);
    endpoints[1].address.rx_can_id = endpoints[1].address.tx_can_id;
    CHECK(!mblink_mercedes_ecu_endpoint_is_valid(&endpoints[1]));
    endpoints[1] = make_endpoint("engine-b", 0x800U, 0x7e9U);
    CHECK(!mblink_mercedes_ecu_endpoint_is_valid(&endpoints[1]));
    endpoints[1] = make_endpoint("engine-b", 0x7e1U, 0x7e9U);
    endpoints[1].address.target_type = MBLINK_ISOTP_TARGET_FUNCTIONAL;
    CHECK(!mblink_mercedes_ecu_endpoint_is_valid(&endpoints[1]));
    return 0;
}

static int test_definition_validation(void)
{
    MblinkMercedesDidDefinition candidate = make_definition(
        0x1234U, MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DEFINITION_CANDIDATE, "fixture-candidate");
    MblinkMercedesDidDefinition verified = candidate;

    CHECK(mblink_mercedes_did_definition_is_valid(&candidate));
    CHECK(!mblink_mercedes_did_definition_is_verified(&candidate));
    CHECK(strcmp(mblink_mercedes_definition_status_name(candidate.status),
                 "candidate") == 0);
    CHECK(strcmp(mblink_mercedes_module_kind_name(candidate.module),
                 "engine") == 0);

    candidate.status = MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
    candidate.provenance = "fixture-source";
    CHECK(mblink_mercedes_did_definition_is_valid(&candidate));
    CHECK(!mblink_mercedes_did_definition_is_verified(&candidate));
    CHECK(strcmp(mblink_mercedes_definition_status_name(candidate.status),
                 "source-corroborated") == 0);

    verified.status = MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED;
    verified.provenance = "vehicle-capture-test";
    CHECK(mblink_mercedes_did_definition_is_valid(&verified));
    CHECK(mblink_mercedes_did_definition_is_verified(&verified));
    CHECK(strcmp(mblink_mercedes_definition_status_name(verified.status),
                 "vehicle-verified") == 0);

    candidate.provenance = "";
    CHECK(!mblink_mercedes_did_definition_is_valid(&candidate));
    candidate = verified;
    candidate.uds.key = "";
    CHECK(!mblink_mercedes_did_definition_is_valid(&candidate));
    candidate = verified;
    candidate.uds.minimum_length = 5U;
    candidate.uds.maximum_length = 4U;
    CHECK(!mblink_mercedes_did_definition_is_valid(&candidate));
    return 0;
}

static int test_profile_lookup_and_duplicates(void)
{
    MblinkMercedesDidDefinition definitions[2] = {
        make_definition(0x1234U, MBLINK_MERCEDES_MODULE_ENGINE,
                        MBLINK_MERCEDES_DEFINITION_CANDIDATE, "fixture-a"),
        make_definition(0x1235U, MBLINK_MERCEDES_MODULE_ENGINE,
                        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
                        "fixture-b")
    };
    MblinkMercedesVehicleProfile profile = {
        .chassis_code = "C207",
        .engine_family = "OM651",
        .display_name = "test profile",
        .endpoints = NULL,
        .endpoint_count = 0U,
        .definitions = definitions,
        .definition_count = 2U
    };

    CHECK(mblink_mercedes_vehicle_profile_is_valid(&profile));
    CHECK(mblink_mercedes_profile_find_did(
              &profile, MBLINK_MERCEDES_MODULE_ENGINE, 0x1234U) ==
          &definitions[0]);
    CHECK(mblink_mercedes_profile_find_did(
              &profile, MBLINK_MERCEDES_MODULE_ENGINE, 0x1235U) ==
          &definitions[1]);
    CHECK(mblink_mercedes_profile_find_did(
              &profile, MBLINK_MERCEDES_MODULE_TRANSMISSION, 0x1234U) == NULL);

    definitions[1].uds.identifier = 0x1234U;
    CHECK(!mblink_mercedes_vehicle_profile_is_valid(&profile));

    definitions[1].module = MBLINK_MERCEDES_MODULE_TRANSMISSION;
    CHECK(mblink_mercedes_vehicle_profile_is_valid(&profile));
    return 0;
}

static int test_decode_wrapper(void)
{
    MblinkMercedesDidDefinition definition = make_definition(
        0x1234U, MBLINK_MERCEDES_MODULE_ENGINE,
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        "vehicle-capture-test");
    const uint8_t good[] = { 0x62U, 0x12U, 0x34U, 0xaaU, 0xbbU };
    const uint8_t too_short[] = { 0x62U, 0x12U, 0x34U, 0xaaU };
    MblinkUdsDidValue value;

    CHECK(mblink_mercedes_decode_defined_did(
              good, sizeof(good), &definition, &value) == MBLINK_UDS_RESULT_OK);
    CHECK(value.definition == &definition.uds);
    CHECK(value.data_length == 2U);
    CHECK(value.data[0] == 0xaaU && value.data[1] == 0xbbU);

    memset(&value, 0x5a, sizeof(value));
    {
        MblinkUdsDidValue snapshot = value;
        CHECK(mblink_mercedes_decode_defined_did(
                  too_short, sizeof(too_short), &definition, &value) ==
              MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
        CHECK(memcmp(&value, &snapshot, sizeof(value)) == 0);
    }
    return 0;
}

int main(void)
{
    if (test_development_profile() != 0) return 1;
    if (test_endpoint_validation() != 0) return 1;
    if (test_definition_validation() != 0) return 1;
    if (test_profile_lookup_and_duplicates() != 0) return 1;
    if (test_decode_wrapper() != 0) return 1;
    return 0;
}
