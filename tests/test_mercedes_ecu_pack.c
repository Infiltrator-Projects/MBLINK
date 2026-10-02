// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_ecu_pack.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static bool pack_section_contains(
    const MblinkMercedesEcuPack *pack,
    bool startup,
    uint8_t service,
    uint16_t identifier)
{
    size_t cursor = 0U;
    MblinkMercedesEcuDataItem item;
    const MblinkMercedesEcuDataAcquisition acquisition = startup
        ? MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE
        : MBLINK_MERCEDES_ECU_DATA_USER_POLLING;

    while (mblink_mercedes_ecu_pack_next_item(
            pack, acquisition, &cursor, &item)) {
        if (item.service == service && item.identifier == identifier)
            return true;
    }
    return false;
}

static int test_ic204_pack(void)
{
    MblinkMercedesModuleScanEntry module;
    MblinkMercedesEcuPack pack;
    MblinkMercedesEcuDataItem item;
    bool saw_f111 = false;
    bool saw_f150 = false;
    bool saw_assyst_daily = false;
    bool saw_assyst_overfill = false;
    bool saw_assyst_maintenance = false;
    size_t advertised = 0U;

    /*
     * 0x60A -> 0x481 is not a protocol identity by itself: the global
     * catalogue contains UDS IC_204-family clusters and KWP2000 KI221 on the
     * same physical route.  Preserve the observed protocol until ECU identity
     * resolves a family.
     */
    CHECK(mblink_mercedes_ecu_pack_route_protocol_mask(
              UINT32_C(0x60a), UINT32_C(0x481), false) ==
          (MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
           MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK));
    CHECK(mblink_mercedes_module_scan_resolve_controller(
        UINT32_C(0x60a), UINT32_C(0x481), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        NULL, NULL, NULL, NULL, &module));
    CHECK(mblink_mercedes_module_scan_entry_protocol(&module) ==
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
    CHECK(module.controller_family == NULL);

    /*
     * Once IC_204 identity is known, its pack owns communications.  Even a
     * stale/wrong KWP observation is corrected to the pack's documented UDS.
     */
    CHECK(mblink_mercedes_module_scan_resolve_controller(
        UINT32_C(0x60a), UINT32_C(0x481), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        "IC_204", NULL, NULL, NULL, &module));
    CHECK(module.controller_family != NULL);
    CHECK(strcmp(module.controller_family->key, "cluster-ic204") == 0);
    CHECK(mblink_mercedes_module_scan_entry_protocol(&module) ==
          MBLINK_MERCEDES_DIAGNOSTIC_UDS);

    CHECK(mblink_mercedes_ecu_pack_resolve(
        "instrument-cluster", "cluster-ic204",
        UINT32_C(0x60a), UINT32_C(0x481), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));
    CHECK(pack.protocol_authoritative);
    CHECK(pack.observed_protocol_conflict);
    CHECK(pack.protocol == MBLINK_MERCEDES_DIAGNOSTIC_UDS);

    CHECK(mblink_mercedes_ecu_pack_resolve_module(&module, &pack));
    CHECK(strcmp(pack.key, "cluster-ic204") == 0);
    CHECK(strcmp(pack.ecu_name, "IC_204") == 0);
    CHECK(strcmp(pack.display_name, "IC_204 instrument cluster") == 0);
    CHECK(strcmp(pack.component_designation, "A1") == 0);
    CHECK(pack.tx_can_id == UINT32_C(0x60a));
    CHECK(pack.rx_can_id == UINT32_C(0x481));
    CHECK(pack.protocol == MBLINK_MERCEDES_DIAGNOSTIC_UDS);
    CHECK(pack.documented_profile != NULL);
    CHECK(pack.documented_profile->read_count == 9U);
    CHECK(mblink_mercedes_ecu_pack_alias_count(&pack) >= 2U);

    /*
     * Exact W204 IC_204.cbf contributes 50 unique service+identifier Data
     * reads after variant duplicates are collapsed. The global exact profile
     * contributes nine additional documented safe reads.
     */
    CHECK(mblink_mercedes_ecu_pack_data_item_count(&pack) == 59U);
    for (size_t index = 0U;
         index < mblink_mercedes_ecu_pack_data_item_count(&pack);
         ++index) {
        CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
        if (item.advertised) {
            CHECK(item.status ==
                MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
            ++advertised;
        } else {
        }
        if (item.name != NULL) {
            CHECK(strchr(item.name, '_') == NULL);
        }
        if (item.service == UINT8_C(0x22) &&
            item.identifier == UINT16_C(0x0302)) {
            saw_assyst_daily = true;
            CHECK(item.name != NULL);
            CHECK(strcmp(item.name, "Average daily distance") == 0);
            CHECK(item.acquisition ==
                MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE);
        }
        if (item.service == UINT8_C(0x22) &&
            item.identifier == UINT16_C(0x0406)) {
            saw_assyst_overfill = true;
            CHECK(item.name != NULL);
            CHECK(strcmp(item.name,
                "ASSYST oil overfill threshold 1") == 0);
        }
        if (item.service == UINT8_C(0x22) &&
            item.identifier == UINT16_C(0x0408)) {
            saw_assyst_maintenance = true;
            CHECK(item.name != NULL);
            CHECK(strcmp(item.name,
                "ASSYST maintenance 1 hex dump data") == 0);
        }
        if (item.service == UINT8_C(0x22) &&
            item.identifier == UINT16_C(0xf111)) {
            saw_f111 = true;
            CHECK(item.name != NULL);
            CHECK(strstr(item.name, "hardware") != NULL ||
                  strstr(item.name, "Hardware") != NULL);
            CHECK(item.kind == MBLINK_MERCEDES_ECU_DATA_IDENTIFICATION);
        }
        if (item.service == UINT8_C(0x22) &&
            item.identifier == UINT16_C(0xf150)) {
            const MblinkMercedesDocumentedField *field;
            saw_f150 = true;
            CHECK(item.field_count == 3U);
            field = mblink_mercedes_ecu_pack_field_at(&item, 0U);
            CHECK(field != NULL);
            CHECK(strcmp(field->name, "Hardware version year") == 0);
        }
    }
    CHECK(advertised == 59U);
    CHECK(saw_f111 && saw_f150);
    CHECK(saw_assyst_daily && saw_assyst_overfill &&
          saw_assyst_maintenance);
    CHECK(pack_section_contains(
        &pack, true, UINT8_C(0x22), UINT16_C(0x0302)));
    CHECK(!pack_section_contains(
        &pack, false, UINT8_C(0x22), UINT16_C(0x0302)));
    return 0;
}

static int test_egs53_pack(void)
{
    MblinkMercedesModuleScanEntry module;
    MblinkMercedesEcuPack pack;
    MblinkMercedesEcuDataItem item;
    size_t canonical_2130 = 0U;

    CHECK(mblink_mercedes_module_scan_resolve_controller(
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        NULL, "0034464310", NULL, NULL, &module));
    CHECK(module.controller_family != NULL);
    CHECK(strcmp(module.controller_family->key, "transmission-egs53") == 0);
    CHECK(mblink_mercedes_ecu_pack_resolve_module(&module, &pack));
    CHECK(strcmp(pack.key, "transmission-egs53") == 0);
    CHECK(strcmp(pack.ecu_name, "EGS53") == 0);
    CHECK(pack.protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);

    for (size_t index = 0U;
         index < mblink_mercedes_ecu_pack_data_item_count(&pack);
         ++index) {
        CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
        if (item.service == UINT8_C(0x21) &&
            item.identifier == UINT16_C(0x30) &&
            item.stable_key != NULL) {
            ++canonical_2130;
            CHECK(item.advertised);
            CHECK(item.allow_duplicate_wire);
        }
    }
    CHECK(canonical_2130 == 13U);
    return 0;
}

static int test_raw_observation_stays_unadvertised(void)
{
    MblinkMercedesEcuPack pack;
    MblinkMercedesEcuDataItem item;
    bool saw_raw = false;

    CHECK(mblink_mercedes_ecu_pack_resolve(
        "esp", "esp-abr2xt",
        UINT32_C(0x632), UINT32_C(0x486), false,
        MBLINK_MERCEDES_DIAGNOSTIC_UDS, &pack));

    for (size_t index = 0U;
         index < mblink_mercedes_ecu_pack_data_item_count(&pack);
         ++index) {
        CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
        /*
         * 0x2003 is no longer raw: ABR2XT.cbf now documents it. 0x20DF
         * remains capture-only evidence and therefore must stay unadvertised.
         */
        if (item.identifier == UINT16_C(0x20df) &&
            item.kind == MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED) {
            saw_raw = true;
            CHECK(!item.advertised);
        }
    }
    CHECK(saw_raw);
    return 0;
}

static int test_orc204_and_orc212_keep_separate_exact_catalogues(void)
{
    static const struct {
        const char *controller_key;
        const char *identity;
        const char *profile_name;
    } cases[] = {
        { "restraints-orc204", "ORC_204", "ORC_204" },
        { "restraints-orc212", "ORC_212", "ORC_212_X" }
    };

    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        MblinkMercedesModuleScanEntry module;
        MblinkMercedesEcuPack pack;
        size_t classified = 0U;

        CHECK(mblink_mercedes_module_scan_resolve_controller(
            UINT32_C(0x64a), UINT32_C(0x489), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
            cases[case_index].identity, NULL, NULL, NULL, &module));
        CHECK(module.controller_family != NULL);
        CHECK(strcmp(module.controller_family->key,
                     cases[case_index].controller_key) == 0);
        CHECK(mblink_mercedes_ecu_pack_resolve_module(&module, &pack));
        CHECK(pack.documented_profile != NULL);
        CHECK(strcmp(pack.documented_profile->name,
                     cases[case_index].profile_name) == 0);
        CHECK(mblink_mercedes_documented_ecu_read_count(
                  pack.documented_profile) == 5U);

        for (size_t item_index = 0U;
             item_index < mblink_mercedes_ecu_pack_data_item_count(&pack);
             ++item_index) {
            MblinkMercedesEcuDataItem item;
            CHECK(mblink_mercedes_ecu_pack_data_item_at(
                &pack, item_index, &item));
            if (item.status ==
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED &&
                item.advertised &&
                (item.acquisition ==
                     MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE ||
                 item.acquisition ==
                     MBLINK_MERCEDES_ECU_DATA_USER_POLLING)) {
                ++classified;
            }
        }
        CHECK(classified >= 5U);
    }
    return 0;
}

static int test_orc_startup_names_are_specific(void)
{
    static const char *controllers[] = {
        "restraints-orc204",
        "restraints-orc212"
    };

    for (size_t controller_index = 0U;
         controller_index < sizeof(controllers) / sizeof(controllers[0]);
         ++controller_index) {
        MblinkMercedesEcuPack pack;
        bool saw_configuration = false;
        bool saw_lock_state = false;

        CHECK(mblink_mercedes_ecu_pack_resolve(
            NULL, controllers[controller_index],
            UINT32_C(0x64a), UINT32_C(0x489), false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));

        size_t cursor = 0U;
        MblinkMercedesEcuDataItem item;
        while (mblink_mercedes_ecu_pack_next_item(
                &pack, MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
                &cursor, &item)) {
            if (item.service != UINT8_C(0x21) || item.name == NULL) continue;
            if (item.identifier == UINT16_C(0x02)) {
                saw_configuration = true;
                CHECK(strcmp(
                    item.name,
                    "Restraint equipment configuration") == 0);
            } else if (item.identifier == UINT16_C(0x58)) {
                saw_lock_state = true;
                CHECK(strcmp(
                    item.name,
                    "ECU lock state / tester identification") == 0);
            }
        }
        CHECK(saw_configuration);
        CHECK(saw_lock_state);
    }
    return 0;
}

static int test_documented_reads_are_classified_by_acquisition(void)
{
    static const struct {
        const char *controller_key;
        uint32_t tx;
        uint32_t rx;
        uint16_t identifier;
        bool startup;
    } cases[] = {
        { "restraints-orc212", UINT32_C(0x64a), UINT32_C(0x489),
          UINT16_C(0x58), true },
        { "headunit-hu204", UINT32_C(0x652), UINT32_C(0x48a),
          UINT16_C(0xe1), false }
    };

    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        MblinkMercedesEcuPack pack;
        bool found = false;

        CHECK(mblink_mercedes_ecu_pack_resolve(
            NULL, cases[case_index].controller_key,
            cases[case_index].tx, cases[case_index].rx, false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));

        size_t cursor = 0U;
        MblinkMercedesEcuDataItem item;
        const MblinkMercedesEcuDataAcquisition acquisition =
            cases[case_index].startup
                ? MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE
                : MBLINK_MERCEDES_ECU_DATA_USER_POLLING;
        while (mblink_mercedes_ecu_pack_next_item(
                &pack, acquisition, &cursor, &item)) {
            if (item.service == UINT8_C(0x21) &&
                item.identifier == cases[case_index].identifier) {
                found = true;
                CHECK(item.status ==
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
                CHECK(item.advertised);
            }
        }
        CHECK(found);
    }
    return 0;
}

static int test_documented_identification_reads_are_startup_data(void)
{
    MblinkMercedesEcuPack pack;
    bool saw_identity = false;

    CHECK(mblink_mercedes_ecu_pack_resolve(
        NULL, "headunit-hu204",
        UINT32_C(0x652), UINT32_C(0x48a), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));

    size_t cursor = 0U;
    MblinkMercedesEcuDataItem item;
    while (mblink_mercedes_ecu_pack_next_item(
            &pack, MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE,
            &cursor, &item)) {
        if (item.service == UINT8_C(0x1a) &&
            item.identifier == UINT16_C(0x87) &&
            item.status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
            saw_identity = true;
            CHECK(item.kind == MBLINK_MERCEDES_ECU_DATA_IDENTIFICATION);
            CHECK(item.acquisition == MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE);
            CHECK(item.acquired_during_identification);
        }
    }
    CHECK(saw_identity);
    return 0;
}

static int test_documented_polling_controller_data_stays_selectable(void)
{
    static const struct {
        const char *module_key;
        const char *controller_key;
        uint32_t tx;
        uint32_t rx;
        uint16_t identifier;
    } cases[] = {
        { "gateway", "gateway-cgw212",
          UINT32_C(0x602), UINT32_C(0x480), UINT16_C(0x0310) },
        { "camera", "camera-mfk",
          UINT32_C(0x6a2), UINT32_C(0x494), UINT16_C(0x0201) },
        { "camera", "camera-mfk",
          UINT32_C(0x6a2), UINT32_C(0x494), UINT16_C(0x0254) },
        { "fuel-pump", "fuel-pump-fscu",
          UINT32_C(0x6fa), UINT32_C(0x49f), UINT16_C(0x001c) }
    };

    /*
     * These individually reviewed controller Data services remain on the
     * user-polling side. Static items move only when their semantics have been
     * established; one quiet capture must never reclassify a documented read.
     */
    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        MblinkMercedesEcuPack pack;
        bool found = false;

        CHECK(mblink_mercedes_ecu_pack_resolve(
            cases[case_index].module_key,
            cases[case_index].controller_key,
            cases[case_index].tx, cases[case_index].rx, false,
            MBLINK_MERCEDES_DIAGNOSTIC_UDS, &pack));

        size_t cursor = 0U;
        MblinkMercedesEcuDataItem item;
        while (mblink_mercedes_ecu_pack_next_item(
                &pack, MBLINK_MERCEDES_ECU_DATA_USER_POLLING,
                &cursor, &item)) {
            if (item.service == UINT8_C(0x22) &&
                item.identifier == cases[case_index].identifier &&
                item.status ==
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
                found = true;
                CHECK(item.acquisition ==
                    MBLINK_MERCEDES_ECU_DATA_USER_POLLING);
                CHECK(item.advertised);
            }
        }
        CHECK(found);
    }
    return 0;
}

