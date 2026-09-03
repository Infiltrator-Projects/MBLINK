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
 *    from the real scan and must still count as a responding ECU. The route
 *    is now source-backed as Mercedes GS KWP2000, so current discovery sends
 *    KWP TesterPresent 3E 01 rather than the older generic UDS 3E 00 probe.
 *  - The 0x64A -> 0x489 ORC fault and 0x652 -> 0x48A clean head-unit DTC
 *    inventory are copied from the 2026-08-30 drive capture.
 *  - The ORC_212 identity and one DTC payload are synthetic.  They exist only
 *    to prove that once a restraint ECU answers, the current scanner records
 *    its fault immediately instead of waiting for the full forensic sweep.
 *
 * The user's VIN is intentionally not stored in this public fixture.
 */
#include "mblink/mercedes_module_scan.h"
#include "mblink/mercedes.h"
#include "mblink/uds_dtc.h"
#include "support/c207_20260903_evidence.h"

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

static int prepare_exact_11_candidate(MblinkMercedesModuleScan *scan,
                                      const char *header,
                                      const char *receive)
{
    CHECK(accept_expected_ok(scan, header) == 0);
    CHECK(accept_expected_ok(scan, receive) == 0);
    return 0;
}

static int prepare_full_unknown_11_candidate(
    MblinkMercedesModuleScan *scan,
    const char *header)
{
    /*
     * FULL forensic discovery is allowed to learn an otherwise unknown
     * Mercedes 11-bit response identifier from a headered diagnostic reply.
     * The receive window is opened only around the bounded read-only probe;
     * MOBILE_CENSUS never uses this path.
     */
    CHECK(!scan->candidate_route_locked);
    CHECK(accept_expected_ok(scan, header) == 0);
    CHECK(accept_expected_ok(scan, "ATCRA") == 0);
    CHECK(accept_expected_ok(scan, "ATCF000") == 0);
    CHECK(accept_expected_ok(scan, "ATCM000") == 0);
    return 0;
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

static int replay_captured_full_scan_misses(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response no_data =
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.scope == MBLINK_MERCEDES_MODULE_SCAN_FULL);
    CHECK(scan.candidate_tx == UINT32_C(0x612));
    {
        const size_t index600 = full_target_index_for_tx(UINT32_C(0x600));
        CHECK(index600 != (size_t)-1);
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, index600));
        CHECK(scan.candidate_tx == UINT32_C(0x600));
    }

    CHECK(accept_expected_ok(&scan, "ATSP6") == 0);
    CHECK(accept_expected_ok(&scan, "ATH0") == 0);
    CHECK(accept_expected_ok(&scan, "ATCAF1") == 0);
    CHECK(accept_expected_ok(&scan, "ATCFC1") == 0);
    CHECK(accept_expected_ok(&scan, "ATST32") == 0);

    /*
     * The vehicle capture proved that these requests receive NO DATA. FULL now
     * treats TX+8 only as a placeholder for generic 11-bit targets, so replay
     * the bounded headered discovery setup before those captured misses.
     */
    CHECK(prepare_full_unknown_11_candidate(&scan, "ATSH600") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "1902FF", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "22F190", &no_data) == 0);
    CHECK(scan.full_target_index ==
          full_target_index_for_tx(UINT32_C(0x601)));
    CHECK(scan.candidate_tx == UINT32_C(0x601));
    CHECK(scan.module_count == 0U);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER);

    CHECK(prepare_full_unknown_11_candidate(&scan, "ATSH601") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "1902FF", &no_data) == 0);
    CHECK(accept_expected_response(&scan, "22F190", &no_data) == 0);
    /*
     * 0x602 is now probed earlier on Daimler's exact 0x602 -> 0x480 route,
     * so the generic request+8 enumeration skips directly to 0x603.
     */
    CHECK(scan.full_target_index ==
          full_target_index_for_tx(UINT32_C(0x603)));
    CHECK(scan.candidate_tx == UINT32_C(0x603));
    CHECK(scan.module_count == 0U);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER);
    return 0;
}

