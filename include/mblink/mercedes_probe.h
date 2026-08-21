// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_probe.h
 * @brief Read-only Mercedes ECU endpoint probing over ELM-managed CAN.
 *
 * The probe coordinates existing ELM CAN and UDS contracts. It configures one
 * caller-selected physical endpoint, sends a positive-response TesterPresent
 * request, then attempts the ISO 14229 standard VIN DID 0xF190. Completion
 * proves only that the endpoint answered UDS. A VIN response is evidence about
 * that endpoint, not permission to promote manufacturer-specific definitions.
 * The requests do not enter a session or write vehicle data, although a
 * responding ECU may refresh its diagnostic inactivity timer.
 */
#ifndef MBLINK_MERCEDES_PROBE_H
#define MBLINK_MERCEDES_PROBE_H

#include "mblink/elm327_can.h"
#include "mblink/mercedes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_PROBE_VIN_LENGTH 17U
#define MBLINK_MERCEDES_PROBE_VIN_DID 0xF190U

typedef enum {
    MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL = 0,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_READ_STANDARD_VIN,
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

typedef struct {
    const MblinkMercedesEcuEndpointDefinition *endpoint;
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
} MblinkMercedesEcuProbe;

const char *mblink_mercedes_ecu_probe_result_name(
    MblinkMercedesEcuProbeResult result);

const char *mblink_mercedes_ecu_probe_stage_name(
    MblinkMercedesEcuProbeStage stage);

const char *mblink_mercedes_ecu_probe_vin_result_name(
    MblinkMercedesEcuProbeVinResult result);

/**
 * Begin probing one borrowed endpoint definition.
 *
 * The endpoint and its text fields must remain valid for the probe lifetime.
 */
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_begin(
    MblinkMercedesEcuProbe *probe,
    const MblinkMercedesEcuEndpointDefinition *endpoint);

/**
 * Build the current ELM AT command, `3E00` TesterPresent command, or the
 * standardized `22F190` VIN request.
 */
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_command(
    const MblinkMercedesEcuProbe *probe,
    char *buffer,
    size_t buffer_size,
    size_t *written);

/**
 * Accept one already-parsed ELM response for the current probe stage.
 *
 * TesterPresent/channel failures remain terminal and preserve the failing
 * layer. The standardized VIN read is optional enrichment after a confirmed
 * positive TesterPresent: a negative, silent or malformed VIN response is
 * recorded in the VIN result fields and the endpoint probe still completes.
 */
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_accept(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
