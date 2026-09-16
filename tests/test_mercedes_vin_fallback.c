// SPDX-License-Identifier: GPL-3.0-or-later
/* Exercise the public identity-first scanner, not the legacy core test shim. */
#include "../include/mblink/mercedes_module_scan.h"
#include "link/diagnostic_flow.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)

static MblinkElm327Response response(const char *text)
{
    MblinkElm327Response value;
    memset(&value, 0, sizeof(value));
    value.result = MBLINK_ELM327_RESULT_OK;
    value.length = strlen(text);
    memcpy(value.text, text, value.length + 1U);
    return value;
}

static int prepare(MblinkMercedesModuleScan *scan, uint32_t tx, size_t probe)
{
    const link_discover_sweep_plan *plan = mblink_discover_mobile_census_plan();
    CHECK(mblink_mercedes_module_scan_begin_mobile_census(scan) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
    for (size_t index = 0U; index < plan->target_count; ++index) {
        link_discover_sweep_target target;
        CHECK(link_discover_sweep_plan_target_at(plan, index, &target));
        if (target.tx_can_id != tx) continue;
        CHECK(mblink_mercedes_module_scan_set_full_target(scan, index));
        scan->vin_probe_index = probe;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        return 0;
    }
    CHECK(false);
    return 1;
}

int main(void)
{
    static const struct { uint32_t tx; size_t probe; const char *payload; } positives[] = {
        {0x612U, 0U, "62F1A05744443230373330333246313239313538"},
        {0x7e0U, 0U, "4902015744443230373330333246313239313538"},
        {0x7e0U, 2U, "5A905744443230373330333246313239313538"},
        {0x612U, 1U, "61055744443230373330333246313239313538"},
        {0x7e7U, 0U, "62F1905744443230373330333246313239313538"}
    };
    static const char *invalid[] = {
        "7F2231", /* refusal is module-presence evidence only */
        "62F1905744443230373330333246313239313538", /* wrong DID */
        "61F1A05744443230373330333246313239313538", /* wrong service */
        "62F1A05744443230373330333246313239314938", /* forbidden I */
        "62F1A057444432303733303332463132393135",   /* short */
        "62F1A0574444323037333033324631323931353830" /* overlong */
    };
    MblinkMercedesModuleScan scan;
    for (size_t i = 0U; i < sizeof(positives) / sizeof(positives[0]); ++i) {
        MblinkElm327Response reply = response(positives[i].payload);
        CHECK(prepare(&scan, positives[i].tx, positives[i].probe) == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &reply) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(strcmp(scan.vin, "WDD2073032F129158") == 0);
        CHECK(scan.vin_tx_can_id == positives[i].tx);
        CHECK(scan.module_count == 1U);
    }
    for (size_t i = 0U; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        MblinkElm327Response reply = response(invalid[i]);
        CHECK(prepare(&scan, 0x612U, 0U) == 0);
        CHECK(mblink_mercedes_module_scan_accept(&scan, &reply) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(scan.vin[0] == '\0');
    }
    {
        LinkDiagnosticFlow flow;
        LinkDiagnosticFlowEvent event;
        LinkDiagnosticFlowConfig config = LINK_DIAGNOSTIC_FLOW_CONFIG_INIT;
        MblinkElm327Response reply = response(positives[0].payload);
        CHECK(prepare(&scan, 0x612U, 0U) == 0);
        scan.candidate_route_locked = false;
        mblink_mercedes_module_scan_capture_vin(&scan, &reply);
        CHECK(scan.vin[0] == '\0');
        scan.candidate_route_locked = true;
        CHECK(mblink_mercedes_module_scan_accept(&scan, &reply) == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK);
        CHECK(link_diagnostic_flow_init(&flow, &config) == LINK_DIAGNOSTIC_FLOW_RESULT_OK);
        /* State reached after the standard VIN attempt returned NO DATA. */
        flow.standard_vin_attempted = true;
        flow.stage = LINK_DIAGNOSTIC_FLOW_MANUFACTURER_EXTENSION;
        CHECK(link_diagnostic_flow_adopt_manufacturer_vin(&flow, scan.vin, &event));
        CHECK(event.vin_available);
        CHECK(strcmp(link_diagnostic_flow_standard_vin(&flow), scan.vin) == 0);
        CHECK(scan.vin_rx_can_id == 0x482U);
        CHECK(scan.vin_source == MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0);
        CHECK(mblink_mercedes_module_scan_set_full_target(&scan, 0U));
        CHECK(strcmp(scan.vin, "WDD2073032F129158") == 0);
        reply = response("62F1A053414A414435364C363457443738343335");
        mblink_mercedes_module_scan_capture_vin(&scan, &reply);
        CHECK(strcmp(scan.vin, "WDD2073032F129158") == 0);
    }
    puts("Mercedes fallback VIN retention and shared-flow integration passed");
    return 0;
}