static int replay_captured_negative_tester_present(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response tester_negative =
        make_response(MBLINK_ELM327_RESULT_OK, "7F3E12", false);
    MblinkElm327Response no_data =
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(mblink_mercedes_module_scan_set_full_target(
              &scan, full_target_index_for_tx(UINT32_C(0x7e1))));
    CHECK(scan.candidate_tx == UINT32_C(0x7e1));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;

    CHECK(prepare_exact_11_candidate(&scan, "ATSH7E1", "ATCRA7E9") == 0);
    CHECK(accept_expected_response(&scan, "3E01", &tester_negative) == 0);

    CHECK(scan.module_count == 1U);
    CHECK(scan.modules[0].tx_can_id == UINT32_C(0x7e1));
    CHECK(scan.modules[0].rx_can_id == UINT32_C(0x7e9));
    CHECK(scan.modules[0].protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
    CHECK(scan.modules[0].kind == MBLINK_MERCEDES_MODULE_TRANSMISSION);
    CHECK(scan.modules[0].tester_present_response);
    CHECK(scan.candidate_route_locked);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);

    CHECK(accept_expected_response(&scan, "1802FF00", &no_data) == 0);
    CHECK(scan.modules[0].dtc_result == MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE);
    /*
     * KWP GS routes deliberately skip UDS F197/F187/F188/F191 identity reads;
     * the source-backed route classification already establishes transmission.
     */
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER);
    return 0;
}

static int full_unknown_11_bit_candidate_is_not_forced_to_plus_eight(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response no_data =
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(mblink_mercedes_module_scan_set_full_target(
              &scan, full_target_index_for_tx(UINT32_C(0x630))));
    CHECK(scan.candidate_tx == UINT32_C(0x630));
    CHECK(scan.candidate_rx == UINT32_C(0x638));
    CHECK(!scan.candidate_route_locked);
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;

    CHECK(prepare_full_unknown_11_candidate(&scan, "ATSH630") == 0);
    CHECK(accept_expected_response(&scan, "3E00", &no_data) == 0);
    CHECK(scan.module_count == 0U);
    CHECK(scan.stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK);
    return 0;
}

static int replay_20260903_unclassified_602_fault(void)
{
    MblinkMercedesModuleScan scan;
    const MblinkC207RouteEvidence *evidence =
        &mblink_c207_20260903_routes[0];
    MblinkElm327Response session =
        make_response(MBLINK_ELM327_RESULT_OK,
                      evidence->session_response, false);
    MblinkElm327Response tester =
        make_response(MBLINK_ELM327_RESULT_OK,
                      evidence->tester_present_response, false);
    MblinkElm327Response dtc =
        make_response(MBLINK_ELM327_RESULT_OK,
                      evidence->dtc_response, false);

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(mblink_mercedes_module_scan_set_full_target(
              &scan, full_target_index_for_tx(evidence->tx_can_id)));
    CHECK(scan.candidate_tx == UINT32_C(0x602));
    CHECK(scan.candidate_rx == UINT32_C(0x480));
    CHECK(scan.candidate_route_locked);

    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
    CHECK(accept_expected_ok(&scan, "ATCRA480") == 0);
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION);
    CHECK(accept_expected_response(&scan, "1003", &session) == 0);
    CHECK(scan.module_count == 1U);

    CHECK(accept_expected_response(&scan, "3E00", &tester) == 0);
    CHECK(scan.module_count == 1U);
    CHECK(accept_expected_response(&scan, "1902FF", &dtc) == 0);

    CHECK(scan.modules[0].tx_can_id == UINT32_C(0x602));
    CHECK(scan.modules[0].rx_can_id == UINT32_C(0x480));
    CHECK(scan.modules[0].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_AVAILABLE);
    CHECK(scan.modules[0].dtcs.availability_mask == UINT8_C(0x7b));
    CHECK(scan.modules[0].dtcs.count == 1U);
    CHECK(scan.modules[0].dtcs.records[0].code == UINT32_C(0xd18100));
    CHECK(scan.modules[0].dtcs.records[0].status == UINT8_C(0x50));
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    return 0;
}

