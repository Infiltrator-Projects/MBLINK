// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_ecu_pack.h"

#include "mblink/mercedes_transmission.h"

#include <stdio.h>
#include <string.h>

typedef struct MblinkMercedesCanonicalTransmissionSignal {
    const char *stable_key;
    const char *short_name;
    const char *name;
} MblinkMercedesCanonicalTransmissionSignal;

static const MblinkMercedesCanonicalTransmissionSignal
    canonical_2130_signals[] = {
    { "mercedes.transmission.oil_temperature", "ATF",
      "Transmission oil temperature" },
    { "mercedes.transmission.actual_gear", "GEAR", "Current gear" },
    { "mercedes.transmission.target_gear", "TARGET", "Target gear" },
    { "mercedes.transmission.tcc_state", "TCC",
      "Torque converter clutch state" },
    { "mercedes.transmission.recognised_gear", "RECOG",
      "Recognised transmission gear" },
    { "mercedes.transmission.selector_position", "SELECT",
      "Selector position" },
    { "mercedes.transmission.drive_program", "PROGRAM",
      "Transmission drive program" },
    { "mercedes.transmission.tcc_delta_speed_raw", "TCC Δ",
      "Torque converter delta speed (raw)" },
    { "mercedes.transmission.tcc_speed_raw", "TCC SPD",
      "Torque converter speed (raw)" },
    { "mercedes.transmission.tcc_pressure_raw", "TCC P",
      "Torque converter pressure (raw)" },
    { "mercedes.transmission.engine_torque_signed_raw", "ENG TQ",
      "Engine torque (signed raw)" },
    { "mercedes.transmission.converter_torque_signed_raw", "CONV TQ",
      "Converter torque (signed raw)" },
    { "mercedes.transmission.output_speed_raw", "OUT SPD",
      "Transmission output speed (raw)" }
};

static bool pack_is_transmission_controller(
    const MblinkMercedesEcuPack *pack)
{
    return pack != NULL &&
        pack->module_definition != NULL &&
        pack->module_definition->kind == MBLINK_MERCEDES_MODULE_TRANSMISSION &&
        strcmp(pack->module_definition->key, "selector") != 0;
}

static MblinkMercedesTransmissionFamily pack_transmission_family(
    const MblinkMercedesEcuPack *pack)
{
    if (!pack_is_transmission_controller(pack) ||
        pack->controller_family == NULL ||
        pack->controller_family->key == NULL) {
        return MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN;
    }
    return mblink_mercedes_transmission_family_from_controller_family_key(
        pack->controller_family->key);
}

static bool is_identification_read(uint8_t service, uint16_t identifier)
{
    if (service == UINT8_C(0x22))
        return identifier >= UINT16_C(0xf100) &&
               identifier <= UINT16_C(0xf1ff);
    if (service == UINT8_C(0x1a))
        return identifier >= UINT16_C(0x0086) &&
               identifier <= UINT16_C(0x009f);
    return false;
}

static void clear_item(MblinkMercedesEcuDataItem *item)
{
    if (item != NULL) memset(item, 0, sizeof(*item));
}

typedef struct MblinkMercedesAssystWorkshopCodeMap {
    const char *workshop_code;
    const char *service_code;
} MblinkMercedesAssystWorkshopCodeMap;

/*
 * IC_204 / 204-212-era ASSYST PLUS mapping.
 *
 * This table belongs to the resolved controller family, not to Mercedes
 * globally. Other ASSYST generations may reuse workshop-code strings with
 * different service-display semantics and must therefore supply their own
 * ECU-pack mapping.
 */