static int test_documented_pid_catalogue_does_not_depend_on_vehicle_response(void)
{
    static const struct {
        const char *module_key;
        const char *controller_key;
        uint32_t tx;
        uint32_t rx;
        uint16_t expected_identifier;
    } cases[] = {
        { "gateway", "gateway-cgw212",
          UINT32_C(0x602), UINT32_C(0x480), UINT16_C(0xd243) },
        { "camera", "camera-mfk",
          UINT32_C(0x6a2), UINT32_C(0x494), UINT16_C(0x0220) },
        { "fuel-pump", "fuel-pump-fscu",
          UINT32_C(0x6fa), UINT32_C(0x49f), UINT16_C(0x000b) }
    };

    /*
     * No vehicle response is supplied to this test. Resolving the exact ECU
     * definition alone must publish its documented PIDs, just as standard OBD
     * PID definitions exist independently of a particular car's response.
     */
    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        MblinkMercedesEcuPack pack;
        MblinkMercedesEcuDataItem item;
        bool found = false;

        CHECK(mblink_mercedes_ecu_pack_resolve(
            cases[case_index].module_key,
            cases[case_index].controller_key,
            cases[case_index].tx, cases[case_index].rx, false,
            MBLINK_MERCEDES_DIAGNOSTIC_UDS, &pack));

        for (size_t index = 0U;
             index < mblink_mercedes_ecu_pack_data_item_count(&pack);
             ++index) {
            CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
            if (item.service == UINT8_C(0x22) &&
                item.identifier == cases[case_index].expected_identifier &&
                item.status ==
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
                found = true;
                CHECK(item.kind == MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE);
                CHECK(item.advertised);
            }
        }
        CHECK(found);
    }
    return 0;
}

