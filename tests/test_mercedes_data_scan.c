// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_data_scan.h"
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
    CHECK(accept_command(scan, "1003", response_ok("5003001400C8")) == 0);
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
    CHECK(scan.current_no_response_retries == 1U);
    CHECK(accept_command(&scan, "2158", response_no_data()) == 0);
    CHECK(scan.current_identifier == UINT16_C(0x58));
    CHECK(scan.no_response_count == 0U);
    CHECK(scan.current_no_response_retries == 2U);
    CHECK(accept_command(
              &scan, "2158", response_ok("61580090556800")) == 0);
    CHECK(scan.current_no_response_retries == 0U);
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
    CHECK(scan.current_no_response_retries == 0U);

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
    CHECK(mblink_mercedes_data_scan_begin(&scan, &config) ==
          MBLINK_MERCEDES_DATA_SCAN_RESULT_OK);
    CHECK(accept_command(&scan, "ATSP6", ok) == 0);
    CHECK(accept_command(&scan, "ATH0", ok) == 0);
    CHECK(accept_command(&scan, "ATCAF1", ok) == 0);
    CHECK(accept_command(&scan, "ATCFC1", ok) == 0);
    CHECK(accept_command(&scan, "ATST64", ok) == 0);
    CHECK(accept_command(&scan, "ATSH632", ok) == 0);
    CHECK(accept_command(&scan, "ATCRA486", ok) == 0);
    CHECK(accept_command(
              &scan, "1003", response_ok("7F10785003001400C8")) == 0);
    CHECK(accept_command(&scan, "3E00", response_ok("7E00")) == 0);
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

    CHECK(mblink_mercedes_data_record_decode_known_numeric_for_route(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_MODULE_TRANSMISSION,
        &record, &value, &name, &unit));
    CHECK(value == 50.0);
    CHECK(strcmp(name, "Transmission oil temperature") == 0);
    CHECK(strcmp(unit, "°C") == 0);
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

static int test_runtime_candidate_catalog(void)
{
    const MblinkMercedesControllerDataProfileEntry *entry;

    /*
     * Automatic candidates are no longer inferred from a CAN route.
     */
    CHECK(mblink_mercedes_data_runtime_candidate_identifier_count_for_route(
        UINT32_C(0x7e0), UINT32_C(0x7e8), false) == 0U);
    CHECK(mblink_mercedes_data_runtime_candidate_identifier_count_for_route(
        UINT32_C(0x632), UINT32_C(0x486), false) == 0U);
    CHECK(mblink_mercedes_data_runtime_candidate_identifier_count_for_route(
        UINT32_C(0x64a), UINT32_C(0x489), false) == 0U);
    CHECK(mblink_mercedes_data_runtime_candidate_identifier_count_for_route(
        UINT32_C(0x652), UINT32_C(0x48a), false) == 0U);

    CHECK(strcmp(
        mblink_mercedes_data_profile_key_for_controller(
            "engine-cdi", "CRD3", NULL, NULL),
        "engine-crd3") == 0);
    CHECK(strcmp(
        mblink_mercedes_data_profile_key_for_controller(
            "engine-cdi", "CDID3 Delphi", NULL, NULL),
        "engine-crd3") == 0);
    CHECK(mblink_mercedes_data_profile_key_for_controller(
        "engine-cdi", "EDC17", NULL, NULL) == NULL);
    CHECK(mblink_mercedes_data_profile_key_for_controller(
        "engine-me", "MED17", NULL, NULL) == NULL);
    CHECK(strcmp(
        mblink_mercedes_data_profile_key_for_controller(
            "esp", "ABR2XT", NULL, NULL),
        "esp-abr2xt") == 0);
    CHECK(mblink_mercedes_data_profile_key_for_controller(
        "esp", "ESP212", NULL, NULL) == NULL);
    CHECK(strcmp(
        mblink_mercedes_data_profile_key_for_controller(
            "restraints-orc", "ORC_212", NULL, NULL),
        "restraints-orc212") == 0);
    CHECK(strcmp(
        mblink_mercedes_data_profile_key_for_controller(
            "audio-headunit", "HU_204", NULL, NULL),
        "headunit-hu204") == 0);

    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 1U);
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) == 0U);
    entry = mblink_mercedes_controller_data_profile_identifier_at(
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS, 0U);
    CHECK(entry != NULL && entry->identifier == UINT16_C(0x2007));
    CHECK(entry->live);
    CHECK(strcmp(entry->name, "Battery voltage") == 0);

    entry = mblink_mercedes_controller_data_profile_find(
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS, UINT16_C(0x2001));
    CHECK(entry != NULL && entry->live);
    entry = mblink_mercedes_controller_data_profile_find(
        "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT16_C(0x0058));
    CHECK(entry != NULL && entry->live);
    entry = mblink_mercedes_controller_data_profile_find(
        "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT16_C(0x0001));
    CHECK(entry != NULL && entry->live);
    CHECK(mblink_mercedes_controller_data_profile_identifier_count(
        "engine-me", MBLINK_MERCEDES_DIAGNOSTIC_UDS) == 0U);

    CHECK(mblink_mercedes_data_identifier_is_runtime_refreshable(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, UINT16_C(0x0030)));
    CHECK(!mblink_mercedes_data_identifier_is_runtime_refreshable(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, UINT16_C(0x00e0)));
    CHECK(!mblink_mercedes_data_identifier_is_runtime_refreshable(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, UINT16_C(0x00eb)));
    return 0;
}

int main(void)
{
    if (test_uds_data_scan() != 0) return 1;
    if (test_kwp_local_identifier_scan() != 0) return 1;
    if (test_7e1_transmission_temperature_candidate() != 0) return 1;
    if (test_full_rli30_numeric_prefers_full_layout() != 0) return 1;
    if (test_transmission_standard_kwp_metadata() != 0) return 1;
    if (test_c207_vehicle_verified_raw_positives() != 0) return 1;
    if (test_targeted_positive_identifier_refresh() != 0) return 1;
    if (test_source_candidate_identifier_probe() != 0) return 1;
    if (test_runtime_candidate_catalog() != 0) return 1;
    puts("Mercedes manufacturer data scan tests passed");
    return 0;
}
