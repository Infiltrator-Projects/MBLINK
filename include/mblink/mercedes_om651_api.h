// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_om651_api.h
 * @brief External ABI for the header-owned OM651 capability catalogue.
 *
 * The catalogue itself remains header-defined for portable consumers. These
 * wrappers provide stable external symbols for Clang/Swift importers and other
 * FFI users that should not depend on importing C static-inline functions.
 */
#ifndef MBLINK_MERCEDES_OM651_API_H
#define MBLINK_MERCEDES_OM651_API_H

#include "mblink/mercedes_om651.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t mblink_mercedes_om651_catalog_count(void);
const MblinkMercedesOm651SignalDefinition *
mblink_mercedes_om651_catalog_at(size_t index);

#ifdef __cplusplus
}
#endif

#endif
