// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_data_scan.h"

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
    CHECK(accept_command(scan, "ATST20", ok) == 0);
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
    CHECK(accept_command(&scan, "ATST20", ok) == 0);
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

int main(void)
{
    if (test_uds_data_scan() != 0) return 1;
    if (test_kwp_local_identifier_scan() != 0) return 1;
    puts("Mercedes manufacturer data scan tests passed");
    return 0;
}
