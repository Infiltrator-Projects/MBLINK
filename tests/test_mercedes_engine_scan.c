// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_engine_scan.h"
#include "support/elm_trace_replay.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static const MblinkTestElmTraceEntry engine_scan_fixture[] = {
    { "ATSH7E0", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "ATCRA7E8", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "ATCAF1", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "ATCFC1", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "3E00", MBLINK_ELM327_RESULT_OK, "7E00", false },
    { "22F190", MBLINK_ELM327_RESULT_OK,
      "62F1905744443230373330323246313233343536", false },
    { "22F18C", MBLINK_ELM327_RESULT_OK, "62F18C4352443353455249414C", false },
    { "22F187", MBLINK_ELM327_RESULT_OK, "62F1874136353139303030313030", false },
    { "22F188", MBLINK_ELM327_RESULT_OK, "62F188535746303030303031", false },
    { "22F189", MBLINK_ELM327_RESULT_OK, "62F189303031", false },
    { "22F191", MBLINK_ELM327_RESULT_OK, "62F191485746303030303031", false },
    { "22F197", MBLINK_ELM327_RESULT_OK, "62F19743524433", false },
    { "22F100", MBLINK_ELM327_RESULT_OK, "62F10002213101", false },
    { "22F154", MBLINK_ELM327_RESULT_OK, "62F15440", false },
    { "22F196", MBLINK_ELM327_RESULT_OK, "62F196010203040506", false },
    { "221001", MBLINK_ELM327_RESULT_OK, "621001000000001000", false },
    { "221002", MBLINK_ELM327_RESULT_OK, "621002000010", false },
    { "1902FF", MBLINK_ELM327_RESULT_OK,
      "5902FF0112345609ABCDEF28", false }
};

static const MblinkMercedesEcuEndpointDefinition *engine_endpoint(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    if (profile == NULL) {
        return NULL;
    }
    return mblink_mercedes_profile_find_endpoint(
        profile, "c207-om651-engine-eobd-11bit");
}

static int test_full_engine_scan_replay(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEngineScan scan;
    MblinkTestElmTraceReplay replay;

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_engine_scan_begin(&scan, endpoint));
    CHECK(scan.stage == MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE);

    mblink_test_elm_trace_replay_init(
        &replay,
        engine_scan_fixture,
        sizeof(engine_scan_fixture) / sizeof(engine_scan_fixture[0]));

    while (scan.stage != MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE) {
        char command[MBLINK_ELM327_MAX_COMMAND];
        size_t written = 0U;
        MblinkElm327Response response;
        MblinkMercedesEcuProbeResult result;

        CHECK(scan.stage != MBLINK_MERCEDES_ENGINE_SCAN_STAGE_FAILED);
        CHECK(mblink_mercedes_engine_scan_command(
                  &scan, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(written == strlen(command));
        CHECK(mblink_test_elm_trace_replay_next(&replay, command, &response));
        result = mblink_mercedes_engine_scan_accept(&scan, &response);
        CHECK(result == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
              result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    }

    CHECK(mblink_test_elm_trace_replay_complete(&replay));
    CHECK(scan.dtc_result == MBLINK_MERCEDES_ENGINE_DTC_AVAILABLE);
    CHECK(scan.dtcs.count == 2U);
    CHECK(scan.dtcs.records[0].code == UINT32_C(0x123456));
    CHECK(scan.dtcs.records[0].status == 0x09U);
    CHECK(scan.dtcs.records[1].code == UINT32_C(0xabcdef));
    CHECK(scan.dtcs.records[1].status == 0x28U);
    CHECK(strcmp(mblink_mercedes_engine_scan_stage_name(scan.stage),
                 "complete") == 0);
    CHECK(strcmp(mblink_mercedes_engine_dtc_result_name(scan.dtc_result),
                 "available") == 0);
    return 0;
}

static int test_optional_dtc_negative_response(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEngineScan scan;
    MblinkElm327Response response = {0};

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_engine_scan_begin(&scan, endpoint));
    scan.stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_READ_DTCS;
    response.result = MBLINK_ELM327_RESULT_OK;
    memcpy(response.text, "7F1931", 6U);
    response.text[6] = '\0';
    response.length = 6U;

    CHECK(mblink_mercedes_engine_scan_accept(&scan, &response) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    CHECK(scan.stage == MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE);
    CHECK(scan.dtc_result == MBLINK_MERCEDES_ENGINE_DTC_NEGATIVE_RESPONSE);
    CHECK(scan.dtc_negative_response_code == 0x31U);
    return 0;
}

int main(void)
{
    if (test_full_engine_scan_replay() != 0) return 1;
    if (test_optional_dtc_negative_response() != 0) return 1;
    puts("Mercedes engine scan tests passed");
    return 0;
}
