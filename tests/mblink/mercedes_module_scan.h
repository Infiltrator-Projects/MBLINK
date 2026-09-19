// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Core-state-machine regression tests intentionally exercise the unchanged
 * transport/census machinery below the public identity-first orchestration.
 * Public scan ordering is covered by test_mercedes_transmission.c.
 */
#include "../../include/mblink/mercedes_module_scan_core.h"

/* Exercise the transport/census core rather than the identity-first wrapper. */
#define mblink_mercedes_module_scan_begin mblink_mercedes_module_scan_begin_core
#define mblink_mercedes_module_scan_begin_gateway mblink_mercedes_module_scan_begin_gateway_core
#define mblink_mercedes_module_scan_begin_mobile_census mblink_mercedes_module_scan_begin_mobile_census_core
#define mblink_mercedes_module_scan_begin_full mblink_mercedes_module_scan_begin_full_core
#define mblink_mercedes_module_scan_command mblink_mercedes_module_scan_command_core
#define mblink_mercedes_module_scan_accept mblink_mercedes_module_scan_accept_core
