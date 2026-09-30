// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_transmission.h
 * @brief Read-only Mercedes EGS/VGS transmission diagnostic and CAN decoders.
 *
 * The definitions in this file are deliberately family-scoped. Mercedes used
 * materially different transmission messages across EGS51, EGS52 and EGS53,
 * while later VGS/NAG2 controllers also expose useful KWP2000 local records.
 * Decoding never transmits anything: callers provide bytes already obtained
 * from a read-only diagnostic response or passive CAN observation.
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

#define MBLINK_MERCEDES_EGS53_TCM_A1_CAN_ID UINT32_C(0x02f1)
#define MBLINK_MERCEDES_EGS53_TCM_A2_CAN_ID UINT32_C(0x02e2)
#define MBLINK_MERCEDES_EGS53_ENG_RQ1_CAN_ID UINT32_C(0x00f1)
#define MBLINK_MERCEDES_EGS53_ENG_RQ2_CAN_ID UINT32_C(0x00f3)
#define MBLINK_MERCEDES_EGS53_ENG_RQ3_CAN_ID UINT32_C(0x00f4)
#define MBLINK_MERCEDES_EGS53_SBW_RS_TCM_CAN_ID UINT32_C(0x01bd)
#define MBLINK_MERCEDES_EGS53_TCM_DISP_RQ_CAN_ID UINT32_C(0x02f3)

typedef enum MblinkMercedesTransmissionFamily {
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN = 0,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS51,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_VGS_NAG2,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_7228_CVT,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_7240_DCT,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_7250_9G,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_MCT,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_AMG_DCT,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_EV_SINGLE_SPEED,
    MBLINK_MERCEDES_TRANSMISSION_FAMILY_ULTIMATE_NAG52
} MblinkMercedesTransmissionFamily;

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

/**
 * Siemens EGS53 KWP local identifier B1 variant-coding record.
 *
 * The first 42 bytes are the coding record observed on a field-verified
 * A0034464310 EGS53. Bytes 43+ are retained as trailing metadata rather than
 * silently folded into the coding. Fields whose public mapping is not yet
 * fully corroborated are named as candidates so the UI can show confidence.
 */
typedef struct MblinkMercedesEgs53VariantCoding {
    char variant_code[5];
    uint8_t drive_program_flags;
    bool comfort_sport_coding;
    bool manual_program_coding;
    bool agility_program_coding;
    uint8_t paddle_coding_flags;
    bool paddle_coding_bit_set;
    uint16_t rear_axle_ratio_milli;
    uint16_t tyre_circumference_mm_candidate;
    uint16_t stored_crc;
    uint16_t calculated_crc;
    bool crc_valid;
    uint8_t trailing_metadata[4];
    size_t trailing_metadata_length;
} MblinkMercedesEgs53VariantCoding;

bool mblink_mercedes_transmission_decode_egs53_variant_coding(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesEgs53VariantCoding *decoded);
bool mblink_mercedes_transmission_format_egs53_variant_coding(
    const uint8_t *data,
    size_t data_length,
    char *buffer,
    size_t buffer_size);

/**
 * Canonical live values recoverable from a source/vehicle-corroborated
 * Mercedes GS KWP 21 30 response. The decoder qualifies the response by
 * layout/length and never treats diagnostic route 0x7E1 -> 0x7E9 as proof of
 * a particular EGS/VGS controller family.
 */
typedef struct MblinkMercedesTransmissionLive2130 {
    bool rich_layout;
    bool oil_temperature_available;
    double oil_temperature_c;
    bool actual_gear_available;
    uint8_t actual_gear_code;
    bool target_gear_available;
    uint8_t target_gear_code;
    bool tcc_status_available;
    uint8_t tcc_status_code;
    bool recognised_gear_available;
    uint8_t recognised_gear_code;
    bool selector_position_available;
    uint8_t selector_position_code;
    bool drive_program_available;
    uint8_t drive_program_code;
    bool tcc_delta_speed_available;
    uint16_t tcc_delta_speed_raw;
    bool tcc_speed_available;
    uint16_t tcc_speed_raw;
    bool tcc_pressure_available;
    uint16_t tcc_pressure_raw;
    bool engine_torque_available;
    int16_t engine_torque_signed_raw;
    bool converter_torque_available;
    int16_t converter_torque_signed_raw;
    bool output_speed_available;
    uint16_t output_speed_raw;
} MblinkMercedesTransmissionLive2130;