static const MblinkMercedesAssystWorkshopCodeMap
    ic204_assyst_workshop_code_map[] = {
    { "505", "A" },
    { "D0D", "A1" },
    { "550A", "A3" },
    { "D50J", "A4" },
    { "DD0S", "A6" },
    { "G50M", "A7" },
    { "Q50V", "A8" },
    { "GD0V", "A9" },
    { "850D", "A0" },
    { "QD051", "AC" },
    { "KD001", "AF" },
    { "801050E", "AH" },
    { "VD0A1", "AG" },
    { "10D0E", "AK" },
    { "15D0K", "AP" },
    { "606", "B" },
    { "E0E", "B1" },
    { "8E0N", "B2" },
    { "560B", "B3" },
    { "B60H", "B4" },
    { "5E0K", "B5" },
    { "3E0H", "B5" },
    { "DE0T", "B6" },
    { "G60N", "B7" },
    { "GN061", "B7" },
    { "Q60W", "B8" },
    { "GE0W", "B9" },
    { "1607", "B0" },
    { "960F", "B0" },
    { "TE091", "BC" },
    { "M60T", "BD" },
    { "V6031", "BE" },
    { "ME031", "BF" },
    { "KE011", "BF" },
    { "10607", "BH" },
    { "1460B", "BH" },
    { "10E0F", "BK" },
    { "14E0K", "BK" },
    { "15E0L", "BP" },
    { "1XE0E1", "BQ" },
    { "1Q60X", "BS" },
    { "10405", "CH" }
};

static bool ecu_pack_is_family(
    const MblinkMercedesEcuPack *pack,
    const char *family_key)
{
    return pack != NULL && family_key != NULL &&
        pack->controller_family != NULL &&
        pack->controller_family->key != NULL &&
        strcmp(pack->controller_family->key, family_key) == 0;
}

static bool ecu_pack_ascii_payload(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size)
{
    size_t length;

    if (record == NULL || buffer == NULL || buffer_size == 0U)
        return false;
    length = record->data_length;
    while (length > 0U &&
           (record->data[length - 1U] == UINT8_C(0x00) ||
            record->data[length - 1U] == UINT8_C(0xff))) {
        --length;
    }
    if (length == 0U || length + 1U > buffer_size) return false;
    for (size_t index = 0U; index < length; ++index) {
        if (record->data[index] < UINT8_C(0x20) ||
            record->data[index] > UINT8_C(0x7e)) {
            return false;
        }
        buffer[index] = (char)record->data[index];
    }
    buffer[length] = '\0';
    return true;
}

bool mblink_mercedes_ecu_pack_decode_numeric_value(
    const MblinkMercedesEcuPack *pack,
    const MblinkMercedesDataRecord *record,
    double *value,
    const char **name,
    const char **unit)
{
    if (value != NULL) *value = 0.0;
    if (name != NULL) *name = NULL;
    if (unit != NULL) *unit = NULL;
    if (pack == NULL || record == NULL ||
        value == NULL || name == NULL || unit == NULL) {
        return false;
    }

    if (ecu_pack_is_family(pack, "cluster-ic204") &&
        record->service == UINT8_C(0x22) &&
        record->identifier == UINT16_C(0x0302) &&
        record->data_length == 3U) {
        const uint32_t raw =
            ((uint32_t)record->data[0] << 16U) |
            ((uint32_t)record->data[1] << 8U) |
            (uint32_t)record->data[2];
        *value = (double)raw / 1000.0;
        *name = "Average daily distance";
        *unit = "km/day";
        return true;
    }

    return false;
}

