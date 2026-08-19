// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/elm327_can.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "mblink-elm327-can-test: %s\n", message);
        failures++;
    }
}

static MblinkElm327Response ok_response(void)
{
    MblinkElm327Response response;
    memset(&response, 0, sizeof(response));
    response.result = MBLINK_ELM327_RESULT_OK;
    response.ok_seen = true;
    response.prompt_seen = true;
    return response;
}

static MblinkElm327Response text_response(const char *text)
{
    MblinkElm327Response response;
    memset(&response, 0, sizeof(response));
    response.result = MBLINK_ELM327_RESULT_OK;
    response.prompt_seen = true;
    if (text != NULL) {
        size_t length = strlen(text);
        if (length >= sizeof(response.text)) length = sizeof(response.text) - 1U;
        memcpy(response.text, text, length);
        response.text[length] = '\0';
        response.length = length;
        response.line_count = 1U;
        for (size_t index = 0U; index < length; ++index) {
            if (response.text[index] == '\n') response.line_count++;
        }
    }
    return response;
}

static void test_channel_configuration_11_bit(void)
{
    MblinkElm327CanChannelConfig config = { 0x700U, 0x708U, false };
    MblinkElm327CanChannelState state;
    MblinkElm327Response response = ok_response();
    char command[32];
    static const char *expected[] = {
        "ATSH700", "ATCRA708", "ATCAF1", "ATCFC1"
    };

    check(mblink_elm327_can_channel_config_is_valid(&config),
          "valid 11-bit channel rejected");
    check(mblink_elm327_can_channel_begin(&state, &config) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "11-bit channel begin failed");

    for (size_t index = 0U; index < 4U; ++index) {
        check(mblink_elm327_can_channel_command(
                  &state, command, sizeof(command)) ==
                  MBLINK_ELM327_CAN_RESULT_OK,
              "11-bit stage command failed");
        check(strcmp(command, expected[index]) == 0,
              "11-bit stage command mismatch");
        check(mblink_elm327_can_channel_accept(&state, &response) ==
                  MBLINK_ELM327_CAN_RESULT_OK,
              "11-bit stage response rejected");
    }

    check(state.stage == MBLINK_ELM327_CAN_STAGE_COMPLETE,
          "11-bit channel did not complete");
    check(mblink_elm327_can_channel_command(
              &state, command, sizeof(command)) ==
              MBLINK_ELM327_CAN_RESULT_FAILED_STATE,
          "complete channel unexpectedly returned another command");
}

static void test_channel_configuration_29_bit(void)
{
    MblinkElm327CanChannelConfig config = {
        UINT32_C(0x18daf110), UINT32_C(0x18da10f1), true
    };
    MblinkElm327CanChannelState state;
    char command[32];

    check(mblink_elm327_can_channel_config_is_valid(&config),
          "valid 29-bit channel rejected");
    check(mblink_elm327_can_channel_begin(&state, &config) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "29-bit channel begin failed");
    check(mblink_elm327_can_channel_command(
              &state, command, sizeof(command)) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "29-bit header command failed");
    check(strcmp(command, "ATSH18DAF110") == 0,
          "29-bit header command mismatch");

    check(mblink_elm327_can_channel_accept(&state, &ok_response()) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "29-bit header accept failed");
    check(mblink_elm327_can_channel_command(
              &state, command, sizeof(command)) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "29-bit receive-filter command failed");
    check(strcmp(command, "ATCRA18DA10F1") == 0,
          "29-bit receive-filter command mismatch");
}

static void test_channel_validation_and_failure(void)
{
    MblinkElm327CanChannelConfig invalid11 = { 0x800U, 0x708U, false };
    MblinkElm327CanChannelConfig invalid29 = {
        UINT32_C(0x20000000), UINT32_C(0x18da10f1), true
    };
    MblinkElm327CanChannelConfig valid = { 0x700U, 0x708U, false };
    MblinkElm327CanChannelState state;
    MblinkElm327Response no_ok = text_response("ELM327");
    MblinkElm327Response elm_error;
    char small[4];

    check(!mblink_elm327_can_channel_config_is_valid(&invalid11),
          "out-of-range 11-bit identifier accepted");
    check(!mblink_elm327_can_channel_config_is_valid(&invalid29),
          "out-of-range 29-bit identifier accepted");
    check(mblink_elm327_can_channel_begin(&state, &invalid11) ==
              MBLINK_ELM327_CAN_RESULT_INVALID_ARGUMENT,
          "invalid channel began successfully");

    check(mblink_elm327_can_channel_begin(&state, &valid) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "failure-test channel begin failed");
    check(mblink_elm327_can_channel_command(&state, small, sizeof(small)) ==
              MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL,
          "small channel command buffer not rejected");
    check(small[0] == '\0', "failed channel command did not clear output");
    check(mblink_elm327_can_channel_accept(&state, &no_ok) ==
              MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE,
          "configuration response without OK accepted");
    check(state.stage == MBLINK_ELM327_CAN_STAGE_FAILED,
          "malformed response did not latch failed state");

    check(mblink_elm327_can_channel_begin(&state, &valid) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "ELM-error channel begin failed");
    memset(&elm_error, 0, sizeof(elm_error));
    elm_error.result = MBLINK_ELM327_RESULT_CAN_ERROR;
    check(mblink_elm327_can_channel_accept(&state, &elm_error) ==
              MBLINK_ELM327_CAN_RESULT_ELM_ERROR,
          "ELM CAN error not surfaced");
    check(state.elm_failure == MBLINK_ELM327_RESULT_CAN_ERROR,
          "underlying ELM error not retained");
}