static int test_generated_cbf_controller_pid_catalogues(void)
{
    static const struct {
        const char *profile_key;
        MblinkMercedesDiagnosticProtocol protocol;
        size_t expected_count;
        uint8_t sample_service;
        uint16_t sample_identifier;
    } cases[] = {
        { "gateway-cgw204", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          11U, UINT8_C(0x22), UINT16_C(0x0026) },
        { "eis-ezs204", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          9U, UINT8_C(0x22), UINT16_C(0x0228) },
        { "cluster-ic204", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          50U, UINT8_C(0x22), UINT16_C(0x0402) },
        { "steering-sccm204", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          4U, UINT8_C(0x22), UINT16_C(0x0163) },
        { "restraints-orc204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
          27U, UINT8_C(0x21), UINT16_C(0x0001) },
        /* 24 HU_204.cbf reads plus captured raw 0x02. */
        { "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
          25U, UINT8_C(0x21), UINT16_C(0x0006) },
        { "fuel-pump-fscu", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          6U, UINT8_C(0x22), UINT16_C(0x000a) },
        /*
         * ABR2XT has 18 exact CBF reads plus seven unique vehicle-observed raw
         * identifiers retained in the same controller profile.
         */
        { "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          25U, UINT8_C(0x22), UINT16_C(0x2001) },
        { "gateway-cgw212", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          11U, UINT8_C(0x22), UINT16_C(0xd243) },
        { "camera-mfk", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          16U, UINT8_C(0x22), UINT16_C(0x0220) },
        { "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
          5U, UINT8_C(0x22), UINT16_C(0x1001) }
    };

    for (size_t index = 0U;
         index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry;
        CHECK(mblink_mercedes_controller_data_profile_identifier_count(
                  cases[index].profile_key,
                  cases[index].protocol) ==
              cases[index].expected_count);
        entry = mblink_mercedes_controller_data_profile_find_service(
            cases[index].profile_key,
            cases[index].protocol,
            cases[index].sample_service,
            cases[index].sample_identifier);
        CHECK(entry != NULL);
        CHECK(entry->service == cases[index].sample_service);
        CHECK(entry->status ==
            MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
    }
    return 0;
}

static int test_exact_204_controller_family_resolution(void)
{
    const char *profile;

    profile = mblink_mercedes_data_profile_key_for_controller(
        "eis-ezs", "EIS_204", NULL, NULL);
    CHECK(profile != NULL);
    CHECK(strcmp(profile, "eis-ezs204") == 0);

    profile = mblink_mercedes_data_profile_key_for_controller(
        "central-gateway", "CGW_204", NULL, NULL);
    CHECK(profile != NULL);
    CHECK(strcmp(profile, "gateway-cgw204") == 0);

    profile = mblink_mercedes_data_profile_key_for_controller(
        "steering-column", "SCCM_204", NULL, NULL);
    CHECK(profile != NULL);
    CHECK(strcmp(profile, "steering-sccm204") == 0);

    profile = mblink_mercedes_data_profile_key_for_controller(
        "instrument-cluster", "IC_204", NULL, NULL);
    CHECK(profile != NULL);
    CHECK(strcmp(profile, "cluster-ic204") == 0);
    return 0;
}

static int test_all_c207_exact_profile_reads_reach_one_pack_section(void)
{
    static const struct {
        const char *controller_key;
        uint32_t tx;
        uint32_t rx;
        MblinkMercedesDiagnosticProtocol protocol;
        size_t expected_profile_reads;
    } cases[] = {
        { "gateway-cgw204", UINT32_C(0x602), UINT32_C(0x480),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 9U },
        { "cluster-ic204", UINT32_C(0x60a), UINT32_C(0x481),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 9U },
        { "eis-ezs204", UINT32_C(0x612), UINT32_C(0x482),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 9U },
        { "steering-sccm204", UINT32_C(0x622), UINT32_C(0x484),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 2U },
        { "esp-abr2xt", UINT32_C(0x632), UINT32_C(0x486),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 8U },
        { "restraints-orc204", UINT32_C(0x64a), UINT32_C(0x489),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, 5U },
        { "restraints-orc212", UINT32_C(0x64a), UINT32_C(0x489),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, 5U },
        { "headunit-hu204", UINT32_C(0x652), UINT32_C(0x48a),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, 8U },
        { "camera-mfk", UINT32_C(0x6a2), UINT32_C(0x494),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 9U },
        { "pretensioner-rbtmfl204", UINT32_C(0x6ba), UINT32_C(0x497),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 9U },
        { "pretensioner-rbtmfr204", UINT32_C(0x6c2), UINT32_C(0x498),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 9U },
        { "fuel-pump-fscu", UINT32_C(0x6fa), UINT32_C(0x49f),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 10U },
        { "engine-crd3", UINT32_C(0x7e0), UINT32_C(0x7e8),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS, 6U },
        { "transmission-egs53", UINT32_C(0x7e1), UINT32_C(0x7e9),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, 4U }
    };

    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        MblinkMercedesEcuPack pack;
        CHECK(mblink_mercedes_ecu_pack_resolve(
            NULL, cases[case_index].controller_key,
            cases[case_index].tx, cases[case_index].rx, false,
            cases[case_index].protocol, &pack));
        CHECK(pack.documented_profile != NULL);
        CHECK(mblink_mercedes_documented_ecu_read_count(
                  pack.documented_profile) ==
              cases[case_index].expected_profile_reads);

        for (size_t read_index = 0U;
             read_index < mblink_mercedes_documented_ecu_read_count(
                 pack.documented_profile);
             ++read_index) {
            const MblinkMercedesDocumentedRead *read =
                mblink_mercedes_documented_ecu_read_at(
                    pack.documented_profile, read_index);
            bool found = false;
            CHECK(read != NULL);
            CHECK(mblink_mercedes_documented_read_is_safe(
                read->service, read->identifier));

            for (size_t item_index = 0U;
                 item_index < mblink_mercedes_ecu_pack_data_item_count(&pack);
                 ++item_index) {
                MblinkMercedesEcuDataItem item;
                CHECK(mblink_mercedes_ecu_pack_data_item_at(
                    &pack, item_index, &item));
                if (item.service == read->service &&
                    item.identifier == read->identifier &&
                    item.status ==
                        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
                    CHECK(item.acquisition ==
                              MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE ||
                          item.acquisition ==
                              MBLINK_MERCEDES_ECU_DATA_USER_POLLING);
                    found = true;
                    break;
                }
            }
            CHECK(found);
        }
    }
    return 0;
}

static int test_all_c207_source_backed_controller_data_is_classified(void)
{
    static const struct {
        const char *controller_key;
        uint32_t tx;
        uint32_t rx;
        MblinkMercedesDiagnosticProtocol protocol;
    } cases[] = {
        { "gateway-cgw204", UINT32_C(0x602), UINT32_C(0x480),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "cluster-ic204", UINT32_C(0x60a), UINT32_C(0x481),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "eis-ezs204", UINT32_C(0x612), UINT32_C(0x482),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "steering-sccm204", UINT32_C(0x622), UINT32_C(0x484),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "esp-abr2xt", UINT32_C(0x632), UINT32_C(0x486),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "restraints-orc204", UINT32_C(0x64a), UINT32_C(0x489),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 },
        { "restraints-orc212", UINT32_C(0x64a), UINT32_C(0x489),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 },
        { "headunit-hu204", UINT32_C(0x652), UINT32_C(0x48a),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 },
        { "camera-mfk", UINT32_C(0x6a2), UINT32_C(0x494),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "pretensioner-rbtmfl204", UINT32_C(0x6ba), UINT32_C(0x497),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "pretensioner-rbtmfr204", UINT32_C(0x6c2), UINT32_C(0x498),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "fuel-pump-fscu", UINT32_C(0x6fa), UINT32_C(0x49f),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "engine-crd3", UINT32_C(0x7e0), UINT32_C(0x7e8),
          MBLINK_MERCEDES_DIAGNOSTIC_UDS },
        { "transmission-egs53", UINT32_C(0x7e1), UINT32_C(0x7e9),
          MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 }
    };

    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]); ++case_index) {
        MblinkMercedesEcuPack pack;
        size_t profile_count;
        CHECK(mblink_mercedes_ecu_pack_resolve(
            NULL, cases[case_index].controller_key,
            cases[case_index].tx, cases[case_index].rx, false,
            cases[case_index].protocol, &pack));

        profile_count = mblink_mercedes_controller_data_profile_identifier_count(
            pack.controller_data_profile_key, pack.protocol);
        for (size_t profile_index = 0U;
             profile_index < profile_count; ++profile_index) {
            const MblinkMercedesControllerDataProfileEntry *entry =
                mblink_mercedes_controller_data_profile_identifier_at(
                    pack.controller_data_profile_key,
                    pack.protocol, profile_index);
            bool found = false;

            CHECK(entry != NULL);
            if (entry->status !=
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
                continue;
            }

            for (size_t item_index = 0U;
                 item_index < mblink_mercedes_ecu_pack_data_item_count(&pack);
                 ++item_index) {
                MblinkMercedesEcuDataItem item;
                CHECK(mblink_mercedes_ecu_pack_data_item_at(
                    &pack, item_index, &item));
                if (item.service == entry->service &&
                    item.identifier == entry->identifier &&
                    item.status ==
                        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
                    CHECK(item.acquisition ==
                              MBLINK_MERCEDES_ECU_DATA_STARTUP_ONCE ||
                          item.acquisition ==
                              MBLINK_MERCEDES_ECU_DATA_USER_POLLING);
                    found = true;
                    break;
                }
            }
            CHECK(found);
        }
    }
    return 0;
}