bool mblink_mercedes_ecu_pack_format_value(
    const MblinkMercedesEcuPack *pack,
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size,
    const char **name)
{
    char code[16];

    if (name != NULL) *name = NULL;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (pack == NULL || record == NULL ||
        buffer == NULL || buffer_size == 0U || name == NULL) {
        return false;
    }

    if (!ecu_pack_is_family(pack, "cluster-ic204") ||
        record->service != UINT8_C(0x22)) {
        return false;
    }

    /*
     * IC_204 ASSYST/WIA measured-value record (22 0402).
     * Captured 00 75 00 is interpreted as an inferred WIA remaining-time
     * record: the middle byte is the day count while the surrounding bytes
     * are currently zero state/mode fields. Keep the decoder deliberately
     * narrow until additional captures establish those outer fields.
     */
    if (record->identifier == UINT16_C(0x0402) &&
        record->data_length == 3U &&
        record->data[0] == UINT8_C(0x00) &&
        record->data[2] == UINT8_C(0x00)) {
        const int count = snprintf(
            buffer, buffer_size, "%u days",
            (unsigned int)record->data[1]);
        if (count < 0 || (size_t)count >= buffer_size)
            return false;
        *name = "Service due in";
        return true;
    }

    /*
     * IC_204 CBF names 22 0408 as ASSYST maintenance 1. Mercedes service
     * documentation defines Service 1 as the minor basic service / Service A.
     * The three captured bytes are still an unresolved packed state record, so
     * present only the source-backed semantic identity and keep the raw bytes
     * as evidence rather than inventing subfield meanings.
     */
    if (record->identifier == UINT16_C(0x0408) &&
        record->data_length == 3U) {
        const char *status = "Tracked by ASSYST";
        const size_t length = strlen(status);
        if (length + 1U > buffer_size) return false;
        memcpy(buffer, status, length + 1U);
        *name = "Service A maintenance";
        return true;
    }

    if (record->identifier != UINT16_C(0x0306) ||
        !ecu_pack_ascii_payload(record, code, sizeof(code))) {
        return false;
    }

    for (size_t index = 0U;
         index < sizeof(ic204_assyst_workshop_code_map) /
                     sizeof(ic204_assyst_workshop_code_map[0]);
         ++index) {
        if (strcmp(
                code,
                ic204_assyst_workshop_code_map[index].workshop_code) == 0) {
            const int count = snprintf(
                buffer, buffer_size, "Service %s",
                ic204_assyst_workshop_code_map[index].service_code);
            if (count < 0 || (size_t)count >= buffer_size)
                return false;
            *name = "Next service";
            return true;
        }
    }

    {
        const int count = snprintf(
            buffer, buffer_size, "Workshop code %s", code);
        if (count < 0 || (size_t)count >= buffer_size)
            return false;
        *name = "Next service";
        return true;
    }
}

typedef struct MblinkMercedesEcuPackDataPolicy {
    const char *controller_family_key;
    uint8_t service;
    uint16_t identifier;
    MblinkMercedesEcuDataAcquisition acquisition;
    const char *name_override;
} MblinkMercedesEcuPackDataPolicy;

/*
 * Controller-family-owned acquisition policy.
 *
 * This is data, not discovery logic: once a module resolves to an ECU pack,
 * these rows become part of that pack's STARTUP_ONCE / USER_POLLING split.
 * Adding another controller-specific startup value therefore never changes
 * the discovery state machine or the generic startup scheduler.
 */
static const MblinkMercedesEcuPackDataPolicy pack_data_policies[] = {
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0302),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "Average daily distance" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0306),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "Next service" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0402),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "Service due in" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0408),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "Service A maintenance" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0002),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "Restraint equipment configuration" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0058),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "ECU lock state / tester identification" },
    { "restraints-orc212", UINT8_C(0x21), UINT16_C(0x0002),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "Restraint equipment configuration" },
    { "restraints-orc212", UINT8_C(0x21), UINT16_C(0x0058),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
      "ECU lock state / tester identification" },
    { "transmission-egs53", UINT8_C(0x21), UINT16_C(0x00b1),
      MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE, NULL }
};

static const MblinkMercedesEcuPackDataPolicy *pack_data_policy_for_item(
    const MblinkMercedesEcuPack *pack,
    uint8_t service,
    uint16_t identifier)
{
    const char *key = pack != NULL && pack->controller_family != NULL
        ? pack->controller_family->key : NULL;
    size_t index;

    if (key == NULL) return NULL;
    for (index = 0U;
         index < sizeof(pack_data_policies) / sizeof(pack_data_policies[0]);
         ++index) {
        const MblinkMercedesEcuPackDataPolicy *policy =
            &pack_data_policies[index];
        if (policy->service == service &&
            policy->identifier == identifier &&
            strcmp(policy->controller_family_key, key) == 0) {
            return policy;
        }
    }
    return NULL;
}

static MblinkMercedesEcuDataAcquisition pack_item_acquisition(
    const MblinkMercedesEcuPack *pack,
    uint8_t service,
    uint16_t identifier)
{
    const MblinkMercedesEcuPackDataPolicy *policy;

    if (mblink_mercedes_documented_read_is_module_metadata(
            service, identifier)) {
        return MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE;
    }
    policy = pack_data_policy_for_item(pack, service, identifier);
    return policy != NULL
        ? policy->acquisition
        : MBLINK_MERCEDES_ECU_DATA_USER_POLLING;
}

