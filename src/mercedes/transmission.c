// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_transmission.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static uint64_t payload_le64(const uint8_t *payload)
{
    uint64_t value = 0U;
    for (size_t index = 0U; index < 8U; ++index)
        value |= ((uint64_t)payload[index]) << (index * 8U);
    return value;
}

static uint32_t extract_bits(uint64_t value, unsigned int offset,
                             unsigned int length)
{
    const uint64_t mask = length == 64U
        ? UINT64_MAX : ((UINT64_C(1) << length) - UINT64_C(1));
    return (uint32_t)((value >> offset) & mask);
}

static uint16_t be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8U) | (uint16_t)data[1]);
}

static MblinkMercedesTorqueConverterState tcc_state(
    bool slipping, bool open, bool closed)
{
    const unsigned int count =
        (slipping ? 1U : 0U) + (open ? 1U : 0U) + (closed ? 1U : 0U);
    if (count > 1U) return MBLINK_MERCEDES_TCC_CONFLICT;
    if (closed) return MBLINK_MERCEDES_TCC_CLOSED;
    if (open) return MBLINK_MERCEDES_TCC_OPEN;
    if (slipping) return MBLINK_MERCEDES_TCC_SLIPPING;
    return MBLINK_MERCEDES_TCC_UNKNOWN;
}

