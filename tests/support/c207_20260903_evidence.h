// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file c207_20260903_evidence.h
 * @brief Sanitised real-vehicle evidence from the 2026-09-03 C207 capture.
 *
 * Source: MBLINK diagnostic-evidence CSV captured from a 2011 C207 E 250 CDI.
 * Capture software was MBLINK 0.7.153 / LINK 0.14.81.  The old application
 * version is recorded as provenance only: these constants describe what the
 * vehicle and ECUs actually returned and are replayed against current code.
 *
 * The vehicle VIN and other identifying strings are deliberately not retained
 * in this public regression fixture.
 */
#ifndef MBLINK_TEST_C207_20260903_EVIDENCE_H
#define MBLINK_TEST_C207_20260903_EVIDENCE_H

#include <stdint.h>

#define MBLINK_C207_20260903_MBLINK_VERSION "0.7.153"
#define MBLINK_C207_20260903_LINK_VERSION "0.14.81"

typedef struct MblinkC207RouteEvidence {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    const char *session_response;
    const char *tester_present_response;
    const char *dtc_response;
} MblinkC207RouteEvidence;

/*
 * Responders observed in the completed forensic pass.  0x632's DTC text is
 * intentionally preserved as the truncated ELM/indexed fragment that appeared
 * in the exported evidence; callers must not manufacture a complete DTC from
 * it.
 */
static const MblinkC207RouteEvidence mblink_c207_20260903_routes[] = {
    { UINT32_C(0x602), UINT32_C(0x480),
      "5003001400C8", "7E00", "59027BD1810050" },
    { UINT32_C(0x612), UINT32_C(0x482),
      "5003001400C8", "7E00", "590219" },
    { UINT32_C(0x632), UINT32_C(0x486),
      "7F1078\n5003001400C8", "7E00",
      "7F1978\n7F1978\n01B\n0:590239475C00" },
    { UINT32_C(0x64A), UINT32_C(0x489),
      "", "7E", "7F1878\n58019B51E0" },
    { UINT32_C(0x652), UINT32_C(0x48A),
      "", "7E", "7F1878\n5800" },
    { UINT32_C(0x7E0), UINT32_C(0x7E8),
      "", "7E00", "7F1978\n5902FF" },
    { UINT32_C(0x7E1), UINT32_C(0x7E9),
      "", "7F3E12", "7F1911" }
};

#define MBLINK_C207_20260903_ROUTE_COUNT \
    (sizeof(mblink_c207_20260903_routes) / \
     sizeof(mblink_c207_20260903_routes[0]))

/* Exact dual-responder Mode 01 replies from the same drive. */
#define MBLINK_C207_20260903_PID30_REPLY \
    "7E80341301F\n7E903413038"
#define MBLINK_C207_20260903_PID31_REPLY \
    "7E80441310672\n7E90441310AEC"

/* Exact 7E8 fuel-level samples spanning the observed high-tank range. */
#define MBLINK_C207_20260903_PID2F_96_REPLY "7E803412FF6"
#define MBLINK_C207_20260903_PID2F_98_REPLY "7E803412FFA"
#define MBLINK_C207_20260903_PID2F_100_REPLY "7E803412FFF"

/* Pedal/throttle examples that must remain distinct concepts. */
#define MBLINK_C207_20260903_PID11_REPLY "7E8034111E0"
#define MBLINK_C207_20260903_PID49_REPLY \
    "7E80341490E\n7E90341490E"
#define MBLINK_C207_20260903_PID4A_REPLY "7E803414A0F"

#endif