static void test_pdu_command(void)
{
    const uint8_t request[] = { 0x22U, 0xf1U, 0x90U };
    uint8_t maximum[MBLINK_ELM327_CAN_MAX_REQUEST_PDU];
    uint8_t too_large[MBLINK_ELM327_CAN_MAX_REQUEST_PDU + 1U];
    char command[MBLINK_ELM327_MAX_COMMAND];
    char small[5];
    size_t written = 99U;

    memset(maximum, 0xab, sizeof(maximum));
    memset(too_large, 0xcd, sizeof(too_large));

    check(mblink_elm327_can_build_pdu_command(
              request, sizeof(request), command, sizeof(command), &written) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "UDS PDU command build failed");
    check(written == 6U && strcmp(command, "22F190") == 0,
          "UDS PDU command mismatch");

    written = 99U;
    check(mblink_elm327_can_build_pdu_command(
              request, sizeof(request), small, sizeof(small), &written) ==
              MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL,
          "small PDU command output not rejected");
    check(written == 0U && small[0] == '\0',
          "failed PDU build was not transactional");

    check(mblink_elm327_can_build_pdu_command(
              maximum, sizeof(maximum), command, sizeof(command), &written) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "maximum PDU command rejected");
    check(written == MBLINK_ELM327_CAN_MAX_REQUEST_PDU * 2U,
          "maximum PDU command length mismatch");

    check(mblink_elm327_can_build_pdu_command(
              too_large, sizeof(too_large), command, sizeof(command), &written) ==
              MBLINK_ELM327_CAN_RESULT_PDU_TOO_LARGE,
          "oversized PDU command accepted");
}

static void test_pdu_decode_single(void)
{
    MblinkElm327Response response = text_response("62 F1 90 57 44 44 31");
    uint8_t pdu[32];
    size_t length = 0U;
    const uint8_t expected[] = { 0x62U, 0xf1U, 0x90U, 0x57U,
                                 0x44U, 0x44U, 0x31U };

    memset(pdu, 0xee, sizeof(pdu));
    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "single-line PDU decode failed");
    check(length == sizeof(expected) &&
          memcmp(pdu, expected, sizeof(expected)) == 0,
          "single-line PDU bytes mismatch");
}

static void test_pdu_decode_indexed(void)
{
    MblinkElm327Response response = text_response(
        "00A\n0: 62 F1 90 41 42 43\n1: 44 45 46 47");
    uint8_t pdu[32];
    size_t length = 0U;
    const uint8_t expected[] = {
        0x62U, 0xf1U, 0x90U, 0x41U, 0x42U,
        0x43U, 0x44U, 0x45U, 0x46U, 0x47U
    };

    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_OK,
          "indexed PDU decode failed");
    check(length == sizeof(expected) &&
          memcmp(pdu, expected, sizeof(expected)) == 0,
          "indexed PDU bytes mismatch");

    response = text_response(
        "009\n0: 62 F1 90 41 42 43\n1: 44 45 46 47");
    length = 77U;
    memset(pdu, 0xa5, sizeof(pdu));
    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE,
          "indexed declared-length mismatch accepted");
    check(length == 0U && pdu[0] == 0xa5U,
          "failed indexed decode modified caller output");

    response = text_response(
        "00A\n0: 62 F1 90 41 42 43\n2: 44 45 46 47");
    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE,
          "wrong indexed sequence accepted");
}

static void test_pdu_decode_failures(void)
{
    MblinkElm327Response response = text_response("62F190\n62F191");
    uint8_t pdu[2] = { 0xaaU, 0xbbU };
    size_t length = 88U;

    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_UNEXPECTED_RESPONSE,
          "multiple plain PDU lines accepted");
    check(length == 0U && pdu[0] == 0xaaU,
          "unexpected-response path modified caller output");

    response = text_response("62F190");
    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL,
          "small PDU decode output not rejected");
    check(length == 0U && pdu[0] == 0xaaU,
          "small-buffer decode modified caller output");

    response.result = MBLINK_ELM327_RESULT_NO_DATA;
    check(mblink_elm327_can_decode_pdu(
              &response, pdu, sizeof(pdu), &length) ==
              MBLINK_ELM327_CAN_RESULT_ELM_ERROR,
          "ELM error response was not surfaced");
}

int main(void)
{
    test_channel_configuration_11_bit();
    test_channel_configuration_29_bit();
    test_channel_validation_and_failure();
    test_pdu_command();
    test_pdu_decode_single();
    test_pdu_decode_indexed();
    test_pdu_decode_failures();

    if (failures != 0) {
        fprintf(stderr, "%d ELM CAN test(s) failed\n", failures);
        return EXIT_FAILURE;
    }
    puts("ELM CAN tests passed");
    return EXIT_SUCCESS;
}
