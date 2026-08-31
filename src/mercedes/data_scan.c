// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_data_scan.h"

#include "mblink/elm327_can.h"
#include "mblink/kwp2000.h"
#include "mblink/uds.h"

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

MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin_identifiers(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const uint16_t *identifiers,
    size_t identifier_count)
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
    scan->identifier_count = identifier_count;
    scan->identifier_index = 0U;
    scan->current_identifier = scan->identifiers[0U];
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
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
        return write_text("ATST20", buffer, buffer_size, written);
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

static void accept_uds_identifier(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_DATA_SCAN_PDU_CAPACITY];
    size_t pdu_length = 0U;
    MblinkUdsDidRecord record;
    MblinkUdsResult result;

    if (response->result != MBLINK_ELM327_RESULT_OK) {
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

bool mblink_mercedes_data_record_decode_known_numeric(
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
    return false;
}
