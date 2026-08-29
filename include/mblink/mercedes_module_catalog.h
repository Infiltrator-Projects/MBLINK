// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_catalog.h
 * @brief Source-corroborated W212/C207 control-unit family catalogue.
 *
 * This catalogue describes module families expected on the W212/S212/C207/A207
 * architecture. It does not assign diagnostic addresses: addresses are learned
 * from live read-only discovery. A returned ECU identity can promote an
 * otherwise anonymous responder to a source-corroborated module family.
 */
#ifndef MBLINK_MERCEDES_MODULE_CATALOG_H
#define MBLINK_MERCEDES_MODULE_CATALOG_H

#include "mblink/mercedes.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_MODULE_ALIAS_COUNT 4U

typedef enum MblinkMercedesModulePresence {
    MBLINK_MERCEDES_MODULE_PRESENCE_CORE = 0,
    MBLINK_MERCEDES_MODULE_PRESENCE_POWERTRAIN_VARIANT,
    MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT
} MblinkMercedesModulePresence;

typedef struct MblinkMercedesModuleDefinition {
    const char *key;
    const char *display_name;
    const char *component_designation;
    const char *network;
    MblinkMercedesModuleKind kind;
    MblinkMercedesModulePresence presence;
    const char *identity_aliases[MBLINK_MERCEDES_MODULE_ALIAS_COUNT];
    MblinkMercedesDefinitionStatus status;
    const char *provenance;
} MblinkMercedesModuleDefinition;

static inline const char *mblink_mercedes_module_presence_name(
    MblinkMercedesModulePresence presence)
{
    switch (presence) {
    case MBLINK_MERCEDES_MODULE_PRESENCE_CORE: return "core";
    case MBLINK_MERCEDES_MODULE_PRESENCE_POWERTRAIN_VARIANT:
        return "powertrain-variant";
    case MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT:
        return "optional-equipment";
    }
    return "unknown";
}

