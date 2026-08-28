// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Bounded read-only Mercedes CAN/UDS module discovery and DTC inventory.
 *
 * The scan never sends coding, programming, security-access, routine-control,
 * reset or clear-DTC services. It probes physical diagnostic endpoints using
 * TesterPresent, ReadDTCInformation and ReadDataByIdentifier only, then reads
 * each responding ECU's DTC memory. The automatic iPhone quick pass is bounded
 * to the standard 11-bit EOBD physical range. Gateway discovery and the
 * complete Mercedes-owned 11/29-bit target plan are explicit wider scopes;
 * they are never silently substituted for the mobile quick pass. A requested
 * full sweep consumes mblink_discover_full_sweep_plan(); LINK defines the plan
 * contract and generic transport/safety machinery, while MBLINK owns the map.
 */
#ifndef MBLINK_MERCEDES_MODULE_SCAN_H
#define MBLINK_MERCEDES_MODULE_SCAN_H

#include "mblink/discover.h"
#include "mblink/elm327.h"
#include "mblink/elm327_can.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_module_catalog.h"
#include "mblink/uds.h"
#include "mblink/uds_dtc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES 64U
#define MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY 512U

typedef enum MblinkMercedesModuleScanResult {
    MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK = 0,
    MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE,
    MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT,
    MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL,
    MBLINK_MERCEDES_MODULE_SCAN_RESULT_ADAPTER_ERROR,
    MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE
} MblinkMercedesModuleScanResult;

typedef enum MblinkMercedesModuleDtcResult {
    MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED = 0,
    MBLINK_MERCEDES_MODULE_DTC_AVAILABLE,
    MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE,
    MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE,
    MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE
} MblinkMercedesModuleDtcResult;

typedef enum MblinkMercedesModuleScanScope {
    MBLINK_MERCEDES_MODULE_SCAN_QUICK = 0,
    MBLINK_MERCEDES_MODULE_SCAN_GATEWAY,
    MBLINK_MERCEDES_MODULE_SCAN_FULL,
    MBLINK_MERCEDES_MODULE_SCAN_CACHED
} MblinkMercedesModuleScanScope;

static inline const char *mblink_mercedes_module_scan_scope_name(
    MblinkMercedesModuleScanScope scope)
{
    switch (scope) {
    case MBLINK_MERCEDES_MODULE_SCAN_QUICK: return "quick";
    case MBLINK_MERCEDES_MODULE_SCAN_GATEWAY: return "gateway-census";
    case MBLINK_MERCEDES_MODULE_SCAN_FULL: return "full-forensic";
    case MBLINK_MERCEDES_MODULE_SCAN_CACHED: return "saved-profile";
    }
    return "unknown";
}

typedef enum MblinkMercedesModuleScanStage {
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11 = 0,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESET_RECEIVE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_FILTER,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED
} MblinkMercedesModuleScanStage;

typedef struct MblinkMercedesModuleScanEntry {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool extended_id;
    MblinkMercedesModuleKind kind;
    bool tester_present_response;
    bool identity_available;
    char identity[64];
    bool spare_part_number_available;
    char spare_part_number[64];
    bool software_number_available;
    char software_number[64];
    bool hardware_number_available;
    char hardware_number[64];
    const MblinkMercedesModuleDefinition *definition;
    MblinkMercedesDefinitionStatus identification_status;
    MblinkMercedesModuleDtcResult dtc_result;
    MblinkUdsResult dtc_uds_result;
    uint8_t dtc_negative_response_code;
    MblinkUdsDtcList dtcs;
} MblinkMercedesModuleScanEntry;

typedef struct MblinkMercedesModuleScan {
    MblinkMercedesModuleScanStage stage;
    MblinkMercedesModuleScanResult failure;
    MblinkMercedesModuleScanEntry modules[MBLINK_MERCEDES_MODULE_SCAN_MAX_MODULES];
    size_t module_count;
    bool truncated;
    MblinkMercedesModuleScanScope scope;
    size_t full_target_index;
    uint16_t gateway_target;
    uint32_t candidate_tx;
    uint32_t candidate_rx;
    bool candidate_extended;
    bool candidate_route_locked;
    size_t dtc_index;
} MblinkMercedesModuleScan;

