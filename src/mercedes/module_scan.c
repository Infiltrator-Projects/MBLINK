// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file module_scan.c
 * @brief Mercedes identity-first module-scan implementation.
 *
 * The state machine is compiled once instead of being emitted into every
 * translation unit that includes the public scanner API. LINK still owns
 * generic discovery/transport machinery; this file remains Mercedes-specific.
 */
#include "mblink/mercedes_module_scan.h"

#include "infiltratr/core.h"

const char *mblink_mercedes_module_scan_scope_name(
    MblinkMercedesModuleScanScope scope)
{
    switch (scope) {
    case MBLINK_MERCEDES_MODULE_SCAN_QUICK: return "quick";
    case MBLINK_MERCEDES_MODULE_SCAN_GATEWAY: return "gateway-census";
    case MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS: return "mobile-census";
    case MBLINK_MERCEDES_MODULE_SCAN_FULL: return "full-forensic";
    case MBLINK_MERCEDES_MODULE_SCAN_CACHED: return "saved-profile";
    }
    return "unknown";
}

bool mblink_mercedes_module_scan_uses_target_plan(
    MblinkMercedesModuleScanScope scope)
{
    return scope == MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS ||
           scope == MBLINK_MERCEDES_MODULE_SCAN_FULL;
}

const link_discover_sweep_plan *
mblink_mercedes_module_scan_target_plan(MblinkMercedesModuleScanScope scope)
{
    if (scope == MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS)
        return mblink_discover_mobile_census_plan();
    if (scope == MBLINK_MERCEDES_MODULE_SCAN_FULL)
        return mblink_discover_full_sweep_plan();
    return NULL;
}

const char *mblink_mercedes_module_scan_result_name(MblinkMercedesModuleScanResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK: return "ok";
    case MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE: return "complete";
    case MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT: return "invalid-argument";
    case MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL: return "buffer-too-small";
    case MBLINK_MERCEDES_MODULE_SCAN_RESULT_ADAPTER_ERROR: return "adapter-error";
    case MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE: return "failed-state";
    }
    return "unknown";
}

