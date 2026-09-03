// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_factory_data.h
 * @brief Offline Mercedes factory-option evidence and conflict precedence.
 *
 * This layer contains no network client and no credential handling. It keeps
 * reusable facts learned from Mercedes-Benz factory data separate from live
 * ECU evidence. A factory option may prioritise a module family for discovery,
 * but it never proves a controller identity or authorises a write operation.
 */
#ifndef MBLINK_MERCEDES_FACTORY_DATA_H
#define MBLINK_MERCEDES_FACTORY_DATA_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MblinkMercedesEvidenceRank {
    MBLINK_MERCEDES_EVIDENCE_NONE = 0,
    MBLINK_MERCEDES_EVIDENCE_PLATFORM_ASSUMPTION = 10,
    MBLINK_MERCEDES_EVIDENCE_GENERIC_METADATA = 20,
    MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_SPEC = 30,
    MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_OPTION = 40,
    MBLINK_MERCEDES_EVIDENCE_POSITIVE_DIAGNOSTIC = 50,
    MBLINK_MERCEDES_EVIDENCE_ECU_IDENTITY = 60
} MblinkMercedesEvidenceRank;

const char *mblink_mercedes_evidence_rank_name(
    MblinkMercedesEvidenceRank rank);

/**
 * Return true only when candidate is strictly stronger than current.
 *
 * This encodes the rule used when Mercedes factory metadata conflicts with the
 * physical vehicle: ECU identity and proven diagnostic responses win over a
 * VIN option; a VIN option wins over generic vehicle metadata.
 */
bool mblink_mercedes_evidence_should_replace(
    MblinkMercedesEvidenceRank current,
    MblinkMercedesEvidenceRank candidate);

typedef struct MblinkMercedesFactoryOptionDefinition {
    const char *code;
    const char *description;
    const char *module_key_hint;
    const char *observed_baumuster;
    const char *provenance;
} MblinkMercedesFactoryOptionDefinition;

size_t mblink_mercedes_factory_option_count(void);
const MblinkMercedesFactoryOptionDefinition *
mblink_mercedes_factory_option_at(size_t index);
const MblinkMercedesFactoryOptionDefinition *
mblink_mercedes_factory_option_for_code(const char *code);

/**
 * A module hint changes scan order only. It is not controller-family proof.
 */
bool mblink_mercedes_factory_option_prioritises_module(
    const char *option_code,
    const char *module_key);

#ifdef __cplusplus
}
#endif

#endif
