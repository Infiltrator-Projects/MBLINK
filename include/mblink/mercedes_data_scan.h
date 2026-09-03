// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_data_scan.h
 * @brief Bounded read-only discovery of manufacturer data identifiers per ECU.
 *
 * This is deliberately separate from legislated SAE Mode 01.  A discovered
 * Mercedes module is addressed on its physical TX/RX route and queried with
 * read-only UDS ReadDataByIdentifier (0x22) or KWP2000
 * ReadDataByLocalIdentifier (0x21).  Positive responses are retained as raw
 * module-owned values even when their semantic/scaling definition is not yet
 * known.
 */
#ifndef MBLINK_MERCEDES_DATA_SCAN_H
#define MBLINK_MERCEDES_DATA_SCAN_H

#include "mblink/discover.h"
#include "mblink/elm327.h"
#include "mblink/mercedes.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS 256U
#define MBLINK_MERCEDES_DATA_SCAN_MAX_DATA 96U

typedef enum MblinkMercedesDataScanStage {
    MBLINK_MERCEDES_DATA_SCAN_STAGE_INIT_PROTOCOL = 0,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_HEADERS_OFF,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_AUTO_FORMAT,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_FLOW_CONTROL,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_TIMEOUT,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_HEADER,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_RECEIVE,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_EXTENDED_SESSION,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE,
    MBLINK_MERCEDES_DATA_SCAN_STAGE_FAILED
} MblinkMercedesDataScanStage;

typedef enum MblinkMercedesDataScanResult {
    MBLINK_MERCEDES_DATA_SCAN_RESULT_OK = 0,
    MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE,
    MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT,
    MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL,
    MBLINK_MERCEDES_DATA_SCAN_RESULT_ADAPTER_ERROR,
    MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE
} MblinkMercedesDataScanResult;

typedef struct MblinkMercedesDataScanConfig {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool extended_id;
    MblinkMercedesDiagnosticProtocol protocol;
    MblinkMercedesModuleKind module_kind;
    uint16_t first_identifier;
    uint16_t last_identifier;
    bool request_extended_session;
} MblinkMercedesDataScanConfig;

typedef struct MblinkMercedesDataRecord {
    uint16_t identifier;
    uint8_t service;
    size_t data_length;
    bool truncated;
    uint8_t data[MBLINK_MERCEDES_DATA_SCAN_MAX_DATA];
} MblinkMercedesDataRecord;

typedef struct MblinkMercedesDataScan {
    MblinkMercedesDataScanConfig config;
    MblinkMercedesDataScanStage stage;
    MblinkMercedesDataScanResult failure;
    uint16_t current_identifier;
    size_t attempted_count;
    size_t positive_count;
    size_t negative_count;
    size_t no_response_count;
    size_t invalid_count;
    bool truncated;
    /*
     * Optional explicit identifier sequence used for fast refreshes after a
     * full discovery pass has already proved which IDs respond on this ECU.
     * The list is copied into the scan so callers do not own its lifetime.
     */
    bool identifier_list_active;
    size_t identifier_count;
    size_t identifier_index;
    /*
     * A refresh identifier is already proven to exist, so transient ELM
     * NO DATA is retried twice before it is counted as a missed response.
     * Retries do not advance the identifier or inflate attempted_count.
     */
    uint8_t current_no_response_retries;
    uint16_t identifiers[MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS];
    MblinkMercedesDataRecord records[MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS];
} MblinkMercedesDataScan;

const char *mblink_mercedes_data_scan_result_name(
    MblinkMercedesDataScanResult result);
const char *mblink_mercedes_data_scan_stage_name(
    MblinkMercedesDataScanStage stage);
bool mblink_mercedes_data_scan_config_is_valid(
    const MblinkMercedesDataScanConfig *config);

/**
 * Default discovery ranges used by the UI's per-module manufacturer-data scan.
 *
 * UDS starts with Daimler's 0x20xx actual-value neighbourhood because 0x2007
 * is source-backed on the C207 CRD3 family. KWP2000 uses the complete bounded
 * local-identifier byte range. These are discovery ranges, not semantic claims:
 * every positive response remains raw/unmapped until a definition is proven.
 */
MblinkMercedesDataScanConfig mblink_mercedes_data_scan_default_config(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesModuleKind module_kind);

MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config);

/**
 * Begin a bounded refresh using only previously proven positive identifiers.
 *
 * This does not change semantic evidence: a positive identifier remains raw
 * until its meaning/scaling is independently verified.  It only avoids
 * repeating the entire discovery range when the caller already has a
 * module-scoped list of responsive IDs.
 */
MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin_identifiers(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const uint16_t *identifiers,
    size_t identifier_count);
MblinkMercedesDataScanResult mblink_mercedes_data_scan_command(
    const MblinkMercedesDataScan *scan,
    char *buffer,
    size_t buffer_size,
    size_t *written);
MblinkMercedesDataScanResult mblink_mercedes_data_scan_accept(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response);
uint64_t mblink_mercedes_data_scan_timeout_ms(
    const MblinkMercedesDataScan *scan);
size_t mblink_mercedes_data_scan_record_count(
    const MblinkMercedesDataScan *scan);
const MblinkMercedesDataRecord *mblink_mercedes_data_scan_record_at(
    const MblinkMercedesDataScan *scan,
    size_t index);

/** Format only the identifier/service portion (e.g. "UDS DID 0x2007"). */
bool mblink_mercedes_data_record_format_code(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size);

/** Format the raw response payload as uppercase hex with no invented scaling. */
bool mblink_mercedes_data_record_format_hex(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size);

/**
 * Decode the one currently source-backed numeric actual-value definition.
 * Returns false for every unmapped identifier so callers cannot accidentally
 * present guessed engineering units.
 */
bool mblink_mercedes_data_record_decode_known_numeric(
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record,
    double *value,
    const char **name,
    const char **unit);

/**
 * Route-aware numeric decoder for definitions whose meaning is tied to a
 * specific physical ECU endpoint rather than only a coarse module kind.
 *
 * The 0x7E1 -> 0x7E9 Mercedes transmission-temperature candidate is one such
 * definition: public Mercedes-owner validation uses request 0x21 0x30 and the
 * 12th response byte minus 50 degrees C, while the exact VGS/EGS family can
 * remain unidentified.
 */
bool mblink_mercedes_data_record_decode_known_numeric_for_route(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record,
    double *value,
    const char **name,
    const char **unit);

#ifdef __cplusplus
}
#endif

#endif
