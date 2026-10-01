// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_data_scan.h"
#include "mblink/mercedes_documented_ecus.h"
#include "mblink/mercedes_transmission.h"
#include "mblink/kwp2000.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; \
} } while (0)

static MblinkElm327Response response_ok(const char *text)
{
    MblinkElm327Response response;
    size_t length = text != NULL ? strlen(text) : 0U;
    memset(&response, 0, sizeof(response));
    response.result = MBLINK_ELM327_RESULT_OK;
    response.ok_seen = text != NULL && strcmp(text, "OK") == 0;
    if (length >= sizeof(response.text)) length = sizeof(response.text) - 1U;
    if (length != 0U) memcpy(response.text, text, length);
    response.text[length] = '\0';
    response.length = length;
    return response;
}

static MblinkElm327Response response_no_data(void)
{
    MblinkElm327Response response;
    memset(&response, 0, sizeof(response));
    response.result = MBLINK_ELM327_RESULT_NO_DATA;
    return response;
}

static int accept_command(
    MblinkMercedesDataScan *scan,
    const char *expected,
    MblinkElm327Response response)
{
    char command[32];
    size_t written = 0U;
    CHECK(mblink_mercedes_data_scan_command(
              scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(strcmp(command, expected) == 0);
    CHECK(written == strlen(expected));
    {
        MblinkMercedesDataScanResult result =
            mblink_mercedes_data_scan_accept(scan, &response);
        CHECK(result == MBLINK_MERCEDES_DATA_SCAN_RESULT_OK ||
              result == MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE);

        /*
         * Most tests care about the data operation rather than the cleanup
         * handshake. Transparently complete a documented route teardown here,
         * while the dedicated HU_204 regression below checks the exact 10 81.
         */
        if (scan->stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_QUIT_SESSION) {
            char quit_command[32];
            char documented[5];
            size_t quit_written = 0U;
            MblinkElm327Response no_reply = response_no_data();
            CHECK(mblink_mercedes_documented_route_control_command(
                scan->config.tx_can_id, scan->config.rx_can_id,
                scan->config.extended_id, scan->config.protocol, true,
                documented, sizeof(documented)));
            CHECK(mblink_mercedes_data_scan_command(
                scan, quit_command, sizeof(quit_command), &quit_written) ==
                MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
            CHECK(strcmp(quit_command, documented) == 0);
            CHECK(quit_written == strlen(documented));
            CHECK(mblink_mercedes_data_scan_accept(scan, &no_reply) ==
                MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE);
        }
    }
    return 0;
}

static int configure_to_data(MblinkMercedesDataScan *scan)
{
    MblinkElm327Response ok = response_ok("OK");
    CHECK(accept_command(scan, "ATSP6", ok) == 0);
    CHECK(accept_command(scan, "ATH0", ok) == 0);
    CHECK(accept_command(scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(scan, "ATST64", ok) == 0);
    CHECK(accept_command(scan, "ATSH7E0", ok) == 0);
    CHECK(accept_command(scan, "ATCRA7E8", ok) == 0);
    CHECK(accept_command(scan, "3E00", response_ok("7E00")) == 0);
    return 0;
}

static int test_uds_data_scan(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x7e0), UINT32_C(0x7e8), false,
            MBLINK_MERCEDES_DIAGNOSTIC_UDS,
            MBLINK_MERCEDES_MODULE_ENGINE);
    const MblinkMercedesDataRecord *record;
    char text[64];
    double value = 0.0;
    const char *name = NULL;
    const char *unit = NULL;

    config.first_identifier = UINT16_C(0x2007);
    config.last_identifier = UINT16_C(0x2008);
    CHECK(!config.request_extended_session);
    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(configure_to_data(&scan) == 0);

    CHECK(accept_command(
              &scan, "222007", response_ok("6220070720")) == 0);
    CHECK(scan.current_identifier == UINT16_C(0x2008));
    CHECK(accept_command(
              &scan, "222008", response_ok("7F2231")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.attempted_count == 2U);
    CHECK(scan.positive_count == 1U);
    CHECK(scan.negative_count == 1U);

    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL);
    CHECK(record->identifier == UINT16_C(0x2007));
    CHECK(mblink_mercedes_data_record_format_code(
        record, text, sizeof(text)));
    CHECK(strcmp(text, "UDS DID 0x2007") == 0);
    CHECK(mblink_mercedes_data_record_format_hex(
        record, text, sizeof(text)));
    CHECK(strcmp(text, "0720") == 0);
    CHECK(mblink_mercedes_data_record_decode_known_numeric(
        MBLINK_MERCEDES_MODULE_ENGINE,
        record, &value, &name, &unit));
    CHECK(value == 14.25);
    CHECK(strcmp(name, "Battery voltage") == 0);
    CHECK(strcmp(unit, "V") == 0);
    return 0;
}

static int test_targeted_positive_identifier_refresh(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x64a), UINT32_C(0x489), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_RESTRAINTS);
    const uint16_t identifiers[] = { UINT16_C(0x58), UINT16_C(0xe0) };
    MblinkElm327Response ok = response_ok("OK");
    const MblinkMercedesDataRecord *record;
    char text[64];

    CHECK(mblink_mercedes_data_scan_begin_identifiers(
              &scan, &config, identifiers,
              sizeof(identifiers) / sizeof(identifiers[0])) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(scan.identifier_list_active);
    CHECK(scan.identifier_count == 2U);
    CHECK(scan.current_identifier == UINT16_C(0x58));

    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH64A", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA489", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7E")) == 0);

    /*
     * Regression for the C207 shrink-on-every-refresh fault: a known-positive
     * identifier may hit the adapter timeout transiently.  It must remain the
     * current identifier and be retried rather than being discarded.
     */
    CHECK(accept_command(&scan, "2158", response_no_data()) == 0);
    CHECK(scan.current_identifier == UINT16_C(0x58));
    CHECK(scan.no_response_count == 0U);
    CHECK(scan.current_transient_retries == 1U);
    CHECK(accept_command(&scan, "2158", response_no_data()) == 0);
    CHECK(scan.current_identifier == UINT16_C(0x58));
    CHECK(scan.no_response_count == 0U);
    CHECK(scan.current_transient_retries == 2U);
    CHECK(accept_command(
              &scan, "2158", response_ok("61580090556800")) == 0);
    CHECK(scan.current_transient_retries == 0U);
    CHECK(scan.current_identifier == UINT16_C(0xe0));
    CHECK(accept_command(
              &scan, "21E0", response_ok("014\n0:61E000380406")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.attempted_count == 2U);
    CHECK(scan.positive_count == 2U);

    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x58));
    CHECK(mblink_mercedes_data_record_format_hex(record, text, sizeof(text)));
    CHECK(strcmp(text, "0090556800") == 0);
    record = mblink_mercedes_data_scan_record_at(&scan, 1U);
    CHECK(record != NULL && record->identifier == UINT16_C(0xe0));
    CHECK(mblink_mercedes_data_record_format_hex(record, text, sizeof(text)));
    CHECK(strcmp(text, "00380406") == 0);

    {
        const uint16_t duplicate[] = { UINT16_C(0x58), UINT16_C(0x58) };
        CHECK(mblink_mercedes_data_scan_begin_identifiers(
                  &scan, &config, duplicate, 2U) ==
              MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT);
    }
    {
        const uint16_t invalid_kwp[] = { UINT16_C(0x0100) };
        CHECK(mblink_mercedes_data_scan_begin_identifiers(
                  &scan, &config, invalid_kwp, 1U) ==
              MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT);
    }
    return 0;
}

