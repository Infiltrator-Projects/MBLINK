// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file probe.c
 * @brief MBLINK ABI wrappers for LINK's ELM327 capability probe.
 */
#include "mblink/elm327_probe.h"

void mblink_elm327_probe_begin(MblinkElm327ProbeState *state)
{
    link_elm327_probe_begin(state);
}

const char *mblink_elm327_probe_command(const MblinkElm327ProbeState *state)
{
    return link_elm327_probe_command(state);
}

MblinkElm327Result mblink_elm327_probe_accept(
    MblinkElm327ProbeState *state,
    const MblinkElm327Response *response)
{
    return link_elm327_probe_accept(state, response);
}
