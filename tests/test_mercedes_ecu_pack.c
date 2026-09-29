// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_ecu_pack.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_ic204_pack(void)
{
    MblinkMercedesModuleScanEntry module;
    MblinkMercedesEcuPack pack;
    MblinkMercedesEcuDataItem item;
    bool saw_f111 = false;
    bool saw_f150 = false;
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
     * IC_204 owns 62 source-generated CBF Data DIDs plus the nine documented
     * identification/manual reads from the global controller catalogue.
     */
    CHECK(mblink_mercedes_ecu_pack_data_item_count(&pack) == 71U);
    for (size_t index = 0U;
         index < mblink_mercedes_ecu_pack_data_item_count(&pack);
         ++index) {
        CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
        if (item.advertised) {
            CHECK(item.live);
            CHECK(item.status ==
                MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED);
            ++advertised;
        } else {
            CHECK(!item.live);
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
    CHECK(advertised == 71U);
    CHECK(saw_f111 && saw_f150);
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
            CHECK(item.live);
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
        size_t selectable = 0U;

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
                item.advertised && item.live) {
                ++selectable;
            }
        }
        CHECK(selectable >= 5U);
    }
    return 0;
}

static int test_documented_reads_are_selectable_in_pid_catalogue(void)
{
    static const struct {
        const char *controller_key;
        uint32_t tx;
        uint32_t rx;
        uint16_t documented_identifier;
    } cases[] = {
        { "restraints-orc212", UINT32_C(0x64a), UINT32_C(0x489),
          UINT16_C(0x58) },
        { "headunit-hu204", UINT32_C(0x652), UINT32_C(0x48a),
          UINT16_C(0xe1) }
    };

    /*
     * Catalogue-completion phase: a safe read documented for the exact
     * controller belongs in PID Setup even when it was historically described
     * as static/manual data.
     */
    for (size_t case_index = 0U;
         case_index < sizeof(cases) / sizeof(cases[0]);
         ++case_index) {
        MblinkMercedesEcuPack pack;
        MblinkMercedesEcuDataItem item;
        bool saw_documented = false;

        CHECK(mblink_mercedes_ecu_pack_resolve(
            NULL, cases[case_index].controller_key,
            cases[case_index].tx, cases[case_index].rx, false,
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));

        for (size_t index = 0U;
             index < mblink_mercedes_ecu_pack_data_item_count(&pack);
             ++index) {
            CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
            if (item.service == UINT8_C(0x21) &&
                item.identifier == cases[case_index].documented_identifier &&
                item.status ==
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED &&
                item.kind == MBLINK_MERCEDES_ECU_DATA_DOCUMENTED_READ) {
                saw_documented = true;
                CHECK(item.advertised);
                CHECK(item.live);
            }
        }
        CHECK(saw_documented);
    }
    return 0;
}

static int test_documented_identification_reads_are_selectable(void)
{
    MblinkMercedesEcuPack pack;
    MblinkMercedesEcuDataItem item;
    bool saw_identity = false;

    CHECK(mblink_mercedes_ecu_pack_resolve(
        NULL, "headunit-hu204",
        UINT32_C(0x652), UINT32_C(0x48a), false,
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000, &pack));

    for (size_t index = 0U;
         index < mblink_mercedes_ecu_pack_data_item_count(&pack);
         ++index) {
        CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
        if (item.service == UINT8_C(0x1a) &&
            item.identifier == UINT16_C(0x87) &&
            item.status == MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
            saw_identity = true;
            CHECK(item.kind == MBLINK_MERCEDES_ECU_DATA_IDENTIFICATION);
            CHECK(item.advertised);
            CHECK(item.live);
        }
    }
    CHECK(saw_identity);
    return 0;
}

static int test_documented_controller_data_stays_selectable_in_pid_setup(void)
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
     * These source-backed controller Data services are deliberately selectable
     * in PID Setup even when a value changes rarely or appears static in one
     * capture. Monitoring membership comes from the exact controller
     * definition, never from whether this particular vehicle has responded.
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
                item.identifier == cases[case_index].identifier &&
                item.status ==
                    MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED) {
                found = true;
                CHECK(item.kind == MBLINK_MERCEDES_ECU_DATA_LIVE_VALUE);
                CHECK(item.live);
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
        size_t expected_count;
        uint16_t sample_identifier;
    } cases[] = {
        { "eis-ezs204", 9U, UINT16_C(0x0228) },
        /* 18 CBF-defined DIDs plus seven remaining raw observations. */
        { "esp-abr2xt", 25U, UINT16_C(0x2001) },
        { "gateway-cgw212", 11U, UINT16_C(0xd243) },
        { "cluster-ic204", 62U, UINT16_C(0x0001) },
        { "camera-mfk", 16U, UINT16_C(0x0220) },
        { "fuel-pump-fscu", 6U, UINT16_C(0x000b) }
    };

    for (size_t index = 0U;
         index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry;
        CHECK(mblink_mercedes_controller_data_profile_identifier_count(
                  cases[index].profile_key,
                  MBLINK_MERCEDES_DIAGNOSTIC_UDS) ==
              cases[index].expected_count);
        entry = mblink_mercedes_controller_data_profile_find(
            cases[index].profile_key,
            MBLINK_MERCEDES_DIAGNOSTIC_UDS,
            cases[index].sample_identifier);
        CHECK(entry != NULL);
        CHECK(entry->live);
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

static int test_all_c207_exact_profile_reads_reach_pid_setup(void)
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
            bool found_selectable = false;
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
                        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED &&
                    item.live && item.advertised) {
                    found_selectable = true;
                    break;
                }
            }
            CHECK(found_selectable);
        }
    }
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
    if (test_documented_reads_are_selectable_in_pid_catalogue() != 0) return 1;
    if (test_documented_identification_reads_are_selectable() != 0) return 1;
    if (test_documented_controller_data_stays_selectable_in_pid_setup() != 0) return 1;
    if (test_documented_pid_catalogue_does_not_depend_on_vehicle_response() != 0) return 1;
    if (test_generated_cbf_controller_pid_catalogues() != 0) return 1;
    if (test_exact_204_controller_family_resolution() != 0) return 1;
    if (test_all_c207_exact_profile_reads_reach_pid_setup() != 0) return 1;
    if (test_20260928_capture_routes_under_new_engine() != 0) return 1;
    puts("Mercedes ECU definition pack tests passed");
    return 0;
}
