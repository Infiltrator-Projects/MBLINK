// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/parameter.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "mblink-parameter-test: %s\n", message);
    }
    return condition;
}

int main(void)
{
    bool passed = true;
    char buffer[64];
    MblinkObd2Sample obd = { 0x0cU, 1234.4, MBLINK_OBD2_UNIT_RPM };
    MblinkParameterSample parameter = { 0 };
    const MblinkParameterDefinition *rpm =
        mblink_parameter_obd2_definition(0x0cU);
    const MblinkParameterDefinition *maf =
        mblink_parameter_obd2_definition(0x10U);
    const MblinkParameterDefinition *rail =
        mblink_parameter_obd2_definition(0x23U);
    const MblinkParameterDefinition *throttle_valve =
        mblink_parameter_obd2_definition(0x11U);
    const MblinkParameterDefinition *pedal_d =
        mblink_parameter_obd2_definition(0x49U);
    const MblinkParameterDefinition *pedal_e =
        mblink_parameter_obd2_definition(0x4aU);
    const MblinkParameterDefinition *fuel_level =
        mblink_parameter_obd2_definition(0x2fU);
    const MblinkParameterDefinition *engine_runtime =
        mblink_parameter_obd2_definition(0x1fU);
    const MblinkParameterDefinition *distance_since_clear =
        mblink_parameter_obd2_definition(0x31U);
    const MblinkParameterDefinition *mil_runtime =
        mblink_parameter_obd2_definition(0x4dU);
    const MblinkParameterDefinition *throttle_g =
        mblink_parameter_obd2_definition(0x8dU);
    const MblinkParameterDefinition *reflash_distance =
        mblink_parameter_obd2_definition(0xc7U);
    const MblinkParameterDefinition *max_speed_limit =
        mblink_parameter_obd2_definition(0xaaU);
    const MblinkParameterDefinition *traction_battery_soh =
        mblink_parameter_obd2_definition(0xb2U);
    const MblinkParameterDefinition *engine_odometer =
        mblink_parameter_obd2_definition(0xd3U);

    passed &= check(mblink_parameter_obd2_definition_count() == 62U,
                    "expanded standard scalar descriptor count mismatch");
    passed &= check(rpm != NULL && maf != NULL && rail != NULL &&
                    throttle_valve != NULL && pedal_d != NULL &&
                    pedal_e != NULL && fuel_level != NULL &&
                    engine_runtime != NULL && distance_since_clear != NULL &&
                    mil_runtime != NULL && throttle_g != NULL &&
                    reflash_distance != NULL && max_speed_limit != NULL &&
                    traction_battery_soh != NULL && engine_odometer != NULL,
                    "expected OBD descriptors missing");
    passed &= check(mblink_parameter_obd2_definition(0xffU) == NULL,
                    "unknown PID unexpectedly has a descriptor");
    passed &= check(mblink_parameter_obd2_definition_at(62U) == NULL,
                    "out-of-range descriptor index should fail");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key("obd2.engine.rpm") == rpm,
        "stable-key lookup mismatch");
    passed &= check(
        throttle_valve != NULL &&
        strcmp(throttle_valve->name, "Absolute throttle valve position") == 0,
        "PID 0x11 must be labelled as throttle valve, not accelerator pedal");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key(
            "obd2.driver.accelerator_pedal_d") == pedal_d,
        "accelerator-pedal D stable-key lookup mismatch");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key(
            "obd2.driver.accelerator_pedal_e") == pedal_e,
        "accelerator-pedal E stable-key lookup mismatch");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key(
            "obd2.fuel.tank_level") == fuel_level,
        "fuel-level stable-key lookup mismatch");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key(
            "obd2.engine.runtime") == engine_runtime,
        "engine-runtime stable-key lookup mismatch");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key(
            "obd2.maintenance.distance_since_clear") == distance_since_clear,
        "distance-since-clear stable-key lookup mismatch");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key(
            "obd2.emissions.mil_runtime") == mil_runtime,
        "MIL-runtime stable-key lookup mismatch");
    passed &= check(
        mblink_parameter_obd2_definition_for_stable_key("obd2.missing") == NULL,
        "unknown stable key unexpectedly resolved");

    passed &= check(mblink_parameter_obd2_definition(0x06U) != NULL &&
                    mblink_parameter_obd2_definition(0xa6U) != NULL,
                    "expanded shared scalar catalogue is not visible through MBLINK");
    passed &= check(throttle_g != NULL && reflash_distance != NULL &&
                    max_speed_limit != NULL &&
                    traction_battery_soh != NULL &&
                    engine_odometer != NULL,
                    "LINK 0.14.59 scalar definitions are missing");
    passed &= check(mblink_parameter_obd2_definition(0x7aU) == NULL &&
                    mblink_parameter_obd2_definition(0x7cU) == NULL,
                    "structured DPF PIDs must not be flattened into fake scalars");
    passed &= check(mblink_obd2_mode01_identifier_count() == 256U &&
                    mblink_obd2_mode01_assigned_count() == 220U,
                    "complete SAE Mode 01 namespace is not visible through MBLINK");
    passed &= check(mblink_obd2_pid_definition(0x01U, 0x7aU) != NULL,
                    "structured SAE PID 0x7A is missing from shared catalogue");

    if (rpm != NULL) {
        MblinkParameterKey same = rpm->key;
        MblinkParameterKey different = rpm->key;
        different.identifier++;
        passed &= check(mblink_parameter_definition_is_valid(rpm),
                        "RPM definition should validate");
        passed &= check(strcmp(rpm->short_name, "RPM") == 0,
                        "RPM short name mismatch");
        passed &= check(mblink_parameter_key_equal(&rpm->key, &same),
                        "equal keys did not compare equal");
        passed &= check(!mblink_parameter_key_equal(&rpm->key, &different),
                        "different keys compared equal");
    }

    passed &= check(mblink_parameter_from_obd2(&obd, 42U, &parameter),
                    "OBD conversion failed");
    passed &= check(parameter.definition == rpm && parameter.available &&
                    parameter.timestamp_ms == 42U &&
                    fabs(parameter.value - 1234.4) < 0.0001,
                    "OBD conversion produced wrong parameter");
    passed &= check(mblink_parameter_format_sample(
                        &parameter, buffer, sizeof(buffer)) &&
                    strcmp(buffer, "1234 rpm") == 0,
                    "RPM formatting mismatch");

    obd.pid = 0x10U;
    obd.value = 12.34;
    obd.unit = MBLINK_OBD2_UNIT_GRAMS_PER_SECOND;
    passed &= check(mblink_parameter_from_obd2(&obd, 100U, &parameter),
                    "MAF conversion failed");
    passed &= check(parameter.definition == maf &&
                    mblink_parameter_format_sample(
                        &parameter, buffer, sizeof(buffer)) &&
                    strcmp(buffer, "12.3 g/s") == 0,
                    "MAF formatting mismatch");

    obd.pid = 0x23U;
    obd.value = 123400.0;
    obd.unit = MBLINK_OBD2_UNIT_KPA;
    passed &= check(mblink_parameter_from_obd2(&obd, 110U, &parameter),
                    "rail-pressure conversion failed");
    passed &= check(parameter.definition == rail &&
                    mblink_parameter_format_sample(
                        &parameter, buffer, sizeof(buffer)) &&
                    strcmp(buffer, "123.4 MPa") == 0,
                    "rail-pressure display auto-scaling mismatch");

    if (rpm != NULL) {
        MblinkParameterSample unavailable = { rpm, 0U, false, NAN };
        passed &= check(mblink_parameter_sample_is_valid(&unavailable),
                        "unavailable sample should validate");
        passed &= check(mblink_parameter_format_sample(
                            &unavailable, buffer, sizeof(buffer)) &&
                        strcmp(buffer, "N/A") == 0,
                        "unavailable formatting mismatch");
    }

    {
        MblinkParameterSample sentinel = parameter;
        obd.pid = 0x0cU;
        obd.value = 2000.0;
        obd.unit = MBLINK_OBD2_UNIT_KPA;
        passed &= check(!mblink_parameter_from_obd2(&obd, 200U, &parameter),
                        "wrong OBD unit should be rejected");
        passed &= check(memcmp(&parameter, &sentinel, sizeof(parameter)) == 0,
                        "failed conversion must be transactional");
    }

    {
        MblinkParameterDefinition uds = {
            { MBLINK_PARAMETER_PROTOCOL_UDS, 1U, 0xf190U },
            "mercedes.engine.example", "EXAMPLE", "Example UDS value", "", 1U,
            false, 0.0, 0.0
        };
        MblinkParameterDefinition invalid = uds;
        passed &= check(mblink_parameter_definition_is_valid(&uds),
                        "valid UDS definition rejected");
        invalid.key.identifier = 0x10000U;
        passed &= check(!mblink_parameter_definition_is_valid(&invalid),
                        "out-of-range UDS identifier accepted");
        invalid = uds;
        invalid.decimal_places = 10U;
        passed &= check(!mblink_parameter_definition_is_valid(&invalid),
                        "invalid decimal precision accepted");
        invalid = uds;
        invalid.short_name = "";
        passed &= check(!mblink_parameter_definition_is_valid(&invalid),
                        "empty short name accepted");
    }

    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
