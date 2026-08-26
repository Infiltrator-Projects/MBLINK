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

    CHECK(mblink_mercedes_module_scan_begin(&scan) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(send_ok(&scan, "ATSP6") == 0);
    CHECK(send_ok(&scan, "ATH0") == 0);
    CHECK(send_ok(&scan, "ATCAF1") == 0);
    CHECK(send_ok(&scan, "ATCFC1") == 0);
    CHECK(send_ok(&scan, "ATST10") == 0);
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
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F197") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &engine_identity) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.modules[0].identity_available);
    CHECK(strcmp(scan.modules[0].identity, "CRD3") == 0);

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
    CHECK(strcmp(mblink_mercedes_module_scan_module_name(&scan.modules[1]), "Secondary EOBD powertrain ECU") == 0);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 2U);
    CHECK(mblink_mercedes_module_scan_timeout_ms(&scan) > 0U);

    /*
     * The C207 capture proves that normal discovery should stop after 0x7E7.
     * With a responding ECU already recorded, proceed to its DTC inventory.
     */
    memset(&scan, 0, sizeof(scan));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    scan.discovery_mode = 0U;
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
    scan.discovery_mode = 0U;
    mblink_mercedes_module_scan_set_11_candidate(&scan, UINT32_C(0x7e7));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);

    /* Explicit FULL starts at the broad Mercedes diagnostic range, not 7E0. */
    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_FULL);
    CHECK(scan.discovery_mode == 1U);
    CHECK(scan.candidate_tx == UINT32_C(0x600));
    CHECK(scan.candidate_rx == UINT32_C(0x608));

    /* A responder can supply F197 evidence that becomes its label. */
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
    CHECK(mblink_mercedes_module_scan_accept(&scan, &tester) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.module_count == 1U);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F197") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &unknown_identity) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.modules[0].identity_available);
    CHECK(strcmp(scan.modules[0].identity, "ESP") == 0);
    CHECK(strcmp(mblink_mercedes_module_scan_module_name(&scan.modules[0]), "ESP") == 0);
    CHECK(scan.candidate_tx == UINT32_C(0x601));

    /* Exhaust the 11-bit range, then deliberately enter the 29-bit phase. */
    memset(&scan, 0, sizeof(scan));
    scan.scope = MBLINK_MERCEDES_MODULE_SCAN_FULL;
    scan.discovery_mode = 1U;
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    mblink_mercedes_module_scan_set_11_candidate(&scan, UINT32_C(0x7f7));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29);
    CHECK(send_ok(&scan, "ATSP7") == 0);
    CHECK(scan.discovery_mode == 2U);
    CHECK(scan.candidate_extended);
    CHECK(scan.normal_fixed_target == 0U);

    /* The tester address F1 is skipped during normal-fixed target enumeration. */
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    mblink_mercedes_module_scan_set_29_candidate(&scan, UINT16_C(0x00f0));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.normal_fixed_target == UINT16_C(0x00f2));

    /* FF is the last normal-fixed target. */
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    scan.module_count = 0U;
    mblink_mercedes_module_scan_set_29_candidate(&scan, UINT16_C(0x00ff));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);
    return 0;
}