/**
 * Independently selectable values carried by KWP local record 0x30.
 *
 * Each bit selects one logical value while the wire request remains the single
 * 21 30 transaction. The order is persistent API for selection masks.
 */
typedef enum MblinkMercedesTransmission2130Field {
    MBLINK_MERCEDES_TRANSMISSION_2130_OIL_TEMPERATURE = 0,
    MBLINK_MERCEDES_TRANSMISSION_2130_ACTUAL_GEAR,
    MBLINK_MERCEDES_TRANSMISSION_2130_TARGET_GEAR,
    MBLINK_MERCEDES_TRANSMISSION_2130_TCC_STATE,
    MBLINK_MERCEDES_TRANSMISSION_2130_RECOGNISED_GEAR,
    MBLINK_MERCEDES_TRANSMISSION_2130_SELECTOR_POSITION,
    MBLINK_MERCEDES_TRANSMISSION_2130_DRIVE_PROGRAM,
    MBLINK_MERCEDES_TRANSMISSION_2130_TCC_DELTA_SPEED,
    MBLINK_MERCEDES_TRANSMISSION_2130_TCC_SPEED,
    MBLINK_MERCEDES_TRANSMISSION_2130_TCC_PRESSURE,
    MBLINK_MERCEDES_TRANSMISSION_2130_ENGINE_TORQUE,
    MBLINK_MERCEDES_TRANSMISSION_2130_CONVERTER_TORQUE,
    MBLINK_MERCEDES_TRANSMISSION_2130_OUTPUT_SPEED,
    MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_COUNT
} MblinkMercedesTransmission2130Field;

#define MBLINK_MERCEDES_TRANSMISSION_2130_FIELD_MASK(FIELD) \
    (UINT64_C(1) << (unsigned int)(FIELD))

/* EGS51 / early 722.6: six-byte GS_218h broadcast. */
typedef struct MblinkMercedesEgs51Gs218 {
    double torque_request;
    bool automatic_transmission;
    bool gearbox_program_ok;
    bool off_road;
    bool park_or_neutral;
    bool garage_shift;
    bool start_enabled;
    bool torque_request_enabled;
    uint8_t target_gear_code;
    uint8_t actual_gear_code;
    MblinkMercedesTorqueConverterState torque_converter;
    bool large_gearbox;
    bool limp_home;
    bool shifting;
    bool kickdown;
    bool front_wheel_drive;
    uint8_t tcc_torque_multiplier_raw;
    uint8_t error_counter;
} MblinkMercedesEgs51Gs218;

/* EGS52 / 722.6 source matrix. */
typedef struct MblinkMercedesGs218 {
    bool engine_torque_toggle;
    bool engine_torque_request_min;
    bool engine_torque_request_max;
    uint16_t requested_engine_torque_raw;
    uint8_t target_gear_code;
    uint8_t actual_gear_code;
    MblinkMercedesTorqueConverterState torque_converter;
    bool off_road_gear;
    bool basic_shift_program_ok;
    bool driving_resistance_high;
    bool shifting;
    bool manual_shift_mode;
    bool gearbox_ok;
    bool start_bang;
    bool start_enabled;
    bool limp_home;
    bool overtemperature;
    bool kickdown;
    uint8_t drive_program_code;
    bool engine_torque_parity;
    bool drivetrain_control_1;
    bool drivetrain_control_0;
    bool converter_unloaded;
    bool emergency_engine_off_confirm;
    bool emergency_engine_off;
    uint8_t creep_or_calid_raw;
    uint8_t error_check_state;
    bool calid_cvn_active;
    uint8_t error_counter;
} MblinkMercedesGs218;

typedef struct MblinkMercedesGs338 {
    uint16_t output_speed_rpm;
    bool mil_request;
    bool nak_parity;
    bool nak_toggle;
    bool power_free_in_drive;
    uint8_t race_start_state;
    uint16_t pilot_torque_raw;
    uint16_t starting_area_torque_raw;
    uint16_t turbine_speed_rpm;
} MblinkMercedesGs338;

typedef struct MblinkMercedesGs418 {
    uint8_t display_position_code;
    uint8_t drive_program_code;
    uint8_t transmission_temperature_raw;
    double transmission_temperature_c;
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
    bool wheel_torque_parity;
    bool wheel_torque_toggle;
    uint8_t selector_position_code;
    uint16_t wheel_torque_factor_raw;
} MblinkMercedesGs418;

