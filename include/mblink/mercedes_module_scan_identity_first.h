// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_module_scan.h
 * @brief Public identity-first Mercedes module discovery orchestration.
 *
 * A Mercedes module scan must learn WHAT a responding ECU is before it starts
 * asking that ECU for fault memory or live values.  The underlying bounded
 * transport/census state machine lives in mercedes_module_scan_core.h; this
 * public layer changes discovery order only:
 *
 *   route/session setup -> ECU self-identification -> metadata -> DTC pass
 *
 * UDS ECUs are asked for F197/F187/F188/F191.  KWP2000 ECUs are asked first
 * with DaimlerChrysler ReadECUIdentification 1A 87 (mandatory DCX/MMC ECU
 * identification), then 1A 86 and 1A 89 as bounded read-only fallbacks.
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

static inline bool mblink_mercedes_module_scan_identity_first_active(
    const MblinkMercedesModuleScan *scan)
{
    return scan != NULL &&
           scan->scope != MBLINK_MERCEDES_MODULE_SCAN_CACHED &&
           scan->dtc_index == MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL;
}

static inline void mblink_mercedes_module_scan_mark_identity_first(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;
    scan->dtc_index = MBLINK_MERCEDES_IDENTITY_FIRST_SENTINEL;
    scan->vin_probe_index = 0U;
}

static inline bool mblink_mercedes_module_scan_kwp_identity_mode(
    const MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return false;
    return mblink_mercedes_module_scan_candidate_protocol(scan) ==
               MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 ||
           scan->vin_probe_index >=
               MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER;
}

static inline size_t mblink_mercedes_module_scan_kwp_identity_index(
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

static inline uint8_t mblink_mercedes_module_scan_kwp_identity_option(
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
    return index < sizeof(options) / sizeof(options[0])
        ? options[index] : 0U;
}

static inline bool mblink_mercedes_module_scan_format_kwp_identity_command(
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

typedef enum MblinkMercedesIdentityResponseState {
    MBLINK_MERCEDES_IDENTITY_RESPONSE_NONE = 0,
    MBLINK_MERCEDES_IDENTITY_RESPONSE_POSITIVE,
    MBLINK_MERCEDES_IDENTITY_RESPONSE_NEGATIVE,
    MBLINK_MERCEDES_IDENTITY_RESPONSE_GATEWAY_MISS
} MblinkMercedesIdentityResponseState;

static inline MblinkMercedesIdentityResponseState
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

static inline bool mblink_mercedes_module_scan_copy_printable(
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

static inline bool mblink_mercedes_module_scan_decode_kwp_identity_record(
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

static inline void mblink_mercedes_module_scan_capture_kwp_87(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response)
{
    MblinkKwp2000EcuIdentificationRecord record;
    char part[sizeof(module->spare_part_number)];

    if (module == NULL ||
        !mblink_mercedes_module_scan_decode_kwp_identity_record(
            response, UINT8_C(0x87), &record) ||
        record.data_length < 10U) {
        return;
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
}

static inline bool mblink_mercedes_module_scan_bcd_part_number(
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

static inline void mblink_mercedes_module_scan_capture_kwp_86(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response)
{
    MblinkKwp2000EcuIdentificationRecord record;
    char part[sizeof(module->spare_part_number)];

    if (module == NULL ||
        !mblink_mercedes_module_scan_decode_kwp_identity_record(
            response, UINT8_C(0x86), &record) ||
        record.data_length < 12U) {
        return;
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
}

static inline void mblink_mercedes_module_scan_capture_kwp_89(
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
        return;
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
}

static inline void mblink_mercedes_module_scan_capture_kwp_identity(
    MblinkMercedesModuleScanEntry *module,
    const MblinkElm327Response *response,
    uint8_t option)
{
    switch (option) {
    case UINT8_C(0x87):
        mblink_mercedes_module_scan_capture_kwp_87(module, response);
        break;
    case UINT8_C(0x86):
        mblink_mercedes_module_scan_capture_kwp_86(module, response);
        break;
    case UINT8_C(0x89):
        mblink_mercedes_module_scan_capture_kwp_89(module, response);
        break;
    default:
        break;
    }
}

static inline void mblink_mercedes_module_scan_start_kwp_identity_fallback(
    MblinkMercedesModuleScan *scan)
{
    if (scan == NULL) return;
    scan->vin_probe_index =
        MBLINK_MERCEDES_KWP_IDENTITY_FALLBACK_MARKER;
    scan->stage = MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY;
}

static inline bool mblink_mercedes_module_scan_advance_kwp_identity(
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

static inline MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin(MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

static inline MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_gateway(MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_gateway_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

static inline MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_mobile_census(
    MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_mobile_census_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

static inline MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_begin_full(MblinkMercedesModuleScan *scan)
{
    MblinkMercedesModuleScanResult result =
        mblink_mercedes_module_scan_begin_full_core(scan);
    if (result == MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK)
        mblink_mercedes_module_scan_mark_identity_first(scan);
    return result;
}

static inline MblinkMercedesModuleScanResult
mblink_mercedes_module_scan_command(
    const MblinkMercedesModuleScan *scan,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    if (mblink_mercedes_module_scan_identity_first_active(scan) &&
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY &&
        mblink_mercedes_module_scan_kwp_identity_mode(scan)) {
        return mblink_mercedes_module_scan_format_kwp_identity_command(
                   mblink_mercedes_module_scan_kwp_identity_option(scan),
                   buffer, buffer_size, written)
            ? MBLINK_MERCEDES_MODULE_SCAN_RESULT_OK
            : MBLINK_MERCEDES_MODULE_SCAN_RESULT_BUFFER_TOO_SMALL;
    }
    return mblink_mercedes_module_scan_command_core(
        scan, buffer, buffer_size, written);
}

static inline MblinkMercedesModuleScanResult
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
        state = mblink_mercedes_module_scan_identity_response_state(
            response, MBLINK_KWP2000_SERVICE_READ_ECU_IDENTIFICATION,
            UINT8_C(0x5a), option, &nrc);

        if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_GATEWAY_MISS) {
            mblink_mercedes_module_scan_advance_candidate(scan);
        } else if (state == MBLINK_MERCEDES_IDENTITY_RESPONSE_POSITIVE) {
            module = mblink_mercedes_module_scan_record_module(scan, false);
            if (module != NULL) {
                module->protocol = MBLINK_MERCEDES_DIAGNOSTIC_KWP2000;
                mblink_mercedes_module_scan_capture_kwp_identity(
                    module, response, option);
            }
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

static inline MblinkMercedesModuleScanResult
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

    if (identity_first &&
        scan->stage == MBLINK_MERCEDES_MODULE_SCAN_STAGE_DISCOVERY_IDENTITY) {
        return mblink_mercedes_module_scan_accept_identity(scan, response);
    }

    result = mblink_mercedes_module_scan_accept_core(scan, response);
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

#ifdef __cplusplus
}
#endif
#endif
