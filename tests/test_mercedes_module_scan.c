// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_module_scan.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static MblinkElm327Response response(MblinkElm327Result result,
                           const char *text,
                           bool ok_seen)
{
    MblinkElm327Response value;
    size_t length = text != NULL ? strlen(text) : 0U;
    memset(&value, 0, sizeof(value));
    value.result = result;
    value.ok_seen = ok_seen;
    if (length >= sizeof(value.text)) length = sizeof(value.text) - 1U;
    if (length != 0U) memcpy(value.text, text, length);
    value.text[length] = '\0';
    value.length = length;
    return value;
}

static int send_ok(MblinkMercedesModuleScan *scan, const char *expected)
{
    char command[32];
    size_t written = 0U;
    MblinkElm327Response ok = response(MBLINK_ELM327_RESULT_OK, "OK", true);
    CHECK(mblink_mercedes_module_scan_command(scan, command, sizeof(command), &written) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, expected) == 0);
    CHECK(written == strlen(expected));
    CHECK(mblink_mercedes_module_scan_accept(scan, &ok) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    return 0;
}

static int accept_identity_metadata(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *spare,
    const MblinkElm327Response *software,
    const MblinkElm327Response *hardware)
{
    char command[32];
    size_t written = 0U;

    CHECK(scan->stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART);
    CHECK(mblink_mercedes_module_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F187") == 0);
    CHECK(mblink_mercedes_module_scan_accept(scan, spare) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

    CHECK(scan->stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE);
    CHECK(mblink_mercedes_module_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F188") == 0);
    CHECK(mblink_mercedes_module_scan_accept(scan, software) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

    CHECK(scan->stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE);
    CHECK(mblink_mercedes_module_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F191") == 0);
    CHECK(mblink_mercedes_module_scan_accept(scan, hardware) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    return 0;
}

int main(void)
{
    MblinkMercedesModuleScan scan;
    char command[32];
    size_t written = 0U;
    MblinkElm327Response tester = response(MBLINK_ELM327_RESULT_OK, "7E00", false);
    MblinkElm327Response no_data = response(MBLINK_ELM327_RESULT_NO_DATA, "", false);
    MblinkElm327Response dtcs = response(MBLINK_ELM327_RESULT_OK,
        "5902FF12345609ABCDEF28", false);
    MblinkElm327Response engine_identity = response(
        MBLINK_ELM327_RESULT_OK, "62F19743524433", false);
    MblinkElm327Response unknown_identity = response(
        MBLINK_ELM327_RESULT_OK, "62F197455350", false);
    MblinkElm327Response spare = response(
        MBLINK_ELM327_RESULT_OK,
        "62F18736353139303131383031", false);
    MblinkElm327Response software = response(
        MBLINK_ELM327_RESULT_OK,
        "62F18836353139303230303031", false);
    MblinkElm327Response hardware = response(
        MBLINK_ELM327_RESULT_OK,
        "62F19136353139303430303031", false);

    CHECK(mblink_mercedes_module_scan_begin(&scan) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(send_ok(&scan, "ATSP6") == 0);
    CHECK(send_ok(&scan, "ATH0") == 0);
    CHECK(send_ok(&scan, "ATCAF1") == 0);
    CHECK(send_ok(&scan, "ATCFC1") == 0);
    CHECK(send_ok(&scan, "ATST20") == 0);
    CHECK(send_ok(&scan, "ATSH7E0") == 0);
    CHECK(send_ok(&scan, "ATCRA7E8") == 0);

    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "3E00") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &tester) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.module_count == 1U);
    CHECK(scan.modules[0].tx_can_id == UINT32_C(0x7e0));
    CHECK(scan.modules[0].rx_can_id == UINT32_C(0x7e8));
    CHECK(scan.modules[0].kind == MBLINK_MERCEDES_MODULE_ENGINE);
    CHECK(scan.modules[0].tester_present_response);
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
    CHECK(mblink_mercedes_module_scan_command(
              &scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "1902FF") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.modules[0].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F197") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &engine_identity) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.modules[0].identity_available);
    CHECK(strcmp(scan.modules[0].identity, "CRD3") == 0);
    CHECK(scan.modules[0].definition != NULL);
    CHECK(strcmp(scan.modules[0].definition->key, "engine-cdi") == 0);
    CHECK(scan.modules[0].kind == MBLINK_MERCEDES_MODULE_ENGINE);
    CHECK(scan.modules[0].identification_status ==
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    CHECK(accept_identity_metadata(
              &scan, &spare, &software, &hardware) == 0);
    CHECK(scan.modules[0].spare_part_number_available);
    CHECK(strcmp(scan.modules[0].spare_part_number, "6519011801") == 0);
    CHECK(scan.modules[0].software_number_available);
    CHECK(strcmp(scan.modules[0].software_number, "6519020001") == 0);
    CHECK(scan.modules[0].hardware_number_available);
    CHECK(strcmp(scan.modules[0].hardware_number, "6519040001") == 0);

    CHECK(send_ok(&scan, "ATSH7E1") == 0);
    CHECK(send_ok(&scan, "ATCRA7E9") == 0);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "3E00") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "1902FF") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &dtcs) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.module_count == 2U);
    CHECK(scan.modules[1].kind == MBLINK_MERCEDES_MODULE_TRANSMISSION);
    CHECK(scan.modules[1].dtc_result == MBLINK_MERCEDES_MODULE_DTC_AVAILABLE);
    CHECK(scan.modules[1].dtcs.count == 2U);
    CHECK(scan.modules[1].dtcs.records[0].code == UINT32_C(0x123456));
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F197") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(!scan.modules[1].identity_available);
    CHECK(accept_identity_metadata(
              &scan, &no_data, &no_data, &no_data) == 0);
    CHECK(strcmp(mblink_mercedes_module_scan_module_name(&scan.modules[1]),
                 "Transmission ECU candidate (7E1/7E9 EOBD responder)") == 0);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 2U);
    CHECK(mblink_mercedes_module_scan_classified_count(&scan) == 2U);
    CHECK(mblink_mercedes_module_scan_timeout_ms(&scan) > 0U);

    /*
     * QUICK discovery still stops after 0x7E7. The normal product path now
     * uses the gateway census below; QUICK remains a bounded compatibility mode.
     */
    memset(&scan, 0, sizeof(scan));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    scan.module_count = 1U;
    scan.modules[0].tx_can_id = UINT32_C(0x7e0);
    scan.modules[0].rx_can_id = UINT32_C(0x7e8);
    scan.modules[0].extended_id = false;
    mblink_mercedes_module_scan_set_11_candidate(&scan, UINT32_C(0x7e7));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "ATSP6") == 0);

    /* With no responders there is nothing further to brute-force. */
    memset(&scan, 0, sizeof(scan));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    mblink_mercedes_module_scan_set_11_candidate(&scan, UINT32_C(0x7e7));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);

    /* Normal product scan: EOBD first, then gateway-routed 29-bit targets. */
    {
        const MblinkMercedesModuleDefinition *definition;

        CHECK(mblink_mercedes_c207_module_definition_count() >= 23U);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "CRD3-651-WMA4BD3");
        CHECK(definition != NULL);
        CHECK(definition->kind == MBLINK_MERCEDES_MODULE_ENGINE);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "VGS3_0402");
        CHECK(definition != NULL);
        CHECK(definition->kind == MBLINK_MERCEDES_MODULE_TRANSMISSION);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "ORC_212");
        CHECK(definition != NULL);
        CHECK(definition->kind == MBLINK_MERCEDES_MODULE_RESTRAINTS);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "IC_204");
        CHECK(definition != NULL);
        CHECK(definition->kind ==
              MBLINK_MERCEDES_MODULE_INSTRUMENT_CLUSTER);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "HU_204");
        CHECK(definition != NULL);
        CHECK(strcmp(definition->key, "audio-headunit") == 0);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "RBTMFL_204");
        CHECK(definition != NULL);
        CHECK(definition->kind == MBLINK_MERCEDES_MODULE_RESTRAINTS);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "RBTMFR_204");
        CHECK(definition != NULL);
        CHECK(definition->kind == MBLINK_MERCEDES_MODULE_RESTRAINTS);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "SEATD_212");
        CHECK(definition != NULL);
        CHECK(strcmp(definition->key, "seat-driver") == 0);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "SEATP_204");
        CHECK(definition != NULL);
        CHECK(strcmp(definition->key, "seat-passenger") == 0);
        CHECK(mblink_mercedes_c207_module_definition_for_identity(
                  "TOTALLY_UNKNOWN_ECU") == NULL);

        CHECK(mblink_mercedes_module_scan_begin_gateway(&scan) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_GATEWAY);
        CHECK(mblink_mercedes_module_scan_planned_target_count(
                  scan.scope) == 263U);
        CHECK(scan.candidate_tx == UINT32_C(0x7e0));

        memset(&scan, 0, sizeof(scan));
        scan.scope = MBLINK_MERCEDES_MODULE_SCAN_GATEWAY;
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        mblink_mercedes_module_scan_set_11_candidate(
            &scan, UINT32_C(0x7e7));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29);
        CHECK(scan.gateway_target == UINT16_C(0x00));
        CHECK(scan.candidate_extended);
        CHECK(scan.candidate_tx == UINT32_C(0x18da00f1));
        CHECK(scan.candidate_rx == UINT32_C(0x18daf100));
        CHECK(send_ok(&scan, "ATSP7") == 0);

        /* Tester address F1 is never probed as an ECU. */
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(mblink_mercedes_module_scan_set_gateway_target(
                  &scan, UINT16_C(0xf0)));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.gateway_target == UINT16_C(0xf2));
        CHECK(scan.candidate_tx == UINT32_C(0x18daf2f1));
        CHECK(scan.candidate_rx == UINT32_C(0x18daf1f2));

        /* FF is the final gateway-routed logical target. */
        memset(&scan, 0, sizeof(scan));
        scan.scope = MBLINK_MERCEDES_MODULE_SCAN_GATEWAY;
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(mblink_mercedes_module_scan_set_gateway_target(
                  &scan, UINT16_C(0xff)));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);
    }

    /*
     * A VIN-keyed saved profile reuses only known routes. It reinitialises
     * the adapter, validates each saved ECU with TesterPresent, refreshes its
     * DTC memory, and never walks undiscovered addresses.
     */
    {
        MblinkMercedesModuleScanEntry cached[4];
        memset(cached, 0, sizeof(cached));
        cached[0].tx_can_id = UINT32_C(0x7e0);
        cached[0].rx_can_id = UINT32_C(0x7e8);
        cached[0].kind = MBLINK_MERCEDES_MODULE_ENGINE;
        cached[0].identity_available = true;
        (void)snprintf(
            cached[0].identity, sizeof(cached[0].identity), "%s", "CRD3");

        cached[1].tx_can_id = UINT32_C(0x18da02f1);
        cached[1].rx_can_id = UINT32_C(0x18daf102);
        cached[1].extended_id = true;
        cached[1].kind = MBLINK_MERCEDES_MODULE_OTHER;

        cached[2].tx_can_id = UINT32_C(0x612);
        cached[2].rx_can_id = UINT32_C(0x482);
        cached[2].kind = MBLINK_MERCEDES_MODULE_BODY;

        cached[3].tx_can_id = UINT32_C(0x64a);
        cached[3].rx_can_id = UINT32_C(0x489);
        cached[3].kind = MBLINK_MERCEDES_MODULE_RESTRAINTS;

        CHECK(mblink_mercedes_module_scan_begin_cached(
                  &scan, cached, 4U) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED);
        CHECK(scan.module_count == 4U);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11);
        CHECK(scan.modules[0].identity_available);
        CHECK(strcmp(scan.modules[0].identity, "CRD3") == 0);

        CHECK(send_ok(&scan, "ATSP6") == 0);
        CHECK(send_ok(&scan, "ATH0") == 0);
        CHECK(send_ok(&scan, "ATCAF1") == 0);
        CHECK(send_ok(&scan, "ATCFC1") == 0);
        CHECK(send_ok(&scan, "ATST20") == 0);

        CHECK(send_ok(&scan, "ATSP6") == 0);
        CHECK(send_ok(&scan, "ATSH7E0") == 0);
        CHECK(send_ok(&scan, "ATCRA7E8") == 0);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "3E00") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &tester) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.modules[0].tester_present_response);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "1902FF") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &dtcs) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

        CHECK(send_ok(&scan, "ATSP7") == 0);
        CHECK(send_ok(&scan, "ATSH18DA02F1") == 0);
        CHECK(send_ok(&scan, "ATCRA18DAF102") == 0);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "3E00") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(!scan.modules[1].tester_present_response);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "1902FF") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

        CHECK(send_ok(&scan, "ATSP6") == 0);
        CHECK(send_ok(&scan, "ATSH612") == 0);
        CHECK(send_ok(&scan, "ATCRA482") == 0);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "1003") == 0);
        {
            MblinkElm327Response session_response =
                response(MBLINK_ELM327_RESULT_OK, "5003001400C8", false);
            CHECK(mblink_mercedes_module_scan_accept(
                      &scan, &session_response) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        }
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "3E00") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &tester) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.modules[2].tester_present_response);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "1902FF") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

        CHECK(send_ok(&scan, "ATSP6") == 0);
        CHECK(send_ok(&scan, "ATSH64A") == 0);
        CHECK(send_ok(&scan, "ATCRA489") == 0);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE);
        CHECK(scan.modules[3].protocol ==
              MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
        CHECK(scan.modules[3].definition != NULL);
        CHECK(strcmp(scan.modules[3].definition->key,
                     "restraints-orc") == 0);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "3E01") == 0);
        {
            MblinkElm327Response kwp_tester =
                response(MBLINK_ELM327_RESULT_OK, "7E", false);
            CHECK(mblink_mercedes_module_scan_accept(
                      &scan, &kwp_tester) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        }
        CHECK(scan.modules[3].tester_present_response);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "1802FF00") == 0);
        {
            MblinkElm327Response kwp_dtcs =
                response(MBLINK_ELM327_RESULT_OK, "5801D6AA20", false);
            CHECK(mblink_mercedes_module_scan_accept(
                      &scan, &kwp_dtcs) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE);
        }

        CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);
        CHECK(mblink_mercedes_module_scan_fresh_response_count(&scan) == 3U);
        CHECK(scan.modules[0].dtcs.count == 2U);
        CHECK(scan.modules[3].kwp_dtcs.count == 1U);
        CHECK(scan.modules[3].kwp_dtcs.entries[0].code ==
              UINT16_C(0xd6aa));
        CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 3U);
    }

    /*
     * The first-VIN census uses one TesterPresent on ordinary dead addresses.
     * Source-corroborated Mercedes nonstandard routes get bounded read-only
     * fallback probes because their independently published RX identifier is
     * stronger evidence than the generic request+8 assumption.
     */
    {
        const link_discover_sweep_plan *plan =
            mblink_discover_full_sweep_plan();
        MblinkElm327Response tester_response =
            response(MBLINK_ELM327_RESULT_OK, "7E00", false);

        CHECK(link_discover_sweep_plan_is_valid(plan));
        CHECK(mblink_mercedes_module_scan_begin_mobile_census(&scan) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS);
        CHECK(mblink_mercedes_module_scan_planned_target_count(scan.scope) ==
              plan->target_count);
        CHECK(scan.candidate_tx == UINT32_C(0x600));
        CHECK(scan.candidate_rx == UINT32_C(0x608));

        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT;
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "ATST20") == 0);

        /* A dead 11-bit address advances after TesterPresent alone. */
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 0U));
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.full_target_index == 1U);
        CHECK(scan.candidate_tx == UINT32_C(0x601));
        CHECK(scan.module_count == 0U);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER);

        /* A real response on the plan-defined RX route gets deeper reads. */
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 0U));
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        CHECK(mblink_mercedes_module_scan_accept(&scan, &tester_response) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.module_count == 1U);
        CHECK(scan.modules[0].tx_can_id == UINT32_C(0x600));
        CHECK(scan.modules[0].rx_can_id == UINT32_C(0x608));
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);

        /*
         * Published EIS_212 route evidence overrides request+8. Even when
         * TesterPresent is quiet, the bounded read-only fallbacks are retained
         * and any valid negative UDS reply proves that the ECU exists.
         */
        {
            MblinkElm327Response session_response =
                response(MBLINK_ELM327_RESULT_OK, "5003001400C8", false);
            MblinkElm327Response uds_negative =
                response(MBLINK_ELM327_RESULT_OK, "7F2231", false);
            CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 18U));
            CHECK(scan.candidate_tx == UINT32_C(0x612));
            CHECK(scan.candidate_rx == UINT32_C(0x482));

            scan.stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
            CHECK(send_ok(&scan, "ATCRA482") == 0);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION);
            CHECK(mblink_mercedes_module_scan_command(
                      &scan, command, sizeof(command), &written) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(strcmp(command, "1003") == 0);
            CHECK(mblink_mercedes_module_scan_accept(
                      &scan, &session_response) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.module_count == 2U);
            CHECK(scan.modules[1].tx_can_id == UINT32_C(0x612));
            CHECK(scan.modules[1].rx_can_id == UINT32_C(0x482));
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT);

            CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
            CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
            {
                const MblinkMercedesModuleScanEntry *eis = &scan.modules[1];
                CHECK(eis->tx_can_id == UINT32_C(0x612));
                CHECK(eis->rx_can_id == UINT32_C(0x482));
                CHECK(eis->definition != NULL);
                CHECK(strcmp(eis->definition->key, "eis-ezs") == 0);
                CHECK(eis->kind == MBLINK_MERCEDES_MODULE_BODY);
            }

            /*
             * ORC_212 is a source-backed KWP2000-over-CAN route, not UDS.
             * It must use the exact 0x64A -> 0x489 pair, KWP TesterPresent
             * response type 0x01 and the KWP DTC-by-status service.
             */
            {
                MblinkElm327Response kwp_tester =
                    response(MBLINK_ELM327_RESULT_OK, "7E", false);
                MblinkElm327Response kwp_dtcs =
                    response(MBLINK_ELM327_RESULT_OK,
                             "5801D6AA20", false);

                CHECK(mblink_mercedes_module_scan_set_full_target(
                          &scan, 74U));
                CHECK(scan.candidate_tx == UINT32_C(0x64a));
                CHECK(scan.candidate_rx == UINT32_C(0x489));
                CHECK(mblink_mercedes_module_scan_candidate_protocol(&scan) ==
                      MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);

                scan.stage =
                    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
                CHECK(send_ok(&scan, "ATCRA489") == 0);
                CHECK(scan.stage ==
                      MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT);
                CHECK(mblink_mercedes_module_scan_command(
                          &scan, command, sizeof(command), &written) ==
                      MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
                CHECK(strcmp(command, "3E01") == 0);
                CHECK(mblink_mercedes_module_scan_accept(
                          &scan, &kwp_tester) ==
                      MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
                CHECK(scan.module_count == 3U);
                CHECK(scan.modules[2].protocol ==
                      MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
                CHECK(scan.modules[2].definition != NULL);
                CHECK(strcmp(scan.modules[2].definition->key,
                             "restraints-orc") == 0);
                CHECK(scan.modules[2].kind ==
                      MBLINK_MERCEDES_MODULE_RESTRAINTS);
                CHECK(scan.stage ==
                      MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
                CHECK(mblink_mercedes_module_scan_command(
                          &scan, command, sizeof(command), &written) ==
                      MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
                CHECK(strcmp(command, "1802FF00") == 0);
                CHECK(mblink_mercedes_module_scan_accept(
                          &scan, &kwp_dtcs) ==
                      MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
                CHECK(scan.modules[2].kwp_dtcs.count == 1U);
                CHECK(scan.full_target_index == 75U);
                CHECK(scan.candidate_tx == UINT32_C(0x64b));
            }

            /*
             * If the transient session itself stays silent, retain the same
             * exact-route fallbacks and let F100/its negative response prove
             * presence. EPS212 gives us a second independently published pair.
             */
            CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 178U));
            CHECK(scan.candidate_tx == UINT32_C(0x6b2));
            CHECK(scan.candidate_rx == UINT32_C(0x496));
            scan.stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
            CHECK(send_ok(&scan, "ATCRA496") == 0);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION);
            CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT);
            CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
            CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK);
            CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.stage ==
                  MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK);
            CHECK(mblink_mercedes_module_scan_command(
                      &scan, command, sizeof(command), &written) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(strcmp(command, "22F100") == 0);
            CHECK(mblink_mercedes_module_scan_accept(
                      &scan, &uds_negative) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
            CHECK(scan.module_count == 4U);
            {
                const MblinkMercedesModuleScanEntry *eps = &scan.modules[3];
                CHECK(eps->tx_can_id == UINT32_C(0x6b2));
                CHECK(eps->rx_can_id == UINT32_C(0x496));
                CHECK(eps->definition != NULL);
                CHECK(strcmp(eps->definition->key, "steering-column") == 0);
            }
        }

        /* A dead 29-bit logical target also advances after one presence probe. */
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 504U));
        CHECK(scan.candidate_extended);
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.full_target_index == 505U);
    }

    /* Explicit FULL consumes the one Mercedes-owned plan. */
    {
        const link_discover_sweep_plan *plan =
            mblink_discover_full_sweep_plan();
        link_discover_sweep_target target;

        CHECK(link_discover_sweep_plan_is_valid(plan));
        CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_FULL);
        CHECK(scan.full_target_index == 0U);
        CHECK(scan.candidate_tx == UINT32_C(0x600));
        CHECK(scan.candidate_rx == UINT32_C(0x608));

        /* Full forensic discovery gets the deliberately longer ELM timeout. */
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT;
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "ATST32") == 0);
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 0U));
        CHECK(scan.candidate_route_locked);
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;

        /* A responder can still supply F197 evidence that becomes its label. */
        CHECK(mblink_mercedes_module_scan_accept(&scan, &tester) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.module_count == 1U);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "1902FF") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.modules[0].dtc_result ==
              MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "22F197") == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &unknown_identity) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.modules[0].identity_available);
        CHECK(strcmp(scan.modules[0].identity, "ESP") == 0);
        CHECK(scan.modules[0].definition != NULL);
        CHECK(scan.modules[0].kind == MBLINK_MERCEDES_MODULE_ABS_ESP);
        CHECK(strcmp(
                  mblink_mercedes_module_scan_module_name(&scan.modules[0]),
                  "Electronic Stability Program (ESP) control unit") == 0);
        CHECK(accept_identity_metadata(
                  &scan, &no_data, &no_data, &no_data) == 0);
        CHECK(scan.full_target_index == 1U);
        CHECK(scan.candidate_tx == UINT32_C(0x601));

        /* The plan itself controls the 11-bit to 29-bit transition. */
        memset(&scan, 0, sizeof(scan));
        scan.scope = MBLINK_MERCEDES_MODULE_SCAN_FULL;
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 503U));
        CHECK(scan.candidate_tx == UINT32_C(0x7f7));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.full_target_index == 504U);
        CHECK(scan.candidate_extended);
        CHECK(scan.candidate_tx == UINT32_C(0x18da00f1));
        CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29);
        CHECK(send_ok(&scan, "ATSP7") == 0);

        /* F1 is absent because the MBLINK plan skips the tester address. */
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 744U));
        CHECK(scan.candidate_tx == UINT32_C(0x18daf0f1));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.full_target_index == 745U);
        CHECK(scan.candidate_tx == UINT32_C(0x18daf2f1));

        /* The plan's final target ends discovery. */
        memset(&scan, 0, sizeof(scan));
        scan.scope = MBLINK_MERCEDES_MODULE_SCAN_FULL;
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(link_discover_sweep_plan_target_at(
                  plan, plan->target_count - 1U, &target));
        CHECK(mblink_mercedes_module_scan_set_full_target(
                  &scan, plan->target_count - 1U));
        CHECK(scan.candidate_tx == target.tx_can_id);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE);
        CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);
    }

    return 0;
}