static const char *pack_item_name(
    const MblinkMercedesEcuPack *pack,
    uint8_t service,
    uint16_t identifier,
    const char *fallback)
{
    const MblinkMercedesEcuPackDataPolicy *policy =
        pack_data_policy_for_item(pack, service, identifier);
    return policy != NULL && policy->name_override != NULL
        ? policy->name_override : fallback;
}

static bool pack_item_acquired_during_identification(
    uint8_t service,
    uint16_t identifier)
{
    if (service == UINT8_C(0x22)) {
        return identifier == UINT16_C(0xf187) ||
               identifier == UINT16_C(0xf188) ||
               identifier == UINT16_C(0xf191) ||
               identifier == UINT16_C(0xf197);
    }
    return service == UINT8_C(0x1a) &&
        (identifier == UINT16_C(0x0086) ||
         identifier == UINT16_C(0x0087) ||
         identifier == UINT16_C(0x0089));
}

static void classify_item_acquisition(
    const MblinkMercedesEcuPack *pack,
    MblinkMercedesEcuDataItem *item)
{
    if (item == NULL) return;
    item->acquisition = pack_item_acquisition(
        pack, item->service, item->identifier);
    item->acquired_during_identification =
        item->acquisition == MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE &&
        pack_item_acquired_during_identification(
            item->service, item->identifier);
}

static const MblinkMercedesDocumentedEcuProfile *
pack_profile_for_controller(
    const MblinkMercedesControllerFamilyDefinition *controller,
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol observed_protocol,
    bool *protocol_authoritative,
    bool *protocol_conflict)
{
    const MblinkMercedesDocumentedEcuProfile *uds_profile;
    const MblinkMercedesDocumentedEcuProfile *kwp_profile;
    const MblinkMercedesDocumentedEcuProfile *profile;

    if (protocol_authoritative != NULL) *protocol_authoritative = false;
    if (protocol_conflict != NULL) *protocol_conflict = false;
    if (controller == NULL || controller->key == NULL) return NULL;

    uds_profile =
        mblink_mercedes_documented_ecu_profile_for_controller_family(
            controller->key, tx_can_id, rx_can_id, extended_id,
            MBLINK_MERCEDES_DIAGNOSTIC_UDS);
    kwp_profile =
        mblink_mercedes_documented_ecu_profile_for_controller_family(
            controller->key, tx_can_id, rx_can_id, extended_id,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);

    /*
     * A controller family can only dictate transport when its own documented
     * pack has one unambiguous protocol on this route.  If future evidence
     * publishes two protocol variants for the same family/route, retain the
     * protocol actually observed on the vehicle until identity is refined.
     */
    if (uds_profile != NULL && kwp_profile != NULL &&
        uds_profile != kwp_profile) {
        return observed_protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
            ? kwp_profile : uds_profile;
    }

    profile = uds_profile != NULL ? uds_profile : kwp_profile;
    if (profile != NULL && profile->protocol_known) {
        if (protocol_authoritative != NULL) *protocol_authoritative = true;
        if (protocol_conflict != NULL)
            *protocol_conflict = profile->protocol != observed_protocol;
    }
    return profile;
}

uint8_t mblink_mercedes_ecu_pack_route_protocol_mask(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id)
{
    uint8_t mask = 0U;
    const size_t count =
        mblink_mercedes_documented_ecu_profile_count_for_route(
            tx_can_id, rx_can_id, extended_id);

    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesDocumentedEcuProfile *profile =
            mblink_mercedes_documented_ecu_profile_at_for_route(
                tx_can_id, rx_can_id, extended_id, index);
        if (profile == NULL || !profile->protocol_known) continue;
        if (profile->protocol == MBLINK_MERCEDES_DIAGNOSTIC_UDS)
            mask |= MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK;
        else if (profile->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000)
            mask |= MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK;
    }
    return mask;
}

