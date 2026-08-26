// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_probe.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static MblinkElm327Response make_response(
    MblinkElm327Result result,
    const char *text,
    bool ok_seen)
{
    MblinkElm327Response response;
    size_t length = text == NULL ? 0U : strlen(text);

    memset(&response, 0, sizeof(response));
    response.result = result;
    response.ok_seen = ok_seen;
    if (length >= sizeof(response.text)) length = sizeof(response.text) - 1U;
    if (length != 0U) memcpy(response.text, text, length);
    response.text[length] = '\0';
    response.length = length;
    return response;
}

static const MblinkMercedesEcuEndpointDefinition *engine_endpoint(void)
{
    const MblinkMercedesVehicleProfile *profile =
        mblink_mercedes_c207_om651_profile();
    return mblink_mercedes_profile_find_endpoint(
        profile, "c207-om651-engine-eobd-11bit");
}

static int advance_configuration(MblinkMercedesEcuProbe *probe)
{
    static const char *commands[] = {
        "ATSH7E0", "ATCRA7E8", "ATCAF1", "ATCFC1"
    };
    MblinkElm327Response ok = make_response(
        MBLINK_ELM327_RESULT_OK, "OK", true);

    for (size_t index = 0U; index < 4U; ++index) {
        char command[32];
        size_t written = 0U;
        CHECK(mblink_mercedes_ecu_probe_command(
                  probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(strcmp(command, commands[index]) == 0);
        CHECK(written == strlen(commands[index]));
        CHECK(mblink_mercedes_ecu_probe_accept(probe, &ok) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    }
    CHECK(probe->stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT);
    return 0;
}

static int advance_tester_present(MblinkMercedesEcuProbe *probe)
{
    MblinkElm327Response reply = make_response(
        MBLINK_ELM327_RESULT_OK, "7E00", false);
    char command[16];
    size_t written = 0U;

    CHECK(advance_configuration(probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_command(
              probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "3E00") == 0);
    CHECK(written == 4U);
    CHECK(mblink_mercedes_ecu_probe_accept(probe, &reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe->stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN);
    return 0;
}

static int advance_standard_vin(MblinkMercedesEcuProbe *probe)
{
    MblinkElm327Response vin_reply = make_response(
        MBLINK_ELM327_RESULT_OK,
        "62F1905744443230373330323246313233343536",
        false);
    char command[16];
    size_t written = 0U;

    CHECK(advance_tester_present(probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_command(
              probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "22F190") == 0);
    CHECK(written == 6U);
    CHECK(mblink_mercedes_ecu_probe_accept(probe, &vin_reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe->stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY);
    CHECK(probe->vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE);
    CHECK(strcmp(probe->vin, "WDD2073022F123456") == 0);
    return 0;
}

static int test_successful_read_only_identity_probe(void)
{
    static const char *identity_commands[] = {
        "22F18C", "22F187", "22F188", "22F189", "22F191", "22F197"
    };
    static const char *identity_responses[] = {
        "62F18C534E313233343536",
        "62F18736353139303131383031",
        "62F18836353139303230303031",
        "62F1894D45313231313031",
        "62F19136353139303430303031",
        "62F197435244332D3635312D574D4134424433"
    };
    static const char *crd3_commands[] = {
        "22F100", "22F154", "22F196", "221001", "221002"
    };
    static const char *crd3_responses[] = {
        "62F10002100001",
        "62F15440",
        "62F196010203040506",
        "621001AABBCCDD",
        "6210021122"
    };
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuProbe probe;

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_identity_did_count() == 11U);
    CHECK(mblink_mercedes_ecu_probe_identity_did_at(0U) == 0xF18CU);
    CHECK(mblink_mercedes_ecu_probe_identity_did_at(5U) == 0xF197U);
    CHECK(mblink_mercedes_ecu_probe_identity_did_at(6U) == 0xF100U);
    CHECK(mblink_mercedes_ecu_probe_identity_did_at(10U) == 0x1002U);
    CHECK(mblink_mercedes_ecu_probe_identity_did_at(11U) == 0U);
    CHECK(strcmp(mblink_mercedes_ecu_probe_identity_did_name(0U),
                 "ECU serial number") == 0);
    CHECK(strcmp(mblink_mercedes_ecu_probe_identity_did_name(6U),
                 "CRD3 session / variant") == 0);
    CHECK(mblink_mercedes_ecu_probe_identity_did_name(11U) == NULL);

    CHECK(mblink_mercedes_ecu_probe_crd3_did_count() == 5U);
    CHECK(mblink_mercedes_ecu_probe_crd3_did_at(0U) == 0xF100U);
    CHECK(mblink_mercedes_ecu_probe_crd3_did_at(4U) == 0x1002U);
    CHECK(mblink_mercedes_ecu_probe_crd3_did_at(5U) == 0U);
    CHECK(strcmp(mblink_mercedes_ecu_probe_crd3_did_name(1U),
                 "CRD3 supplier identifier") == 0);
    CHECK(mblink_mercedes_ecu_probe_crd3_did_name(5U) == NULL);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_standard_vin(&probe) == 0);
    CHECK(strcmp(mblink_mercedes_ecu_probe_stage_name(probe.stage),
                 "read-standard-identity") == 0);

    for (size_t index = 0U; index < 6U; ++index) {
        char command[16];
        size_t written = 0U;
        MblinkElm327Response response = make_response(
            MBLINK_ELM327_RESULT_OK, identity_responses[index], false);

        CHECK(probe.identity_index == index);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(strcmp(command, identity_commands[index]) == 0);
        CHECK(written == 6U);
        CHECK(mblink_mercedes_ecu_probe_accept(&probe, &response) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    }

    CHECK(probe.stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_CRD3_FINGERPRINT);
    CHECK(strcmp(mblink_mercedes_ecu_probe_stage_name(probe.stage),
                 "read-crd3-fingerprint") == 0);

    for (size_t index = 0U; index < 5U; ++index) {
        char command[16];
        size_t written = 0U;
        MblinkElm327Response response = make_response(
            MBLINK_ELM327_RESULT_OK, crd3_responses[index], false);

        CHECK(probe.crd3_index == index);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(strcmp(command, crd3_commands[index]) == 0);
        CHECK(written == 6U);
        CHECK(mblink_mercedes_ecu_probe_accept(&probe, &response) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    }

    CHECK(probe.stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION);
    CHECK(strcmp(mblink_mercedes_ecu_probe_stage_name(probe.stage),
                 "read-dtc-information") == 0);
    CHECK(probe.crd3_session_variant_available);
    CHECK(probe.crd3_session_variant.gateway_mode == 0x02U);
    CHECK(probe.crd3_session_variant.variant == 0x1000U);
    CHECK(probe.crd3_session_variant.session == 0x01U);
    CHECK(probe.crd3_supplier_available);
    CHECK(probe.crd3_supplier.supplier_identifier == 64U);
    CHECK(strcmp(probe.crd3_supplier.supplier_name, "Delphi") == 0);
    CHECK(probe.ecu_spare_part_number_available);
    CHECK(strcmp(probe.ecu_spare_part_number, "6519011801") == 0);
    CHECK(probe.ecu_software_number_available);
    CHECK(strcmp(probe.ecu_software_number, "6519020001") == 0);
    CHECK(probe.ecu_hardware_number_available);
    CHECK(strcmp(probe.ecu_hardware_number, "6519040001") == 0);
    CHECK(probe.ecu_system_name_available);
    CHECK(strcmp(probe.ecu_system_name, "CRD3-651-WMA4BD3") == 0);
    CHECK(probe.crd3_hardware_match ==
          MBLINK_MERCEDES_CRD3_PROFILE_MATCH_STRONG);
    CHECK(probe.crd3_hardware_profile != NULL);
    CHECK(strcmp(probe.crd3_hardware_profile->ecu_family,
                 "Delphi CRD3.10") == 0);
    CHECK(strcmp(probe.crd3_hardware_profile->microcontroller,
                 "Infineon TriCore TC1797") == 0);

    {
        char command[16];
        size_t written = 0U;
        MblinkElm327Response response = make_response(
            MBLINK_ELM327_RESULT_OK,
            "5902FF12345609ABCDEF28",
            false);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(strcmp(command, "1902FF") == 0);
        CHECK(written == 6U);
        CHECK(mblink_mercedes_ecu_probe_accept(&probe, &response) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    }

    CHECK(probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE);
    CHECK(probe.identity_positive_mask == 0x7FFU);
    CHECK(probe.identity_negative_mask == 0U);
    CHECK(probe.identity_no_response_mask == 0U);
    CHECK(probe.identity_invalid_mask == 0U);
    CHECK(probe.crd3_positive_mask == 0x1FU);
    CHECK(probe.crd3_negative_mask == 0U);
    CHECK(probe.crd3_no_response_mask == 0U);
    CHECK(probe.crd3_invalid_mask == 0U);
    CHECK(probe.dtc_result == MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE);
    CHECK(probe.dtcs.count == 2U);
    CHECK(probe.dtcs.records[0].code == UINT32_C(0x123456));
    CHECK(probe.dtcs.records[0].status == 0x09U);
    CHECK(probe.dtcs.records[1].code == UINT32_C(0xabcdef));
    CHECK(probe.dtcs.records[1].status == 0x28U);
    CHECK(strcmp(mblink_mercedes_ecu_probe_dtc_result_name(probe.dtc_result),
                 "available") == 0);
    CHECK(strcmp(mblink_mercedes_ecu_probe_vin_result_name(probe.vin_result),
                 "available") == 0);
    return 0;
}

static int test_optional_identification_failures_continue(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response vin_negative = make_response(
        MBLINK_ELM327_RESULT_OK, "7F2231", false);
    MblinkElm327Response identity_outcomes[6] = {
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false),
        make_response(MBLINK_ELM327_RESULT_OK, "7F2231", false),
        make_response(MBLINK_ELM327_RESULT_OK, "GG", false),
        make_response(MBLINK_ELM327_RESULT_OK, "62F18901", false),
        make_response(MBLINK_ELM327_RESULT_OK, "62F19102", false),
        make_response(MBLINK_ELM327_RESULT_OK, "7F2211", false)
    };
    MblinkElm327Response crd3_outcomes[5] = {
        make_response(MBLINK_ELM327_RESULT_OK, "62F10002100001", false),
        make_response(MBLINK_ELM327_RESULT_NO_DATA, "", false),
        make_response(MBLINK_ELM327_RESULT_OK, "7F2231", false),
        make_response(MBLINK_ELM327_RESULT_OK, "GG", false),
        make_response(MBLINK_ELM327_RESULT_OK, "6210021122", false)
    };
    char command[16];
    size_t written = 0U;

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_tester_present(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "22F190") == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &vin_negative) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.vin_result ==
          MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE);
    CHECK(probe.vin_negative_response_code == 0x31U);

    for (size_t index = 0U; index < 6U; ++index) {
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(mblink_mercedes_ecu_probe_accept(
                  &probe, &identity_outcomes[index]) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    }

    /*
     * VIN was unavailable and the standard identity did not prove CRD3.
     * The safe default is therefore to skip family-specific DIDs.
     */
    CHECK(!probe.crd3_fingerprint_allowed);
    CHECK(!probe.crd3_fingerprint_attempted);
    CHECK(probe.stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION);
    {
        MblinkElm327Response noData = make_response(
            MBLINK_ELM327_RESULT_NO_DATA, "", false);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(strcmp(command, "1902FF") == 0);
        CHECK(mblink_mercedes_ecu_probe_accept(&probe, &noData) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE);
    }

    CHECK(probe.failure == MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.identity_no_response_mask == 0x001U);
    CHECK(probe.identity_negative_mask == 0x022U);
    CHECK(probe.identity_invalid_mask == 0x004U);
    CHECK(probe.identity_positive_mask == 0x018U);
    CHECK(probe.crd3_positive_mask == 0U);
    CHECK(probe.crd3_no_response_mask == 0U);
    CHECK(probe.crd3_negative_mask == 0U);
    CHECK(probe.crd3_invalid_mask == 0U);
    CHECK(!probe.crd3_session_variant_available);
    CHECK(!probe.crd3_supplier_available);
    CHECK(probe.dtc_result == MBLINK_MERCEDES_ECU_PROBE_DTC_NO_RESPONSE);
    return 0;
}

static int test_failure_provenance(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response elm_error = make_response(
        MBLINK_ELM327_RESULT_NO_DATA, "", false);
    MblinkElm327Response malformed = make_response(
        MBLINK_ELM327_RESULT_OK, "GG", false);
    MblinkElm327Response negative = make_response(
        MBLINK_ELM327_RESULT_OK, "7F3E11", false);

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &elm_error) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR);
    CHECK(probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED);
    CHECK(probe.elm_can_failure == MBLINK_ELM327_CAN_RESULT_ELM_ERROR);
    CHECK(probe.elm_failure == MBLINK_ELM327_RESULT_NO_DATA);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_configuration(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &malformed) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR);
    CHECK(probe.elm_can_failure ==
          MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE);
    CHECK(probe.elm_failure == MBLINK_ELM327_RESULT_OK);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_configuration(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &negative) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.stage ==
          MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN);
    CHECK(probe.uds_failure == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE);
    CHECK(probe.uds_negative_response_code == 0x11U);
    CHECK(probe.vin_result == MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED);
    return 0;
}

static int test_argument_and_buffer_failures(void)
{
    const MblinkMercedesEcuEndpointDefinition *endpoint = engine_endpoint();
    MblinkMercedesEcuEndpointDefinition invalid;
    MblinkMercedesEcuProbe probe;
    char buffer[4] = "bad";
    size_t written = 99U;

    CHECK(endpoint != NULL);
    invalid = *endpoint;
    invalid.provenance = "";
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, &invalid) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, buffer, sizeof(buffer), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL);
    CHECK(buffer[0] == '\0');
    CHECK(written == 0U);

    invalid = *endpoint;
    invalid.address.addressing_mode = MBLINK_ISOTP_ADDRESSING_EXTENDED;
    invalid.address.tx_address_extension = 0xf1U;
    invalid.address.rx_address_extension = 0x10U;
    CHECK(mblink_mercedes_ecu_endpoint_is_valid(&invalid));
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, &invalid) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT);

    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    probe.stage = (MblinkMercedesEcuProbeStage)99;
    CHECK(mblink_mercedes_ecu_probe_accept(
              &probe, &(MblinkElm327Response){0}) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE);
    CHECK(strcmp(mblink_mercedes_ecu_probe_dtc_result_name(
                     MBLINK_MERCEDES_ECU_PROBE_DTC_NEGATIVE_RESPONSE),
                 "negative-response") == 0);
    return 0;
}

