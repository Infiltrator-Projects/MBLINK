// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_signal_catalog.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

int main(void)
{
    const MblinkMercedesBackendSignalDefinition *dpf =
        mblink_mercedes_backend_signal_find_backend_key("filterParticleLoading");
    const MblinkMercedesBackendSignalDefinition *adblue =
        mblink_mercedes_backend_signal_find_key("mercedes.backend.adblue_level");
    const MblinkMercedesBackendSignalDefinition *fuel =
        mblink_mercedes_backend_signal_find_backend_key("tanklevelpercent");

    CHECK(mblink_mercedes_backend_signal_count() >= 30U);
    CHECK(dpf != NULL);
    CHECK(dpf->value_type == MBLINK_MERCEDES_SIGNAL_INT);
    CHECK(dpf->diagnostic_research_priority);
    CHECK(strstr(dpf->applicability, "model/year") != NULL);
    CHECK(strcmp(dpf->unit_family, "none") == 0);
    CHECK(strstr(dpf->note, "no ECU") != NULL);

    CHECK(adblue != NULL);
    CHECK(strcmp(adblue->backend_key, "tankLevelAdBlue") == 0);
    CHECK(strcmp(adblue->unit_family, "ratio") == 0);

    CHECK(fuel != NULL);
    CHECK(fuel->correlation_reference_key != NULL);
    CHECK(strcmp(fuel->correlation_reference_key,
                 "obd2.fuel.tank_level") == 0);
    CHECK(strcmp(mblink_mercedes_signal_value_type_name(
                     MBLINK_MERCEDES_SIGNAL_DOUBLE),
                 "double") == 0);

    puts("Mercedes backend signal catalogue tests passed");
    return 0;
}
