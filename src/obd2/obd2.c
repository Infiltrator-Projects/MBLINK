// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file obd2.c
 * @brief Portable standard OBD-II request and decoding engine.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#include "mblink/obd2.h"

#include "infiltratr/core.h"

#include <stdio.h>
#include <string.h>

#define OBD2_MAX_LINE_BYTES 256U

static int obd2_hex_value(char value)
{
    if (value >= '0' && value <= '9') {
        return (int)(value - '0');
    }
    if (value >= 'A' && value <= 'F') {
        return (int)(value - 'A') + 10;
    }
    if (value >= 'a' && value <= 'f') {
        return (int)(value - 'a') + 10;
    }
    return -1;
}

static bool obd2_space(char value)
{
    return value == ' ' || value == '\t' || value == '\r';
}

static char obd2_hex_digit(unsigned int value)
{
    static const char digits[] = "0123456789ABCDEF";
    return digits[value & 0x0fU];
}

static MblinkObd2Result obd2_write_command(
    const uint8_t *bytes, size_t byte_count, char *buffer, size_t buffer_size)
{
    size_t index;
    size_t needed;

    if (bytes == NULL || byte_count == 0U || buffer == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    if (byte_count > (SIZE_MAX - 1U) / 2U) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    needed = byte_count * 2U + 1U;
    if (buffer_size < needed) {
        if (buffer_size != 0U) {
            buffer[0] = '\0';
        }
        return MBLINK_OBD2_RESULT_BUFFER_TOO_SMALL;
    }

    for (index = 0U; index < byte_count; ++index) {
        buffer[index * 2U] = obd2_hex_digit((unsigned int)bytes[index] >> 4U);
        buffer[index * 2U + 1U] = obd2_hex_digit(bytes[index]);
    }
    buffer[byte_count * 2U] = '\0';
    return MBLINK_OBD2_RESULT_OK;
}

static MblinkObd2Result obd2_response_ready(
    const MblinkElm327Response *response)
{
    if (response == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    if (response->result != MBLINK_ELM327_RESULT_OK) {
        return MBLINK_OBD2_RESULT_ELM_ERROR;
    }
    if (response->text[0] == '\0') {
        return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
    }
    return MBLINK_OBD2_RESULT_OK;
}

static MblinkObd2Result obd2_parse_hex_line(
    const char *line, size_t line_length,
    uint8_t *bytes, size_t bytes_size, size_t *byte_count)
{
    size_t index;
    size_t count = 0U;
    int high = -1;

    if (line == NULL || bytes == NULL || byte_count == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    *byte_count = 0U;

    for (index = 0U; index < line_length; ++index) {
        int value;
        char current = line[index];

        if (obd2_space(current)) {
            continue;
        }
        value = obd2_hex_value(current);
        if (value < 0) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        if (high < 0) {
            high = value;
            continue;
        }
        if (count >= bytes_size) {
            return MBLINK_OBD2_RESULT_BUFFER_TOO_SMALL;
        }
        bytes[count++] = (uint8_t)(((unsigned int)high << 4U) |
                                   (unsigned int)value);
        high = -1;
    }

    if (high >= 0 || count == 0U) {
        return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
    }
    *byte_count = count;
    return MBLINK_OBD2_RESULT_OK;
}

static MblinkObd2Result obd2_find_pid_payload(
    const MblinkElm327Response *response,
    uint8_t response_service,
    uint8_t pid,
    bool has_frame_number,
    uint8_t frame_number,
    uint8_t *payload,
    size_t payload_size,
    size_t *payload_length)
{
    const char *cursor;
    MblinkObd2Result result;

    if (payload == NULL || payload_length == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    *payload_length = 0U;

    result = obd2_response_ready(response);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }

    cursor = response->text;
    while (*cursor != '\0') {
        const char *end = strchr(cursor, '\n');
        size_t line_length = end == NULL ? strlen(cursor)
                                         : (size_t)(end - cursor);
        uint8_t bytes[OBD2_MAX_LINE_BYTES];
        size_t byte_count = 0U;
        size_t data_index;
        size_t data_length;

        result = obd2_parse_hex_line(cursor, line_length, bytes,
                                     sizeof(bytes), &byte_count);
        if (result != MBLINK_OBD2_RESULT_OK) {
            return result;
        }

        if (byte_count >= 2U && bytes[0] == response_service &&
            bytes[1] == pid) {
            data_index = 2U;
            if (has_frame_number) {
                if (byte_count < 3U || bytes[2] != frame_number) {
                    goto next_line;
                }
                data_index = 3U;
            }
            data_length = byte_count - data_index;
            if (data_length > payload_size) {
                return MBLINK_OBD2_RESULT_BUFFER_TOO_SMALL;
            }
            if (data_length != 0U) {
                memcpy(payload, bytes + data_index, data_length);
            }
            *payload_length = data_length;
            return MBLINK_OBD2_RESULT_OK;
        }

next_line:
        if (end == NULL) {
            break;
        }
        cursor = end + 1;
    }

    return MBLINK_OBD2_RESULT_UNEXPECTED_RESPONSE;
}

static MblinkObd2Result obd2_decode_sample_data(
    uint8_t pid, const uint8_t *data, size_t length, MblinkObd2Sample *sample)
{
    if (data == NULL || sample == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }

    sample->pid = pid;
    sample->value = 0.0;
    sample->unit = MBLINK_OBD2_UNIT_NONE;

    switch (pid) {
    case 0x04U:
        if (length < 1U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value = (double)data[0] * 100.0 / 255.0;
        sample->unit = MBLINK_OBD2_UNIT_PERCENT;
        return MBLINK_OBD2_RESULT_OK;
    case 0x05U:
        if (length < 1U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value = (double)data[0] - 40.0;
        sample->unit = MBLINK_OBD2_UNIT_CELSIUS;
        return MBLINK_OBD2_RESULT_OK;
    case 0x0bU:
        if (length < 1U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value = (double)data[0];
        sample->unit = MBLINK_OBD2_UNIT_KPA;
        return MBLINK_OBD2_RESULT_OK;
    case 0x0cU:
        if (length < 2U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value =
            (double)(((unsigned int)data[0] << 8U) | data[1]) / 4.0;
        sample->unit = MBLINK_OBD2_UNIT_RPM;
        return MBLINK_OBD2_RESULT_OK;
    case 0x0dU:
        if (length < 1U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value = (double)data[0];
        sample->unit = MBLINK_OBD2_UNIT_KMH;
        return MBLINK_OBD2_RESULT_OK;
    case 0x0fU:
        if (length < 1U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value = (double)data[0] - 40.0;
        sample->unit = MBLINK_OBD2_UNIT_CELSIUS;
        return MBLINK_OBD2_RESULT_OK;
    case 0x10U:
        if (length < 2U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value =
            (double)(((unsigned int)data[0] << 8U) | data[1]) / 100.0;
        sample->unit = MBLINK_OBD2_UNIT_GRAMS_PER_SECOND;
        return MBLINK_OBD2_RESULT_OK;
    case 0x11U:
        if (length < 1U) {
            return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
        }
        sample->value = (double)data[0] * 100.0 / 255.0;
        sample->unit = MBLINK_OBD2_UNIT_PERCENT;
        return MBLINK_OBD2_RESULT_OK;
    default:
        return MBLINK_OBD2_RESULT_UNSUPPORTED_PID;
    }
}

static uint8_t obd2_dtc_response_service(MblinkObd2DtcKind kind)
{
    switch (kind) {
    case MBLINK_OBD2_DTC_STORED:
        return 0x43U;
    case MBLINK_OBD2_DTC_PENDING:
        return 0x47U;
    case MBLINK_OBD2_DTC_PERMANENT:
        return 0x4aU;
    }
    return 0U;
}

static uint8_t obd2_dtc_request_service(MblinkObd2DtcKind kind)
{
    switch (kind) {
    case MBLINK_OBD2_DTC_STORED:
        return 0x03U;
    case MBLINK_OBD2_DTC_PENDING:
        return 0x07U;
    case MBLINK_OBD2_DTC_PERMANENT:
        return 0x0aU;
    }
    return 0U;
}

static bool obd2_dtc_exists(const MblinkObd2DtcList *list, const char *code)
{
    size_t index;

    if (list == NULL || code == NULL) {
        return false;
    }
    for (index = 0U; index < list->count; ++index) {
        if (infiltratr_string_equal(list->entries[index].code, code)) {
            return true;
        }
    }
    return false;
}

const char *mblink_obd2_result_name(MblinkObd2Result result)
{
    switch (result) {
    case MBLINK_OBD2_RESULT_OK:
        return "ok";
    case MBLINK_OBD2_RESULT_INVALID_ARGUMENT:
        return "invalid-argument";
    case MBLINK_OBD2_RESULT_ELM_ERROR:
        return "elm-error";
    case MBLINK_OBD2_RESULT_MALFORMED_RESPONSE:
        return "malformed-response";
    case MBLINK_OBD2_RESULT_UNEXPECTED_RESPONSE:
        return "unexpected-response";
    case MBLINK_OBD2_RESULT_UNSUPPORTED_PID:
        return "unsupported-pid";
    case MBLINK_OBD2_RESULT_BUFFER_TOO_SMALL:
        return "buffer-too-small";
    case MBLINK_OBD2_RESULT_TOO_MANY_DTCS:
        return "too-many-dtcs";
    case MBLINK_OBD2_RESULT_NOT_AUTHORIZED:
        return "not-authorized";
    }
    return "unknown";
}

const char *mblink_obd2_unit_name(MblinkObd2Unit unit)
{
    switch (unit) {
    case MBLINK_OBD2_UNIT_NONE:
        return "";
    case MBLINK_OBD2_UNIT_PERCENT:
        return "%";
    case MBLINK_OBD2_UNIT_CELSIUS:
        return "degC";
    case MBLINK_OBD2_UNIT_KPA:
        return "kPa";
    case MBLINK_OBD2_UNIT_RPM:
        return "rpm";
    case MBLINK_OBD2_UNIT_KMH:
        return "km/h";
    case MBLINK_OBD2_UNIT_GRAMS_PER_SECOND:
        return "g/s";
    }
    return "";
}

const char *mblink_obd2_pid_name(uint8_t pid)
{
    switch (pid) {
    case 0x04U:
        return "Calculated engine load";
    case 0x05U:
        return "Engine coolant temperature";
    case 0x0bU:
        return "Intake manifold absolute pressure";
    case 0x0cU:
        return "Engine speed";
    case 0x0dU:
        return "Vehicle speed";
    case 0x0fU:
        return "Intake air temperature";
    case 0x10U:
        return "Mass air flow rate";
    case 0x11U:
        return "Throttle position";
    default:
        return "Unknown PID";
    }
}

MblinkObd2Result mblink_obd2_build_live_pid_request(
    uint8_t pid, char *buffer, size_t buffer_size)
{
    const uint8_t bytes[] = {0x01U, pid};
    return obd2_write_command(bytes, sizeof(bytes), buffer, buffer_size);
}

MblinkObd2Result mblink_obd2_build_freeze_pid_request(
    uint8_t pid, uint8_t frame_number, char *buffer, size_t buffer_size)
{
    const uint8_t bytes[] = {0x02U, pid, frame_number};
    return obd2_write_command(bytes, sizeof(bytes), buffer, buffer_size);
}

MblinkObd2Result mblink_obd2_build_supported_pid_request(
    uint8_t base_pid, char *buffer, size_t buffer_size)
{
    if ((base_pid & 0x1fU) != 0U || base_pid > 0xe0U) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    return mblink_obd2_build_live_pid_request(base_pid, buffer, buffer_size);
}

MblinkObd2Result mblink_obd2_build_vin_request(
    char *buffer, size_t buffer_size)
{
    const uint8_t bytes[] = {0x09U, 0x02U};
    return obd2_write_command(bytes, sizeof(bytes), buffer, buffer_size);
}

MblinkObd2Result mblink_obd2_build_dtc_request(
    MblinkObd2DtcKind kind, char *buffer, size_t buffer_size)
{
    uint8_t service = obd2_dtc_request_service(kind);

    if (service == 0U) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    return obd2_write_command(&service, 1U, buffer, buffer_size);
}

MblinkObd2Result mblink_obd2_build_clear_dtc_request(
    const MblinkObd2ClearAuthorization *authorization,
    char *buffer, size_t buffer_size)
{
    const uint8_t service = 0x04U;

    if (authorization == NULL || !authorization->confirmed ||
        !authorization->acknowledge_readiness_reset) {
        if (buffer != NULL && buffer_size != 0U) {
            buffer[0] = '\0';
        }
        return MBLINK_OBD2_RESULT_NOT_AUTHORIZED;
    }
    return obd2_write_command(&service, 1U, buffer, buffer_size);
}

void mblink_obd2_pid_set_clear(MblinkObd2PidSet *set)
{
    if (set != NULL) {
        memset(set, 0, sizeof(*set));
    }
}

bool mblink_obd2_pid_set_contains(const MblinkObd2PidSet *set, uint8_t pid)
{
    uint8_t mask;

    if (set == NULL) {
        return false;
    }
    mask = (uint8_t)(1U << (pid & 7U));
    return (set->bits[pid >> 3U] & mask) != 0U;
}

MblinkObd2Result mblink_obd2_accept_supported_pids(
    const MblinkElm327Response *response,
    uint8_t base_pid,
    MblinkObd2PidSet *set,
    bool *has_more)
{
    const char *cursor;
    bool matched = false;
    MblinkObd2Result result;

    if (set == NULL || has_more == NULL ||
        (base_pid & 0x1fU) != 0U || base_pid > 0xe0U) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    *has_more = false;

    result = obd2_response_ready(response);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }

    cursor = response->text;
    while (*cursor != '\0') {
        const char *end = strchr(cursor, '\n');
        size_t line_length = end == NULL ? strlen(cursor)
                                         : (size_t)(end - cursor);
        uint8_t bytes[OBD2_MAX_LINE_BYTES];
        size_t byte_count = 0U;
        uint32_t mask;
        unsigned int bit;

        result = obd2_parse_hex_line(cursor, line_length, bytes,
                                     sizeof(bytes), &byte_count);
        if (result != MBLINK_OBD2_RESULT_OK) {
            return result;
        }
        if (byte_count >= 6U && bytes[0] == 0x41U && bytes[1] == base_pid) {
            matched = true;
            mask = ((uint32_t)bytes[2] << 24U) |
                   ((uint32_t)bytes[3] << 16U) |
                   ((uint32_t)bytes[4] << 8U) |
                   (uint32_t)bytes[5];

            for (bit = 0U; bit < 32U; ++bit) {
                if ((mask & (UINT32_C(1) << (31U - bit))) != 0U) {
                    unsigned int pid = (unsigned int)base_pid + bit + 1U;
                    if (pid <= 0xffU) {
                        set->bits[pid >> 3U] |=
                            (uint8_t)(1U << (pid & 7U));
                    }
                }
            }
        }

        if (end == NULL) {
            break;
        }
        cursor = end + 1;
    }

    if (!matched) {
        return MBLINK_OBD2_RESULT_UNEXPECTED_RESPONSE;
    }
    if (base_pid <= 0xc0U) {
        *has_more = mblink_obd2_pid_set_contains(
            set, (uint8_t)(base_pid + 0x20U));
    }
    return MBLINK_OBD2_RESULT_OK;
}

MblinkObd2Result mblink_obd2_decode_live_pid(
    const MblinkElm327Response *response,
    uint8_t pid,
    MblinkObd2Sample *sample)
{
    uint8_t data[16];
    size_t length = 0U;
    MblinkObd2Result result;

    if (sample == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    result = obd2_find_pid_payload(response, 0x41U, pid, false, 0U,
                                   data, sizeof(data), &length);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }
    return obd2_decode_sample_data(pid, data, length, sample);
}

MblinkObd2Result mblink_obd2_decode_freeze_pid(
    const MblinkElm327Response *response,
    uint8_t pid,
    uint8_t frame_number,
    MblinkObd2Sample *sample)
{
    uint8_t data[16];
    size_t length = 0U;
    MblinkObd2Result result;

    if (sample == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    result = obd2_find_pid_payload(response, 0x42U, pid, true, frame_number,
                                   data, sizeof(data), &length);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }
    return obd2_decode_sample_data(pid, data, length, sample);
}

MblinkObd2Result mblink_obd2_decode_readiness(
    const MblinkElm327Response *response,
    MblinkObd2Readiness *readiness)
{
    uint8_t data[8];
    size_t length = 0U;
    MblinkObd2Result result;

    if (readiness == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    result = obd2_find_pid_payload(response, 0x41U, 0x01U, false, 0U,
                                   data, sizeof(data), &length);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }
    if (length < 4U) {
        return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
    }

    memset(readiness, 0, sizeof(*readiness));
    memcpy(readiness->raw, data, sizeof(readiness->raw));
    readiness->mil_on = (data[0] & 0x80U) != 0U;
    readiness->confirmed_dtc_count = data[0] & 0x7fU;
    readiness->compression_ignition = (data[1] & 0x08U) != 0U;
    readiness->continuous_supported = data[1] & 0x07U;
    readiness->continuous_incomplete = (data[1] >> 4U) & 0x07U;
    readiness->noncontinuous_supported = data[2];
    readiness->noncontinuous_incomplete = data[3];
    return MBLINK_OBD2_RESULT_OK;
}

MblinkObd2Result mblink_obd2_decode_vin(
    const MblinkElm327Response *response,
    char vin[MBLINK_OBD2_VIN_LENGTH + 1U])
{
    const char *cursor;
    size_t written = 0U;
    MblinkObd2Result result;

    if (vin == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    vin[0] = '\0';

    result = obd2_response_ready(response);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }

    cursor = response->text;
    while (*cursor != '\0' && written < MBLINK_OBD2_VIN_LENGTH) {
        const char *end = strchr(cursor, '\n');
        size_t line_length = end == NULL ? strlen(cursor)
                                         : (size_t)(end - cursor);
        uint8_t bytes[OBD2_MAX_LINE_BYTES];
        size_t byte_count = 0U;
        size_t index;

        result = obd2_parse_hex_line(cursor, line_length, bytes,
                                     sizeof(bytes), &byte_count);
        if (result != MBLINK_OBD2_RESULT_OK) {
            return result;
        }

        if (byte_count >= 4U && bytes[0] == 0x49U && bytes[1] == 0x02U) {
            for (index = 3U; index < byte_count &&
                             written < MBLINK_OBD2_VIN_LENGTH; ++index) {
                if (bytes[index] < 0x20U || bytes[index] > 0x7eU) {
                    return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
                }
                vin[written++] = (char)bytes[index];
            }
        }

        if (end == NULL) {
            break;
        }
        cursor = end + 1;
    }

    if (written != MBLINK_OBD2_VIN_LENGTH) {
        vin[0] = '\0';
        return MBLINK_OBD2_RESULT_UNEXPECTED_RESPONSE;
    }
    vin[written] = '\0';
    return MBLINK_OBD2_RESULT_OK;
}

MblinkObd2Result mblink_obd2_decode_dtc_pair(
    uint8_t high,
    uint8_t low,
    char code[MBLINK_OBD2_DTC_TEXT_LENGTH])
{
    static const char system_chars[] = {'P', 'C', 'B', 'U'};

    if (code == NULL) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }

    code[0] = system_chars[(high >> 6U) & 0x03U];
    code[1] = obd2_hex_digit((high >> 4U) & 0x03U);
    code[2] = obd2_hex_digit(high);
    code[3] = obd2_hex_digit(low >> 4U);
    code[4] = obd2_hex_digit(low);
    code[5] = '\0';
    return MBLINK_OBD2_RESULT_OK;
}

MblinkObd2Result mblink_obd2_decode_dtcs(
    const MblinkElm327Response *response,
    MblinkObd2DtcKind kind,
    MblinkObd2DtcList *list)
{
    const char *cursor;
    uint8_t response_service = obd2_dtc_response_service(kind);
    bool matched = false;
    MblinkObd2Result result;

    if (list == NULL || response_service == 0U) {
        return MBLINK_OBD2_RESULT_INVALID_ARGUMENT;
    }
    memset(list, 0, sizeof(*list));

    result = obd2_response_ready(response);
    if (result != MBLINK_OBD2_RESULT_OK) {
        return result;
    }

    cursor = response->text;
    while (*cursor != '\0') {
        const char *end = strchr(cursor, '\n');
        size_t line_length = end == NULL ? strlen(cursor)
                                         : (size_t)(end - cursor);
        uint8_t bytes[OBD2_MAX_LINE_BYTES];
        size_t byte_count = 0U;
        size_t index;

        result = obd2_parse_hex_line(cursor, line_length, bytes,
                                     sizeof(bytes), &byte_count);
        if (result != MBLINK_OBD2_RESULT_OK) {
            return result;
        }

        if (byte_count >= 1U && bytes[0] == response_service) {
            matched = true;
            if (((byte_count - 1U) & 1U) != 0U) {
                return MBLINK_OBD2_RESULT_MALFORMED_RESPONSE;
            }
            for (index = 1U; index + 1U < byte_count; index += 2U) {
                char code[MBLINK_OBD2_DTC_TEXT_LENGTH];

                if (bytes[index] == 0U && bytes[index + 1U] == 0U) {
                    continue;
                }
                result = mblink_obd2_decode_dtc_pair(
                    bytes[index], bytes[index + 1U], code);
                if (result != MBLINK_OBD2_RESULT_OK) {
                    return result;
                }
                if (obd2_dtc_exists(list, code)) {
                    continue;
                }
                if (list->count >= MBLINK_OBD2_MAX_DTCS) {
                    return MBLINK_OBD2_RESULT_TOO_MANY_DTCS;
                }
                list->entries[list->count].kind = kind;
                infiltratr_copy_string(list->entries[list->count].code,
                                       sizeof(list->entries[list->count].code),
                                       code);
                list->count++;
            }
        }

        if (end == NULL) {
            break;
        }
        cursor = end + 1;
    }

    return matched ? MBLINK_OBD2_RESULT_OK
                   : MBLINK_OBD2_RESULT_UNEXPECTED_RESPONSE;
}
