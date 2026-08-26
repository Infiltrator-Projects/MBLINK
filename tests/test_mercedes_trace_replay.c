// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_probe.h"
#include "support/elm_trace_replay.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static const MblinkTestElmTraceEntry c207_crd3_fixture[] = {
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
      "5902FF12345609ABCDEF28", false }
};

static int replay_probe_fixture(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    const MblinkMercedesEcuEndpointDefinition *endpoint;
    MblinkMercedesEcuProbe probe;
    MblinkTestElmTraceReplay replay;

    CHECK(profile != NULL);
    endpoint = mblink_mercedes_profile_find_endpoint(
        profile, "c207-om651-engine-eobd-11bit");
    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);

    mblink_test_elm_trace_replay_init(
        &replay,
        c207_crd3_fixture,
        sizeof(c207_crd3_fixture) / sizeof(c207_crd3_fixture[0]));

    while (probe.stage != MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE) {
        char command[MBLINK_ELM327_MAX_COMMAND];
        size_t written = 0U;
        MblinkElm327Response response;
        MblinkMercedesEcuProbeResult result;

        CHECK(probe.stage != MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(written == strlen(command));
        CHECK(mblink_test_elm_trace_replay_next(
            &replay, command, &response));

        result = mblink_mercedes_ecu_probe_accept(&probe, &response);
        CHECK(result == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
              result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    }

    CHECK(mblink_test_elm_trace_replay_complete(&replay));
    CHECK(strcmp(probe.vin, "WDD2073022F123456") == 0);
    CHECK(probe.identity_positive_mask == 0x7ffU);
    CHECK(probe.crd3_positive_mask == 0x1fU);
    CHECK(probe.identity_negative_mask == 0U);
    CHECK(probe.identity_no_response_mask == 0U);
    CHECK(probe.identity_invalid_mask == 0U);
    CHECK(probe.crd3_session_variant_available);
    CHECK(probe.crd3_session_variant.variant == 0x2131U);
    CHECK(probe.crd3_supplier_available);
    CHECK(probe.crd3_supplier.supplier_identifier == 64U);
    CHECK(mblink_mercedes_ecu_probe_matches_om651_cdid3_delphi_signature(
        &probe));
    CHECK(probe.dtc_result == MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE);
    CHECK(probe.dtcs.count == 2U);
    CHECK(probe.dtcs.records[0].code == UINT32_C(0x123456));
    CHECK(probe.dtcs.records[1].code == UINT32_C(0xabcdef));
    return 0;
}

static const MblinkTestElmTraceEntry c207_vehicle_shape_fixture[] = {
    { "ATSH7E0", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "ATCRA7E8", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "ATCAF1", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "ATCFC1", MBLINK_ELM327_RESULT_OK, "OK", true },
    { "3E00", MBLINK_ELM327_RESULT_OK, "7E00", false },
    /* Same indexed framing shape as the vehicle capture, but synthetic VIN. */
    { "22F190", MBLINK_ELM327_RESULT_OK,
      "014\n0:62F190574444\n1:3230373330323246\n2:313233343536", false },
    /* Vehicle adapter padded these declared-length indexed replies with FF. */
    { "22F18C", MBLINK_ELM327_RESULT_OK,
      "00B\n0:62F18C333134\n1:3932333333FFFF", false },
    { "22F187", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    { "22F188", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    { "22F189", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    { "22F191", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    { "22F197", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    { "22F100", MBLINK_ELM327_RESULT_OK, "62F10002110F01", false },
    { "22F154", MBLINK_ELM327_RESULT_OK, "62F1540040", false },
    { "22F196", MBLINK_ELM327_RESULT_OK,
      "009\n0:62F196454430\n1:353037FFFFFFFF", false },
    { "221001", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    { "221002", MBLINK_ELM327_RESULT_OK, "7F2231", false },
    /* Real ECU emitted response-pending followed by the final positive reply. */
    { "1902FF", MBLINK_ELM327_RESULT_OK, "7F1978\n5902FF", false }
};

static int replay_vehicle_capture_shape(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    const MblinkMercedesEcuEndpointDefinition *endpoint;
    MblinkMercedesEcuProbe probe;
    MblinkTestElmTraceReplay replay;

    CHECK(profile != NULL);
    endpoint = mblink_mercedes_profile_find_endpoint(
        profile, "c207-om651-engine-eobd-11bit");
    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);

    mblink_test_elm_trace_replay_init(
        &replay,
        c207_vehicle_shape_fixture,
        sizeof(c207_vehicle_shape_fixture) /
            sizeof(c207_vehicle_shape_fixture[0]));

    while (probe.stage != MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE) {
        char command[MBLINK_ELM327_MAX_COMMAND];
        size_t written = 0U;
        MblinkElm327Response response;
        MblinkMercedesEcuProbeResult result;

        CHECK(probe.stage != MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(mblink_test_elm_trace_replay_next(
            &replay, command, &response));
        result = mblink_mercedes_ecu_probe_accept(&probe, &response);
        CHECK(result == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
              result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    }

    CHECK(mblink_test_elm_trace_replay_complete(&replay));
    CHECK(strcmp(probe.vin, "WDD2073022F123456") == 0);
    CHECK(probe.identity_positive_mask == UINT32_C(0x1c1));
    CHECK(probe.identity_negative_mask == UINT32_C(0x63e));
    CHECK(probe.identity_invalid_mask == 0U);
    CHECK(probe.identity_no_response_mask == 0U);
    CHECK(probe.crd3_positive_mask == UINT32_C(0x07));
    CHECK(probe.crd3_negative_mask == UINT32_C(0x18));
    CHECK(probe.crd3_session_variant_available);
    /*
     * The vehicle returned F154 payload 00 40, not the one-byte layout from
     * the published CRD3 simulator. Preserve the raw positive DID evidence,
     * but do not pretend the supplier encoding has been proven.
     */
    CHECK(!probe.crd3_supplier_available);
    CHECK(probe.dtc_result == MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE);
    CHECK(probe.dtcs.count == 0U);
    return 0;
}

static int replay_detects_command_drift(void)
{
    const MblinkTestElmTraceEntry entries[] = {
        { "22F100", MBLINK_ELM327_RESULT_OK, "62F10002213101", false }
    };
    MblinkTestElmTraceReplay replay;
    MblinkElm327Response response;

    mblink_test_elm_trace_replay_init(&replay, entries, 1U);
    CHECK(!mblink_test_elm_trace_replay_next(&replay, "22F154", &response));
    CHECK(replay.failed);
    CHECK(!mblink_test_elm_trace_replay_complete(&replay));
    return 0;
}

int main(void)
{
    if (replay_probe_fixture() != 0) return 1;
    if (replay_vehicle_capture_shape() != 0) return 1;
    if (replay_detects_command_drift() != 0) return 1;
    puts("Mercedes trace replay tests passed");
    return 0;
}