static int test_startup_and_polling_sections_are_separate(void)
{
    MblinkMercedesEcuPack pack;

    CHECK(mblink_mercedes_ecu_pack_resolve(
        NULL, "restraints-orc212",
        UINT32_C(0x64a), UINT32_C(0x489), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));
    CHECK(pack_section_contains(
        &pack, true, UINT8_C(0x21), UINT16_C(0x0002)));
    CHECK(pack_section_contains(
        &pack, true, UINT8_C(0x21), UINT16_C(0x0058)));
    CHECK(!pack_section_contains(
        &pack, false, UINT8_C(0x21), UINT16_C(0x0002)));
    CHECK(!pack_section_contains(
        &pack, false, UINT8_C(0x21), UINT16_C(0x0058)));

    CHECK(mblink_mercedes_ecu_pack_resolve(
        NULL, "transmission-egs53",
        UINT32_C(0x7e1), UINT32_C(0x7e9), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));
    CHECK(pack_section_contains(
        &pack, true, UINT8_C(0x21), UINT16_C(0x00b1)));
    CHECK(!pack_section_contains(
        &pack, false, UINT8_C(0x21), UINT16_C(0x00b1)));
    CHECK(pack_section_contains(
        &pack, false, UINT8_C(0x21), UINT16_C(0x0030)));

    CHECK(mblink_mercedes_ecu_pack_resolve(
        NULL, "gateway-cgw204",
        UINT32_C(0x602), UINT32_C(0x480), false,
        MBLINK_MERCEDES_DIAGNOSTIC_UDS, &pack));
    CHECK(pack_section_contains(
        &pack, true, UINT8_C(0x22), UINT16_C(0xf153)));
    CHECK(!pack_section_contains(
        &pack, false, UINT8_C(0x22), UINT16_C(0xf153)));

    return 0;
}

