// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file obd2.h
 * @brief Portable SAE OBD-II request, parser and decoder API.
 *
 * This layer consumes normalised ELM327 responses and owns standard OBD-II
 * request construction, supported-PID discovery, common live/freeze-frame PID
 * formulas, readiness decoding, VIN extraction and diagnostic trouble codes.
 * It is independent of BLE, CoreBluetooth and manufacturer-specific data.
 */
#ifndef MBLINK_OBD2_H
#define MBLINK_OBD2_H

#include "mblink/elm327.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_OBD2_VIN_LENGTH 17U
#define MBLINK_OBD2_DTC_TEXT_LENGTH 6U
#define MBLINK_OBD2_MAX_DTCS 64U
#define MBLINK_OBD2_PID_SET_BYTES 32U

typedef enum {
    MBLINK_OBD2_RESULT_OK = 0,
    MBLINK_OBD2_RESULT_INVALID_ARGUMENT,
    MBLINK_OBD2_RESULT_ELM_ERROR,
    MBLINK_OBD2_RESULT_MALFORMED_RESPONSE,
    MBLINK_OBD2_RESULT_UNEXPECTED_RESPONSE,
    MBLINK_OBD2_RESULT_UNSUPPORTED_PID,
    MBLINK_OBD2_RESULT_BUFFER_TOO_SMALL,
    MBLINK_OBD2_RESULT_TOO_MANY_DTCS,
    MBLINK_OBD2_RESULT_NOT_AUTHORIZED
} MblinkObd2Result;

typedef enum {
    MBLINK_OBD2_UNIT_NONE = 0,
    MBLINK_OBD2_UNIT_PERCENT,
    MBLINK_OBD2_UNIT_CELSIUS,
    MBLINK_OBD2_UNIT_KPA,
    MBLINK_OBD2_UNIT_RPM,
    MBLINK_OBD2_UNIT_KMH,
    MBLINK_OBD2_UNIT_GRAMS_PER_SECOND
} MblinkObd2Unit;

typedef struct {
    uint8_t pid;
    double value;
    MblinkObd2Unit unit;
} MblinkObd2Sample;

typedef struct {
    uint8_t bits[MBLINK_OBD2_PID_SET_BYTES];
} MblinkObd2PidSet;

typedef enum {
    MBLINK_OBD2_DTC_STORED = 0,
    MBLINK_OBD2_DTC_PENDING,
    MBLINK_OBD2_DTC_PERMANENT
} MblinkObd2DtcKind;

typedef struct {
    MblinkObd2DtcKind kind;
    char code[MBLINK_OBD2_DTC_TEXT_LENGTH];
} MblinkObd2Dtc;

typedef struct {
    MblinkObd2Dtc entries[MBLINK_OBD2_MAX_DTCS];
    size_t count;
} MblinkObd2DtcList;

typedef struct {
    bool mil_on;
    uint8_t confirmed_dtc_count;
    bool compression_ignition;
    uint8_t continuous_supported;
    uint8_t continuous_incomplete;
    uint8_t noncontinuous_supported;
    uint8_t noncontinuous_incomplete;
    uint8_t raw[4];
} MblinkObd2Readiness;

typedef struct {
    bool confirmed;
    bool acknowledge_readiness_reset;
} MblinkObd2ClearAuthorization;

#define MBLINK_OBD2_CLEAR_AUTHORIZATION_INIT \
    { .confirmed = false, .acknowledge_readiness_reset = false }

const char *mblink_obd2_result_name(MblinkObd2Result result);
const char *mblink_obd2_unit_name(MblinkObd2Unit unit);
const char *mblink_obd2_pid_name(uint8_t pid);

MblinkObd2Result mblink_obd2_build_live_pid_request(
    uint8_t pid, char *buffer, size_t buffer_size);

MblinkObd2Result mblink_obd2_build_freeze_pid_request(
    uint8_t pid, uint8_t frame_number, char *buffer, size_t buffer_size);

MblinkObd2Result mblink_obd2_build_supported_pid_request(
    uint8_t base_pid, char *buffer, size_t buffer_size);

MblinkObd2Result mblink_obd2_build_vin_request(
    char *buffer, size_t buffer_size);

MblinkObd2Result mblink_obd2_build_dtc_request(
    MblinkObd2DtcKind kind, char *buffer, size_t buffer_size);

MblinkObd2Result mblink_obd2_build_clear_dtc_request(
    const MblinkObd2ClearAuthorization *authorization,
    char *buffer, size_t buffer_size);

void mblink_obd2_pid_set_clear(MblinkObd2PidSet *set);
bool mblink_obd2_pid_set_contains(const MblinkObd2PidSet *set, uint8_t pid);

/** Union one supported-PID block transactionally; `set` changes only on OK. */
MblinkObd2Result mblink_obd2_accept_supported_pids(
    const MblinkElm327Response *response,
    uint8_t base_pid,
    MblinkObd2PidSet *set,
    bool *has_more);

MblinkObd2Result mblink_obd2_decode_live_pid(
    const MblinkElm327Response *response,
    uint8_t pid,
    MblinkObd2Sample *sample);

MblinkObd2Result mblink_obd2_decode_freeze_pid(
    const MblinkElm327Response *response,
    uint8_t pid,
    uint8_t frame_number,
    MblinkObd2Sample *sample);

MblinkObd2Result mblink_obd2_decode_readiness(
    const MblinkElm327Response *response,
    MblinkObd2Readiness *readiness);

MblinkObd2Result mblink_obd2_decode_vin(
    const MblinkElm327Response *response,
    char vin[MBLINK_OBD2_VIN_LENGTH + 1U]);

/** Decode a complete DTC response transactionally; `list` changes only on OK. */
MblinkObd2Result mblink_obd2_decode_dtcs(
    const MblinkElm327Response *response,
    MblinkObd2DtcKind kind,
    MblinkObd2DtcList *list);

MblinkObd2Result mblink_obd2_decode_dtc_pair(
    uint8_t high,
    uint8_t low,
    char code[MBLINK_OBD2_DTC_TEXT_LENGTH]);

#ifdef __cplusplus
}
#endif

#endif