/*
 * Stock-EGS52-compatible KWP2000 local records reverse engineered for DAS.
 * 16-bit fields are retained as source-order numeric values; unknown physical
 * scaling remains explicitly raw rather than guessed.
 */
typedef struct MblinkMercedesKwpRli30 {
    uint16_t tcc_delta_speed_raw;
    uint16_t tcc_speed_raw;
    uint16_t tcc_pressure_raw;
    uint8_t tcc_status;
    uint8_t selector_position;
    uint8_t drive_program;
    uint8_t recognised_gear;
    uint8_t actual_gear_code;
    uint8_t target_gear_code;
    double atf_temperature_c;
    uint16_t engine_torque_raw;
    int16_t engine_torque_signed_raw;
    uint16_t converter_torque_raw;
    int16_t converter_torque_signed_raw;
    uint16_t output_speed_raw;
    bool kickdown;
    bool start_enable;
    bool start_lockout_reason;
    bool reverse_park_lock;
    bool starter_relay;
    bool solenoid_1245;
    bool solenoid_23;
    bool solenoid_34;
    bool program_button;
    bool selector_plus;
    bool selector_minus;
    bool not_switching;
    bool gear_protection;
    bool tcc_open_request;
    bool downshift;
    bool upshift;
    bool drivetrain_control_0;
    bool drivetrain_control_1;
    bool release_circuit;
    bool steering_wheel_plus;
    bool steering_wheel_minus;
    bool converter_clutch_enabled;
    bool current_error;
    bool emergency_mode;
    bool asr_active;
    bool transmission_protection_ack;
    bool bang_start;
    bool minimum_torque_request;
    bool circuit_break;
    bool double_circuit;
    bool converter_ok;
} MblinkMercedesKwpRli30;

typedef struct MblinkMercedesKwpRli31 {
    uint16_t n2_pulse_count;
    uint16_t n3_pulse_count;
    uint16_t input_rpm;
    uint16_t engine_rpm;
    uint16_t front_left_wheel_speed_raw;
    uint16_t front_right_wheel_speed_raw;
    uint16_t rear_left_wheel_speed_raw;
    uint16_t rear_right_wheel_speed_raw;
    uint16_t rear_vehicle_speed_raw;
    uint16_t front_vehicle_speed_raw;
} MblinkMercedesKwpRli31;

typedef struct MblinkMercedesKwpRli32 {
    uint8_t pedal_percent;
    uint16_t upshift_delta_rpm_raw;
    uint16_t downshift_delta_rpm_raw;
    uint8_t pedal_delta_percent;
    uint16_t pitch_raw;
    uint8_t driving_status;
    uint8_t engine_warmup_shift_state;
    uint8_t requested_low_gear_limit;
    uint8_t requested_high_gear_limit;
} MblinkMercedesKwpRli32;

typedef struct MblinkMercedesKwpRli33 {
    uint8_t valve_flag;
    uint8_t shift_valve_state;
    uint16_t spc_pressure_raw;
    uint16_t mpc_pressure_raw;
    uint16_t spc_target_current_raw;
    uint16_t spc_actual_current_raw;
    uint16_t mpc_target_current_raw;
    uint16_t mpc_actual_current_raw;
    uint8_t tcc_pwm_raw;
} MblinkMercedesKwpRli33;

/* EGS53 / later 722.6 and VGS-compatible powertrain CAN matrix. */
typedef struct MblinkMercedesEgs53TcmA1 {
    double oil_temperature_c;
    bool tcc_no_load;
    uint8_t clutch_state;
    bool limp_home;
    bool basic_shift_program_ok;
    bool driving_resistance_high;
    bool overtemperature;
    bool off_road_active;
    uint8_t selector_position_code;
    bool manual_program_active;
    uint8_t drive_program_code;
    bool start_brake_request;
    uint16_t drive_shaft_torque_nm;
} MblinkMercedesEgs53TcmA1;

typedef struct MblinkMercedesEgs53TcmA2 {
    double requested_current_duty_percent;
    uint16_t turbine_rpm;
    bool mil_request;
    uint16_t desired_slip_rpm;
    uint8_t calid_cvn_data;
    uint8_t error_check_state;
    bool calid_cvn_active;
    uint8_t calid_cvn_error_counter;
} MblinkMercedesEgs53TcmA2;

