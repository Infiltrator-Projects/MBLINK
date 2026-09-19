// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Bounded read-only Mercedes CAN/UDS module discovery and DTC inventory.
 *
 * The scan never sends coding, programming, security-access, routine-control,
 * reset or clear-DTC services. It probes physical diagnostic endpoints using
 * TesterPresent, ReadDTCInformation and ReadDataByIdentifier, plus an
 * evidence-gated 10 03 session request on the exact C207 routes where a
 * positive response was captured. It then reads each responding ECU's DTC
 * memory. The iPhone first-VIN mobile census walks
 * the compact 47-slot Mercedes gateway request/response lattice with exact
 * receive filters, plus source-backed exceptions and the eight legislated OBD
 * physical slots. The workstation FULL scope retains the exhaustive 11/29-bit
 * forensic sweep and can learn unknown response identifiers. LINK defines the
 * generic plan contract/transport/safety machinery; MBLINK owns both maps.
 */
#ifndef MBLINK_MERCEDES_MODULE_SCAN_H
#define MBLINK_MERCEDES_MODULE_SCAN_H

#include "mblink/discover.h"
#include "mblink/elm327.h"
#include "mblink/elm327_can.h"
#include "mblink/mercedes.h"
#include "mblink/mercedes_module_catalog.h"
#include "mblink/kwp2000.h"
#include "mblink/obd2.h"
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
    MBLINK_MERCEDES_MODULE_SCAN_MOBILE_CENSUS,
    MBLINK_MERCEDES_MODULE_SCAN_FULL,
    MBLINK_MERCEDES_MODULE_SCAN_CACHED
} MblinkMercedesModuleScanScope;

const char *mblink_mercedes_module_scan_scope_name(
    MblinkMercedesModuleScanScope scope);

bool mblink_mercedes_module_scan_uses_target_plan(
    MblinkMercedesModuleScanScope scope);

const link_discover_sweep_plan *
mblink_mercedes_module_scan_target_plan(MblinkMercedesModuleScanScope scope);

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
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_EXTENDED_SESSION,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_TESTER_PRESENT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_DTC_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_SET_TIMEOUT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VIN_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_VARIANT_FALLBACK,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_RESTORE_TIMEOUT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SPARE_PART,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_SOFTWARE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_HARDWARE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_PROTOCOL_29,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_SWITCH_HEADERS_OFF_29,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_PROTOCOL,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_HEADER,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_SET_RECEIVE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_EXTENDED_SESSION,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_VALIDATE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_DTC_READ,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_COMPLETE,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_FAILED,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_SET_TIMEOUT,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY,
    MBLINK_MERCEDES_MODULE_SCAN_STAGE_CACHED_IDENTITY_RESTORE_TIMEOUT
} MblinkMercedesModuleScanStage;

typedef struct MblinkMercedesModuleScanEntry {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool extended_id;
    MblinkMercedesDiagnosticProtocol protocol;
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
    const MblinkMercedesControllerFamilyDefinition *controller_family;
    MblinkMercedesDefinitionStatus identification_status;
    MblinkMercedesModuleDtcResult dtc_result;
    MblinkUdsResult dtc_uds_result;
    MblinkKwp2000Result dtc_kwp_result;
    uint8_t dtc_negative_response_code;
    MblinkUdsDtcList dtcs;
    MblinkKwp2000DtcList kwp_dtcs;
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
    size_t vin_probe_index;
    /* First positive VIN from a physically filtered, matching VIN request. */
    char vin[LINK_OBD2_VIN_LENGTH + 1U];
    uint32_t vin_tx_can_id;
    uint32_t vin_rx_can_id;
    MblinkMercedesVinProbe vin_source;
    bool vin_timeout_long;
    bool kwp_identity_captured;
    size_t dtc_index;
    /* Resume only after the transport owner has resynchronised the wire. */
    bool resume_pending;
    MblinkMercedesModuleScanStage resume_stage;
    unsigned recovery_count;
    uint32_t recovery_tx;
    uint32_t recovery_rx;
    bool recovery_extended;
    bool single_module_refresh;
} MblinkMercedesModuleScan;

const char *mblink_mercedes_module_scan_result_name(MblinkMercedesModuleScanResult result);

const char *mblink_mercedes_module_scan_stage_name(MblinkMercedesModuleScanStage stage);

const char *mblink_mercedes_module_scan_module_name(const MblinkMercedesModuleScanEntry *module);

MblinkMercedesModuleKind mblink_mercedes_module_scan_kind(uint32_t tx_can_id, bool extended_id);

bool mblink_mercedes_module_scan_write_command(const char *command, char *buffer, size_t buffer_size, size_t *written);

void mblink_mercedes_module_scan_set_11_candidate(MblinkMercedesModuleScan *scan, uint32_t tx);

bool mblink_mercedes_module_scan_set_gateway_target(
    MblinkMercedesModuleScan *scan,
    uint16_t target);

size_t mblink_mercedes_module_scan_planned_target_count(
    MblinkMercedesModuleScanScope scope);

bool mblink_mercedes_module_scan_set_full_target(
    MblinkMercedesModuleScan *scan,
    size_t index);

