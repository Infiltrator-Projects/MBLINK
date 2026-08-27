// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file test_mercedes_captured_deep_scan.c
 * @brief Replay the 2026-08-27 C207/OM651 ELM327 scan shapes.
 *
 * This test deliberately separates evidence captured from the real vehicle
 * from one synthetic ORC/SRS fault injection:
 *
 *  - The 0x600/0x601 NO DATA sweep behaviour is copied from the real scan.
 *  - The 0x7E1 -> 0x7E9 TesterPresent negative response 7F 3E 12 is copied
 *    from the real scan and must still count as a responding ECU.
 *  - The ORC_212 identity and one DTC payload are synthetic.  They exist only
 *    to prove that once a restraint ECU answers, the current scanner records
 *    its fault immediately instead of waiting for the full forensic sweep.
 *
 * The user's VIN is intentionally not stored in this public fixture.
 */
#include "mblink/mercedes_module_scan.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; \
} } while (0)

static MblinkElm327Response make_response(
    MblinkElm327Result result,
    const char *text,
    bool ok_seen)
{
    MblinkElm327Response response;
    size_t length = text != NULL ? strlen(text) : 0U;

    memset(&response, 0, sizeof(response));
    response.result = result;
    response.ok_seen = ok_seen;
    if (length >= sizeof(response.text)) length = sizeof(response.text) - 1U;
    if (length != 0U) memcpy(response.text, text, length);
    response.text[length] = '\0';
    response.length = length;
    return response;
}

static int accept_expected_ok(
    MblinkMercedesModuleScan *scan,
    const char *expected)
{
    char command[32];
    size_t written = 0U;
    MblinkElm327Response ok =
        make_response(MBLINK_ELM327_RESULT_OK, "OK", true);

    CHECK(mblink_mercedes_module_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, expected) == 0);
    CHECK(written == strlen(expected));
    CHECK(mblink_mercedes_module_scan_accept(scan, &ok) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    return 0;
}

static int accept_expected_response(
    MblinkMercedesModuleScan *scan,
    const char *expected,
    const MblinkElm327Response *response)
{
    char command[32];
    size_t written = 0U;

    CHECK(mblink_mercedes_module_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, expected) == 0);
    CHECK(mblink_mercedes_module_scan_accept(scan, response) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    return 0;
}

static int prepare_full_11_candidate(MblinkMercedesModuleScan *scan,
                                     const char *header)
{
    CHECK(accept_expected_ok(scan, "ATH1") == 0);
    CHECK(accept_expected_ok(scan, header) == 0);
    CHECK(accept_expected_ok(scan, "ATCRA") == 0);
    CHECK(accept_expected_ok(scan, "ATCF000") == 0);
    CHECK(accept_expected_ok(scan, "ATCM000") == 0);
    return 0;
}

static int replay_captured_full_scan_misses(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response no_data =
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_FULL);
    CHECK(scan.candidate_tx == UINT32_C(0x600));

    CHECK(accept_expected_ok(&scan, "ATSP6") == 0);
    CHECK(accept_expected_ok(&scan, "ATH0") == 0);
    CHECK(accept_expected_ok(&scan, "ATCAF1") == 0);
    CHECK(accept_expected_ok(&scan, "ATCFC1") == 0);
    CHECK(accept_expected_ok(&scan, "ATST32") == 0);

    /*
     * The vehicle capture proved that these requests receive NO DATA. The
     * current full-scan transport setup is deliberately different: headers
     * are enabled and the receive filter is widened so a responder cannot be
     * hidden merely because its CAN ID is not request+8.
     */
    CHECK(prepare_full_11_candidate(&scan, "ATSH600") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "1902FF", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "22F190", &no_data) == 0);
    CHECK(scan.full_target_index == 1U);
    CHECK(scan.candidate_tx == UINT32_C(0x601));
    CHECK(scan.module_count == 0U);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS);

    CHECK(prepare_full_11_candidate(&scan, "ATSH601") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "1902FF", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "22F190", &no_data) == 0);
    CHECK(scan.full_target_index == 2U);
    CHECK(scan.candidate_tx == UINT32_C(0x602));
    CHECK(scan.module_count == 0U);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS);
    return 0;
}

