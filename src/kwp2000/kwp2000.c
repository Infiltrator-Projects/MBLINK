// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file kwp2000.c
 * @brief iOS build bridge to LINK's shared ISO 14230-3 KWP2000 implementation.
 */
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/src/kwp2000/kwp2000.c"
#else
typedef int mblink_kwp2000_compat_translation_unit;
#endif
