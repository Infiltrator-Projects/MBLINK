// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_om651.h
 * @brief Evidence-backed OM651/CDID3 diagnostic capability catalogue.
 *
 * This catalogue names manufacturer-level values that independent Mercedes
 * diagnostic material shows on OM651/CDID3 systems. It deliberately does not
 * assign UDS DIDs, payload offsets, scaling or units until those mappings have
 * defensible provenance. The catalogue therefore lets UI, fixtures and future
 * decoders agree on stable identities without turning scanner labels into
 * guessed protocol definitions.
 */
#ifndef MBLINK_MERCEDES_OM651_H
#define MBLINK_MERCEDES_OM651_H

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_MERCEDES_OM651_CATEGORY_DPF = 0,
    MBLINK_MERCEDES_OM651_CATEGORY_EXHAUST,
    MBLINK_MERCEDES_OM651_CATEGORY_FUEL,
    MBLINK_MERCEDES_OM651_CATEGORY_AIR,
    MBLINK_MERCEDES_OM651_CATEGORY_EGR,
    MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR
} MblinkMercedesOm651SignalCategory;

typedef enum {
    /** Capability exists in independent OM651/CDID3 diagnostic material. */
    MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED = 0,
    /** DID/encoding candidate exists but has not been vehicle-verified. */
    MBLINK_MERCEDES_OM651_SIGNAL_MAPPING_CANDIDATE,
    /** Request, encoding and meaning are backed by a reproducible fixture. */
    MBLINK_MERCEDES_OM651_SIGNAL_VEHICLE_VERIFIED
} MblinkMercedesOm651SignalStatus;

typedef struct {
    const char *key;
    const char *name;
    MblinkMercedesOm651SignalCategory category;
    MblinkMercedesOm651SignalStatus status;
    const char *provenance;
} MblinkMercedesOm651SignalDefinition;

static inline const char *mblink_mercedes_om651_signal_category_name(
    MblinkMercedesOm651SignalCategory category)
{
    switch (category) {
    case MBLINK_MERCEDES_OM651_CATEGORY_DPF: return "dpf";
    case MBLINK_MERCEDES_OM651_CATEGORY_EXHAUST: return "exhaust";
    case MBLINK_MERCEDES_OM651_CATEGORY_FUEL: return "fuel";
    case MBLINK_MERCEDES_OM651_CATEGORY_AIR: return "air";
    case MBLINK_MERCEDES_OM651_CATEGORY_EGR: return "egr";
    case MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR: return "injector";
    }
    return "unknown";
}

static inline const char *mblink_mercedes_om651_signal_status_name(
    MblinkMercedesOm651SignalStatus status)
{
    switch (status) {
    case MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED:
        return "corroborated-unmapped";
    case MBLINK_MERCEDES_OM651_SIGNAL_MAPPING_CANDIDATE:
        return "mapping-candidate";
    case MBLINK_MERCEDES_OM651_SIGNAL_VEHICLE_VERIFIED:
        return "vehicle-verified";
    }
    return "unknown";
}

static inline const MblinkMercedesOm651SignalDefinition *
mblink_mercedes_om651_signal_definitions(size_t *count)
{
    static const char provenance[] =
        "Mercedes OM651/CDID3 diagnostic actual-value catalogues; protocol mapping pending";
    static const MblinkMercedesOm651SignalDefinition definitions[] = {
        {
            "mercedes.om651.dpf.differential_pressure",
            "DPF differential pressure",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.fill_percent",
            "DPF fill level",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.ash_mass",
            "DPF ash content",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.soot_mass",
            "DPF soot mass",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.soot_mass_simulated",
            "DPF simulated soot mass",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.soot_mass_model",
            "DPF model soot mass",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.regeneration_status",
            "DPF regeneration status",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.distance_since_regeneration",
            "Distance since DPF regeneration",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.dpf.distance_since_regeneration_corrected",
            "Corrected distance since DPF regeneration",
            MBLINK_MERCEDES_OM651_CATEGORY_DPF,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.exhaust.temperature_pre_turbo",
            "Exhaust temperature before turbocharger",
            MBLINK_MERCEDES_OM651_CATEGORY_EXHAUST,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.exhaust.temperature_pre_catalyst",
            "Exhaust temperature before catalyst",
            MBLINK_MERCEDES_OM651_CATEGORY_EXHAUST,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.exhaust.dpf_inlet_temperature",
            "DPF inlet temperature",
            MBLINK_MERCEDES_OM651_CATEGORY_EXHAUST,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.fuel.rail_pressure",
            "Fuel rail pressure",
            MBLINK_MERCEDES_OM651_CATEGORY_FUEL,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.air.boost_pressure",
            "Boost pressure",
            MBLINK_MERCEDES_OM651_CATEGORY_AIR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.egr.command_or_rate",
            "EGR command / rate",
            MBLINK_MERCEDES_OM651_CATEGORY_EGR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.smooth_running.cylinder1",
            "Smooth-running correction cylinder 1",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.smooth_running.cylinder2",
            "Smooth-running correction cylinder 2",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.smooth_running.cylinder3",
            "Smooth-running correction cylinder 3",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.smooth_running.cylinder4",
            "Smooth-running correction cylinder 4",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.correction.cylinder1",
            "Injector correction factor cylinder 1",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.correction.cylinder2",
            "Injector correction factor cylinder 2",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.correction.cylinder3",
            "Injector correction factor cylinder 3",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        },
        {
            "mercedes.om651.injector.correction.cylinder4",
            "Injector correction factor cylinder 4",
            MBLINK_MERCEDES_OM651_CATEGORY_INJECTOR,
            MBLINK_MERCEDES_OM651_SIGNAL_CORROBORATED_UNMAPPED,
            provenance
        }
    };

    if (count != NULL) {
        *count = sizeof(definitions) / sizeof(definitions[0]);
    }
    return definitions;
}

static inline size_t mblink_mercedes_om651_signal_count(void)
{
    size_t count = 0U;
    (void)mblink_mercedes_om651_signal_definitions(&count);
    return count;
}

static inline const MblinkMercedesOm651SignalDefinition *
mblink_mercedes_om651_signal_at(size_t index)
{
    size_t count = 0U;
    const MblinkMercedesOm651SignalDefinition *definitions =
        mblink_mercedes_om651_signal_definitions(&count);
    return index < count ? &definitions[index] : NULL;
}

static inline const MblinkMercedesOm651SignalDefinition *
mblink_mercedes_om651_find_signal(const char *key)
{
    size_t count = 0U;
    const MblinkMercedesOm651SignalDefinition *definitions;
    size_t index;

    if (key == NULL || key[0] == '\0') {
        return NULL;
    }

    definitions = mblink_mercedes_om651_signal_definitions(&count);
    for (index = 0U; index < count; ++index) {
        if (strcmp(definitions[index].key, key) == 0) {
            return &definitions[index];
        }
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
