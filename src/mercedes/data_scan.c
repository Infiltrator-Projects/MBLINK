// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_data_scan.h"

#include "mblink/elm327_can.h"
#include "mblink/kwp2000.h"
#include "mblink/mercedes_transmission.h"
#include "mblink/mercedes_module_catalog.h"
#include "mblink/uds.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MBLINK_MERCEDES_DATA_SCAN_PDU_CAPACITY 512U

static MblinkMercedesDataScanResult fail_scan(
    MblinkMercedesDataScan *scan,
    MblinkMercedesDataScanResult result)
{
    if (scan != NULL) {
        scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_FAILED;
        scan->failure = result;
    }
    return result;
}

static bool at_ok(const MblinkElm327Response *response)
{
    return response != NULL &&
           response->result == MBLINK_ELM327_RESULT_OK;
}

static MblinkMercedesDataScanResult write_text(
    const char *text,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    size_t length;
    if (written != NULL) *written = 0U;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (text == NULL || buffer == NULL || written == NULL)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
    length = strlen(text);
    if (length + 1U > buffer_size)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
    memcpy(buffer, text, length + 1U);
    *written = length;
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

static MblinkMercedesDataScanResult format_can_command(
    const char *prefix,
    uint32_t identifier,
    bool extended,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    int count;
    if (written != NULL) *written = 0U;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (prefix == NULL || buffer == NULL || written == NULL)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
    count = extended
        ? snprintf(buffer, buffer_size, "%s%08X", prefix,
                   (unsigned int)identifier)
        : snprintf(buffer, buffer_size, "%s%03X", prefix,
                   (unsigned int)identifier);
    if (count < 0 || (size_t)count >= buffer_size)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
    *written = (size_t)count;
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

static void advance_identifier(MblinkMercedesDataScan *scan)
{
    if (scan == NULL) return;
    scan->current_no_response_retries = 0U;
    scan->attempted_count++;

    if (scan->identifier_list_active) {
        scan->identifier_index++;
        if (scan->identifier_index >= scan->identifier_count) {
            scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE;
            return;
        }
        scan->current_identifier =
            scan->identifiers[scan->identifier_index];
        return;
    }

    if (scan->current_identifier >= scan->config.last_identifier) {
        scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE;
        return;
    }
    scan->current_identifier++;
}

static void record_positive(
    MblinkMercedesDataScan *scan,
    uint8_t service,
    uint16_t identifier,
    const uint8_t *data,
    size_t data_length)
{
    MblinkMercedesDataRecord *record;
    size_t stored = data_length;

    if (scan == NULL || (data == NULL && data_length != 0U)) return;
    scan->positive_count++;
    if (scan->positive_count > MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS) {
        scan->truncated = true;
        return;
    }
    record = &scan->records[scan->positive_count - 1U];
    memset(record, 0, sizeof(*record));
    record->identifier = identifier;
    record->service = service;
    if (stored > sizeof(record->data)) {
        stored = sizeof(record->data);
        record->truncated = true;
    }
    record->data_length = stored;
    if (stored != 0U) memcpy(record->data, data, stored);
}

const char *mblink_mercedes_data_scan_result_name(
    MblinkMercedesDataScanResult result)
{
    switch (result) {
    case MBLINK_MERCEDES_DATA_SCAN_RESULT_OK: return "ok";
    case MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE: return "complete";
    case MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT:
        return "invalid-argument";
    case MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL:
        return "buffer-too-small";
    case MBLINK_MERCEDES_DATA_SCAN_RESULT_ADAPTER_ERROR:
        return "adapter-error";
    case MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE:
        return "failed-state";
    }
    return "unknown";
}

const char *mblink_mercedes_data_scan_stage_name(
    MblinkMercedesDataScanStage stage)
{
    switch (stage) {
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_INIT_PROTOCOL:
        return "initialise-can";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_HEADERS_OFF:
        return "headers-off";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_AUTO_FORMAT:
        return "auto-format";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_FLOW_CONTROL:
        return "flow-control";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TIMEOUT:
        return "timeout";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_HEADER:
        return "set-header";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_RECEIVE:
        return "set-receive";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_EXTENDED_SESSION:
        return "extended-session";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT:
        return "tester-present";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER:
        return "read-identifier";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE: return "complete";
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_FAILED: return "failed";
    }
    return "unknown";
}

bool mblink_mercedes_data_scan_config_is_valid(
    const MblinkMercedesDataScanConfig *config)
{
    uint32_t max_id;
    if (config == NULL ||
        config->protocol > MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 ||
        config->first_identifier > config->last_identifier) {
        return false;
    }
    max_id = config->extended_id
        ? UINT32_C(0x1fffffff) : UINT32_C(0x7ff);
    if (config->tx_can_id > max_id || config->rx_can_id > max_id)
        return false;
    if (config->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        if (config->first_identifier == 0U ||
            config->last_identifier > UINT16_C(0x00ff)) {
            return false;
        }
    }
    return true;
}

MblinkMercedesDataScanConfig mblink_mercedes_data_scan_default_config(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesModuleKind module_kind)
{
    MblinkMercedesDataScanConfig config;
    memset(&config, 0, sizeof(config));
    config.tx_can_id = tx_can_id;
    config.rx_can_id = rx_can_id;
    config.extended_id = extended_id;
    config.protocol = protocol;
    config.module_kind = module_kind;
    config.request_extended_session =
        protocol == MBLINK_MERCEDES_DIAGNOSTIC_UDS;

    if (protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        config.first_identifier = UINT16_C(0x0001);
        config.last_identifier = UINT16_C(0x00ff);
    } else {
        config.first_identifier = UINT16_C(0x2000);
        config.last_identifier = UINT16_C(0x20ff);
    }
    return config;
}

/*
 * Controller-scoped read-only data profiles.
 *
 * These are deliberately keyed by ECU family rather than CAN route. The same
 * address or broad module kind can host different controller generations with
 * different diagnostic namespaces.
 *
 * The three raw records below are vehicle-verified only as positive identifiers
 * on the named C207 controller families; their semantics remain unknown.
 */
static const MblinkMercedesControllerDataProfileEntry
    controller_data_profile[] = {
    {
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT16_C(0x2007), true, "Battery voltage",
        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
        "CaesarSuite CRD3 DT_2007 documents DID 0x2007 and its scaling; "
        "C207 capture independently proves the positive response."
    },
    {
        "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT16_C(0x2001), true, "Vehicle-verified raw data 0x2001",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        "2026-09-03 C207 capture proves a positive 0x2001 response on the "
        "source-corroborated ABR2XT/ESP controller family; semantics unknown."
    },
    {
        "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT16_C(0x0058), true, "Vehicle-verified raw local record 0x58",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        "2026-09-03 C207 capture proves local record 0x58 on ORC_212; "
        "semantics unknown."
    },
    {
        "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT16_C(0x0001), true, "Vehicle-verified raw local record 0x01",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        "2026-09-03 C207 capture proves local record 0x01 on HU_204; "
        "semantics unknown."
    }
};

const char *mblink_mercedes_data_profile_key_for_controller(
    const char *module_key,
    const char *identity,
    const char *software_number,
    const char *hardware_number)
{
    const MblinkMercedesControllerFamilyDefinition *family =
        mblink_mercedes_controller_family_definition_for_evidence(
            module_key, identity, software_number, hardware_number);
    if (family == NULL) return NULL;

    /*
     * Only families with independently source-backed/captured read identifiers
     * are active data profiles. All other controller families can still be
     * identified and displayed without inheriting guessed services.
     */
    if (strcmp(family->key, "engine-crd3") == 0)
        return "engine-crd3";
    if (strcmp(family->key, "esp-abr2xt") == 0)
        return "esp-abr2xt";
    if (strcmp(family->key, "restraints-orc212") == 0)
        return "restraints-orc212";
    if (strcmp(family->key, "headunit-hu204") == 0)
        return "headunit-hu204";
    return NULL;
}

size_t mblink_mercedes_controller_data_profile_identifier_count(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol)
{
    size_t count = 0U;
    size_t index;

    if (profile_key == NULL || profile_key[0] == '\0') return 0U;
    for (index = 0U;
         index < sizeof(controller_data_profile) /
             sizeof(controller_data_profile[0]);
         ++index) {
        if (controller_data_profile[index].protocol == protocol &&
            strcmp(controller_data_profile[index].profile_key,
                   profile_key) == 0) {
            ++count;
        }
    }
    return count;
}

const MblinkMercedesControllerDataProfileEntry *
mblink_mercedes_controller_data_profile_identifier_at(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol,
    size_t requested_index)
{
    size_t match_index = 0U;
    size_t index;

    if (profile_key == NULL || profile_key[0] == '\0') return NULL;
    for (index = 0U;
         index < sizeof(controller_data_profile) /
             sizeof(controller_data_profile[0]);
         ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            &controller_data_profile[index];
        if (entry->protocol != protocol ||
            strcmp(entry->profile_key, profile_key) != 0) {
            continue;
        }
        if (match_index == requested_index) return entry;
        ++match_index;
    }
    return NULL;
}

const MblinkMercedesControllerDataProfileEntry *
mblink_mercedes_controller_data_profile_find(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol,
    uint16_t identifier)
{
    size_t index;
    const size_t count =
        mblink_mercedes_controller_data_profile_identifier_count(
            profile_key, protocol);
    for (index = 0U; index < count; ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            mblink_mercedes_controller_data_profile_identifier_at(
                profile_key, protocol, index);
        if (entry != NULL && entry->identifier == identifier) return entry;
    }
    return NULL;
}

size_t mblink_mercedes_data_runtime_candidate_identifier_count_for_route(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id)
{
    (void)tx_can_id;
    (void)rx_can_id;
    (void)extended_id;
    return 0U;
}

uint16_t mblink_mercedes_data_runtime_candidate_identifier_at_for_route(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    size_t index)
{
    (void)tx_can_id;
    (void)rx_can_id;
    (void)extended_id;
    (void)index;
    return 0U;
}

bool mblink_mercedes_data_identifier_is_runtime_refreshable(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    uint16_t identifier)
{
    (void)tx_can_id;
    (void)rx_can_id;
    (void)extended_id;

    /*
     * DaimlerChrysler KWP local records E0-EB are identification/configuration
     * metadata. They can be discovered and displayed, but repeatedly reading
     * them adds bus traffic without producing live telemetry.
     */
    if (protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 &&
        identifier >= UINT16_C(0x00e0) &&
        identifier <= UINT16_C(0x00eb)) {
        return false;
    }

    if (protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000)
        return identifier != 0U && identifier <= UINT16_C(0x00ff);

    return true;
}

static MblinkMercedesDataScanResult initialise_scan(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config)
{
    if (scan == NULL || !mblink_mercedes_data_scan_config_is_valid(config))
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
    memset(scan, 0, sizeof(*scan));
    scan->config = *config;
    scan->current_identifier = config->first_identifier;
    scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_INIT_PROTOCOL;
    scan->failure = MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config)
{
    return initialise_scan(scan, config);
}

static MblinkMercedesDataScanResult begin_identifier_list(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const uint16_t *identifiers,
    size_t identifier_count,
    bool retry_no_response)
{
    MblinkMercedesDataScanResult result;
    size_t index;
    size_t previous;

    if (identifiers == NULL || identifier_count == 0U ||
        identifier_count > MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS) {
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
    }

    result = initialise_scan(scan, config);
    if (result != MBLINK_MERCEDES_DATA_SCAN_RESULT_OK) return result;

    for (index = 0U; index < identifier_count; ++index) {
        const uint16_t identifier = identifiers[index];
        if (config->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000 &&
            (identifier == 0U || identifier > UINT16_C(0x00ff))) {
            memset(scan, 0, sizeof(*scan));
            return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
        }
        for (previous = 0U; previous < index; ++previous) {
            if (identifiers[previous] == identifier) {
                memset(scan, 0, sizeof(*scan));
                return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
            }
        }
        scan->identifiers[index] = identifier;
    }

    scan->identifier_list_active = true;
    scan->identifier_list_retry_no_response = retry_no_response;
    scan->identifier_count = identifier_count;
    scan->identifier_index = 0U;
    scan->current_identifier = scan->identifiers[0U];
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin_identifiers(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const uint16_t *identifiers,
    size_t identifier_count)
{
    return begin_identifier_list(
        scan, config, identifiers, identifier_count, true);
}

MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin_probe_identifiers(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const uint16_t *identifiers,
    size_t identifier_count)
{
    return begin_identifier_list(
        scan, config, identifiers, identifier_count, false);
}

MblinkMercedesDataScanResult mblink_mercedes_data_scan_command(
    const MblinkMercedesDataScan *scan,
    char *buffer,
    size_t buffer_size,
    size_t *written)
{
    char command[16];
    int count;

    if (scan == NULL || buffer == NULL || written == NULL)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;

    switch (scan->stage) {
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_INIT_PROTOCOL:
        return write_text(scan->config.extended_id ? "ATSP7" : "ATSP6",
                          buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_HEADERS_OFF:
        return write_text("ATH0", buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_AUTO_FORMAT:
        return write_text("ATCAF1", buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_FLOW_CONTROL:
        return write_text("ATCFC1", buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TIMEOUT:
        /* 0x64 * 4 ms = 400 ms.  The old ATST20 (128 ms) was
         * proven too short by C207 captures: known-positive DIDs
         * repeatedly fell through to ELM NO DATA at the timeout. */
        return write_text("ATST64", buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_HEADER:
        return format_can_command(
            "ATSH", scan->config.tx_can_id, scan->config.extended_id,
            buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_RECEIVE:
        return format_can_command(
            "ATCRA", scan->config.rx_can_id, scan->config.extended_id,
            buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_EXTENDED_SESSION:
        return write_text("1003", buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT:
        return write_text(
            scan->config.protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                ? "3E01" : "3E00",
            buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER:
        count = scan->config.protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
            ? snprintf(command, sizeof(command), "21%02X",
                       (unsigned int)scan->current_identifier)
            : snprintf(command, sizeof(command), "22%04X",
                       (unsigned int)scan->current_identifier);
        if (count < 0 || (size_t)count >= sizeof(command))
            return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
        return write_text(command, buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE:
        if (buffer_size != 0U) buffer[0] = '\0';
        *written = 0U;
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_FAILED:
        break;
    }
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE;
}

static MblinkMercedesDataScanResult accept_adapter_step(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response,
    MblinkMercedesDataScanStage next)
{
    if (!at_ok(response))
        return fail_scan(
            scan, MBLINK_MERCEDES_DATA_SCAN_RESULT_ADAPTER_ERROR);
    scan->stage = next;
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

static bool retry_known_identifier_after_no_response(
    MblinkMercedesDataScan *scan)
{
    if (scan == NULL || !scan->identifier_list_active ||
        !scan->identifier_list_retry_no_response ||
        scan->current_no_response_retries >= 2U) {
        return false;
    }
    scan->current_no_response_retries++;
    return true;
}

static void accept_uds_identifier(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_DATA_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkUdsDidRecord record;
    MblinkUdsResult result;

    if (response->result != MBLINK_ELM327_RESULT_OK) {
        /*
         * A refresh list contains identifiers already proven positive.
         * One ELM timeout is therefore not evidence that the DID vanished.
         * Retry the same identifier twice before recording a miss.
         */
        if (retry_known_identifier_after_no_response(scan)) return;
        scan->no_response_count++;
        advance_identifier(scan);
        return;
    }
    if (mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
        MBLINK_ELM327_CAN_RESULT_OK) {
        scan->invalid_count++;
        advance_identifier(scan);
        return;
    }
    result = mblink_uds_decode_read_did_response(
        pdu, pdu_length, scan->current_identifier, &record);
    if (result == MBLINK_UDS_RESULT_OK) {
        record_positive(
            scan, MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
            scan->current_identifier, record.data, record.data_length);
    } else if (result == MBLINK_UDS_RESULT_NEGATIVE_RESPONSE) {
        scan->negative_count++;
    } else {
        scan->invalid_count++;
    }
    advance_identifier(scan);
}

static void accept_kwp_identifier(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_DATA_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkKwp2000LocalIdentifierRecord record;
    MblinkKwp2000Result result;

    if (response->result != MBLINK_ELM327_RESULT_OK) {
        /*
         * A refresh list contains identifiers already proven positive.
         * One ELM timeout is therefore not evidence that the DID vanished.
         * Retry the same identifier twice before recording a miss.
         */
        if (retry_known_identifier_after_no_response(scan)) return;
        scan->no_response_count++;
        advance_identifier(scan);
        return;
    }
    if (mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &pdu_length) !=
        MBLINK_ELM327_CAN_RESULT_OK) {
        scan->invalid_count++;
        advance_identifier(scan);
        return;
    }
    result = mblink_kwp2000_decode_read_local_identifier_response(
        pdu, pdu_length, (uint8_t)scan->current_identifier, &record);
    if (result == MBLINK_KWP2000_RESULT_OK) {
        record_positive(
            scan, MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER,
            scan->current_identifier, record.data, record.data_length);
    } else if (result == MBLINK_KWP2000_RESULT_NEGATIVE_RESPONSE) {
        scan->negative_count++;
    } else {
        scan->invalid_count++;
    }
    advance_identifier(scan);
}

MblinkMercedesDataScanResult mblink_mercedes_data_scan_accept(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response)
{
    if (scan == NULL || response == NULL)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;

    switch (scan->stage) {
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_INIT_PROTOCOL:
        return accept_adapter_step(
            scan, response, MBLINK_MERCEDES_DATA_SCAN_STAGE_HEADERS_OFF);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_HEADERS_OFF:
        return accept_adapter_step(
            scan, response, MBLINK_MERCEDES_DATA_SCAN_STAGE_AUTO_FORMAT);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_AUTO_FORMAT:
        return accept_adapter_step(
            scan, response, MBLINK_MERCEDES_DATA_SCAN_STAGE_FLOW_CONTROL);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_FLOW_CONTROL:
        return accept_adapter_step(
            scan, response, MBLINK_MERCEDES_DATA_SCAN_STAGE_TIMEOUT);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TIMEOUT:
        return accept_adapter_step(
            scan, response, MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_HEADER);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_HEADER:
        return accept_adapter_step(
            scan, response, MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_RECEIVE);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_RECEIVE:
        if (!at_ok(response))
            return fail_scan(
                scan, MBLINK_MERCEDES_DATA_SCAN_RESULT_ADAPTER_ERROR);
        scan->stage =
            scan->config.request_extended_session &&
            scan->config.protocol == MBLINK_MERCEDES_DIAGNOSTIC_UDS
                ? MBLINK_MERCEDES_DATA_SCAN_STAGE_EXTENDED_SESSION
                : MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT;
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_EXTENDED_SESSION:
        /*
         * The module was already discovered. A rejected/quiet extended-session
         * transition does not erase it; continue with read-only presence/data
         * requests because some ECUs expose actual values in the default
         * session while others prefer 0x10 03.
         */
        scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT;
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT:
        scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER;
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER:
        if (scan->config.protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000)
            accept_kwp_identifier(scan, response);
        else
            accept_uds_identifier(scan, response);
        return scan->stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE
            ? MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE
            : MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE:
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_FAILED:
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE;
    }
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE;
}

uint64_t mblink_mercedes_data_scan_timeout_ms(
    const MblinkMercedesDataScan *scan)
{
    if (scan == NULL) return UINT64_C(4000);
    if (scan->stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER)
        return UINT64_C(2500);
    return UINT64_C(4000);
}

size_t mblink_mercedes_data_scan_record_count(
    const MblinkMercedesDataScan *scan)
{
    if (scan == NULL) return 0U;
    return scan->positive_count > MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS
        ? MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS
        : scan->positive_count;
}

const MblinkMercedesDataRecord *mblink_mercedes_data_scan_record_at(
    const MblinkMercedesDataScan *scan,
    size_t index)
{
    if (scan == NULL || index >= mblink_mercedes_data_scan_record_count(scan))
        return NULL;
    return &scan->records[index];
}

bool mblink_mercedes_data_record_format_code(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size)
{
    int count;
    if (record == NULL || buffer == NULL || buffer_size == 0U) return false;
    if (record->service == MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER) {
        count = snprintf(
            buffer, buffer_size, "UDS DID 0x%04X",
            (unsigned int)record->identifier);
    } else if (
        record->service ==
        MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER) {
        count = snprintf(
            buffer, buffer_size, "KWP local ID 0x%02X",
            (unsigned int)record->identifier);
    } else {
        count = snprintf(
            buffer, buffer_size, "Data ID 0x%04X",
            (unsigned int)record->identifier);
    }
    return count >= 0 && (size_t)count < buffer_size;
}

bool mblink_mercedes_data_record_format_hex(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size)
{
    size_t offset = 0U;
    if (record == NULL || buffer == NULL || buffer_size == 0U) return false;
    buffer[0] = '\0';
    for (size_t index = 0U; index < record->data_length; ++index) {
        int count;
        if (offset + 3U > buffer_size) {
            buffer[0] = '\0';
            return false;
        }
        count = snprintf(
            buffer + offset, buffer_size - offset,
            "%02X", (unsigned int)record->data[index]);
        if (count != 2) {
            buffer[0] = '\0';
            return false;
        }
        offset += 2U;
    }
    return true;
}

bool mblink_mercedes_data_record_decode_known_numeric_for_route(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record,
    double *value,
    const char **name,
    const char **unit)
{
    if (value != NULL) *value = 0.0;
    if (name != NULL) *name = NULL;
    if (unit != NULL) *unit = NULL;
    if (record == NULL || value == NULL || name == NULL || unit == NULL)
        return false;

    /*
     * Current source-backed actual-value mapping from the C207/OM651 CRD3
     * evidence catalogue: DT_2007_IN_Battery_voltage.
     */
    if (module_kind == MBLINK_MERCEDES_MODULE_ENGINE &&
        record->service == MBLINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER &&
        record->identifier == UINT16_C(0x2007) &&
        record->data_length == 2U) {
        const uint16_t raw =
            (uint16_t)(((uint16_t)record->data[0] << 8U) |
                       record->data[1]);
        *value = (double)raw * 0.0078125;
        *name = "Battery voltage";
        *unit = "V";
        return true;
    }

    /*
     * Mercedes transmission oil temperature candidate:
     *
     *   physical request/response: 0x7E1 -> 0x7E9
     *   request: 21 30
     *   positive response: 61 30 ...
     *   Torque equation: L - 50
     *
     * L is byte 11 of the complete positive response. The generic KWP decoder
     * removes the leading 61 30 bytes before storing record->data, so the same
     * source-backed byte is record->data[9].
     *
     * This mapping is source-corroborated across several Mercedes 722.9/W204/
     * W212 reports but remains vehicle-unverified until a real MBLINK capture
     * returns a plausible changing value on the development C207.
     */
    if (!extended_id &&
        tx_can_id == UINT32_C(0x7e1) &&
        rx_can_id == UINT32_C(0x7e9) &&
        record->service ==
            MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER &&
        record->identifier == UINT16_C(0x0030)) {
        /*
         * Two public 21 30 layouts exist. A full DAS-compatible EGS52 RLI 30
         * carries ATF at data[11], while the shorter community 722.9 custom
         * PID evidence carries complete-response byte L at data[9]. Prefer the
         * richer, length-qualified RLI decoder so a long response can never be
         * misinterpreted as the compact layout.
         */
        MblinkMercedesKwpRli30 rli;
        if (mblink_mercedes_transmission_decode_kwp_rli30(
                record->data, record->data_length, &rli)) {
            *value = rli.atf_temperature_c;
            *name = "Transmission oil temperature";
            *unit = "°C";
            return true;
        }
        {
            MblinkMercedesTransmission2130 decoded;
            if (mblink_mercedes_transmission_decode_2130(
                    record->data, record->data_length, &decoded) &&
                decoded.oil_temperature_available) {
                *value = decoded.oil_temperature_c;
                *name = "Transmission oil temperature";
                *unit = "°C";
                return true;
            }
        }
    }

    return false;
}


static uint32_t data_be24(const uint8_t *data)
{
    return ((uint32_t)data[0] << 16U) |
           ((uint32_t)data[1] << 8U) |
           (uint32_t)data[2];
}

static bool record_is_transmission_kwp(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record)
{
    const bool explicit_gs_route =
        !extended_id &&
        tx_can_id == UINT32_C(0x7e1) &&
        rx_can_id == UINT32_C(0x7e9);
    return (module_kind == MBLINK_MERCEDES_MODULE_TRANSMISSION ||
            explicit_gs_route) &&
        record != NULL &&
        record->service ==
            MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER;
}

static bool format_ascii_payload(
    const uint8_t *data,
    size_t data_length,
    char *buffer,
    size_t buffer_size)
{
    size_t length = data_length;
    if (data == NULL || buffer == NULL || buffer_size == 0U)
        return false;
    while (length > 0U &&
           (data[length - 1U] == UINT8_C(0x00) ||
            data[length - 1U] == UINT8_C(0xff))) {
        --length;
    }
    if (length == 0U || length + 1U > buffer_size) return false;
    for (size_t index = 0U; index < length; ++index) {
        if (data[index] < UINT8_C(0x20) || data[index] > UINT8_C(0x7e))
            return false;
        buffer[index] = (char)data[index];
    }
    buffer[length] = '\0';
    return true;
}

bool mblink_mercedes_data_record_format_known_for_route(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size,
    const char **name)
{
    int count;
    if (name != NULL) *name = NULL;
    if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
    if (record == NULL || buffer == NULL || buffer_size == 0U ||
        name == NULL || !record_is_transmission_kwp(
            tx_can_id, rx_can_id, extended_id, module_kind, record)) {
        return false;
    }

    switch ((uint8_t)record->identifier) {
    case UINT8_C(0x30): {
        MblinkMercedesKwpRli30 decoded;
        MblinkMercedesTransmission2130 compact;
        *name = "Transmission actual values";
        if (mblink_mercedes_transmission_decode_kwp_rli30(
                record->data, record->data_length, &decoded)) {
            count = snprintf(
                buffer, buffer_size,
                "ATF %.1f °C · actual %s · target %s · "
                "TCC state %u · TCC Δspeed %u · TCC speed %u · "
                "TCC pressure raw %u · output speed raw %u · "
                "engine torque raw %u · converter torque raw %u · "
                "selector 0x%02X · program 0x%02X · "
                "kickdown %s · emergency %s · ASR %s · "
                "solenoids 12/45=%s 2/3=%s 3/4=%s",
                decoded.atf_temperature_c,
                mblink_mercedes_transmission_actual_gear_name(
                    decoded.actual_gear_code),
                mblink_mercedes_transmission_target_gear_name(
                    decoded.target_gear_code),
                (unsigned int)decoded.tcc_status,
                (unsigned int)decoded.tcc_delta_speed_raw,
                (unsigned int)decoded.tcc_speed_raw,
                (unsigned int)decoded.tcc_pressure_raw,
                (unsigned int)decoded.output_speed_raw,
                (unsigned int)decoded.engine_torque_raw,
                (unsigned int)decoded.converter_torque_raw,
                (unsigned int)decoded.selector_position,
                (unsigned int)decoded.drive_program,
                decoded.kickdown ? "yes" : "no",
                decoded.emergency_mode ? "yes" : "no",
                decoded.asr_active ? "active" : "inactive",
                decoded.solenoid_1245 ? "on" : "off",
                decoded.solenoid_23 ? "on" : "off",
                decoded.solenoid_34 ? "on" : "off");
            return count >= 0 && (size_t)count < buffer_size;
        }
        if (mblink_mercedes_transmission_decode_2130(
                record->data, record->data_length, &compact)) {
            char gear_text[32];
            if (compact.actual_gear_code == 0U) {
                (void)snprintf(gear_text, sizeof(gear_text), "%s", "N");
            } else if (compact.actual_gear_code >= 1U &&
                       compact.actual_gear_code <= 7U) {
                (void)snprintf(
                    gear_text, sizeof(gear_text), "%u",
                    (unsigned int)compact.actual_gear_code);
            } else if (compact.actual_gear_code == 11U ||
                       compact.actual_gear_code == 13U) {
                (void)snprintf(
                    gear_text, sizeof(gear_text),
                    "P/R candidate code 0x%X",
                    (unsigned int)compact.actual_gear_code);
            } else {
                (void)snprintf(
                    gear_text, sizeof(gear_text), "code 0x%X",
                    (unsigned int)compact.actual_gear_code);
            }
            count = snprintf(
                buffer, buffer_size,
                "ATF %.1f °C · current gear %s",
                compact.oil_temperature_c, gear_text);
            return count >= 0 && (size_t)count < buffer_size;
        }
        return false;
    }
    case UINT8_C(0x31): {
        MblinkMercedesKwpRli31 decoded;
        *name = "Transmission speed sensors";
        if (!mblink_mercedes_transmission_decode_kwp_rli31(
                record->data, record->data_length, &decoded)) {
            return false;
        }
        count = snprintf(
            buffer, buffer_size,
            "N2 %u · N3 %u · input %u rpm · engine %u rpm · "
            "wheel raw FL %u FR %u RL %u RR %u · "
            "vehicle-speed raw rear %u front %u",
            (unsigned int)decoded.n2_pulse_count,
            (unsigned int)decoded.n3_pulse_count,
            (unsigned int)decoded.input_rpm,
            (unsigned int)decoded.engine_rpm,
            (unsigned int)decoded.front_left_wheel_speed_raw,
            (unsigned int)decoded.front_right_wheel_speed_raw,
            (unsigned int)decoded.rear_left_wheel_speed_raw,
            (unsigned int)decoded.rear_right_wheel_speed_raw,
            (unsigned int)decoded.rear_vehicle_speed_raw,
            (unsigned int)decoded.front_vehicle_speed_raw);
        return count >= 0 && (size_t)count < buffer_size;
    }
    case UINT8_C(0x32): {
        MblinkMercedesKwpRli32 decoded;
        *name = "Transmission driving dynamics";
        if (!mblink_mercedes_transmission_decode_kwp_rli32(
                record->data, record->data_length, &decoded)) {
            return false;
        }
        count = snprintf(
            buffer, buffer_size,
            "pedal %u%% · pedal Δ %u · upshift Δrpm raw %u · "
            "downshift Δrpm raw %u · pitch raw %u · "
            "driving state 0x%02X · warmup shift 0x%02X · "
            "requested gear range %u..%u",
            (unsigned int)decoded.pedal_percent,
            (unsigned int)decoded.pedal_delta_percent,
            (unsigned int)decoded.upshift_delta_rpm_raw,
            (unsigned int)decoded.downshift_delta_rpm_raw,
            (unsigned int)decoded.pitch_raw,
            (unsigned int)decoded.driving_status,
            (unsigned int)decoded.engine_warmup_shift_state,
            (unsigned int)decoded.requested_low_gear_limit,
            (unsigned int)decoded.requested_high_gear_limit);
        return count >= 0 && (size_t)count < buffer_size;
    }
    case UINT8_C(0x33): {
        MblinkMercedesKwpRli33 decoded;
        *name = "Transmission hydraulics and solenoids";
        if (!mblink_mercedes_transmission_decode_kwp_rli33(
                record->data, record->data_length, &decoded)) {
            return false;
        }
        count = snprintf(
            buffer, buffer_size,
            "valves 0x%02X/state %u · SPC pressure raw %u · "
            "MPC pressure raw %u · SPC target/actual current raw %u/%u · "
            "MPC target/actual current raw %u/%u · TCC PWM raw %u",
            (unsigned int)decoded.valve_flag,
            (unsigned int)decoded.shift_valve_state,
            (unsigned int)decoded.spc_pressure_raw,
            (unsigned int)decoded.mpc_pressure_raw,
            (unsigned int)decoded.spc_target_current_raw,
            (unsigned int)decoded.spc_actual_current_raw,
            (unsigned int)decoded.mpc_target_current_raw,
            (unsigned int)decoded.mpc_actual_current_raw,
            (unsigned int)decoded.tcc_pwm_raw);
        return count >= 0 && (size_t)count < buffer_size;
    }
    case UINT8_C(0xe0):
        *name = "Development data";
        /*
         * DaimlerChrysler KWP2000 Requirements Definition 2.2, table
         * 4.4.4-9: processor type followed by communication-matrix, CAN
         * driver, network-management, KWP, transport, DBKOM and Flexer
         * versions. Version bytes are BCD, so display the wire representation
         * directly rather than pretending a decimal conversion is valid for
         * malformed vendor data.
         */
        if (record->data_length >= 18U) {
            count = snprintf(
                buffer, buffer_size,
                "processor 0x%02X%02X · matrix week/year %02X/%02X · "
                "CAN %02X.%02X · NM %02X.%02X · KWP %02X.%02X · "
                "transport %02X.%02X · DBKOM %02X.%02X · "
                "Flexer %02X.%02X · reserved %02X%02X",
                (unsigned int)record->data[0],
                (unsigned int)record->data[1],
                (unsigned int)record->data[2],
                (unsigned int)record->data[3],
                (unsigned int)record->data[4],
                (unsigned int)record->data[5],
                (unsigned int)record->data[6],
                (unsigned int)record->data[7],
                (unsigned int)record->data[8],
                (unsigned int)record->data[9],
                (unsigned int)record->data[10],
                (unsigned int)record->data[11],
                (unsigned int)record->data[12],
                (unsigned int)record->data[13],
                (unsigned int)record->data[14],
                (unsigned int)record->data[15],
                (unsigned int)record->data[16],
                (unsigned int)record->data[17]);
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    case UINT8_C(0xe1):
        *name = "ECU serial number";
        if (format_ascii_payload(
                record->data, record->data_length,
                buffer, buffer_size)) {
            return true;
        }
        break;
    case UINT8_C(0xe2):
        *name = "DBCom communication-matrix data";
        /*
         * Table 4.4.4-11 defines three 24-bit address/size pairs for Flash,
         * RAM and EEPROM plus the Flash data-format identifier.
         */
        if (record->data_length >= 19U) {
            count = snprintf(
                buffer, buffer_size,
                "Flash 0x%06X +%u bytes (format 0x%02X) · "
                "RAM 0x%06X +%u bytes · EEPROM 0x%06X +%u bytes",
                (unsigned int)data_be24(&record->data[0]),
                (unsigned int)data_be24(&record->data[4]),
                (unsigned int)record->data[3],
                (unsigned int)data_be24(&record->data[7]),
                (unsigned int)data_be24(&record->data[10]),
                (unsigned int)data_be24(&record->data[13]),
                (unsigned int)data_be24(&record->data[16]));
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    case UINT8_C(0xe3):
        *name = "Operating-system version";
        if (record->data_length != 0U) {
            size_t used = 0U;
            count = snprintf(buffer, buffer_size, "0x");
            if (count < 0 || (size_t)count >= buffer_size) return false;
            used = (size_t)count;
            for (size_t index = 0U; index < record->data_length; ++index) {
                count = snprintf(
                    buffer + used, buffer_size - used, "%02X",
                    (unsigned int)record->data[index]);
                if (count < 0 || (size_t)count >= buffer_size - used)
                    return false;
                used += (size_t)count;
            }
            return true;
        }
        break;
    case UINT8_C(0xe4):
        *name = "ECU reprogramming identification";
        break;
    case UINT8_C(0xe5):
        *name = "Vehicle information";
        if (record->data_length >= 4U) {
            count = snprintf(
                buffer, buffer_size,
                "model year 0x%02X · vehicle 0x%02X · "
                "body style 0x%02X · country 0x%02X",
                (unsigned int)record->data[0],
                (unsigned int)record->data[1],
                (unsigned int)record->data[2],
                (unsigned int)record->data[3]);
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    case UINT8_C(0xe6):
        *name = "Flash information 1";
        break;
    case UINT8_C(0xe7):
        *name = "Flash information 2";
        if (record->data_length == 19U) {
            count = snprintf(
                buffer, buffer_size,
                "19-byte hardware-scan/programming-statistics record retained raw");
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    case UINT8_C(0xe8):
        *name = "System-diagnostic general parameters";
        /*
         * DaimlerChrysler KWP2000 Requirements Definition 2.2 defines the
         * fixed prefix: communication/global-process-data flags, SDCOM
         * version/build metadata, configuration/reference and checksum bytes.
         * Keep BCD-looking fields hexadecimal here instead of silently
         * converting malformed manufacturer data into calendar numbers.
         */
        if (record->data_length >= 15U) {
            count = snprintf(
                buffer, buffer_size,
                "flags 0x%02X · SDCOM version %02X.%02X.%02X · "
                "build %02X%02X-%02X-%02X · config 0x%02X%02X · "
                "reference 0x%02X%02X · checksum %02X%02X%02X · "
                "%zu byte%s total",
                (unsigned int)record->data[0],
                (unsigned int)record->data[1],
                (unsigned int)record->data[2],
                (unsigned int)record->data[3],
                (unsigned int)record->data[4],
                (unsigned int)record->data[5],
                (unsigned int)record->data[6],
                (unsigned int)record->data[7],
                (unsigned int)record->data[8],
                (unsigned int)record->data[9],
                (unsigned int)record->data[10],
                (unsigned int)record->data[11],
                (unsigned int)record->data[12],
                (unsigned int)record->data[13],
                (unsigned int)record->data[14],
                record->data_length,
                record->data_length == 1U ? "" : "s");
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    case UINT8_C(0xe9):
        *name = "System-diagnostic global parameters";
        if (record->data_length >= 4U) {
            const uint16_t first_can_position =
                (uint16_t)(((uint16_t)record->data[2] << 8U) |
                           (uint16_t)record->data[3]);
            count = snprintf(
                buffer, buffer_size,
                "global analog values %u · global states %u · "
                "first CAN data-frame position %u · "
                "%zu descriptor byte%s retained",
                (unsigned int)record->data[0],
                (unsigned int)record->data[1],
                (unsigned int)first_can_position,
                record->data_length - 4U,
                record->data_length - 4U == 1U ? "" : "s");
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    case UINT8_C(0xea):
        *name = "ECU configuration";
        break;
    case UINT8_C(0xeb):
        *name = "Diagnostic protocol information";
        if (record->data_length >= 3U) {
            count = snprintf(
                buffer, buffer_size,
                "KWP requirements version 0x%02X · "
                "flash requirements version 0x%02X · "
                "diagnostic level %u",
                (unsigned int)record->data[0],
                (unsigned int)record->data[1],
                (unsigned int)record->data[2]);
            return count >= 0 && (size_t)count < buffer_size;
        }
        break;
    default:
        return false;
    }

    /*
     * The Daimler KWP standard defines these record identities, but several
     * payload formats are ECU/program-specific. Preserve bytes losslessly and
     * label the record without inventing a decoder.
     */
    count = snprintf(
        buffer, buffer_size, "Source-defined record · %zu byte%s retained raw",
        record->data_length, record->data_length == 1U ? "" : "s");
    return count >= 0 && (size_t)count < buffer_size;
}

bool mblink_mercedes_data_record_decode_known_numeric(
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record,
    double *value,
    const char **name,
    const char **unit)
{
    return mblink_mercedes_data_record_decode_known_numeric_for_route(
        0U, 0U, false, module_kind, record, value, name, unit);
}
