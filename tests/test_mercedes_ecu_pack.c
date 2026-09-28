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

    CHECK(mblink_mercedes_ecu_pack_data_item_count(&pack) == 9U);
    for (size_t index = 0U;
         index < mblink_mercedes_ecu_pack_data_item_count(&pack);
         ++index) {
        CHECK(mblink_mercedes_ecu_pack_data_item_at(&pack, index, &item));
        if (item.advertised) ++advertised;
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
    CHECK(advertised == 9U);
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
    CHECK(canonical_2130 == 7U);
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
        if (item.identifier == UINT16_C(0x2003) &&
            item.kind == MBLINK_MERCEDES_ECU_DATA_RAW_OBSERVED) {
            saw_raw = true;
            CHECK(!item.advertised);
        }
    }
    CHECK(saw_raw);
    return 0;
}

static int test_20260928_capture_routes_under_new_engine(void)
{
    static const struct {
        uint32_t tx;
        uint32_t rx;
        uint8_t expected_mask;
    } routes[] = {
        { UINT32_C(0x60a), UINT32_C(0x481),
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
    if (test_20260928_capture_routes_under_new_engine() != 0) return 1;
    puts("Mercedes ECU definition pack tests passed");
    return 0;
}
