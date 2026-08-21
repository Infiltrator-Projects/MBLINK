// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_engine_scan.h
 * @brief Composite read-only Mercedes engine evidence scan.
 *
 * The underlying Mercedes ECU probe now owns the complete read-only engine
 * sequence: endpoint setup, VIN/identity, CRD3 fingerprinting and UDS
 * ReadDTCInformation. This compatibility wrapper keeps the higher-level engine
 * scan API while exposing the same decoded CRD3 and DTC evidence without a
 * duplicate second DTC request.
 */
#ifndef MBLINK_MERCEDES_ENGINE_SCAN_H
#define MBLINK_MERCEDES_ENGINE_SCAN_H

#include "mblink/mercedes_crd3_evidence.h"
#include "mblink/mercedes_probe.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE = 0,
    /* Retained for source compatibility; new scans do not enter this stage. */
    MBLINK_MERCEDES_ENGINE_SCAN_STAGE_READ_DTCS,
    MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE,
    MBLINK_MERCEDES_ENGINE_SCAN_STAGE_FAILED
} MblinkMercedesEngineScanStage;

typedef enum {
    MBLINK_MERCEDES_ENGINE_DTC_NOT_ATTEMPTED = 0,
    MBLINK_MERCEDES_ENGINE_DTC_AVAILABLE,
    MBLINK_MERCEDES_ENGINE_DTC_NO_RESPONSE,
    MBLINK_MERCEDES_ENGINE_DTC_NEGATIVE_RESPONSE,
    MBLINK_MERCEDES_ENGINE_DTC_INVALID_RESPONSE
} MblinkMercedesEngineDtcResult;

typedef struct {
    MblinkMercedesEcuProbe probe;
    MblinkMercedesCrd3Evidence crd3_evidence;
    MblinkMercedesEngineScanStage stage;
    MblinkMercedesEcuProbeResult probe_result;
    MblinkMercedesEngineDtcResult dtc_result;
    MblinkElm327CanResult dtc_elm_can_result;
    MblinkElm327Result dtc_elm_result;
    MblinkUdsResult dtc_uds_result;
    uint8_t dtc_negative_response_code;
    MblinkUdsDtcList dtcs;
} MblinkMercedesEngineScan;

static inline const char *mblink_mercedes_engine_scan_stage_name(
    MblinkMercedesEngineScanStage stage)
{
    switch (stage) {
    case MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE: return "probe";
    case MBLINK_MERCEDES_ENGINE_SCAN_STAGE_READ_DTCS: return "read-dtcs";
    case MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_ENGINE_SCAN_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

static inline const char *mblink_mercedes_engine_dtc_result_name(
    MblinkMercedesEngineDtcResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ENGINE_DTC_NOT_ATTEMPTED: return "not-attempted";
    case MBLINK_MERCEDES_ENGINE_DTC_AVAILABLE: return "available";
    case MBLINK_MERCEDES_ENGINE_DTC_NO_RESPONSE: return "no-response";
    case MBLINK_MERCEDES_ENGINE_DTC_NEGATIVE_RESPONSE:
        return "negative-response";
    case MBLINK_MERCEDES_ENGINE_DTC_INVALID_RESPONSE: return "invalid-response";
    }
    return "unknown";
}

static inline MblinkMercedesEngineDtcResult
mblink_mercedes_engine_scan_map_dtc_result(MblinkMercedesEcuProbeDtcResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NOT_ATTEMPTED:
        return MBLINK_MERCEDES_ENGINE_DTC_NOT_ATTEMPTED;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE:
        return MBLINK_MERCEDES_ENGINE_DTC_AVAILABLE;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NO_RESPONSE:
        return MBLINK_MERCEDES_ENGINE_DTC_NO_RESPONSE;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_NEGATIVE_RESPONSE:
        return MBLINK_MERCEDES_ENGINE_DTC_NEGATIVE_RESPONSE;
    case MBLINK_MERCEDES_ECU_PROBE_DTC_INVALID_RESPONSE:
        return MBLINK_MERCEDES_ENGINE_DTC_INVALID_RESPONSE;
    }
    return MBLINK_MERCEDES_ENGINE_DTC_INVALID_RESPONSE;
}

static inline void mblink_mercedes_engine_scan_sync_evidence(
    MblinkMercedesEngineScan *scan)
{
    if (scan == NULL) {
        return;
    }

    mblink_mercedes_crd3_evidence_init(&scan->crd3_evidence);
    if (scan->probe.crd3_session_variant_available) {
        scan->crd3_evidence.session_variant = scan->probe.crd3_session_variant;
        scan->crd3_evidence.session_variant_available = true;
    }
    if (scan->probe.crd3_supplier_available) {
        scan->crd3_evidence.supplier = scan->probe.crd3_supplier;
        scan->crd3_evidence.supplier_available = true;
    }
    scan->crd3_evidence.om651_cdid3_delphi_signature =
        mblink_mercedes_ecu_probe_matches_om651_cdid3_delphi_signature(
            &scan->probe);

    scan->dtc_result =
        mblink_mercedes_engine_scan_map_dtc_result(scan->probe.dtc_result);
    scan->dtc_elm_can_result = scan->probe.dtc_elm_can_result;
    scan->dtc_elm_result = scan->probe.dtc_elm_result;
    scan->dtc_uds_result = scan->probe.dtc_uds_result;
    scan->dtc_negative_response_code = scan->probe.dtc_negative_response_code;
    scan->dtcs = scan->probe.dtcs;
}

static inline bool mblink_mercedes_engine_scan_begin(
    MblinkMercedesEngineScan *scan,
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    if (scan == NULL) {
        return false;
    }
    memset(scan, 0, sizeof(*scan));
    mblink_mercedes_crd3_evidence_init(&scan->crd3_evidence);
    scan->probe_result = mblink_mercedes_ecu_probe_begin(&scan->probe, endpoint);
    if (scan->probe_result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK) {
        scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_FAILED;
        return false;
    }
    scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE;
    scan->dtc_result = MBLINK_MERCEDES_ENGINE_DTC_NOT_ATTEMPTED;
    scan->dtc_elm_can_result = MBLINK_ELM327_CAN_RESULT_OK;
    scan->dtc_elm_result = MBLINK_ELM327_RESULT_OK;
    scan->dtc_uds_result = MBLINK_UDS_RESULT_OK;
    return true;
}

static inline MblinkMercedesEcuProbeResult mblink_mercedes_engine_scan_command(
    const MblinkMercedesEngineScan *scan,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    if (scan == NULL) {
        if (written != NULL) *written = 0U;
        if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    if (scan->stage != MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE) {
        if (written != NULL) *written = 0U;
        if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }
    return mblink_mercedes_ecu_probe_command(
        &scan->probe, buffer, buffer_size, written);
}

static inline MblinkMercedesEcuProbeResult mblink_mercedes_engine_scan_accept(
    MblinkMercedesEngineScan *scan,
    const MblinkElm327Response *response)
{
    MblinkMercedesEcuProbeResult result;

    if (scan == NULL || response == NULL) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }
    if (scan->stage != MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
    }

    result = mblink_mercedes_ecu_probe_accept(&scan->probe, response);
    scan->probe_result = result;
    if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
        mblink_mercedes_engine_scan_sync_evidence(scan);
        scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE;
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE;
    }
    if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
        scan->probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED) {
        mblink_mercedes_engine_scan_sync_evidence(scan);
        scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_FAILED;
    }
    return result;
}

#ifdef __cplusplus
}
#endif

#endif
