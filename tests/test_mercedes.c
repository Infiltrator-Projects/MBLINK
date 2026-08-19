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
    CHECK(strcmp(profile->display_name, "Mercedes-Benz C207 / OM651") == 0);
    CHECK(profile->definitions == NULL);
    CHECK(profile->definition_count == 0U);
    CHECK(mblink_mercedes_vehicle_profile_is_valid(profile));
    CHECK(mblink_mercedes_profile_find_did(
              profile, MBLINK_MERCEDES_MODULE_ENGINE, 0x1234U) == NULL);
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
    if (test_definition_validation() != 0) return 1;
    if (test_profile_lookup_and_duplicates() != 0) return 1;
    if (test_decode_wrapper() != 0) return 1;
    return 0;
}
