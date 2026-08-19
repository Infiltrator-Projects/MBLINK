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

    passed &= check(mblink_parameter_obd2_definition_count() == 8U,
                    "standard descriptor count mismatch");
    passed &= check(rpm != NULL && maf != NULL,
                    "expected OBD descriptors missing");
    passed &= check(mblink_parameter_obd2_definition(0xffU) == NULL,
                    "unknown PID unexpectedly has a descriptor");
    passed &= check(mblink_parameter_obd2_definition_at(8U) == NULL,
                    "out-of-range descriptor index should fail");

    if (rpm != NULL) {
        MblinkParameterKey same = rpm->key;
        MblinkParameterKey different = rpm->key;
        different.identifier++;
        passed &= check(mblink_parameter_definition_is_valid(rpm),
                        "RPM definition should validate");
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
            "mercedes.engine.example", "Example UDS value", "", 1U,
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
    }

    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