static int test_source_candidate_identifier_probe(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x7e1), UINT32_C(0x7e9), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_TRANSMISSION);
    const uint16_t identifiers[] = {
        UINT16_C(0x30), UINT16_C(0x31), UINT16_C(0xe1)
    };
    MblinkElm327Response ok = response_ok("OK");

    CHECK(mblink_mercedes_data_scan_begin_probe_identifiers(
              &scan, &config, identifiers,
              sizeof(identifiers) / sizeof(identifiers[0])) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(scan.identifier_list_active);
    CHECK(!scan.identifier_list_retry_no_response);

    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH7E1", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA7E9", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7E")) == 0);

    /* Candidate NO DATA advances immediately: no three-time refresh retry. */
    CHECK(accept_command(&scan, "2130", response_no_data()) == 0);
    CHECK(scan.current_identifier == UINT16_C(0x31));
    CHECK(scan.no_response_count == 1U);
    CHECK(scan.current_transient_retries == 0U);

    CHECK(accept_command(&scan, "2131", response_no_data()) == 0);
    CHECK(scan.current_identifier == UINT16_C(0xe1));
    CHECK(scan.no_response_count == 2U);

    CHECK(accept_command(&scan, "21E1", response_ok("61E131323334")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.attempted_count == 3U);
    CHECK(scan.positive_count == 1U);
    return 0;
}

static int test_c207_vehicle_verified_raw_positives(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config;
    MblinkElm327Response ok = response_ok("OK");
    const MblinkMercedesDataRecord *record;
    char text[MBLINK_MERCEDES_DATA_SCAN_MAX_DATA * 2U + 1U];

    /*
     * Vehicle evidence captured on the C207 proves these identifiers respond
     * on these exact physical routes.  Their semantics are intentionally not
     * guessed here: this regression locks down routing, positive-response
     * handling and exact raw payload retention only.
     */
    config = mblink_mercedes_data_scan_default_config(
        UINT32_C(0x632), UINT32_C(0x486), false,
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_MODULE_ABS_ESP);
    config.first_identifier = UINT16_C(0x2001);
    config.last_identifier = UINT16_C(0x2001);
    CHECK(!config.request_extended_session);
    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH632", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA486", ok) == 0);
    /* Safety regression: automatic ESP reads stay in the default diagnostic
     * session.  10 03 must never appear between route setup and TesterPresent. */
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT);
    CHECK(accept_command(&scan, "3E00", response_ok("7E00")) == 0);

    /*
     * The road capture contained a late 62 F154 response after 22 F155 was
     * already on the wire. A stale positive for a different DID must never be
     * consumed as the current value or advance the scan.
     */
    CHECK(accept_command(
              &scan, "222001", response_ok("62200405110000")) == 0);
    CHECK(scan.current_identifier == UINT16_C(0x2001));
    CHECK(scan.current_transient_retries == 1U);
    CHECK(scan.positive_count == 0U);
    CHECK(accept_command(
              &scan, "222001", response_ok("011\n0:622001061A06")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.positive_count == 1U);
    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x2001));
    CHECK(mblink_mercedes_data_record_format_hex(record, text, sizeof(text)));
    CHECK(strcmp(text, "061A06") == 0);

    config = mblink_mercedes_data_scan_default_config(
        UINT32_C(0x64a), UINT32_C(0x489), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        MBLINK_MERCEDES_MODULE_RESTRAINTS);
    config.first_identifier = UINT16_C(0x58);
    config.last_identifier = UINT16_C(0x58);
    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH64A", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA489", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7E")) == 0);
    CHECK(accept_command(
              &scan, "2158", response_ok("61580090556800")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x58));
    CHECK(mblink_mercedes_data_record_format_hex(record, text, sizeof(text)));
    CHECK(strcmp(text, "0090556800") == 0);
    CHECK(!mblink_mercedes_data_record_decode_known_numeric(
        MBLINK_MERCEDES_MODULE_RESTRAINTS,
        record, &(double){0.0}, &(const char *){0}, &(const char *){0}));

    config = mblink_mercedes_data_scan_default_config(
        UINT32_C(0x652), UINT32_C(0x48a), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        MBLINK_MERCEDES_MODULE_OTHER);
    config.first_identifier = UINT16_C(0x01);
    config.last_identifier = UINT16_C(0x01);
    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH652", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA48A", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7E")) == 0);
    CHECK(accept_command(
              &scan, "2101", response_ok("7F2178\n012\n0:610110102210")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x01));
    CHECK(mblink_mercedes_data_record_format_hex(record, text, sizeof(text)));
    CHECK(strcmp(text, "10102210") == 0);

    return 0;
}

