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
        "5902FF0112345609ABCDEF28", false);

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
    CHECK(strcmp(mblink_mercedes_module_scan_module_name(&scan.modules[1]), "Transmission ECU") == 0);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 2U);
    CHECK(mblink_mercedes_module_scan_timeout_ms(&scan) > 0U);

    /* The normal discovery pass must remain bounded to the legislated
       physical diagnostic CAN range.  A missed 0x7E7 must move directly to
       the optional 29-bit phase instead of brute-forcing 0x600..0x7F7. */
    memset(&scan, 0, sizeof(scan));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
    scan.discovery_mode = 0U;
    mblink_mercedes_module_scan_set_11_candidate(&scan, UINT32_C(0x7e7));
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29);
    CHECK(scan.discovery_mode == 2U);
    CHECK(mblink_mercedes_module_scan_command(&scan, command, sizeof(command), &written) ==
MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "ATSP7") == 0);
    return 0;
}
