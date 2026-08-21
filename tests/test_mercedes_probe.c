// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_probe.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static MblinkElm327Response make_response(
    MblinkElm327Result result,
    const char *text,
    bool ok_seen)
{
    MblinkElm327Response response;
    size_t length = text == NULL ? 0U : strlen(text);

    memset(&response, 0, sizeof(response));
    response.result = result;
    response.ok_seen = ok_seen;
    if (length >= sizeof(response.text)) length = sizeof(response.text) - 1U;
    if (length != 0U) memcpy(response.text, text, length);
    response.text[length] = '\0';
    response.length = length;
    return response;
}

static const MblinkMercedesEcuEndpointDefinition *engine_endpoint(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    return mblink_mercedes_profile_find_endpoint(
        profile, "c207-om651-engine-eobd-11bit");
}

static int advance_configuration(MblinkMercedesEcuProbe *probe)
{
    static const char *commands[] = {
        "ATSH7E0", "ATCRA7E8", "ATCAF1", "ATCFC1"
    };
    MblinkElm327Response ok = make_response(
        MBLINK_ELM327_RESULT_OK, "OK", true);

    for (size_t index = 0U; index < 4U; ++index) {
        char command[32];
        size_t written = 0U;
        CHECK(mblink_mercedes_ecu_probe_command(
                  probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(strcmp(command, commands[index]) == 0);
        CHECK(written == strlen(commands[index]));
        CHECK(mblink_mercedes_ecu_probe_accept(probe, &ok) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    }
    CHECK(probe->stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT);
    return 0;
}

static int advance_tester_present(MblinkMercedesEcuProbe *probe)
{
    MblinkElm327Response reply = make_response(
        MBLINK_ELM327_RESULT_OK, "7E00", false);
    char command[16];
    size_t written = 0U;

    CHECK(advance_configuration(probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_command(
              probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "3E00") == 0);
    CHECK(written == 4U);
    CHECK(mblink_mercedes_ecu_probe_accept(probe, &reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe->stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN);
    return 0;
}

static int test_successful_read_only_probe_with_standard_vin(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response vin_reply = make_response(
        MBLINK_ELM327_RESULT_OK,
        "62F1905744443230373330323246313233343536",
        false);
    char command[16];
    size_t written = 0U;

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(mblink_mercedes_ecu_probe_stage_name(probe.stage),
                 "configure-channel") == 0);
    CHECK(advance_tester_present(&probe) == 0);
    CHECK(strcmp(mblink_mercedes_ecu_probe_stage_name(probe.stage),
                 "read-standard-vin") == 0);

    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "22F190") == 0);
    CHECK(written == 6U);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &vin_reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    CHECK(probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE);
    CHECK(probe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE);
    CHECK(strcmp(probe.vin, "WDD2073022F123456") == 0);
    CHECK(strcmp(mblink_mercedes_ecu_probe_vin_result_name(probe.vin_result),
                 "available") == 0);
    CHECK(strcmp(mblink_mercedes_ecu_probe_result_name(
                     MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE),
                 "complete") == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &vin_reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE);
    return 0;
}

static int test_optional_vin_does_not_invalidate_confirmed_endpoint(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response negative = make_response(
        MBLINK_ELM327_RESULT_OK, "7F2231", false);
    MblinkElm327Response no_data = make_response(
        MBLINK_ELM327_RESULT_NO_DATA, "", false);
    MblinkElm327Response short_vin = make_response(
        MBLINK_ELM327_RESULT_OK, "62F190574444323037", false);

    CHECK(endpoint != NULL);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_tester_present(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &negative) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    CHECK(probe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.vin_result ==
          MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE);
    CHECK(probe.vin_uds_result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE);
    CHECK(probe.vin_negative_response_code == 0x31U);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_tester_present(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &no_data) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    CHECK(probe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE);
    CHECK(probe.vin_elm_result == MBLINK_ELM327_RESULT_NO_DATA);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_tester_present(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &short_vin) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    CHECK(probe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE);
    CHECK(probe.vin[0] == '\0');
    return 0;
}

static int test_failure_provenance(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response elm_error = make_response(
        MBLINK_ELM327_RESULT_NO_DATA, "", false);
    MblinkElm327Response malformed = make_response(
        MBLINK_ELM327_RESULT_OK, "GG", false);
    MblinkElm327Response negative = make_response(
        MBLINK_ELM327_RESULT_OK, "7F3E11", false);

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &elm_error) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR);
    CHECK(probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED);
    CHECK(probe.elm_can_failure == MBLINK_ELM327_CAN_RESULT_ELM_ERROR);
    CHECK(probe.elm_failure == MBLINK_ELM327_RESULT_NO_DATA);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_configuration(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &malformed) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR);
    CHECK(probe.elm_can_failure ==
          MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE);
    CHECK(probe.elm_failure == MBLINK_ELM327_RESULT_OK);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_configuration(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &negative) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR);
    CHECK(probe.uds_failure == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE);
    CHECK(probe.uds_negative_response_code == 0x11U);
    CHECK(probe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED);
    return 0;
}

static int test_argument_and_buffer_failures(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuEndpointDefinition invalid;
    MblinkMercedesEcuProbe probe;
    char buffer[4] = "bad";
    size_t written = 99U;

    CHECK(endpoint != NULL);
    invalid = *endpoint;
    invalid.provenance = "";
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, &invalid) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, buffer, sizeof(buffer), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL);
    CHECK(buffer[0] == '\0');
    CHECK(written == 0U);

    invalid = *endpoint;
    invalid.address.addressing_mode = MBLINK_ISOTP_ADDRESSING_EXTENDED;
    invalid.address.tx_address_extension = 0xf1U;
    invalid.address.rx_address_extension = 0x10U;
    CHECK(mblink_mercedes_ecu_endpoint_is_valid(&invalid));
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, &invalid) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT);
    CHECK(strcmp(mblink_mercedes_ecu_probe_result_name(
                     MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT),
                 "unsupported-endpoint") == 0);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    probe.stage = (MblinkMercedesEcuProbeStage)99;
    CHECK(mblink_mercedes_ecu_probe_accept(
              &probe, &(MblinkElm327Response){0}) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE);
    return 0;
}

int main(void)
{
    if (test_successful_read_only_probe_with_standard_vin() != 0) return 1;
    if (test_optional_vin_does_not_invalidate_confirmed_endpoint() != 0) return 1;
    if (test_failure_provenance() != 0) return 1;
    if (test_argument_and_buffer_failures() != 0) return 1;
    puts("Mercedes ECU probe tests passed");
    return 0;
}