const char *mblink_mercedes_module_scan_stage_name(MblinkMercedesModuleScanStage stage)
{
    switch (stage) {
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_SET_TIMEOUT: return "transmission-identity-timeout";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY: return "transmission-identity";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_RESTORE_TIMEOUT: return "transmission-identity-restore-timeout";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11: return "initialise-11-bit-can";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS: return "headers-off";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT: return "auto-formatting";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL: return "flow-control";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT: return "scan-timeout";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS: return "discover-enable-headers";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER: return "discover-set-header";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESET_RECEIVE: return "discover-reset-receive";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_FILTER: return "discover-set-filter";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK: return "discover-set-mask";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF: return "discover-lock-headers-off";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE: return "discover-set-receive";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION: return "discover-extended-session";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT: return "discover-tester-present";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK: return "discover-dtc-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_SET_TIMEOUT: return "discover-vin-timeout";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: return "discover-vin-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK: return "discover-variant-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESTORE_TIMEOUT: return "discover-restore-timeout";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY: return "discover-system-name";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART: return "discover-spare-part";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE: return "discover-software-number";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE: return "discover-hardware-number";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29: return "initialise-29-bit-can";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29: return "29-bit-headers-off";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL: return "fault-set-protocol";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER: return "fault-set-header";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE: return "fault-set-receive";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION: return "fault-extended-session";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE: return "validate-saved-module";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return "read-module-faults";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

const char *mblink_mercedes_module_scan_module_name(const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL) return "Mercedes ECU";
    if (module->controller_family != NULL)
        return module->controller_family->display_name;
    if (module->definition != NULL) return module->definition->display_name;
    if (!module->extended_id && module->tx_can_id == UINT32_C(0x7e0)) return "Engine ECU";
    if (module->identity_available && module->identity[0] != '\0') return module->identity;
    if (!module->extended_id && module->tx_can_id == UINT32_C(0x7e1))
        return "Transmission ECU / GS (7E1/7E9)";
    switch (module->kind) {
    case MBLINK_MERCEDES_MODULE_ENGINE: return "Engine ECU";
    case MBLINK_MERCEDES_MODULE_TRANSMISSION: return "Transmission ECU";
    case MBLINK_MERCEDES_MODULE_ABS_ESP: return "ABS / ESP ECU";
    case MBLINK_MERCEDES_MODULE_RESTRAINTS: return "Restraints ECU";
    case MBLINK_MERCEDES_MODULE_CLIMATE: return "Climate ECU";
    case MBLINK_MERCEDES_MODULE_INSTRUMENT_CLUSTER: return "Instrument cluster ECU";
    case MBLINK_MERCEDES_MODULE_BODY: return "Body ECU";
    case MBLINK_MERCEDES_MODULE_OTHER: return "Mercedes ECU";
    }
    return "Mercedes ECU";
}

MblinkMercedesModuleKind mblink_mercedes_module_scan_kind(uint32_t tx_can_id, bool extended_id)
{
    if (!extended_id && tx_can_id == UINT32_C(0x7e0))
        return MBLINK_MERCEDES_MODULE_ENGINE;
    /*
     * Mercedes CAN definitions explicitly name 0x7E1 D_RQ_GS (KWP2000
     * diagnostic request to gearbox control) and 0x7E9 D_RS_GS (diagnostic
     * response from gearbox control). Field evidence independently proves
     * 0x7E9 is live on at least one supported Mercedes configuration. Exact
     * VGS/EGS family still requires identity, but
     * the module class itself is source-backed transmission/GS.
     */
    if (!extended_id && tx_can_id == UINT32_C(0x7e1))
        return MBLINK_MERCEDES_MODULE_TRANSMISSION;
    return MBLINK_MERCEDES_MODULE_OTHER;
}

bool mblink_mercedes_module_scan_write_command(const char *command, char *buffer, size_t buffer_size, size_t *written)
{
    size_t length;
    if (written != NULL) *written = 0U;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (command == NULL || buffer == NULL || written == NULL) return false;
    length = strlen(command);
    if (length + 1U > buffer_size) return false;
    memcpy(buffer, command, length + 1U);
    *written = length;
    return true;
}

void mblink_mercedes_module_scan_set_11_candidate(MblinkMercedesModuleScan *scan, uint32_t tx)
{
    scan->candidate_tx = tx;
    scan->candidate_rx = tx + UINT32_C(8);
    scan->candidate_extended = false;
    scan->candidate_route_locked = false;
    scan->vin_probe_index = 0U;
}

bool mblink_mercedes_module_scan_set_gateway_target(
    MblinkMercedesModuleScan *scan,
    uint16_t target)
{
    if (scan == NULL || target > UINT16_C(0xff) ||
        target == UINT16_C(0xf1)) {
        return false;
    }
    scan->gateway_target = target;
    scan->candidate_tx =
        UINT32_C(0x18da00f1) | ((uint32_t)target << 8U);
    scan->candidate_rx =
        UINT32_C(0x18daf100) | (uint32_t)target;
    scan->candidate_extended = true;
    scan->candidate_route_locked = true;
    scan->vin_probe_index = 0U;
    return true;
}

size_t mblink_mercedes_module_scan_planned_target_count(
    MblinkMercedesModuleScanScope scope)
{
    switch (scope) {
    case MBLINK_MERCEDES_MODULE_SCAN_QUICK:
        return 8U;
    case MBLINK_MERCEDES_MODULE_SCAN_GATEWAY:
        /* 8 EOBD endpoints plus 256 logical targets minus tester address F1. */
        return 263U;
    case MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS:
    case MBLINK_MERCEDES_MODULE_SCAN_FULL: {
        const link_discover_sweep_plan *plan =
            mblink_mercedes_module_scan_target_plan(scope);
        return link_discover_sweep_plan_is_valid(plan)
            ? plan->target_count : 0U;
    }
    case MBLINK_MERCEDES_MODULE_SCAN_CACHED:
        return 0U;
    }
    return 0U;
}

bool mblink_mercedes_module_scan_set_full_target(
    MblinkMercedesModuleScan *scan,
    size_t index)
{
    const link_discover_sweep_plan *plan;
    link_discover_sweep_target target;

    if (scan == NULL) return false;
    plan = mblink_mercedes_module_scan_target_plan(scan->scope);
    if (!link_discover_sweep_plan_target_at(plan, index, &target) ||
        target.bitrate != 500000U) {
        return false;
    }
    scan->full_target_index = index;
    scan->candidate_tx = target.tx_can_id;
    scan->candidate_rx = target.rx_can_id;
    scan->candidate_extended = target.extended_id;
    scan->vin_probe_index = 0U;
    /*
     * Only lock routes whose RX identifier is authoritative.  Source-backed
     * Mercedes routes carry independently evidenced TX/RX pairs, 29-bit normal
     * fixed addressing defines both identifiers, and ISO 15765-4 legislated
     * OBD physical requests 0x7E0..0x7E7 have the standard +8 response slots.
     *
     * Every other 11-bit sweep entry uses TX+8 only as an initial placeholder.
     * Leave those candidates unlocked so the existing headered discovery path
     * can temporarily widen the ELM receive filter, learn the ECU's real reply
     * CAN ID from a valid UDS response, then immediately lock back onto it.
     */
    {
        const MblinkMercedesKnownRoute *known_route =
            !target.extended_id
                ? mblink_mercedes_known_route_for_tx(target.tx_can_id)
                : NULL;
        const bool standard_obd_route =
            !target.extended_id &&
            target.tx_can_id >= UINT32_C(0x7e0) &&
            target.tx_can_id <= UINT32_C(0x7e7) &&
            target.rx_can_id == target.tx_can_id + UINT32_C(8);

        scan->candidate_route_locked =
            scan->scope == MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS ||
            target.extended_id ||
            (known_route != NULL &&
             known_route->rx_can_id == target.rx_can_id) ||
            standard_obd_route;
    }
    return true;
}

const MblinkMercedesKnownRoute *
mblink_mercedes_module_scan_known_route(const MblinkMercedesModuleScan *scan)
{
    const MblinkMercedesKnownRoute *route;
    if (scan == NULL || scan->candidate_extended) return NULL;
    route = mblink_mercedes_known_route_for_tx(scan->candidate_tx);
    return route != NULL && route->rx_can_id == scan->candidate_rx
        ? route : NULL;
}

const MblinkMercedesKnownRoute *
mblink_mercedes_module_scan_known_entry_route(
    const MblinkMercedesModuleScanEntry *module)
{
    const MblinkMercedesKnownRoute *route;
    if (module == NULL || module->extended_id) return NULL;
    route = mblink_mercedes_known_route_for_tx(module->tx_can_id);
    return route != NULL && route->rx_can_id == module->rx_can_id
        ? route : NULL;
}

MblinkMercedesDiagnosticProtocol
mblink_mercedes_module_scan_candidate_protocol(
    const MblinkMercedesModuleScan *scan)
{
    const MblinkMercedesKnownRoute *route =
        mblink_mercedes_module_scan_known_route(scan);
    return route != NULL ? route->protocol : MBLINK_MERCEDES_DIAGNOSTIC_UDS;
}

bool mblink_mercedes_module_scan_is_production_vin_target(
    const MblinkMercedesModuleScan *scan)
{
    if (scan == NULL || scan->candidate_extended) return false;
    return
        (scan->candidate_tx == UINT32_C(0x7e0) &&
         scan->candidate_rx == UINT32_C(0x7e8)) ||
        (scan->candidate_tx == UINT32_C(0x4e0) &&
         scan->candidate_rx == UINT32_C(0x5ff)) ||
        (scan->candidate_tx == UINT32_C(0x602) &&
         scan->candidate_rx == UINT32_C(0x480)) ||
        (scan->candidate_tx == UINT32_C(0x607) &&
         scan->candidate_rx == UINT32_C(0x587)) ||
        (scan->candidate_tx == UINT32_C(0x612) &&
         scan->candidate_rx == UINT32_C(0x482));
}

MblinkMercedesVinProbe mblink_mercedes_module_scan_vin_probe_at(
    const MblinkMercedesModuleScan *scan,
    size_t index)
{
    const MblinkMercedesKnownRoute *route;
    if (scan == NULL || scan->candidate_extended)
        return MBLINK_MERCEDES_VIN_PROBE_NONE;

    if (scan->candidate_tx == UINT32_C(0x7e0) &&
        scan->candidate_rx == UINT32_C(0x7e8)) {
        static const MblinkMercedesVinProbe probes[] = {
            MBLINK_MERCEDES_VIN_PROBE_OBD_0902,
            MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0,
            MBLINK_MERCEDES_VIN_PROBE_KWP_1A90
        };
        return index < INFILTRATR_ARRAY_LENGTH(probes)
            ? probes[index] : MBLINK_MERCEDES_VIN_PROBE_NONE;
    }
    if (scan->candidate_tx == UINT32_C(0x4e0) &&
        scan->candidate_rx == UINT32_C(0x5ff)) {
        static const MblinkMercedesVinProbe probes[] = {
            MBLINK_MERCEDES_VIN_PROBE_KWP_2105,
            MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0,
            MBLINK_MERCEDES_VIN_PROBE_KWP_1A90
        };
        return index < INFILTRATR_ARRAY_LENGTH(probes)
            ? probes[index] : MBLINK_MERCEDES_VIN_PROBE_NONE;
    }
    if (scan->candidate_tx == UINT32_C(0x612) &&
        scan->candidate_rx == UINT32_C(0x482)) {
        static const MblinkMercedesVinProbe probes[] = {
            MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0,
            MBLINK_MERCEDES_VIN_PROBE_KWP_2105
        };
        return index < INFILTRATR_ARRAY_LENGTH(probes)
            ? probes[index] : MBLINK_MERCEDES_VIN_PROBE_NONE;
    }
    if ((scan->candidate_tx == UINT32_C(0x602) &&
         scan->candidate_rx == UINT32_C(0x480)) ||
        (scan->candidate_tx == UINT32_C(0x607) &&
         scan->candidate_rx == UINT32_C(0x587))) {
        return index == 0U
            ? MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0
            : MBLINK_MERCEDES_VIN_PROBE_NONE;
    }

    route = mblink_mercedes_module_scan_known_route(scan);
    if (route != NULL)
        return index == 0U ? route->vin_probe
                           : MBLINK_MERCEDES_VIN_PROBE_NONE;

    return index == 0U ? MBLINK_MERCEDES_VIN_PROBE_UDS_F190
                       : MBLINK_MERCEDES_VIN_PROBE_NONE;
}

const char *mblink_mercedes_module_scan_vin_command(
    const MblinkMercedesModuleScan *scan)
{
    switch (mblink_mercedes_module_scan_vin_probe_at(
                scan, scan != NULL ? scan->vin_probe_index : 0U)) {
    case MBLINK_MERCEDES_VIN_PROBE_OBD_0902: return "0902";
    case MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0: return "22F1A0";
    case MBLINK_MERCEDES_VIN_PROBE_KWP_1A90: return "1A90";
    case MBLINK_MERCEDES_VIN_PROBE_KWP_2105: return "2105";
    case MBLINK_MERCEDES_VIN_PROBE_UDS_F190: return "22F190";
    case MBLINK_MERCEDES_VIN_PROBE_NONE: return NULL;
    }
    return NULL;
}

bool mblink_mercedes_module_scan_has_next_vin_probe(
    const MblinkMercedesModuleScan *scan)
{
    return scan != NULL &&
        mblink_mercedes_module_scan_vin_probe_at(
            scan, scan->vin_probe_index + 1U) !=
            MBLINK_MERCEDES_VIN_PROBE_NONE;
}

void mblink_mercedes_module_scan_enter_vin_fallback(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;
    scan->vin_probe_index = 0U;
    scan->stage =
        mblink_mercedes_module_scan_is_production_vin_target(scan) &&
        !scan->vin_timeout_long
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_SET_TIMEOUT
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
}

MblinkMercedesDiagnosticProtocol
mblink_mercedes_module_scan_entry_protocol(
    const MblinkMercedesModuleScanEntry *module)
{
    const MblinkMercedesKnownRoute *route =
        mblink_mercedes_module_scan_known_entry_route(module);
    return route != NULL ? route->protocol
                         : (module != NULL ? module->protocol
                                           : MBLINK_MERCEDES_DIAGNOSTIC_UDS);
}

void mblink_mercedes_module_scan_finish_discovery(MblinkMercedesModuleScan *scan)
{
    size_t index;
    bool needs_dtc_pass = false;

    if (scan == NULL) return;
    /*
     * Discovery now captures DTC memory as soon as a responder is found.  A
     * second pass is only required for unusual modules discovered solely by a
     * VIN/identity fallback whose DTC request produced no usable response.
     */
    for (index = 0U; index < scan->module_count; ++index) {
        if (scan->modules[index].dtc_result ==
            MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED) {
            needs_dtc_pass = true;
            break;
        }
    }
    scan->dtc_index = 0U;
    scan->stage = scan->module_count == 0U
        ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE
        : (needs_dtc_pass
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE);
}

void mblink_mercedes_module_scan_advance_candidate(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;

    /*
     * The production cascade allows 1050 ms. ELM327 ATST tops out at 0xFF
     * (1020 ms), so only these exact fallback targets are widened. Restore the
     * normal fast census timeout before moving to the next ECU.
     */
    if (scan->vin_timeout_long) {
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESTORE_TIMEOUT;
        return;
    }

    if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_QUICK) {
        if (scan->candidate_tx < UINT32_C(0x7e7)) {
            mblink_mercedes_module_scan_set_11_candidate(
                scan, scan->candidate_tx + 1U);
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
            return;
        }
        mblink_mercedes_module_scan_finish_discovery(scan);
        return;
    }

    if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_GATEWAY) {
        if (!scan->candidate_extended) {
            if (scan->candidate_tx < UINT32_C(0x7e7)) {
                mblink_mercedes_module_scan_set_11_candidate(
                    scan, scan->candidate_tx + 1U);
                scan->stage =
                    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
                return;
            }
            if (!mblink_mercedes_module_scan_set_gateway_target(scan, 0U)) {
                scan->failure =
                    MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED;
                return;
            }
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29;
            return;
        }

        {
            uint16_t next = (uint16_t)(scan->gateway_target + 1U);
            if (next == UINT16_C(0xf1)) ++next;
            if (next > UINT16_C(0xff)) {
                mblink_mercedes_module_scan_finish_discovery(scan);
                return;
            }
            if (!mblink_mercedes_module_scan_set_gateway_target(scan, next)) {
                scan->failure =
                    MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED;
                return;
            }
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
            return;
        }
    }

    {
        const link_discover_sweep_plan *plan =
            mblink_mercedes_module_scan_target_plan(scan->scope);
        const size_t next = scan->full_target_index + 1U;
        const bool was_extended = scan->candidate_extended;

        if (!link_discover_sweep_plan_is_valid(plan) ||
            next >= plan->target_count) {
            mblink_mercedes_module_scan_finish_discovery(scan);
            return;
        }
        if (!mblink_mercedes_module_scan_set_full_target(scan, next)) {
            scan->failure =
                MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED;
            return;
        }
        if (scan->candidate_extended != was_extended) {
            scan->stage = scan->candidate_extended
                ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29
                : MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
            return;
        }
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
    }
}

MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_record_module(MblinkMercedesModuleScan *scan, bool tester_present)
{
    size_t index;
    for (index = 0U; index < scan->module_count; ++index) {
        MblinkMercedesModuleScanEntry *module = &scan->modules[index];
        if (module->tx_can_id == scan->candidate_tx && module->rx_can_id == scan->candidate_rx && module->extended_id == scan->candidate_extended) {
            module->tester_present_response |= tester_present;
            return module;
        }
    }
    if (scan->module_count >= MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES) {
        scan->truncated = true;
        return NULL;
    }
    MblinkMercedesModuleScanEntry *module = &scan->modules[scan->module_count++];
    memset(module, 0, sizeof(*module));
    module->tx_can_id = scan->candidate_tx;
    module->rx_can_id = scan->candidate_rx;
    module->extended_id = scan->candidate_extended;
    module->protocol = mblink_mercedes_module_scan_candidate_protocol(scan);
    module->kind = mblink_mercedes_module_scan_kind(scan->candidate_tx, scan->candidate_extended);
    module->tester_present_response = tester_present;
    module->identification_status = MBLINK_MERCEDES_DEFINITION_CANDIDATE;
    module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED;
    {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_module_scan_known_route(scan);
        if (route != NULL) {
            const MblinkMercedesModuleDefinition *definition =
                mblink_mercedes_module_definition_for_key(
                    route->module_key);
            if (definition != NULL) {
                module->definition = definition;
                module->kind = definition->kind;
                module->identification_status = definition->status;
            }
        }
    }
    return module;
}

MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_find_candidate(
    MblinkMercedesModuleScan *scan)
{
    size_t index;
    if (scan == NULL) return NULL;
    for (index = 0U; index < scan->module_count; ++index) {
        MblinkMercedesModuleScanEntry *module = &scan->modules[index];
        if (module->tx_can_id == scan->candidate_tx &&
            module->rx_can_id == scan->candidate_rx &&
            module->extended_id == scan->candidate_extended) {
            return module;
        }
    }
    return NULL;
}

bool mblink_mercedes_module_scan_capture_text_did(
    const MblinkElm327Response *response,
    uint16_t did,
    char *destination,
    size_t destination_capacity)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkUdsDidRecord record;
    size_t length;
    size_t index;

    if (response == NULL || destination == NULL ||
        destination_capacity == 0U ||
        response->result != MBLINK_ELM327_RESULT_OK) {
        return false;
    }
    destination[0] = '\0';
    if (mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
        MBLINK_ELM327_CAN_RESULT_OK) {
        return false;
    }
    if (mblink_uds_decode_read_did_response(
            pdu, pdu_length, did, &record) !=
        MBLINK_UDS_RESULT_OK) {
        return false;
    }

    length = record.data_length;
    while (length != 0U &&
           (record.data[length - 1U] == 0U ||
            record.data[length - 1U] == UINT8_C(0xff) ||
            record.data[length - 1U] == (uint8_t)' ')) {
        --length;
    }
    if (length == 0U) return false;
    if (length >= destination_capacity) length = destination_capacity - 1U;
    for (index = 0U; index < length; ++index) {
        const uint8_t value = record.data[index];
        if (value < UINT8_C(0x20) || value > UINT8_C(0x7e)) {
            destination[0] = '\0';
            return false;
        }
        destination[index] = (char)value;
    }
    destination[length] = '\0';
    return true;
}

void mblink_mercedes_module_scan_classify_controller_family(
    MblinkMercedesModuleScanEntry *module)
{
    const char *module_key;

    if (module == NULL || module->definition == NULL) return;
    module_key = module->definition->key;
    module->controller_family =
        mblink_mercedes_controller_family_definition_for_evidence(
            module_key,
            module->identity_available ? module->identity : NULL,
            module->software_number_available ? module->software_number : NULL,
            module->hardware_number_available ? module->hardware_number : NULL);
}

void mblink_mercedes_module_scan_classify_identity(
    MblinkMercedesModuleScanEntry *module)
{
    const MblinkMercedesModuleDefinition *definition;

    if (module == NULL) return;
    if (module->identity_available && module->identity[0] != '\0') {
        definition =
            mblink_mercedes_module_definition_for_identity(module->identity);
        if (definition != NULL) {
            module->definition = definition;
            module->kind = definition->kind;
            module->identification_status = definition->status;
        }
    }
    mblink_mercedes_module_scan_classify_controller_family(module);
}

void mblink_mercedes_module_scan_capture_identity(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response)
{
    if (module == NULL) return;
    module->identity_available =
        mblink_mercedes_module_scan_capture_text_did(
            response, UINT16_C(0xf197),
            module->identity, sizeof(module->identity));
    mblink_mercedes_module_scan_classify_identity(module);
}

bool mblink_mercedes_module_scan_decode_uds(const MblinkElm327Response *response, uint8_t service, uint8_t *pdu, size_t pdu_capacity, size_t *pdu_length, MblinkUdsResponse *uds)
{
    MblinkUdsResult result;
    if (response == NULL || pdu == NULL || pdu_length == NULL || uds == NULL || response->result != MBLINK_ELM327_RESULT_OK) return false;
    if (mblink_elm327_can_decode_pdu(response, pdu, pdu_capacity, pdu_length) != MBLINK_ELM327_CAN_RESULT_OK) return false;
    result = mblink_uds_decode_response(service, pdu, *pdu_length, uds);
    return result == MBLINK_UDS_RESULT_OK || result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE;
}

bool mblink_mercedes_module_scan_decode_kwp(
    const MblinkElm327Response *response,
    uint8_t service)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkKwp2000Response decoded;
    MblinkKwp2000Result result;

    if (response == NULL || response->result != MBLINK_ELM327_RESULT_OK)
        return false;
    if (mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
        MBLINK_ELM327_CAN_RESULT_OK) {
        return false;
    }
    result = mblink_kwp2000_decode_response(
        service, pdu, pdu_length, &decoded);
    return result == MBLINK_KWP2000_RESULT_OK ||
           result == MBLINK_KWP2000_RESULT_NEGATIVE_RESPONSE;
}

bool mblink_mercedes_module_scan_decode_candidate(
    const MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response,
    uint8_t uds_service,
    uint8_t kwp_service)
{
    if (mblink_mercedes_module_scan_candidate_protocol(scan) ==
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        return mblink_mercedes_module_scan_decode_kwp(response, kwp_service);
    }
    {
        uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
        size_t pdu_length = 0U;
        MblinkUdsResponse uds;
        return mblink_mercedes_module_scan_decode_uds(
            response, uds_service, pdu, sizeof(pdu), &pdu_length, &uds);
    }
}

void mblink_mercedes_module_scan_capture_vin(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t length = 0U;
    size_t offset = 0U;
    char vin[LINK_OBD2_VIN_LENGTH + 1U] = {0};
    MblinkMercedesVinProbe probe;
    if (scan == NULL || response == NULL || scan->vin[0] != '\0' ||
        !scan->candidate_route_locked ||
        response->result != MBLINK_ELM327_RESULT_OK) return;
    probe = mblink_mercedes_module_scan_vin_probe_at(scan, scan->vin_probe_index);
    if (probe == MBLINK_MERCEDES_VIN_PROBE_OBD_0902) {
        if (mblink_obd2_decode_vin(response, vin) != LINK_OBD2_RESULT_OK) return;
    } else {
        if (mblink_elm327_can_decode_pdu(response, pdu, sizeof(pdu), &length) !=
            MBLINK_ELM327_CAN_RESULT_OK) return;
        if ((probe == MBLINK_MERCEDES_VIN_PROBE_UDS_F190 ||
             probe == MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0) &&
            length == 3U + LINK_OBD2_VIN_LENGTH &&
            pdu[0] == 0x62U && pdu[1] == 0xf1U &&
            pdu[2] == (probe == MBLINK_MERCEDES_VIN_PROBE_UDS_F190 ? 0x90U : 0xa0U)) {
            offset = 3U;
        } else if (length == 2U + LINK_OBD2_VIN_LENGTH &&
                   ((probe == MBLINK_MERCEDES_VIN_PROBE_KWP_1A90 &&
                     pdu[0] == 0x5aU && pdu[1] == 0x90U) ||
                    (probe == MBLINK_MERCEDES_VIN_PROBE_KWP_2105 &&
                     pdu[0] == 0x61U && pdu[1] == 0x05U))) {
            offset = 2U;
        } else return;
        for (size_t index = 0U; index < LINK_OBD2_VIN_LENGTH; ++index) {
            const uint8_t value = pdu[offset + index];
            if (!((value >= '0' && value <= '9') ||
                  (value >= 'A' && value <= 'Z' &&
                   value != 'I' && value != 'O' && value != 'Q'))) return;
            vin[index] = (char)value;
        }
    }
    memcpy(scan->vin, vin, sizeof(scan->vin));
    scan->vin_tx_can_id = scan->candidate_tx;
    scan->vin_rx_can_id = scan->candidate_rx;
    scan->vin_source = probe;
}

bool mblink_mercedes_module_scan_decode_vin_probe(
    const MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response)
{
    const MblinkMercedesVinProbe probe =
        mblink_mercedes_module_scan_vin_probe_at(
            scan, scan != NULL ? scan->vin_probe_index : 0U);

    if (probe == MBLINK_MERCEDES_VIN_PROBE_OBD_0902) {
        uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
        size_t pdu_length = 0U;
        if (response == NULL ||
            response->result != MBLINK_ELM327_RESULT_OK ||
            mblink_elm327_can_decode_pdu(
                response, pdu, sizeof(pdu), &pdu_length) !=
                MBLINK_ELM327_CAN_RESULT_OK) {
            return false;
        }
        return (pdu_length >= 2U &&
                pdu[0] == UINT8_C(0x49) &&
                pdu[1] == UINT8_C(0x02)) ||
               (pdu_length >= 2U &&
                pdu[0] == UINT8_C(0x7f) &&
                pdu[1] == UINT8_C(0x09));
    }
    if (probe == MBLINK_MERCEDES_VIN_PROBE_KWP_1A90)
        return mblink_mercedes_module_scan_decode_kwp(
            response, UINT8_C(0x1a));
    if (probe == MBLINK_MERCEDES_VIN_PROBE_KWP_2105)
        return mblink_mercedes_module_scan_decode_kwp(
            response, UINT8_C(0x21));
    if (probe == MBLINK_MERCEDES_VIN_PROBE_UDS_F190 ||
        probe == MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0) {
        uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
        size_t pdu_length = 0U;
        MblinkUdsResponse uds;
        return mblink_mercedes_module_scan_decode_uds(
            response, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
            pdu, sizeof(pdu), &pdu_length, &uds);
    }
    return false;
}

