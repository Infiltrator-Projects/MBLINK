// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/obd2.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    MblinkDtcKnowledge fault;
    char status[LINK_DTC_STATUS_TEXT_LENGTH];

    check(mblink_dtc_resolve("P0401", &fault), "MBLINK resolves shared P0401");
    check(fault.definition_known, "P0401 definition is known");
    check(strcmp(fault.title,
                 "Exhaust Gas Recirculation Flow Insufficient Detected") == 0,
          "P0401 human-readable title reaches MBLINK");
    check(strcmp(fault.category, "EGR/emissions") == 0,
          "P0401 category reaches MBLINK");
    check(fault.origin == MBLINK_DTC_ORIGIN_STANDARD_GENERIC,
          "P0401 is standards-generic");

    check(mblink_dtc_resolve("P2453", &fault), "MBLINK resolves DPF pressure fault");
    check(fault.definition_known && strstr(fault.title, "Particulate Filter") != NULL,
          "DPF fault translation reaches MBLINK");

    check(mblink_dtc_resolve("P1450", &fault),
          "valid manufacturer-specific code remains a diagnostic record");
    check(!fault.definition_known &&
          fault.origin == MBLINK_DTC_ORIGIN_MANUFACTURER_SPECIFIC,
          "manufacturer-specific unknown is explicit rather than fabricated");

    check(mblink_dtc_format_uds_status(0x0dU, status, sizeof(status)),
          "shared UDS status formatting reaches MBLINK");
    check(strstr(status, "Test failed") != NULL &&
          strstr(status, "Pending") != NULL &&
          strstr(status, "Confirmed") != NULL,
          "UDS 0x0D status is translated");

    if (failures != 0) {
        fprintf(stderr, "%d MBLINK DTC knowledge test(s) failed\n", failures);
        return 1;
    }
    return 0;
}
