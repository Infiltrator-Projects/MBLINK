// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_did_lab.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_catalogue_and_decode(void)
{
    const MblinkMercedesDidLabDefinition *battery =
        mblink_mercedes_did_lab_find_identifier(UINT16_C(0x2007));
    const MblinkMercedesDidLabDefinition *fuel =
        mblink_mercedes_did_lab_find_key("mercedes.om651.fuel.tank_level");
    const uint8_t response[] = { 0x62U, 0x20U, 0x07U, 0x05U, 0xfcU };
    double value = 0.0;

    CHECK(mblink_mercedes_did_lab_count() >= 14U);
    CHECK(battery != NULL);
    CHECK(strcmp(battery->stable_key,
                 "mercedes.om651.electrical.battery_voltage") == 0);
    CHECK(battery->status ==
          MBLINK_MERCEDES_DID_LAB_SOURCE_BACKED_CANDIDATE);
    CHECK(battery->identifier_known);
    CHECK(battery->raw_length == 2U);
    CHECK(strcmp(battery->unit, "V") == 0);
    CHECK(!mblink_mercedes_did_lab_can_auto_poll(battery));
    CHECK(mblink_mercedes_did_lab_decode_response(
              battery, response, sizeof(response), &value) ==
          MBLINK_MERCEDES_DID_LAB_DECODE_OK);
    CHECK(fabs(value - 11.96875) < 0.000001);

    {
        const MblinkMercedesDidLabDefinition *vgs_temp =
            mblink_mercedes_did_lab_find_key(
                "mercedes.transmission.7229.oil_temperature");
        const MblinkMercedesDidLabDefinition *dct_pressure =
            mblink_mercedes_did_lab_find_key(
                "mercedes.transmission.7240.pressure_sensors");
        const MblinkMercedesDidLabDefinition *nineg_position =
            mblink_mercedes_did_lab_find_key(
                "mercedes.transmission.7250.position_sensors");
        CHECK(vgs_temp != NULL);
        CHECK(vgs_temp->module == MBLINK_MERCEDES_MODULE_TRANSMISSION);
        CHECK(!vgs_temp->identifier_known);
        CHECK(strcmp(vgs_temp->unit, "°C") == 0);
        CHECK(dct_pressure != NULL && !dct_pressure->identifier_known);
        CHECK(nineg_position != NULL && !nineg_position->identifier_known);
    }

    CHECK(fuel != NULL);
    CHECK(!fuel->identifier_known);
    CHECK(strcmp(fuel->unit, "L") == 0);
    CHECK(mblink_mercedes_did_lab_decode_response(
              fuel, response, sizeof(response), &value) ==
          MBLINK_MERCEDES_DID_LAB_DECODE_UNMAPPED);
    return 0;
}

static int test_best_linear_correlation(void)
{
    MblinkSignalPoint reference[120];
    MblinkSignalPoint candidate[120];
    MblinkSignalCorrelationResult result;
    size_t index;

    for (index = 0U; index < 120U; ++index) {
        const double base =
            (double)((index * 17U) % 53U) +
            (double)((index * index) % 19U) * 0.25;
        reference[index].timestamp_ms = (uint64_t)index * UINT64_C(100);
        reference[index].value = base;
        candidate[index].timestamp_ms =
            reference[index].timestamp_ms + UINT64_C(200);
        candidate[index].value = 2.5 * base + 7.0;
    }

    CHECK(mblink_signal_correlation_best_linear(
        reference, 120U, candidate, 120U,
        500U, 100U, 20U, &result));
    CHECK(result.pair_count >= 100U);
    CHECK(result.lag_ms == 200);
    CHECK(fabs(result.pearson_r - 1.0) < 0.000001);
    CHECK(fabs(result.slope - 2.5) < 0.000001);
    CHECK(fabs(result.intercept - 7.0) < 0.000001);
    CHECK(result.normalized_rmse < 0.000001);
    CHECK(strcmp(mblink_signal_correlation_strength(&result),
                 "very-strong") == 0);
    return 0;
}

static int test_bad_series_rejected(void)
{
    const MblinkSignalPoint reference[] = {
        { 100U, 1.0 }, { 90U, 2.0 }, { 200U, 3.0 }
    };
    const MblinkSignalPoint candidate[] = {
        { 100U, 2.0 }, { 200U, 4.0 }, { 300U, 6.0 }
    };
    MblinkSignalCorrelationResult result;

    CHECK(!mblink_signal_correlation_best_linear(
        reference, 3U, candidate, 3U, 100U, 50U, 20U, &result));
    return 0;
}

int main(void)
{
    if (test_catalogue_and_decode() != 0) return 1;
    if (test_best_linear_correlation() != 0) return 1;
    if (test_bad_series_rejected() != 0) return 1;
    puts("Mercedes DID lab tests passed");
    return 0;
}
