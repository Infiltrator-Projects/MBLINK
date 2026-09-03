// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Keep all existing transmission decoder tests and add public-scan regression
 * coverage proving that Mercedes module discovery asks WHAT an ECU is before
 * reading DTCs, including mixed-protocol targets reached through the CGW/N93
 * gateway census.
 */
#include "../include/mblink/mercedes_module_scan.h"

int mblink_transmission_core_main(void);
#define main mblink_transmission_core_main
#include "test_mercedes_transmission_core.inc"
#undef main

static MblinkElm327Response idscan_response(
    MblinkElm327Result result,
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

static int idscan_send_ok(
    MblinkMercedesModuleScan *scan,
    const char *expected)
{
    char command[32];
    size_t written = 0U;
    MblinkElm327Response ok =
        idscan_response(MBLINK_ELM327_RESULT_OK, "OK", true);

    CHECK(mblink_mercedes_module_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, expected) == 0);
    CHECK(written == strlen(expected));
    CHECK(mblink_mercedes_module_scan_accept(scan, &ok) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    return 0;
}

static int test_identity_first_direct_uds(void)
{
    MblinkMercedesModuleScan scan;
    char command[32];
    size_t written = 0U;
    MblinkElm327Response crd3 = idscan_response(
        MBLINK_ELM327_RESULT_OK, "62F19743524433", false);

    CHECK(mblink_mercedes_module_scan_begin(&scan) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.dtc_index == MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL);
    CHECK(idscan_send_ok(&scan, "ATSP6") == 0);
    CHECK(idscan_send_ok(&scan, "ATH0") == 0);
    CHECK(idscan_send_ok(&scan, "ATCAF1") == 0);
    CHECK(idscan_send_ok(&scan, "ATCFC1") == 0);
    CHECK(idscan_send_ok(&scan, "ATST20") == 0);
    CHECK(idscan_send_ok(&scan, "ATSH7E0") == 0);
    CHECK(idscan_send_ok(&scan, "ATCRA7E8") == 0);

    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    CHECK(mblink_mercedes_module_scan_command(
              &scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F197") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &crd3) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.module_count == 1U);
    CHECK(scan.modules[0].identity_available);
    CHECK(strcmp(scan.modules[0].identity, "CRD3") == 0);
    CHECK(scan.modules[0].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED);
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART);
    return 0;
}

static int test_identity_first_kwp_transmission(void)
{
    MblinkMercedesModuleScan scan;
    char command[32];
    size_t written = 0U;
    MblinkElm327Response identity = idscan_response(
        MBLINK_ELM327_RESULT_OK,
        "5A8700080251FF030144020030333235343531343332",
        false);

    memset(&scan, 0, sizeof(scan));
    scan.scope = MBLINK_MERCEDES_MODULE_SCAN_QUICK;
    scan.dtc_index = MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL;
    mblink_mercedes_module_scan_set_11_candidate(&scan, UINT32_C(0x7e1));
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;

    CHECK(mblink_mercedes_module_scan_command(
              &scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "1A87") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &identity) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.module_count == 1U);
    CHECK(scan.modules[0].protocol ==
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
    CHECK(scan.modules[0].definition != NULL);
    CHECK(strcmp(scan.modules[0].definition->key, "transmission-vgs") == 0);
    CHECK(scan.modules[0].identity_available);
    CHECK(strstr(scan.modules[0].identity, "S08 V02 D51") != NULL);
    CHECK(scan.modules[0].spare_part_number_available);
    CHECK(strcmp(scan.modules[0].spare_part_number, "0325451432") == 0);
    CHECK(scan.modules[0].hardware_number_available);
    CHECK(strcmp(scan.modules[0].hardware_number, "03.01") == 0);
    CHECK(scan.modules[0].software_number_available);
    CHECK(strcmp(scan.modules[0].software_number, "44.02.00") == 0);
    CHECK(scan.modules[0].dtc_result ==
          MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED);
    CHECK(scan.candidate_tx == UINT32_C(0x7e2));
    return 0;
}

static int test_identity_first_n93_mixed_protocol(void)
{
    MblinkMercedesModuleScan scan;
    char command[32];
    size_t written = 0U;
    MblinkElm327Response no_data =
        idscan_response(MBLINK_ELM327_RESULT_NO_DATA, "", false);
    MblinkElm327Response gateway_miss =
        idscan_response(MBLINK_ELM327_RESULT_OK, "7F1AA1", false);

    memset(&scan, 0, sizeof(scan));
    scan.scope = MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS;
    scan.dtc_index = MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL;
    scan.full_target_index = 1U; /* 0x60A -> 0x481 in the N93 lattice. */
    scan.candidate_tx = UINT32_C(0x60a);
    scan.candidate_rx = UINT32_C(0x481);
    scan.candidate_extended = false;
    scan.candidate_route_locked = true;
    scan.stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;

    /* First ask the N93-routed target using UDS system-name identity. */
    CHECK(mblink_mercedes_module_scan_command(
              &scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "22F197") == 0);
    CHECK(mblink_mercedes_module_scan_accept(&scan, &no_data) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);

    /* If it is an older KWP ECU behind N93, ask its mandatory DCX identity. */
    CHECK(scan.stage ==
          MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY);
    CHECK(mblink_mercedes_module_scan_command(
              &scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(strcmp(command, "1A87") == 0);

    /* N93 gateway A1 means unknown destination, not a phantom ECU. */
    CHECK(mblink_mercedes_module_scan_accept(&scan, &gateway_miss) ==
          MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    CHECK(scan.module_count == 0U);
    CHECK(scan.candidate_tx == UINT32_C(0x612));
    CHECK(scan.candidate_rx == UINT32_C(0x482));
    return 0;
}

int main(void)
{
    CHECK(mblink_transmission_core_main() == 0);
    CHECK(test_identity_first_direct_uds() == 0);
    CHECK(test_identity_first_kwp_transmission() == 0);
    CHECK(test_identity_first_n93_mixed_protocol() == 0);
    return 0;
}