static inline const MblinkMercedesModuleDefinition *
mblink_mercedes_c207_module_definition_at(size_t index)
{
    /*
     * Mercedes W212 introduction/service material establishes the gateway and
     * CAN topology. W212/C207 service information independently identifies the
     * control-unit designations below. Identity aliases are diagnostic names
     * used by Mercedes tooling/parts literature; they are only classifiers for
     * returned text, never assumed addresses.
     */
    static const MblinkMercedesModuleDefinition definitions[] = {
        {
            "engine-cdi", "CDI diesel engine control unit", "N3/9",
            "drivetrain CAN", MBLINK_MERCEDES_MODULE_ENGINE,
            MBLINK_MERCEDES_MODULE_PRESENCE_POWERTRAIN_VARIANT,
            { "CRD3", "CDID", "CDI", "EDC17" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies N3/9 as the CDI control unit; public CRD3/CDID material corroborates the diagnostic family names."
        },
        {
            "engine-me", "ME-SFI petrol engine control unit", "N3/10",
            "drivetrain CAN", MBLINK_MERCEDES_MODULE_ENGINE,
            MBLINK_MERCEDES_MODULE_PRESENCE_POWERTRAIN_VARIANT,
            { "ME-SFI", "MED17", "SIM271", "ME9" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes W212 introduction/service material identifies N3/10 as the gasoline ME-SFI control unit."
        },
        {
            "transmission-vgs", "Transmission control unit (VGS / EGS)",
            "Y3/8n4 / N15/3", "drivetrain CAN",
            MBLINK_MERCEDES_MODULE_TRANSMISSION,
            MBLINK_MERCEDES_MODULE_PRESENCE_POWERTRAIN_VARIANT,
            { "VGS", "EGS", "VGSNAG", "ETC" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service material identifies the fully integrated VGS and EGS/ETC transmission-control families."
        },
        {
            "selector", "Electronic selector / DIRECT SELECT control unit",
            "A80 / N15/5", "drivetrain CAN",
            MBLINK_MERCEDES_MODULE_TRANSMISSION,
            MBLINK_MERCEDES_MODULE_PRESENCE_POWERTRAIN_VARIANT,
            { "ISM", "EWM", "ESM", "DIRECT SELECT" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212 service topology identifies the electronic selector/DIRECT SELECT controller on the drivetrain network."
        },
        {
            "esp", "Electronic Stability Program (ESP) control unit",
            "N47-5 / N30/4 / N30/7", "chassis / drivetrain CAN",
            MBLINK_MERCEDES_MODULE_ABS_ESP,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "ESP212", "ESP", "ABS", "BAS" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes W212/C207 service and introduction material identifies the ESP control unit and its base/premium variants."
        },
        {
            "restraints-orc", "Occupant restraint / airbag control unit (ORC)",
            "N2/7", "vehicle CAN",
            MBLINK_MERCEDES_MODULE_RESTRAINTS,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "ORC", "SRS", "AIRBAG", "RESTRAINT" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies N2/7 restraints; genuine 207/212 Mercedes parts identify the controller family as ORC."
        },
        {
            "instrument-cluster", "Instrument cluster", "A1",
            "interior / drivetrain CAN",
            MBLINK_MERCEDES_MODULE_INSTRUMENT_CLUSTER,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "IC_204", "IC204", "IC_212", "KOMBI" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes W212 service topology identifies A1 as the instrument cluster; IC_204/IC_212 are established diagnostic family names."
        },
        {
            "audio-headunit", "Audio 20 / COMAND head unit", "A2",
            "interior / telematics CAN", MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "HU_204", "HU204", "AUDIO 20", "COMAND" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 diagnostic coverage and Mercedes service reports identify the A2 head unit as HU_204/Audio 20/COMAND."
        },
        {
            "audio-controller", "Audio / COMAND operating unit", "A40/9",
            "interior / telematics CAN", MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "CTRLC_204", "CTRLC204", "CTRLC", "AUDIO/COMAND" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 diagnostic coverage identifies the Audio/COMAND operating unit as CTRLC_204."
        },
        {
            "central-display", "Audio / COMAND central display", "A40/8",
            "interior / telematics CAN", MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "DISPC_204", "DISPC204", "DSPC_204", "ZAN" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 diagnostic coverage identifies the central information display as the 204-family DISP/DSPC/ZAN unit."
        },
        {
            "belt-pretensioner-left", "PRE-SAFE reversible belt tensioner · front left",
            "A76", "chassis / restraints CAN",
            MBLINK_MERCEDES_MODULE_RESTRAINTS,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "RBTMFL_204", "RBTMFL", "PRETENSION FRONT LEFT", "BELTPRETENSIONER LEFT" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 diagnostic coverage and Mercedes PRE-SAFE service information identify the left reversible emergency tensioner family as RBTMFL_204."
        },
        {
            "belt-pretensioner-right", "PRE-SAFE reversible belt tensioner · front right",
            "A76/1", "chassis / restraints CAN",
            MBLINK_MERCEDES_MODULE_RESTRAINTS,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "RBTMFR_204", "RBTMFR", "PRETENSION FRONT RIGHT", "BELTPRETENSIONER RIGHT" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 diagnostic coverage and Mercedes PRE-SAFE service information identify the right reversible emergency tensioner family as RBTMFR_204."
        },
        {
            "seat-driver", "Driver seat adjustment control unit", "driver seat module",
            "interior CAN", MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "SEATD_212", "SEATD", "SSGF", "SEAT DRIVER" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212 diagnostic coverage identifies SEATD/SSGF as the front-left driver seat adjustment controller."
        },
        {
            "seat-passenger", "Front passenger seat adjustment control unit",
            "passenger seat module", "interior CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "SEATP_204", "SEATP", "SSGB", "SEAT PASSENGER" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "204/207/212-family diagnostic coverage identifies SEATP/SSGB as the front-right passenger seat adjustment controller."
        },
        {
            "central-gateway", "Central gateway (CGW)", "N93",
            "diagnostic CAN gateway",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "CGW", "ZGW", "CENTRAL GATEWAY", "GATEWAY" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes W212 introduction material documents N93 central-gateway functionality integrated with the front SAM housing and bridging diagnostic CAN to vehicle networks."
        },
        {
            "front-sam", "Front SAM control unit", "N10/1",
            "multi-CAN gateway / body",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "SAMF", "SAM-F", "FRONT SAM", "SAM_FRONT" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes models 204/207/212 service information identifies N10/1 as the front SAM; W212 introduction material documents its gateway housing."
        },
        {
            "rear-sam", "Rear SAM control unit", "N10/2",
            "interior CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "SAMR", "SAM-R", "REAR SAM", "SAM_REAR" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes 204/207/212 service information identifies N10/2 as the rear SAM."
        },
        {
            "eis-ezs", "Electronic ignition switch (EIS / EZS)", "N73",
            "vehicle CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "EZS", "EIS", "EZS_212", "EIS_212" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies N73 as the EIS/EZS control unit."
        },
        {
            "steering-column", "Steering column control unit", "N80",
            "vehicle CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "MRM", "SCM", "STEERING COLUMN", "STW" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies N80 as the steering-column module."
        },
        {
            "climate", "Automatic climate control (KLA / AAC)", "N22/7",
            "interior CAN",
            MBLINK_MERCEDES_MODULE_CLIMATE,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "KLA", "AAC", "KLA_212", "AAC_212" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 wiring/service information identifies the automatic climate control unit; Mercedes parts literature uses KLA."
        },
        {
            "airmatic-ads", "AIRmatic / adaptive damping control unit",
            "N51 / N51/3 / N51/5", "chassis CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "AIRMATIC", "ADS_", "ADS212", "DAMPING" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies AIRmatic/ADS control units when the corresponding suspension option is installed."
        },
        {
            "distronic", "DISTRONIC / distance-regulation control unit",
            "A89 / N63/1", "chassis / drivetrain CAN",
            MBLINK_MERCEDES_MODULE_OTHER,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "DISTRONIC", "DTR", "DISTRONIC PLUS", "A89" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies DTR/DISTRONIC control units on appropriately equipped vehicles."
        },
        {
            "headlamp-range", "Headlamp range control unit", "N71",
            "vehicle CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_OPTIONAL_EQUIPMENT,
            { "HRA", "LWR", "HEADLAMP RANGE", "HLC" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "W212/C207 service topology identifies N71 as the headlamp-range adjustment controller where fitted."
        }
    };

    return index < sizeof(definitions) / sizeof(definitions[0])
        ? &definitions[index] : NULL;
}

static inline size_t mblink_mercedes_c207_module_definition_count(void)
{
    size_t count = 0U;
    while (mblink_mercedes_c207_module_definition_at(count) != NULL) ++count;
    return count;
}

static inline bool mblink_mercedes_module_ascii_contains_case_insensitive(
    const char *text,
    const char *needle)
{
    size_t text_length;
    size_t needle_length;
    size_t start;
    size_t offset;

    if (text == NULL || needle == NULL || needle[0] == '\0') return false;
    text_length = strlen(text);
    needle_length = strlen(needle);
    if (needle_length > text_length) return false;

    for (start = 0U; start + needle_length <= text_length; ++start) {
        for (offset = 0U; offset < needle_length; ++offset) {
            if (toupper((unsigned char)text[start + offset]) !=
                toupper((unsigned char)needle[offset])) {
                break;
            }
        }
        if (offset == needle_length) return true;
    }
    return false;
}

static inline const MblinkMercedesModuleDefinition *
mblink_mercedes_c207_module_definition_for_identity(const char *identity)
{
    size_t index;

    if (identity == NULL || identity[0] == '\0') return NULL;
    for (index = 0U; ; ++index) {
        const MblinkMercedesModuleDefinition *definition =
            mblink_mercedes_c207_module_definition_at(index);
        size_t alias_index;
        if (definition == NULL) break;
        for (alias_index = 0U;
             alias_index < MBLINK_MERCEDES_MODULE_ALIAS_COUNT;
             ++alias_index) {
            const char *alias = definition->identity_aliases[alias_index];
            if (alias != NULL &&
                mblink_mercedes_module_ascii_contains_case_insensitive(
                    identity, alias)) {
                return definition;
            }
        }
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
