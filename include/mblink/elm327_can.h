// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327_can.h
 * @brief ELM327-managed ISO 15765 CAN diagnostic channel contracts.
 *
 * This path is for text-command adapters that perform ISO-TP formatting and
 * flow control internally. Raw CAN providers use mblink/isotp.h instead.
 * UDS remains above both paths and consumes complete PDUs.
 */
#ifndef MBLINK_ELM327_CAN_H
#define MBLINK_ELM327_CAN_H

#include "mblink/elm327.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum complete PDU that fits in one current ELM session command. */
#define MBLINK_ELM327_CAN_MAX_REQUEST_PDU 31U

/**
 * One physical diagnostic endpoint pair.
 *
 * `extended_id == false` uses 11-bit CAN identifiers. `true` uses 29-bit CAN
 * identifiers. MBLINK does not infer manufacturer addresses here; callers must
 * supply addresses from a validated discovery/profile source.
 */
typedef struct {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool extended_id;
} MblinkElm327CanChannelConfig;

typedef enum {
    MBLINK_ELM327_CAN_STAGE_SET_HEADER = 0,
    MBLINK_ELM327_CAN_STAGE_SET_RECEIVE_ADDRESS,
    MBLINK_ELM327_CAN_STAGE_ENABLE_AUTO_FORMATTING,
    MBLINK_ELM327_CAN_STAGE_ENABLE_FLOW_CONTROL,
    MBLINK_ELM327_CAN_STAGE_COMPLETE,
    MBLINK_ELM327_CAN_STAGE_FAILED
} MblinkElm327CanStage;

typedef enum {
    MBLINK_ELM327_CAN_RESULT_OK = 0,
    MBLINK_ELM327_CAN_RESULT_INVALID_ARGUMENT,
    MBLINK_ELM327_CAN_RESULT_BUFFER_TOO_SMALL,
    MBLINK_ELM327_CAN_RESULT_PDU_TOO_LARGE,
    MBLINK_ELM327_CAN_RESULT_ELM_ERROR,
    MBLINK_ELM327_CAN_RESULT_MALFORMED_RESPONSE,
    MBLINK_ELM327_CAN_RESULT_UNEXPECTED_RESPONSE,
    MBLINK_ELM327_CAN_RESULT_FAILED_STATE
} MblinkElm327CanResult;

typedef struct {
    MblinkElm327CanChannelConfig config;
    MblinkElm327CanStage stage;
    MblinkElm327CanResult failure;
    MblinkElm327Result elm_failure;
} MblinkElm327CanChannelState;

const char *mblink_elm327_can_result_name(MblinkElm327CanResult result);
const char *mblink_elm327_can_stage_name(MblinkElm327CanStage stage);

bool mblink_elm327_can_channel_config_is_valid(
    const MblinkElm327CanChannelConfig *config);

/** Initialise a configuration sequence from a validated caller-owned address. */
MblinkElm327CanResult mblink_elm327_can_channel_begin(
    MblinkElm327CanChannelState *state,
    const MblinkElm327CanChannelConfig *config);

/**
 * Build the AT command for the current configuration stage.
 *
 * Commands are `ATSH`, `ATCRA`, `ATCAF1`, then `ATCFC1`. The output buffer is
 * cleared on entry when possible. COMPLETE/FAILED states have no command.
 */
MblinkElm327CanResult mblink_elm327_can_channel_command(
    const MblinkElm327CanChannelState *state,
    char *buffer,
    size_t buffer_size);

/** Accept one parsed response; every configuration stage requires `OK`. */
MblinkElm327CanResult mblink_elm327_can_channel_accept(
    MblinkElm327CanChannelState *state,
    const MblinkElm327Response *response);

/**
 * Render one complete diagnostic PDU as the hex command consumed by an
 * ELM327-managed ISO 15765 channel. No PCI/ISO-TP bytes are added here.
 */
MblinkElm327CanResult mblink_elm327_can_build_pdu_command(
    const uint8_t *pdu,
    size_t pdu_length,
    char *buffer,
    size_t buffer_size,
    size_t *written);

/**
 * Decode one complete PDU from an ELM response with CAN auto-formatting on.
 *
 * Both single-line hex payloads and ELM indexed multi-line output (`0:`,
 * `1:`...) are accepted. Optional three-hex-digit indexed total length is
 * validated. Caller output is copied only on complete success.
 */
MblinkElm327CanResult mblink_elm327_can_decode_pdu(
    const MblinkElm327Response *response,
    uint8_t *pdu,
    size_t pdu_size,
    size_t *pdu_length);

#ifdef __cplusplus
}
#endif

#endif
