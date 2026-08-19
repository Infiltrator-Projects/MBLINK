// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/parameter.h"
#include "mblink/parameter_store.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "mblink-parameter-store-test: %s\n", message);
    }
    return condition;
}

int main(void)
{
    bool passed = true;
    MblinkParameterStore store;
    const MblinkParameterDefinition *rpm =
        mblink_parameter_obd2_definition(0x0cU);
    const MblinkParameterDefinition *coolant =
        mblink_parameter_obd2_definition(0x05U);
    MblinkObd2Sample obd = { 0x0cU, 1500.0, MBLINK_OBD2_UNIT_RPM };
    MblinkParameterSample parameter;
    MblinkParameterSample latest;

    mblink_parameter_store_init(&store);
    passed &= check(mblink_parameter_store_definition_count(&store) == 0U,
                    "new store should have no definitions");
    passed &= check(mblink_parameter_store_history_count(&store) == 0U,
                    "new store should have no history");

    passed &= check(rpm != NULL && coolant != NULL,
                    "standard definitions missing");
    if (rpm == NULL || coolant == NULL) {
        return EXIT_FAILURE;
    }

    passed &= check(mblink_parameter_store_register(&store, rpm) ==
                        MBLINK_PARAMETER_STORE_OK,
                    "RPM registration failed");
    passed &= check(mblink_parameter_store_register(&store, coolant) ==
                        MBLINK_PARAMETER_STORE_OK,
                    "coolant registration failed");
    passed &= check(mblink_parameter_store_definition_count(&store) == 2U,
                    "definition count mismatch");
    passed &= check(mblink_parameter_store_definition_at(&store, 0U) == rpm,
                    "definition order mismatch");
    passed &= check(mblink_parameter_store_definition(&store, &rpm->key) == rpm,
                    "key lookup mismatch");
    passed &= check(mblink_parameter_store_definition_for_stable_key(
                        &store, "obd2.engine.coolant") == coolant,
                    "stable-key lookup mismatch");
    passed &= check(mblink_parameter_store_register(&store, rpm) ==
                        MBLINK_PARAMETER_STORE_DUPLICATE_KEY,
                    "duplicate key should be rejected");

    {
        MblinkParameterDefinition duplicate_stable = *rpm;
        duplicate_stable.key.identifier = 0x77U;
        passed &= check(mblink_parameter_store_register(
                            &store, &duplicate_stable) ==
                            MBLINK_PARAMETER_STORE_DUPLICATE_STABLE_KEY,
                        "duplicate stable key should be rejected");
    }

    passed &= check(mblink_parameter_store_set_favourite(
                        &store, &rpm->key, true) == MBLINK_PARAMETER_STORE_OK,
                    "set favourite failed");
    passed &= check(mblink_parameter_store_is_favourite(&store, &rpm->key),
                    "favourite state missing");

    passed &= check(mblink_parameter_from_obd2(&obd, 10U, &parameter),
                    "OBD conversion failed");
    passed &= check(mblink_parameter_store_record(&store, &parameter) ==
                        MBLINK_PARAMETER_STORE_OK,
                    "record failed");
    passed &= check(mblink_parameter_store_latest(
                        &store, &rpm->key, &latest),
                    "latest lookup failed");
    passed &= check(latest.definition == rpm && latest.timestamp_ms == 10U &&
                    latest.available && latest.value == 1500.0,
                    "latest sample mismatch");
    passed &= check(mblink_parameter_store_history_count(&store) == 1U &&
                    mblink_parameter_store_total_sample_count(&store) == 1U,
                    "history counters mismatch");

    {
        MblinkParameterDefinition copied_definition = *rpm;
        MblinkParameterSample wrong_definition = parameter;
        wrong_definition.definition = &copied_definition;
        passed &= check(mblink_parameter_store_record(
                            &store, &wrong_definition) ==
                            MBLINK_PARAMETER_STORE_DEFINITION_MISMATCH,
                        "non-canonical definition pointer should be rejected");
        passed &= check(mblink_parameter_store_total_sample_count(&store) == 1U,
                        "failed record changed sample count");
    }

    mblink_parameter_store_clear_samples(&store);
    passed &= check(mblink_parameter_store_history_count(&store) == 0U &&
                    mblink_parameter_store_total_sample_count(&store) == 0U,
                    "clear did not reset sample state");
    passed &= check(!mblink_parameter_store_latest(
                        &store, &rpm->key, &latest),
                    "clear left latest sample valid");
    passed &= check(mblink_parameter_store_is_favourite(&store, &rpm->key),
                    "clear should preserve favourites");
    passed &= check(mblink_parameter_store_definition_count(&store) == 2U,
                    "clear should preserve definitions");

    for (size_t index = 0U;
         index < MBLINK_PARAMETER_STORE_HISTORY_CAPACITY + 5U;
         ++index) {
        obd.value = 1000.0 + (double)index;
        passed &= check(mblink_parameter_from_obd2(
                            &obd, (uint64_t)index, &parameter),
                        "loop OBD conversion failed");
        passed &= check(mblink_parameter_store_record(&store, &parameter) ==
                            MBLINK_PARAMETER_STORE_OK,
                        "loop record failed");
    }

    passed &= check(mblink_parameter_store_history_count(&store) ==
                        MBLINK_PARAMETER_STORE_HISTORY_CAPACITY,
                    "history ring did not cap at capacity");
    passed &= check(mblink_parameter_store_total_sample_count(&store) ==
                        MBLINK_PARAMETER_STORE_HISTORY_CAPACITY + 5U,
                    "total sample count should exceed retained history");

    {
        MblinkParameterSample first;
        MblinkParameterSample last;
        passed &= check(mblink_parameter_store_history_at(&store, 0U, &first),
                        "oldest retained sample unavailable");
        passed &= check(mblink_parameter_store_history_at(
                            &store,
                            MBLINK_PARAMETER_STORE_HISTORY_CAPACITY - 1U,
                            &last),
                        "newest retained sample unavailable");
        passed &= check(first.timestamp_ms == 5U,
                        "history ring retained the wrong oldest sample");
        passed &= check(last.timestamp_ms ==
                            MBLINK_PARAMETER_STORE_HISTORY_CAPACITY + 4U,
                        "history ring retained the wrong newest sample");
        passed &= check(!mblink_parameter_store_history_at(
                            &store,
                            MBLINK_PARAMETER_STORE_HISTORY_CAPACITY,
                            &last),
                        "out-of-range history index should fail");
    }

    passed &= check(strcmp(mblink_parameter_store_result_name(
                               MBLINK_PARAMETER_STORE_DEFINITION_MISMATCH),
                           "definition-mismatch") == 0,
                    "result name mismatch");

    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
