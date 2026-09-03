// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/discover.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int check_kwp_target(
    const link_discover_sweep_plan *plan,
    size_t index,
    uint32_t tx,
    uint32_t rx,
    const uint8_t *vin_probe,
    size_t vin_probe_length)
{
    link_discover_sweep_target target;
    const link_discover_sweep_probe *probes = NULL;
    size_t probe_count = 0U;
    const link_discover_sweep_probe *identity = NULL;
    link_discover_sweep_decode_identity_fn decoder = NULL;

    CHECK(link_discover_sweep_plan_target_at(plan, index, &target));
    CHECK(!target.extended_id);
    CHECK(target.tx_can_id == tx);
    CHECK(target.rx_can_id == rx);
    CHECK(link_discover_sweep_plan_probes_for_target(
              plan, &target, &probes, &probe_count, &identity, &decoder));
    CHECK(probe_count == (vin_probe != NULL ? 3U : 2U));
    CHECK(probes[0].payload_length == 2U);
    CHECK(probes[0].payload[0] == UINT8_C(0x3e));
    CHECK(probes[0].payload[1] == UINT8_C(0x01));
    CHECK(probes[1].payload_length == 4U);
    CHECK(probes[1].payload[0] == UINT8_C(0x18));
    if (vin_probe != NULL) {
        CHECK(probes[2].payload_length == vin_probe_length);
        CHECK(memcmp(probes[2].payload, vin_probe, vin_probe_length) == 0);
    }
    CHECK(identity == NULL);
    CHECK(decoder == NULL);
    return 0;
}

static int check_uds_f1a0_target(
    const link_discover_sweep_plan *plan,
    size_t index,
    uint32_t tx,
    uint32_t rx)
{
    link_discover_sweep_target target;
    const link_discover_sweep_probe *probes = NULL;
    size_t probe_count = 0U;
    const link_discover_sweep_probe *identity = NULL;
    link_discover_sweep_decode_identity_fn decoder = NULL;

    CHECK(link_discover_sweep_plan_target_at(plan, index, &target));
    CHECK(!target.extended_id);
    CHECK(target.tx_can_id == tx);
    CHECK(target.rx_can_id == rx);
    CHECK(link_discover_sweep_plan_probes_for_target(
              plan, &target, &probes, &probe_count, &identity, &decoder));
    CHECK(probe_count == 3U);
    CHECK(probes[2].payload_length == 3U);
    CHECK(probes[2].payload[0] == UINT8_C(0x22));
    CHECK(probes[2].payload[1] == UINT8_C(0xf1));
    CHECK(probes[2].payload[2] == UINT8_C(0xa0));
    CHECK(identity != NULL);
    CHECK(decoder != NULL);
    return 0;
}