static int replay_captured_kwp_module_faults(void)
{
    MblinkMercedesModuleScan scan;
    MblinkElm327Response tester =
        make_response(MBLINK_ELM327_RESULT_OK, "7E", false);
    MblinkElm327Response orc_dtcs =
        make_response(MBLINK_ELM327_RESULT_OK, "7F1878\n58019B51E0", false);
    MblinkElm327Response head_unit_dtcs =
        make_response(MBLINK_ELM327_RESULT_OK, "7F1878\n5800", false);
    size_t orc_index;
    size_t head_unit_index;

    CHECK(mblink_mercedes_module_scan_begin_full(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

    /* Exact ORC route and raw DTC record from the vehicle capture. */
    CHECK(mblink_mercedes_module_scan_set_full_target(
              &scan, full_target_index_for_tx(UINT32_C(0x64a))));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
    CHECK(accept_expected_ok(&scan, "ATCRA489") == 0);
    CHECK(accept_expected_response(&scan, "3E01", &tester) == 0);
    CHECK(accept_expected_response(&scan, "1802FF00", &orc_dtcs) == 0);
    CHECK(scan.module_count == 1U);
    orc_index = 0U;
    CHECK(scan.modules[orc_index].tx_can_id == UINT32_C(0x64a));
    CHECK(scan.modules[orc_index].rx_can_id == UINT32_C(0x489));
    CHECK(scan.modules[orc_index].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_AVAILABLE);
    CHECK(scan.modules[orc_index].kwp_dtcs.count == 1U);
    CHECK(scan.modules[orc_index].kwp_dtcs.entries[0].code ==
          UINT16_C(0x9b51));
    CHECK(scan.modules[orc_index].kwp_dtcs.entries[0].status ==
          UINT8_C(0xe0));
    {
        const MblinkMercedesKwpDtcDefinition *definition =
            mblink_mercedes_kwp_dtc_find(
                scan.modules[orc_index].definition->key,
                scan.modules[orc_index].kwp_dtcs.entries[0].code);
        CHECK(definition != NULL);
        CHECK(strstr(definition->description,
                     "Driver seat-belt buckle") != NULL);
    }

    /* The following source-corroborated route returned a valid empty list. */
    CHECK(scan.candidate_tx == UINT32_C(0x652));
    CHECK(scan.candidate_rx == UINT32_C(0x48a));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
    CHECK(accept_expected_ok(&scan, "ATCRA48A") == 0);
    CHECK(accept_expected_response(&scan, "3E01", &tester) == 0);
    CHECK(accept_expected_response(&scan, "1802FF00", &head_unit_dtcs) == 0);
    CHECK(scan.module_count == 2U);
    head_unit_index = 1U;
    CHECK(scan.modules[head_unit_index].tx_can_id == UINT32_C(0x652));
    CHECK(scan.modules[head_unit_index].rx_can_id == UINT32_C(0x48a));
    CHECK(scan.modules[head_unit_index].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_AVAILABLE);
    CHECK(scan.modules[head_unit_index].kwp_dtcs.count == 0U);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 1U);
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
    CHECK(scan.modules[0].controller_family != NULL);
    CHECK(strcmp(scan.modules[0].controller_family->key,
                 "restraints-orc212") == 0);
    CHECK(strcmp(mblink_mercedes_module_scan_module_name(&scan.modules[0]),
                 "ORC_212 restraint controller") == 0);
    CHECK(mblink_mercedes_module_scan_total_dtc_count(&scan) == 1U);
    return 0;
}

static int replay_20260903_field_evidence(void)
{
    MblinkUdsDtcList list;
    const uint8_t ecu602_dtc[] = {
        UINT8_C(0x59), UINT8_C(0x02), UINT8_C(0x7b),
        UINT8_C(0xd1), UINT8_C(0x81), UINT8_C(0x00), UINT8_C(0x50)
    };
    const uint8_t eis_empty_dtc[] = {
        UINT8_C(0x59), UINT8_C(0x02), UINT8_C(0x19)
    };
    const uint8_t engine_empty_dtc[] = {
        UINT8_C(0x59), UINT8_C(0x02), UINT8_C(0xff)
    };
    const uint8_t esp_truncated_dtc[] = {
        UINT8_C(0x59), UINT8_C(0x02), UINT8_C(0x39),
        UINT8_C(0x47), UINT8_C(0x5c), UINT8_C(0x00)
    };

    CHECK(MBLINK_C207_20260903_ROUTE_COUNT == 7U);

    CHECK(mblink_c207_20260903_routes[0].tx_can_id == UINT32_C(0x602));
    CHECK(mblink_c207_20260903_routes[0].rx_can_id == UINT32_C(0x480));
    CHECK(strcmp(mblink_c207_20260903_routes[0].dtc_response,
                 "59027BD1810050") == 0);
    CHECK(mblink_uds_decode_report_dtcs_by_status_mask_response(
              ecu602_dtc, sizeof(ecu602_dtc), &list) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(list.availability_mask == UINT8_C(0x7b));
    CHECK(list.count == 1U);
    CHECK(list.records[0].code == UINT32_C(0xd18100));
    CHECK(list.records[0].status == UINT8_C(0x50));

    CHECK(mblink_c207_20260903_routes[1].tx_can_id == UINT32_C(0x612));
    CHECK(mblink_c207_20260903_routes[1].rx_can_id == UINT32_C(0x482));
    CHECK(mblink_uds_decode_report_dtcs_by_status_mask_response(
              eis_empty_dtc, sizeof(eis_empty_dtc), &list) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(list.availability_mask == UINT8_C(0x19));
    CHECK(list.count == 0U);

    CHECK(mblink_c207_20260903_routes[2].tx_can_id == UINT32_C(0x632));
    CHECK(mblink_c207_20260903_routes[2].rx_can_id == UINT32_C(0x486));
    /*
     * The CSV ended this indexed ELM fragment at 59 02 39 47 5C 00.  After
     * the availability mask only three bytes remain, so there is no complete
     * four-byte DTC/status record.  Reject it rather than inventing status.
     */
    CHECK(mblink_uds_decode_report_dtcs_by_status_mask_response(
              esp_truncated_dtc, sizeof(esp_truncated_dtc), &list) ==
          MBLINK_UDS_RESULT_MALFORMED_PDU);

    CHECK(mblink_c207_20260903_routes[3].tx_can_id == UINT32_C(0x64a));
    CHECK(mblink_c207_20260903_routes[3].rx_can_id == UINT32_C(0x489));
    CHECK(strcmp(mblink_c207_20260903_routes[3].dtc_response,
                 "7F1878\n58019B51E0") == 0);

    CHECK(mblink_c207_20260903_routes[4].tx_can_id == UINT32_C(0x652));
    CHECK(mblink_c207_20260903_routes[4].rx_can_id == UINT32_C(0x48a));
    CHECK(strcmp(mblink_c207_20260903_routes[4].dtc_response,
                 "7F1878\n5800") == 0);

    CHECK(mblink_c207_20260903_routes[5].tx_can_id == UINT32_C(0x7e0));
    CHECK(mblink_c207_20260903_routes[5].rx_can_id == UINT32_C(0x7e8));
    CHECK(mblink_uds_decode_report_dtcs_by_status_mask_response(
              engine_empty_dtc, sizeof(engine_empty_dtc), &list) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(list.availability_mask == UINT8_C(0xff));
    CHECK(list.count == 0U);

    CHECK(mblink_c207_20260903_routes[6].tx_can_id == UINT32_C(0x7e1));
    CHECK(mblink_c207_20260903_routes[6].rx_can_id == UINT32_C(0x7e9));
    CHECK(strcmp(mblink_c207_20260903_routes[6].tester_present_response,
                 "7F3E12") == 0);
    CHECK(strcmp(mblink_c207_20260903_routes[6].dtc_response,
                 "7F1911") == 0);

    return 0;
}

int main(void)
{
    if (replay_20260903_field_evidence() != 0) return 1;
    if (replay_captured_full_scan_misses() != 0) return 1;
    if (replay_captured_negative_tester_present() != 0) return 1;
    if (full_unknown_11_bit_candidate_is_not_forced_to_plus_eight() != 0) return 1;
    if (replay_20260903_unclassified_602_fault() != 0) return 1;
    if (replay_captured_kwp_module_faults() != 0) return 1;
    if (inject_restraint_fault_after_real_scan_shapes() != 0) return 1;
    puts("Captured C207 deep-scan replay tests passed");
    return 0;
}