static int test_7e1_transmission_temperature_candidate(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x7e1), UINT32_C(0x7e9), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_TRANSMISSION);
    MblinkElm327Response ok = response_ok("OK");
    const MblinkMercedesDataRecord *record;
    double value = 0.0;
    const char *name = NULL;
    const char *unit = NULL;

    /*
     * Generic scan configuration no longer derives a transmission data set
     * from the 7E1/7E9 address. This decoder regression supplies the exact
     * identifier explicitly; controller-family profiles own automatic lists.
     */
    CHECK(config.protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
    CHECK(!config.request_extended_session);
    config.first_identifier = UINT16_C(0x30);
    config.last_identifier = UINT16_C(0x30);

    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH7E1", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA7E9", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7F3E12")) == 0);

    /*
     * Synthetic positive response shape used only to exercise the published
     * L-50 formula.  0x64 at complete-response byte 11 = 50 deg C.
     */
    CHECK(accept_command(
              &scan, "2130",
              response_ok("613000000000000000000064")) == 0);
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.positive_count == 1U);

    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL);
    CHECK(record->service ==
          MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER);
    CHECK(record->identifier == UINT16_C(0x30));
    CHECK(record->data_length == 10U);
    CHECK(record->data[9] == UINT8_C(0x64));
    CHECK(mblink_mercedes_data_record_decode_known_numeric_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        record, &value, &name, &unit));
    CHECK(value == 50.0);
    CHECK(strcmp(name, "Transmission oil temperature") == 0);
    CHECK(strcmp(unit, "°C") == 0);
    {
        char structured[256];
        const char *structured_name = NULL;
        CHECK(mblink_mercedes_data_record_format_known_for_route(
            UINT32_C(0x7e1), UINT32_C(0x7e9), false,
            MBLINK_MERCEDES_MODULE_TRANSMISSION,
            record, structured, sizeof(structured), &structured_name));
        CHECK(strcmp(structured_name, "Transmission actual values") == 0);
        CHECK(strstr(structured, "ATF 50.0 °C") != NULL);
    }

    /* The same bytes must not be promoted on an unrelated ECU route. */
    CHECK(!mblink_mercedes_data_record_decode_known_numeric_for_route(
        UINT32_C(0x64a), UINT32_C(0x489), false,
        MBLINK_MERCEDES_MODULE_RESTRAINTS,
        record, &value, &name, &unit));
    return 0;
}

static int test_full_rli30_numeric_prefers_full_layout(void)
{
    MblinkMercedesDataRecord record;
    double value = 0.0;
    const char *name = NULL;
    const char *unit = NULL;

    memset(&record, 0, sizeof(record));
    record.service =
        MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER;
    record.identifier = UINT16_C(0x30);
    record.data_length = 24U;

    /*
     * Deliberately make compact-layout data[9] nonsensical (0x01 -> -49 C).
     * Full EGS52 RLI30 puts packed gears at data[10] and ATF at data[11].
     */
    record.data[9] = UINT8_C(0x01);
    record.data[10] = UINT8_C(0x54); /* target 5, actual 4 */
    record.data[11] = UINT8_C(0x64); /* 50 C */
    record.data[12] = UINT8_C(0xff);
    record.data[13] = UINT8_C(0xdb); /* captured -37 signed raw */
    record.data[14] = UINT8_C(0xff);
    record.data[15] = UINT8_C(0xdb); /* captured -37 signed raw */

    CHECK(mblink_mercedes_data_record_decode_known_numeric_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, &value, &name, &unit));
    CHECK(value == 50.0);
    CHECK(strcmp(name, "Transmission oil temperature") == 0);
    CHECK(strcmp(unit, "°C") == 0);
    {
        char structured[512];
        const char *structured_name = NULL;
        CHECK(mblink_mercedes_data_record_format_known_for_route(
            UINT32_C(0x7e1), UINT32_C(0x7e9), false,
            MBLINK_MERCEDES_MODULE_TRANSMISSION,
            &record, structured, sizeof(structured), &structured_name));
        CHECK(strcmp(structured_name, "Transmission actual values") == 0);
        CHECK(strstr(structured, "engine torque signed raw -37") != NULL);
        CHECK(strstr(structured, "converter torque signed raw -37") != NULL);
        CHECK(strstr(structured, "65499") == NULL);
    }
    return 0;
}

