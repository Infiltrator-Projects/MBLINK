// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file uds.c
 * @brief iOS build bridge to LINK's shared ISO 14229 UDS implementation.
 */
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/src/uds/uds.c"
#else
typedef int mblink_uds_compat_translation_unit;
#endif
