// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_me_diaglogic.h
 * @brief Allocation-free decoder for the archived DiagLogic Values.proto ABI.
 *
 * The schema is an interoperability fact recovered from the official Mercedes
 * me Adapter 4.7.61 application. Decoding this protobuf does not imply that
 * LINK can decode the native ABI independently; active adapter use remains evidence-gated.
 */
#ifndef MBMBLINK_MERCEDES_ME_DIAGLOGIC_H
#define MBMBLINK_MERCEDES_ME_DIAGLOGIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MblinkMercedesMeDiaglogicResult {
    MBLINK_MERCEDES_ME_DIAGLOGIC_OK = 0,
    MBLINK_MERCEDES_ME_DIAGLOGIC_INVALID_ARGUMENT,
    MBLINK_MERCEDES_ME_DIAGLOGIC_TRUNCATED,
    MBLINK_MERCEDES_ME_DIAGLOGIC_MALFORMED,
    MBLINK_MERCEDES_ME_DIAGLOGIC_REQUIRED_FIELD_MISSING
} MblinkMercedesMeDiaglogicResult;

typedef struct MblinkMercedesMeProtoSlice {
    const uint8_t *data;
    size_t size;
} MblinkMercedesMeProtoSlice;

typedef enum MblinkMercedesMeValueType {
    MBLINK_MERCEDES_ME_VALUE_UNKNOWN = 0,
    MBLINK_MERCEDES_ME_VALUE_BOOLEAN = 1,
    MBLINK_MERCEDES_ME_VALUE_DOUBLE = 2,
    MBLINK_MERCEDES_ME_VALUE_LONG = 3,
    MBLINK_MERCEDES_ME_VALUE_STRING = 4,
    MBLINK_MERCEDES_ME_VALUE_BYTEARRAY = 5
} MblinkMercedesMeValueType;

typedef enum MblinkMercedesMeDtcFlag {
    MBLINK_MERCEDES_ME_DTC_FLAG_UNKNOWN = 0,
    MBLINK_MERCEDES_ME_DTC_SPORADIC = 1,
    MBLINK_MERCEDES_ME_DTC_STATIC = 2
} MblinkMercedesMeDtcFlag;

typedef struct MblinkMercedesMeDiaglogicValue {
    int value_type;
    bool has_boolean_value;
    bool boolean_value;
    bool has_double_value;
    double double_value;
    bool has_long_value;
    int64_t long_value;
    MblinkMercedesMeProtoSlice string_value;
    MblinkMercedesMeProtoSlice bytes_value;
} MblinkMercedesMeDiaglogicValue;

typedef struct MblinkMercedesMeDiaglogicMeasuredItem {
    MblinkMercedesMeProtoSlice data_id;
    MblinkMercedesMeProtoSlice unit;
    bool has_value;
    MblinkMercedesMeDiaglogicValue value;
    bool has_responded_device_address;
    int64_t responded_device_address;
    bool has_timestamp;
    int64_t timestamp;
} MblinkMercedesMeDiaglogicMeasuredItem;

typedef struct MblinkMercedesMeDiaglogicDtc {
    MblinkMercedesMeProtoSlice trouble_code;
    int sporadic_flag;
    MblinkMercedesMeProtoSlice display_text;
    MblinkMercedesMeProtoSlice responded_device_id;
} MblinkMercedesMeDiaglogicDtc;

typedef struct MblinkMercedesMeDiaglogicVehicleConfiguration {
    MblinkMercedesMeProtoSlice version;
    MblinkMercedesMeProtoSlice variant;
    bool has_timestamp;
    int64_t timestamp;
} MblinkMercedesMeDiaglogicVehicleConfiguration;

typedef struct MblinkMercedesMeDiaglogicVehicleStatus {
    MblinkMercedesMeProtoSlice assigned_vin;
    MblinkMercedesMeProtoSlice error_code_as_string;
    MblinkMercedesMeProtoSlice error_message;
    bool has_vehicle_configuration;
    MblinkMercedesMeDiaglogicVehicleConfiguration vehicle_configuration;
    MblinkMercedesMeProtoSlice obd_adapter_sw_version;
    size_t dtc_collection_count;
    size_t measured_item_count;
} MblinkMercedesMeDiaglogicVehicleStatus;

