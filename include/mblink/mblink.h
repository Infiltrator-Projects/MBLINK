// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.h
 * @brief Public portable C interface for the MBLINK diagnostics core.
 */
#ifndef MBLINK_MBLINK_H
#define MBLINK_MBLINK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Return the semantic version of the linked MBLINK core. */
const char *mblink_version(void);

/** Validate the core's shared project-metadata contract. */
bool mblink_self_check(void);

#ifdef __cplusplus
}
#endif

#endif