int main(void)
{
    const link_discover_sweep_plan *plan =
        mblink_discover_full_sweep_plan();
    link_discover_sweep_target target;
    char label[64];
    size_t index;
    size_t known_seen[9] = {
        0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U
    };
    const uint32_t known_tx[9] = {
        UINT32_C(0x612), UINT32_C(0x632), UINT32_C(0x64a),
        UINT32_C(0x652), UINT32_C(0x6b2), UINT32_C(0x602),
        UINT32_C(0x607), UINT32_C(0x4e0), UINT32_C(0x7e1)
    };
    static const uint8_t kwp_2105[] = {0x21U, 0x05U};

    CHECK(link_discover_sweep_plan_is_valid(plan));
    CHECK(plan->target_count == 760U);
    CHECK(mblink_mercedes_known_route_count() == 9U);

    /* Source-backed physical routes are deliberately first. */
    CHECK(link_discover_sweep_plan_target_at(plan, 0U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x612));
    CHECK(target.rx_can_id == UINT32_C(0x482));
    CHECK(link_discover_sweep_plan_target_at(plan, 1U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x632));
    CHECK(target.rx_can_id == UINT32_C(0x486));
    CHECK(check_kwp_target(
              plan, 2U, UINT32_C(0x64a), UINT32_C(0x489),
              NULL, 0U) == 0);
    CHECK(check_kwp_target(
              plan, 3U, UINT32_C(0x652), UINT32_C(0x48a),
              NULL, 0U) == 0);
    CHECK(link_discover_sweep_plan_target_at(plan, 4U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x6b2));
    CHECK(target.rx_can_id == UINT32_C(0x496));
    CHECK(check_uds_f1a0_target(
              plan, 5U, UINT32_C(0x602), UINT32_C(0x480)) == 0);
    CHECK(check_uds_f1a0_target(
              plan, 6U, UINT32_C(0x607), UINT32_C(0x587)) == 0);
    CHECK(check_kwp_target(
              plan, 7U, UINT32_C(0x4e0), UINT32_C(0x5ff),
              kwp_2105, sizeof(kwp_2105)) == 0);
    CHECK(check_kwp_target(
              plan, 8U, UINT32_C(0x7e1), UINT32_C(0x7e9),
              NULL, 0U) == 0);
    CHECK(check_uds_f1a0_target(
              plan, 0U, UINT32_C(0x612), UINT32_C(0x482)) == 0);

    {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_known_route_for_tx(UINT32_C(0x652));
        CHECK(route != NULL);
        CHECK(route->rx_can_id == UINT32_C(0x48a));
        CHECK(route->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
        CHECK(strcmp(route->module_key, "audio-headunit") == 0);
    }
    {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_known_route_for_tx(UINT32_C(0x7e1));
        CHECK(route != NULL);
        CHECK(route->rx_can_id == UINT32_C(0x7e9));
        CHECK(route->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
        CHECK(strcmp(route->module_key, "transmission-vgs") == 0);
    }

    /* Generic enumeration follows and excludes all source-backed in-range routes. */
    CHECK(link_discover_sweep_plan_target_at(plan, 9U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x600));
    CHECK(target.rx_can_id == UINT32_C(0x608));
    CHECK(link_discover_sweep_plan_target_at(plan, 10U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x601));
    CHECK(target.rx_can_id == UINT32_C(0x609));
    CHECK(link_discover_sweep_plan_target_at(plan, 11U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x603));
    CHECK(target.rx_can_id == UINT32_C(0x60b));

    for (index = 0U; index < 505U; ++index) {
        size_t known;
        CHECK(link_discover_sweep_plan_target_at(plan, index, &target));
        CHECK(!target.extended_id);
        for (known = 0U; known < 9U; ++known) {
            if (target.tx_can_id == known_tx[known]) ++known_seen[known];
        }
    }
    for (index = 0U; index < 9U; ++index) CHECK(known_seen[index] == 1U);

    /* The GS route is now priority source-backed instead of generic. */
    CHECK(link_discover_sweep_plan_target_at(plan, 8U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x7e1));
    CHECK(target.rx_can_id == UINT32_C(0x7e9));
    CHECK(link_discover_sweep_plan_target_at(plan, 504U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x7f7));
    CHECK(target.rx_can_id == UINT32_C(0x7ff));

    CHECK(link_discover_sweep_plan_target_at(plan, 505U, &target));
    CHECK(target.extended_id);
    CHECK(target.tx_can_id == UINT32_C(0x18da00f1));
    CHECK(target.rx_can_id == UINT32_C(0x18daf100));

    CHECK(link_discover_sweep_plan_target_at(plan, 745U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x18daf0f1));
    CHECK(link_discover_sweep_plan_target_at(plan, 746U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x18daf2f1));

    CHECK(link_discover_sweep_plan_target_at(
        plan, plan->target_count - 1U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x18dafff1));
    CHECK(target.rx_can_id == UINT32_C(0x18daf1ff));

    {
        static const uint8_t f197[] = {
            0x62U, 0xf1U, 0x97U, 'E', 'S', 'P', ' ', 0xffU
        };
        CHECK(plan->decode_identity(
            f197, sizeof(f197), label, sizeof(label)));
        CHECK(strcmp(label, "ESP") == 0);
    }

    CHECK(plan->fallback_label != NULL);
    CHECK(strcmp(plan->fallback_label(&target),
                 "Unidentified Mercedes ECU") == 0);
    puts("MBLINK Mercedes discover plan tests passed");
    return 0;
}
