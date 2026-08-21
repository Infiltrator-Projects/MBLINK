// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_om651.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_catalogue(void)
{
    size_t count = 0U;
    const MblinkMercedesOm651SignalDefinition *definitions =
        mblink_mercedes_om651_signal_definitions(&count);

    CHECK(definitions != NULL);
    CHECK(count == 23U);
    for (size_t index = 0U; index < count; ++index) {
        CHECK(definitions[index].key != NULL);
        CHECK(definitions[index].key[0] != '\0');
        CHECK(definitions[index].name != NULL);
        CHECK(definitions[index].name[0] != '\0');
        CHECK(definitions[index].provenance != NULL);
        CHECK(definitions[index].provenance[0] != '\0');
        CHECK(definitions[index].status ==
              MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED);
        for (size_t earlier = 0U; earlier < index; ++earlier) {
            CHECK(strcmp(definitions[earlier].key, definitions[index].key) != 0);
        }
    }
    return 0;
}

static int test_lookup_and_status(void)
{
    const MblinkMercedesOm651SignalDefinition *signal =
        mblink_mercedes_om651_find_signal(
            "mercedes.om651.dpf.soot_mass");

    CHECK(signal != NULL);
    CHECK(strcmp(signal->name, "DPF soot mass") == 0);
    CHECK(signal->category == MBLINK_MERCEDES_OM651_CATEGORY_DPF);
    CHECK(strcmp(mblink_mercedes_om651_signal_category_name(signal->category),
                 "dpf") == 0);
    CHECK(strcmp(mblink_mercedes_om651_signal_status_name(signal->status),
                 "corroborated-unmapped") == 0);
    CHECK(mblink_mercedes_om651_find_signal("not.real") == NULL);
    CHECK(mblink_mercedes_om651_find_signal(NULL) == NULL);
    return 0;
}

static int test_priority_groups_exist(void)
{
    static const char *keys[] = {
        "mercedes.om651.dpf.differential_pressure",
        "mercedes.om651.dpf.regeneration_status",
        "mercedes.om651.fuel.rail_pressure",
        "mercedes.om651.air.boost_pressure",
        "mercedes.om651.egr.command_or_rate",
        "mercedes.om651.injector.smooth_running.cylinder1",
        "mercedes.om651.injector.correction.cylinder4"
    };

    for (size_t index = 0U; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        CHECK(mblink_mercedes_om651_find_signal(keys[index]) != NULL);
    }
    return 0;
}

int main(void)
{
    if (test_catalogue() != 0) return 1;
    if (test_lookup_and_status() != 0) return 1;
    if (test_priority_groups_exist() != 0) return 1;
    puts("Mercedes OM651 catalogue tests passed");
    return 0;
}