bool mblink_mercedes_transmission_decode_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmission2130 *decoded)
{
    MblinkMercedesTransmission2130 value;
    if (data == NULL || decoded == NULL || data_length < 10U) return false;
    memset(&value, 0, sizeof(value));

    /*
     * Public Mercedes custom-PID evidence:
     * complete response byte L (index 11) minus 50 deg C.
     * The stored payload begins after 61 30, therefore byte L is data[9].
     */
    value.oil_temperature_available = true;
    value.oil_temperature_c = (double)data[9] - 50.0;

    /*
     * Community 722.9/W164 evidence reports current gear in the low nibble
     * of complete response byte F. F is index 5 of the complete response,
     * hence data[3] after stripping 61 30.
     */
    value.actual_gear_code_available = data_length >= 4U;
    value.actual_gear_code = (uint8_t)(data[3] & UINT8_C(0x0f));

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs51_gs218(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs51Gs218 *decoded)
{
    MblinkMercedesEgs51Gs218 value;
    uint64_t raw;
    bool slipping;
    bool open;
    bool closed;

    if (payload == NULL || decoded == NULL || payload_length < 6U)
        return false;
    memset(&value, 0, sizeof(value));

    raw = 0U;
    for (size_t index = 0U; index < 6U; ++index)
        raw |= ((uint64_t)payload[index]) << (index * 8U);

    value.torque_request = (double)payload[0] * 0.5;
    value.automatic_transmission = extract_bits(raw, 8U, 1U) == 0U;
    value.gearbox_program_ok = extract_bits(raw, 10U, 1U) != 0U;
    value.off_road = extract_bits(raw, 11U, 1U) != 0U;
    value.park_or_neutral = extract_bits(raw, 12U, 1U) != 0U;
    value.garage_shift = extract_bits(raw, 13U, 1U) != 0U;
    value.start_enabled = extract_bits(raw, 14U, 1U) != 0U;
    value.torque_request_enabled = extract_bits(raw, 15U, 1U) != 0U;
    value.target_gear_code = (uint8_t)extract_bits(raw, 16U, 4U);
    value.actual_gear_code = (uint8_t)extract_bits(raw, 20U, 4U);
    slipping = extract_bits(raw, 24U, 1U) != 0U;
    open = extract_bits(raw, 25U, 1U) != 0U;
    closed = extract_bits(raw, 26U, 1U) != 0U;
    value.torque_converter = tcc_state(slipping, open, closed);
    value.large_gearbox = extract_bits(raw, 27U, 1U) != 0U;
    value.limp_home = extract_bits(raw, 28U, 1U) != 0U;
    value.shifting = extract_bits(raw, 29U, 1U) != 0U;
    value.kickdown = extract_bits(raw, 30U, 1U) != 0U;
    value.front_wheel_drive = extract_bits(raw, 31U, 1U) != 0U;
    value.tcc_torque_multiplier_raw = payload[4];
    value.error_counter = (uint8_t)extract_bits(raw, 44U, 4U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_gs218(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs218 *decoded)
{
    MblinkMercedesGs218 value;
    uint64_t raw;
    bool slipping;
    bool open;
    bool closed;

    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.engine_torque_toggle = extract_bits(raw, 0U, 1U) != 0U;
    value.engine_torque_request_min = extract_bits(raw, 1U, 1U) != 0U;
    value.engine_torque_request_max = extract_bits(raw, 2U, 1U) != 0U;
    value.requested_engine_torque_raw =
        (uint16_t)extract_bits(raw, 3U, 13U);
    value.target_gear_code = (uint8_t)extract_bits(raw, 16U, 4U);
    value.actual_gear_code = (uint8_t)extract_bits(raw, 20U, 4U);
    slipping = extract_bits(raw, 24U, 1U) != 0U;
    open = extract_bits(raw, 25U, 1U) != 0U;
    closed = extract_bits(raw, 26U, 1U) != 0U;
    value.torque_converter = tcc_state(slipping, open, closed);
    value.off_road_gear = extract_bits(raw, 27U, 1U) != 0U;
    value.basic_shift_program_ok = extract_bits(raw, 28U, 1U) != 0U;
    value.driving_resistance_high = extract_bits(raw, 29U, 1U) != 0U;
    value.shifting = extract_bits(raw, 30U, 1U) != 0U;
    value.manual_shift_mode = extract_bits(raw, 31U, 1U) != 0U;
    value.gearbox_ok = extract_bits(raw, 32U, 1U) != 0U;
    value.start_bang = extract_bits(raw, 33U, 1U) != 0U;
    value.start_enabled = extract_bits(raw, 34U, 1U) != 0U;
    value.limp_home = extract_bits(raw, 35U, 1U) != 0U;
    value.overtemperature = extract_bits(raw, 36U, 1U) != 0U;
    value.kickdown = extract_bits(raw, 37U, 1U) != 0U;
    value.drive_program_code = (uint8_t)extract_bits(raw, 38U, 2U);
    value.engine_torque_parity = extract_bits(raw, 40U, 1U) != 0U;
    value.drivetrain_control_1 = extract_bits(raw, 41U, 1U) != 0U;
    value.drivetrain_control_0 = extract_bits(raw, 42U, 1U) != 0U;
    value.converter_unloaded = extract_bits(raw, 45U, 1U) != 0U;
    value.emergency_engine_off_confirm =
        extract_bits(raw, 46U, 1U) != 0U;
    value.emergency_engine_off = extract_bits(raw, 47U, 1U) != 0U;
    value.creep_or_calid_raw = (uint8_t)extract_bits(raw, 48U, 8U);
    value.error_check_state = (uint8_t)extract_bits(raw, 56U, 2U);
    value.calid_cvn_active = extract_bits(raw, 58U, 1U) != 0U;
    value.error_counter = (uint8_t)extract_bits(raw, 59U, 5U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_gs338(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs338 *decoded)
{
    MblinkMercedesGs338 value;
    uint64_t raw;

    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    /*
     * The source EGS52 implementation assigns NTURBINE directly in rpm before
     * serialising the generated frame, so the 16-bit field is not merely an
     * unidentified raw value. NAB is documented as transmission output speed,
     * with FFFF used on families that do not provide it.
     */
    value.output_speed_rpm = (uint16_t)extract_bits(raw, 0U, 16U);
    value.mil_request = extract_bits(raw, 16U, 1U) != 0U;
    value.nak_parity = extract_bits(raw, 17U, 1U) != 0U;
    value.nak_toggle = extract_bits(raw, 18U, 1U) != 0U;
    value.power_free_in_drive = extract_bits(raw, 19U, 1U) != 0U;
    value.race_start_state = (uint8_t)extract_bits(raw, 20U, 2U);
    value.pilot_torque_raw = (uint16_t)extract_bits(raw, 22U, 10U);
    value.starting_area_torque_raw =
        (uint16_t)extract_bits(raw, 32U, 16U);
    value.turbine_speed_rpm = (uint16_t)extract_bits(raw, 48U, 16U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_gs418(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs418 *decoded)
{
    MblinkMercedesGs418 value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.display_position_code = (uint8_t)extract_bits(raw, 0U, 8U);
    value.drive_program_code = (uint8_t)extract_bits(raw, 8U, 8U);
    value.transmission_temperature_raw =
        (uint8_t)extract_bits(raw, 16U, 8U);
    /*
     * The reverse-engineered EGS52 transmitter stores MAX(temp,-50)+50 in
     * T_GET, independently corroborating the Mercedes raw-minus-50 convention.
     */
    value.transmission_temperature_c =
        (double)value.transmission_temperature_raw - 50.0;
    value.all_wheel_drive = extract_bits(raw, 24U, 1U) != 0U;
    value.front_wheel_drive = extract_bits(raw, 25U, 1U) != 0U;
    value.shifting = extract_bits(raw, 26U, 1U) != 0U;
    value.cvt = extract_bits(raw, 27U, 1U) != 0U;
    value.mechanism_variant = (uint8_t)extract_bits(raw, 28U, 2U);
    value.brake_required = extract_bits(raw, 30U, 1U) != 0U;
    value.kickdown = extract_bits(raw, 31U, 1U) != 0U;
    value.target_gear_code = (uint8_t)extract_bits(raw, 32U, 4U);
    value.actual_gear_code = (uint8_t)extract_bits(raw, 36U, 4U);
    value.torque_loss_raw = (uint8_t)extract_bits(raw, 40U, 8U);
    value.wheel_torque_parity = extract_bits(raw, 48U, 1U) != 0U;
    value.wheel_torque_toggle = extract_bits(raw, 49U, 1U) != 0U;
    value.selector_position_code = (uint8_t)extract_bits(raw, 50U, 3U);
    value.wheel_torque_factor_raw =
        (uint16_t)extract_bits(raw, 53U, 11U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_kwp_rli30(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli30 *decoded)
{
    MblinkMercedesKwpRli30 value;
    if (data == NULL || decoded == NULL || data_length < 24U) return false;
    memset(&value, 0, sizeof(value));

    value.tcc_delta_speed_raw = be16(&data[0]);
    value.tcc_speed_raw = be16(&data[2]);
    value.tcc_pressure_raw = be16(&data[4]);
    value.tcc_status = data[6];
    value.selector_position = data[7];
    value.drive_program = data[8];
    value.recognised_gear = data[9];
    value.actual_gear_code = (uint8_t)(data[10] & UINT8_C(0x0f));
    value.target_gear_code = (uint8_t)((data[10] >> 4U) & UINT8_C(0x0f));
    value.atf_temperature_c = (double)data[11] - 50.0;
    value.engine_torque_raw = be16(&data[12]);
    value.converter_torque_raw = be16(&data[14]);
    value.output_speed_raw = be16(&data[16]);

    value.kickdown = (data[18] & UINT8_C(0x01)) != 0U;
    value.start_enable = (data[18] & UINT8_C(0x02)) != 0U;
    value.start_lockout_reason = (data[18] & UINT8_C(0x04)) != 0U;
    value.reverse_park_lock = (data[18] & UINT8_C(0x08)) != 0U;
    value.starter_relay = (data[18] & UINT8_C(0x10)) != 0U;
    value.solenoid_1245 = (data[18] & UINT8_C(0x20)) != 0U;
    value.solenoid_23 = (data[18] & UINT8_C(0x40)) != 0U;
    value.solenoid_34 = (data[18] & UINT8_C(0x80)) != 0U;

    value.program_button = (data[19] & UINT8_C(0x01)) != 0U;
    value.selector_plus = (data[19] & UINT8_C(0x02)) != 0U;
    value.selector_minus = (data[19] & UINT8_C(0x04)) != 0U;
    value.not_switching = (data[19] & UINT8_C(0x08)) != 0U;
    value.gear_protection = (data[19] & UINT8_C(0x10)) != 0U;
    value.tcc_open_request = (data[19] & UINT8_C(0x20)) != 0U;

    value.downshift = (data[20] & UINT8_C(0x01)) != 0U;
    value.upshift = (data[20] & UINT8_C(0x02)) != 0U;
    value.drivetrain_control_0 = (data[20] & UINT8_C(0x04)) != 0U;
    value.drivetrain_control_1 = (data[20] & UINT8_C(0x08)) != 0U;
    value.release_circuit = (data[20] & UINT8_C(0x10)) != 0U;
    value.steering_wheel_plus = (data[20] & UINT8_C(0x20)) != 0U;
    value.steering_wheel_minus = (data[20] & UINT8_C(0x40)) != 0U;
    value.converter_clutch_enabled = (data[20] & UINT8_C(0x80)) != 0U;

    value.current_error = (data[21] & UINT8_C(0x01)) != 0U;
    value.emergency_mode = (data[21] & UINT8_C(0x02)) != 0U;
    value.asr_active = (data[21] & UINT8_C(0x04)) != 0U;
    value.transmission_protection_ack =
        (data[21] & UINT8_C(0x08)) != 0U;
    value.bang_start = (data[21] & UINT8_C(0x10)) != 0U;
    value.minimum_torque_request = (data[21] & UINT8_C(0x20)) != 0U;
    value.circuit_break = (data[21] & UINT8_C(0x40)) != 0U;
    value.double_circuit = (data[21] & UINT8_C(0x80)) != 0U;

    value.converter_ok = (data[23] & UINT8_C(0x02)) != 0U;
    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_kwp_rli31(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli31 *decoded)
{
    MblinkMercedesKwpRli31 value;
    if (data == NULL || decoded == NULL || data_length < 20U) return false;
    memset(&value, 0, sizeof(value));

    value.n2_pulse_count = be16(&data[0]);
    value.n3_pulse_count = be16(&data[2]);
    value.input_rpm = be16(&data[4]);
    value.engine_rpm = be16(&data[6]);
    value.front_left_wheel_speed_raw = be16(&data[8]);
    value.front_right_wheel_speed_raw = be16(&data[10]);
    value.rear_left_wheel_speed_raw = be16(&data[12]);
    value.rear_right_wheel_speed_raw = be16(&data[14]);
    value.rear_vehicle_speed_raw = be16(&data[16]);
    value.front_vehicle_speed_raw = be16(&data[18]);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_kwp_rli32(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli32 *decoded)
{
    MblinkMercedesKwpRli32 value;
    if (data == NULL || decoded == NULL || data_length < 12U) return false;
    memset(&value, 0, sizeof(value));

    value.pedal_percent = data[0];
    value.upshift_delta_rpm_raw = be16(&data[1]);
    value.downshift_delta_rpm_raw = be16(&data[3]);
    value.pedal_delta_percent = data[5];
    value.pitch_raw = be16(&data[6]);
    value.driving_status = data[8];
    value.engine_warmup_shift_state = data[9];
    value.requested_low_gear_limit = data[10];
    value.requested_high_gear_limit = data[11];

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_kwp_rli33(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli33 *decoded)
{
    MblinkMercedesKwpRli33 value;
    if (data == NULL || decoded == NULL || data_length < 15U) return false;
    memset(&value, 0, sizeof(value));

    value.valve_flag = data[0];
    value.shift_valve_state = data[1];
    value.spc_pressure_raw = be16(&data[2]);
    value.mpc_pressure_raw = be16(&data[4]);
    value.spc_target_current_raw = be16(&data[6]);
    value.spc_actual_current_raw = be16(&data[8]);
    value.mpc_target_current_raw = be16(&data[10]);
    value.mpc_actual_current_raw = be16(&data[12]);
    value.tcc_pwm_raw = data[14];

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_tcm_a1(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53TcmA1 *decoded)
{
    MblinkMercedesEgs53TcmA1 value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.oil_temperature_c = (double)extract_bits(raw, 0U, 8U) - 50.0;
    value.tcc_no_load = extract_bits(raw, 8U, 1U) != 0U;
    value.clutch_state = (uint8_t)extract_bits(raw, 9U, 3U);
    value.limp_home = extract_bits(raw, 12U, 1U) != 0U;
    value.basic_shift_program_ok = extract_bits(raw, 13U, 1U) != 0U;
    value.driving_resistance_high = extract_bits(raw, 14U, 1U) != 0U;
    value.overtemperature = extract_bits(raw, 15U, 1U) != 0U;
    value.off_road_active = extract_bits(raw, 16U, 1U) != 0U;
    value.selector_position_code = (uint8_t)extract_bits(raw, 17U, 3U);
    value.manual_program_active = extract_bits(raw, 20U, 1U) != 0U;
    value.drive_program_code = (uint8_t)extract_bits(raw, 21U, 3U);
    value.start_brake_request = extract_bits(raw, 24U, 1U) != 0U;
    value.drive_shaft_torque_nm = (uint16_t)extract_bits(raw, 32U, 16U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_tcm_a2(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53TcmA2 *decoded)
{
    MblinkMercedesEgs53TcmA2 value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.requested_current_duty_percent =
        (double)extract_bits(raw, 0U, 8U) * 0.5;
    value.turbine_rpm = (uint16_t)extract_bits(raw, 18U, 14U);
    value.mil_request = extract_bits(raw, 32U, 1U) != 0U;
    value.desired_slip_rpm = (uint16_t)extract_bits(raw, 34U, 14U);
    value.calid_cvn_data = (uint8_t)extract_bits(raw, 48U, 8U);
    value.error_check_state = (uint8_t)extract_bits(raw, 56U, 2U);
    value.calid_cvn_active = extract_bits(raw, 58U, 1U) != 0U;
    value.calid_cvn_error_counter = (uint8_t)extract_bits(raw, 59U, 5U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_eng_rq1(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53EngRq1 *decoded)
{
    MblinkMercedesEgs53EngRq1 value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.torque_request_min = extract_bits(raw, 0U, 1U) != 0U;
    value.torque_request_max = extract_bits(raw, 1U, 1U) != 0U;
    value.requested_engine_torque_nm =
        (double)extract_bits(raw, 3U, 13U) * 0.25 - 500.0;
    value.intervention_mode = (uint8_t)extract_bits(raw, 16U, 2U);
    value.downshift_mode = (uint8_t)extract_bits(raw, 18U, 3U);
    value.engine_sync_time_s =
        (double)extract_bits(raw, 24U, 8U) * 0.02;
    value.stop_start_enable_request = extract_bits(raw, 32U, 1U) != 0U;
    value.requested_engine_rpm = (uint16_t)extract_bits(raw, 34U, 14U);
    value.message_counter = (uint8_t)extract_bits(raw, 48U, 4U);
    value.engine_start_enable_request = extract_bits(raw, 52U, 1U) != 0U;
    value.emergency_engine_off_request = extract_bits(raw, 53U, 1U) != 0U;
    value.jump_start_active = extract_bits(raw, 54U, 1U) != 0U;
    value.crc = (uint8_t)extract_bits(raw, 56U, 8U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_eng_rq2(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53EngRq2 *decoded)
{
    MblinkMercedesEgs53EngRq2 value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.target_gear_code = (uint8_t)extract_bits(raw, 0U, 4U);
    value.actual_gear_code = (uint8_t)extract_bits(raw, 4U, 4U);
    value.transmission_ratio =
        (double)extract_bits(raw, 8U, 8U) * 0.02;
    value.engine_to_wheel_torque_ratio =
        (double)extract_bits(raw, 18U, 14U) * 0.01;
    value.transmission_torque_loss_nm =
        (double)extract_bits(raw, 32U, 8U) * 0.25;
    value.vehicle_drive_style = (uint8_t)extract_bits(raw, 40U, 2U);
    value.transmission_style = (uint8_t)extract_bits(raw, 42U, 2U);
    value.transmission_mechanics_style =
        (uint8_t)extract_bits(raw, 44U, 2U);
    value.transmission_shift_style =
        (uint8_t)extract_bits(raw, 46U, 2U);
    value.message_counter = (uint8_t)extract_bits(raw, 48U, 4U);
    value.crc = (uint8_t)extract_bits(raw, 56U, 8U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_eng_rq3(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53EngRq3 *decoded)
{
    MblinkMercedesEgs53EngRq3 value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.maximum_acceleration_state =
        (uint8_t)extract_bits(raw, 1U, 2U);
    value.wet_driveaway_clutch_torque_nm =
        (double)extract_bits(raw, 3U, 13U) * 0.25 - 500.0;
    value.message_counter = (uint8_t)extract_bits(raw, 48U, 4U);
    value.crc = (uint8_t)extract_bits(raw, 56U, 8U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_sbw_rs_tcm(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53SbwRsTcm *decoded)
{
    MblinkMercedesEgs53SbwRsTcm value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.message_transmitter_id = (uint8_t)extract_bits(raw, 0U, 2U);
    value.starter_lockout = extract_bits(raw, 7U, 1U) != 0U;
    value.selector_valve_position = (uint8_t)extract_bits(raw, 8U, 4U);
    value.selector_position_request = (uint8_t)extract_bits(raw, 12U, 4U);
    value.selector_sensor_percent =
        (double)extract_bits(raw, 16U, 8U) * 0.4;
    value.message_counter = (uint8_t)extract_bits(raw, 48U, 4U);
    value.crc = (uint8_t)extract_bits(raw, 56U, 8U);

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_egs53_tcm_display_request(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53TcmDisplayRequest *decoded)
{
    MblinkMercedesEgs53TcmDisplayRequest value;
    uint64_t raw;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.display_position_code = (uint8_t)extract_bits(raw, 0U, 8U);
    value.display_program_code = (uint8_t)extract_bits(raw, 8U, 8U);
    value.shift_by_wire_beep_request = extract_bits(raw, 16U, 1U) != 0U;
    value.shift_recommendation = (uint8_t)extract_bits(raw, 17U, 2U);
    value.shift_by_wire_message = (uint8_t)extract_bits(raw, 21U, 3U);
    value.selector_lock_2_display = (uint8_t)extract_bits(raw, 24U, 4U);
    value.selector_lock_1_display = (uint8_t)extract_bits(raw, 28U, 4U);
    value.selector_lock_4_display = (uint8_t)extract_bits(raw, 32U, 4U);
    value.selector_lock_3_display = (uint8_t)extract_bits(raw, 36U, 4U);
    value.target_gear_display_code = (uint8_t)extract_bits(raw, 40U, 8U);
    value.race_start_display_state = (uint8_t)extract_bits(raw, 48U, 3U);

    *decoded = value;
    return true;
}


bool mblink_mercedes_transmission_format_can_frame(
    MblinkMercedesTransmissionFamily family,
    uint32_t can_id,
    const uint8_t *payload,
    size_t payload_length,
    char *buffer,
    size_t buffer_size)
{
    int count;
    if (payload == NULL || buffer == NULL || buffer_size == 0U) return false;
    buffer[0] = '\0';

    if (family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51 &&
        can_id == MBLINK_MERCEDES_GS_218_CAN_ID) {
        MblinkMercedesEgs51Gs218 d;
        if (!mblink_mercedes_transmission_decode_egs51_gs218(
                payload, payload_length, &d)) return false;
        count = snprintf(
            buffer, buffer_size,
            "EGS51 GS_218 · torque %.1f · target %s · actual %s · "
            "TCC %s · program-ok %s · limp %s · shifting %s · "
            "kickdown %s · start %s · P/N %s · FWD %s · error %u",
            d.torque_request,
            mblink_mercedes_transmission_target_gear_name(d.target_gear_code),
            mblink_mercedes_transmission_actual_gear_name(d.actual_gear_code),
            mblink_mercedes_transmission_tcc_name(d.torque_converter),
            d.gearbox_program_ok ? "yes" : "no",
            d.limp_home ? "yes" : "no",
            d.shifting ? "yes" : "no",
            d.kickdown ? "yes" : "no",
            d.start_enabled ? "enabled" : "blocked",
            d.park_or_neutral ? "yes" : "no",
            d.front_wheel_drive ? "yes" : "no",
            (unsigned int)d.error_counter);
        return count >= 0 && (size_t)count < buffer_size;
    }

    if (family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52) {
        if (can_id == MBLINK_MERCEDES_GS_218_CAN_ID) {
            MblinkMercedesGs218 d;
            if (!mblink_mercedes_transmission_decode_gs218(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS52 GS_218 · target %s · actual %s · TCC %s · "
                "shift %s · manual %s · gearbox-ok %s · limp %s · "
                "overtemp %s · kickdown %s · program %u · "
                "torque-request raw %u · error-state %u/%u",
                mblink_mercedes_transmission_target_gear_name(d.target_gear_code),
                mblink_mercedes_transmission_actual_gear_name(d.actual_gear_code),
                mblink_mercedes_transmission_tcc_name(d.torque_converter),
                d.shifting ? "yes" : "no",
                d.manual_shift_mode ? "yes" : "no",
                d.gearbox_ok ? "yes" : "no",
                d.limp_home ? "yes" : "no",
                d.overtemperature ? "yes" : "no",
                d.kickdown ? "yes" : "no",
                (unsigned int)d.drive_program_code,
                (unsigned int)d.requested_engine_torque_raw,
                (unsigned int)d.error_check_state,
                (unsigned int)d.error_counter);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_GS_338_CAN_ID) {
            MblinkMercedesGs338 d;
            if (!mblink_mercedes_transmission_decode_gs338(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS52 GS_338 · output %u rpm · turbine %u rpm · "
                "race-start %u · MIL %s · power-free-D %s · "
                "pilot-torque raw %u · starting-torque raw %u",
                (unsigned int)d.output_speed_rpm,
                (unsigned int)d.turbine_speed_rpm,
                (unsigned int)d.race_start_state,
                d.mil_request ? "requested" : "off",
                d.power_free_in_drive ? "yes" : "no",
                (unsigned int)d.pilot_torque_raw,
                (unsigned int)d.starting_area_torque_raw);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_GS_418_CAN_ID) {
            MblinkMercedesGs418 d;
            if (!mblink_mercedes_transmission_decode_gs418(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS52 GS_418 · display 0x%02X · program 0x%02X · "
                "ATF %.1f °C · target %s · actual %s · selector %s · "
                "AWD %s · FWD %s · CVT %s · mech %u · "
                "kickdown %s · torque-loss raw %u · wheel-factor raw %u",
                (unsigned int)d.display_position_code,
                (unsigned int)d.drive_program_code,
                d.transmission_temperature_c,
                mblink_mercedes_transmission_target_gear_name(d.target_gear_code),
                mblink_mercedes_transmission_actual_gear_name(d.actual_gear_code),
                mblink_mercedes_transmission_selector_name(d.selector_position_code),
                d.all_wheel_drive ? "yes" : "no",
                d.front_wheel_drive ? "yes" : "no",
                d.cvt ? "yes" : "no",
                (unsigned int)d.mechanism_variant,
                d.kickdown ? "yes" : "no",
                (unsigned int)d.torque_loss_raw,
                (unsigned int)d.wheel_torque_factor_raw);
            return count >= 0 && (size_t)count < buffer_size;
        }
    }

    if (family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53) {
        if (can_id == MBLINK_MERCEDES_EGS53_TCM_A1_CAN_ID) {
            MblinkMercedesEgs53TcmA1 d;
            if (!mblink_mercedes_transmission_decode_egs53_tcm_a1(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 TCM_A1 · ATF %.1f °C · clutch %u · limp %s · "
                "overtemp %s · selector %s · manual %s · program %u · "
                "drive-shaft torque %u Nm",
                d.oil_temperature_c, (unsigned int)d.clutch_state,
                d.limp_home ? "yes" : "no",
                d.overtemperature ? "yes" : "no",
                mblink_mercedes_transmission_selector_name(d.selector_position_code),
                d.manual_program_active ? "yes" : "no",
                (unsigned int)d.drive_program_code,
                (unsigned int)d.drive_shaft_torque_nm);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_EGS53_TCM_A2_CAN_ID) {
            MblinkMercedesEgs53TcmA2 d;
            if (!mblink_mercedes_transmission_decode_egs53_tcm_a2(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 TCM_A2 · duty %.1f%% · turbine %u rpm · "
                "desired slip %u rpm · MIL %s · CALID/CVN 0x%02X · error %u/%u",
                d.requested_current_duty_percent,
                (unsigned int)d.turbine_rpm,
                (unsigned int)d.desired_slip_rpm,
                d.mil_request ? "requested" : "off",
                (unsigned int)d.calid_cvn_data,
                (unsigned int)d.error_check_state,
                (unsigned int)d.calid_cvn_error_counter);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_EGS53_ENG_RQ1_CAN_ID) {
            MblinkMercedesEgs53EngRq1 d;
            if (!mblink_mercedes_transmission_decode_egs53_eng_rq1(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 ENG_RQ1 · requested torque %.1f Nm · "
                "requested engine %u rpm · intervention %u · downshift %u · "
                "sync %.2f s · start-enable %s · emergency-off %s",
                d.requested_engine_torque_nm,
                (unsigned int)d.requested_engine_rpm,
                (unsigned int)d.intervention_mode,
                (unsigned int)d.downshift_mode,
                d.engine_sync_time_s,
                d.engine_start_enable_request ? "yes" : "no",
                d.emergency_engine_off_request ? "yes" : "no");
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_EGS53_ENG_RQ2_CAN_ID) {
            MblinkMercedesEgs53EngRq2 d;
            if (!mblink_mercedes_transmission_decode_egs53_eng_rq2(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 ENG_RQ2 · target %s · actual %s · ratio %.2f · "
                "engine/wheel ratio %.2f · loss %.2f Nm · "
                "drive-style %u · transmission-style %u · mechanics %u · shift-style %u",
                mblink_mercedes_transmission_target_gear_name(d.target_gear_code),
                mblink_mercedes_transmission_actual_gear_name(d.actual_gear_code),
                d.transmission_ratio,
                d.engine_to_wheel_torque_ratio,
                d.transmission_torque_loss_nm,
                (unsigned int)d.vehicle_drive_style,
                (unsigned int)d.transmission_style,
                (unsigned int)d.transmission_mechanics_style,
                (unsigned int)d.transmission_shift_style);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_EGS53_ENG_RQ3_CAN_ID) {
            MblinkMercedesEgs53EngRq3 d;
            if (!mblink_mercedes_transmission_decode_egs53_eng_rq3(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 ENG_RQ3 · max-acceleration state %u · wet clutch torque %.1f Nm",
                (unsigned int)d.maximum_acceleration_state,
                d.wet_driveaway_clutch_torque_nm);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_EGS53_SBW_RS_TCM_CAN_ID) {
            MblinkMercedesEgs53SbwRsTcm d;
            if (!mblink_mercedes_transmission_decode_egs53_sbw_rs_tcm(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 SBW_RS_TCM · sender %u · starter lockout %s · "
                "selector valve %u · selector request %u · sensor %.1f%%",
                (unsigned int)d.message_transmitter_id,
                d.starter_lockout ? "yes" : "no",
                (unsigned int)d.selector_valve_position,
                (unsigned int)d.selector_position_request,
                d.selector_sensor_percent);
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (can_id == MBLINK_MERCEDES_EGS53_TCM_DISP_RQ_CAN_ID) {
            MblinkMercedesEgs53TcmDisplayRequest d;
            if (!mblink_mercedes_transmission_decode_egs53_tcm_display_request(
                    payload, payload_length, &d)) return false;
            count = snprintf(
                buffer, buffer_size,
                "EGS53 TCM_DISP_RQ · position 0x%02X · program 0x%02X · "
                "shift recommendation %u · SBW message %u · target display 0x%02X · "
                "race-start display %u",
                (unsigned int)d.display_position_code,
                (unsigned int)d.display_program_code,
                (unsigned int)d.shift_recommendation,
                (unsigned int)d.shift_by_wire_message,
                (unsigned int)d.target_gear_display_code,
                (unsigned int)d.race_start_display_state);
            return count >= 0 && (size_t)count < buffer_size;
        }
    }

    return false;
}

static const uint8_t k_kwp_oem_metadata_ids[] = {
    UINT8_C(0xe0), UINT8_C(0xe1), UINT8_C(0xe2), UINT8_C(0xe3),
    UINT8_C(0xe4), UINT8_C(0xe5), UINT8_C(0xe6), UINT8_C(0xe7),
    UINT8_C(0xe8), UINT8_C(0xe9), UINT8_C(0xea), UINT8_C(0xeb)
};

static const uint8_t k_kwp_egs52_ids[] = {
    UINT8_C(0x30), UINT8_C(0x31), UINT8_C(0x32), UINT8_C(0x33),
    UINT8_C(0xe0), UINT8_C(0xe1), UINT8_C(0xe2), UINT8_C(0xe3),
    UINT8_C(0xe4), UINT8_C(0xe5), UINT8_C(0xe6), UINT8_C(0xe7),
    UINT8_C(0xe8), UINT8_C(0xe9), UINT8_C(0xea), UINT8_C(0xeb)
};

static const uint8_t k_kwp_vgs_nag2_ids[] = {
    UINT8_C(0x30),
    UINT8_C(0xe0), UINT8_C(0xe1), UINT8_C(0xe2), UINT8_C(0xe3),
    UINT8_C(0xe4), UINT8_C(0xe5), UINT8_C(0xe6), UINT8_C(0xe7),
    UINT8_C(0xe8), UINT8_C(0xe9), UINT8_C(0xea), UINT8_C(0xeb)
};

/*
 * Ultimate NAG52 is a replacement-controller firmware, not an OEM EGS52
 * diagnostic namespace. Its GPLv3 source defines these read-only live records
 * explicitly. In particular, 0x30 means clutch speeds here, not the OEM EGS52
 * RLI 0x30 actual-values layout.
 */
static const uint8_t k_kwp_ultimate_nag52_ids[] = {
    UINT8_C(0x20), UINT8_C(0x21), UINT8_C(0x22), UINT8_C(0x23),
    UINT8_C(0x24), UINT8_C(0x25), UINT8_C(0x27), UINT8_C(0x30),
    UINT8_C(0x31), UINT8_C(0x32)
};

static bool contains_ascii_ci(const char *text, const char *needle)
{
    size_t text_length;
    size_t needle_length;
    size_t start;
    size_t offset;

    if (text == NULL || needle == NULL || needle[0] == '\0') return false;
    text_length = strlen(text);
    needle_length = strlen(needle);
    if (needle_length > text_length) return false;

    for (start = 0U; start + needle_length <= text_length; ++start) {
        for (offset = 0U; offset < needle_length; ++offset) {
            const unsigned char left = (unsigned char)text[start + offset];
            const unsigned char right = (unsigned char)needle[offset];
            if (tolower(left) != tolower(right)) break;
        }
        if (offset == needle_length) return true;
    }
    return false;
}

MblinkMercedesTransmissionFamily
mblink_mercedes_transmission_family_from_identity(const char *identity)
{
    if (identity == NULL || identity[0] == '\0')
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN;

    if (contains_ascii_ci(identity, "ULTIMATE NAG52") ||
        contains_ascii_ci(identity, "ULTIMATE-NAG52")) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52;
    }
    if (contains_ascii_ci(identity, "EGS51"))
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51;
    if (contains_ascii_ci(identity, "EGS52"))
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52;
    if (contains_ascii_ci(identity, "EGS53"))
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53;
    /*
     * Match the specific later families before NAG2. "VGS" by itself is not
     * a safe family discriminator because Mercedes reused VGS terminology.
     */
    if (contains_ascii_ci(identity, "725.0") ||
        contains_ascii_ci(identity, "9G-TRONIC") ||
        contains_ascii_ci(identity, "9G TRONIC")) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G;
    }
    if (contains_ascii_ci(identity, "724.0") ||
        contains_ascii_ci(identity, "7G-DCT")) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_7240_DCT;
    }
    if (contains_ascii_ci(identity, "722.8") ||
        contains_ascii_ci(identity, "CVT")) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_7228_CVT;
    }
    if (contains_ascii_ci(identity, "722.9") ||
        contains_ascii_ci(identity, "NAG2")) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2;
    }
    if (contains_ascii_ci(identity, "AMG MCT"))
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_MCT;
    if (contains_ascii_ci(identity, "AMG DCT"))
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_DCT;
    if (contains_ascii_ci(identity, "SINGLE-SPEED") ||
        contains_ascii_ci(identity, "SINGLE SPEED")) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_EV_SINGLE_SPEED;
    }
    return MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN;
}

static const uint8_t *kwp_ids_for_family(
    MblinkMercedesTransmissionFamily family, size_t *count)
{
    if (count != NULL) *count = 0U;

    switch (family) {
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52:
        if (count != NULL)
            *count = sizeof(k_kwp_egs52_ids) / sizeof(k_kwp_egs52_ids[0]);
        return k_kwp_egs52_ids;
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2:
        if (count != NULL)
            *count = sizeof(k_kwp_vgs_nag2_ids) /
                sizeof(k_kwp_vgs_nag2_ids[0]);
        return k_kwp_vgs_nag2_ids;
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52:
        if (count != NULL)
            *count = sizeof(k_kwp_ultimate_nag52_ids) /
                sizeof(k_kwp_ultimate_nag52_ids[0]);
        return k_kwp_ultimate_nag52_ids;
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53:
        if (count != NULL)
            *count = sizeof(k_kwp_oem_metadata_ids) /
                sizeof(k_kwp_oem_metadata_ids[0]);
        return k_kwp_oem_metadata_ids;
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7228_CVT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7240_DCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_MCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_DCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EV_SINGLE_SPEED:
        return NULL;
    }
    return NULL;
}

size_t mblink_mercedes_transmission_kwp_read_identifier_count_for_family(
    MblinkMercedesTransmissionFamily family)
{
    size_t count = 0U;
    (void)kwp_ids_for_family(family, &count);
    return count;
}

uint8_t mblink_mercedes_transmission_kwp_read_identifier_at_for_family(
    MblinkMercedesTransmissionFamily family, size_t index)
{
    size_t count = 0U;
    const uint8_t *ids = kwp_ids_for_family(family, &count);
    return ids != NULL && index < count ? ids[index] : 0U;
}

static const char *kwp_oem_metadata_name(uint8_t id)
{
    switch (id) {
    case UINT8_C(0xe0): return "Development data";
    case UINT8_C(0xe1): return "ECU serial number";
    case UINT8_C(0xe2): return "DBCom communication-matrix data";
    case UINT8_C(0xe3): return "Operating-system version";
    case UINT8_C(0xe4): return "ECU reprogramming identification";
    case UINT8_C(0xe5): return "Vehicle information";
    case UINT8_C(0xe6): return "Flash information 1";
    case UINT8_C(0xe7): return "Flash information 2";
    case UINT8_C(0xe8): return "System-diagnostic general parameters";
    case UINT8_C(0xe9): return "System-diagnostic global parameters";
    case UINT8_C(0xea): return "ECU configuration";
    case UINT8_C(0xeb): return "Diagnostic protocol information";
    default: return NULL;
    }
}

const char *mblink_mercedes_transmission_kwp_read_identifier_name_for_family(
    MblinkMercedesTransmissionFamily family, uint8_t id)
{
    const char *metadata = kwp_oem_metadata_name(id);
    if (metadata != NULL &&
        family != MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52) {
        return metadata;
    }

    switch (family) {
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52:
        switch (id) {
        case UINT8_C(0x30): return "Transmission actual values / RLI 30";
        case UINT8_C(0x31): return "Transmission speed sensors / RLI 31";
        case UINT8_C(0x32): return "Transmission driving dynamics / RLI 32";
        case UINT8_C(0x33):
            return "Transmission hydraulics and solenoids / RLI 33";
        default: return NULL;
        }
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2:
        return id == UINT8_C(0x30)
            ? "Transmission actual values / compact RLI 30" : NULL;
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52:
        switch (id) {
        case UINT8_C(0x20): return "Gearbox sensors";
        case UINT8_C(0x21): return "Solenoid status";
        case UINT8_C(0x22): return "CAN data";
        case UINT8_C(0x23): return "System usage";
        case UINT8_C(0x24): return "Torque-converter status";
        case UINT8_C(0x25): return "Gearbox pressures";
        case UINT8_C(0x27): return "Shift live data";
        case UINT8_C(0x30): return "Clutch speeds";
        case UINT8_C(0x31): return "Shift algorithm feedback";
        case UINT8_C(0x32): return "Driving dynamics";
        default: return NULL;
        }
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7228_CVT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7240_DCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_MCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_DCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EV_SINGLE_SPEED:
        return NULL;
    }
    return NULL;
}

bool mblink_mercedes_transmission_kwp_identifier_is_live_for_family(
    MblinkMercedesTransmissionFamily family, uint8_t id)
{
    switch (family) {
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52:
        return id >= UINT8_C(0x30) && id <= UINT8_C(0x33);
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2:
        return id == UINT8_C(0x30);
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52:
        return id == UINT8_C(0x20) || id == UINT8_C(0x21) ||
               id == UINT8_C(0x22) || id == UINT8_C(0x23) ||
               id == UINT8_C(0x24) || id == UINT8_C(0x25) ||
               id == UINT8_C(0x27) || id == UINT8_C(0x30) ||
               id == UINT8_C(0x31) || id == UINT8_C(0x32);
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7228_CVT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7240_DCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_MCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_DCT:
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EV_SINGLE_SPEED:
        return false;
    }
    return false;
}

size_t mblink_mercedes_transmission_kwp_read_identifier_count(void)
{
    return mblink_mercedes_transmission_kwp_read_identifier_count_for_family(
        MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52);
}

uint8_t mblink_mercedes_transmission_kwp_read_identifier_at(size_t index)
{
    return mblink_mercedes_transmission_kwp_read_identifier_at_for_family(
        MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52, index);
}

const char *mblink_mercedes_transmission_kwp_read_identifier_name(uint8_t id)
{
    return mblink_mercedes_transmission_kwp_read_identifier_name_for_family(
        MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52, id);
}

const char *mblink_mercedes_transmission_family_name(
    MblinkMercedesTransmissionFamily family)
{
    switch (family) {
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN: return "Unknown";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51: return "EGS51";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52: return "EGS52 / NAG1";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53: return "EGS53";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52:
        return "Ultimate NAG52";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2: return "VGS / NAG2 / 722.9";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7228_CVT: return "722.8 CVT";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7240_DCT: return "724.0 7G-DCT";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G: return "725.0 9G-Tronic";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_MCT: return "AMG MCT";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_DCT: return "AMG DCT";
    case MBLINK_MERCEDES_TRANSMISSION_FAMILY_EV_SINGLE_SPEED: return "EV single-speed";
    }
    return "Unknown";
}

const char *mblink_mercedes_transmission_actual_gear_name(uint8_t code)
{
    switch (code & UINT8_C(0x0f)) {
    case 0U: return "N";
    case 1U: return "1";
    case 2U: return "2";
    case 3U: return "3";
    case 4U: return "4";
    case 5U: return "5";
    case 6U: return "6";
    case 7U: return "7";
    case 8U: return "D-CVT";
    case 9U: return "R-CVT";
    case 10U: return "R3";
    case 11U: return "R";
    case 12U: return "R2";
    case 13U: return "P";
    case 14U: return "No force";
    case 15U: return "Unavailable";
    }
    return "Unavailable";
}

const char *mblink_mercedes_transmission_target_gear_name(uint8_t code)
{
    switch (code & UINT8_C(0x0f)) {
    case 0U: return "N";
    case 1U: return "1";
    case 2U: return "2";
    case 3U: return "3";
    case 4U: return "4";
    case 5U: return "5";
    case 6U: return "6";
    case 7U: return "7";
    case 8U: return "D-CVT";
    case 9U: return "R-CVT";
    case 10U: return "R3";
    case 11U: return "R";
    case 12U: return "R2";
    case 13U: return "P";
    case 14U: return "Cancel";
    case 15U: return "Unavailable";
    }
    return "Unavailable";
}

const char *mblink_mercedes_transmission_selector_name(uint8_t code)
{
    switch (code & UINT8_C(0x07)) {
    case 0U: return "P";
    case 1U: return "R";
    case 2U: return "N";
    case 4U: return "D";
    case 7U: return "Unavailable";
    default: return "Intermediate";
    }
}

const char *mblink_mercedes_transmission_tcc_name(
    MblinkMercedesTorqueConverterState state)
{
    switch (state) {
    case MBLINK_MERCEDES_TCC_UNKNOWN: return "unknown";
    case MBLINK_MERCEDES_TCC_SLIPPING: return "slipping";
    case MBLINK_MERCEDES_TCC_OPEN: return "open";
    case MBLINK_MERCEDES_TCC_CLOSED: return "closed";
    case MBLINK_MERCEDES_TCC_CONFLICT: return "conflicting-state-bits";
    }
    return "unknown";
}
