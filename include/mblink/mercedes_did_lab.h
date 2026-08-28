// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_did_lab.h
 * @brief Offline Mercedes/Delphi DID research and signal-correlation contracts.
 *
 * Source-backed mappings may be decoded and compared offline before they are
 * allowed into normal live polling. Only vehicle-verified definitions are
 * eligible for automatic transmission.
 */
#ifndef MBLINK_MERCEDES_DID_LAB_H
#define MBLINK_MERCEDES_DID_LAB_H

#include "mblink/mercedes.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_DID_LAB_CONCEPT_ONLY = 0,
    MBLINK_MERCEDES_DID_LAB_SOURCE_BACKED_CANDIDATE,
    MBLINK_MERCEDES_DID_LAB_VEHICLE_VERIFIED
} MblinkMercedesDidLabStatus;

typedef enum {
    MBLINK_MERCEDES_DID_LAB_UNSIGNED_BIG_ENDIAN = 0,
    MBLINK_MERCEDES_DID_LAB_SIGNED_BIG_ENDIAN,
    MBLINK_MERCEDES_DID_LAB_UNSIGNED_LITTLE_ENDIAN,
    MBLINK_MERCEDES_DID_LAB_SIGNED_LITTLE_ENDIAN
} MblinkMercedesDidLabEncoding;

typedef struct {
    const char *stable_key;
    const char *name;
    const char *ecu_family;
    MblinkMercedesModuleKind module;
    MblinkMercedesDidLabStatus status;
    bool identifier_known;
    uint16_t identifier;
    size_t raw_length;
    MblinkMercedesDidLabEncoding encoding;
    double factor;
    double offset;
    const char *unit;
    const char *correlation_reference_key;
    const char *provenance;
    const char *source_locator;
} MblinkMercedesDidLabDefinition;

typedef enum {
    MBLINK_MERCEDES_DID_LAB_DECODE_OK = 0,
    MBLINK_MERCEDES_DID_LAB_DECODE_INVALID_ARGUMENT,
    MBLINK_MERCEDES_DID_LAB_DECODE_UNMAPPED,
    MBLINK_MERCEDES_DID_LAB_DECODE_MALFORMED,
    MBLINK_MERCEDES_DID_LAB_DECODE_UNEXPECTED_RESPONSE
} MblinkMercedesDidLabDecodeResult;

typedef struct {
    uint64_t timestamp_ms;
    double value;
} MblinkSignalPoint;

typedef struct {
    size_t pair_count;
    int64_t lag_ms;
    double pearson_r;
    double slope;
    double intercept;
    double rmse;
    double normalized_rmse;
    double score;
} MblinkSignalCorrelationResult;

const char *mblink_mercedes_did_lab_status_name(
    MblinkMercedesDidLabStatus status);
const char *mblink_mercedes_did_lab_decode_result_name(
    MblinkMercedesDidLabDecodeResult result);
size_t mblink_mercedes_did_lab_count(void);
const MblinkMercedesDidLabDefinition *mblink_mercedes_did_lab_at(size_t index);
const MblinkMercedesDidLabDefinition *mblink_mercedes_did_lab_find_key(
    const char *stable_key);
const MblinkMercedesDidLabDefinition *mblink_mercedes_did_lab_find_identifier(
    uint16_t identifier);
bool mblink_mercedes_did_lab_can_auto_poll(
    const MblinkMercedesDidLabDefinition *definition);
MblinkMercedesDidLabDecodeResult mblink_mercedes_did_lab_decode_response(
    const MblinkMercedesDidLabDefinition *definition,
    const uint8_t *pdu,
    size_t pdu_length,
    double *value);

bool mblink_signal_correlation_best_linear(
    const MblinkSignalPoint *reference,
    size_t reference_count,
    const MblinkSignalPoint *candidate,
    size_t candidate_count,
    uint64_t max_lag_ms,
    uint64_t lag_step_ms,
    uint64_t pair_tolerance_ms,
    MblinkSignalCorrelationResult *result);
const char *mblink_signal_correlation_strength(
    const MblinkSignalCorrelationResult *result);

#ifdef __cplusplus
}
#endif
#endif
