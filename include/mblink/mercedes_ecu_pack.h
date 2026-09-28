// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_ecu_pack.h
 * @brief Unified, controller-scoped Mercedes ECU definition packs.
 *
 * One pack is the public diagnostic description of one resolved ECU family:
 * friendly/module identity, physical route, protocol/session metadata and the
 * family-owned documented/live data catalogue.  The legacy evidence tables
 * remain the backing store while callers consume this single coherent view.
 */
#ifndef MBLINK_MERCEDES_ECU_PACK_H
#define MBLINK_MERCEDES_ECU_PACK_H

#include "mblink/mercedes_data_scan.h"
#include "mblink/mercedes_documented_ecus.h"
#include "mblink/mercedes_module_scan_core.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MblinkMercedesEcuDataKind {
    MBLINK_MERCEDES_ECU_DATA_IDENTIFICATION = 0,
    MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE,
    MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ,
    MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED
} MblinkMercedesEcuDataKind;

typedef struct MblinkMercedesEcuDataItem {
    uint8_t service;
    uint16_t identifier;
    const char *stable_key;
    const char *short_name;
    const char *name;
    const char *unit;
    MblinkMercedesEcuDataKind kind;
    MblinkMercedesDefinitionStatus status;
    const char *provenance;
    bool live;
    bool advertised;
    bool allow_duplicate_wire;
    size_t field_count;
} MblinkMercedesEcuDataItem;

typedef struct MblinkMercedesEcuPack {
    const char *key;
    const char *ecu_name;
    const char *display_name;
    const char *component_designation;
    const char *network;
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool extended_id;
    MblinkMercedesDiagnosticProtocol protocol;
    bool protocol_authoritative;
    bool observed_protocol_conflict;
    const char *session_command;
    const char *tester_present_command;
    const char *quit_command;
    const char *provenance;
    const MblinkMercedesModuleDefinition *module_definition;
    const MblinkMercedesControllerFamilyDefinition *controller_family;
    const MblinkMercedesDocumentedEcuProfile *documented_profile;
    const char *controller_data_profile_key;
} MblinkMercedesEcuPack;

bool mblink_mercedes_ecu_pack_resolve(
    const char *module_key,
    const char *controller_family_key,
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesEcuPack *pack);

bool mblink_mercedes_ecu_pack_resolve_module(
    const MblinkMercedesModuleScanEntry *module,
    MblinkMercedesEcuPack *pack);

#define MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK UINT8_C(0x01)
#define MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK UINT8_C(0x02)

uint8_t mblink_mercedes_ecu_pack_route_protocol_mask(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id);

size_t mblink_mercedes_ecu_pack_route_profile_count_for_protocol(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol);

size_t mblink_mercedes_ecu_pack_alias_count(
    const MblinkMercedesEcuPack *pack);
const char *mblink_mercedes_ecu_pack_alias_at(
    const MblinkMercedesEcuPack *pack,
    size_t index);

size_t mblink_mercedes_ecu_pack_data_item_count(
    const MblinkMercedesEcuPack *pack);
bool mblink_mercedes_ecu_pack_data_item_at(
    const MblinkMercedesEcuPack *pack,
    size_t index,
    MblinkMercedesEcuDataItem *item);

const MblinkMercedesDocumentedField *mblink_mercedes_ecu_pack_field_at(
    const MblinkMercedesEcuDataItem *item,
    size_t index);

const char *mblink_mercedes_ecu_data_kind_name(
    MblinkMercedesEcuDataKind kind);

#ifdef __cplusplus
}
#endif

#endif