static int test_transmission_standard_kwp_metadata(void)
{
    MblinkMercedesDataRecord record;
    char text[512];
    const char *name = NULL;

    memset(&record, 0, sizeof(record));
    record.service = MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER;

    record.identifier = UINT16_C(0xe0);
    record.data_length = 18U;
    for (size_t index = 0U; index < record.data_length; ++index)
        record.data[index] = (uint8_t)(index + 1U);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Development data") == 0);
    CHECK(strstr(text, "processor 0x0102") != NULL);
    CHECK(strstr(text, "KWP 09.0A") != NULL);

    record.identifier = UINT16_C(0xe2);
    record.data_length = 19U;
    memset(record.data, 0, sizeof(record.data));
    record.data[0] = 0x01; record.data[1] = 0x02; record.data[2] = 0x03;
    record.data[3] = 0x44;
    record.data[4] = 0x00; record.data[5] = 0x10; record.data[6] = 0x00;
    record.data[7] = 0x04; record.data[8] = 0x05; record.data[9] = 0x06;
    record.data[10] = 0x00; record.data[11] = 0x20; record.data[12] = 0x00;
    record.data[13] = 0x07; record.data[14] = 0x08; record.data[15] = 0x09;
    record.data[16] = 0x00; record.data[17] = 0x00; record.data[18] = 0x80;
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "DBCom communication-matrix data") == 0);
    CHECK(strstr(text, "Flash 0x010203 +4096 bytes") != NULL);
    CHECK(strstr(text, "RAM 0x040506 +8192 bytes") != NULL);
    CHECK(strstr(text, "EEPROM 0x070809 +128 bytes") != NULL);

    record.identifier = UINT16_C(0xe3);
    record.data_length = 3U;
    record.data[0] = UINT8_C(0x12);
    record.data[1] = UINT8_C(0x34);
    record.data[2] = UINT8_C(0x56);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Operating-system version") == 0);
    CHECK(strcmp(text, "0x123456") == 0);

    record.identifier = UINT16_C(0xe7);
    record.data_length = 19U;
    memset(record.data, 0xaa, record.data_length);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Flash information 2") == 0);
    CHECK(strstr(text, "19-byte hardware-scan") != NULL);

    record.identifier = UINT16_C(0xe1);
    record.data_length = 4U;
    record.data[0] = UINT8_C(0x01);
    record.data[1] = UINT8_C(0x02);
    record.data[2] = UINT8_C(0x03);
    record.data[3] = UINT8_C(0x04);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "ECU serial number") == 0);
    CHECK(strstr(text, "retained raw") != NULL);

    /* Standard Daimler KWP metadata is not tied to the 7E1/7E9 route. */
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x0640), UINT32_C(0x0480), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "ECU serial number") == 0);

    record.identifier = UINT16_C(0xe5);
    record.data_length = 4U;
    record.data[0] = UINT8_C(0x11);
    record.data[1] = UINT8_C(0x22);
    record.data[2] = UINT8_C(0x33);
    record.data[3] = UINT8_C(0x44);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Vehicle information") == 0);
    CHECK(strstr(text, "model year 0x11") != NULL);
    CHECK(strstr(text, "country 0x44") != NULL);

    record.identifier = UINT16_C(0xe8);
    record.data_length = 15U;
    for (size_t index = 0U; index < record.data_length; ++index)
        record.data[index] = (uint8_t)(index + 1U);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "System-diagnostic general parameters") == 0);
    CHECK(strstr(text, "SDCOM version 02.03.04") != NULL);
    CHECK(strstr(text, "checksum 0D0E0F") != NULL);

    record.identifier = UINT16_C(0xe9);
    record.data_length = 7U;
    record.data[0] = UINT8_C(3);
    record.data[1] = UINT8_C(4);
    record.data[2] = UINT8_C(0x01);
    record.data[3] = UINT8_C(0x23);
    record.data[4] = UINT8_C(0xaa);
    record.data[5] = UINT8_C(0xbb);
    record.data[6] = UINT8_C(0xcc);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "System-diagnostic global parameters") == 0);
    CHECK(strstr(text, "global analog values 3") != NULL);
    CHECK(strstr(text, "first CAN data-frame position 291") != NULL);
    CHECK(strstr(text, "3 descriptor bytes retained") != NULL);

    record.identifier = UINT16_C(0xeb);
    record.data_length = 3U;
    record.data[0] = UINT8_C(0x22);
    record.data[1] = UINT8_C(0x10);
    record.data[2] = UINT8_C(0x03);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Diagnostic protocol information") == 0);
    CHECK(strstr(text, "diagnostic level 3") != NULL);

    return 0;
}

static int test_kwp_local_identifier_scan(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x64a), UINT32_C(0x489), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_RESTRAINTS);
    MblinkElm327Response ok = response_ok("OK");
    const MblinkMercedesDataRecord *record;
    char text[64];

    config.first_identifier = UINT16_C(0x01);
    config.last_identifier = UINT16_C(0x02);
    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH64A", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA489", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7E")) == 0);
    CHECK(accept_command(&scan, "2101", response_ok("6101AABB")) == 0);
    CHECK(accept_command(&scan, "2102", response_no_data()) == 0);

    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.positive_count == 1U);
    CHECK(scan.no_response_count == 1U);
    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x01));
    CHECK(mblink_mercedes_data_record_format_code(
        record, text, sizeof(text)));
    CHECK(strcmp(text, "KWP local ID 0x01") == 0);
    CHECK(mblink_mercedes_data_record_format_hex(
        record, text, sizeof(text)));
    CHECK(strcmp(text, "AABB") == 0);
    CHECK(!mblink_mercedes_data_record_decode_known_numeric(
        MBLINK_MERCEDES_MODULE_RESTRAINTS,
        record, &(double){0.0}, &(const char *){0}, &(const char *){0}));
    return 0;
}

static bool route_evidence_has(
    uint32_t tx, uint32_t rx,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesModuleKind kind,
    uint16_t identifier,
    bool *live)
{
    const size_t count = mblink_mercedes_route_evidence_identifier_count(
        tx, rx, false, protocol, kind);
    size_t index;
    for (index = 0U; index < count; ++index) {
        const MblinkMercedesRouteEvidenceEntry *entry =
            mblink_mercedes_route_evidence_identifier_at(
                tx, rx, false, protocol, kind, index);
        if (entry != NULL && entry->identifier == identifier) {
            if (live != NULL) *live = entry->live;
            return true;
        }
    }
    return false;
}