static int test_petrol_profile_skips_crd3_extension(void)
{
    static const char *identity_responses[] = {
        "62F18C534E504554524F4C",
        "62F18732373131353030333931",
        "62F18832373139303230303031",
        "62F1894D323731",
        "62F19132373131353030333931",
        "62F1974D3237312E383630"
    };
    const MblinkMercedesEcuEndpointDefinition *endpoint =
        mblink_mercedes_generic_engine_endpoint();
    MblinkMercedesEcuProbe probe;
    MblinkElm327Response vin_reply = make_response(
        MBLINK_ELM327_RESULT_OK,
        "62F1905744443230373334373146313233343536",
        false);
    char command[32];
    size_t written = 0U;

    CHECK(endpoint != NULL);
    CHECK(mblink_mercedes_ecu_probe_begin(&probe, endpoint) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(advance_tester_present(&probe) == 0);
    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "22F190") == 0);
    CHECK(mblink_mercedes_ecu_probe_accept(&probe, &vin_reply) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(probe.identified_profile == mblink_mercedes_c207_m271_profile());

    for (size_t index = 0U; index < 6U; ++index) {
        MblinkElm327Response response = make_response(
            MBLINK_ELM327_RESULT_OK, identity_responses[index], false);
        CHECK(mblink_mercedes_ecu_probe_command(
                  &probe, command, sizeof(command), &written) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
        CHECK(mblink_mercedes_ecu_probe_accept(&probe, &response) ==
              MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    }

    CHECK(!probe.crd3_fingerprint_allowed);
    CHECK(!probe.crd3_fingerprint_attempted);
    CHECK(probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION);
    CHECK(mblink_mercedes_ecu_probe_command(
              &probe, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_ECU_PROBE_RESULT_OK);
    CHECK(strcmp(command, "1902FF") == 0);
    return 0;
}

int main(void)
{
    if (test_successful_read_only_identity_probe() != 0) return 1;
    if (test_petrol_profile_skips_crd3_extension() != 0) return 1;
    if (test_optional_identification_failures_continue() != 0) return 1;
    if (test_failure_provenance() != 0) return 1;
    if (test_argument_and_buffer_failures() != 0) return 1;
    puts("Mercedes ECU probe tests passed");
    return 0;
}
