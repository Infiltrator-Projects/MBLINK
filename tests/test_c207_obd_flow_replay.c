// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file test_c207_obd_flow_replay.c
 * @brief Replay the standard OBD front half of the captured C207 session.
 *
 * The PID-support replies below are copied from the 2026-08-27 vehicle
 * capture.  Indexed Mode 09 VIN framing and the dual-responder empty Mode 03
 * shape are derived from the 2026-08-28 C207/Vgate evidence; identifying VIN
 * digits are replaced with a synthetic fixture value. Pending DTC remains a
 * NO DATA control; permanent DTC replays the captured `7F 0A 22` response.
 * The test proves that neither real empty stored-DTC framing nor an unavailable
 * optional Mode 0A inventory aborts before the Mercedes extension.
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
     * first.  The captured VIN and stored-DTC framing are replayed below;
     * pending remains an explicit NO DATA control and permanent mode uses the
     * exact captured negative-response shape.
     */
    CHECK(link_diagnostic_flow_next_action(&flow, 800U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0902") == 0);
    response = ok_response(
        "014\n"
        "0:490201574444\n"
        "1:32303733303232\n"
        "2:46313233343536", false);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 800U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.kind == LINK_DIAGNOSTIC_FLOW_EVENT_STANDARD_VIN);
    CHECK(event.vin_available);
    CHECK(event.vin != NULL);
    CHECK(strcmp(event.vin, "WDD2073022F123456") == 0);
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_SCANNING_STORED_DTCS);

    CHECK(link_diagnostic_flow_next_action(&flow, 900U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "03") == 0);
    response = ok_response("4300\n4300", false);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 900U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.kind == LINK_DIAGNOSTIC_FLOW_EVENT_DTC_LIST);
    CHECK(event.dtc_kind == LINK_OBD2_DTC_STORED);
    CHECK(event.dtc_list != NULL && event.dtc_list->count == 0U);

    CHECK(link_diagnostic_flow_next_action(&flow, 1000U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "07") == 0);
    response = no_data_response();
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 1000U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.dtc_kind == LINK_OBD2_DTC_PENDING);

    CHECK(link_diagnostic_flow_next_action(&flow, 1100U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0A") == 0);
    response = ok_response("7F0A22", false);
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 1100U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(event.dtc_kind == LINK_OBD2_DTC_PERMANENT);
    CHECK(!event.dtc_response_available);
    CHECK(event.dtc_negative_response);
    CHECK(event.dtc_negative_response_code == UINT8_C(0x22));
    CHECK(flow.standard_dtc_inventory_complete);
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_MANUFACTURER_EXTENSION);

    CHECK(link_diagnostic_flow_next_action(&flow, 1200U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(action.kind ==
          LINK_DIAGNOSTIC_FLOW_ACTION_MANUFACTURER_EXTENSION);

    puts("Captured C207 OBD-flow replay tests passed");
    return 0;
}
