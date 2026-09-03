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

static size_t mobile_target_index_for_tx(uint32_t tx)
{
    const link_discover_sweep_plan *plan =
        mblink_discover_mobile_census_plan();
    link_discover_sweep_target target;
    size_t index;

    if (!link_discover_sweep_plan_is_valid(plan)) return (size_t)-1;
    for (index = 0U; index < plan->target_count; ++index) {
        if (link_discover_sweep_plan_target_at(plan, index, &target) &&
            !target.extended_id && target.tx_can_id == tx) {
            return index;
        }
    }
    return (size_t)-1;
}

static size_t full_target_index_for_tx(uint32_t tx)
{
    const link_discover_sweep_plan *plan =
        mblink_discover_full_sweep_plan();
    link_discover_sweep_target target;
    size_t index;

    if (!link_discover_sweep_plan_is_valid(plan)) return (size_t)-1;
    for (index = 0U; index < plan->target_count; ++index) {
        if (link_discover_sweep_plan_target_at(plan, index, &target) &&
            !target.extended_id && target.tx_can_id == tx) {
            return index;
        }
    }
    return (size_t)-1;
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
    CHECK(scan.modules[1].kind == MBLINK_MERCEDES_MODULE_OTHER);
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
                 "Secondary EOBD powertrain ECU (7E1/7E9)") == 0);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 2U);
    CHECK(mblink_mercedes_module_scan_classified_count(&scan) == 1U);
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

    /* Normal product scan: source-backed routes first, then the full census. */
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
                "CGW_212");
        CHECK(definition != NULL);
        CHECK(strcmp(definition->key, "central-gateway") == 0);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "SAMF_212");
        CHECK(definition != NULL);
        CHECK(strcmp(definition->key, "front-sam") == 0);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "SAMR_212");
        CHECK(definition != NULL);
        CHECK(strcmp(definition->key, "rear-sam") == 0);
        definition =
            mblink_mercedes_c207_module_definition_for_identity(
                "HVAC_212");
        CHECK(definition != NULL);
        CHECK(definition->kind == MBLINK_MERCEDES_MODULE_CLIMATE);
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
     * The iPhone first-VIN census uses the compact Mercedes gateway lattice,
     * never the workstation's 760-target forensic sweep.
     */
    {
        const link_discover_sweep_plan *plan =
            mblink_discover_mobile_census_plan();
        link_discover_sweep_target target;

        CHECK(link_discover_sweep_plan_is_valid(plan));
        CHECK(plan->target_count == 57U);
        CHECK(mblink_mercedes_module_scan_begin_mobile_census(&scan) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS);
        CHECK(mblink_mercedes_module_scan_planned_target_count(scan.scope) ==
              plan->target_count);

        CHECK(link_discover_sweep_plan_target_at(plan, 0U, &target));
        CHECK(target.tx_can_id == UINT32_C(0x602));
        CHECK(target.rx_can_id == UINT32_C(0x480));
        CHECK(link_discover_sweep_plan_target_at(plan, 1U, &target));
        CHECK(target.tx_can_id == UINT32_C(0x60a));
        CHECK(target.rx_can_id == UINT32_C(0x481));
        CHECK(link_discover_sweep_plan_target_at(plan, 2U, &target));
        CHECK(target.tx_can_id == UINT32_C(0x612));
        CHECK(target.rx_can_id == UINT32_C(0x482));
        CHECK(link_discover_sweep_plan_target_at(plan, 46U, &target));
        CHECK(target.tx_can_id == UINT32_C(0x772));
        CHECK(target.rx_can_id == UINT32_C(0x4ae));

        CHECK(mobile_target_index_for_tx(UINT32_C(0x607)) != (size_t)-1);
        CHECK(mobile_target_index_for_tx(UINT32_C(0x4e0)) != (size_t)-1);
        CHECK(mobile_target_index_for_tx(UINT32_C(0x7e0)) != (size_t)-1);
        CHECK(mobile_target_index_for_tx(UINT32_C(0x7e7)) != (size_t)-1);

        CHECK(scan.candidate_tx == UINT32_C(0x602));
        CHECK(scan.candidate_rx == UINT32_C(0x480));
        CHECK(scan.candidate_route_locked);

        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT;
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "ATST20") == 0);

        /*
         * An ordinary lattice slot uses the exact receive ID. The iPhone
         * never clears ATCRA / opens a zero mask just to discover that slot.
         */
        CHECK(mblink_mercedes_module_scan_set_full_target(
                  &scan, mobile_target_index_for_tx(UINT32_C(0x60a))));
        CHECK(scan.candidate_route_locked);
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
        CHECK(send_ok(&scan, "ATSH60A") == 0);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE);
        CHECK(send_ok(&scan, "ATCRA481") == 0);
        CHECK(scan.stage ==
              MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.candidate_tx == UINT32_C(0x612));
        CHECK(scan.candidate_rx == UINT32_C(0x482));

        /*
         * Source-backed slots retain protocol overrides. ORC_212 is KWP2000
         * on the exact 0x64A -> 0x489 lattice pair.
         */
        CHECK(mblink_mercedes_module_scan_set_full_target(
                  &scan, mobile_target_index_for_tx(UINT32_C(0x64a))));
        CHECK(scan.candidate_rx == UINT32_C(0x489));
        CHECK(mblink_mercedes_module_scan_candidate_protocol(&scan) ==
              MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
        CHECK(send_ok(&scan, "ATSH64A") == 0);
        CHECK(send_ok(&scan, "ATCRA489") == 0);
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "3E01") == 0);

        /*
         * Broad unknown-response learning remains FULL-only so workstation
         * forensics can still discover a valid non-+8 response route.
         */
        memset(&scan, 0, sizeof(scan));
        scan.scope = MBLINK_MERCEDES_MODULE_SCAN_FULL;
        CHECK(mblink_mercedes_module_scan_set_full_target(
                  &scan, full_target_index_for_tx(UINT32_C(0x600))));
        CHECK(!scan.candidate_route_locked);
        scan.stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
        CHECK(send_ok(&scan, "ATSH600") == 0);
        CHECK(send_ok(&scan, "ATCRA") == 0);
        CHECK(send_ok(&scan, "ATCF000") == 0);
        CHECK(send_ok(&scan, "ATCM000") == 0);
        {
            MblinkElm327Response non_plus_eight =
                response(MBLINK_ELM327_RESULT_OK, "4A0027E00", false);
            CHECK(mblink_mercedes_module_scan_accept(
                      &scan, &non_plus_eight) ==
                  MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        }
        CHECK(scan.candidate_route_locked);
        CHECK(scan.candidate_rx == UINT32_C(0x4a0));
        CHECK(scan.module_count == 1U);
        CHECK(scan.modules[0].tx_can_id == UINT32_C(0x600));
        CHECK(scan.modules[0].rx_can_id == UINT32_C(0x4a0));
    }

    /* Explicit FULL consumes the one Mercedes-owned plan. */
    {
        const link_discover_sweep_plan *plan =
            mblink_discover_full_sweep_plan();
        link_discover_sweep_target target;

        CHECK(link_discover_sweep_plan_is_valid(plan));
        CHECK(plan->target_count == 760U);
        CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_FULL);
        CHECK(scan.full_target_index == 0U);
        CHECK(scan.candidate_tx == UINT32_C(0x612));
        CHECK(scan.candidate_rx == UINT32_C(0x482));

        /* Full forensic discovery gets the deliberately longer ELM timeout. */
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT;
        CHECK(mblink_mercedes_module_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(command, "ATST32") == 0);
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 0U));
        CHECK(scan.candidate_route_locked);
        CHECK(scan.candidate_tx == UINT32_C(0x612));
        CHECK(scan.candidate_rx == UINT32_C(0x482));
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
        CHECK(scan.candidate_tx == UINT32_C(0x632));
        CHECK(scan.candidate_rx == UINT32_C(0x486));

        /* The plan itself controls the 11-bit to 29-bit transition. */
        memset(&scan, 0, sizeof(scan));
        scan.scope = MBLINK_MERCEDES_MODULE_SCAN_FULL;
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 504U));
        CHECK(scan.candidate_tx == UINT32_C(0x7f7));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.full_target_index == 505U);
        CHECK(scan.candidate_extended);
        CHECK(scan.candidate_tx == UINT32_C(0x18da00f1));
        CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29);
        CHECK(send_ok(&scan, "ATSP7") == 0);

        /* F1 is absent because the MBLINK plan skips the tester address. */
        scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 745U));
        CHECK(scan.candidate_tx == UINT32_C(0x18daf0f1));
        CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
              MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.full_target_index == 746U);
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
