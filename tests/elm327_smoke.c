// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/elm327.h"

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

static MblinkElm327Response parse_complete(const char *command,
                                           const char *wire)
{
    MblinkElm327Parser parser;
    MblinkElm327Response response;
    size_t consumed = 0U;
    MblinkElm327Result result;

    memset(&response, 0, sizeof(response));
    result = mblink_elm327_parser_begin(&parser, command);
    check(result == MBLINK_ELM327_RESULT_OK, "parser begin");
    result = mblink_elm327_parser_feed(&parser, (const uint8_t *)wire,
                                       strlen(wire), &consumed);
    check(result == MBLINK_ELM327_RESULT_OK, "complete response finds prompt");
    check(consumed <= strlen(wire), "consumed is bounded");
    (void)mblink_elm327_parser_finish(&parser, &response);
    return response;
}

static void test_command_framing(void)
{
    uint8_t bytes[16];
    size_t written = 0U;
    MblinkElm327Result result;

    result = mblink_elm327_build_command("  atz  ", bytes, sizeof(bytes), &written);
    check(result == MBLINK_ELM327_RESULT_OK, "command framing succeeds");
    check(written == 4U, "command framing writes exact size");
    check(memcmp(bytes, "atz\r", 4U) == 0, "command framing trims and appends CR");

    result = mblink_elm327_build_command("ATZ\nATI", bytes, sizeof(bytes), &written);
    check(result == MBLINK_ELM327_RESULT_INVALID_ARGUMENT,
          "embedded newline is rejected");
}

static void test_fragmented_response(void)
{
    MblinkElm327Parser parser;
    MblinkElm327Response response;
    size_t consumed = 0U;
    MblinkElm327Result result;

    check(mblink_elm327_parser_begin(&parser, "010C") == MBLINK_ELM327_RESULT_OK,
          "fragment parser begin");
    result = mblink_elm327_parser_feed(&parser, (const uint8_t *)"010C\r41 0C ",
                                       strlen("010C\r41 0C "), &consumed);
    check(result == MBLINK_ELM327_RESULT_MORE_DATA, "fragment needs more data");
    result = mblink_elm327_parser_feed(&parser, (const uint8_t *)"1A F8\r>tail",
                                       strlen("1A F8\r>tail"), &consumed);
    check(result == MBLINK_ELM327_RESULT_OK, "second fragment completes");
    check(consumed == 7U, "parser stops consuming at prompt");
    result = mblink_elm327_parser_finish(&parser, &response);
    check(result == MBLINK_ELM327_RESULT_OK, "fragmented response parses");
    check(response.echo_removed, "echo is removed");
    check(response.prompt_seen, "prompt is recorded");
    check(response.line_count == 1U, "one payload line remains");
    check(strcmp(response.text, "41 0C 1A F8") == 0, "payload is normalised");
}

static void test_status_classification(void)
{
    MblinkElm327Response response;

    response = parse_complete("0100", "0100\rSEARCHING...\rNO DATA\r>");
    check(response.result == MBLINK_ELM327_RESULT_NO_DATA, "NO DATA classified");
    check(response.searching_seen, "SEARCHING marker retained as metadata");
    check(response.echo_removed, "status response echo removed");

    response = parse_complete("ATFOO", "ATFOO\r?\r>");
    check(response.result == MBLINK_ELM327_RESULT_UNSUPPORTED_COMMAND,
          "question mark classified as unsupported command");

    response = parse_complete("0100", "UNABLE TO CONNECT\r>");
    check(response.result == MBLINK_ELM327_RESULT_UNABLE_TO_CONNECT,
          "unable-to-connect classified");

    response = parse_complete("0100", ">");
    check(response.result == MBLINK_ELM327_RESULT_MALFORMED_RESPONSE,
          "empty prompt response is malformed");
}

static void test_initialisation(void)
{
    MblinkElm327InitState state;
    MblinkElm327Response response;
    const char *expected[] = {"ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0", "ATI"};
    size_t index;

    mblink_elm327_init_begin(&state);
    for (index = 0U; index < 7U; ++index) {
        check(strcmp(mblink_elm327_init_command(&state), expected[index]) == 0,
              "initialisation command order");
        if (index == 0U) {
            response = parse_complete("ATZ", "ATZ\rELM327 v2.3\r>");
        } else if (index == 6U) {
            response = parse_complete("ATI", "ELM327 v2.3\r>");
        } else {
            response = parse_complete(expected[index], "OK\r>");
        }
        check(mblink_elm327_init_accept(&state, &response) == MBLINK_ELM327_RESULT_OK,
              "initialisation stage accepts response");
    }
    check(state.stage == MBLINK_ELM327_INIT_COMPLETE, "initialisation completes");
    check(strcmp(state.adapter_id, "ELM327 v2.3") == 0, "adapter identity captured");
    check(mblink_elm327_init_command(&state) == NULL, "complete state has no command");
}

static void test_initialisation_failure(void)
{
    MblinkElm327InitState state;
    MblinkElm327Response response;

    mblink_elm327_init_begin(&state);
    response = parse_complete("ATZ", "ELM327 v2.3\r>");
    check(mblink_elm327_init_accept(&state, &response) == MBLINK_ELM327_RESULT_OK,
          "reset accepted before failure test");
    response = parse_complete("ATE0", "?\r>");
    check(mblink_elm327_init_accept(&state, &response) ==
              MBLINK_ELM327_RESULT_UNSUPPORTED_COMMAND,
          "init surfaces adapter error");
    check(state.stage == MBLINK_ELM327_INIT_FAILED, "init enters failed state");
}

int main(void)
{
    test_command_framing();
    test_fragmented_response();
    test_status_classification();
    test_initialisation();
    test_initialisation_failure();

    if (failures != 0) {
        fprintf(stderr, "%d ELM327 smoke test(s) failed\n", failures);
        return 1;
    }
    puts("ELM327 smoke tests passed");
    return 0;
}
