// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_ecu_pack.h"

#include "mblink/mercedes_transmission.h"

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

static bool pack_is_orc_controller(const MblinkMercedesEcuPack *pack)
{
    const char *key = pack != NULL && pack->controller_family != NULL
        ? pack->controller_family->key : NULL;
    return key != NULL &&
        (strcmp(key, "restraints-orc204") == 0 ||
         strcmp(key, "restraints-orc212") == 0);
}

static bool pack_item_is_startup_once(
    const MblinkMercedesEcuPack *pack,
    uint8_t service,
    uint16_t identifier)
{
    if (mblink_mercedes_documented_read_is_module_metadata(
            service, identifier)) {
        return true;
    }
    if (pack_transmission_family(pack) ==
            MBLINK_MERCEDES_TRANSMISSION_FAMILY_EGS53 &&
        service == UINT8_C(0x21) &&
        identifier == UINT16_C(0x00b1)) {
        return true;
    }
    return pack_is_orc_controller(pack) &&
        service == UINT8_C(0x21) &&
        (identifier == UINT16_C(0x0002) ||
         identifier == UINT16_C(0x0058));
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
    item->acquisition = pack_item_is_startup_once(
        pack, item->service, item->identifier)
        ? MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE
        : MBLINK_MERCEDES_ECU_DATA_USER_POLLING;
    item->acquired_during_identification =
        item->acquisition == MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE &&
        pack_item_acquired_during_identification(
            item->service, item->identifier);
}

static const char *controller_specific_read_name(
    const MblinkMercedesEcuPack *pack,
    uint8_t service,
    uint16_t identifier,
    const char *fallback)
{
    const char *key;

    if (pack == NULL || pack->controller_family == NULL ||
        pack->controller_family->key == NULL ||
        service != UINT8_C(0x21)) {
        return fallback;
    }

    key = pack->controller_family->key;
    if (strcmp(key, "restraints-orc204") != 0 &&
        strcmp(key, "restraints-orc212") != 0) {
        return fallback;
    }

    switch (identifier) {
    case UINT16_C(0x0002):
        return "Restraint equipment configuration";
    case UINT16_C(0x0058):
        return "ECU lock state / tester identification";
    default:
        return fallback;
    }
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
        item->live = true;
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
         * PID Setup is currently the complete documented read catalogue.
         * Do not hide a valid family-owned KWP record merely because an older
         * presentation layer classified it as static/non-live.
         */
        item->live = true;
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
        item->name = controller_specific_read_name(
            pack, entry->service, entry->identifier, entry->name);
        if (entry->status ==
                MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
            item->kind = entry->live
                ? MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE
                : MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ;
        } else {
            item->kind = MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED;
        }
        item->status = entry->status;
        item->provenance = entry->provenance;
        /*
         * At this stage PID Setup is the complete source-backed controller
         * read catalogue. Historical live/static classification must not
         * suppress a documented Data service. Capture-only observations remain
         * excluded because they do not establish semantics.
         */
        item->live =
            entry->status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED;
        item->advertised = item->live;
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
        item->name = controller_specific_read_name(
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
             * Catalogue-completion phase: every safe read documented for the
             * exact controller profile is selectable in PID Setup. Keep the
             * semantic kind (identification vs documented-read) so a later UI
             * pass can group/curate without losing catalogue completeness.
             */
            item->live =
                mblink_mercedes_documented_read_is_safe(
                    read->service, read->identifier);
            item->advertised = item->live;
        }
        item->field_count =
            mblink_mercedes_documented_field_count(
                read->service, read->identifier);
        classify_item_acquisition(pack, item);
        return true;
    }

    return false;
}

static bool data_item_at_for_acquisition(
    const MblinkMercedesEcuPack *pack,
    MblinkMercedesEcuDataAcquisition acquisition,
    size_t wanted,
    MblinkMercedesEcuDataItem *item)
{
    size_t seen = 0U;
    const size_t count = mblink_mercedes_ecu_pack_data_item_count(pack);

    if (item == NULL) return false;
    for (size_t index = 0U; index < count; ++index) {
        MblinkMercedesEcuDataItem candidate;
        if (!mblink_mercedes_ecu_pack_data_item_at(
                pack, index, &candidate)) {
            continue;
        }
        if (candidate.acquisition != acquisition) continue;
        if (seen++ == wanted) {
            *item = candidate;
            return true;
        }
    }
    clear_item(item);
    return false;
}

static size_t data_item_count_for_acquisition(
    const MblinkMercedesEcuPack *pack,
    MblinkMercedesEcuDataAcquisition acquisition)
{
    size_t result = 0U;
    const size_t count = mblink_mercedes_ecu_pack_data_item_count(pack);

    for (size_t index = 0U; index < count; ++index) {
        MblinkMercedesEcuDataItem item;
        if (mblink_mercedes_ecu_pack_data_item_at(pack, index, &item) &&
            item.acquisition == acquisition) {
            ++result;
        }
    }
    return result;
}

size_t mblink_mercedes_ecu_pack_startup_item_count(
    const MblinkMercedesEcuPack *pack)
{
    return data_item_count_for_acquisition(
        pack, MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE);
}

bool mblink_mercedes_ecu_pack_startup_item_at(
    const MblinkMercedesEcuPack *pack,
    size_t index,
    MblinkMercedesEcuDataItem *item)
{
    return data_item_at_for_acquisition(
        pack, MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE, index, item);
}

size_t mblink_mercedes_ecu_pack_polling_item_count(
    const MblinkMercedesEcuPack *pack)
{
    return data_item_count_for_acquisition(
        pack, MBLINK_MERCEDES_ECU_DATA_USER_POLLING);
}

bool mblink_mercedes_ecu_pack_polling_item_at(
    const MblinkMercedesEcuPack *pack,
    size_t index,
    MblinkMercedesEcuDataItem *item)
{
    return data_item_at_for_acquisition(
        pack, MBLINK_MERCEDES_ECU_DATA_USER_POLLING, index, item);
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
