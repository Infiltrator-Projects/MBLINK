// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Bounded read-only Mercedes CAN/UDS module discovery and DTC inventory.
 *
 * The scan never sends coding, programming, security-access, routine-control,
 * reset or clear-DTC services. It probes physical diagnostic endpoints using
 * TesterPresent, ReadDTCInformation and ReadDataByIdentifier only, then reads
 * each responding ECU's DTC memory. Both 11-bit physical addressing and the
 * ISO 15765 normal-fixed 29-bit pattern are covered.
 */
#ifndef MBLINK_MERCEDES_MODULE_SCAN_H
#define MBLINK_MERCEDES_MODULE_SCAN_H

#include "mblink/elm327.h"
#include "mblink/elm327_can.h"
#include "mblink/mercedes.h"
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

typedef enum MblinkMercedesModuleScanStage {
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11 = 0,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_HEADERS,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_AUTO_FORMAT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_FLOW_CONTROL,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE,
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
    unsigned int discovery_mode;
    uint32_t candidate_tx;
    uint32_t candidate_rx;
    bool candidate_extended;
    uint16_t normal_fixed_target;
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
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER: return "discover-set-header";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE: return "discover-set-receive";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT: return "discover-tester-present";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK: return "discover-dtc-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: return "discover-vin-fallback";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29: return "initialise-29-bit-can";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL: return "fault-set-protocol";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER: return "fault-set-header";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE: return "fault-set-receive";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return "read-module-faults";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

static inline const char *mblink_mercedes_module_scan_module_name(const MblinkMercedesModuleScanEntry *module)
{
    if (module == NULL) return "Mercedes ECU";
    if (!module->extended_id && module->tx_can_id == UINT32_C(0x7e0)) return "Engine ECU";
    if (!module->extended_id && module->tx_can_id == UINT32_C(0x7e1)) return "Transmission ECU";
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
    if (!extended_id && tx_can_id == UINT32_C(0x7e1)) return MBLINK_MERCEDES_MODULE_TRANSMISSION;
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
}

static inline void mblink_mercedes_module_scan_set_29_candidate(MblinkMercedesModuleScan *scan, uint16_t target)
{
    scan->normal_fixed_target = target;
    scan->candidate_tx = UINT32_C(0x18da00f1) | ((uint32_t)target << 8U);
    scan->candidate_rx = UINT32_C(0x18daf100) | (uint32_t)target;
    scan->candidate_extended = true;
}

static inline void mblink_mercedes_module_scan_finish_discovery(MblinkMercedesModuleScan *scan)
{
    scan->dtc_index = 0U;
    scan->stage = scan->module_count == 0U ? MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE : MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL;
}

static inline void mblink_mercedes_module_scan_advance_candidate(MblinkMercedesModuleScan *scan)
{
    /* Normal eleven-bit OBD physical addressing is deliberately bounded to
       0x7E0..0x7E7.  The previous implementation continued by brute-forcing
       0x600..0x7F7, causing hundreds of header/filter/probe cycles on a live
       ELM327 connection.  After the bounded physical range, move directly to
       the optional ISO 15765 normal-fixed twenty-nine-bit discovery phase. */
    if (scan->discovery_mode == 0U) {
        if (scan->candidate_tx < UINT32_C(0x7e7)) {
            mblink_mercedes_module_scan_set_11_candidate(scan, scan->candidate_tx + 1U);
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
            return;
        }
        scan->discovery_mode = 2U;
        scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29;
        return;
    }
    if (scan->discovery_mode == 2U) {
        uint16_t next = (uint16_t)(scan->normal_fixed_target + 1U);
        if (next == 0xF1U) ++next;
        if (next <= 0xFFU) {
            mblink_mercedes_module_scan_set_29_candidate(scan, next);
            scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER;
            return;
        }
    }
    mblink_mercedes_module_scan_finish_discovery(scan);
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
    module->dtc_result = MBLINK_MERCEDES_MODULE_DTC_NOT_ATTEMPTED;
    return module;
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
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_PROTOCOL_11;
    scan->discovery_mode = 0U;
    mblink_mercedes_module_scan_set_11_candidate(scan, UINT32_C(0x7e0));
    return MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK;
}

static inline uint64_t mblink_mercedes_module_scan_timeout_ms(const MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return UINT64_C(1500);
    switch (scan->stage) {
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: return UINT64_C(900);
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return UINT64_C(4000);
    default: return UINT64_C(1500);
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
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT: return WRITE("ATST10");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29: return WRITE("ATSP7");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER: return mblink_mercedes_module_scan_format_can_command("ATSH", scan->candidate_tx, scan->candidate_extended, buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE: return mblink_mercedes_module_scan_format_can_command("ATCRA", scan->candidate_rx, scan->candidate_extended, buffer, buffer_size, written) ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT: return WRITE("3E00");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ: return WRITE("1902FF");
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK: return WRITE("22F190");
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
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_INIT_TIMEOUT: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; mblink_mercedes_module_scan_set_29_candidate(scan, 0U); scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_HEADER: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SET_RECEIVE: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT:
        present = mblink_mercedes_module_scan_decode_uds(response, MBLINK_UDS_SERVICE_TESTER_PRESENT, pdu, sizeof(pdu), &pdu_length, &uds);
        if (present) { (void)mblink_mercedes_module_scan_record_module(scan, true); mblink_mercedes_module_scan_advance_candidate(scan); } else scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK:
        present = mblink_mercedes_module_scan_decode_uds(response, MBLINK_UDS_SERVICE_READ_DTC_INFORMATION, pdu, sizeof(pdu), &pdu_length, &uds);
        if (present) { module = mblink_mercedes_module_scan_record_module(scan, false); if (module != NULL) mblink_mercedes_module_scan_capture_dtc(module, response); mblink_mercedes_module_scan_advance_candidate(scan); } else scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK:
        present = mblink_mercedes_module_scan_decode_uds(response, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER, pdu, sizeof(pdu), &pdu_length, &uds); if (present) (void)mblink_mercedes_module_scan_record_module(scan, false); mblink_mercedes_module_scan_advance_candidate(scan); break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE; break;
    case MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE: if (!mblink_mercedes_module_scan_at_ok(response)) goto adapter_failure; scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ; break;
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

#ifdef __cplusplus
}
#endif

#endif