const MblinkMercedesKnownRoute *
mblink_mercedes_module_scan_known_route(const MblinkMercedesModuleScan *scan);

const MblinkMercedesKnownRoute *
mblink_mercedes_module_scan_known_entry_route(
    const MblinkMercedesModuleScanEntry *module);

MblinkMercedesDiagnosticProtocol
mblink_mercedes_module_scan_candidate_protocol(
    const MblinkMercedesModuleScan *scan);

bool mblink_mercedes_module_scan_is_production_vin_target(
    const MblinkMercedesModuleScan *scan);

/*
 * Exact request order recovered from the production Mercedes me
 * MSA_VIN_cascade parameterisation. Keep it limited to the evidenced TX/RX
 * pairs so the ordinary 47-slot mobile census remains fast.
 */
MblinkMercedesVinProbe mblink_mercedes_module_scan_vin_probe_at(
    const MblinkMercedesModuleScan *scan,
    size_t index);

const char *mblink_mercedes_module_scan_vin_command(
    const MblinkMercedesModuleScan *scan);

bool mblink_mercedes_module_scan_has_next_vin_probe(
    const MblinkMercedesModuleScan *scan);

void mblink_mercedes_module_scan_enter_vin_fallback(
    MblinkMercedesModuleScan *scan);

MblinkMercedesDiagnosticProtocol
mblink_mercedes_module_scan_entry_protocol(
    const MblinkMercedesModuleScanEntry *module);

void mblink_mercedes_module_scan_finish_discovery(MblinkMercedesModuleScan *scan);

void mblink_mercedes_module_scan_advance_candidate(
    MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_record_module(MblinkMercedesModuleScan *scan, bool tester_present);


MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_find_candidate(
    MblinkMercedesModuleScan *scan);

bool mblink_mercedes_module_scan_capture_text_did(
    const MblinkElm327Response *response,
    uint16_t did,
    char *destination,
    size_t destination_capacity);

void mblink_mercedes_module_scan_classify_controller_family(
    MblinkMercedesModuleScanEntry *module);

void mblink_mercedes_module_scan_classify_identity(
    MblinkMercedesModuleScanEntry *module);

void mblink_mercedes_module_scan_capture_identity(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_decode_uds(const MblinkElm327Response *response, uint8_t service, uint8_t *pdu, size_t pdu_capacity, size_t *pdu_length, MblinkUdsResponse *uds);

bool mblink_mercedes_module_scan_decode_kwp(
    const MblinkElm327Response *response,
    uint8_t service);

bool mblink_mercedes_module_scan_decode_candidate(
    const MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response,
    uint8_t uds_service,
    uint8_t kwp_service);

void mblink_mercedes_module_scan_capture_vin(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_decode_vin_probe(
    const MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_decode_entry(
    const MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response,
    uint8_t uds_service,
    uint8_t kwp_service);

void mblink_mercedes_module_scan_capture_dtc(MblinkMercedesModuleScanEntry *module, const MblinkElm327Response *response);

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_begin_core(MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_gateway_core(MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_mobile_census_core(
    MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_cached(
    MblinkMercedesModuleScan *scan,
    const MblinkMercedesModuleScanEntry *modules,
    size_t module_count);

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_begin_full_core(
    MblinkMercedesModuleScan *scan);

uint64_t mblink_mercedes_module_scan_timeout_ms(const MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_command_core(const MblinkMercedesModuleScan *scan, char *buffer, size_t buffer_size, size_t *written);

int mblink_mercedes_module_scan_hex_value(char value);

/*
 * Headered 11-bit route learning for generic Mercedes sweep candidates.
 * Source-backed routes and standardized OBD slots stay on exact receive
 * filters, while an otherwise unknown physical request temporarily accepts
 * headered traffic and uses a valid UDS positive/negative response to learn
 * the ECU's actual response CAN identifier before re-locking the filter.
 */
bool mblink_mercedes_module_scan_headered_11_route(
    const MblinkElm327Response *response,
    uint8_t request_service,
    uint32_t *rx_can_id);

bool mblink_mercedes_module_scan_at_ok(const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_accept_adapter_transition(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response,
    MblinkMercedesModuleScanStage next_stage);

MblinkMercedesModuleScanResult mblink_mercedes_module_scan_accept_core(MblinkMercedesModuleScan *scan, const MblinkElm327Response *response);

size_t mblink_mercedes_module_scan_module_count(const MblinkMercedesModuleScan *scan);
const MblinkMercedesModuleScanEntry *mblink_mercedes_module_scan_module_at(const MblinkMercedesModuleScan *scan, size_t index);
size_t mblink_mercedes_module_scan_entry_dtc_count(
    const MblinkMercedesModuleScanEntry *module);

size_t mblink_mercedes_module_scan_total_dtc_count(
    const MblinkMercedesModuleScan *scan);

size_t mblink_mercedes_module_scan_fresh_response_count(
    const MblinkMercedesModuleScan *scan);

size_t mblink_mercedes_module_scan_classified_count(
    const MblinkMercedesModuleScan *scan);

#ifdef __cplusplus
}
#endif

#endif