static inline const char *mblink_mercedes_module_scan_result_name(MblinkMercedesModuleScanResult result)
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

static inline const char *mblink_mercedes_module_scan_stage_name(MblinkMercedesModuleScanStage stage)
{
    switch (stage) {
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
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT: return "discover-tester-present";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK: return "discover-dtc-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: return "discover-vin-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY: return "discover-system-name";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART: return "discover-spare-part";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE: return "discover-software-number";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE: return "discover-hardware-number";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29: return "initialise-29-bit-can";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29: return "29-bit-headers-off";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL: return "fault-set-protocol";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER: return "fault-set-header";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE: return "fault-set-receive";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE: return "validate-saved-module";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return "read-module-faults";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

static inline const char *mblink_mercedes_module_scan_module_name(const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL) return "Mercedes ECU";
    if (module->definition != NULL) return module->definition->display_name;
    if (!module->extended_id && module->tx_can_id == UINT32_C(0x7e0)) return "Engine ECU";
    if (module->identity_available && module->identity[0] != '\0') return module->identity;
    if (!module->extended_id && module->tx_can_id == UINT32_C(0x7e1)) return "Secondary EOBD powertrain ECU";
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

static inline MblinkMercedesModuleKind mblink_mercedes_module_scan_kind(uint32_t tx_can_id, bool extended_id)
{
    if (!extended_id && tx_can_id == UINT32_C(0x7e0)) return MBLINK_MERCEDES_MODULE_ENGINE;
    if (!extended_id && tx_can_id == UINT32_C(0x7e1)) return MBLINK_MERCEDES_MODULE_OTHER;
    return MBLINK_MERCEDES_MODULE_OTHER;
}

static inline bool mblink_mercedes_module_scan_write_command(const char *command, char *buffer, size_t buffer_size, size_t *written)
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

static inline bool mblink_mercedes_module_scan_format_can_command(const char *prefix, uint32_t id, bool extended_id, char *buffer, size_t buffer_size, size_t *written)
{
    int count;
    if (written != NULL) *written = 0U;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (prefix == NULL || buffer == NULL || written == NULL) return false;
    count = extended_id ? snprintf(buffer, buffer_size, "%s%08X", prefix, (unsigned int)id) : snprintf(buffer, buffer_size, "%s%03X", prefix, (unsigned int)id);
    if (count < 0 || (size_t)count >= buffer_size) return false;
    *written = (size_t)count;
    return true;
}

static inline void mblink_mercedes_module_scan_set_11_candidate(MblinkMercedesModuleScan *scan, uint32_t tx)
{
    scan->candidate_tx = tx;
    scan->candidate_rx = tx + UINT32_C(8);
    scan->candidate_extended = false;
    scan->candidate_route_locked = false;
}

static inline bool mblink_mercedes_module_scan_set_gateway_target(
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
    return true;
}

static inline size_t mblink_mercedes_module_scan_planned_target_count(
    MblinkMercedesModuleScanScope scope)
{
    switch (scope) {
    case MBLINK_MERCEDES_MODULE_SCAN_QUICK:
        return 8U;
    case MBLINK_MERCEDES_MODULE_SCAN_GATEWAY:
        /* 8 EOBD endpoints plus 256 logical targets minus tester address F1. */
        return 263U;
    case MBLINK_MERCEDES_MODULE_SCAN_FULL: {
        const link_discover_sweep_plan *plan =
            mblink_discover_full_sweep_plan();
        return link_discover_sweep_plan_is_valid(plan)
            ? plan->target_count : 0U;
    }
    case MBLINK_MERCEDES_MODULE_SCAN_CACHED:
        return 0U;
    }
    return 0U;
}

static inline bool mblink_mercedes_module_scan_set_full_target(
    MblinkMercedesModuleScan *scan,
    size_t index)
{
    const link_discover_sweep_plan *plan;
    link_discover_sweep_target target;

    if (scan == NULL) return false;
    plan = mblink_discover_full_sweep_plan();
    if (!link_discover_sweep_plan_target_at(plan, index, &target) ||
        target.bitrate != 500000U) {
        return false;
    }
    scan->full_target_index = index;
    scan->candidate_tx = target.tx_can_id;
    scan->candidate_rx = target.rx_can_id;
    scan->candidate_extended = target.extended_id;
    scan->candidate_route_locked = target.extended_id;
    return true;
}

static inline void mblink_mercedes_module_scan_finish_discovery(MblinkMercedesModuleScan *scan)
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

static inline void mblink_mercedes_module_scan_advance_candidate(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;

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
            mblink_discover_full_sweep_plan();
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
        scan->stage = scan->candidate_extended
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS;
    }
}

static inline MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_record_module(MblinkMercedesModuleScan *scan, bool tester_present)
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
    module->kind = mblink_mercedes_module_scan_kind(scan->candidate_tx, scan->candidate_extended);
    module->tester_present_response = tester_present;
    module->identification_status = MBLINK_MERCEDES_DEFINITION_CANDIDATE;
    module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED;
    return module;
}


