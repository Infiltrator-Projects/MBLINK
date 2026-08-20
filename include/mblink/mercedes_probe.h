// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_probe.h
 * @brief Read-only Mercedes ECU endpoint probing over ELM-managed CAN.
 *
 * The probe coordinates existing ELM CAN and UDS contracts. It configures one
 * caller-selected physical endpoint, then sends a positive-response
 * TesterPresent request. Completion proves only that the endpoint answered UDS;
 * it does not verify a vehicle profile, ECU identity or manufacturer DID. The
 * request does not enter a session or write vehicle data, although a responding
 * ECU may refresh its diagnostic inactivity timer.
 */
#ifndef MBLINK_MERCEDES_PROBE_H
#define MBLINK_MERCEDES_PROBE_H

#include "mblink/elm327_can.h"
#include "mblink/mercedes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_ECU_PROBE_STAGE_CONFIGURE_CHANNEL = 0,
    MBLINK_MERCEDES_ECU_PROBE_STAGE_TESTER_PRESENT,
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

typedef struct {
    const MblinkMercedesEcuEndpointDefinition *endpoint;
    MblinkElm327CanChannelState channel;
    MblinkMercedesEcuProbeStage stage;
    MblinkMercedesEcuProbeResult failure;
    MblinkElm327CanResult elm_can_failure;
    MblinkElm327Result elm_failure;
    MblinkUdsResult uds_failure;
    uint8_t uds_negative_response_code;
} MblinkMercedesEcuProbe;

const char *mblink_mercedes_ecu_probe_result_name(
    MblinkMercedesEcuProbeResult result);

const char *mblink_mercedes_ecu_probe_stage_name(
    MblinkMercedesEcuProbeStage stage);

/**
 * Begin probing one borrowed endpoint definition.
 *
 * The endpoint and its text fields must remain valid for the probe lifetime.
 */
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_begin(
    MblinkMercedesEcuProbe *probe,
    const MblinkMercedesEcuEndpointDefinition *endpoint);

/** Build the current ELM AT command or the `3E00` UDS probe command. */
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_command(
    const MblinkMercedesEcuProbe *probe,
    char *buffer,
    size_t buffer_size,
    size_t *written);

/**
 * Accept one already-parsed ELM response for the current probe stage.
 *
 * Terminal failures preserve the failing ELM-CAN layer, the underlying ELM
 * result when applicable, and the UDS negative-response code when supplied.
 */
MblinkMercedesEcuProbeResult mblink_mercedes_ecu_probe_accept(
    MblinkMercedesEcuProbe *probe,
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
