// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_factory_data.h"

#include <string.h>

#define ARRAY_LENGTH(values) (sizeof(values) / sizeof((values)[0]))

/*
 * These option meanings were observed in an official Mercedes-Benz Vehicle
 * Specification response for Baumuster 207303. The table stores individual
 * option semantics only; it intentionally does not store a VIN or one
 * vehicle's complete option combination.
 *
 * module_key_hint is conservative. NULL means the option is useful factory
 * context but is not yet strong enough to prioritise one controller family.
 */
static const MblinkMercedesFactoryOptionDefinition factory_options[] = {
    {"423", "5-speed automatic transmission", "transmission-vgs", "207303",
     "Mercedes-Benz Vehicle Specification API, option 423"},
    {"580", "THERMATIC automatic climate control", "climate", "207303",
     "Mercedes-Benz Vehicle Specification API, option 580"},
    {"527", "COMAND APS", "audio-headunit", "207303",
     "Mercedes-Benz Vehicle Specification API, option 527"},
    {"275", "Driver-seat memory package", "seat-driver", "207303",
     "Mercedes-Benz Vehicle Specification API, option 275"},
    {"241", "Front-passenger electric seat with memory", "seat-passenger", "207303",
     "Mercedes-Benz Vehicle Specification API, option 241"},
    {"621", "Intelligent Light System for left-hand traffic", "headlamp-range", "207303",
     "Mercedes-Benz Vehicle Specification API, option 621"},
    {"608", "Adaptive High-Beam Assistant", "headlamp-range", "207303",
     "Mercedes-Benz Vehicle Specification API, option 608"},
    {"293", "Rear side airbags", "restraints-orc", "207303",
     "Mercedes-Benz Vehicle Specification API, option 293"},
    {"U18", "Automatic child-seat recognition", "restraints-orc", "207303",
     "Mercedes-Benz Vehicle Specification API, option U18"},
    {"919", "Enhanced climate cooling package", "climate", "207303",
     "Mercedes-Benz Vehicle Specification API, option 919"},
    {"230", "PARKTRONIC", NULL, "207303",
     "Mercedes-Benz Vehicle Specification API, option 230"},
    {"413", "Panoramic sliding roof", NULL, "207303",
     "Mercedes-Benz Vehicle Specification API, option 413"},
    {"477", "Tyre-pressure-loss warning", NULL, "207303",
     "Mercedes-Benz Vehicle Specification API, option 477"},
    {"474", "Diesel particulate filter", NULL, "207303",
     "Mercedes-Benz Vehicle Specification API, option 474"}
};

const char *mblink_mercedes_evidence_rank_name(
    MblinkMercedesEvidenceRank rank)
{
    switch (rank) {
    case MBLINK_MERCEDES_EVIDENCE_NONE: return "none";
    case MBLINK_MERCEDES_EVIDENCE_PLATFORM_ASSUMPTION:
        return "platform assumption";
    case MBLINK_MERCEDES_EVIDENCE_GENERIC_METADATA:
        return "generic metadata";
    case MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_SPEC:
        return "VIN factory specification";
    case MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_OPTION:
        return "VIN factory option";
    case MBLINK_MERCEDES_EVIDENCE_POSITIVE_DIAGNOSTIC:
        return "positive diagnostic evidence";
    case MBLINK_MERCEDES_EVIDENCE_ECU_IDENTITY:
        return "ECU identity";
    }
    return "unknown";
}

bool mblink_mercedes_evidence_should_replace(
    MblinkMercedesEvidenceRank current,
    MblinkMercedesEvidenceRank candidate)
{
    return candidate > current;
}

size_t mblink_mercedes_factory_option_count(void)
{
    return ARRAY_LENGTH(factory_options);
}

const MblinkMercedesFactoryOptionDefinition *
mblink_mercedes_factory_option_at(size_t index)
{
    return index < ARRAY_LENGTH(factory_options)
        ? &factory_options[index] : NULL;
}

const MblinkMercedesFactoryOptionDefinition *
mblink_mercedes_factory_option_for_code(const char *code)
{
    size_t index;
    if (code == NULL || code[0] == '\0') return NULL;
    for (index = 0U; index < ARRAY_LENGTH(factory_options); ++index) {
        if (strcmp(factory_options[index].code, code) == 0)
            return &factory_options[index];
    }
    return NULL;
}

bool mblink_mercedes_factory_option_prioritises_module(
    const char *option_code,
    const char *module_key)
{
    const MblinkMercedesFactoryOptionDefinition *definition =
        mblink_mercedes_factory_option_for_code(option_code);
    return definition != NULL &&
           definition->module_key_hint != NULL &&
           module_key != NULL &&
           strcmp(definition->module_key_hint, module_key) == 0;
}
