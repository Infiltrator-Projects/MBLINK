// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_catalog.h
 * @brief Source-corroborated Mercedes module and controller-family catalogue.
 *
 * This public header contains the catalogue types and lookup API only.
 * Definition tables and lookup implementation live in module_catalog.c so
 * every consumer shares one canonical copy.
 */
#ifndef MBLINK_MERCEDES_MODULE_CATALOG_H
#define MBLINK_MERCEDES_MODULE_CATALOG_H

#include "mblink/mercedes.h"

#include <stdbool.h>
#include <stddef.h>

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

const char *mblink_mercedes_module_presence_name(
    MblinkMercedesModulePresence presence);

const MblinkMercedesModuleDefinition *
mblink_mercedes_module_definition_at(size_t index);

size_t mblink_mercedes_module_definition_count(void);

const MblinkMercedesModuleDefinition *
mblink_mercedes_module_definition_for_key(const char *key);

bool mblink_mercedes_module_ascii_contains_case_insensitive(
    const char *text,
    const char *needle);

const MblinkMercedesModuleDefinition *
mblink_mercedes_module_definition_for_identity(const char *identity);

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

const MblinkMercedesControllerFamilyDefinition *
mblink_mercedes_controller_family_definition_at(size_t index);

size_t mblink_mercedes_controller_family_definition_count(void);

const MblinkMercedesControllerFamilyDefinition *
mblink_mercedes_controller_family_definition_for_key(const char *key);

const MblinkMercedesControllerFamilyDefinition *
mblink_mercedes_controller_family_definition_for_evidence(
    const char *module_key,
    const char *identity,
    const char *software_number,
    const char *hardware_number);


#ifdef __cplusplus
}
#endif

#endif
