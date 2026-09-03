// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Stable public entry point for identity-first Mercedes discovery.
 *
 * The implementation is kept separately so this public entry point can retain
 * source-level compatibility checks without duplicating the scanner itself.
 * The source-backed 0x7E1 -> 0x7E9 fallback label remains:
 * Transmission ECU / GS (7E1/7E9)
 */
#ifndef MBLINK_MERCEDES_MODULE_SCAN_ENTRY_H
#define MBLINK_MERCEDES_MODULE_SCAN_ENTRY_H

#include "mblink/mercedes_module_scan_identity_first.h"

#endif
