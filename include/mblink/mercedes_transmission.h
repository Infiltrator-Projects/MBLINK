// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_transmission.h
 * @brief Read-only Mercedes GS/VGS transmission diagnostic and broadcast decoders.
 *
 * The frame layouts in this file are source-backed Mercedes GS definitions.
 * Decoding a frame never causes vehicle transmission: callers supply bytes that
 * were already received from a diagnostic response or passive CAN capture.
 */
#ifndef MBLINK_MERCEDES_TRANSMISSION_H
#define MBLINK_MERCEDES_TRANSMISSION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_GS_218_CAN_ID UINT32_C(0x0218)
#define MBLINK_MERCEDES_GS_338_CAN_ID UINT32_C(0x0338)
#define MBLINK_MERCEDES_GS_418_CAN_ID UINT32_C(0x0418)

typedef enum MblinkMercedesTorqueConverterState {
    MBLINK_MERCEDES_TCC_UNKNOWN = 0,
    MBLINK_MERCEDES_TCC_SLIPPING,
    MBLINK_MERCEDES_TCC_OPEN,
    MBLINK_MERCEDES_TCC_CLOSED,
    MBLINK_MERCEDES_TCC_CONFLICT
} MblinkMercedesTorqueConverterState;

typedef struct MblinkMercedesTransmission2130 {
    bool oil_temperature_available;
    double oil_temperature_c;
    bool actual_gear_code_available;
    uint8_t actual_gear_code;
} MblinkMercedesTransmission2130;

typedef struct MblinkMercedesGs218 {
    uint8_t target_gear_code;
    uint8_t actual_gear_code;
    MblinkMercedesTorqueConverterState torque_converter;
    bool manual_shift_mode;
    bool shifting;
    bool gearbox_ok;
    bool limp_home;
    bool overtemperature;
    bool kickdown;
    uint8_t drive_program_code;
    bool off_road_gear;
    bool start_enabled;
    bool converter_unloaded;
} MblinkMercedesGs218;

typedef struct MblinkMercedesGs338 {
    uint16_t output_speed_raw;
    uint16_t turbine_speed_raw;
} MblinkMercedesGs338;

typedef struct MblinkMercedesGs418 {
    uint8_t display_position_code;
    uint8_t drive_program_code;
    uint8_t transmission_temperature_raw;
    bool all_wheel_drive;
    bool front_wheel_drive;
    bool shifting;
    bool cvt;
    uint8_t mechanism_variant;
    bool brake_required;
    bool kickdown;
    uint8_t target_gear_code;
    uint8_t actual_gear_code;
    uint8_t torque_loss_raw;
    uint8_t selector_position_code;
    uint16_t wheel_torque_factor_raw;
} MblinkMercedesGs418;

/**
 * Decode the source-corroborated 7E1/7E9 KWP local-ID 0x30 response payload.
 * The data pointer starts after the leading positive response bytes 61 30.
 */
bool mblink_mercedes_transmission_decode_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmission2130 *decoded);

bool mblink_mercedes_transmission_decode_gs218(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs218 *decoded);

bool mblink_mercedes_transmission_decode_gs338(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs338 *decoded);

bool mblink_mercedes_transmission_decode_gs418(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesGs418 *decoded);

const char *mblink_mercedes_transmission_actual_gear_name(uint8_t code);
const char *mblink_mercedes_transmission_target_gear_name(uint8_t code);
const char *mblink_mercedes_transmission_selector_name(uint8_t code);
const char *mblink_mercedes_transmission_tcc_name(
    MblinkMercedesTorqueConverterState state);

#ifdef __cplusplus
}
#endif
#endif
