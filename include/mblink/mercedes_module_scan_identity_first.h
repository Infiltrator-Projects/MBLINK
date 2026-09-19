// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Public identity-first Mercedes module discovery orchestration.
 *
 * A Mercedes module scan must learn WHAT a responding ECU is before it starts
 * asking that ECU for fault memory or live values.  The underlying bounded
 * transport/census state machine lives in mercedes_module_scan_core.h; this
 * public layer orders discovery and fills missing cached transmission identity:
 *
 *   route/session setup -> ECU self-identification -> metadata -> DTC pass
 *
 * UDS ECUs are asked for F197/F187/F188/F191.  KWP2000 ECUs are asked first
 * with DaimlerChrysler ReadECUIdentification 1A 87 (mandatory DCX/MMC ECU
 * identification), then 1A 86 and 1A 89 as bounded read-only fallbacks.
 * Transmission controllers collect all three replies, including on cached
 * reconnects. Existing saved identities are retained; otherwise the first
 * valid identity is used. The shared evidence recorder retains raw replies,
 * refusals and ELM NO DATA results. Host transport failures use LINK recovery.
 * Unknown Mercedes gateway-routed targets try UDS identity first and then the
 * KWP identity service before falling back to presence probes.  Gateway NRCs
 * A0/A1 are never treated as proof that a destination ECU exists.
 */
#ifndef MBLINK_MERCEDES_MODULE_SCAN_PUBLIC_H
#define MBLINK_MERCEDES_MODULE_SCAN_PUBLIC_H

/* Keep the well-tested transport/census machinery available as the core. */
#define mblink_mercedes_module_scan_begin \
    mblink_mercedes_module_scan_begin_core
#define mblink_mercedes_module_scan_begin_gateway \
    mblink_mercedes_module_scan_begin_gateway_core
#define mblink_mercedes_module_scan_begin_mobile_census \
    mblink_mercedes_module_scan_begin_mobile_census_core
#define mblink_mercedes_module_scan_begin_full \
    mblink_mercedes_module_scan_begin_full_core
#define mblink_mercedes_module_scan_command \
    mblink_mercedes_module_scan_command_core
#define mblink_mercedes_module_scan_accept \
    mblink_mercedes_module_scan_accept_core
#include "mblink/mercedes_module_scan_core.h"
#undef mblink_mercedes_module_scan_begin
#undef mblink_mercedes_module_scan_begin_gateway
#undef mblink_mercedes_module_scan_begin_mobile_census
#undef mblink_mercedes_module_scan_begin_full
#undef mblink_mercedes_module_scan_command
#undef mblink_mercedes_module_scan_accept

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL SIZE_MAX
#define MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER ((size_t)0x80U)

bool mblink_mercedes_module_scan_identity_first_active(
    const MblinkMercedesModuleScan *scan);

void mblink_mercedes_module_scan_mark_identity_first(
    MblinkMercedesModuleScan *scan);

bool mblink_mercedes_module_scan_kwp_identity_mode(
    const MblinkMercedesModuleScan *scan);

size_t mblink_mercedes_module_scan_kwp_identity_index(
    const MblinkMercedesModuleScan *scan);

uint8_t mblink_mercedes_module_scan_kwp_identity_option(
    const MblinkMercedesModuleScan *scan);

bool mblink_mercedes_module_scan_format_kwp_identity_command(
    uint8_t option,
    char *buffer,
    size_t buffer_size,
    size_t *written);

typedef enum MblinkMercedesIdentityResponseState {
    MBLINK_MERCEDES_IDENTITY_RESPONSE_NONE = 0,
    MBLINK_MERCEDES_IDENTITY_RESPONSE_POSITIVE,
    MBLINK_MERCEDES_IDENTITY_RESPONSE_NEGATIVE,
    MBLINK_MERCEDES_IDENTITY_RESPONSE_GATEWAY_MISS
} MblinkMercedesIdentityResponseState;

MblinkMercedesIdentityResponseState
mblink_mercedes_module_scan_identity_response_state(
    const MblinkElm327Response *response,
    uint8_t request_service,
    uint8_t positive_service,
    uint8_t expected_option,
    uint8_t *negative_response_code);

bool mblink_mercedes_module_scan_copy_printable(
    const uint8_t *data,
    size_t data_length,
    char *destination,
    size_t destination_capacity);

bool mblink_mercedes_module_scan_decode_kwp_identity_record(
    const MblinkElm327Response *response,
    uint8_t option,
    MblinkKwp2000EcuIdentificationRecord *record);

bool mblink_mercedes_module_scan_capture_kwp_87(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_bcd_part_number(
    const uint8_t data[5],
    char *destination,
    size_t destination_capacity);

bool mblink_mercedes_module_scan_capture_kwp_86(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_capture_kwp_89(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response);

bool mblink_mercedes_module_scan_capture_kwp_identity(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response,
    uint8_t option);

bool mblink_mercedes_module_scan_is_kwp_transmission(
    const MblinkMercedesModuleScanEntry *module);

void mblink_mercedes_module_scan_start_kwp_identity_fallback(
    MblinkMercedesModuleScan *scan);

bool mblink_mercedes_module_scan_advance_kwp_identity(
    MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin(MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_gateway(MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_mobile_census(
    MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_full(MblinkMercedesModuleScan *scan);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_command(
    const MblinkMercedesModuleScan *scan,
    char *buffer,
    size_t buffer_size,
    size_t *written);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_accept_identity(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response);

MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_accept(
    MblinkMercedesModuleScan *scan,
    const MblinkElm327Response *response);

/* A watchdog cancels the outstanding command; it is not an ECU NO DATA reply.
 * Keep every captured entry, replay adapter setup, and restart the route. A
 * second interruption on the same route skips it, with at most three resumes
 * for the scan. The owner must defer this until LINK owns a fresh wire slot. */
bool mblink_mercedes_module_scan_resume_after_interruption(
    MblinkMercedesModuleScan *scan);

/* A late legislated-OBD response proves this route is alive, not its family.
 * Probe the source-backed GS route once without replacing the other modules
 * or their captured faults. The Apple owner supplies current-session evidence
 * and enforces one follow-up per connection. */
bool mblink_mercedes_module_scan_begin_late_transmission(
    MblinkMercedesModuleScan *scan);

#ifdef __cplusplus
}
#endif
#endif
