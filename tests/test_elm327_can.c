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
    MblinkElm327Response response = ok_response();
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

    check(mblink_elm327_can_channel_accept(&state, &response) ==
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

    elm_error = text_response("?");
    elm_error.result = MBLINK_ELM327_RESULT_ADAPTER_ERROR;
    check(mblink_elm327_can_channel_accept(&state, &elm_error) ==
              MBLINK_ELM327_CAN_RESULT_ELM_ERROR,
          "ELM error response not reported");
}

int main(void)
{
    test_channel_configuration_11_bit();
    test_channel_configuration_29_bit();
    test_channel_validation_and_failure();

    if (failures != 0) {
        fprintf(stderr, "%d ELM327 CAN test(s) failed\n", failures);
        return EXIT_FAILURE;
    }

    puts("All ELM327 CAN tests passed.");
    return EXIT_SUCCESS;
}