static int replay_captured_negative_tester_present(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response tester_negative_headered =
        make_response(MBLINK_ELM327_RESULT_OK, "7E9037F3E12", false);
    MblinkElm327Response no_data =
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 481U));
    CHECK(scan.candidate_tx == UINT32_C(0x7e1));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS;

    CHECK(prepare_full_11_candidate(&scan, "ATSH7E1") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &tester_negative_headered) == 0);

    CHECK(scan.module_count == 1U);
    CHECK(scan.modules[0].tx_can_id == UINT32_C(0x7e1));
    CHECK(scan.modules[0].rx_can_id == UINT32_C(0x7e9));
    CHECK(scan.modules[0].tester_present_response);
    CHECK(scan.candidate_route_locked);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF);
    CHECK(accept_expected_ok(&scan, "ATH0") == 0);
    CHECK(accept_expected_ok(&scan, "ATCRA7E9") == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);

    CHECK(accept_expected_response(&scan, "1902FF", &no_data) == 0);
    CHECK(scan.modules[0].dtc_result == MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    return 0;
}

static int learn_non_plus_eight_route(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response tester_headered =
        make_response(MBLINK_ELM327_RESULT_OK, "640027E00", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 48U));
    CHECK(scan.candidate_tx == UINT32_C(0x630));
    CHECK(scan.candidate_rx == UINT32_C(0x638)); /* old plan hint only */
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS;

    CHECK(prepare_full_11_candidate(&scan, "ATSH630") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &tester_headered) == 0);
    CHECK(scan.module_count == 1U);
    CHECK(scan.modules[0].tx_can_id == UINT32_C(0x630));
    CHECK(scan.modules[0].rx_can_id == UINT32_C(0x640));
    CHECK(scan.candidate_rx == UINT32_C(0x640));
    CHECK(scan.candidate_rx != scan.candidate_tx + UINT32_C(8));
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF);
    CHECK(accept_expected_ok(&scan, "ATH0") == 0);
    CHECK(accept_expected_ok(&scan, "ATCRA640") == 0);
    return 0;
}

static int inject_restraint_fault_after_real_scan_shapes(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response tester =
        make_response(MBLINK_ELM327_RESULT_OK, "7E00", false);
    MblinkElm327Response dtc =
        make_response(MBLINK_ELM327_RESULT_OK,
                      "7F1978\n5902FF12345609", false);
    MblinkElm327Response orc_identity =
        make_response(MBLINK_ELM327_RESULT_OK,
                      "62F1974F52435F323132", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

    /*
     * Synthetic responder only: use a gateway-routed physical address so the
     * test proves the same path used after the 11-bit Linux census.  The
     * identity ORC_212 is already evidence-backed in the C207 module catalog.
     */
    scan.candidate_tx = UINT32_C(0x18da10f1);
    scan.candidate_rx = UINT32_C(0x18daf110);
    scan.candidate_extended = true;
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;

    CHECK(accept_expected_response(&scan, "3E00", &tester) == 0);
    CHECK(scan.module_count == 1U);
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);

    /*
     * Response-pending followed by a positive DTC reply is the same framing
     * behaviour captured from the real CRD3 engine ECU.
     */
    CHECK(accept_expected_response(&scan, "1902FF", &dtc) == 0);
    CHECK(scan.modules[0].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_AVAILABLE);
    CHECK(scan.modules[0].dtcs.count == 1U);
    CHECK(scan.modules[0].dtcs.records[0].code == UINT32_C(0x123456));
    CHECK(scan.modules[0].dtcs.records[0].status == UINT8_C(0x09));
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);

    CHECK(accept_expected_response(&scan, "22F197", &orc_identity) == 0);
    CHECK(scan.modules[0].identity_available);
    CHECK(strcmp(scan.modules[0].identity, "ORC_212") == 0);
    CHECK(scan.modules[0].definition != NULL);
    CHECK(scan.modules[0].kind == MBLINK_MERCEDES_MODULE_RESTRAINTS);
    CHECK(strcmp(mblink_mercedes_module_scan_module_name(&scan.modules[0]),
                 "Occupant restraint / airbag control unit (ORC)") == 0);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 1U);
    return 0;
}

int main(void)
{
    if (replay_captured_full_scan_misses() != 0) return 1;
    if (replay_captured_negative_tester_present() != 0) return 1;
    if (learn_non_plus_eight_route() != 0) return 1;
    if (inject_restraint_fault_after_real_scan_shapes() != 0) return 1;
    puts("Captured C207 deep-scan replay tests passed");
    return 0;
}
