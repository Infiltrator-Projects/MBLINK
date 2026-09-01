// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/isotp.h"
#include "mblink/uds_services.h"
#include "mblink/obd2.h"
#include "mblink/j1979da.h"

#include <string.h>

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

    if (mblink_obd2_pid_definition_count() != 235U) {
        fputs("MBLINK did not inherit the completed LINK J1979 catalogue\n", stderr);
        return 1;
    }
    if (mblink_obd2_parameter_identifier_namespace_count(0x01U) != 256U ||
        mblink_obd2_parameter_identifier_namespace_count(0x05U) != 256U ||
        mblink_obd2_parameter_identifier_namespace_count(0x06U) != 256U ||
        mblink_obd2_parameter_identifier_namespace_count(0x09U) != 256U ||
        strcmp(mblink_obd2_obdonuds_revision(), "J1979-2_202604") != 0) {
        fputs("MBLINK did not inherit complete parameterized OBD namespaces\n", stderr);
        return 1;
    }
    {
        const MblinkElm327ProtocolDefinition *legacy =
            mblink_elm327_protocol_definition(MBLINK_ELM327_PROTOCOL_SAE_J1850_PWM);
        char protocol_command[8];
        if (mblink_elm327_protocol_definition_count() != 13U ||
            legacy == NULL || !legacy->classic_j1979_obd ||
            mblink_elm327_build_set_protocol_command(
                MBLINK_ELM327_PROTOCOL_ISO_9141_2,
                protocol_command, sizeof(protocol_command)) != MBLINK_ELM327_RESULT_OK ||
            strcmp(protocol_command, "ATSP3") != 0) {
            fputs("MBLINK did not inherit the legacy OBD transport model\n", stderr);
            return 1;
        }
    }
    if (strcmp(mblink_j1979_revision(), "J1979_202505") != 0 ||
        strcmp(mblink_j1979da_revision(), "J1979DA_202607") != 0 ||
        strcmp(mblink_j1979da_public_semantics_revision(),
               "J1979DA_201110+verified-public-updates") != 0) {
        fputs("MBLINK J1979/J1979-DA revision boundary is wrong\n", stderr);
        return 1;
    }
    {
        char service05[16];
        const MblinkJ1979UnitScaling *scaling =
            mblink_j1979_mode06_uasid_definition(0x0AU);
        if (mblink_j1979_build_mode05_request(
                0x01U, 0x01U, service05, sizeof(service05)) !=
                MBLINK_OBD2_RESULT_OK ||
            strcmp(service05, "050101") != 0 ||
            scaling == NULL ||
            scaling->scale < 0.1219 || scaling->scale > 0.1221 ||
            mblink_j1979_mode06_mid_classification(0x11U) !=
                MBLINK_J1979_IDENTIFIER_STANDARD) {
            fputs("MBLINK did not inherit corrected J1979 Mode 05/06 semantics\n", stderr);
            return 1;
        }
    }

    if (mblink_dtc_catalogue_definition_count() != 9533U ||
        strcmp(mblink_dtc_range_model_revision(), "J2012_202509") != 0 ||
        strcmp(mblink_dtc_catalogue_audit_revision(), "J2012DA_202607") != 0) {
        fputs("MBLINK did not inherit the audited LINK J2012 catalogue\n", stderr);
        return 1;
    }
    {
        char command[16];
        uint16_t did = 0U;
        if (mblink_obd2_obdonuds_pid_to_did(UINT16_C(0x0100), &did) !=
                MBLINK_OBD2_RESULT_OK ||
            did != UINT16_C(0xf500) ||
            mblink_obd2_build_obdonuds_pid_request(
                UINT16_C(0x0100), command, sizeof(command)) !=
                MBLINK_OBD2_RESULT_OK ||
            strcmp(command, "22F500") != 0) {
            fputs("MBLINK J1979-2 OBDonUDS facade is incomplete\n", stderr);
            return 1;
        }
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
