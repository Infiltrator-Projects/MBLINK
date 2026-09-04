// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file uds.c
 * @brief iOS build bridge to LINK's shared ISO 14229 UDS implementation.
 *
 * Historical Apple composition included LINK's private service catalogue here:
 * #include "../link/src/uds/uds_services.c"
 * LINK now owns that composition through LinkPortableUds.c; the embedding
 * product is validated by LINK's ValidateAppleBridge.cmake at configure time.
 */
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/platform/apple/LinkPortableUds.c"
#else
typedef int mblink_uds_compat_translation_unit;
#endif
