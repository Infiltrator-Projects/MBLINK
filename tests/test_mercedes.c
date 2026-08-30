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
    const MblinkMercedesEcuEndpointDefinition *generic_endpoint =
        mblink_mercedes_generic_engine_endpoint();
    const MblinkMercedesVehicleProfile *generic =
        mblink_mercedes_generic_profile();
    const MblinkMercedesVehicleProfile *c207 =
        mblink_mercedes_c207_generic_profile();
    const MblinkMercedesVehicleProfile *om651 =
        mblink_mercedes_c207_om651_profile();
    const MblinkMercedesVehicleProfile *m271 =
        mblink_mercedes_c207_m271_profile();
    const MblinkMercedesVehicleProfile *m274 =
        mblink_mercedes_c207_m274_profile();

    CHECK(generic_endpoint != NULL);
    CHECK(generic_endpoint->address.tx_can_id == 0x7e0U);
    CHECK(generic_endpoint->address.rx_can_id == 0x7e8U);
    CHECK(generic_endpoint->status == MBLINK_MERCEDES_DEFINITION_CANDIDATE);
    CHECK(mblink_mercedes_ecu_endpoint_is_valid(generic_endpoint));
    CHECK(!mblink_mercedes_ecu_endpoint_is_verified(generic_endpoint));

    CHECK(mblink_mercedes_vehicle_profile_is_valid(generic));
    CHECK(strcmp(generic->engine_family, "Unidentified") == 0);
    CHECK(mblink_mercedes_vehicle_profile_is_valid(c207));
    CHECK(strcmp(c207->chassis_code, "C207") == 0);
    CHECK(strcmp(c207->engine_family, "Unidentified") == 0);

    CHECK(mblink_mercedes_vehicle_profile_is_valid(om651));
    CHECK(strcmp(om651->chassis_code, "C207") == 0);
    CHECK(strcmp(om651->engine_family, "OM651") == 0);
    CHECK(om651->endpoint_count == 2U);
    CHECK(om651->endpoints[0].status ==
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    CHECK(!mblink_mercedes_ecu_endpoint_is_verified(&om651->endpoints[0]));
    CHECK(mblink_mercedes_profile_is_crd3_candidate(om651));

    CHECK(mblink_mercedes_vehicle_profile_is_valid(m271));
    CHECK(strcmp(m271->engine_family, "M271") == 0);
    CHECK(!mblink_mercedes_profile_is_crd3_candidate(m271));
    CHECK(mblink_mercedes_vehicle_profile_is_valid(m274));
    CHECK(strcmp(m274->engine_family, "M274") == 0);
    CHECK(!mblink_mercedes_profile_is_crd3_candidate(m274));

    CHECK(mblink_mercedes_profile_for_vin("WDD2073022F123456") == om651);
    CHECK(mblink_mercedes_profile_for_vin("WDD2073032F123456") == om651);
    CHECK(mblink_mercedes_profile_for_vin("WDD2073471F123456") == m271);
    CHECK(mblink_mercedes_profile_for_vin("WDD2073361F123456") == m274);
    CHECK(mblink_mercedes_profile_for_vin("WDD2073991F123456") == c207);
    CHECK(mblink_mercedes_profile_for_vin("WDD2120001A123456") == generic);
    CHECK(mblink_mercedes_profile_for_vin(NULL) == generic);
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

static int test_kwp_dtc_lookup(void)
{
    const MblinkMercedesKwpDtcDefinition *definition;

    CHECK(mblink_mercedes_kwp_dtc_count() >= 1U);
    definition = mblink_mercedes_kwp_dtc_find(
        "restraints-orc", UINT16_C(0x9b51));
    CHECK(definition != NULL);
    CHECK(definition == mblink_mercedes_kwp_dtc_at(0U));
    CHECK(definition->code == UINT16_C(0x9b51));
    CHECK(strcmp(definition->module_key, "restraints-orc") == 0);
    CHECK(strstr(definition->description, "Driver seat-belt buckle") != NULL);
    CHECK(strstr(definition->description, "short to positive") != NULL);
    CHECK(strstr(definition->description, "open circuit") != NULL);
    CHECK(definition->status ==
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    CHECK(definition->provenance != NULL && definition->provenance[0] != '\0');

    /* The same numeric value must not leak into another module namespace. */
    CHECK(mblink_mercedes_kwp_dtc_find(
              "engine-crd3", UINT16_C(0x9b51)) == NULL);
    CHECK(mblink_mercedes_kwp_dtc_find(
              "restraints-orc", UINT16_C(0x9b52)) == NULL);
    CHECK(mblink_mercedes_kwp_dtc_find(NULL, UINT16_C(0x9b51)) == NULL);
    CHECK(mblink_mercedes_kwp_dtc_at(mblink_mercedes_kwp_dtc_count()) == NULL);
    return 0;
}

int main(void)
{
    if (test_development_profile() != 0) return 1;
    if (test_endpoint_validation() != 0) return 1;
    if (test_definition_validation() != 0) return 1;
    if (test_profile_lookup_and_duplicates() != 0) return 1;
    if (test_decode_wrapper() != 0) return 1;
    if (test_kwp_dtc_lookup() != 0) return 1;
    return 0;
}
