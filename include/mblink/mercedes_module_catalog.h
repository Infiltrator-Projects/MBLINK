// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_catalog.h
 * @brief Source-corroborated Mercedes module and controller-family catalogue.
 *
 * The catalogue separates broad vehicle modules from the ECU/controller family
 * actually implementing them. Chassis/platform observations are evidence and
 * applicability metadata only; they never make a controller definition
 * universal. Diagnostic addresses are learned from live read-only discovery.
 * Returned ECU identity/software/hardware evidence can progressively refine an
 * anonymous responder from module -> controller family -> later exact variant.
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
mblink_mercedes_module_definition_at(size_t index)
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
            "W212/C207 diagnostic coverage identifies the A2 head unit as HU_204/Audio 20/COMAND; a public HU_204 HSCAN_KW2C3PE_500 trace proves tester 0x652 and response 0x48A."
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
            "central-gateway", "Central gateway (CGW)",
            "N93 / CGW processor in N10/1",
            "integrated diagnostic CAN gateway",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "CGW_212", "CGW", "ZGW", "CENTRAL GATEWAY" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes Series 207/212 documentation places a second, separately diagnosable central-gateway microprocessor inside the N10/1 front-SAM housing; it bridges diagnostic CAN to the vehicle subnetworks."
        },
        {
            "front-sam", "Front SAM control unit", "N10/1",
            "multi-CAN gateway / body",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "SAMF_212", "SAMF", "SAM-F", "FRONT SAM" },
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
            "Mercedes Series 207/212 service information identifies N10/1 as the front SAM; its SAM processor shares the housing with, but is separately diagnosable from, the integrated CGW processor."
        },
        {
            "rear-sam", "Rear SAM control unit", "N10/2",
            "interior CAN",
            MBLINK_MERCEDES_MODULE_BODY,
            MBLINK_MERCEDES_MODULE_PRESENCE_CORE,
            { "SAMR_212", "SAMR", "SAM-R", "REAR SAM" },
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
            { "HVAC_212", "KLA_212", "AAC_212", "KLA" },
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

static inline size_t mblink_mercedes_module_definition_count(void)
{
    size_t count = 0U;
    while (mblink_mercedes_module_definition_at(count) != NULL) ++count;
    return count;
}

static inline const MblinkMercedesModuleDefinition *
mblink_mercedes_module_definition_for_key(const char *key)
{
    size_t index;
    if (key == NULL || key[0] == '\0') return NULL;
    for (index = 0U; ; ++index) {
        const MblinkMercedesModuleDefinition *definition =
            mblink_mercedes_module_definition_at(index);
        if (definition == NULL) return NULL;
        if (strcmp(definition->key, key) == 0) return definition;
    }
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
mblink_mercedes_module_definition_for_identity(const char *identity)
{
    size_t index;

    if (identity == NULL || identity[0] == '\0') return NULL;
    for (index = 0U; ; ++index) {
        const MblinkMercedesModuleDefinition *definition =
            mblink_mercedes_module_definition_at(index);
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


#define MBLINK_MERCEDES_CONTROLLER_ALIAS_COUNT 4U

typedef struct MblinkMercedesControllerFamilyDefinition {
    const char *key;
    const char *module_key;
    const char *display_name;
    const char *identity_aliases[MBLINK_MERCEDES_CONTROLLER_ALIAS_COUNT];
    MblinkMercedesDefinitionStatus status;
    const char *applicability;
    const char *provenance;
} MblinkMercedesControllerFamilyDefinition;

/*
 * Controller families are narrower than module families. Entries with broad
 * aliases (for example CDI, VGS, ESP) are deliberately placed after the more
 * specific variants so exact family evidence wins first.
 *
 * Presence here does not mean MBLINK will poll any manufacturer data. The
 * data/service layer independently decides whether a source-backed identifier
 * set exists for the selected controller family.
 */
static inline const MblinkMercedesControllerFamilyDefinition *
mblink_mercedes_controller_family_definition_at(size_t index)
{
    static const MblinkMercedesControllerFamilyDefinition definitions[] = {
        { "engine-crd3", "engine-cdi", "CRD3 / CDID3 diesel ECU",
          { "CRD3", "CDID3", "CDID", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes CRD3/CDID3 applications; exact chassis/year varies",
          "Public Caesar/diagnostic material identifies the CRD3/CDID3 family." },
        { "engine-edc17", "engine-cdi", "EDC17 diesel ECU",
          { "EDC17", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes EDC17 applications; exact suffix/software determines services",
          "Mercedes diagnostic identities and service literature use EDC17." },
        { "engine-cdi-generic", "engine-cdi", "CDI diesel ECU",
          { "CDI", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic CDI family only; requires further identity refinement",
          "Broad CDI identity retained only as a fallback classifier." },

        { "engine-med17", "engine-me", "MED17 petrol ECU",
          { "MED17", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes MED17 applications; exact suffix/software determines services",
          "Mercedes petrol diagnostic identities use MED17." },
        { "engine-sim271", "engine-me", "SIM271 petrol ECU",
          { "SIM271", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes SIM271 applications",
          "Mercedes service/diagnostic identities use SIM271." },
        { "engine-me9", "engine-me", "ME9 petrol ECU",
          { "ME9", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes ME9 applications",
          "Mercedes service/diagnostic identities use ME9." },
        { "engine-me-generic", "engine-me", "ME-SFI petrol ECU",
          { "ME-SFI", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic ME-SFI family only; requires further identity refinement",
          "Broad ME-SFI identity retained only as a fallback classifier." },

        { "transmission-egs51", "transmission-vgs", "EGS51 transmission ECU",
          { "EGS51", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes EGS51 applications",
          "Mercedes transmission diagnostic families distinguish EGS51." },
        { "transmission-egs52", "transmission-vgs", "EGS52 / NAG1 transmission ECU",
          { "EGS52", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes EGS52/NAG1 applications",
          "DAS/KWP and open NAG52 material distinguish OEM EGS52." },
        { "transmission-egs53", "transmission-vgs", "EGS53 transmission ECU",
          { "EGS53", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes EGS53 applications",
          "Mercedes transmission diagnostic families distinguish EGS53." },
        { "transmission-vgs-nag2", "transmission-vgs", "VGS / NAG2 / 722.9 transmission ECU",
          { "VGSNAG", "NAG2", "722.9", "VGS3" },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes 722.9/VGS applications",
          "Mercedes diagnostic identities distinguish the VGS/NAG2 family." },
        { "transmission-vgs-generic", "transmission-vgs", "VGS / ETC transmission ECU",
          { "VGS", "ETC", "EGS", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic transmission family only; requires further identity refinement",
          "Broad VGS/ETC/EGS identity retained only as a fallback classifier." },

        { "selector-ism", "selector", "Intelligent Servo Module selector",
          { "ISM", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes ISM-equipped applications",
          "Mercedes service topology identifies ISM as a selector controller." },
        { "selector-ewm", "selector", "Electronic selector module (EWM)",
          { "EWM", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes EWM-equipped applications",
          "Mercedes diagnostic identities use EWM." },
        { "selector-esm", "selector", "Electronic selector module (ESM)",
          { "ESM", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes ESM-equipped applications",
          "Mercedes diagnostic identities use ESM." },
        { "selector-direct-select", "selector", "DIRECT SELECT controller",
          { "DIRECT SELECT", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes DIRECT SELECT applications",
          "Mercedes service topology identifies DIRECT SELECT separately from the TCU." },

        { "esp-abr2xt", "esp", "ABR2XT brake/ESP controller",
          { "ABR2XT", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Source-backed ABR2XT applications; exact chassis/year varies",
          "Caesar material identifies ABR2XT and its independent diagnostic route." },
        { "esp-esp212", "esp", "ESP212 controller",
          { "ESP212", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era ESP212 evidence; identity match required",
          "Mercedes diagnostic identities use ESP212." },
        { "esp-generic", "esp", "ESP / ABS / BAS controller",
          { "ESP", "ABS", "BAS", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic brake/stability family only; requires further identity refinement",
          "Broad ESP/ABS/BAS names retained only as fallback classifiers." },

        { "restraints-orc212", "restraints-orc", "ORC_212 restraint controller",
          { "ORC_212", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era ORC_212 evidence; identity match required",
          "Mercedes diagnostic coverage identifies ORC_212." },
        { "restraints-orc-generic", "restraints-orc", "ORC / SRS restraint controller",
          { "ORC", "SRS", "AIRBAG", "RESTRAINT" },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic restraint family only; requires further identity refinement",
          "Broad ORC/SRS identities retained only as fallback classifiers." },
        { "pretensioner-rbtmfl204", "belt-pretensioner-left",
          "RBTMFL_204 reversible belt tensioner",
          { "RBTMFL_204", "RBTMFL", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era left PRE-SAFE tensioner evidence",
          "Mercedes PRE-SAFE diagnostic coverage identifies RBTMFL." },
        { "pretensioner-rbtmfr204", "belt-pretensioner-right",
          "RBTMFR_204 reversible belt tensioner",
          { "RBTMFR_204", "RBTMFR", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era right PRE-SAFE tensioner evidence",
          "Mercedes PRE-SAFE diagnostic coverage identifies RBTMFR." },

        { "cluster-ic204", "instrument-cluster", "IC_204 instrument cluster",
          { "IC_204", "IC204", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204-family cluster evidence; identity match required",
          "Mercedes diagnostic identities use IC_204." },
        { "cluster-ic212", "instrument-cluster", "IC_212 instrument cluster",
          { "IC_212", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "212-family cluster evidence; identity match required",
          "Mercedes diagnostic identities use IC_212." },
        { "cluster-kombi", "instrument-cluster", "KOMBI instrument cluster",
          { "KOMBI", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic Mercedes cluster identity; exact generation varies",
          "KOMBI is an established Mercedes cluster identity." },

        { "headunit-hu204", "audio-headunit", "HU_204 head unit",
          { "HU_204", "HU204", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era HU_204 evidence; identity match required",
          "Public Monaco trace and Mercedes coverage identify HU_204." },
        { "audio-ctrlc204", "audio-controller", "CTRLC_204 operating unit",
          { "CTRLC_204", "CTRLC204", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era CTRLC_204 evidence",
          "Mercedes diagnostic coverage identifies CTRLC_204." },
        { "display-dispc204", "central-display", "DISPC/DSPC_204 display",
          { "DISPC_204", "DISPC204", "DSPC_204", "ZAN" },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era display-controller evidence",
          "Mercedes diagnostic coverage identifies the 204-family display unit." },

        { "gateway-cgw212", "central-gateway", "CGW_212 central gateway",
          { "CGW_212", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "212/207-era CGW_212 evidence",
          "Mercedes service topology identifies CGW_212." },
        { "gateway-zgw", "central-gateway", "ZGW central gateway",
          { "ZGW", "CGW", "CENTRAL GATEWAY", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic Mercedes central-gateway family; exact generation varies",
          "Mercedes gateway nomenclature uses ZGW/CGW." },

        { "sam-front-212", "front-sam", "SAMF_212 front SAM",
          { "SAMF_212", "SAMF", "SAM-F", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era front-SAM evidence",
          "Mercedes service topology identifies SAMF_212." },
        { "sam-rear-212", "rear-sam", "SAMR_212 rear SAM",
          { "SAMR_212", "SAMR", "SAM-R", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era rear-SAM evidence",
          "Mercedes service topology identifies SAMR_212." },

        { "eis-ezs212", "eis-ezs", "EIS/EZS_212 ignition controller",
          { "EZS_212", "EIS_212", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "212/207-era EIS/EZS evidence",
          "Mercedes service/diagnostic coverage identifies EIS/EZS_212." },
        { "eis-ezs-generic", "eis-ezs", "EIS / EZS ignition controller",
          { "EZS", "EIS", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic Mercedes ignition-controller family",
          "Broad EIS/EZS identity retained only as a fallback classifier." },

        { "steering-mrm", "steering-column", "MRM steering-column controller",
          { "MRM", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes MRM applications; exact generation varies",
          "Mercedes CAN/diagnostic material identifies MRM." },
        { "steering-scm", "steering-column", "SCM/STW steering-column controller",
          { "SCM", "STW", "STEERING COLUMN", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic steering-column controller family",
          "Mercedes diagnostic identities use SCM/STW." },

        { "climate-212", "climate", "KLA/AAC/HVAC_212 climate controller",
          { "HVAC_212", "KLA_212", "AAC_212", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era climate-controller evidence",
          "Mercedes diagnostic coverage identifies the 212-family climate controller." },
        { "climate-kla-generic", "climate", "KLA climate controller",
          { "KLA", NULL, NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic Mercedes KLA family; exact generation varies",
          "KLA is established Mercedes climate-control nomenclature." },

        { "airmatic-ads212", "airmatic-ads", "ADS212 AIRmatic controller",
          { "ADS212", "ADS_", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "212/207-era ADS evidence where option is fitted",
          "Mercedes service topology identifies ADS/AIRmatic controllers." },
        { "airmatic-generic", "airmatic-ads", "AIRmatic / damping controller",
          { "AIRMATIC", "DAMPING", NULL, NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Generic AIRmatic family; exact generation varies",
          "Broad AIRmatic/damping identity retained as fallback." },

        { "distronic-dtr", "distronic", "DISTRONIC / DTR controller",
          { "DISTRONIC PLUS", "DISTRONIC", "DTR", "A89" },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes DISTRONIC-equipped applications",
          "Mercedes service topology identifies DTR/DISTRONIC controllers." },
        { "headlamp-range", "headlamp-range", "Headlamp range controller",
          { "HRA", "LWR", "HLC", "HEADLAMP RANGE" },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "Mercedes headlamp-range-equipped applications",
          "Mercedes service topology identifies HRA/LWR/HLC controllers." },

        { "seat-driver-212", "seat-driver", "SEATD_212 / SSGF driver-seat controller",
          { "SEATD_212", "SEATD", "SSGF", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "212/207-era driver-seat-controller evidence",
          "Mercedes diagnostic coverage identifies SEATD/SSGF." },
        { "seat-passenger-204", "seat-passenger", "SEATP_204 / SSGB passenger-seat controller",
          { "SEATP_204", "SEATP", "SSGB", NULL },
          MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
          "204/207/212-era passenger-seat-controller evidence",
          "Mercedes diagnostic coverage identifies SEATP/SSGB." }
    };
    return index < sizeof(definitions) / sizeof(definitions[0])
        ? &definitions[index] : NULL;
}

static inline size_t mblink_mercedes_controller_family_definition_count(void)
{
    size_t count = 0U;
    while (mblink_mercedes_controller_family_definition_at(count) != NULL)
        ++count;
    return count;
}

static inline const MblinkMercedesControllerFamilyDefinition *
mblink_mercedes_controller_family_definition_for_key(const char *key)
{
    size_t index;
    if (key == NULL || key[0] == '\0') return NULL;
    for (index = 0U; ; ++index) {
        const MblinkMercedesControllerFamilyDefinition *definition =
            mblink_mercedes_controller_family_definition_at(index);
        if (definition == NULL) return NULL;
        if (strcmp(definition->key, key) == 0) return definition;
    }
}

static inline const MblinkMercedesControllerFamilyDefinition *
mblink_mercedes_controller_family_definition_for_evidence(
    const char *module_key,
    const char *identity,
    const char *software_number,
    const char *hardware_number)
{
    size_t index;
    if (module_key == NULL || module_key[0] == '\0') return NULL;

    for (index = 0U; ; ++index) {
        const MblinkMercedesControllerFamilyDefinition *definition =
            mblink_mercedes_controller_family_definition_at(index);
        size_t alias_index;
        if (definition == NULL) return NULL;
        if (strcmp(definition->module_key, module_key) != 0) continue;

        for (alias_index = 0U;
             alias_index < MBLINK_MERCEDES_CONTROLLER_ALIAS_COUNT;
             ++alias_index) {
            const char *alias = definition->identity_aliases[alias_index];
            if (alias == NULL) continue;
            if (mblink_mercedes_module_ascii_contains_case_insensitive(
                    identity, alias) ||
                mblink_mercedes_module_ascii_contains_case_insensitive(
                    software_number, alias) ||
                mblink_mercedes_module_ascii_contains_case_insensitive(
                    hardware_number, alias)) {
                return definition;
            }
        }
    }
}

/*
 * Compatibility aliases for callers built against the earlier development-
 * vehicle-scoped API. New code must use the Mercedes-wide names above.
 */
static inline const MblinkMercedesModuleDefinition *
mblink_mercedes_c207_module_definition_at(size_t index)
{
    return mblink_mercedes_module_definition_at(index);
}

static inline size_t mblink_mercedes_c207_module_definition_count(void)
{
    return mblink_mercedes_module_definition_count();
}

static inline const MblinkMercedesModuleDefinition *
mblink_mercedes_c207_module_definition_for_key(const char *key)
{
    return mblink_mercedes_module_definition_for_key(key);
}

static inline const MblinkMercedesModuleDefinition *
mblink_mercedes_c207_module_definition_for_identity(const char *identity)
{
    return mblink_mercedes_module_definition_for_identity(identity);
}

#ifdef __cplusplus
}
#endif

#endif
