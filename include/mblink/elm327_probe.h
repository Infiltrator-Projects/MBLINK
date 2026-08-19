// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327_probe.h
 * @brief Adapter and active-protocol capability probing for ELM327 devices.
 */
#ifndef MBLINK_ELM327_PROBE_H
#define MBLINK_ELM327_PROBE_H

#include "mblink/elm327.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ELM327_MAX_DEVICE_DESCRIPTION 96U
#define MBLINK_ELM327_MAX_PROTOCOL_DESCRIPTION 128U

typedef enum {
    MBLINK_ELM327_PROBE_DEVICE_DESCRIPTION = 0,
    MBLINK_ELM327_PROBE_PROTOCOL_DESCRIPTION,
    MBLINK_ELM327_PROBE_PROTOCOL_NUMBER,
    MBLINK_ELM327_PROBE_COMPLETE,
    MBLINK_ELM327_PROBE_FAILED
} MblinkElm327ProbeStage;

typedef struct {
    MblinkElm327ProbeStage stage;
    MblinkElm327Result failure;
    bool device_description_supported;
    bool protocol_was_automatic;
    uint8_t protocol_number;
    char device_description[MBLINK_ELM327_MAX_DEVICE_DESCRIPTION];
    char protocol_description[MBLINK_ELM327_MAX_PROTOCOL_DESCRIPTION];
} MblinkElm327ProbeState;

/** Begin the deterministic AT@1 -> ATDP -> ATDPN probe sequence. */
void mblink_elm327_probe_begin(MblinkElm327ProbeState *state);

/** Return the command required by the current probe stage. */
const char *mblink_elm327_probe_command(
    const MblinkElm327ProbeState *state);

/**
 * Accept the parsed response for the current probe stage.
 *
 * AT@1 is optional because compatible adapters may not implement it. ATDP and
 * ATDPN are required to establish the currently selected OBD protocol.
 */
MblinkElm327Result mblink_elm327_probe_accept(
    MblinkElm327ProbeState *state,
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
