// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_probe.h
 * @brief Mercedes profile layer over LINK's shared read-only ECU probe.
 *
 * LINK owns channel setup, TesterPresent, bounded UDS DID acquisition, DTC
 * inventory, safety policy and raw result capture. MBLINK supplies the
 * Mercedes/CRD3 DID catalogue and interprets only Mercedes-specific evidence.
 */
#ifndef MBLINK_MERCEDES_PROBE_H
#define MBLINK_MERCEDES_PROBE_H

#include "link/ecu_probe.h"
#include "mblink/elm327_can.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_crd3.h"
#include "mblink/uds_dtc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_PROBE_VIN_LENGTH 17U
#define MBLINK_MERCEDES_PROBE_VIN_DID 0xF190U
#define MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT 6U
#define MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT 5U
#define MBLINK_MERCEDES_PROBE_EVIDENCE_DID_COUNT \
    (MBLINK_MERCEDES_PROBE_IDENTITY_DID_COUNT + \
     MBLINK_MERCEDES_PROBE_CRD3_DID_COUNT)

typedef enum {
    MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL = 0,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_IDENTITY,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_CRD3_FINGERPRINT,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_DTC_INFORMATION,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_COMPLETE,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_FAILED
} MblinkMercedesEcuProbeStage;

typedef enum {
    MBLINK_MERCEDES_ECU_PROBE_RESULT_OK = 0,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_COMPLETE,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_INVALID_ARGUMENT,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_UNSUPPORTED_ENDPOINT,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_BUFFER_TOO_SMALL,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_CHANNEL_ERROR,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_PDU_ERROR,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_UDS_ERROR,
    MBLINK_MERCEDES_ECU_PROBE_RESULT_FAILED_STATE
} MblinkMercedesEcuProbeResult;

typedef enum {
    MBLINK_MERCEDES_ECU_PROBE_VIN_NOT_ATTEMPTED = 0,
    MBLINK_MERCEDES_ECU_PROBE_VIN_AVAILABLE,
    MBLINK_MERCEDES_ECU_PROBE_VIN_NO_RESPONSE,
    MBLINK_MERCEDES_ECU_PROBE_VIN_NEGATIVE_RESPONSE,
    MBLINK_MERCEDES_ECU_PROBE_VIN_INVALID_RESPONSE
} MblinkMercedesEcuProbeVinResult;

typedef enum {
    MBLINK_MERCEDES_ECU_PROBE_DTC_NOT_ATTEMPTED = 0,
    MBLINK_MERCEDES_ECU_PROBE_DTC_AVAILABLE,
    MBLINK_MERCEDES_ECU_PROBE_DTC_NO_RESPONSE,
    MBLINK_MERCEDES_ECU_PROBE_DTC_NEGATIVE_RESPONSE,
    MBLINK_MERCEDES_ECU_PROBE_DTC_INVALID_RESPONSE
} MblinkMercedesEcuProbeDtcResult;

typedef struct {
    const MblinkMercedesEcuEndpointDefinition *endpoint;
    LinkEcuProbe shared;
    MblinkElm327CanChannelState channel;
    MblinkMercedesEcuProbeStage stage;
    MblinkMercedesEcuProbeResult failure;
    MblinkElm327CanResult elm_can_failure;
    MblinkElm327Result elm_failure;
    MblinkUdsResult uds_failure;
    uint8_t uds_negative_response_code;

    MblinkMercedesEcuProbeVinResult vin_result;
    MblinkElm327CanResult vin_elm_can_result;
    MblinkElm327Result vin_elm_result;
    MblinkUdsResult vin_uds_result;
    uint8_t vin_negative_response_code;
    char vin[MBLINK_MERCEDES_PROBE_VIN_LENGTH + 1U];

    size_t identity_index;
    uint32_t identity_positive_mask;
    uint32_t identity_negative_mask;
    uint32_t identity_no_response_mask;
    uint32_t identity_invalid_mask;

    size_t crd3_index;
    uint32_t crd3_positive_mask;
    uint32_t crd3_negative_mask;
    uint32_t crd3_no_response_mask;
    uint32_t crd3_invalid_mask;
    bool crd3_session_variant_available;
    MblinkMercedesCrd3SessionVariant crd3_session_variant;
    bool crd3_supplier_available;
    MblinkMercedesCrd3Supplier crd3_supplier;

    MblinkMercedesEcuProbeDtcResult dtc_result;
    MblinkElm327CanResult dtc_elm_can_result;
    MblinkElm327Result dtc_elm_result;
    MblinkUdsResult dtc_uds_result;
    uint8_t dtc_negative_response_code;
    MblinkUdsDtcList dtcs;
} MblinkMercedesEcuProbe;

static inline bool mblink_mercedes_ecu_probe_matches_om651_cdid3_delphi_signature(
    const MblinkMercedesEcuProbe *probe)
{
    return probe != NULL &&
           probe->crd3_session_variant_available &&
           probe->crd3_supplier_available &&
           mblink_mercedes_crd3_matches_om651_cdid3_delphi_signature(
               &probe->crd3_session_variant, &probe->crd3_supplier);
}

const char *mblink_mercedes_ecu_probe_result_name(
    MblinkMercedesEcuProbeResult result);
const char *mblink_mercedes_ecu_probe_stage_name(
    MblinkMercedesEcuProbeStage stage);
const char *mblink_mercedes_ecu_probe_vin_result_name(
    MblinkMercedesEcuProbeVinResult result);
const char *mblink_mercedes_ecu_probe_dtc_result_name(
    MblinkMercedesEcuProbeDtcResult result);

size_t mblink_mercedes_ecu_probe_identity_did_count(void);
uint16_t mblink_mercedes_ecu_probe_identity_did_at(size_t index);
const char *mblink_mercedes_ecu_probe_identity_did_name(size_t index);
size_t mblink_mercedes_ecu_probe_crd3_did_count(void);
uint16_t mblink_mercedes_ecu_probe_crd3_did_at(size_t index);
const char *mblink_mercedes_ecu_probe_crd3_did_name(size_t index);

MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_begin(
    MblinkMercedesEcuProbe *probe,
    const MblinkMercedesEcuEndpointDefinition *endpoint);
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_command(
    const MblinkMercedesEcuProbe *probe,
    char *buffer,
    size_t buffer_size,
    size_t *written);
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_accept(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
