// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_transmission.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static void put_bits(uint8_t payload[8], unsigned int offset,
                     unsigned int length, uint32_t value)
{
    for (unsigned int bit = 0U; bit < length; ++bit) {
        const unsigned int absolute = offset + bit;
        const unsigned int byte_index = absolute / 8U;
        const unsigned int bit_index = absolute % 8U;
        const uint8_t mask = (uint8_t)(UINT8_C(1) << bit_index);
        if ((value & (UINT32_C(1) << bit)) != 0U)
            payload[byte_index] |= mask;
        else
            payload[byte_index] &= (uint8_t)~mask;
    }
}

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

static int test_egs51_gs218(void)
{
    uint8_t payload[6] = {0};
    MblinkMercedesEgs51Gs218 decoded;

    payload[0] = UINT8_C(100); /* 50 torque units */
    payload[1] = UINT8_C(0xF4); /* automatic, OK, P/N, garage, start, torque req */
    payload[2] = UINT8_C(0x43); /* target 3, actual 4 */
    payload[3] = UINT8_C(0xF4); /* closed, large, limp, shifting, kickdown, FWD */
    payload[4] = UINT8_C(0x55);
    payload[5] = UINT8_C(0xA0);

    CHECK(mblink_mercedes_transmission_decode_egs51_gs218(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.torque_request == 50.0);
    CHECK(decoded.automatic_transmission);
    CHECK(decoded.gearbox_program_ok);
    CHECK(decoded.park_or_neutral);
    CHECK(decoded.start_enabled);
    CHECK(decoded.torque_request_enabled);
    CHECK(decoded.target_gear_code == 3U);
    CHECK(decoded.actual_gear_code == 4U);
    CHECK(decoded.torque_converter == MBLINK_MERCEDES_TCC_CLOSED);
    CHECK(decoded.large_gearbox);
    CHECK(decoded.limp_home);
    CHECK(decoded.shifting);
    CHECK(decoded.kickdown);
    CHECK(decoded.front_wheel_drive);
    CHECK(decoded.tcc_torque_multiplier_raw == UINT8_C(0x55));
    CHECK(decoded.error_counter == 10U);
    return 0;
}

static int test_egs52_gs218(void)
{
    uint8_t payload[8] = {0};
    MblinkMercedesGs218 decoded;

    put_bits(payload, 0U, 1U, 1U);
    put_bits(payload, 3U, 13U, UINT32_C(1234));
    put_bits(payload, 16U, 4U, 5U);
    put_bits(payload, 20U, 4U, 6U);
    put_bits(payload, 26U, 1U, 1U);
    put_bits(payload, 28U, 1U, 1U);
    put_bits(payload, 30U, 1U, 1U);
    put_bits(payload, 31U, 1U, 1U);
    put_bits(payload, 32U, 1U, 1U);
    put_bits(payload, 35U, 1U, 1U);
    put_bits(payload, 36U, 1U, 1U);
    put_bits(payload, 37U, 1U, 1U);
    put_bits(payload, 38U, 2U, 1U);
    put_bits(payload, 45U, 1U, 1U);
    put_bits(payload, 48U, 8U, UINT32_C(0x77));
    put_bits(payload, 56U, 2U, 2U);
    put_bits(payload, 58U, 1U, 1U);
    put_bits(payload, 59U, 5U, 17U);

    CHECK(mblink_mercedes_transmission_decode_gs218(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.engine_torque_toggle);
    CHECK(decoded.requested_engine_torque_raw == UINT16_C(1234));
    CHECK(decoded.target_gear_code == 5U);
    CHECK(decoded.actual_gear_code == 6U);
    CHECK(decoded.torque_converter == MBLINK_MERCEDES_TCC_CLOSED);
    CHECK(decoded.basic_shift_program_ok);
    CHECK(decoded.shifting);
    CHECK(decoded.manual_shift_mode);
    CHECK(decoded.gearbox_ok);
    CHECK(decoded.limp_home);
    CHECK(decoded.overtemperature);
    CHECK(decoded.kickdown);
    CHECK(decoded.drive_program_code == 1U);
    CHECK(decoded.converter_unloaded);
    CHECK(decoded.creep_or_calid_raw == UINT8_C(0x77));
    CHECK(decoded.error_check_state == 2U);
    CHECK(decoded.calid_cvn_active);
    CHECK(decoded.error_counter == 17U);
    return 0;
}

static int test_egs52_gs338(void)
{
    uint8_t payload[8] = {0};
    MblinkMercedesGs338 decoded;

    put_bits(payload, 0U, 16U, UINT32_C(0x1234));
    put_bits(payload, 16U, 1U, 1U);
    put_bits(payload, 19U, 1U, 1U);
    put_bits(payload, 20U, 2U, 2U);
    put_bits(payload, 22U, 10U, 511U);
    put_bits(payload, 32U, 16U, UINT32_C(0x3456));
    put_bits(payload, 48U, 16U, UINT32_C(0x5678));

    CHECK(mblink_mercedes_transmission_decode_gs338(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.output_speed_rpm == UINT16_C(0x1234));
    CHECK(decoded.mil_request);
    CHECK(decoded.power_free_in_drive);
    CHECK(decoded.race_start_state == 2U);
    CHECK(decoded.pilot_torque_raw == UINT16_C(511));
    CHECK(decoded.starting_area_torque_raw == UINT16_C(0x3456));
    CHECK(decoded.turbine_speed_rpm == UINT16_C(0x5678));
    return 0;
}

static int test_egs52_gs418(void)
{
    uint8_t payload[8] = {0};
    MblinkMercedesGs418 decoded;

    payload[0] = UINT8_C(0x44); /* D */
    payload[1] = UINT8_C(0x53); /* S */
    payload[2] = UINT8_C(0x64); /* 50 C */
    put_bits(payload, 24U, 1U, 1U);
    put_bits(payload, 26U, 1U, 1U);
    put_bits(payload, 28U, 2U, 2U);
    put_bits(payload, 30U, 1U, 1U);
    put_bits(payload, 31U, 1U, 1U);
    put_bits(payload, 32U, 4U, 4U);
    put_bits(payload, 36U, 4U, 5U);
    put_bits(payload, 40U, 8U, UINT32_C(0x22));
    put_bits(payload, 48U, 1U, 1U);
    put_bits(payload, 50U, 3U, 4U);
    put_bits(payload, 53U, 11U, UINT32_C(0x345));

    CHECK(mblink_mercedes_transmission_decode_gs418(
        payload, sizeof(payload), &decoded));
    CHECK(decoded.display_position_code == UINT8_C(0x44));
    CHECK(decoded.drive_program_code == UINT8_C(0x53));
    CHECK(decoded.transmission_temperature_raw == UINT8_C(0x64));
    CHECK(decoded.transmission_temperature_c == 50.0);
    CHECK(decoded.all_wheel_drive);
    CHECK(decoded.shifting);
    CHECK(decoded.mechanism_variant == 2U);
    CHECK(decoded.brake_required);
    CHECK(decoded.kickdown);
    CHECK(decoded.target_gear_code == 4U);
    CHECK(decoded.actual_gear_code == 5U);
    CHECK(decoded.torque_loss_raw == UINT8_C(0x22));
    CHECK(decoded.wheel_torque_parity);
    CHECK(decoded.selector_position_code == 4U);
    CHECK(decoded.wheel_torque_factor_raw == UINT16_C(0x345));
    CHECK(strcmp(mblink_mercedes_transmission_selector_name(
        decoded.selector_position_code), "D") == 0);
    return 0;
}

static int test_kwp_records(void)
{
    uint8_t rli30[24] = {0};
    uint8_t rli31[20] = {0};
    uint8_t rli32[12] = {0};
    uint8_t rli33[15] = {0};
    MblinkMercedesKwpRli30 d30;
    MblinkMercedesKwpRli31 d31;
    MblinkMercedesKwpRli32 d32;
    MblinkMercedesKwpRli33 d33;

    rli30[0] = 0x01; rli30[1] = 0x02;
    rli30[10] = UINT8_C(0x54);
    rli30[11] = UINT8_C(100);
    rli30[16] = 0x12; rli30[17] = 0x34;
    rli30[18] = UINT8_C(0xE1);
    rli30[19] = UINT8_C(0x3F);
    rli30[20] = UINT8_C(0xFF);
    rli30[21] = UINT8_C(0xFF);
    rli30[23] = UINT8_C(0x02);
    CHECK(mblink_mercedes_transmission_decode_kwp_rli30(
        rli30, sizeof(rli30), &d30));
    CHECK(d30.tcc_delta_speed_raw == UINT16_C(0x0102));
    CHECK(d30.actual_gear_code == 4U);
    CHECK(d30.target_gear_code == 5U);
    CHECK(d30.atf_temperature_c == 50.0);
    CHECK(d30.output_speed_raw == UINT16_C(0x1234));
    CHECK(d30.kickdown);
    CHECK(d30.solenoid_1245);
    CHECK(d30.solenoid_23);
    CHECK(d30.solenoid_34);
    CHECK(d30.program_button);
    CHECK(d30.converter_clutch_enabled);
    CHECK(d30.emergency_mode);
    CHECK(d30.converter_ok);

    for (size_t i = 0U; i < sizeof(rli31); i += 2U) {
        rli31[i] = 0x01;
        rli31[i + 1U] = (uint8_t)(i / 2U + 1U);
    }
    CHECK(mblink_mercedes_transmission_decode_kwp_rli31(
        rli31, sizeof(rli31), &d31));
    CHECK(d31.n2_pulse_count == UINT16_C(0x0101));
    CHECK(d31.input_rpm == UINT16_C(0x0103));
    CHECK(d31.front_vehicle_speed_raw == UINT16_C(0x010A));

    rli32[0] = 42U;
    rli32[1] = 0x01; rli32[2] = 0x23;
    rli32[3] = 0x04; rli32[4] = 0x56;
    rli32[5] = 7U;
    rli32[6] = 0x00; rli32[7] = 0x77;
    rli32[8] = 8U; rli32[9] = 9U;
    rli32[10] = 2U; rli32[11] = 6U;
    CHECK(mblink_mercedes_transmission_decode_kwp_rli32(
        rli32, sizeof(rli32), &d32));
    CHECK(d32.pedal_percent == 42U);
    CHECK(d32.upshift_delta_rpm_raw == UINT16_C(0x0123));
    CHECK(d32.requested_high_gear_limit == 6U);

    rli33[0] = 1U; rli33[1] = 7U;
    for (size_t i = 2U; i < 14U; i += 2U) {
        rli33[i] = 0x01;
        rli33[i + 1U] = (uint8_t)i;
    }
    rli33[14] = 0x80U;
    CHECK(mblink_mercedes_transmission_decode_kwp_rli33(
        rli33, sizeof(rli33), &d33));
    CHECK(d33.valve_flag == 1U);
    CHECK(d33.shift_valve_state == 7U);
    CHECK(d33.spc_pressure_raw == UINT16_C(0x0102));
    CHECK(d33.mpc_actual_current_raw == UINT16_C(0x010C));
    CHECK(d33.tcc_pwm_raw == UINT8_C(0x80));

    CHECK(mblink_mercedes_transmission_kwp_read_identifier_count() == 15U);
    CHECK(mblink_mercedes_transmission_kwp_read_identifier_at(0U) == UINT8_C(0x30));
    CHECK(mblink_mercedes_transmission_kwp_read_identifier_at(14U) == UINT8_C(0xEB));
    CHECK(strcmp(mblink_mercedes_transmission_kwp_read_identifier_name(
        UINT8_C(0xE1)), "ECU serial number") == 0);
    return 0;
}

static int test_egs53(void)
{
    uint8_t payload[8] = {0};
    MblinkMercedesEgs53TcmA1 a1;
    MblinkMercedesEgs53TcmA2 a2;
    MblinkMercedesEgs53EngRq1 rq1;
    MblinkMercedesEgs53EngRq2 rq2;
    MblinkMercedesEgs53EngRq3 rq3;
    MblinkMercedesEgs53SbwRsTcm sbw;
    MblinkMercedesEgs53TcmDisplayRequest display;

    put_bits(payload, 0U, 8U, 100U);
    put_bits(payload, 9U, 3U, 4U);
    put_bits(payload, 12U, 1U, 1U);
    put_bits(payload, 15U, 1U, 1U);
    put_bits(payload, 17U, 3U, 4U);
    put_bits(payload, 20U, 1U, 1U);
    put_bits(payload, 21U, 3U, 1U);
    put_bits(payload, 32U, 16U, 420U);
    CHECK(mblink_mercedes_transmission_decode_egs53_tcm_a1(
        payload, sizeof(payload), &a1));
    CHECK(a1.oil_temperature_c == 50.0);
    CHECK(a1.clutch_state == 4U);
    CHECK(a1.limp_home);
    CHECK(a1.overtemperature);
    CHECK(a1.selector_position_code == 4U);
    CHECK(a1.manual_program_active);
    CHECK(a1.drive_program_code == 1U);
    CHECK(a1.drive_shaft_torque_nm == UINT16_C(420));

    memset(payload, 0, sizeof(payload));
    put_bits(payload, 0U, 8U, 100U);
    put_bits(payload, 18U, 14U, 2500U);
    put_bits(payload, 32U, 1U, 1U);
    put_bits(payload, 34U, 14U, 125U);
    put_bits(payload, 48U, 8U, UINT32_C(0xAA));
    put_bits(payload, 56U, 2U, 2U);
    put_bits(payload, 58U, 1U, 1U);
    put_bits(payload, 59U, 5U, 9U);
    CHECK(mblink_mercedes_transmission_decode_egs53_tcm_a2(
        payload, sizeof(payload), &a2));
    CHECK(a2.requested_current_duty_percent == 50.0);
    CHECK(a2.turbine_rpm == UINT16_C(2500));
    CHECK(a2.mil_request);
    CHECK(a2.desired_slip_rpm == UINT16_C(125));
    CHECK(a2.calid_cvn_data == UINT8_C(0xAA));
    CHECK(a2.calid_cvn_error_counter == 9U);

    memset(payload, 0, sizeof(payload));
    put_bits(payload, 3U, 13U, 2400U); /* 100 Nm */
    put_bits(payload, 16U, 2U, 3U);
    put_bits(payload, 18U, 3U, 2U);
    put_bits(payload, 24U, 8U, 50U); /* 1s */
    put_bits(payload, 34U, 14U, 3000U);
    put_bits(payload, 52U, 1U, 1U);
    put_bits(payload, 56U, 8U, UINT32_C(0x5A));
    CHECK(mblink_mercedes_transmission_decode_egs53_eng_rq1(
        payload, sizeof(payload), &rq1));
    CHECK(rq1.requested_engine_torque_nm == 100.0);
    CHECK(rq1.intervention_mode == 3U);
    CHECK(rq1.downshift_mode == 2U);
    CHECK(rq1.engine_sync_time_s == 1.0);
    CHECK(rq1.requested_engine_rpm == UINT16_C(3000));
    CHECK(rq1.engine_start_enable_request);
    CHECK(rq1.crc == UINT8_C(0x5A));

    memset(payload, 0, sizeof(payload));
    put_bits(payload, 0U, 4U, 7U);
    put_bits(payload, 4U, 4U, 6U);
    put_bits(payload, 8U, 8U, 125U); /* 2.5 */
    put_bits(payload, 18U, 14U, 350U); /* 3.5 */
    put_bits(payload, 32U, 8U, 80U); /* 20 Nm */
    put_bits(payload, 40U, 2U, 2U);
    put_bits(payload, 42U, 2U, 0U);
    put_bits(payload, 44U, 2U, 2U);
    put_bits(payload, 46U, 2U, 1U);
    CHECK(mblink_mercedes_transmission_decode_egs53_eng_rq2(
        payload, sizeof(payload), &rq2));
    CHECK(rq2.target_gear_code == 7U);
    CHECK(rq2.actual_gear_code == 6U);
    CHECK(rq2.transmission_ratio == 2.5);
    CHECK(rq2.engine_to_wheel_torque_ratio == 3.5);
    CHECK(rq2.transmission_torque_loss_nm == 20.0);
    CHECK(rq2.vehicle_drive_style == 2U);
    CHECK(rq2.transmission_mechanics_style == 2U);
    CHECK(rq2.transmission_shift_style == 1U);

    memset(payload, 0, sizeof(payload));
    put_bits(payload, 1U, 2U, 2U);
    put_bits(payload, 3U, 13U, 2200U); /* 50 Nm */
    CHECK(mblink_mercedes_transmission_decode_egs53_eng_rq3(
        payload, sizeof(payload), &rq3));
    CHECK(rq3.maximum_acceleration_state == 2U);
    CHECK(rq3.wet_driveaway_clutch_torque_nm == 50.0);

    memset(payload, 0, sizeof(payload));
    put_bits(payload, 0U, 2U, 0U);
    put_bits(payload, 7U, 1U, 1U);
    put_bits(payload, 8U, 4U, 5U);
    put_bits(payload, 12U, 4U, 6U);
    put_bits(payload, 16U, 8U, 125U); /* 50% */
    CHECK(mblink_mercedes_transmission_decode_egs53_sbw_rs_tcm(
        payload, sizeof(payload), &sbw));
    CHECK(sbw.message_transmitter_id == 0U);
    CHECK(sbw.starter_lockout);
    CHECK(sbw.selector_valve_position == 5U);
    CHECK(sbw.selector_position_request == 6U);
    CHECK(sbw.selector_sensor_percent == 50.0);

    memset(payload, 0, sizeof(payload));
    put_bits(payload, 0U, 8U, (uint32_t)'D');
    put_bits(payload, 8U, 8U, (uint32_t)'S');
    put_bits(payload, 16U, 1U, 1U);
    put_bits(payload, 17U, 2U, 1U);
    put_bits(payload, 21U, 3U, 4U);
    put_bits(payload, 40U, 8U, (uint32_t)'7');
    put_bits(payload, 48U, 3U, 2U);
    CHECK(mblink_mercedes_transmission_decode_egs53_tcm_display_request(
        payload, sizeof(payload), &display));
    CHECK(display.display_position_code == (uint8_t)'D');
    CHECK(display.display_program_code == (uint8_t)'S');
    CHECK(display.shift_by_wire_beep_request);
    CHECK(display.shift_recommendation == 1U);
    CHECK(display.shift_by_wire_message == 4U);
    CHECK(display.target_gear_display_code == (uint8_t)'7');
    CHECK(display.race_start_display_state == 2U);
    return 0;
}

static int test_family_names(void)
{
    CHECK(strcmp(mblink_mercedes_transmission_family_name(
        MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52),
        "EGS52 / NAG1") == 0);
    CHECK(strcmp(mblink_mercedes_transmission_family_name(
        MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G),
        "725.0 9G-Tronic") == 0);
    return 0;
}

int main(void)
{
    if (test_2130() != 0) return 1;
    if (test_egs51_gs218() != 0) return 1;
    if (test_egs52_gs218() != 0) return 1;
    if (test_egs52_gs338() != 0) return 1;
    if (test_egs52_gs418() != 0) return 1;
    if (test_kwp_records() != 0) return 1;
    if (test_egs53() != 0) return 1;
    if (test_family_names() != 0) return 1;
    puts("Mercedes transmission decoder tests passed");
    return 0;
}
