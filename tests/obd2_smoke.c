// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/elm327.h"
#include "mblink/obd2.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static MblinkElm327Response parse_response(const char *command, const char *wire)
{
    MblinkElm327Parser parser;
    MblinkElm327Response response;
    size_t consumed = 0U;
    MblinkElm327Result result;

    memset(&response, 0, sizeof(response));
    result = mblink_elm327_parser_begin(&parser, command);
    check(result == MBLINK_ELM327_RESULT_OK, "ELM parser begins");
    result = mblink_elm327_parser_feed(
        &parser, (const uint8_t *)wire, strlen(wire), &consumed);
    check(result == MBLINK_ELM327_RESULT_OK, "ELM parser reaches prompt");
    result = mblink_elm327_parser_finish(&parser, &response);
    check(result == MBLINK_ELM327_RESULT_OK, "ELM response normalises");
    return response;
}

static bool near_value(double value, double expected, double tolerance)
{
    double difference = value - expected;
    if (difference < 0.0) {
        difference = -difference;
    }
    return difference <= tolerance;
}

static void test_request_builders(void)
{
    char command[16];
    MblinkObd2ClearAuthorization authorization =
        MBLINK_OBD2_CLEAR_AUTHORIZATION_INIT;

    check(mblink_obd2_build_live_pid_request(
              0x0cU, command, sizeof(command)) == MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "010C") == 0,
          "live PID command");
    check(mblink_obd2_build_freeze_pid_request(
              0x0cU, 0x00U, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "020C00") == 0,
          "freeze-frame PID command");
    check(mblink_obd2_build_supported_pid_request(
              0x20U, command, sizeof(command)) == MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "0120") == 0,
          "supported PID block command");
    check(mblink_obd2_build_supported_pid_request(
              0x21U, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_INVALID_ARGUMENT,
          "misaligned supported PID block rejected");
    check(mblink_obd2_build_vin_request(command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "0902") == 0,
          "VIN command");
    check(mblink_obd2_build_dtc_request(
              MBLINK_OBD2_DTC_STORED, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "03") == 0,
          "stored DTC command");
    check(mblink_obd2_build_dtc_request(
              MBLINK_OBD2_DTC_PENDING, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "07") == 0,
          "pending DTC command");
    check(mblink_obd2_build_dtc_request(
              MBLINK_OBD2_DTC_PERMANENT, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "0A") == 0,
          "permanent DTC command");

    strcpy(command, "sentinel");
    check(mblink_obd2_build_clear_dtc_request(
              &authorization, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_NOT_AUTHORIZED &&
              command[0] == '\0',
          "clear DTC command is gated");
    authorization.confirmed = true;
    check(mblink_obd2_build_clear_dtc_request(
              &authorization, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_NOT_AUTHORIZED,
          "clear requires readiness-reset acknowledgement");
    authorization.acknowledge_readiness_reset = true;
    check(mblink_obd2_build_clear_dtc_request(
              &authorization, command, sizeof(command)) ==
              MBLINK_OBD2_RESULT_OK &&
              strcmp(command, "04") == 0,
          "clear command requires both acknowledgements");
}

static void test_supported_pid_discovery(void)
{
    MblinkObd2PidSet set;
    MblinkElm327Response response;
    bool has_more = false;

    mblink_obd2_pid_set_clear(&set);
    response = parse_response("0100", "0100\r410080000001\r>");
    check(mblink_obd2_accept_supported_pids(
              &response, 0x00U, &set, &has_more) == MBLINK_OBD2_RESULT_OK,
          "first supported PID block decodes");
    check(mblink_obd2_pid_set_contains(&set, 0x01U),
          "PID 01 marked supported");
    check(mblink_obd2_pid_set_contains(&set, 0x20U),
          "continuation PID marked supported");
    check(has_more, "continuation block requested");

    response = parse_response("0120", "412080000000\r>");
    check(mblink_obd2_accept_supported_pids(
              &response, 0x20U, &set, &has_more) == MBLINK_OBD2_RESULT_OK,
          "second supported PID block decodes");
    check(mblink_obd2_pid_set_contains(&set, 0x21U),
          "PID 21 marked supported");
    check(!has_more, "enumeration stops without continuation bit");
}

static void test_live_pid_decoding(void)
{
    struct {
        uint8_t pid;
        const char *command;
        const char *wire;
        double expected;
        double tolerance;
        MblinkObd2Unit unit;
    } cases[] = {
        {0x04U, "0104", "410480\r>", 50.196, 0.01, MBLINK_OBD2_UNIT_PERCENT},
        {0x05U, "0105", "41057B\r>", 83.0, 0.001, MBLINK_OBD2_UNIT_CELSIUS},
        {0x0bU, "010B", "410B64\r>", 100.0, 0.001, MBLINK_OBD2_UNIT_KPA},
        {0x0cU, "010C", "410C1AF8\r>", 1726.0, 0.001, MBLINK_OBD2_UNIT_RPM},
        {0x0dU, "010D", "410D64\r>", 100.0, 0.001, MBLINK_OBD2_UNIT_KMH},
        {0x0fU, "010F", "410F50\r>", 40.0, 0.001, MBLINK_OBD2_UNIT_CELSIUS},
        {0x10U, "0110", "41101388\r>", 50.0, 0.001,
         MBLINK_OBD2_UNIT_GRAMS_PER_SECOND},
        {0x11U, "0111", "411180\r>", 50.196, 0.01, MBLINK_OBD2_UNIT_PERCENT}
    };
    size_t index;

    for (index = 0U; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        MblinkElm327Response response =
            parse_response(cases[index].command, cases[index].wire);
        MblinkObd2Sample sample;
        check(mblink_obd2_decode_live_pid(
                  &response, cases[index].pid, &sample) ==
                  MBLINK_OBD2_RESULT_OK,
              "live PID decodes");
        check(sample.pid == cases[index].pid, "sample keeps PID");
        check(sample.unit == cases[index].unit, "sample unit matches");
        check(near_value(sample.value, cases[index].expected,
                         cases[index].tolerance),
              "sample formula matches");
    }
}

static void test_freeze_frame_and_readiness(void)
{
    MblinkElm327Response response;
    MblinkObd2Sample sample;
    MblinkObd2Readiness readiness;

    response = parse_response("020C00", "420C001AF8\r>");
    check(mblink_obd2_decode_freeze_pid(
              &response, 0x0cU, 0x00U, &sample) == MBLINK_OBD2_RESULT_OK,
          "freeze-frame RPM decodes");
    check(near_value(sample.value, 1726.0, 0.001),
          "freeze-frame formula matches live formula");

    response = parse_response("0101", "410181070000\r>");
    check(mblink_obd2_decode_readiness(
              &response, &readiness) == MBLINK_OBD2_RESULT_OK,
          "readiness decodes");
    check(readiness.mil_on, "MIL bit decodes");
    check(readiness.confirmed_dtc_count == 1U, "DTC count decodes");
    check(!readiness.compression_ignition, "spark ignition bit decodes");
    check(readiness.continuous_supported == 0x07U,
          "continuous support mask decodes");
    check(readiness.continuous_incomplete == 0x00U,
          "continuous completion mask decodes");

    response = parse_response("0101", "4101000F8040\r>");
    check(mblink_obd2_decode_readiness(
              &response, &readiness) == MBLINK_OBD2_RESULT_OK,
          "compression readiness decodes");
    check(readiness.compression_ignition,
          "compression ignition bit decodes");
    check(readiness.noncontinuous_supported == 0x80U &&
              readiness.noncontinuous_incomplete == 0x40U,
          "compression monitor masks retained");
}

static void test_vin(void)
{
    MblinkElm327Response response = parse_response(
        "0902",
        "4902015744443230373030303030303030303030\r>");
    char vin[MBLINK_OBD2_VIN_LENGTH + 1U];

    check(mblink_obd2_decode_vin(&response, vin) == MBLINK_OBD2_RESULT_OK,
          "VIN decodes");
    check(strcmp(vin, "WDD20700000000000") == 0, "VIN text matches");
}

static void test_dtcs(void)
{
    MblinkElm327Response response;
    MblinkObd2DtcList list;
    char code[MBLINK_OBD2_DTC_TEXT_LENGTH];

    check(mblink_obd2_decode_dtc_pair(
              0x01U, 0x33U, code) == MBLINK_OBD2_RESULT_OK &&
              strcmp(code, "P0133") == 0,
          "raw DTC pair decodes");

    response = parse_response("03", "430133C1230000\r>");
    check(mblink_obd2_decode_dtcs(
              &response, MBLINK_OBD2_DTC_STORED, &list) ==
              MBLINK_OBD2_RESULT_OK,
          "stored DTC response decodes");
    check(list.count == 2U, "stored DTC count");
    check(strcmp(list.entries[0].code, "P0133") == 0,
          "first stored DTC");
    check(strcmp(list.entries[1].code, "U0123") == 0,
          "second stored DTC");

    response = parse_response("07", "470200\r>");
    check(mblink_obd2_decode_dtcs(
              &response, MBLINK_OBD2_DTC_PENDING, &list) ==
              MBLINK_OBD2_RESULT_OK &&
              list.count == 1U &&
              strcmp(list.entries[0].code, "P0200") == 0,
          "pending DTC response decodes");

    response = parse_response("0A", "4A0300\r>");
    check(mblink_obd2_decode_dtcs(
              &response, MBLINK_OBD2_DTC_PERMANENT, &list) ==
              MBLINK_OBD2_RESULT_OK &&
              list.count == 1U &&
              strcmp(list.entries[0].code, "P0300") == 0,
          "permanent DTC response decodes");
}

static void test_malformed_and_elm_errors(void)
{
    MblinkElm327Response response;
    MblinkObd2Sample sample;

    response = parse_response("010C", "410CXYZ\r>");
    check(mblink_obd2_decode_live_pid(
              &response, 0x0cU, &sample) ==
              MBLINK_OBD2_RESULT_MALFORMED_RESPONSE,
          "non-hex payload rejected");

    memset(&response, 0, sizeof(response));
    response.result = MBLINK_ELM327_RESULT_NO_DATA;
    check(mblink_obd2_decode_live_pid(
              &response, 0x0cU, &sample) == MBLINK_OBD2_RESULT_ELM_ERROR,
          "ELM error is not reinterpreted as OBD data");
}

int main(void)
{
    test_request_builders();
    test_supported_pid_discovery();
    test_live_pid_decoding();
    test_freeze_frame_and_readiness();
    test_vin();
    test_dtcs();
    test_malformed_and_elm_errors();

    if (failures != 0) {
        fprintf(stderr, "%d OBD-II smoke test(s) failed\n", failures);
        return 1;
    }
    puts("OBD-II smoke tests passed");
    return 0;
}