bool mblink_mercedes_module_scan_decode_entry(
    const MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response,
    uint8_t uds_service,
    uint8_t kwp_service)
{
    if (mblink_mercedes_module_scan_entry_protocol(module) ==
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        return mblink_mercedes_module_scan_decode_kwp(response, kwp_service);
    }
    {
        uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
        size_t pdu_length = 0U;
        MblinkUdsResponse uds;
        return mblink_mercedes_module_scan_decode_uds(
            response, uds_service, pdu, sizeof(pdu), &pdu_length, &uds);
    }
}

void mblink_mercedes_module_scan_capture_dtc(MblinkMercedesModuleScanEntry *module, const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    if (module == NULL || response == NULL) return;

    memset(&module->dtcs, 0, sizeof(module->dtcs));
    memset(&module->kwp_dtcs, 0, sizeof(module->kwp_dtcs));
    module->dtc_negative_response_code = 0U;
    if (response->result != MBLINK_ELM327_RESULT_OK) {
        module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE;
        return;
    }
    if (mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
        MBLINK_ELM327_CAN_RESULT_OK) {
        module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE;
        return;
    }

    if (mblink_mercedes_module_scan_entry_protocol(module) ==
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        MblinkKwp2000Response generic;
        MblinkKwp2000Result result = mblink_kwp2000_decode_response(
            MBLINK_KWP2000_SERVICE_READ_DTC_BY_STATUS,
            pdu, pdu_length, &generic);
        module->dtc_kwp_result = result;
        if (result == MBLINK_KWP2000_RESULT_NEGATIVE_RESPONSE) {
            module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE;
            module->dtc_negative_response_code = generic.negative_response_code;
            return;
        }
        if (result != MBLINK_KWP2000_RESULT_OK) {
            module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE;
            return;
        }
        result = mblink_kwp2000_decode_read_dtc_by_status_response(
            pdu, pdu_length, &module->kwp_dtcs);
        module->dtc_kwp_result = result;
        module->dtc_result =
            (result == MBLINK_KWP2000_RESULT_OK ||
             result == MBLINK_KWP2000_RESULT_TRUNCATED)
                ? MBLINK_MERCEDES_MODULE_DTC_AVAILABLE
                : MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE;
        return;
    }

    {
        MblinkUdsResponse generic;
        MblinkUdsResult result = mblink_uds_decode_response(
            MBLINK_UDS_SERVICE_READ_DTC_INFORMATION,
            pdu, pdu_length, &generic);
        module->dtc_uds_result = result;
        if (result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
            module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE;
            module->dtc_negative_response_code = generic.negative_response_code;
            return;
        }
        if (result != MBLINK_UDS_RESULT_OK) {
            module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE;
            return;
        }
        result = mblink_uds_decode_report_dtcs_by_status_mask_response(
            pdu, pdu_length, &module->dtcs);
        module->dtc_uds_result = result;
        module->dtc_result = result == MBLINK_UDS_RESULT_OK
            ? MBLINK_MERCEDES_MODULE_DTC_AVAILABLE
            : MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE;
    }
}

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_begin(MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    memset(scan, 0, sizeof(*scan));
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_QUICK;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    mblink_mercedes_module_scan_set_11_candidate(scan, UINT32_C(0x7e0));
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_gateway(MblinkMercedesModuleScan *scan)
{
    if (scan == NULL)
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    memset(scan, 0, sizeof(*scan));
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_GATEWAY;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    mblink_mercedes_module_scan_set_11_candidate(scan, UINT32_C(0x7e0));
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_mobile_census(
    MblinkMercedesModuleScan *scan)
{
    const link_discover_sweep_plan *plan;
    if (scan == NULL)
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    plan = mblink_discover_mobile_census_plan();
    if (!link_discover_sweep_plan_is_valid(plan))
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;

    memset(scan, 0, sizeof(*scan));
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS;
    if (!mblink_mercedes_module_scan_set_full_target(scan, 0U))
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    scan->stage = scan->candidate_extended
        ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29
        : MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_cached(
    MblinkMercedesModuleScan *scan,
    const MblinkMercedesModuleScanEntry *modules,
    size_t module_count)
{
    size_t index;
    if (scan == NULL || modules == NULL || module_count == 0U ||
        module_count > MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES) {
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    }

    memset(scan, 0, sizeof(*scan));
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_CACHED;
    scan->module_count = module_count;
    for (index = 0U; index < module_count; ++index) {
        const MblinkMercedesModuleScanEntry *source = &modules[index];
        MblinkMercedesModuleScanEntry *destination = &scan->modules[index];
        const uint32_t max_id = source->extended_id
            ? UINT32_C(0x1fffffff) : UINT32_C(0x7ff);
        if (source->tx_can_id > max_id || source->rx_can_id > max_id) {
            memset(scan, 0, sizeof(*scan));
            return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
        }
        *destination = *source;
        destination->tester_present_response = false;
        destination->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED;
        destination->dtc_uds_result = (MblinkUdsResult)0;
        destination->dtc_kwp_result = (MblinkKwp2000Result)0;
        destination->dtc_negative_response_code = 0U;
        memset(&destination->dtcs, 0, sizeof(destination->dtcs));
        memset(&destination->kwp_dtcs, 0, sizeof(destination->kwp_dtcs));
        {
            const MblinkMercedesKnownRoute *route =
                mblink_mercedes_module_scan_known_entry_route(destination);
            if (route != NULL) {
                const MblinkMercedesModuleDefinition *definition =
                    mblink_mercedes_module_definition_for_key(
                        route->module_key);
                destination->protocol = route->protocol;
                if (definition != NULL) {
                    destination->definition = definition;
                    destination->kind = definition->kind;
                    destination->identification_status = definition->status;
                }
            }
        }
        if (destination->identity_available)
            mblink_mercedes_module_scan_classify_identity(destination);
    }
    scan->dtc_index = 0U;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_begin_full(
    MblinkMercedesModuleScan *scan)
{
    const link_discover_sweep_plan *plan;
    if (scan == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    plan = mblink_discover_full_sweep_plan();
    if (!link_discover_sweep_plan_is_valid(plan)) {
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    }
    memset(scan, 0, sizeof(*scan));
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_FULL;
    if (!mblink_mercedes_module_scan_set_full_target(scan, 0U)) {
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    }
    scan->stage = scan->candidate_extended
        ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29
        : MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

uint64_t mblink_mercedes_module_scan_timeout_ms(const MblinkMercedesModuleScan *scan)
{
    /*
     * The ELM's own ATST timer decides when an unanswered CAN request becomes
     * NO DATA.  Keep the host/BLE watchdog comfortably above that value so a
     * slow CoreBluetooth delivery does not turn an ordinary module miss into
     * a fatal diagnostic-session timeout.
     */
    if (scan == NULL) return UINT64_C(4000);
    switch (scan->stage) {
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_SET_TIMEOUT:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESTORE_TIMEOUT:
        return UINT64_C(4000);
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE:
        return UINT64_C(4000);
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return UINT64_C(5000);
    default: return UINT64_C(4000);
    }
}

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_command(const MblinkMercedesModuleScan *scan, char *buffer, size_t buffer_size, size_t *written)
{
    const MblinkMercedesModuleScanEntry *module;
    if (scan == NULL || buffer == NULL || written == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
#define WRITE(cmd) (mblink_mercedes_module_scan_write_command((cmd), buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL)
    switch (scan->stage) {
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_SET_TIMEOUT: return WRITE("ATST64");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_RESTORE_TIMEOUT: return WRITE("ATST20");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY:
        /* The public identity layer formats the bounded KWP request. */
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11: return WRITE("ATSP6");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS: return WRITE("ATH0");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT: return WRITE("ATCAF1");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL: return WRITE("ATCFC1");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT:
        /*
         * Mobile/gateway discovery gets a 128 ms ELM response window.  The
         * Linux-only forensic sweep deliberately allows 200 ms for gatewayed
         * or sleepy ECUs.  These remain bounded read-only requests.
         */
        return WRITE(scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL
            ? "ATST32" : "ATST20");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29: return WRITE("ATSP7");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29: return WRITE("ATH0");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS: return WRITE("ATH1");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER: return mblink_elm327_can_format_header_command(scan->candidate_tx, scan->candidate_extended, buffer, buffer_size) == MBLINK_ELM327_CAN_RESULT_OK ? (*written = strlen(buffer), MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESET_RECEIVE: return WRITE("ATCRA");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_FILTER: return WRITE("ATCF000");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK: return WRITE("ATCM000");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF: return WRITE("ATH0");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE: return mblink_elm327_can_format_receive_address_command(scan->candidate_rx, scan->candidate_extended, buffer, buffer_size) == MBLINK_ELM327_CAN_RESULT_OK ? (*written = strlen(buffer), MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION: return WRITE("1003");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
        return WRITE(mblink_mercedes_module_scan_candidate_protocol(scan) ==
                         MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                     ? "3E01" : "3E00");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
        return WRITE(mblink_mercedes_module_scan_candidate_protocol(scan) ==
                         MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                     ? "1802FF00" : "1902FF");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_SET_TIMEOUT:
        return WRITE("ATSTFF");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESTORE_TIMEOUT:
        return WRITE(scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL
            ? "ATST32" : "ATST20");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ:
        if (scan->dtc_index >= scan->module_count)
            return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index];
        return WRITE(mblink_mercedes_module_scan_entry_protocol(module) ==
                         MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                     ? "1802FF00" : "1902FF");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION: return WRITE("1003");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE:
        if (scan->dtc_index >= scan->module_count)
            return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index];
        return WRITE(mblink_mercedes_module_scan_entry_protocol(module) ==
                         MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                     ? "3E01" : "3E00");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: {
        const char *vin_command =
            mblink_mercedes_module_scan_vin_command(scan);
        return vin_command != NULL
            ? WRITE(vin_command)
            : MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
    }
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK: return WRITE("22F100");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY: return WRITE("22F197");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART: return WRITE("22F187");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE: return WRITE("22F188");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE: return WRITE("22F191");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL:
        if (scan->dtc_index >= scan->module_count) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index]; return WRITE(module->extended_id ? "ATSP7" : "ATSP6");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER:
        if (scan->dtc_index >= scan->module_count) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index]; return mblink_elm327_can_format_header_command(module->tx_can_id, module->extended_id, buffer, buffer_size) == MBLINK_ELM327_CAN_RESULT_OK ? (*written = strlen(buffer), MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE:
        if (scan->dtc_index >= scan->module_count) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index]; return mblink_elm327_can_format_receive_address_command(module->rx_can_id, module->extended_id, buffer, buffer_size) == MBLINK_ELM327_CAN_RESULT_OK ? (*written = strlen(buffer), MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK) : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: if (buffer_size != 0U) buffer[0] = '\0'; *written = 0U; return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
    }
#undef WRITE
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
}

int mblink_mercedes_module_scan_hex_value(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    return -1;
}

bool mblink_mercedes_module_scan_headered_11_route(
    const MblinkElm327Response *response,
    uint8_t request_service,
    uint32_t *rx_can_id)
{
    const char *cursor;
    if (rx_can_id != NULL) *rx_can_id = 0U;
    if (response == NULL || rx_can_id == NULL ||
        response->result != MBLINK_ELM327_RESULT_OK) return false;

    cursor = response->text;
    while (cursor != NULL && *cursor != '\0') {
        char compact[96];
        size_t compact_length = 0U;
        const char *end = strchr(cursor, '\n');
        const char *line_end = end != NULL ? end : cursor + strlen(cursor);
        const char *p;
        uint32_t id = 0U;
        uint8_t bytes[40];
        size_t byte_count = 0U;
        size_t payload_offset = 0U;

        for (p = cursor; p < line_end && compact_length + 1U < sizeof(compact); ++p) {
            if (*p == ' ' || *p == '\t' || *p == '\r') continue;
            if (mblink_mercedes_module_scan_hex_value(*p) < 0) {
                compact_length = 0U;
                break;
            }
            compact[compact_length++] = *p;
        }
        compact[compact_length] = '\0';
        if (compact_length >= 7U && ((compact_length - 3U) % 2U) == 0U) {
            for (size_t index = 0U; index < 3U; ++index) {
                id = (id << 4U) |
                    (uint32_t)mblink_mercedes_module_scan_hex_value(compact[index]);
            }
            for (size_t index = 3U;
                 index + 1U < compact_length && byte_count < sizeof(bytes);
                 index += 2U) {
                int high = mblink_mercedes_module_scan_hex_value(compact[index]);
                int low = mblink_mercedes_module_scan_hex_value(compact[index + 1U]);
                if (high < 0 || low < 0) { byte_count = 0U; break; }
                bytes[byte_count++] = (uint8_t)((high << 4) | low);
            }
            if (byte_count != 0U) {
                if ((bytes[0] & UINT8_C(0xf0)) == 0U &&
                    (bytes[0] & UINT8_C(0x0f)) != 0U &&
                    (size_t)(bytes[0] & UINT8_C(0x0f)) <= byte_count - 1U) {
                    payload_offset = 1U;
                } else if ((bytes[0] & UINT8_C(0xf0)) == UINT8_C(0x10) &&
                           byte_count >= 3U) {
                    payload_offset = 2U;
                }
                if (byte_count > payload_offset) {
                    const uint8_t *payload = bytes + payload_offset;
                    const size_t payload_length = byte_count - payload_offset;
                    const uint8_t positive = (uint8_t)(request_service + UINT8_C(0x40));
                    if (payload[0] == positive ||
                        (payload_length >= 2U && payload[0] == UINT8_C(0x7f) &&
                         payload[1] == request_service)) {
                        *rx_can_id = id;
                        return true;
                    }
                }
            }
        }
        if (end == NULL) break;
        cursor = end + 1;
    }
    return false;
}

bool mblink_mercedes_module_scan_at_ok(const MblinkElm327Response *response) { return response != NULL && response->result == MBLINK_ELM327_RESULT_OK; }

bool mblink_mercedes_module_scan_accept_adapter_transition(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response,
    MblinkMercedesModuleScanStage next_stage)
{
    if (!mblink_mercedes_module_scan_at_ok(response)) {
        return false;
    }
    scan->stage = next_stage;
    return true;
}

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_accept(MblinkMercedesModuleScan *scan, const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY]; size_t pdu_length = 0U; MblinkUdsResponse uds; MblinkMercedesModuleScanEntry *module; bool present;
    if (scan == NULL || response == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    switch (scan->stage) {
    /* Adapter-control commands are strict: continuing after a rejected setup
     * command would make every following ECU result ambiguous. */
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        if (scan->resume_pending) {
            scan->stage = scan->resume_stage;
            scan->resume_pending = false;
        } else if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED) {
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL;
        } else {
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = (mblink_mercedes_module_scan_uses_target_plan(scan->scope) && !scan->candidate_extended && !scan->candidate_route_locked)
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESET_RECEIVE
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESET_RECEIVE:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_FILTER;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_FILTER:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        /*
         * Public EIS_212/204 traces show the nonstandard 0x612 -> 0x482 route
         * entering the standard UDS extended diagnostic session before
         * variant/identity reads.  Negotiate that transient session only for
         * source-corroborated Mercedes routes; ordinary sweep candidates stay
         * in the default session.
         */
        {
            const MblinkMercedesKnownRoute *route =
                mblink_mercedes_module_scan_known_route(scan);
            scan->stage =
                route != NULL && route->extended_session_evidenced
                    ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION
                    : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION:
        /*
         * A positive or valid negative UDS response proves the exact route is
         * alive.  Lack of a response is not fatal: continue with the bounded
         * read-only presence probes because not every Mercedes ECU requires
         * or accepts the same diagnostic-session transition.
         */
        present = mblink_mercedes_module_scan_decode_uds(
            response, MBLINK_UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL,
            pdu, sizeof(pdu), &pdu_length, &uds);
        if (present)
            (void)mblink_mercedes_module_scan_record_module(scan, false);
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
        if (mblink_mercedes_module_scan_uses_target_plan(scan->scope) &&
            !scan->candidate_extended && !scan->candidate_route_locked) {
            uint32_t learned_rx = 0U;
            if (mblink_mercedes_module_scan_headered_11_route(
                    response, MBLINK_UDS_SERVICE_TESTER_PRESENT, &learned_rx)) {
                scan->candidate_rx = learned_rx;
                scan->candidate_route_locked = true;
                (void)mblink_mercedes_module_scan_record_module(scan, true);
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF;
            } else if (scan->scope ==
                       MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS) {
                mblink_mercedes_module_scan_advance_candidate(scan);
            } else {
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK;
            }
            break;
        }
        present = mblink_mercedes_module_scan_decode_candidate(
            scan, response,
            MBLINK_UDS_SERVICE_TESTER_PRESENT,
            MBLINK_KWP2000_SERVICE_TESTER_PRESENT);
        if (present) {
            (void)mblink_mercedes_module_scan_record_module(scan, true);
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK;
        } else if (scan->scope ==
                       MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS &&
                   mblink_mercedes_module_scan_known_route(scan) == NULL &&
                   !mblink_mercedes_module_scan_is_production_vin_target(scan)) {
            mblink_mercedes_module_scan_advance_candidate(scan);
        } else {
            /*
             * Source-corroborated Mercedes body/chassis routes may stay quiet
             * to TesterPresent in the default diagnostic session.  On those
             * few routes continue with read-only DTC/identity probes instead
             * of throwing away the independently known response CAN ID.
             */
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK;
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
        if (mblink_mercedes_module_scan_uses_target_plan(scan->scope) &&
            !scan->candidate_extended && !scan->candidate_route_locked) {
            uint32_t learned_rx = 0U;
            if (mblink_mercedes_module_scan_headered_11_route(
                    response, MBLINK_UDS_SERVICE_READ_DTC_INFORMATION, &learned_rx)) {
                scan->candidate_rx = learned_rx;
                scan->candidate_route_locked = true;
                (void)mblink_mercedes_module_scan_record_module(scan, false);
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF;
            } else {
                mblink_mercedes_module_scan_enter_vin_fallback(scan);
            }
            break;
        }
        present = mblink_mercedes_module_scan_decode_candidate(
            scan, response,
            MBLINK_UDS_SERVICE_READ_DTC_INFORMATION,
            MBLINK_KWP2000_SERVICE_READ_DTC_BY_STATUS);
        if (present)
            (void)mblink_mercedes_module_scan_record_module(scan, false);
        module = mblink_mercedes_module_scan_find_candidate(scan);
        if (module != NULL)
            mblink_mercedes_module_scan_capture_dtc(module, response);
        if (mblink_mercedes_module_scan_candidate_protocol(scan) ==
            MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
            if (module != NULL)
                mblink_mercedes_module_scan_advance_candidate(scan);
            else if (mblink_mercedes_module_scan_vin_command(scan) != NULL)
                mblink_mercedes_module_scan_enter_vin_fallback(scan);
            else
                mblink_mercedes_module_scan_advance_candidate(scan);
        } else {
            if (module != NULL) {
                scan->stage =
                    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
            } else {
                mblink_mercedes_module_scan_enter_vin_fallback(scan);
            }
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_SET_TIMEOUT:
        if (!mblink_mercedes_module_scan_at_ok(response))
            goto adapter_failure;
        scan->vin_timeout_long = true;
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK:
        if (mblink_mercedes_module_scan_uses_target_plan(scan->scope) &&
            !scan->candidate_extended && !scan->candidate_route_locked) {
            uint32_t learned_rx = 0U;
            if (mblink_mercedes_module_scan_headered_11_route(
                    response, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER, &learned_rx)) {
                scan->candidate_rx = learned_rx;
                scan->candidate_route_locked = true;
                (void)mblink_mercedes_module_scan_record_module(scan, false);
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF;
            } else {
                mblink_mercedes_module_scan_advance_candidate(scan);
            }
            break;
        }
        mblink_mercedes_module_scan_capture_vin(scan, response);
        present = mblink_mercedes_module_scan_decode_vin_probe(scan, response);
        if (present) {
            (void)mblink_mercedes_module_scan_record_module(scan, false);
            if (mblink_mercedes_module_scan_candidate_protocol(scan) ==
                MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
                mblink_mercedes_module_scan_advance_candidate(scan);
            } else {
                scan->stage =
                    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
            }
        } else if (mblink_mercedes_module_scan_has_next_vin_probe(scan)) {
            ++scan->vin_probe_index;
        } else if (mblink_mercedes_module_scan_known_route(scan) != NULL &&
                   mblink_mercedes_module_scan_candidate_protocol(scan) ==
                       MBLINK_MERCEDES_DIAGNOSTIC_UDS) {
            /*
             * F100 is the CAESAR/Vediamo active-diagnostic-information /
             * variant DID shown in published EIS_212 traces.  It is a
             * read-only final presence probe for known non-+8 routes.
             */
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK;
        } else {
            mblink_mercedes_module_scan_advance_candidate(scan);
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK:
        present = mblink_mercedes_module_scan_decode_candidate(
            scan, response,
            MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
            MBLINK_KWP2000_SERVICE_READ_DATA_BY_COMMON_IDENTIFIER);
        if (present) {
            (void)mblink_mercedes_module_scan_record_module(scan, false);
            if (mblink_mercedes_module_scan_candidate_protocol(scan) ==
                MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
                mblink_mercedes_module_scan_advance_candidate(scan);
            } else {
                scan->stage =
                    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
            }
        } else {
            mblink_mercedes_module_scan_advance_candidate(scan);
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY:
        module = mblink_mercedes_module_scan_find_candidate(scan);
        if (module != NULL)
            mblink_mercedes_module_scan_capture_identity(module, response);
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART:
        module = mblink_mercedes_module_scan_find_candidate(scan);
        if (module != NULL) {
            module->spare_part_number_available =
                mblink_mercedes_module_scan_capture_text_did(
                    response, UINT16_C(0xf187),
                    module->spare_part_number,
                    sizeof(module->spare_part_number));
        }
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE:
        module = mblink_mercedes_module_scan_find_candidate(scan);
        if (module != NULL) {
            module->software_number_available =
                mblink_mercedes_module_scan_capture_text_did(
                    response, UINT16_C(0xf188),
                    module->software_number,
                    sizeof(module->software_number));
            mblink_mercedes_module_scan_classify_controller_family(module);
        }
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE:
        module = mblink_mercedes_module_scan_find_candidate(scan);
        if (module != NULL) {
            module->hardware_number_available =
                mblink_mercedes_module_scan_capture_text_did(
                    response, UINT16_C(0xf191),
                    module->hardware_number,
                    sizeof(module->hardware_number));
            mblink_mercedes_module_scan_classify_controller_family(module);
        }
        mblink_mercedes_module_scan_advance_candidate(scan);
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESTORE_TIMEOUT:
        if (!mblink_mercedes_module_scan_at_ok(response))
            goto adapter_failure;
        scan->vin_timeout_long = false;
        mblink_mercedes_module_scan_advance_candidate(scan);
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_SET_TIMEOUT:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_RESTORE_TIMEOUT:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY:
        /* Accepted only by the public identity layer. */
        goto failed_state;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER:
        if (!mblink_mercedes_module_scan_accept_adapter_transition(
                scan, response,
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE))
            goto adapter_failure;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        if (scan->dtc_index >= scan->module_count) goto failed_state;
        {
            const MblinkMercedesKnownRoute *route =
                mblink_mercedes_module_scan_known_entry_route(
                    &scan->modules[scan->dtc_index]);
            scan->stage =
                route != NULL && route->extended_session_evidenced
                    ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION
                    : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE;
        }
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION:
        if (scan->dtc_index >= scan->module_count) goto failed_state;
        /*
         * Cached source-backed routes must recreate the same transient
         * diagnostic-session context used when they were discovered.  Treat a
         * timeout/negative response as non-fatal and let TesterPresent decide
         * whether the saved route is still alive.
         */
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE:
        if (scan->dtc_index >= scan->module_count) goto failed_state;
        present = mblink_mercedes_module_scan_decode_entry(
            &scan->modules[scan->dtc_index], response,
            MBLINK_UDS_SERVICE_TESTER_PRESENT,
            MBLINK_KWP2000_SERVICE_TESTER_PRESENT);
        scan->modules[scan->dtc_index].tester_present_response = present;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ:
        if (scan->dtc_index >= scan->module_count)
            goto failed_state;
        mblink_mercedes_module_scan_capture_dtc(
            &scan->modules[scan->dtc_index], response);
        ++scan->dtc_index;
        scan->stage = scan->single_module_refresh ||
                      scan->dtc_index >= scan->module_count
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE: return MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
    }
    return scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE : MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
adapter_failure: scan->failure = MBLINK_MERCEDES_MODULE_SCAN_RESULT_ADAPTER_ERROR; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED; return scan->failure;
failed_state: scan->failure = MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED; return scan->failure;
}

size_t mblink_mercedes_module_scan_module_count(const MblinkMercedesModuleScan *scan) { return scan != NULL ? scan->module_count : 0U; }

const MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_module_at(const MblinkMercedesModuleScan *scan, size_t index) { return scan != NULL && index < scan->module_count ? &scan->modules[index] : NULL; }

size_t mblink_mercedes_module_scan_entry_dtc_count(
    const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL) return 0U;
    return mblink_mercedes_module_scan_entry_protocol(module) ==
               MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
        ? module->kwp_dtcs.count : module->dtcs.count;
}

size_t mblink_mercedes_module_scan_total_dtc_count(
    const MblinkMercedesModuleScan *scan)
{
    size_t total = 0U;
    size_t index;
    if (scan == NULL) return 0U;
    for (index = 0U; index < scan->module_count; ++index)
        total += mblink_mercedes_module_scan_entry_dtc_count(
            &scan->modules[index]);
    return total;
}

size_t mblink_mercedes_module_scan_fresh_response_count(
    const MblinkMercedesModuleScan *scan)
{
    size_t count = 0U;
    size_t index;
    if (scan == NULL) return 0U;
    for (index = 0U; index < scan->module_count; ++index) {
        const MblinkMercedesModuleScanEntry *module = &scan->modules[index];
        if (module->tester_present_response ||
            (module->dtc_result != MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED &&
             module->dtc_result != MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE)) {
            ++count;
        }
    }
    return count;
}

size_t mblink_mercedes_module_scan_classified_count(
    const MblinkMercedesModuleScan *scan)
{
    size_t count = 0U;
    size_t index;
    if (scan == NULL) return 0U;
    for (index = 0U; index < scan->module_count; ++index) {
        const MblinkMercedesModuleScanEntry *module = &scan->modules[index];
        /*
         * Returned textual identity is the strongest classifier, but an
         * independently source-backed physical route can also establish the
         * module class. In particular Mercedes definitions explicitly name
         * 0x7E1/0x7E9 as the GS gearbox-control diagnostic route.
         */
        if (module->definition != NULL ||
            module->kind != MBLINK_MERCEDES_MODULE_OTHER) {
            ++count;
        }
    }
    return count;
}

bool mblink_mercedes_module_scan_identity_first_active(
    const MblinkMercedesModuleScan *scan)
{
    return scan != NULL &&
           scan->scope != MBLINK_MERCEDES_MODULE_SCAN_CACHED &&
           scan->dtc_index == MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL;
}

void mblink_mercedes_module_scan_mark_identity_first(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;
    scan->dtc_index = MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL;
    scan->vin_probe_index = 0U;
}

bool mblink_mercedes_module_scan_kwp_identity_mode(
    const MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return false;
    return mblink_mercedes_module_scan_candidate_protocol(scan) ==
               MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 ||
           scan->vin_probe_index >=
               MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER;
}

size_t mblink_mercedes_module_scan_kwp_identity_index(
    const MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return 0U;
    if (scan->vin_probe_index >=
        MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER) {
        return scan->vin_probe_index -
            MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER;
    }
    return scan->vin_probe_index;
}

uint8_t mblink_mercedes_module_scan_kwp_identity_option(
    const MblinkMercedesModuleScan *scan)
{
    /*
     * 0x87 is mandatory DCX/MMC ECU identification in Daimler KWP2000 and
     * carries origin, supplier, ECU-identification/diagnostic-version bytes,
     * hardware version, software version and part number.  0x86 (DCS ECU
     * identification) and 0x89 (diagnostic variant code) are safe read-only
     * fallbacks for older/variant implementations.
     */
    static const uint8_t options[] = {
        UINT8_C(0x87), UINT8_C(0x86), UINT8_C(0x89)
    };
    const size_t index =
        mblink_mercedes_module_scan_kwp_identity_index(scan);
    return index < INFILTRATR_ARRAY_LENGTH(options)
        ? options[index] : 0U;
}

bool mblink_mercedes_module_scan_format_kwp_identity_command(
    uint8_t option,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    int count;
    if (buffer == NULL || buffer_size == 0U || written == NULL ||
        option == 0U) {
        if (written != NULL) *written = 0U;
        return false;
    }
    count = snprintf(buffer, buffer_size, "1A%02X", (unsigned int)option);
    if (count < 0 || (size_t)count >= buffer_size) {
        buffer[0] = '\0';
        *written = 0U;
        return false;
    }
    *written = (size_t)count;
    return true;
}

MblinkMercedesIdentityResponseState
mblink_mercedes_module_scan_identity_response_state(
    const MblinkElm327Response *response,
    uint8_t request_service,
    uint8_t positive_service,
    uint8_t expected_option,
    uint8_t *negative_response_code)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;

    if (negative_response_code != NULL) *negative_response_code = 0U;
    if (response == NULL || response->result != MBLINK_ELM327_RESULT_OK ||
        mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
            MBLINK_ELM327_CAN_RESULT_OK ||
        pdu_length == 0U) {
        return MBLINK_MERCEDES_IDENTITY_RESPONSE_NONE;
    }

    if (pdu[0] == positive_service) {
        if (expected_option != 0U &&
            (pdu_length < 2U || pdu[1] != expected_option)) {
            return MBLINK_MERCEDES_IDENTITY_RESPONSE_NONE;
        }
        return MBLINK_MERCEDES_IDENTITY_RESPONSE_POSITIVE;
    }

    if (pdu_length >= 3U && pdu[0] == UINT8_C(0x7f) &&
        pdu[1] == request_service) {
        const uint8_t nrc = pdu[2];
        if (negative_response_code != NULL)
            *negative_response_code = nrc;
        /*
         * Daimler KWP gateway NRC A0 means the gateway forwarded the request
         * but the destination did not respond; A1 means the destination
         * address is unknown.  Neither proves a module exists at the target.
         */
        if (nrc == UINT8_C(0xa0) || nrc == UINT8_C(0xa1))
            return MBLINK_MERCEDES_IDENTITY_RESPONSE_GATEWAY_MISS;
        return MBLINK_MERCEDES_IDENTITY_RESPONSE_NEGATIVE;
    }
    return MBLINK_MERCEDES_IDENTITY_RESPONSE_NONE;
}

bool mblink_mercedes_module_scan_copy_printable(
    const uint8_t *data,
    size_t data_length,
    char *destination,
    size_t destination_capacity)
{
    size_t length;
    size_t index;

    if (destination == NULL || destination_capacity == 0U) return false;
    destination[0] = '\0';
    if (data == NULL || data_length == 0U) return false;

    length = data_length;
    while (length != 0U &&
           (data[length - 1U] == 0U ||
            data[length - 1U] == UINT8_C(0xff) ||
            data[length - 1U] == (uint8_t)' ')) {
        --length;
    }
    if (length == 0U) return false;
    if (length >= destination_capacity) length = destination_capacity - 1U;
    for (index = 0U; index < length; ++index) {
        if (data[index] < UINT8_C(0x20) || data[index] > UINT8_C(0x7e)) {
            destination[0] = '\0';
            return false;
        }
        destination[index] = (char)data[index];
    }
    destination[length] = '\0';
    return true;
}

bool mblink_mercedes_module_scan_decode_kwp_identity_record(
    const MblinkElm327Response *response,
    uint8_t option,
    MblinkKwp2000EcuIdentificationRecord *record)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;

    if (record == NULL || response == NULL ||
        response->result != MBLINK_ELM327_RESULT_OK ||
        mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
            MBLINK_ELM327_CAN_RESULT_OK) {
        return false;
    }
    return mblink_kwp2000_decode_read_ecu_identification_response(
               pdu, pdu_length, option, record) ==
           MBLINK_KWP2000_RESULT_OK;
}

bool mblink_mercedes_module_scan_capture_kwp_87(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response)
{
    MblinkKwp2000EcuIdentificationRecord record;
    char part[sizeof(module->spare_part_number)];

    if (module == NULL ||
        !mblink_mercedes_module_scan_decode_kwp_identity_record(
            response, UINT8_C(0x87), &record) ||
        record.data_length < 10U) {
        return false;
    }

    /*
     * Daimler DCX/MMC $87 payload (after 5A 87):
     *   [0] origin, [1] supplier,
     *   [2] ECU identification / variant byte,
     *   [3] diagnostic version,
     *   [4] reserved,
     *   [5..6] hardware version,
     *   [7..9] software version,
     *   [10..19] corporate part number.
     * Keep the numeric identity tuple verbatim.  Its interpretation is
     * ECU-family/DDT specific, so do not manufacture an EGS generation from a
     * number alone.
     */
    (void)snprintf(
        module->identity, sizeof(module->identity),
        "KWP ECU S%02X V%02X D%02X HW%02X.%02X SW%02X.%02X.%02X",
        (unsigned int)record.data[1],
        (unsigned int)record.data[2],
        (unsigned int)record.data[3],
        (unsigned int)record.data[5],
        (unsigned int)record.data[6],
        (unsigned int)record.data[7],
        (unsigned int)record.data[8],
        (unsigned int)record.data[9]);
    module->identity_available = true;

    (void)snprintf(
        module->hardware_number, sizeof(module->hardware_number),
        "%02X.%02X",
        (unsigned int)record.data[5],
        (unsigned int)record.data[6]);
    module->hardware_number_available = true;
    (void)snprintf(
        module->software_number, sizeof(module->software_number),
        "%02X.%02X.%02X",
        (unsigned int)record.data[7],
        (unsigned int)record.data[8],
        (unsigned int)record.data[9]);
    module->software_number_available = true;

    if (record.data_length > 10U &&
        mblink_mercedes_module_scan_copy_printable(
            record.data + 10U, record.data_length - 10U,
            part, sizeof(part))) {
        (void)snprintf(
            module->spare_part_number,
            sizeof(module->spare_part_number), "%s", part);
        module->spare_part_number_available = true;
    }
    mblink_mercedes_module_scan_classify_identity(module);
    return true;
}

bool mblink_mercedes_module_scan_bcd_part_number(
    const uint8_t data[5],
    char *destination,
    size_t destination_capacity)
{
    size_t index;
    if (data == NULL || destination == NULL || destination_capacity < 11U)
        return false;
    for (index = 0U; index < 5U; ++index) {
        if ((data[index] & UINT8_C(0x0f)) > UINT8_C(9) ||
            ((data[index] >> 4U) & UINT8_C(0x0f)) > UINT8_C(9)) {
            destination[0] = '\0';
            return false;
        }
    }
    (void)snprintf(
        destination, destination_capacity,
        "%02X%02X%02X%02X%02X",
        (unsigned int)data[0], (unsigned int)data[1],
        (unsigned int)data[2], (unsigned int)data[3],
        (unsigned int)data[4]);
    return true;
}

bool mblink_mercedes_module_scan_capture_kwp_86(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response)
{
    MblinkKwp2000EcuIdentificationRecord record;
    char part[sizeof(module->spare_part_number)];

    if (module == NULL ||
        !mblink_mercedes_module_scan_decode_kwp_identity_record(
            response, UINT8_C(0x86), &record) ||
        record.data_length < 12U) {
        return false;
    }

    (void)snprintf(
        module->identity, sizeof(module->identity),
        "KWP DCS S%02X V%02X D%02X HW-W%02X/%02X SW-W%02X/%02X",
        (unsigned int)record.data[9],
        (unsigned int)record.data[10],
        (unsigned int)record.data[11],
        (unsigned int)record.data[5],
        (unsigned int)record.data[6],
        (unsigned int)record.data[7],
        (unsigned int)record.data[8]);
    module->identity_available = true;

    (void)snprintf(
        module->hardware_number, sizeof(module->hardware_number),
        "build-W%02X/%02X",
        (unsigned int)record.data[5],
        (unsigned int)record.data[6]);
    module->hardware_number_available = true;
    (void)snprintf(
        module->software_number, sizeof(module->software_number),
        "build-W%02X/%02X",
        (unsigned int)record.data[7],
        (unsigned int)record.data[8]);
    module->software_number_available = true;

    if (mblink_mercedes_module_scan_bcd_part_number(
            record.data, part, sizeof(part))) {
        (void)snprintf(
            module->spare_part_number,
            sizeof(module->spare_part_number), "%s", part);
        module->spare_part_number_available = true;
    }
    mblink_mercedes_module_scan_classify_identity(module);
    return true;
}

bool mblink_mercedes_module_scan_capture_kwp_89(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response)
{
    MblinkKwp2000EcuIdentificationRecord record;
    size_t index;
    size_t offset = 0U;

    if (module == NULL ||
        !mblink_mercedes_module_scan_decode_kwp_identity_record(
            response, UINT8_C(0x89), &record) ||
        record.data_length == 0U) {
        return false;
    }

    offset = (size_t)snprintf(
        module->identity, sizeof(module->identity), "KWP variant ");
    if (offset >= sizeof(module->identity)) offset = sizeof(module->identity) - 1U;
    for (index = 0U;
         index < record.data_length && index < 8U &&
         offset + 2U < sizeof(module->identity);
         ++index) {
        const int count = snprintf(
            module->identity + offset,
            sizeof(module->identity) - offset,
            "%02X", (unsigned int)record.data[index]);
        if (count < 0 || (size_t)count >= sizeof(module->identity) - offset)
            break;
        offset += (size_t)count;
    }
    module->identity_available = true;
    mblink_mercedes_module_scan_classify_identity(module);
    return true;
}

bool mblink_mercedes_module_scan_capture_kwp_identity(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response,
    uint8_t option)
{
    switch (option) {
    case UINT8_C(0x87):
        return mblink_mercedes_module_scan_capture_kwp_87(module, response);
    case UINT8_C(0x86):
        return mblink_mercedes_module_scan_capture_kwp_86(module, response);
    case UINT8_C(0x89):
        return mblink_mercedes_module_scan_capture_kwp_89(module, response);
    default:
        return false;
    }
}

bool mblink_mercedes_module_scan_is_kwp_transmission(
    const MblinkMercedesModuleScanEntry *module)
{
    return module != NULL && module->definition != NULL &&
           strcmp(module->definition->key, "transmission-vgs") == 0 &&
           mblink_mercedes_module_scan_entry_protocol(module) ==
               MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
}

void mblink_mercedes_module_scan_start_kwp_identity_fallback(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;
    scan->vin_probe_index =
        MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
}

bool mblink_mercedes_module_scan_advance_kwp_identity(
    MblinkMercedesModuleScan *scan)
{
    const bool fallback = scan != NULL &&
        scan->vin_probe_index >=
            MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER;
    const size_t next =
        mblink_mercedes_module_scan_kwp_identity_index(scan) + 1U;

    if (scan == NULL || next >= 3U) return false;
    scan->vin_probe_index = fallback
        ? MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER + next
        : next;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
    return true;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin(MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_gateway(MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_gateway_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_mobile_census(
    MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_mobile_census_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_full(MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_full_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_command(
    const MblinkMercedesModuleScan *scan,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    if ((scan != NULL && scan->scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED &&
         scan->dtc_index < scan->module_count &&
         scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY) ||
        (mblink_mercedes_module_scan_identity_first_active(scan) &&
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY &&
        mblink_mercedes_module_scan_kwp_identity_mode(scan))) {
        return mblink_mercedes_module_scan_format_kwp_identity_command(
                   mblink_mercedes_module_scan_kwp_identity_option(scan),
                   buffer, buffer_size, written)
            ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK
            : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    }
    return mblink_mercedes_module_scan_command_core(
        scan, buffer, buffer_size, written);
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_accept_identity(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response)
{
    MblinkMercedesModuleScanEntry *module;
    MblinkMercedesIdentityResponseState state;
    uint8_t nrc = 0U;

    if (mblink_mercedes_module_scan_kwp_identity_mode(scan)) {
        const uint8_t option =
            mblink_mercedes_module_scan_kwp_identity_option(scan);
        if (option == UINT8_C(0x87)) scan->kwp_identity_captured = false;
        state = mblink_mercedes_module_scan_identity_response_state(
            response, MBLINK_KWP2000_SERVICE_READ_ECU_IDENTIFICATION,
            UINT8_C(0x5a), option, &nrc);

        if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_GATEWAY_MISS) {
            mblink_mercedes_module_scan_advance_candidate(scan);
        } else if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_POSITIVE) {
            module = mblink_mercedes_module_scan_record_module(scan, false);
            if (module != NULL) {
                module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
                if (!scan->kwp_identity_captured)
                    scan->kwp_identity_captured =
                        mblink_mercedes_module_scan_capture_kwp_identity(
                            module, response, option);
            }
            /* Collect the variant too, and never stop on a truncated 5A. */
            if ((mblink_mercedes_module_scan_is_kwp_transmission(module) ||
                 !scan->kwp_identity_captured) &&
                mblink_mercedes_module_scan_advance_kwp_identity(scan))
                return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
            mblink_mercedes_module_scan_advance_candidate(scan);
        } else {
            if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_NEGATIVE) {
                /* A real non-gateway negative reply still proves an ECU is there. */
                module = mblink_mercedes_module_scan_record_module(scan, false);
                if (module != NULL)
                    module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
            }
            if (!mblink_mercedes_module_scan_advance_kwp_identity(scan)) {
                module = mblink_mercedes_module_scan_find_candidate(scan);
                if (module != NULL)
                    mblink_mercedes_module_scan_advance_candidate(scan);
                else {
                    scan->vin_probe_index = 0U;
                    scan->stage =
                        MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
                }
            }
        }
        return scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE
            ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE
            : MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
    }

    /* UDS identity is F197 system name, the strongest normal family label. */
    if (mblink_mercedes_module_scan_uses_target_plan(scan->scope) &&
        !scan->candidate_extended && !scan->candidate_route_locked) {
        uint32_t learned_rx = 0U;
        if (mblink_mercedes_module_scan_headered_11_route(
                response, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
                &learned_rx)) {
            scan->candidate_rx = learned_rx;
            scan->candidate_route_locked = true;
            (void)mblink_mercedes_module_scan_record_module(scan, false);
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF;
            return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
        }
    }

    state = mblink_mercedes_module_scan_identity_response_state(
        response, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
        UINT8_C(0x62), 0U, &nrc);
    if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_GATEWAY_MISS) {
        mblink_mercedes_module_scan_advance_candidate(scan);
    } else if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_POSITIVE) {
        module = mblink_mercedes_module_scan_record_module(scan, false);
        if (module != NULL) {
            module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_UDS;
            mblink_mercedes_module_scan_capture_identity(module, response);
        }
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART;
    } else if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_NEGATIVE) {
        /*
         * A non-A0/A1 response proves a destination replied.  On a route whose
         * protocol is already known to be UDS, keep that ECU and continue with
         * the remaining metadata.  On an unresolved gateway route, try the
         * Daimler KWP identity service before deciding the protocol.
         */
        module = mblink_mercedes_module_scan_record_module(scan, false);
        if (mblink_mercedes_module_scan_known_route(scan) != NULL &&
            mblink_mercedes_module_scan_candidate_protocol(scan) ==
                MBLINK_MERCEDES_DIAGNOSTIC_UDS) {
            if (module != NULL)
                module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_UDS;
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART;
        } else {
            mblink_mercedes_module_scan_start_kwp_identity_fallback(scan);
        }
    } else if (mblink_mercedes_module_scan_known_route(scan) == NULL) {
        /* Mixed-protocol ECU behind the N93/CGW path: try KWP identity too. */
        mblink_mercedes_module_scan_start_kwp_identity_fallback(scan);
    } else {
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
    }

    return scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE
        ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE
        : MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_accept(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response)
{
    const MblinkMercedesModuleScanStage before =
        scan != NULL ? scan->stage : MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED;
    const bool identity_first =
        mblink_mercedes_module_scan_identity_first_active(scan);
    MblinkMercedesModuleScanResult result;

    if (scan == NULL || response == NULL)
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;

    if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED &&
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY &&
        scan->dtc_index < scan->module_count) {
        if (!scan->kwp_identity_captured)
            scan->kwp_identity_captured =
                mblink_mercedes_module_scan_capture_kwp_identity(
                    &scan->modules[scan->dtc_index], response,
                    mblink_mercedes_module_scan_kwp_identity_option(scan));
        /* No retries: unsupported, malformed and ELM NO DATA replies advance. */
        if (++scan->vin_probe_index >= 3U)
            scan->stage =
                MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_RESTORE_TIMEOUT;
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
    }

    if (identity_first &&
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY) {
        return mblink_mercedes_module_scan_accept_identity(scan, response);
    }

    result = mblink_mercedes_module_scan_accept_core(scan, response);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK &&
        scan->scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED &&
        before == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE &&
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE &&
        scan->dtc_index < scan->module_count &&
        mblink_mercedes_module_scan_is_kwp_transmission(
            &scan->modules[scan->dtc_index])) {
        scan->vin_probe_index = 0U;
        /* These extra reads fill missing metadata without replacing a saved
         * family label with an unresolved numeric KWP tuple. All raw replies
         * still reach the shared recorder before this parser runs. */
        scan->kwp_identity_captured =
            scan->modules[scan->dtc_index].identity_available;
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_SET_TIMEOUT;
    }
    if (result != MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK || !identity_first)
        return result;

    /*
     * Adapter route setup and an optional UDS session transition are allowed
     * before identification.  The first ECU-content request after that setup
     * is identity, not TesterPresent/DTC.
     */
    if ((before ==
             MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK ||
         before ==
             MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE ||
         before ==
             MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION ||
         before ==
             MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF) &&
        scan->stage ==
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT) {
        scan->vin_probe_index = 0U;
        scan->stage =
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
        return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
    }

    /*
     * If identity was unavailable and TesterPresent later proves the ECU is
     * alive, still defer its DTC read until the post-discovery DTC pass.  This
     * keeps the scan globally identity-first instead of interleaving fault
     * reads with module census.
     */
    if (before ==
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT &&
        scan->stage ==
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK &&
        mblink_mercedes_module_scan_find_candidate(scan) != NULL) {
        mblink_mercedes_module_scan_advance_candidate(scan);
        return scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE
            ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE
            : MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
    }

    return result;
}

bool mblink_mercedes_module_scan_resume_after_interruption(
    MblinkMercedesModuleScan *scan)
{
    bool dtc_pass;
    uint32_t tx, rx;
    bool extended;
    bool repeated;
    if (scan == NULL || scan->recovery_count >= 3U ||
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE ||
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED)
        return false;
    dtc_pass = scan->scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED ||
        (scan->resume_pending && scan->resume_stage ==
            MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL) ||
        (scan->stage >= MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL);
    if (dtc_pass && scan->dtc_index >= scan->module_count) return false;
    tx = dtc_pass ? scan->modules[scan->dtc_index].tx_can_id : scan->candidate_tx;
    rx = dtc_pass ? scan->modules[scan->dtc_index].rx_can_id : scan->candidate_rx;
    extended = dtc_pass ? scan->modules[scan->dtc_index].extended_id
                        : scan->candidate_extended;
    repeated = scan->recovery_count != 0U && scan->recovery_tx == tx &&
        scan->recovery_rx == rx && scan->recovery_extended == extended;
    ++scan->recovery_count;
    scan->recovery_tx = tx;
    scan->recovery_rx = rx;
    scan->recovery_extended = extended;
    scan->vin_timeout_long = false;
    scan->vin_probe_index = 0U;
    if (repeated) {
        if (dtc_pass) {
            ++scan->dtc_index;
            if (scan->single_module_refresh || scan->dtc_index >= scan->module_count) {
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE;
                return false;
            }
        } else {
            mblink_mercedes_module_scan_advance_candidate(scan);
            if (scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE)
                return false;
            dtc_pass = scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL;
        }
    }
    scan->resume_stage = dtc_pass
        ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL
        : (scan->candidate_extended
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER);
    scan->resume_pending = true;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    return true;
}

bool mblink_mercedes_module_scan_begin_late_transmission(
    MblinkMercedesModuleScan *scan)
{
    size_t index;
    MblinkMercedesModuleScanEntry *entry;
    if (scan == NULL) return false;
    for (index = 0U; index < scan->module_count; ++index) {
        entry = &scan->modules[index];
        if (!entry->extended_id && entry->tx_can_id == UINT32_C(0x7e1) &&
            entry->rx_can_id == UINT32_C(0x7e9)) break;
    }
    if (index == scan->module_count) {
        if (index >= MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES) return false;
        entry = &scan->modules[scan->module_count++];
        memset(entry, 0, sizeof(*entry));
        entry->tx_can_id = UINT32_C(0x7e1);
        entry->rx_can_id = UINT32_C(0x7e9);
        entry->protocol = MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
        entry->definition = mblink_mercedes_module_definition_for_key("transmission-vgs");
        if (entry->definition != NULL) {
            entry->kind = entry->definition->kind;
            entry->identification_status = entry->definition->status;
        }
    }
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_CACHED;
    scan->dtc_index = index;
    scan->single_module_refresh = true;
    scan->resume_pending = false;
    scan->vin_timeout_long = false;
    scan->vin_probe_index = 0U;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    return true;
}
