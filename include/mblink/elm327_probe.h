// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327_probe.h
 * @brief MBLINK compatibility facade for LINK's ELM327 capability probe.
 */
#ifndef MBLINK_ELM327_PROBE_H
#define MBLINK_ELM327_PROBE_H

#include "link/elm327_probe.h"
#include "mblink/elm327.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ELM327_MAX_DEVICE_DESCRIPTION LINK_ELM327_MAX_DEVICE_DESCRIPTION
#define MBLINK_ELM327_MAX_PROTOCOL_DESCRIPTION LINK_ELM327_MAX_PROTOCOL_DESCRIPTION

#define MBLINK_ELM327_PROBE_DEVICE_DESCRIPTION LINK_ELM327_PROBE_DEVICE_DESCRIPTION
#define MBLINK_ELM327_PROBE_PROTOCOL_DESCRIPTION LINK_ELM327_PROBE_PROTOCOL_DESCRIPTION
#define MBLINK_ELM327_PROBE_PROTOCOL_NUMBER LINK_ELM327_PROBE_PROTOCOL_NUMBER
#define MBLINK_ELM327_PROBE_COMPLETE LINK_ELM327_PROBE_COMPLETE
#define MBLINK_ELM327_PROBE_FAILED LINK_ELM327_PROBE_FAILED

typedef LinkElm327ProbeStage MblinkElm327ProbeStage;
typedef LinkElm327ProbeState MblinkElm327ProbeState;

void mblink_elm327_probe_begin(MblinkElm327ProbeState *state);
const char *mblink_elm327_probe_command(const MblinkElm327ProbeState *state);
MblinkElm327Result mblink_elm327_probe_accept(
    MblinkElm327ProbeState *state,
    const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
