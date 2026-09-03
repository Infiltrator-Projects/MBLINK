// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_transmission.h"

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
     * of complete response byte F.  F is index 5 of the complete response,
     * hence data[3] after stripping 61 30. Keep the raw 4-bit code so the UI
     * can display every observed state without inventing P/R semantics.
     */
    value.actual_gear_code_available = data_length >= 4U;
    value.actual_gear_code = (uint8_t)(data[3] & UINT8_C(0x0f));

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
    bool tcc_slipping;
    bool tcc_open;
    bool tcc_closed;

    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));
    raw = payload_le64(payload);

    value.target_gear_code = (uint8_t)extract_bits(raw, 16U, 4U);
    value.actual_gear_code = (uint8_t)extract_bits(raw, 20U, 4U);
    tcc_slipping = extract_bits(raw, 24U, 1U) != 0U;
    tcc_open = extract_bits(raw, 25U, 1U) != 0U;
    tcc_closed = extract_bits(raw, 26U, 1U) != 0U;
    value.torque_converter = tcc_state(tcc_slipping, tcc_open, tcc_closed);
    value.off_road_gear = extract_bits(raw, 27U, 1U) != 0U;
    value.shifting = extract_bits(raw, 30U, 1U) != 0U;
    value.manual_shift_mode = extract_bits(raw, 31U, 1U) != 0U;
    value.gearbox_ok = extract_bits(raw, 32U, 1U) != 0U;
    value.start_enabled = extract_bits(raw, 34U, 1U) != 0U;
    value.limp_home = extract_bits(raw, 35U, 1U) != 0U;
    value.overtemperature = extract_bits(raw, 36U, 1U) != 0U;
    value.kickdown = extract_bits(raw, 37U, 1U) != 0U;
    value.drive_program_code = (uint8_t)extract_bits(raw, 38U, 2U);
    value.converter_unloaded = extract_bits(raw, 45U, 1U) != 0U;

    *decoded = value;
    return true;
}

bool mblink_mercedes_transmission_decode_gs338(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs338 *decoded)
{
    MblinkMercedesGs338 value;
    if (payload == NULL || decoded == NULL || payload_length != 8U)
        return false;
    memset(&value, 0, sizeof(value));

    /*
     * Preserve the source frame's two 16-bit integers losslessly. Their
     * engineering scaling can be promoted independently from frame identity.
     */
    value.output_speed_raw =
        (uint16_t)((uint16_t)payload[0] |
                   ((uint16_t)payload[1] << 8U));
    value.turbine_speed_raw =
        (uint16_t)((uint16_t)payload[6] |
                   ((uint16_t)payload[7] << 8U));

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
    value.selector_position_code = (uint8_t)extract_bits(raw, 50U, 3U);
    value.wheel_torque_factor_raw =
        (uint16_t)extract_bits(raw, 53U, 11U);

    *decoded = value;
    return true;
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
