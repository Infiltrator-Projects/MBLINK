// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_transmission.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) {     fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__);     return 1; } } while (0)

static int test_2130(void)
{
    uint8_t data[10] = {0};
    MblinkMercedesTransmission2130 decoded;

    data[3] = UINT8_C(0x06);
    data[9] = UINT8_C(0x64);
    CHECK(mblink_mercedes_transmission_decode_2130(
        data, sizeof(data), &decoded));
    CHECK(decoded.oil_temperature_available);
    CHECK(decoded.oil_temperature_c == 50.0);
    CHECK(decoded.actual_gear_code_available);
    CHECK(decoded.actual_gear_code == 6U);

    CHECK(strcmp(mblink_mercedes_transmission_actual_gear_name(6U), "6") == 0);
    CHECK(strcmp(mblink_mercedes_transmission_actual_gear_name(11U), "R") == 0);
    CHECK(strcmp(mblink_mercedes_transmission_actual_gear_name(13U), "P") == 0);
    CHECK(strcmp(mblink_mercedes_transmission_target_gear_name(14U), "Cancel") == 0);
    return 0;
}

static int test_gs218(void)
{
    uint8_t payload[8] = {0};
    MblinkMercedesGs218 decoded;

    /* byte2: target gear 5, actual gear 6 */
    payload[2] = UINT8_C(0x65);
    /* byte3: converter closed + shifting + manual mode */
    payload[3] = UINT8_C(0xC4);
    /* byte4: gearbox ok + limp + overtemp + kickdown + comfort program */
    payload[4] = UINT8_C(0xB9);
    /* byte5: converter unloaded at offset45 */
    payload[5] = UINT8_C(0x20);

    CHECK(mblink_mercedes_transmission_decode_gs218(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.target_gear_code == 5U);
    CHECK(decoded.actual_gear_code == 6U);
    CHECK(decoded.torque_converter == MBLINK_MERCEDES_TCC_CLOSED);
    CHECK(decoded.shifting);
    CHECK(decoded.manual_shift_mode);
    CHECK(decoded.gearbox_ok);
    CHECK(decoded.limp_home);
    CHECK(decoded.overtemperature);
    CHECK(decoded.kickdown);
    CHECK(decoded.drive_program_code == 2U);
    CHECK(decoded.converter_unloaded);
    CHECK(strcmp(mblink_mercedes_transmission_tcc_name(
        decoded.torque_converter), "closed") == 0);

    payload[3] = UINT8_C(0x07);
    CHECK(mblink_mercedes_transmission_decode_gs218(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.torque_converter == MBLINK_MERCEDES_TCC_CONFLICT);
    return 0;
}

static int test_gs338(void)
{
    const uint8_t payload[8] = {
        UINT8_C(0x34), UINT8_C(0x12), 0U, 0U, 0U, 0U,
        UINT8_C(0x78), UINT8_C(0x56)
    };
    MblinkMercedesGs338 decoded;
    CHECK(mblink_mercedes_transmission_decode_gs338(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.output_speed_raw == UINT16_C(0x1234));
    CHECK(decoded.turbine_speed_raw == UINT16_C(0x5678));
    return 0;
}

static int test_gs418(void)
{
    uint8_t payload[8] = {0};
    MblinkMercedesGs418 decoded;

    payload[0] = UINT8_C(0x44); /* D */
    payload[1] = UINT8_C(0x53); /* S */
    payload[2] = UINT8_C(0x80); /* temperature raw, no scale asserted */
    /* AWD + shifting + NAG2 large + brake required + kickdown */
    payload[3] = UINT8_C(0xD5);
    /* target gear 4, actual gear 5 */
    payload[4] = UINT8_C(0x54);
    payload[5] = UINT8_C(0x22);
    /* selector D in bits 2..4, factor low three bits in 5..7 */
    payload[6] = UINT8_C(0x70);
    payload[7] = UINT8_C(0x12);

    CHECK(mblink_mercedes_transmission_decode_gs418(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.display_position_code == UINT8_C(0x44));
    CHECK(decoded.drive_program_code == UINT8_C(0x53));
    CHECK(decoded.transmission_temperature_raw == UINT8_C(0x80));
    CHECK(decoded.all_wheel_drive);
    CHECK(!decoded.front_wheel_drive);
    CHECK(decoded.shifting);
    CHECK(!decoded.cvt);
    CHECK(decoded.mechanism_variant == 1U);
    CHECK(decoded.brake_required);
    CHECK(decoded.kickdown);
    CHECK(decoded.target_gear_code == 4U);
    CHECK(decoded.actual_gear_code == 5U);
    CHECK(decoded.torque_loss_raw == UINT8_C(0x22));
    CHECK(decoded.selector_position_code == 4U);
    CHECK(strcmp(mblink_mercedes_transmission_selector_name(
        decoded.selector_position_code), "D") == 0);
    return 0;
}

int main(void)
{
    if (test_2130() != 0) return 1;
    if (test_gs218() != 0) return 1;
    if (test_gs338() != 0) return 1;
    if (test_gs418() != 0) return 1;
    puts("Mercedes transmission decoder tests passed");
    return 0;
}
