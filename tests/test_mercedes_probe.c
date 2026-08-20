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

static int test_successful_read_only_probe(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        mblink_mercedes_profile_find_endpoint(
            profile, "c207-om651-engine-eobd-11bit");
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response reply = make_response(
        MBLINK_ELM327_RESULT_OK, "7E00", false);
    char command[16];
    size_t written = 0U;

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(mblink_mercedes_ecu_probe_stage_name(probe.stage),
                 "configure-channel") == 0);
    CHECK(advance_configuration(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "3E00") == 0);
    CHECK(written == 4U);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    CHECK(probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE);
    CHECK(strcmp(mblink_mercedes_ecu_probe_result_name(
                     MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE),
                 "complete") == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE);
    return 0;
}

static int test_failure_provenance(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        &mblink_mercedes_c207_om651_profile()->endpoints[0];
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response elm_error = make_response(
        MBLINK_ELM327_RESULT_NO_DATA, "", false);
    MblinkElm327Response malformed = make_response(
        MBLINK_ELM327_RESULT_OK, "GG", false);
    MblinkElm327Response negative = make_response(
        MBLINK_ELM327_RESULT_OK, "7F3E11", false);

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
    return 0;
}

static int test_argument_and_buffer_failures(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        &mblink_mercedes_c207_om651_profile()->endpoints[0];
    MblinkMercedesEcuEndpointDefinition invalid = *endpoint;
    MblinkMercedesEcuProbe probe;
    char buffer[4] = "bad";
    size_t written = 99U;

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
    if (test_successful_read_only_probe() != 0) return 1;
    if (test_failure_provenance() != 0) return 1;
    if (test_argument_and_buffer_failures() != 0) return 1;
    puts("Mercedes ECU probe tests passed");
    return 0;
}