static int test_runtime_candidate_catalog(void)
{
    static const uint16_t esp_ids[] = {
        UINT16_C(0x2001), UINT16_C(0x2003), UINT16_C(0x2004),
        UINT16_C(0x2007), UINT16_C(0x2009), UINT16_C(0x200a),
        UINT16_C(0x200d), UINT16_C(0x200f), UINT16_C(0x2010),
        UINT16_C(0x2014), UINT16_C(0x2017), UINT16_C(0x2023),
        UINT16_C(0x2043), UINT16_C(0x2046), UINT16_C(0x2047),
        UINT16_C(0x2070), UINT16_C(0x20c0), UINT16_C(0x20df)
    };
    static const uint16_t orc_ids[] = {
        UINT16_C(0x01), UINT16_C(0x02), UINT16_C(0x07),
        UINT16_C(0x0d), UINT16_C(0x0f), UINT16_C(0x11),
        UINT16_C(0x13), UINT16_C(0x18), UINT16_C(0x23),
        UINT16_C(0x24), UINT16_C(0x2b), UINT16_C(0x2d),
        UINT16_C(0x51), UINT16_C(0x52), UINT16_C(0x58),
        UINT16_C(0x59), UINT16_C(0x60), UINT16_C(0x61),
        UINT16_C(0x62), UINT16_C(0x63), UINT16_C(0x64),
        UINT16_C(0x65), UINT16_C(0x69), UINT16_C(0x70),
        UINT16_C(0x71), UINT16_C(0x72), UINT16_C(0x77),
        UINT16_C(0xe0), UINT16_C(0xe4)
    };
    static const uint16_t hu_ids[] = {
        UINT16_C(0x01), UINT16_C(0x02),
        UINT16_C(0x05), UINT16_C(0x06)
    };
    const MblinkMercedesControllerDataProfileEntry *entry;
    size_t index;
    bool live = false;

    /*
     * Public Vediamo CBF Data services are catalogue facts. They must exist
     * even before any vehicle has answered the DID.
     */
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "gateway-cgw212", MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 11U);
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "camera-mfk", MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 16U);
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "fuel-pump-fscu", MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 6U);
    entry = mblink_mercedes_controller_data_profile_find(
        "gateway-cgw212", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT16_C(0xD243));
    CHECK(entry != NULL && entry->source_dynamic_hint &&
          entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    entry = mblink_mercedes_controller_data_profile_find(
        "camera-mfk", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT16_C(0x0220));
    CHECK(entry != NULL && entry->source_dynamic_hint &&
          entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    entry = mblink_mercedes_controller_data_profile_find(
        "fuel-pump-fscu", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT16_C(0x000B));
    CHECK(entry != NULL && entry->source_dynamic_hint &&
          entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);

    CHECK(strcmp(mblink_mercedes_data_profile_key_for_controller(
        "engine-cdi", "CRD3", NULL, NULL), "engine-crd3") == 0);
    CHECK(mblink_mercedes_data_profile_key_for_controller(
        "engine-cdi", "EDC17", NULL, NULL) == NULL);
    CHECK(strcmp(mblink_mercedes_data_profile_key_for_controller(
        "esp", "ABR2XT", NULL, NULL), "esp-abr2xt") == 0);
    CHECK(strcmp(mblink_mercedes_data_profile_key_for_controller(
        "restraints-orc", "ORC_212", NULL, NULL),
        "restraints-orc212") == 0);
    CHECK(strcmp(mblink_mercedes_data_profile_key_for_controller(
        "audio-headunit", "HU_204", NULL, NULL),
        "headunit-hu204") == 0);

    /*
     * ABR2XT.cbf contributes 18 unique documented Data DIDs. Seven additional
     * DIDs remain vehicle-observed raw evidence, so the merged profile owns 25
     * unique wire identifiers without letting the capture define semantics.
     */
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 25U);
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) ==
        sizeof(orc_ids) / sizeof(orc_ids[0]));
    /*
     * HU_204.cbf contributes 24 exact KWP Data records. Captured 0x02 is not
     * present in that CBF and remains one additional raw observation.
     */
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) == 25U);

    for (index = 0U; index < sizeof(esp_ids) / sizeof(esp_ids[0]); ++index) {
        CHECK(mblink_mercedes_controller_data_profile_find(
            "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
            esp_ids[index]) != NULL);
        CHECK(route_evidence_has(
            UINT32_C(0x632), UINT32_C(0x486),
            MBLINK_MERCEDES_DIAGNOSTIC_UDS,
            MBLINK_MERCEDES_MODULE_ABS_ESP, esp_ids[index], NULL));
    }
    for (index = 0U; index < sizeof(orc_ids) / sizeof(orc_ids[0]); ++index) {
        CHECK(mblink_mercedes_controller_data_profile_find(
            "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            orc_ids[index]) != NULL);
        CHECK(route_evidence_has(
            UINT32_C(0x64a), UINT32_C(0x489),
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_RESTRAINTS, orc_ids[index], NULL));
    }
    for (index = 0U; index < sizeof(hu_ids) / sizeof(hu_ids[0]); ++index) {
        CHECK(mblink_mercedes_controller_data_profile_find(
            "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            hu_ids[index]) != NULL);
        CHECK(route_evidence_has(
            UINT32_C(0x652), UINT32_C(0x48a),
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_BODY, hu_ids[index], NULL));
    }
    for (index = UINT16_C(0x30); index <= UINT16_C(0x33); ++index) {
        live = false;
        CHECK(route_evidence_has(
            UINT32_C(0x7e1), UINT32_C(0x7e9),
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_TRANSMISSION,
            (uint16_t)index, &live));
        CHECK(live);
    }

    CHECK(mblink_mercedes_route_evidence_identifier_count(
        UINT32_C(0x632), UINT32_C(0x486), false,
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_MODULE_ENGINE) == 0U);
    CHECK(mblink_mercedes_route_evidence_identifier_count(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_MODULE_TRANSMISSION) == 0U);

    entry = mblink_mercedes_controller_data_profile_find(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS, UINT16_C(0x200d));
    CHECK(entry != NULL && entry->source_dynamic_hint);
    entry = mblink_mercedes_controller_data_profile_find(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS, UINT16_C(0x2003));
    CHECK(entry != NULL && entry->source_dynamic_hint &&
          entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    entry = mblink_mercedes_controller_data_profile_find(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS, UINT16_C(0x2009));
    CHECK(entry != NULL && entry->source_dynamic_hint &&
          entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    entry = mblink_mercedes_controller_data_profile_find(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS, UINT16_C(0x20c0));
    CHECK(entry != NULL && !entry->source_dynamic_hint);
    entry = mblink_mercedes_controller_data_profile_find(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS, UINT16_C(0x2010));
    CHECK(entry != NULL && entry->source_dynamic_hint &&
          entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    entry = mblink_mercedes_controller_data_profile_find(
        "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT16_C(0x0058));
    CHECK(entry != NULL && !entry->source_dynamic_hint);
    entry = mblink_mercedes_controller_data_profile_find(
        "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT16_C(0x0005));
    CHECK(entry != NULL && entry->source_dynamic_hint);
    /*
     * Recurring polling membership is not duplicated in this evidence layer.
     * The resolved ECU pack owns STARTUP_ONCE versus USER_POLLING.
     */

    return 0;
}

static int test_20260903_transmission_capture_replay(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x7e1), UINT32_C(0x7e9), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_TRANSMISSION);
    static const uint16_t identifiers[] = {
        UINT16_C(0x30), UINT16_C(0x31),
        UINT16_C(0x32), UINT16_C(0x33)
    };
    MblinkElm327Response ok = response_ok("OK");
    const MblinkMercedesDataRecord *record;
    MblinkMercedesKwpRli30 rli30;
    MblinkMercedesKwpRli31 rli31;
    MblinkMercedesKwpRli32 rli32;
    MblinkMercedesKwpRli33 rli33;

    CHECK(mblink_mercedes_data_scan_begin_probe_identifiers(
        &scan, &config, identifiers,
        sizeof(identifiers) / sizeof(identifiers[0])) ==
        MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH7E1", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA7E9", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7F3E12")) == 0);

    CHECK(accept_command(&scan, "2130", response_ok(
        "01A\n0:613000150000\n1:000000080100DD\n"
        "2:59FFF0FFF00000\n3:861900080000FF")) == 0);
    CHECK(accept_command(&scan, "2131", response_ok(
        "016\n0:613101EC0000\n1:0329033C000000\n"
        "2:00000000000000\n3:0000FFFFFFFFFF")) == 0);
    CHECK(accept_command(&scan, "2132", response_ok(
        "00F\n0:613200000000\n1:00000000000000\n"
        "2:0000FFFFFFFFFF")) == 0);
    CHECK(accept_command(&scan, "2133", response_ok(
        "012\n0:6133002407B7\n1:03E802A602A603\n"
        "2:1403150000FFFF")) == 0);

    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    CHECK(scan.positive_count == 4U);

    record = mblink_mercedes_data_scan_record_at(&scan, 0U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x30));
    CHECK(mblink_mercedes_transmission_decode_kwp_rli30(
        record->data, record->data_length, &rli30));
    CHECK(rli30.atf_temperature_c == 39.0);
    CHECK(rli30.actual_gear_code == UINT8_C(13));
    CHECK(rli30.target_gear_code == UINT8_C(13));
    CHECK(strcmp(mblink_mercedes_transmission_actual_gear_name(
        rli30.actual_gear_code), "P") == 0);

    record = mblink_mercedes_data_scan_record_at(&scan, 1U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x31));
    CHECK(mblink_mercedes_transmission_decode_kwp_rli31(
        record->data, record->data_length, &rli31));
    CHECK(rli31.input_rpm == UINT16_C(809));
    CHECK(rli31.engine_rpm == UINT16_C(828));

    record = mblink_mercedes_data_scan_record_at(&scan, 2U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x32));
    CHECK(mblink_mercedes_transmission_decode_kwp_rli32(
        record->data, record->data_length, &rli32));
    CHECK(rli32.pedal_percent == UINT8_C(0));

    record = mblink_mercedes_data_scan_record_at(&scan, 3U);
    CHECK(record != NULL && record->identifier == UINT16_C(0x33));
    CHECK(mblink_mercedes_transmission_decode_kwp_rli33(
        record->data, record->data_length, &rli33));
    CHECK(rli33.spc_pressure_raw == UINT16_C(1975));
    CHECK(rli33.mpc_pressure_raw == UINT16_C(1000));
    CHECK(rli33.spc_target_current_raw == UINT16_C(678));
    CHECK(rli33.spc_actual_current_raw == UINT16_C(678));
    CHECK(rli33.mpc_target_current_raw == UINT16_C(788));
    CHECK(rli33.mpc_actual_current_raw == UINT16_C(789));
    return 0;
}

static int test_documented_global_ecu_catalog(void)
{
    static const struct {uint32_t tx;uint32_t rx;} routes[]={{0x602,0x480},{0x60a,0x481},{0x612,0x482},{0x622,0x484},{0x632,0x486},{0x64a,0x489},{0x652,0x48a},{0x6a2,0x494},{0x6ba,0x497},{0x6c2,0x498},{0x6fa,0x49f},{0x7e0,0x7e8},{0x7e1,0x7e9}};
    CHECK(mblink_mercedes_documented_ecu_profile_count()==1319U);
    for(size_t i=0U;i<sizeof(routes)/sizeof(routes[0]);++i)CHECK(mblink_mercedes_documented_ecu_profile_count_for_route(routes[i].tx,routes[i].rx,false)>0U);
    CHECK(mblink_mercedes_documented_ecu_profile_count_for_route(0x7e1,0x7e9,false)>3U);
    const MblinkMercedesDocumentedEcuProfile*p=mblink_mercedes_documented_ecu_profile_for_controller_family("transmission-egs53",0x7e1,0x7e9,false,MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
    CHECK(p!=NULL&&strcmp(p->name,"EGS53")==0&&p->read_count==4U);
    bool a=false,b=false,c=false,d=false;for(size_t i=0U;i<p->read_count;++i){const MblinkMercedesDocumentedRead*r=mblink_mercedes_documented_ecu_read_at(p,i);if(r->service==0x1a&&r->identifier==0x86)a=true;if(r->service==0x1a&&r->identifier==0x9a)b=true;if(r->service==0x1a&&r->identifier==0x9c)c=true;if(r->service==0x21&&r->identifier==0xb1)d=true;}CHECK(a&&b&&c&&d);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("gateway-cgw204",0x602,0x480,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"CGW_204")==0&&p->read_count==9U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("cluster-ic204",0x60a,0x481,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"IC_204")==0&&p->read_count==9U);
    CHECK(mblink_mercedes_documented_ecu_profile_for_controller_family("cluster-ic204",0x60a,0x481,false,MBLINK_MERCEDES_DIAGNOSTIC_KWP2000)==NULL);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("eis-ezs204",0x612,0x482,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"EIS_204")==0&&p->read_count==9U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("steering-sccm204",0x622,0x484,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"SCCM_204_X")==0&&p->read_count==2U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("steering-sccm212",0x622,0x484,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"SCCM_212_X")==0&&p->read_count==8U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("steering-scm",0x622,0x484,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"SCCM_212_X")==0);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("restraints-orc212",0x64a,0x489,false,MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);CHECK(p!=NULL&&strcmp(p->name,"ORC_212_X")==0&&p->read_count==5U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("headunit-hu204",0x652,0x48a,false,MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);CHECK(p!=NULL&&strcmp(p->name,"HU_204")==0&&p->read_count==8U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("camera-mfk",0x6a2,0x494,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"MPC212")==0&&p->read_count==9U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("pretensioner-rbtmfl204",0x6ba,0x497,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"RBTMFL_204")==0&&p->read_count==9U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("pretensioner-rbtmfr204",0x6c2,0x498,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"RBTMFR_204")==0&&p->read_count==9U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("fuel-pump-fscu",0x6fa,0x49f,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"FSCM212")==0&&p->read_count==10U);
    p=mblink_mercedes_documented_ecu_profile_for_controller_family("engine-crd3",0x7e0,0x7e8,false,MBLINK_MERCEDES_DIAGNOSTIC_UDS);CHECK(p!=NULL&&strcmp(p->name,"CRD3")==0&&p->read_count==6U);
    CHECK(mblink_mercedes_documented_field_count(0x22,0xf150)==3U);
    CHECK(mblink_mercedes_documented_read_layout(0x22,0xf150)==
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH);
    CHECK(mblink_mercedes_documented_read_layout(0x22,0xf151)==
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH);
    CHECK(mblink_mercedes_documented_read_layout(0x22,0xf153)==
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH);
    CHECK(mblink_mercedes_documented_read_layout(0x22,0xf111)==
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_DEFAULT);
    const MblinkMercedesDocumentedField*f=mblink_mercedes_documented_field_at(0x22,0xf150,0U);CHECK(f!=NULL&&f->response_byte==4U&&strcmp(f->name,"Hardware version year")==0);
    CHECK(mblink_mercedes_documented_read_is_safe(0x22,0xf150));CHECK(mblink_mercedes_documented_read_is_safe(0x1a,0x86));CHECK(!mblink_mercedes_documented_read_is_safe(0x27,1));CHECK(!mblink_mercedes_documented_read_is_safe(0x31,1));
    return 0;
}
static int test_documented_uds_version_metadata(void)
{
    MblinkMercedesDataRecord record;
    char text[64];
    const char *name = NULL;

    memset(&record, 0, sizeof(record));
    record.service = UINT8_C(0x22);
    record.data_length = 3U;

    record.identifier = UINT16_C(0xf150);
    record.data[0] = UINT8_C(0x08);
    record.data[1] = UINT8_C(0x2b);
    record.data[2] = UINT8_C(0x01);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Hardware version") == 0);
    CHECK(strcmp(
        text,
        "08/43.01") == 0);

    record.identifier = UINT16_C(0xf151);
    record.data[0] = UINT8_C(0x0a);
    record.data[1] = UINT8_C(0x1d);
    record.data[2] = UINT8_C(0x4b);
    name = NULL;
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Software version") == 0);
    CHECK(strcmp(
        text,
        "10/29.75") == 0);

    record.identifier = UINT16_C(0xf153);
    record.data[2] = UINT8_C(0x48);
    name = NULL;
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Boot software version") == 0);
    CHECK(strcmp(
        text,
        "10/29.72") == 0);

    record.data_length = 2U;
    name = NULL;
    CHECK(!mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    return 0;
}

static int test_documented_uds_short_part_identifiers(void)
{
    MblinkMercedesDataRecord record;
    char text[64];
    const char *name = NULL;

    memset(&record, 0, sizeof(record));
    record.service = UINT8_C(0x22);
    record.data_length = 3U;
    record.data[0] = UINT8_C('2');
    record.data[1] = UINT8_C('1');
    record.data[2] = UINT8_C('2');

    record.identifier = UINT16_C(0xf111);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Mercedes hardware identifier") == 0);
    CHECK(strcmp(text, "212") == 0);

    record.identifier = UINT16_C(0xf121);
    name = NULL;
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Mercedes software identifier") == 0);
    CHECK(strcmp(text, "212") == 0);

    /*
     * A complete printable response remains a part number. Only the short
     * three-digit family form is downgraded to an identifier.
     */
    record.identifier = UINT16_C(0xf111);
    record.data_length = 10U;
    memcpy(record.data, "2125451001", 10U);
    name = NULL;
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_MODULE_OTHER,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Mercedes hardware part number") == 0);
    CHECK(strcmp(text, "2125451001") == 0);
    return 0;
}

static int test_documented_route_read_union(void)
{
    const MblinkMercedesDocumentedRead *read;
    bool saw_f100 = false;
    bool saw_f150 = false;
    bool saw_1a86 = false;

    /*
     * Exact-route fallback is global. It works even before a curated
     * controller-family alias exists and de-duplicates reads shared by several
     * generations on the same Mercedes diagnostic address.
     */
    CHECK(mblink_mercedes_documented_route_read_count(
              0x602, 0x480, false,
              MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 9U);
    for (size_t i = 0U; i < mblink_mercedes_documented_route_read_count(
             0x602, 0x480, false,
             MBLINK_MERCEDES_DIAGNOSTIC_UDS); ++i) {
        read = mblink_mercedes_documented_route_read_at(
            0x602, 0x480, false,
            MBLINK_MERCEDES_DIAGNOSTIC_UDS, i);
        CHECK(read != NULL);
        if (read->service == 0x22 && read->identifier == 0xf100)
            saw_f100 = true;
        if (read->service == 0x22 && read->identifier == 0xf150)
            saw_f150 = true;
    }
    CHECK(saw_f100 && saw_f150);

    CHECK(mblink_mercedes_documented_route_read_count(
              0x7e1, 0x7e9, false,
              MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) == 7U);
    for (size_t i = 0U; i < mblink_mercedes_documented_route_read_count(
             0x7e1, 0x7e9, false,
             MBLINK_MERCEDES_DIAGNOSTIC_KWP2000); ++i) {
        read = mblink_mercedes_documented_route_read_at(
            0x7e1, 0x7e9, false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, i);
        CHECK(read != NULL);
        if (read->service == 0x1a && read->identifier == 0x86)
            saw_1a86 = true;
    }
    CHECK(saw_1a86);

    CHECK(mblink_mercedes_documented_route_read_count(
              0x7e0, 0x7e8, false,
              MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 14U);
    CHECK(mblink_mercedes_documented_route_read_at(
              0x7e0, 0x7e8, false,
              MBLINK_MERCEDES_DIAGNOSTIC_UDS, 14U) == NULL);
    return 0;
}

static int test_documented_kwp_command_list(void)
{
    MblinkMercedesDataScan scan;MblinkMercedesDataScanConfig config=mblink_mercedes_data_scan_default_config(0x7e1,0x7e9,false,MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,MBLINK_MERCEDES_MODULE_TRANSMISSION);
    static const MblinkMercedesDataProbeCommand commands[]={{0x1a,0x86},{0x21,0xb1}};MblinkElm327Response ok=response_ok("OK");
    CHECK(mblink_mercedes_data_scan_begin_probe_commands(&scan,&config,commands,2U)==MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan,"ATSP6",ok)==0);CHECK(accept_command(&scan,"ATH0",ok)==0);CHECK(accept_command(&scan,"ATCAF1",ok)==0);CHECK(accept_command(&scan,"ATCFC1",ok)==0);CHECK(accept_command(&scan,"ATST64",ok)==0);CHECK(accept_command(&scan,"ATSH7E1",ok)==0);CHECK(accept_command(&scan,"ATCRA7E9",ok)==0);CHECK(accept_command(&scan,"3E01",response_ok("7E"))==0);
    CHECK(accept_command(&scan,"1A86",response_ok("5A8600344643104806291808035500110504FFFF"))==0);CHECK(accept_command(&scan,"21B1",response_ok("61B100"))==0);CHECK(scan.positive_count==2U);
    const MblinkMercedesDataRecord*r=mblink_mercedes_data_scan_record_at(&scan,0U);CHECK(r!=NULL&&r->service==0x1a&&r->identifier==0x86);
    return 0;
}

static int test_hu204_factory_reading_keeps_session(void)
{
    MblinkMercedesDataScan scan;
    MblinkMercedesDataScanConfig config =
        mblink_mercedes_data_scan_default_config(
            UINT32_C(0x652), UINT32_C(0x48a), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            MBLINK_MERCEDES_MODULE_BODY);
    const uint16_t identifier = UINT16_C(0x01);
    MblinkElm327Response ok = response_ok("OK");
    char command[32];
    size_t written = 0U;

    CHECK(mblink_mercedes_data_scan_begin_identifiers(
              &scan, &config, &identifier, 1U) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH652", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA48A", ok) == 0);
    CHECK(accept_command(&scan, "3E01", response_ok("7E")) == 0);

    CHECK(mblink_mercedes_data_scan_command(
              &scan, command, sizeof(command), &written) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(strcmp(command, "2101") == 0);
    {
        MblinkElm327Response value =
            response_ok("7F2178\n012\n0:610110102210");
        CHECK(mblink_mercedes_data_scan_accept(&scan, &value) ==
              MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE);
    }
    CHECK(scan.stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE);
    return 0;
}

static int test_orc_dashboard_records_are_semantically_mapped(void)
{
    MblinkMercedesDataRecord record;
    char text[256];
    const char *name = NULL;

    memset(&record, 0, sizeof(record));
    record.service =
        MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER;

    record.identifier = UINT16_C(0x02);
    record.data_length = 4U;
    record.data[0] = UINT8_C(0xc1);
    record.data[1] = UINT8_C(0x24);
    record.data[2] = UINT8_C(0x41);
    record.data[3] = UINT8_C(0x00);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x64a), UINT32_C(0x489), false,
        MBLINK_MERCEDES_MODULE_RESTRAINTS,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "Restraint equipment configuration") == 0);
    CHECK(strcmp(text, "Configuration C1244100") == 0);

    record.identifier = UINT16_C(0x58);
    record.data_length = 5U;
    record.data[0] = UINT8_C(0x00);
    record.data[1] = UINT8_C(0x90);
    record.data[2] = UINT8_C(0x55);
    record.data[3] = UINT8_C(0x68);
    record.data[4] = UINT8_C(0x00);
    CHECK(mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x64a), UINT32_C(0x489), false,
        MBLINK_MERCEDES_MODULE_RESTRAINTS,
        &record, text, sizeof(text), &name));
    CHECK(strcmp(name, "ECU lock state / tester identification") == 0);
    CHECK(strcmp(
        text,
        "Lock state 0x00 · tester identification 90556800") == 0);

    /* Never promote the same local IDs on an unrelated KWP controller. */
    CHECK(!mblink_mercedes_data_record_format_known_for_route(
        UINT32_C(0x652), UINT32_C(0x48a), false,
        MBLINK_MERCEDES_MODULE_BODY,
        &record, text, sizeof(text), &name));
    return 0;
}

int main(void)
{
    if (test_documented_uds_version_metadata() != 0) return 1;
    if (test_documented_uds_short_part_identifiers() != 0) return 1;
    if (test_orc_dashboard_records_are_semantically_mapped() != 0) return 1;
    if (test_hu204_factory_reading_keeps_session() != 0) return 1;
    if (test_uds_data_scan() != 0) return 1;
    if (test_kwp_local_identifier_scan() != 0) return 1;
    if (test_7e1_transmission_temperature_candidate() != 0) return 1;
    if (test_full_rli30_numeric_prefers_full_layout() != 0) return 1;
    if (test_transmission_standard_kwp_metadata() != 0) return 1;
    if (test_c207_vehicle_verified_raw_positives() != 0) return 1;
    if (test_targeted_positive_identifier_refresh() != 0) return 1;
    if (test_source_candidate_identifier_probe() != 0) return 1;
    if (test_runtime_candidate_catalog() != 0) return 1;
    if (test_20260903_transmission_capture_replay() != 0) return 1;
    if (test_documented_global_ecu_catalog() != 0) return 1;
    if (test_documented_route_read_union() != 0) return 1;
    if (test_documented_kwp_command_list() != 0) return 1;
    puts("Mercedes manufacturer data scan tests passed");
    return 0;
}
