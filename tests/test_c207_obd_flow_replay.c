// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file test_c207_obd_flow_replay.c
 * @brief Replay the standard OBD front half of the captured C207 session.
 *
 * The PID-support replies below are copied verbatim from the 2026-08-27
 * vehicle capture.  The later VIN/DTC replies are deliberately NO DATA
 * controls because those standard-mode replies were not captured in that run.
 * The test's purpose is to prove sequencing: standard OBD fault inventory must
 * complete before the Mercedes manufacturer extension is entered.
 */
#include "link/diagnostic_flow.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; \
} } while (0)

static LinkElm327Response ok_response(const char *text, bool ok_seen)
{
    LinkElm327Response response;
    size_t length = text != NULL ? strlen(text) : 0U;

    memset(&response, 0, sizeof(response));
    response.result = LINK_ELM327_RESULT_OK;
    response.ok_seen = ok_seen;
    if (length >= sizeof(response.text)) length = sizeof(response.text) - 1U;
    if (length != 0U) {
        memcpy(response.text, text, length);
        response.text[length] = '\0';
        response.length = length;
    }
    return response;
}

static LinkElm327Response no_data_response(void)
{
    LinkElm327Response response;
    memset(&response, 0, sizeof(response));
    response.result = LINK_ELM327_RESULT_NO_DATA;
    return response;
}

static int complete_captured_initialization(LinkDiagnosticFlow *flow)
{
    static const char *commands[] = {
        "ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0", "ATI"
    };
    size_t index;

    for (index = 0U; index < sizeof(commands) / sizeof(commands[0]); ++index) {
        LinkDiagnosticFlowAction action;
        LinkDiagnosticFlowEvent event;
        LinkElm327Response response;

        CHECK(link_diagnostic_flow_next_action(flow, 100U, &action) ==
              LINK_DIAGNOSTIC_FLOW_RESULT_OK);
        CHECK(action.kind == LINK_DIAGNOSTIC_FLOW_ACTION_SEND_COMMAND);
        CHECK(strcmp(action.command, commands[index]) == 0);

        if (index == 0U || index + 1U == sizeof(commands) / sizeof(commands[0]))
            response = ok_response("ELM327 v2.3", false);
        else
            response = ok_response("", true);

        CHECK(link_diagnostic_flow_accept_response(
                  flow, &response, 100U, &event) ==
              LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    }
    return 0;
}

int main(void)
{
    LinkDiagnosticFlow flow;
    LinkDiagnosticFlowConfig config = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
    LinkDiagnosticFlowAction action;
    LinkDiagnosticFlowEvent event;
    LinkElm327Response response;

    config.manufacturer_extension_after_standard_dtcs = true;
    config.restore_adapter_after_manufacturer_extension = true;
    config.query_timeout_ms = UINT64_C(8000);

    CHECK(link_diagnostic_flow_init(&flow, &config) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(link_diagnostic_flow_start(&flow) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(complete_captured_initialization(&flow) == 0);
    CHECK(strcmp(link_diagnostic_flow_adapter_identifier(&flow),
                 "ELM327 v2.3") == 0);

    CHECK(link_diagnostic_flow_next_action(&flow, 500U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0100") == 0);
    CHECK(action.timeout_ms == UINT64_C(8000));
    response = ok_response(
        "410098180001\n"
        "4100983BA013", false);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 500U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);

    CHECK(link_diagnostic_flow_next_action(&flow, 600U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0120") == 0);
    response = ok_response(
        "4120B003A005\n"
        "412080018001", false);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 600U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);

    CHECK(link_diagnostic_flow_next_action(&flow, 700U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0140") == 0);
    response = ok_response(
        "41404CD80000\n"
        "414040800000", false);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 700U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.kind == LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE);
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_READING_STANDARD_VIN);

    /*
     * The old MBLINK path jumped to the Mercedes extension here.  Current
     * behaviour must attempt standard VIN and all three standard DTC modes
     * first.  NO DATA is used only because these replies were absent from the
     * captured session, not because the real car is assumed not to support them.
     */
    CHECK(link_diagnostic_flow_next_action(&flow, 800U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0902") == 0);
    response = no_data_response();
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 800U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_STORED_DTCS);

    CHECK(link_diagnostic_flow_next_action(&flow, 900U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "03") == 0);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 900U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.kind == LINK_DIAGNOSTIC_FLOW_EVENT_DTC_LIST);
    CHECK(event.dtc_kind == LINK_OBD2_DTC_STORED);

    CHECK(link_diagnostic_flow_next_action(&flow, 1000U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "07") == 0);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 1000U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.dtc_kind == LINK_OBD2_DTC_PENDING);

    CHECK(link_diagnostic_flow_next_action(&flow, 1100U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0A") == 0);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 1100U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.dtc_kind == LINK_OBD2_DTC_PERMANENT);
    CHECK(flow.standard_dtc_inventory_complete);
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_MANUFACTURER_EXTENSION);

    CHECK(link_diagnostic_flow_next_action(&flow, 1200U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(action.kind ==
          LINK_DIAGNOSTIC_FLOW_ACTION_MANUFACTURER_EXTENSION);

    puts("Captured C207 OBD-flow replay tests passed");
    return 0;
}