bool mblink_mercedes_ecu_pack_resolve(
    const char *module_key,
    const char *controller_family_key,
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesEcuPack *pack)
{
    const MblinkMercedesControllerFamilyDefinition *controller = NULL;
    const MblinkMercedesModuleDefinition *module = NULL;
    const MblinkMercedesDocumentedEcuProfile *profile = NULL;
    const char *effective_module_key = module_key;

    if (pack == NULL ||
        protocol > MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        return false;
    }
    memset(pack, 0, sizeof(*pack));

    if (controller_family_key != NULL &&
        controller_family_key[0] != '\0') {
        controller =
            mblink_mercedes_controller_family_definition_for_key(
                controller_family_key);
        if (controller != NULL)
            effective_module_key = controller->module_key;
    }

    if (effective_module_key != NULL && effective_module_key[0] != '\0')
        module = mblink_mercedes_module_definition_for_key(effective_module_key);

    if (controller != NULL) {
        profile = pack_profile_for_controller(
            controller, tx_can_id, rx_can_id, extended_id, protocol,
            &pack->protocol_authoritative,
            &pack->observed_protocol_conflict);
    }

    pack->key = controller != NULL ? controller->key :
        (module != NULL ? module->key : NULL);
    pack->ecu_name = profile != NULL ? profile->name :
        (controller != NULL ? controller->display_name :
         (module != NULL ? module->display_name : NULL));
    pack->display_name = controller != NULL ? controller->display_name :
        (module != NULL ? module->display_name : pack->ecu_name);
    pack->component_designation =
        module != NULL ? module->component_designation : NULL;
    pack->network = module != NULL ? module->network : NULL;
    pack->tx_can_id = tx_can_id;
    pack->rx_can_id = rx_can_id;
    pack->extended_id = extended_id;
    pack->protocol = pack->protocol_authoritative && profile != NULL
        ? profile->protocol : protocol;
    pack->session_command = profile != NULL ? profile->session_command : NULL;
    pack->tester_present_command =
        profile != NULL ? profile->tester_present_command : NULL;
    pack->quit_command = profile != NULL ? profile->quit_command : NULL;
    pack->provenance = controller != NULL ? controller->provenance :
        (module != NULL ? module->provenance :
         mblink_mercedes_documented_route_source());
    pack->module_definition = module;
    pack->controller_family = controller;
    pack->documented_profile = profile;
    pack->controller_data_profile_key =
        controller != NULL &&
        mblink_mercedes_controller_data_profile_identifier_count(
            controller->key, pack->protocol) != 0U
            ? controller->key : NULL;

    return pack->key != NULL || pack->documented_profile != NULL;
}

bool mblink_mercedes_ecu_pack_resolve_module(
    const MblinkMercedesModuleScanEntry *module,
    MblinkMercedesEcuPack *pack)
{
    if (module == NULL) return false;
    return mblink_mercedes_ecu_pack_resolve(
        module->definition != NULL ? module->definition->key : NULL,
        module->controller_family != NULL
            ? module->controller_family->key : NULL,
        module->tx_can_id,
        module->rx_can_id,
        module->extended_id,
        module->protocol,
        pack);
}

size_t mblink_mercedes_ecu_pack_alias_count(
    const MblinkMercedesEcuPack *pack)
{
    size_t count = 0U;
    size_t index;

    if (pack == NULL) return 0U;
    if (pack->controller_family != NULL) {
        for (index = 0U;
             index < MBLINK_MERCEDES_CONTROLLER_ALIAS_COUNT;
             ++index) {
            if (pack->controller_family->identity_aliases[index] != NULL)
                ++count;
        }
    }
    if (pack->module_definition != NULL) {
        for (index = 0U;
             index < MBLINK_MERCEDES_MODULE_ALIAS_COUNT;
             ++index) {
            if (pack->module_definition->identity_aliases[index] != NULL)
                ++count;
        }
    }
    return count;
}

