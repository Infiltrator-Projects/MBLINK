// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_signal_catalog.h
 * @brief First-party Mercedes backend signal semantics for offline research.
 *
 * These entries come from Mercedes-Benz's public MBSDK CarKit model.  They are
 * semantic/backend attribute names, not asserted ECU addresses or UDS DIDs.
 */
#ifndef MBLINK_MERCEDES_SIGNAL_CATALOG_H
#define MBLINK_MERCEDES_SIGNAL_CATALOG_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_SIGNAL_BOOL = 0,
    MBLINK_MERCEDES_SIGNAL_INT,
    MBLINK_MERCEDES_SIGNAL_DOUBLE,
    MBLINK_MERCEDES_SIGNAL_STRING,
    MBLINK_MERCEDES_SIGNAL_COMPOSITE
} MblinkMercedesSignalValueType;

typedef struct {
    const char *stable_key;
    const char *backend_key;
    const char *name;
    MblinkMercedesSignalValueType value_type;
    const char *unit_family;
    const char *correlation_reference_key;
    const char *applicability;
    bool diagnostic_research_priority;
    const char *source_locator;
    const char *note;
} MblinkMercedesBackendSignalDefinition;

const char *mblink_mercedes_signal_value_type_name(
    MblinkMercedesSignalValueType value_type);
size_t mblink_mercedes_backend_signal_count(void);
const MblinkMercedesBackendSignalDefinition *
mblink_mercedes_backend_signal_at(size_t index);
const MblinkMercedesBackendSignalDefinition *
mblink_mercedes_backend_signal_find_key(const char *stable_key);
const MblinkMercedesBackendSignalDefinition *
mblink_mercedes_backend_signal_find_backend_key(const char *backend_key);

#ifdef __cplusplus
}
#endif
#endif