static int test_20260928_capture_routes_under_new_engine(void)
{
    static const struct {
        uint32_t tx;
        uint32_t rx;
        uint8_t expected_mask;
    } routes[] = {
        { UINT32_C(0x602), UINT32_C(0x480),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x60a), UINT32_C(0x481),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x612), UINT32_C(0x482),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x622), UINT32_C(0x484),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x632), UINT32_C(0x486),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x64a), UINT32_C(0x489),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x652), UINT32_C(0x48a),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x6a2), UINT32_C(0x494),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK },
        { UINT32_C(0x6ba), UINT32_C(0x497),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x6c2), UINT32_C(0x498),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x6fa), UINT32_C(0x49f),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x7e0), UINT32_C(0x7e8),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK },
        { UINT32_C(0x7e1), UINT32_C(0x7e9),
          MBLINK_MERCEDES_ECU_PROTOCOL_UDS_MASK |
          MBLINK_MERCEDES_ECU_PROTOCOL_KWP2000_MASK }
    };

    /*
     * 0.7.249 captured these exact C207 routes. Some old route labels selected
     * KWP before the ECU family was known. The pack engine must instead expose
     * every documented protocol variant for reused addresses, while the MFK
     * route is unambiguously UDS. This fixture intentionally stores no VIN or
     * private vehicle identity.
     */
    for (size_t index = 0U; index < sizeof(routes) / sizeof(routes[0]); ++index) {
        CHECK(mblink_mercedes_ecu_pack_route_protocol_mask(
                  routes[index].tx, routes[index].rx, false) ==
              routes[index].expected_mask);
    }
    return 0;
}

int main(void)
{
    if (test_ic204_pack() != 0) return 1;
    if (test_egs53_pack() != 0) return 1;
    if (test_raw_observation_stays_unadvertised() != 0) return 1;
    if (test_orc204_and_orc212_keep_separate_exact_catalogues() != 0) return 1;
    if (test_orc_startup_names_are_specific() != 0) return 1;
    if (test_documented_reads_are_classified_by_acquisition() != 0) return 1;
    if (test_documented_identification_reads_are_startup_data() != 0) return 1;
    if (test_documented_polling_controller_data_stays_selectable() != 0) return 1;
    if (test_documented_pid_catalogue_does_not_depend_on_vehicle_response() != 0) return 1;
    if (test_generated_cbf_controller_pid_catalogues() != 0) return 1;
    if (test_exact_204_controller_family_resolution() != 0) return 1;
    if (test_all_c207_exact_profile_reads_reach_one_pack_section() != 0) return 1;
    if (test_all_c207_source_backed_controller_data_is_classified() != 0) return 1;
    if (test_startup_and_polling_sections_are_separate() != 0) return 1;
    if (test_20260928_capture_routes_under_new_engine() != 0) return 1;
    puts("Mercedes ECU definition pack tests passed");
    return 0;
}
