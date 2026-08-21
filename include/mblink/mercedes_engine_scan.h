// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_engine_scan.h
 * @brief Composite read-only Mercedes engine evidence scan.
 *
 * This state machine extends the C207/OM651 endpoint/identity probe with a
 * standard UDS ReadDTCInformation request while the physical ECU channel is
 * still selected. It remains read-only and does not enter an extended session,
 * perform security access, clear faults or execute routines.
 */
#ifndef MBLINK_MERCEDES_ENGINE_SCAN_H
#define MBLINK_MERCEDES_ENGINE_SCAN_H

#include "mblink/mercedes_probe.h"
#include "mblink/uds_dtc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE = 0,
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

static inline bool mblink_mercedes_engine_scan_begin(
    MblinkMercedesEngineScan *scan,
    const MblinkMercedesEcuEndpointDefinition *endpoint)
{
    if (scan == NULL) {
        return false;
    }
    memset(scan, 0, sizeof(*scan));
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
    if (written != NULL) {
        *written = 0U;
    }
    if (buffer != NULL && buffer_size != 0U) {
        buffer[0] = '\0';
    }
    if (scan == NULL || buffer == NULL || written == NULL || buffer_size == 0U) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }

    if (scan->stage == MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE) {
        return mblink_mercedes_ecu_probe_command(
            &scan->probe, buffer, buffer_size, written);
    }

    if (scan->stage == MBLINK_MERCEDES_ENGINE_SCAN_STAGE_READ_DTCS) {
        uint8_t pdu[3];
        size_t pdu_length = 0U;
        MblinkUdsResult uds_result =
            mblink_uds_build_report_dtcs_by_status_mask_request(
                MBLINK_UDS_DTC_STATUS_MASK_ALL,
                pdu, sizeof(pdu), &pdu_length);
        MblinkElm327CanResult can_result;

        if (uds_result != MBLINK_UDS_RESULT_OK) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR;
        }
        can_result = mblink_elm327_can_build_pdu_command(
            pdu, pdu_length, buffer, buffer_size, written);
        if (can_result == MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL;
        }
        if (can_result != MBLINK_ELM327_CAN_RESULT_OK) {
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR;
        }
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
    }

    return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
}

static inline MblinkMercedesEcuProbeResult mblink_mercedes_engine_scan_accept(
    MblinkMercedesEngineScan *scan,
    const MblinkElm327Response *response)
{
    if (scan == NULL || response == NULL) {
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT;
    }

    if (scan->stage == MBLINK_MERCEDES_ENGINE_SCAN_STAGE_PROBE) {
        MblinkMercedesEcuProbeResult result =
            mblink_mercedes_ecu_probe_accept(&scan->probe, response);
        scan->probe_result = result;
        if (result == MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE) {
            scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_READ_DTCS;
            scan->probe_result = MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_OK;
        }
        if (result != MBLINK_MERCEDES_ECU_PROBE_RESULT_OK ||
            scan->probe.stage == MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED) {
            scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_FAILED;
        }
        return result;
    }

    if (scan->stage == MBLINK_MERCEDES_ENGINE_SCAN_STAGE_READ_DTCS) {
        uint8_t pdu[MBLINK_ELM327_MAX_RESPONSE / 2U];
        size_t pdu_length = 0U;
        MblinkElm327CanResult can_result = mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length);

        scan->dtc_elm_can_result = can_result;
        scan->dtc_elm_result = response->result;
        if (can_result != MBLINK_ELM327_CAN_RESULT_OK) {
            scan->dtc_result = response->result == MBLINK_ELM327_RESULT_NO_DATA
                ? MBLINK_MERCEDES_ENGINE_DTC_NO_RESPONSE
                : MBLINK_MERCEDES_ENGINE_DTC_INVALID_RESPONSE;
            scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE;
            return MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE;
        }

        scan->dtc_uds_result =
            mblink_uds_decode_report_dtcs_by_status_mask_response(
                pdu, pdu_length, &scan->dtcs);
        if (scan->dtc_uds_result == MBLINK_UDS_RESULT_OK) {
            scan->dtc_result = MBLINK_MERCEDES_ENGINE_DTC_AVAILABLE;
        } else if (scan->dtc_uds_result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
            MblinkUdsResponse decoded;
            if (mblink_uds_decode_response(
                    MBLINK_UDS_SERVICE_READ_DTC_INFORMATION,
                    pdu, pdu_length, &decoded) ==
                MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
                scan->dtc_negative_response_code = decoded.negative_response_code;
            }
            scan->dtc_result = MBLINK_MERCEDES_ENGINE_DTC_NEGATIVE_RESPONSE;
        } else {
            scan->dtc_result = MBLINK_MERCEDES_ENGINE_DTC_INVALID_RESPONSE;
        }
        scan->stage = MBLINK_MERCEDES_ENGINE_SCAN_STAGE_COMPLETE;
        return MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE;
    }

    return MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE;
}

#ifdef __cplusplus
}
#endif

#endif