typedef struct MblinkMercedesEgs53EngRq1 {
    bool torque_request_min;
    bool torque_request_max;
    double requested_engine_torque_nm;
    uint8_t intervention_mode;
    uint8_t downshift_mode;
    double engine_sync_time_s;
    bool stop_start_enable_request;
    uint16_t requested_engine_rpm;
    uint8_t message_counter;
    bool engine_start_enable_request;
    bool emergency_engine_off_request;
    bool jump_start_active;
    uint8_t crc;
} MblinkMercedesEgs53EngRq1;

typedef struct MblinkMercedesEgs53EngRq2 {
    uint8_t target_gear_code;
    uint8_t actual_gear_code;
    double transmission_ratio;
    double engine_to_wheel_torque_ratio;
    double transmission_torque_loss_nm;
    uint8_t vehicle_drive_style;
    uint8_t transmission_style;
    uint8_t transmission_mechanics_style;
    uint8_t transmission_shift_style;
    uint8_t message_counter;
    uint8_t crc;
} MblinkMercedesEgs53EngRq2;

typedef struct MblinkMercedesEgs53EngRq3 {
    uint8_t maximum_acceleration_state;
    double wet_driveaway_clutch_torque_nm;
    uint8_t message_counter;
    uint8_t crc;
} MblinkMercedesEgs53EngRq3;

typedef struct MblinkMercedesEgs53SbwRsTcm {
    uint8_t message_transmitter_id;
    bool starter_lockout;
    uint8_t selector_valve_position;
    uint8_t selector_position_request;
    double selector_sensor_percent;
    uint8_t message_counter;
    uint8_t crc;
} MblinkMercedesEgs53SbwRsTcm;

typedef struct MblinkMercedesEgs53TcmDisplayRequest {
    uint8_t display_position_code;
    uint8_t display_program_code;
    bool shift_by_wire_beep_request;
    uint8_t shift_recommendation;
    uint8_t shift_by_wire_message;
    uint8_t selector_lock_2_display;
    uint8_t selector_lock_1_display;
    uint8_t selector_lock_4_display;
    uint8_t selector_lock_3_display;
    uint8_t target_gear_display_code;
    uint8_t race_start_display_state;
} MblinkMercedesEgs53TcmDisplayRequest;

bool mblink_mercedes_transmission_decode_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmission2130 *decoded);

bool mblink_mercedes_transmission_decode_live_2130(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmissionLive2130 *decoded);

/**
 * True only for controller families where local identifier 0x30 is
 * documented as the transmission actual-values record decoded above.
 * UNKNOWN is deliberately false: route evidence may justify reading
 * 0x30, but it does not prove the controller family's semantics.
 */
bool mblink_mercedes_transmission_family_uses_2130_actual_values(
    MblinkMercedesTransmissionFamily family);

/**
 * Family-qualified 21 30 decoder. EGS52 requires the rich RLI-30
 * shape; VGS/NAG2 requires the compact actual-values shape. Other
 * families are rejected even if their namespace also contains 0x30.
 */
bool mblink_mercedes_transmission_decode_live_2130_for_family(
    MblinkMercedesTransmissionFamily family,
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesTransmissionLive2130 *decoded);

/**
 * Decode only the selected logical values from one family-qualified 21 30
 * payload. Unselected fields remain unavailable in the output and their bytes
 * are not interpreted. One or many selected fields still consume one wire
 * response.
 */
bool mblink_mercedes_transmission_decode_live_2130_mask_for_family(
    MblinkMercedesTransmissionFamily family,
    const uint8_t *data,
    size_t data_length,
    uint64_t field_mask,
    MblinkMercedesTransmissionLive2130 *decoded);

bool mblink_mercedes_transmission_decode_egs51_gs218(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs51Gs218 *decoded);

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

bool mblink_mercedes_transmission_decode_kwp_rli30(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli30 *decoded);
bool mblink_mercedes_transmission_decode_kwp_rli31(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli31 *decoded);
bool mblink_mercedes_transmission_decode_kwp_rli32(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli32 *decoded);
bool mblink_mercedes_transmission_decode_kwp_rli33(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesKwpRli33 *decoded);

bool mblink_mercedes_transmission_decode_egs53_tcm_a1(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53TcmA1 *decoded);
bool mblink_mercedes_transmission_decode_egs53_tcm_a2(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53TcmA2 *decoded);
bool mblink_mercedes_transmission_decode_egs53_eng_rq1(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53EngRq1 *decoded);
bool mblink_mercedes_transmission_decode_egs53_eng_rq2(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53EngRq2 *decoded);
bool mblink_mercedes_transmission_decode_egs53_eng_rq3(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53EngRq3 *decoded);
bool mblink_mercedes_transmission_decode_egs53_sbw_rs_tcm(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53SbwRsTcm *decoded);
bool mblink_mercedes_transmission_decode_egs53_tcm_display_request(
    const uint8_t *payload,
    size_t payload_length,
    MblinkMercedesEgs53TcmDisplayRequest *decoded);

