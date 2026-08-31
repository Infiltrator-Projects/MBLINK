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
     * Exact richer 7E8 bitmap from the field capture advertises these seven
     * useful scalar PIDs that older MBLINK builds discovered but did not expose.
     * Keep them as product regression evidence so future catalogue changes
     * cannot silently lose them.
     */
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x1f)));
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x21)));
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x24)));
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x30)));
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x31)));
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x3e)));
    CHECK(link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x4d)));

    /*
     * The exact captured 0x40 support page does NOT advertise PID 0x60.
     * Re-running this evidence against the expanded 2026 J1979 catalogue must
     * therefore stop at the 0x40 page for this vehicle. Newer generic PIDs
     * such as 0x69/0x70/0x7A/0xAA are globally understood by LINK, but this
     * C207 capture does not claim to implement them.
     */
    CHECK(!link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x60)));
    CHECK(!link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x69)));
    CHECK(!link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x70)));
    CHECK(!link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0x7a)));
    CHECK(!link_obd2_pid_set_contains(&flow.supported_pids, UINT8_C(0xaa)));
    CHECK(link_obd2_pid_definition(UINT8_C(0x01), UINT8_C(0x69)) != NULL);
    CHECK(link_obd2_pid_definition(UINT8_C(0x01), UINT8_C(0x70)) != NULL);
    CHECK(link_obd2_pid_definition(UINT8_C(0x01), UINT8_C(0x7a)) != NULL);
    CHECK(link_obd2_pid_definition(UINT8_C(0x01), UINT8_C(0xaa)) != NULL);

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
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_READING_READINESS);

    /*
     * The captured drive did not preserve a standalone 0101 readiness reply.
     * Keep that absence explicit rather than inventing one: the optional
     * context step must tolerate NO DATA, mark readiness unavailable and move
     * on. With no stored SAE DTCs, Mode 02 is correctly not requested.
     */
    CHECK(link_diagnostic_flow_next_action(&flow, 1150U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(strcmp(action.command, "0101") == 0);
    response = no_data_response();
    CHECK(link_diagnostic_flow_accept_response(
              &flow, &response, 1150U, &event) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(flow.readiness_attempted);
    CHECK(!flow.readiness_available);
    CHECK(flow.standard_diagnostic_context_complete);
    CHECK(!flow.freeze_frame_requested);
    CHECK(flow.stage == LINK_DIAGNOSTIC_FLOW_MANUFACTURER_EXTENSION);

    CHECK(link_diagnostic_flow_next_action(&flow, 1200U, &action) ==
          LINK_DIAGNOSTIC_FLOW_RESULT_OK);
    CHECK(action.kind ==
          LINK_DIAGNOSTIC_FLOW_ACTION_MANUFACTURER_EXTENSION);

    /*
     * The 2026-08-30 drive capture proved that the functional live request is
     * answered by both 7E8 and 7E9. Preserve both values and addresses so the
     * product can present engine and transmission-candidate module pages.
     */
    {
        LinkObd2ResponderSampleList responders;
        response = ok_response(
            "7E804410C0CF9\n"
            "7E904410C0CFC", false);
        CHECK(link_obd2_decode_live_pid_responders(
                  &response, UINT8_C(0x0c), &responders) ==
              LINK_OBD2_RESULT_OK);
        CHECK(responders.count == 2U);
        CHECK(responders.samples[0].responder_id == UINT32_C(0x7e8));
        CHECK(responders.samples[0].sample.value == 830.25);
        CHECK(responders.samples[1].responder_id == UINT32_C(0x7e9));
        CHECK(responders.samples[1].sample.value == 831.0);

        /* Same capture, same PID, independently different ECU voltage. */
        response = ok_response(
            "7E80441423827\n"
            "7E90441423778", false);
        CHECK(link_obd2_decode_live_pid_responders(
                  &response, UINT8_C(0x42), &responders) ==
              LINK_OBD2_RESULT_OK);
        CHECK(responders.count == 2U);
        CHECK(responders.samples[0].responder_id == UINT32_C(0x7e8));
        CHECK(responders.samples[0].sample.value == 14.375);
        CHECK(responders.samples[1].responder_id == UINT32_C(0x7e9));
        CHECK(responders.samples[1].sample.value == 14.2);
    }

    /*
     * Replay exact scalar samples from MBLINK-session-20260829-140223.
     * These are useful because they prove that the apparently high PID 0x11
     * reading is the throttle valve, not accelerator demand: the two pedal
     * channels remain near 5-6% while the valve is near 88%.
     */
    {
        LinkObd2Sample sample;

        response = ok_response("4111E0", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x11), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value > 87.84 && sample.value < 87.85);

        response = ok_response("41490E", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x49), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value > 5.49 && sample.value < 5.50);

        response = ok_response("414A0F", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x4a), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value > 5.88 && sample.value < 5.89);

        response = ok_response("412F45", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x2f), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value > 27.05 && sample.value < 27.07);

        response = ok_response("414638", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x46), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value == 16.0);

        response = ok_response("4123000A", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x23), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value == 100.0);

        response = ok_response("41230028", false);
        CHECK(link_obd2_decode_live_pid(
                  &response, UINT8_C(0x23), &sample) ==
              LINK_OBD2_RESULT_OK);
        CHECK(sample.value == 400.0);
    }

    puts("Captured C207 OBD-flow replay tests passed");
    return 0;
}