const char *mblink_mercedes_ecu_pack_alias_at(
    const MblinkMercedesEcuPack *pack,
    size_t wanted)
{
    size_t seen = 0U;
    size_t index;

    if (pack == NULL) return NULL;
    if (pack->controller_family != NULL) {
        for (index = 0U;
             index < MBLINK_MERCEDES_CONTROLLER_ALIAS_COUNT;
             ++index) {
            const char *alias =
                pack->controller_family->identity_aliases[index];
            if (alias == NULL) continue;
            if (seen++ == wanted) return alias;
        }
    }
    if (pack->module_definition != NULL) {
        for (index = 0U;
             index < MBLINK_MERCEDES_MODULE_ALIAS_COUNT;
             ++index) {
            const char *alias =
                pack->module_definition->identity_aliases[index];
            if (alias == NULL) continue;
            if (seen++ == wanted) return alias;
        }
    }
    return NULL;
}

static size_t canonical_signal_count(const MblinkMercedesEcuPack *pack)
{
    const MblinkMercedesTransmissionFamily family =
        pack_transmission_family(pack);
    if (pack == NULL ||
        pack->protocol != MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 ||
        !mblink_mercedes_transmission_family_uses_2130_actual_values(family)) {
        return 0U;
    }
    /*
     * The compact VGS/NAG2 21 30 shape only exposes the seven common fields.
     * The 24-byte EGS52/EGS53 shape also carries the six raw speed/pressure/
     * torque fields that the 2026-09-29 driving capture independently varied.
     */
    return family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS52 ||
           family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53
        ? sizeof(canonical_2130_signals) / sizeof(canonical_2130_signals[0])
        : 7U;
}

static size_t transmission_item_count(const MblinkMercedesEcuPack *pack)
{
    const MblinkMercedesTransmissionFamily family =
        pack_transmission_family(pack);
    if (pack == NULL ||
        pack->protocol != MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 ||
        family == MBLINK_MERCEDES_TRANSMISSION_FAMILY_UNKNOWN) {
        return 0U;
    }
    return mblink_mercedes_transmission_kwp_read_identifier_count_for_family(
        family);
}

size_t mblink_mercedes_ecu_pack_data_item_count(
    const MblinkMercedesEcuPack *pack)
{
    size_t count;
    if (pack == NULL) return 0U;
    count = canonical_signal_count(pack);
    count += transmission_item_count(pack);
    count += mblink_mercedes_controller_data_profile_identifier_count(
        pack->controller_data_profile_key, pack->protocol);
    count += mblink_mercedes_documented_ecu_read_count(
        pack->documented_profile);
    return count;
}