static inline MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_find_candidate(
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

static inline bool mblink_mercedes_module_scan_capture_text_did(
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

static inline void mblink_mercedes_module_scan_classify_identity(
    MblinkMercedesModuleScanEntry *module)
{
    const MblinkMercedesModuleDefinition *definition;

    if (module == NULL || !module->identity_available ||
        module->identity[0] == '\0') {
        return;
    }
    definition =
        mblink_mercedes_c207_module_definition_for_identity(module->identity);
    if (definition == NULL) return;
    module->definition = definition;
    module->kind = definition->kind;
    module->identification_status = definition->status;
}

static inline void mblink_mercedes_module_scan_capture_identity(
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

static inline bool mblink_mercedes_module_scan_decode_uds(const MblinkElm327Response *response, uint8_t service, uint8_t *pdu, size_t pdu_capacity, size_t *pdu_length, MblinkUdsResponse *uds)
{
    MblinkUdsResult result;
    if (response == NULL || pdu == NULL || pdu_length == NULL || uds == NULL || response->result != MBLINK_ELM327_RESULT_OK) return false;
    if (mblink_elm327_can_decode_pdu(response, pdu, pdu_capacity, pdu_length) != MBLINK_ELM327_CAN_RESULT_OK) return false;
    result = mblink_uds_decode_response(service, pdu, *pdu_length, uds);
    return result == MBLINK_UDS_RESULT_OK || result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE;
}

static inline void mblink_mercedes_module_scan_capture_dtc(MblinkMercedesModuleScanEntry *module, const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkUdsResponse generic;
    MblinkUdsResult result;
    if (module == NULL || response == NULL) return;
    memset(&module->dtcs, 0, sizeof(module->dtcs));
    module->dtc_negative_response_code = 0U;
    if (response->result != MBLINK_ELM327_RESULT_OK) { module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NO_RESPONSE; return; }
    if (mblink_elm327_can_decode_pdu(response, pdu, sizeof(pdu), &pdu_length) != MBLINK_ELM327_CAN_RESULT_OK) { module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE; return; }
    result = mblink_uds_decode_response(MBLINK_UDS_SERVICE_READ_DTC_INFORMATION, pdu, pdu_length, &generic);
    module->dtc_uds_result = result;
    if (result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) { module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NEGATIVE_RESPONSE; module->dtc_negative_response_code = generic.negative_response_code; return; }
    if (result != MBLINK_UDS_RESULT_OK) { module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE; return; }
    result = mblink_uds_decode_report_dtcs_by_status_mask_response(pdu, pdu_length, &module->dtcs);
    module->dtc_uds_result = result;
    module->dtc_result = result == MBLINK_UDS_RESULT_OK ? MBLINK_MERCEDES_MODULE_DTC_AVAILABLE : MBLINK_MERCEDES_MODULE_DTC_INVALID_RESPONSE;
}

static inline MblinkMercedesModuleScanResult mblink_mercedes_module_scan_begin(MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    memset(scan, 0, sizeof(*scan));
    scan->scope = MBLINK_MERCEDES_MODULE_SCAN_QUICK;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    mblink_mercedes_module_scan_set_11_candidate(scan, UINT32_C(0x7e0));
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

static inline MblinkMercedesModuleScanResult
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

static inline MblinkMercedesModuleScanResult
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
        destination->dtc_negative_response_code = 0U;
        memset(&destination->dtcs, 0, sizeof(destination->dtcs));
        if (destination->identity_available)
            mblink_mercedes_module_scan_classify_identity(destination);
    }
    scan->dtc_index = 0U;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

static inline MblinkMercedesModuleScanResult mblink_mercedes_module_scan_begin_full(
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

static inline uint64_t mblink_mercedes_module_scan_timeout_ms(const MblinkMercedesModuleScan *scan)
{
    /*
     * The ELM's own ATST timer decides when an unanswered CAN request becomes
     * NO DATA.  Keep the host/BLE watchdog comfortably above that value so a
     * slow CoreBluetooth delivery does not turn an ordinary module miss into
     * a fatal diagnostic-session timeout.
     */
    if (scan == NULL) return UINT64_C(4000);
    switch (scan->stage) {
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE:
        return UINT64_C(4000);
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE:
        return UINT64_C(4000);
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return UINT64_C(5000);
    default: return UINT64_C(4000);
    }
}

static inline MblinkMercedesModuleScanResult mblink_mercedes_module_scan_command(const MblinkMercedesModuleScan *scan, char *buffer, size_t buffer_size, size_t *written)
{
    const MblinkMercedesModuleScanEntry *module;
    if (scan == NULL || buffer == NULL || written == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
#define WRITE(cmd) (mblink_mercedes_module_scan_write_command((cmd), buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL)
    switch (scan->stage) {
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
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER: return mblink_mercedes_module_scan_format_can_command("ATSH", scan->candidate_tx, scan->candidate_extended, buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESET_RECEIVE: return WRITE("ATCRA");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_FILTER: return WRITE("ATCF000");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_MASK: return WRITE("ATCM000");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF: return WRITE("ATH0");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE: return mblink_mercedes_module_scan_format_can_command("ATCRA", scan->candidate_rx, scan->candidate_extended, buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT: return WRITE("3E00");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return WRITE("1902FF");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE: return WRITE("3E00");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: return WRITE("22F190");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY: return WRITE("22F197");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART: return WRITE("22F187");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE: return WRITE("22F188");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE: return WRITE("22F191");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL:
        if (scan->dtc_index >= scan->module_count) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index]; return WRITE(module->extended_id ? "ATSP7" : "ATSP6");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER:
        if (scan->dtc_index >= scan->module_count) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index]; return mblink_mercedes_module_scan_format_can_command("ATSH", module->tx_can_id, module->extended_id, buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE:
        if (scan->dtc_index >= scan->module_count) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
        module = &scan->modules[scan->dtc_index]; return mblink_mercedes_module_scan_format_can_command("ATCRA", module->rx_can_id, module->extended_id, buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: if (buffer_size != 0U) buffer[0] = '\0'; *written = 0U; return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
    }
#undef WRITE
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
}

static inline int mblink_mercedes_module_scan_hex_value(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    return -1;
}

/*
 * FULL 11-bit discovery deliberately learns the actual response CAN ID before
 * locking a receive filter. ATH1 causes an ELM327 to prefix each accepted CAN
 * frame with its header; the parser below accepts both the normal single-frame
 * form (ID + PCI length + UDS payload) and an assembled/direct UDS payload.
 */
static inline bool mblink_mercedes_module_scan_headered_11_route(
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

static inline bool mblink_mercedes_module_scan_at_ok(const MblinkElm327Response *response) { return response != NULL && response->result == MBLINK_ELM327_RESULT_OK; }

static inline MblinkMercedesModuleScanResult mblink_mercedes_module_scan_accept(MblinkMercedesModuleScan *scan, const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY]; size_t pdu_length = 0U; MblinkUdsResponse uds; MblinkMercedesModuleScanEntry *module; bool present;
    if (scan == NULL || response == NULL) return MBLINK_MERCEDES_MODULE_SCAN_RESULT_INVALID_ARGUMENT;
    switch (scan->stage) {
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT:
        if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure;
        if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_CACHED) {
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL;
        } else {
            scan->stage = (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL && !scan->candidate_extended)
                ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_ENABLE_HEADERS
                : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
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
        scan->stage = (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL && !scan->candidate_extended && !scan->candidate_route_locked)
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
        scan->stage = (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL && !scan->candidate_extended && scan->candidate_route_locked)
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
        if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL &&
            !scan->candidate_extended && !scan->candidate_route_locked) {
            uint32_t learned_rx = 0U;
            if (mblink_mercedes_module_scan_headered_11_route(
                    response, MBLINK_UDS_SERVICE_TESTER_PRESENT, &learned_rx)) {
                scan->candidate_rx = learned_rx;
                scan->candidate_route_locked = true;
                (void)mblink_mercedes_module_scan_record_module(scan, true);
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF;
            } else {
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK;
            }
            break;
        }
        present = mblink_mercedes_module_scan_decode_uds(
            response, MBLINK_UDS_SERVICE_TESTER_PRESENT,
            pdu, sizeof(pdu), &pdu_length, &uds);
        if (present) (void)mblink_mercedes_module_scan_record_module(scan, true);
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
        if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL &&
            !scan->candidate_extended && !scan->candidate_route_locked) {
            uint32_t learned_rx = 0U;
            if (mblink_mercedes_module_scan_headered_11_route(
                    response, MBLINK_UDS_SERVICE_READ_DTC_INFORMATION, &learned_rx)) {
                scan->candidate_rx = learned_rx;
                scan->candidate_route_locked = true;
                (void)mblink_mercedes_module_scan_record_module(scan, false);
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_LOCK_HEADERS_OFF;
            } else {
                scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
            }
            break;
        }
        present = mblink_mercedes_module_scan_decode_uds(
            response, MBLINK_UDS_SERVICE_READ_DTC_INFORMATION,
            pdu, sizeof(pdu), &pdu_length, &uds);
        if (present) (void)mblink_mercedes_module_scan_record_module(scan, false);
        module = mblink_mercedes_module_scan_find_candidate(scan);
        if (module != NULL) mblink_mercedes_module_scan_capture_dtc(module, response);
        scan->stage = module != NULL
            ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY
            : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK:
        if (scan->scope == MBLINK_MERCEDES_MODULE_SCAN_FULL &&
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
        present = mblink_mercedes_module_scan_decode_uds(response, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER, pdu, sizeof(pdu), &pdu_length, &uds);
        if (present) {
            (void)mblink_mercedes_module_scan_record_module(scan, false);
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
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
        }
        mblink_mercedes_module_scan_advance_candidate(scan);
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE:
        if (scan->dtc_index >= scan->module_count) goto failed_state;
        present = mblink_mercedes_module_scan_decode_uds(
            response, MBLINK_UDS_SERVICE_TESTER_PRESENT,
            pdu, sizeof(pdu), &pdu_length, &uds);
        scan->modules[scan->dtc_index].tester_present_response = present;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ;
        break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: if (scan->dtc_index >= scan->module_count) goto failed_state; mblink_mercedes_module_scan_capture_dtc(&scan->modules[scan->dtc_index], response); ++scan->dtc_index; scan->stage = scan->dtc_index >= scan->module_count ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE: return MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: return MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE;
    }
    return scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_COMPLETE : MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
adapter_failure: scan->failure = MBLINK_MERCEDES_MODULE_SCAN_RESULT_ADAPTER_ERROR; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED; return scan->failure;
failed_state: scan->failure = MBLINK_MERCEDES_MODULE_SCAN_RESULT_FAILED_STATE; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED; return scan->failure;
}

static inline size_t mblink_mercedes_module_scan_module_count(const MblinkMercedesModuleScan *scan) { return scan != NULL ? scan->module_count : 0U; }
static inline const MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_module_at(const MblinkMercedesModuleScan *scan, size_t index) { return scan != NULL && index < scan->module_count ? &scan->modules[index] : NULL; }
static inline size_t mblink_mercedes_module_scan_total_dtc_count(const MblinkMercedesModuleScan *scan) { size_t total = 0U; if (scan == NULL) return 0U; for (size_t index = 0U; index < scan->module_count; ++index) total += scan->modules[index].dtcs.count; return total; }

static inline size_t mblink_mercedes_module_scan_fresh_response_count(
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

static inline size_t mblink_mercedes_module_scan_classified_count(
    const MblinkMercedesModuleScan *scan)
{
    size_t count = 0U;
    size_t index;
    if (scan == NULL) return 0U;
    for (index = 0U; index < scan->module_count; ++index) {
        if (scan->modules[index].definition != NULL) ++count;
    }
    return count;
}

#ifdef __cplusplus
}
#endif

#endif
