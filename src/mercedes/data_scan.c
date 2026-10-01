// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_data_scan.h"
#include "mblink/mercedes_documented_ecus.h"

#include "mblink/elm327_can.h"
#include "mblink/kwp2000.h"
#include "mblink/mercedes_transmission.h"
#include "mblink/mercedes_module_catalog.h"
#include "mblink/uds.h"

#include "infiltratr/core.h"
#include "infiltratr/endian.h"

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
    if (length >= buffer_size)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
    infiltratr_copy_string(buffer, buffer_size, text);
    *written = length;
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

static void finish_identifier_scan(MblinkMercedesDataScan *scan)
{
    char quit_command[5];

    if (scan == NULL) return;
    scan->stage =
        mblink_mercedes_documented_route_control_command(
            scan->config.tx_can_id, scan->config.rx_can_id,
            scan->config.extended_id, scan->config.protocol, true,
            quit_command, sizeof(quit_command))
            ? MBLINK_MERCEDES_DATA_SCAN_STAGE_QUIT_SESSION
            : MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE;
}

static void advance_identifier(MblinkMercedesDataScan *scan)
{
    if (scan == NULL) return;
    scan->current_transient_retries = 0U;
    scan->attempted_count++;

    if (scan->identifier_list_active) {
        scan->identifier_index++;
        if (scan->identifier_index >= scan->identifier_count) {
            finish_identifier_scan(scan);
            return;
        }
        scan->current_identifier =
            scan->identifiers[scan->identifier_index];
        scan->current_service = scan->services[scan->identifier_index];
        return;
    }

    if (scan->current_identifier >= scan->config.last_identifier) {
        finish_identifier_scan(scan);
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
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_QUIT_SESSION:
        return "quit-session";
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
    const MblinkMercedesKnownRoute *route;
    memset(&config, 0, sizeof(config));
    config.tx_can_id = tx_can_id;
    config.rx_can_id = rx_can_id;
    config.extended_id = extended_id;
    config.protocol = protocol;
    config.module_kind = module_kind;
    route = !extended_id
        ? mblink_mercedes_known_route_for_tx(tx_can_id) : NULL;
    config.request_extended_session =
        route != NULL && route->rx_can_id == rx_can_id &&
        route->protocol == protocol &&
        mblink_mercedes_known_route_allows_automatic_extended_session(route);

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
        scan->services[index] =
            config->protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                ? UINT8_C(0x21) : UINT8_C(0x22);
    }

    scan->identifier_list_active = true;
    scan->identifier_list_retry_no_response = retry_no_response;
    scan->identifier_count = identifier_count;
    scan->identifier_index = 0U;
    scan->current_identifier = scan->identifiers[0U];
    scan->current_service = scan->services[0U];
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

MblinkMercedesDataScanResult mblink_mercedes_data_scan_begin_probe_commands(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const MblinkMercedesDataProbeCommand *commands,
    size_t command_count)
{
    MblinkMercedesDataScanResult result;
    if(scan==NULL||config==NULL||commands==NULL||command_count==0U||command_count>MBLINK_MERCEDES_DATA_SCAN_MAX_RECORDS)
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;
    result=initialise_scan(scan,config);if(result!=MBLINK_MERCEDES_DATA_SCAN_RESULT_OK)return result;
    for(size_t i=0U;i<command_count;++i){
        const uint8_t service=commands[i].service;const uint16_t identifier=commands[i].identifier;bool valid=false;
        if(config->protocol==MBLINK_MERCEDES_DIAGNOSTIC_UDS)valid=service==0x22;
        else if(config->protocol==MBLINK_MERCEDES_DIAGNOSTIC_KWP2000)valid=(service==0x21||service==0x1a)&&identifier!=0U&&identifier<=0xffU;
        if(!valid||!mblink_mercedes_documented_read_is_safe(service,identifier)){memset(scan,0,sizeof(*scan));return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;}
        for(size_t p=0U;p<i;++p)if(scan->services[p]==service&&scan->identifiers[p]==identifier){memset(scan,0,sizeof(*scan));return MBLINK_MERCEDES_DATA_SCAN_RESULT_INVALID_ARGUMENT;}
        scan->services[i]=service;scan->identifiers[i]=identifier;
    }
    scan->identifier_list_active=true;scan->identifier_list_retry_no_response=false;scan->identifier_count=command_count;scan->identifier_index=0U;scan->current_service=scan->services[0U];scan->current_identifier=scan->identifiers[0U];
    return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
}

MblinkMercedesDataScanResult
mblink_mercedes_data_scan_begin_documented_commands(
    MblinkMercedesDataScan *scan,
    const MblinkMercedesDataScanConfig *config,
    const MblinkMercedesDataProbeCommand *commands,
    size_t command_count)
{
    return mblink_mercedes_data_scan_begin_probe_commands(
        scan, config, commands, command_count);
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
        if (mblink_elm327_can_format_header_command(
                scan->config.tx_can_id, scan->config.extended_id,
                buffer, buffer_size) != MBLINK_ELM327_CAN_RESULT_OK) {
            if (written != NULL) *written = 0U;
            return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
        }
        *written = strlen(buffer);
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_SET_RECEIVE:
        if (mblink_elm327_can_format_receive_address_command(
                scan->config.rx_can_id, scan->config.extended_id,
                buffer, buffer_size) != MBLINK_ELM327_CAN_RESULT_OK) {
            if (written != NULL) *written = 0U;
            return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
        }
        *written = strlen(buffer);
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_EXTENDED_SESSION: {
        char session_command[5];
        return write_text(
            mblink_mercedes_documented_route_control_command(
                scan->config.tx_can_id, scan->config.rx_can_id,
                scan->config.extended_id, scan->config.protocol, false,
                session_command, sizeof(session_command))
                ? session_command : "1003",
            buffer, buffer_size, written);
    }
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_TESTER_PRESENT:
        return write_text(
            scan->config.protocol == MBLINK_MERCEDES_DIAGNOSTIC_KWP2000
                ? "3E01" : "3E00",
            buffer, buffer_size, written);
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_READ_IDENTIFIER: {
        const uint8_t service=scan->identifier_list_active?scan->current_service:(scan->config.protocol==MBLINK_MERCEDES_DIAGNOSTIC_KWP2000?UINT8_C(0x21):UINT8_C(0x22));
        if(service==0x22)count=snprintf(command,sizeof(command),"22%04X",(unsigned int)scan->current_identifier);
        else if(service==0x21||service==0x1a)count=snprintf(command,sizeof(command),"%02X%02X",(unsigned int)service,(unsigned int)scan->current_identifier);
        else return MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE;
        if(count<0||(size_t)count>=sizeof(command))return MBLINK_MERCEDES_DATA_SCAN_RESULT_BUFFER_TOO_SMALL;
        return write_text(command,buffer,buffer_size,written);
    }
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_QUIT_SESSION: {
        char quit_command[5];
        if (!mblink_mercedes_documented_route_control_command(
                scan->config.tx_can_id, scan->config.rx_can_id,
                scan->config.extended_id, scan->config.protocol, true,
                quit_command, sizeof(quit_command))) {
            return MBLINK_MERCEDES_DATA_SCAN_RESULT_FAILED_STATE;
        }
        return write_text(quit_command, buffer, buffer_size, written);
    }
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

static bool retry_current_identifier_after_transient(
    MblinkMercedesDataScan *scan,
    bool require_known_positive)
{
    if (scan == NULL || scan->current_transient_retries >= 2U)
        return false;
    if (require_known_positive &&
        (!scan->identifier_list_active ||
         !scan->identifier_list_retry_no_response)) {
        return false;
    }
    scan->current_transient_retries++;
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
        if (retry_current_identifier_after_transient(scan, true)) return;
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
    if (pdu_length >= 3U &&
        pdu[0] == UINT8_C(0x62) &&
        infiltratr_load_be16(pdu + 1U) != scan->current_identifier) {
        /*
         * The ELM can deliver a late positive response after the next command
         * has already been written. Never associate that response with the
         * current DID, and retry the current request before moving on.
         */
        if (retry_current_identifier_after_transient(scan, false)) return;
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
        if (retry_current_identifier_after_transient(scan, true)) return;
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
    if (pdu_length >= 2U &&
        pdu[0] == UINT8_C(0x61) &&
        pdu[1] != (uint8_t)scan->current_identifier) {
        if (retry_current_identifier_after_transient(scan, false)) return;
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

static void accept_kwp_ecu_identification(
    MblinkMercedesDataScan *scan,
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_DATA_SCAN_PDU_CAPACITY];
    size_t length = 0U;

    if (response->result != MBLINK_ELM327_RESULT_OK) {
        if (retry_current_identifier_after_transient(scan, true)) return;
        scan->no_response_count++;
        advance_identifier(scan);
        return;
    }
    if (mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &length) !=
            MBLINK_ELM327_CAN_RESULT_OK ||
        length < 2U) {
        scan->invalid_count++;
        advance_identifier(scan);
        return;
    }
    if (pdu[0] == UINT8_C(0x5a) &&
        pdu[1] != (uint8_t)scan->current_identifier) {
        if (retry_current_identifier_after_transient(scan, false)) return;
        scan->invalid_count++;
        advance_identifier(scan);
        return;
    }
    if (pdu[0] == UINT8_C(0x5a) &&
        pdu[1] == (uint8_t)scan->current_identifier) {
        record_positive(
            scan, UINT8_C(0x1a), scan->current_identifier,
            pdu + 2U, length - 2U);
    } else if (length >= 3U &&
               pdu[0] == UINT8_C(0x7f) &&
               pdu[1] == UINT8_C(0x1a)) {
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
        if(scan->identifier_list_active&&scan->current_service==0x1a)accept_kwp_ecu_identification(scan,response);
        else if(scan->config.protocol==MBLINK_MERCEDES_DIAGNOSTIC_KWP2000)accept_kwp_identifier(scan,response);
        else accept_uds_identifier(scan,response);
        return scan->stage == MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE
            ? MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE
            : MBLINK_MERCEDES_DATA_SCAN_RESULT_OK;
    case MBLINK_MERCEDES_DATA_SCAN_STAGE_QUIT_SESSION:
        /*
         * Teardown is best-effort: a controller is allowed to return to normal
         * operation without acknowledging its default/quit-session command.
         * Do not keep retransmitting the command and accidentally hold an
         * infotainment ECU such as HU_204 in diagnostics.
         */
        scan->stage = MBLINK_MERCEDES_DATA_SCAN_STAGE_COMPLETE;
        return MBLINK_MERCEDES_DATA_SCAN_RESULT_COMPLETE;
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
    } else if (record->service == UINT8_C(0x1a)) {
        count = snprintf(buffer, buffer_size, "KWP ECU ID 0x%02X",
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
        const uint16_t raw = infiltratr_load_be16(record->data);
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

static bool record_is_orc_kwp(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesModuleKind module_kind,
    const MblinkMercedesDataRecord *record)
{
    return !extended_id &&
        tx_can_id == UINT32_C(0x64a) &&
        rx_can_id == UINT32_C(0x489) &&
        module_kind == MBLINK_MERCEDES_MODULE_RESTRAINTS &&
        record != NULL &&
        record->service ==
            MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER;
}

static bool format_orc_kwp_record(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size,
    const char **name)
{
    int count;

    if (record == NULL || buffer == NULL || buffer_size == 0U ||
        name == NULL) {
        return false;
    }

    switch ((uint8_t)record->identifier) {
    case UINT8_C(0x02):
        /*
         * Foxwell/Xentry ORC metadata uses 21 02 as the filter source for
         * installed restraint equipment (pyro fuse, curtain/rear side bags,
         * driver knee bag and rear-door pressure satellites). The public
         * source establishes the record's semantics but not a sufficiently
         * precise portable bit layout for every ORC generation, so preserve
         * the configuration bytes rather than inventing individual states.
         */
        *name = "Restraint equipment configuration";
        if (record->data_length == 0U) return false;
        {
            size_t offset = 0U;
            for (size_t index = 0U; index < record->data_length; ++index) {
                const int written = snprintf(
                    buffer + offset, buffer_size - offset,
                    index == 0U ? "Configuration %02X" : "%02X",
                    (unsigned int)record->data[index]);
                if (written <= 0 ||
                    (size_t)written >= buffer_size - offset) {
                    buffer[0] = '\0';
                    return false;
                }
                offset += (size_t)written;
            }
        }
        return true;

    case UINT8_C(0x58):
        /*
         * Mercedes ORC_204 CBF names 21 58 ECU lock state. A Mercedes DAS
         * restraint-controller report independently exposes the adjacent
         * four-byte tester identification. The captured C207 response
         * 61 58 00 90 55 68 00 therefore has a one-byte lock-state code
         * followed by tester ID 90556800. Do not assign locked/unlocked
         * semantics to the status code until its enum is source-backed.
         */
        *name = "ECU lock state / tester identification";
        if (record->data_length != 5U) return false;
        count = snprintf(
            buffer, buffer_size,
            "Lock state 0x%02X · tester identification %02X%02X%02X%02X",
            (unsigned int)record->data[0],
            (unsigned int)record->data[1],
            (unsigned int)record->data[2],
            (unsigned int)record->data[3],
            (unsigned int)record->data[4]);
        return count >= 0 && (size_t)count < buffer_size;

    default:
        return false;
    }
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

static bool documented_field_u8(
    const MblinkMercedesDataRecord *record,
    const MblinkMercedesDocumentedField *field,
    uint8_t *value)
{
    size_t payload_response_byte;
    size_t data_index;

    if (record == NULL || field == NULL || value == NULL ||
        field->bit_offset != -1 || field->bit_length != 8U) {
        return false;
    }

    if (record->service == UINT8_C(0x22)) {
        payload_response_byte = 4U;
    } else if (record->service == UINT8_C(0x21) ||
               record->service == UINT8_C(0x1a)) {
        payload_response_byte = 3U;
    } else {
        return false;
    }

    if (field->response_byte < payload_response_byte) return false;
    data_index = (size_t)field->response_byte - payload_response_byte;
    if (data_index >= record->data_length) return false;

    *value = record->data[data_index];
    return true;
}

static bool format_documented_year_week_patch(
    const MblinkMercedesDataRecord *record,
    char *buffer,
    size_t buffer_size)
{
    const MblinkMercedesDocumentedField *year_field;
    const MblinkMercedesDocumentedField *week_field;
    const MblinkMercedesDocumentedField *patch_field;
    uint8_t year;
    uint8_t week;
    uint8_t patch;
    unsigned int full_year;
    int count;

    if (record == NULL || buffer == NULL || buffer_size == 0U ||
        mblink_mercedes_documented_field_count(
            record->service, record->identifier) < 3U) {
        return false;
    }

    year_field = mblink_mercedes_documented_field_at(
        record->service, record->identifier, 0U);
    week_field = mblink_mercedes_documented_field_at(
        record->service, record->identifier, 1U);
    patch_field = mblink_mercedes_documented_field_at(
        record->service, record->identifier, 2U);

    if (!documented_field_u8(record, year_field, &year) ||
        !documented_field_u8(record, week_field, &week) ||
        !documented_field_u8(record, patch_field, &patch) ||
        week == 0U || week > 53U) {
        return false;
    }

    /*
     * Mercedes stores the year as two digits in these documented version
     * records. Use the conventional rolling-century interpretation for the
     * user-facing value. The original bytes remain available separately in
     * the diagnostic snapshot's rawHex field.
     */
    full_year = year <= 79U
        ? 2000U + (unsigned int)year
        : 1900U + (unsigned int)year;

    count = snprintf(
        buffer, buffer_size,
        "%u · calendar week %u · patch %u",
        full_year,
        (unsigned int)week,
        (unsigned int)patch);
    return count >= 0 && (size_t)count < buffer_size;
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
        name == NULL) {
        return false;
    }

    /*
     * Presentation semantics belong to the documented read definition, not to
     * a controller/UI special case. A single DID can carry several fields; the
     * catalogue supplies both their byte positions and the composite layout.
     */
    if (mblink_mercedes_documented_read_layout(
            record->service, record->identifier) ==
        MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH) {
        const char *documentedName =
            mblink_mercedes_documented_read_name(
                record->service, record->identifier);
        if (documentedName != NULL &&
            format_documented_year_week_patch(
                record, buffer, buffer_size)) {
            *name = documentedName;
            return true;
        }
    }

    if (record->service == UINT8_C(0x22) &&
        mblink_mercedes_documented_read_is_module_metadata(
            record->service, record->identifier)) {
        const char *documentedName =
            mblink_mercedes_documented_read_name(
                record->service, record->identifier);
        if (documentedName != NULL &&
            format_ascii_payload(
                record->data, record->data_length,
                buffer, buffer_size)) {
            *name = documentedName;
            return true;
        }
    }

    if (record_is_orc_kwp(
            tx_can_id, rx_can_id, extended_id, module_kind, record)) {
        return format_orc_kwp_record(record, buffer, buffer_size, name);
    }

    if (!record_is_transmission_kwp(
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
                "engine torque signed raw %d · converter torque signed raw %d · "
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
                (int)decoded.engine_torque_signed_raw,
                (int)decoded.converter_torque_signed_raw,
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
                infiltratr_copy_string(gear_text, sizeof(gear_text), "N");
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
                infiltratr_load_be16(&record->data[2]);
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
