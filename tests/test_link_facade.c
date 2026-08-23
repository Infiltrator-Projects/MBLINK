// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/isotp.h"
#include "mblink/uds_services.h"

#include <stdio.h>

int main(void)
{
    const MblinkUdsServiceDefinition *routine;

    if (MBLINK_ISOTP_CAN_FD_MAX_DATA_LENGTH != 64U ||
        !mblink_isotp_can_data_length_is_valid(
            true, MBLINK_ISOTP_CAN_FD_MAX_DATA_LENGTH)) {
        fputs("MBLINK CAN-FD ISO-TP facade is incomplete\n", stderr);
        return 1;
    }

    if (mblink_uds_standard_service_count() !=
        MBLINK_UDS_STANDARD_SERVICE_COUNT) {
        fputs("MBLINK UDS service facade has the wrong catalogue size\n", stderr);
        return 1;
    }

    routine = mblink_uds_standard_service_find(
        MBLINK_UDS_SERVICE_ROUTINE_CONTROL);
    if (routine == NULL ||
        routine->service != MBLINK_UDS_SERVICE_ROUTINE_CONTROL ||
        routine->effect != MBLINK_UDS_SERVICE_EFFECT_STATE_CHANGING) {
        fputs("MBLINK UDS service facade did not preserve LINK metadata\n",
              stderr);
        return 1;
    }

    puts("MBLINK LINK facade passed");
    return 0;
}
