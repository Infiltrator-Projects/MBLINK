// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Stable public entry point for identity-first Mercedes discovery.
 *
 * The implementation is kept separately so this public entry point can retain
 * source-level compatibility checks without duplicating the scanner itself.
 * The public implementation provides MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS
 * and retains the source-backed 0x7E1 -> 0x7E9 fallback label:
 * Transmission ECU / GS (7E1/7E9)
 */
#ifndef MBLINK_MERCEDES_MODULE_SCAN_ENTRY_H
#define MBLINK_MERCEDES_MODULE_SCAN_ENTRY_H

#include "mblink/mercedes_module_scan_identity_first.h"

/*
 * GS KWP local identifier 0x30 carries the selector/ATF live payload seen in
 * the C207 field capture. Keep it on a sub-second cadence while the automatic
 * runtime observer is active; other discovered body/infotainment ECUs remain
 * census-only unless explicitly opened by the user.
 */
#ifndef MBLINK_FAST_LIVE_GS_DID
#define MBLINK_FAST_LIVE_GS_DID 0x30U
#endif
#ifndef MBLINK_FAST_LIVE_GS_INTERVAL_SECONDS
#define MBLINK_FAST_LIVE_GS_INTERVAL_SECONDS 0.75
#endif

#endif
