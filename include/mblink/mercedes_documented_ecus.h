// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_MERCEDES_DOCUMENTED_ECUS_H
#define MBLINK_MERCEDES_DOCUMENTED_ECUS_H
#include "mblink/mercedes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct MblinkMercedesDocumentedRead {
    uint8_t service;
    uint16_t identifier;
} MblinkMercedesDocumentedRead;
typedef struct MblinkMercedesDocumentedEcuProfile {
    const char *source_key;
    const char *name;
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool route_available;
    bool extended_id;
    bool protocol_known;
    MblinkMercedesDiagnosticProtocol protocol;
    const char *session_command;
    const char *tester_present_command;
    const char *quit_command;
    size_t read_offset;
    size_t read_count;
} MblinkMercedesDocumentedEcuProfile;
typedef struct MblinkMercedesDocumentedField {
    uint8_t service;
    uint16_t identifier;
    const char *name;
    uint16_t response_byte;
    int8_t bit_offset;
    uint16_t bit_length;
    const char *unit;
    const char *provenance;
} MblinkMercedesDocumentedField;
size_t mblink_mercedes_documented_ecu_profile_count(void);
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_at(size_t index);
size_t mblink_mercedes_documented_ecu_profile_count_for_route(uint32_t tx_can_id,uint32_t rx_can_id,bool extended_id);
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_at_for_route(uint32_t tx_can_id,uint32_t rx_can_id,bool extended_id,size_t index);
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_for_name_and_route(const char *name,uint32_t tx_can_id,uint32_t rx_can_id,bool extended_id);
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_for_controller_family(const char *controller_family_key,uint32_t tx_can_id,uint32_t rx_can_id,bool extended_id,MblinkMercedesDiagnosticProtocol protocol);
size_t mblink_mercedes_documented_ecu_read_count(const MblinkMercedesDocumentedEcuProfile *profile);
const MblinkMercedesDocumentedRead *mblink_mercedes_documented_ecu_read_at(const MblinkMercedesDocumentedEcuProfile *profile,size_t index);

/*
 * Return the de-duplicated union of documented safe read commands for every
 * controller profile that is published on an exact diagnostic route and is
 * compatible with the detected protocol.  This lets an otherwise unnamed ECU
 * immediately benefit from the global catalogue without pretending that an
 * address alone proves one controller generation.
 */
size_t mblink_mercedes_documented_route_read_count(
    uint32_t tx_can_id, uint32_t rx_can_id, bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol);
const MblinkMercedesDocumentedRead *mblink_mercedes_documented_route_read_at(
    uint32_t tx_can_id, uint32_t rx_can_id, bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol, size_t index);

/**
 * Resolve an unambiguous documented diagnostic-session control command for an
 * exact route/protocol. When several controller generations share the route,
 * every matching profile must agree on the same simple two-byte command.
 *
 * Set quit_session=false for the profile's session-entry command and true for
 * its normal/default-session exit command. The command is returned as four
 * uppercase hex characters without a 0x prefix.
 */
bool mblink_mercedes_documented_route_control_command(
    uint32_t tx_can_id, uint32_t rx_can_id, bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol, bool quit_session,
    char *buffer, size_t buffer_size);

const char *mblink_mercedes_documented_read_name(uint8_t service,uint16_t identifier);
bool mblink_mercedes_documented_read_is_safe(uint8_t service,uint16_t identifier);
/* Hardware/software identification belongs to one-time module discovery. */
bool mblink_mercedes_documented_read_is_module_metadata(
    uint8_t service, uint16_t identifier);
size_t mblink_mercedes_documented_field_count(uint8_t service,uint16_t identifier);
const MblinkMercedesDocumentedField *mblink_mercedes_documented_field_at(uint8_t service,uint16_t identifier,size_t index);
const char *mblink_mercedes_documented_route_source(void);
const char *mblink_mercedes_documented_field_source(void);
#ifdef __cplusplus
}
#endif
#endif