typedef struct MblinkMercedesMeDiaglogicPreview {
    bool cycle_completed;
    MblinkMercedesMeProtoSlice pending_action_token;
    bool has_repeatable;
    bool repeatable;
} MblinkMercedesMeDiaglogicPreview;

/*
 * All slices returned below point directly into the caller-owned protobuf
 * buffer. The buffer must therefore remain alive while the slices are used.
 */
typedef void (*MblinkMercedesMeDiaglogicMeasuredItemFn)(
    void *context,
    const MblinkMercedesMeDiaglogicMeasuredItem *item);

typedef void (*MblinkMercedesMeDiaglogicMeasuredItemCollectionFn)(
    void *context,
    MblinkMercedesMeProtoSlice requested_device_id,
    const MblinkMercedesMeDiaglogicMeasuredItem *item);

typedef void (*MblinkMercedesMeDiaglogicDtcFn)(
    void *context,
    MblinkMercedesMeProtoSlice requested_device_id,
    const MblinkMercedesMeDiaglogicDtc *dtc);

typedef struct MblinkMercedesMeDiaglogicCallbacks {
    MblinkMercedesMeDiaglogicMeasuredItemFn measured_item;
    MblinkMercedesMeDiaglogicDtcFn dtc;
    void *context;
} MblinkMercedesMeDiaglogicCallbacks;

typedef struct MblinkMercedesMeDiaglogicReferencePolicy {
    unsigned int live_data_stream_read_timeout_ms;
    unsigned int live_data_availability_timeout_ms;
    unsigned int min_ignition_read_delay_ms;
    int max_ignition_read_speed;
    unsigned int min_mileage_read_delay_ms;
    unsigned int max_mileage_read_delay_ms;
    int max_mileage_read_speed;
    int min_fuel_read_distance;
    int min_negative_mileage_difference;
    unsigned int
        max_distance_since_codes_cleared_measured_mileage_time_difference_ms;
    unsigned int max_speed_age_for_ignition_ms;
    double ignition_off_voltage_threshold_min;
    double ignition_off_voltage_threshold_default;
    double ignition_off_voltage_threshold_max;
    double ignition_off_voltage_below_max_battery_margin;
    unsigned int allowed_live_status_age_ms;
    unsigned int allowed_run_cycle_status_age_ms;
    double invalid_trip_start_mileage;
} MblinkMercedesMeDiaglogicReferencePolicy;

/**
 * Exact scheduling/sanity defaults recovered from libdiaglogic.so.
 *
 * Fields whose native names explicitly encode milliseconds or volts carry
 * those units here. Speed/distance-like fields intentionally keep the native
 * semantic name without inventing a physical unit.
 */
const MblinkMercedesMeDiaglogicReferencePolicy *
mblink_mercedes_me_diaglogic_reference_policy(void);

const char *mblink_mercedes_me_diaglogic_result_name(
    MblinkMercedesMeDiaglogicResult result);

MblinkMercedesMeDiaglogicResult mblink_mercedes_me_diaglogic_decode_value(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicValue *value);

MblinkMercedesMeDiaglogicResult mblink_mercedes_me_diaglogic_decode_measured_item(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicMeasuredItem *item);

MblinkMercedesMeDiaglogicResult
mblink_mercedes_me_diaglogic_decode_measured_item_collection(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicMeasuredItemCollectionFn item_fn,
    void *context,
    size_t *item_count);

MblinkMercedesMeDiaglogicResult mblink_mercedes_me_diaglogic_decode_dtc(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicDtc *dtc);

MblinkMercedesMeDiaglogicResult
mblink_mercedes_me_diaglogic_decode_dtc_collection(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicDtcFn dtc_fn,
    void *context,
    size_t *dtc_count);

MblinkMercedesMeDiaglogicResult
mblink_mercedes_me_diaglogic_decode_vehicle_configuration(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicVehicleConfiguration *configuration);

MblinkMercedesMeDiaglogicResult mblink_mercedes_me_diaglogic_decode_vehicle_status(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicVehicleStatus *status,
    const MblinkMercedesMeDiaglogicCallbacks *callbacks);

MblinkMercedesMeDiaglogicResult mblink_mercedes_me_diaglogic_decode_preview(
    const uint8_t *bytes,
    size_t size,
    MblinkMercedesMeDiaglogicPreview *preview);

#ifdef __cplusplus
}
#endif
#endif