bool mblink_mercedes_ecu_pack_data_item_at(
    const MblinkMercedesEcuPack *pack,
    size_t index,
    MblinkMercedesEcuDataItem *item)
{
    size_t count;
    MblinkMercedesTransmissionFamily family;

    if (pack == NULL || item == NULL) return false;
    clear_item(item);

    count = canonical_signal_count(pack);
    if (index < count) {
        const MblinkMercedesCanonicalTransmissionSignal *signal =
            &canonical_2130_signals[index];
        item->service = UINT8_C(0x21);
        item->identifier = UINT16_C(0x0030);
        item->stable_key = signal->stable_key;
        item->short_name = signal->short_name;
        item->name = signal->name;
        item->kind = MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE;
        item->status = MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        item->provenance =
            "Mercedes GS KWP 21 30 · portable MBLINK family decoder";
        item->advertised = true;
        item->allow_duplicate_wire = true;
        classify_item_acquisition(pack, item);
        return true;
    }
    index -= count;

    count = transmission_item_count(pack);
    if (index < count) {
        const uint8_t local_id =
            mblink_mercedes_transmission_kwp_read_identifier_at_for_family(
                pack_transmission_family(pack), index);
        const bool previously_classified_live =
            mblink_mercedes_transmission_kwp_identifier_is_live_for_family(
                pack_transmission_family(pack), local_id);
        item->service = UINT8_C(0x21);
        item->identifier = (uint16_t)local_id;
        item->name =
            mblink_mercedes_transmission_kwp_read_identifier_name_for_family(
                pack_transmission_family(pack), local_id);
        item->kind = previously_classified_live
            ? MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE
            : MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ;
        item->status = MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        family = pack_transmission_family(pack);
        item->provenance = mblink_mercedes_transmission_family_name(family);
        /*
         * Keep the complete family-owned read catalogue here. Acquisition
         * classification below decides whether this exact item belongs to the
         * startup-once or user-polling section.
         */
        item->advertised = true;
        item->field_count =
            mblink_mercedes_documented_field_count(
                item->service, item->identifier);
        classify_item_acquisition(pack, item);
        return true;
    }
    index -= count;

    count = mblink_mercedes_controller_data_profile_identifier_count(
        pack->controller_data_profile_key, pack->protocol);
    if (index < count) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            mblink_mercedes_controller_data_profile_identifier_at(
                pack->controller_data_profile_key, pack->protocol, index);
        if (entry == NULL) return false;
        item->service = entry->service;
        item->identifier = entry->identifier;
        item->name = pack_item_name(
            pack, entry->service, entry->identifier, entry->name);
        if (entry->status ==
                MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
            item->kind = entry->source_dynamic_hint
                ? MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE
                : MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ;
        } else {
            item->kind = MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED;
        }
        item->status = entry->status;
        item->provenance = entry->provenance;
        /*
         * Preserve every source-backed controller Data service in the pack.
         * Explicit acquisition classification, not an old live/static flag,
         * decides whether a reviewed item is startup-once or user-polling.
         * Capture-only observations remain unadvertised.
         */
        item->advertised =
            entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        item->field_count =
            mblink_mercedes_documented_field_count(
                item->service, item->identifier);
        classify_item_acquisition(pack, item);
        return true;
    }
    index -= count;

    count = mblink_mercedes_documented_ecu_read_count(
        pack->documented_profile);
    if (index < count) {
        const MblinkMercedesDocumentedRead *read =
            mblink_mercedes_documented_ecu_read_at(
                pack->documented_profile, index);
        if (read == NULL) return false;
        item->service = read->service;
        item->identifier = read->identifier;
        item->name = pack_item_name(
            pack, read->service, read->identifier,
            mblink_mercedes_documented_read_name(
                read->service, read->identifier));
        {
            const bool identification = is_identification_read(
                read->service, read->identifier);
            item->kind = identification
                ? MBLINK_MERCEDES_ECU_DATA_IDENTIFICATION
                : MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ;
            item->status = MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
            item->provenance = mblink_mercedes_documented_route_source();
            /*
             * Retain every safe exact-controller read in the canonical pack.
             * The explicit acquisition field then routes each reviewed item to
             * startup-once or user-polling without losing source coverage.
             */
            item->advertised =
                mblink_mercedes_documented_read_is_safe(
                    read->service, read->identifier);
        }
        item->field_count =
            mblink_mercedes_documented_field_count(
                read->service, read->identifier);
        classify_item_acquisition(pack, item);
        return true;
    }

    return false;
}

bool mblink_mercedes_ecu_pack_next_item(
    const MblinkMercedesEcuPack *pack,
    MblinkMercedesEcuDataAcquisition acquisition,
    size_t *cursor,
    MblinkMercedesEcuDataItem *item)
{
    const size_t count = mblink_mercedes_ecu_pack_data_item_count(pack);
    size_t index;

    if (pack == NULL || cursor == NULL || item == NULL) return false;
    for (index = *cursor; index < count; ++index) {
        MblinkMercedesEcuDataItem candidate;
        *cursor = index + 1U;
        if (!mblink_mercedes_ecu_pack_data_item_at(
                pack, index, &candidate)) {
            continue;
        }
        if (candidate.acquisition != acquisition ||
            !candidate.advertised) {
            continue;
        }
        *item = candidate;
        return true;
    }
    *cursor = count;
    clear_item(item);
    return false;
}

const MblinkMercedesDocumentedField *mblink_mercedes_ecu_pack_field_at(
    const MblinkMercedesEcuDataItem *item,
    size_t index)
{
    if (item == NULL || index >= item->field_count) return NULL;
    return mblink_mercedes_documented_field_at(
        item->service, item->identifier, index);
}

const char *mblink_mercedes_ecu_data_kind_name(
    MblinkMercedesEcuDataKind kind)
{
    switch (kind) {
    case MBLINK_MERCEDES_ECU_DATA_IDENTIFICATION: return "identification";
    case MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE: return "live-value";
    case MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ: return "documented-read";
    case MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED: return "raw-observed";
    }
    return "unknown";
}