/**
 * Decode and format one passive Mercedes transmission CAN frame using the
 * explicitly selected generation. This is the transport-neutral entry point
 * for OpenPort/J2534, SocketCAN, STM32 and future raw-CAN capture paths.
 */
bool mblink_mercedes_transmission_format_can_frame(
    MblinkMercedesTransmissionFamily family,
    uint32_t can_id,
    const uint8_t *payload,
    size_t payload_length,
    char *buffer,
    size_t buffer_size);

/**
 * Classify a transmission controller family from an ECU identity string.
 * Unknown or ambiguous identities remain UNKNOWN; callers must not infer a
 * family solely from the diagnostic CAN address.
 */
MblinkMercedesTransmissionFamily
mblink_mercedes_transmission_family_from_identity(const char *identity);

/**
 * Resolve the exact MBLINK controller-family key selected by ECU identity.
 * This is deliberately separate from free-text identity parsing so an exact
 * catalogue match (for example transmission-egs53) cannot be lost when the
 * human-readable KWP identity string is numeric.
 */
MblinkMercedesTransmissionFamily
mblink_mercedes_transmission_family_from_controller_family_key(
    const char *controller_family_key);

/**
 * Controller-family-scoped KWP2000 ReadDataByLocalIdentifier profile.
 *
 * Numeric local identifiers are not globally meaningful across Mercedes
 * transmission controllers. For example, OEM EGS52 RLI 0x30 is an actual-
 * values record while Ultimate NAG52 defines 0x30 as clutch speeds. Always
 * select a family first, then query only that family's profile.
 */
size_t mblink_mercedes_transmission_kwp_read_identifier_count_for_family(
    MblinkMercedesTransmissionFamily family);
uint8_t mblink_mercedes_transmission_kwp_read_identifier_at_for_family(
    MblinkMercedesTransmissionFamily family, size_t index);
const char *mblink_mercedes_transmission_kwp_read_identifier_name_for_family(
    MblinkMercedesTransmissionFamily family, uint8_t id);
bool mblink_mercedes_transmission_kwp_identifier_is_live_for_family(
    MblinkMercedesTransmissionFamily family, uint8_t id);

/**
 * Legacy EGS52 compatibility view. New code must use the family-scoped APIs
 * above rather than applying this list to an arbitrary transmission ECU.
 */
size_t mblink_mercedes_transmission_kwp_read_identifier_count(void);
uint8_t mblink_mercedes_transmission_kwp_read_identifier_at(size_t index);
const char *mblink_mercedes_transmission_kwp_read_identifier_name(uint8_t id);

const char *mblink_mercedes_transmission_family_name(
    MblinkMercedesTransmissionFamily family);
const char *mblink_mercedes_transmission_actual_gear_name(uint8_t code);
const char *mblink_mercedes_transmission_target_gear_name(uint8_t code);
const char *mblink_mercedes_transmission_selector_name(uint8_t code);

/* Vehicle-corroborated Siemens EGS53 KWP RLI 0x30 semantics. */
const char *mblink_mercedes_transmission_egs53_rli30_tcc_state_name(
    uint8_t code);
const char *mblink_mercedes_transmission_egs53_rli30_selector_name(
    uint8_t code);
const char *mblink_mercedes_transmission_egs53_rli30_program_name(
    uint8_t code);
const char *mblink_mercedes_transmission_egs53_rli30_recognised_gear_name(
    uint8_t code);

/* Family-local passive-CAN derived read semantics. */
const char *mblink_mercedes_transmission_egs52_tcc_request_name(
    bool slipping_request, bool open_request);
bool mblink_mercedes_transmission_egs52_esp_torque_intervention_active(
    bool minimum_request, bool maximum_request);
bool mblink_mercedes_transmission_egs52_cruise_torque_intervention_active(
    bool minimum_request, bool maximum_request);
const char *mblink_mercedes_transmission_egs53_tcc_request_name(
    uint8_t request_code);

const char *mblink_mercedes_transmission_tcc_name(
    MblinkMercedesTorqueConverterState state);

#ifdef __cplusplus
}
#endif
#endif
