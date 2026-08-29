// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/discover.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

int main(void)
{
    const link_discover_sweep_plan *plan =
        mblink_discover_full_sweep_plan();
    link_discover_sweep_target target;
    char label[64];

    CHECK(link_discover_sweep_plan_is_valid(plan));
    CHECK(plan->target_count == 759U);

    CHECK(link_discover_sweep_plan_target_at(plan, 0U, &target));
    CHECK(!target.extended_id);
    CHECK(target.tx_can_id == UINT32_C(0x600));
    CHECK(target.rx_can_id == UINT32_C(0x608));
    CHECK(target.bitrate == 500000U);

    CHECK(mblink_mercedes_known_route_count() >= 4U);
    {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_known_route_for_tx(UINT32_C(0x612));
        CHECK(route != NULL);
        CHECK(route->rx_can_id == UINT32_C(0x482));
        CHECK(route->protocol == MBLINK_MERCEDES_DIAGNOSTIC_UDS);
        CHECK(strcmp(route->module_key, "eis-ezs") == 0);
    }
    CHECK(link_discover_sweep_plan_target_at(plan, 18U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x612));
    CHECK(target.rx_can_id == UINT32_C(0x482));
    CHECK(link_discover_sweep_plan_target_at(plan, 50U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x632));
    CHECK(target.rx_can_id == UINT32_C(0x486));
    {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_known_route_for_tx(UINT32_C(0x64a));
        const link_discover_sweep_probe *probes = NULL;
        size_t probe_count = 0U;
        const link_discover_sweep_probe *identity = NULL;
        link_discover_sweep_decode_identity_fn decoder = NULL;

        CHECK(route != NULL);
        CHECK(route->rx_can_id == UINT32_C(0x489));
        CHECK(route->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000);
        CHECK(strcmp(route->module_key, "restraints-orc") == 0);

        CHECK(link_discover_sweep_plan_target_at(plan, 74U, &target));
        CHECK(target.tx_can_id == UINT32_C(0x64a));
        CHECK(target.rx_can_id == UINT32_C(0x489));
        CHECK(link_discover_sweep_plan_probes_for_target(
                  plan, &target, &probes, &probe_count,
                  &identity, &decoder));
        CHECK(probe_count == 3U);
        CHECK(probes[0].payload_length == 2U);
        CHECK(probes[0].payload[0] == UINT8_C(0x3e));
        CHECK(probes[0].payload[1] == UINT8_C(0x01));
        CHECK(probes[1].payload_length == 4U);
        CHECK(probes[1].payload[0] == UINT8_C(0x18));
        CHECK(identity == NULL);
        CHECK(decoder == NULL);
    }
    CHECK(link_discover_sweep_plan_target_at(plan, 178U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x6b2));
    CHECK(target.rx_can_id == UINT32_C(0x496));

    CHECK(link_discover_sweep_plan_target_at(plan, 503U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x7f7));
    CHECK(target.rx_can_id == UINT32_C(0x7ff));

    CHECK(link_discover_sweep_plan_target_at(plan, 504U, &target));
    CHECK(target.extended_id);
    CHECK(target.tx_can_id == UINT32_C(0x18da00f1));
    CHECK(target.rx_can_id == UINT32_C(0x18daf100));

    CHECK(link_discover_sweep_plan_target_at(plan, 744U, &target));
    CHECK(target.tx_can_id == UINT32_C(0x18daf0f1));
    CHECK(link_discover_sweep_plan_target_at(plan, 745U, &target));
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
