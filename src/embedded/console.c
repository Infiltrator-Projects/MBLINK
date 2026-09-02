// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/embedded_console.h"

#include <string.h>

typedef struct {
    char bytes[192U];
    size_t length;
} MblinkConsoleLine;

static void console_write(
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context,
    const char *text)
{
    if (write_fn == NULL || text == NULL) return;
    write_fn(write_context, text, strlen(text));
}

static void line_reset(MblinkConsoleLine *line)
{
    if (line == NULL) return;
    line->length = 0U;
    line->bytes[0] = '\0';
}

static bool line_put_char(MblinkConsoleLine *line, char value)
{
    if (line == NULL || line->length + 1U >= sizeof(line->bytes)) return false;
    line->bytes[line->length++] = value;
    line->bytes[line->length] = '\0';
    return true;
}

static bool line_put_text(MblinkConsoleLine *line, const char *text)
{
    size_t index;
    if (line == NULL || text == NULL) return false;
    for (index = 0U; text[index] != '\0'; ++index) {
        if (!line_put_char(line, text[index])) return false;
    }
    return true;
}

static bool line_put_hex_nibble(MblinkConsoleLine *line, uint8_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    return line_put_char(line, digits[value & UINT8_C(0x0f)]);
}

static bool line_put_hex8(MblinkConsoleLine *line, uint8_t value)
{
    return line_put_hex_nibble(line, (uint8_t)(value >> 4U)) &&
           line_put_hex_nibble(line, value);
}

static bool line_put_hex24(MblinkConsoleLine *line, uint32_t value)
{
    return line_put_hex8(line, (uint8_t)(value >> 16U)) &&
           line_put_hex8(line, (uint8_t)(value >> 8U)) &&
           line_put_hex8(line, (uint8_t)value);
}

static bool line_put_hex32(MblinkConsoleLine *line, uint32_t value)
{
    return line_put_hex8(line, (uint8_t)(value >> 24U)) &&
           line_put_hex8(line, (uint8_t)(value >> 16U)) &&
           line_put_hex8(line, (uint8_t)(value >> 8U)) &&
           line_put_hex8(line, (uint8_t)value);
}

static bool line_put_can_id(MblinkConsoleLine *line, uint32_t value)
{
    if (!line_put_text(line, "0x")) return false;
    if (value <= UINT32_C(0x7ff)) {
        return line_put_hex_nibble(line, (uint8_t)(value >> 8U)) &&
               line_put_hex8(line, (uint8_t)value);
    }
    return line_put_hex32(line, value);
}

static bool line_put_u32(MblinkConsoleLine *line, uint32_t value)
{
    char reversed[10U];
    size_t count = 0U;
    if (value == 0U) return line_put_char(line, '0');
    while (value != 0U && count < sizeof(reversed)) {
        reversed[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    while (count != 0U) {
        --count;
        if (!line_put_char(line, reversed[count])) return false;
    }
    return true;
}

static void emit_line(
    MblinkConsoleLine *line,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    if (line == NULL || write_fn == NULL) return;
    if (line_put_text(line, "\r\n")) {
        write_fn(write_context, line->bytes, line->length);
    }
}

static char ascii_lower(char value)
{
    if (value >= 'A' && value <= 'Z') return (char)(value - 'A' + 'a');
    return value;
}

static bool command_equals(const char *line, size_t length, const char *command)
{
    size_t start = 0U;
    size_t end = length;
    size_t index;
    size_t command_length;

    if (line == NULL || command == NULL) return false;
    while (start < end && (line[start] == ' ' || line[start] == '\t')) ++start;
    while (end > start && (line[end - 1U] == ' ' || line[end - 1U] == '\t')) --end;
    command_length = strlen(command);
    if ((end - start) != command_length) return false;
    for (index = 0U; index < command_length; ++index) {
        if (ascii_lower(line[start + index]) != ascii_lower(command[index]))
            return false;
    }
    return true;
}

static void emit_version(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    line_reset(&line);
    (void)line_put_text(&line, "MBLINK STM32C092");
    if (snapshot != NULL && snapshot->product_version != NULL) {
        (void)line_put_text(&line, " v");
        (void)line_put_text(&line, snapshot->product_version);
    }
    if (snapshot != NULL && snapshot->link_version != NULL) {
        (void)line_put_text(&line, " / LINK v");
        (void)line_put_text(&line, snapshot->link_version);
    }
    emit_line(&line, write_fn, write_context);
}

static void emit_status(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    if (snapshot == NULL) return;

    line_reset(&line);
    (void)line_put_text(&line, "CAN: ");
    (void)line_put_text(&line, snapshot->can_online ? "ONLINE" : "OFFLINE");
    (void)line_put_text(&line, " request=");
    (void)line_put_can_id(&line, snapshot->request_can_id);
    (void)line_put_text(&line, " response=");
    (void)line_put_can_id(&line, snapshot->response_can_id);
    emit_line(&line, write_fn, write_context);

    line_reset(&line);
    (void)line_put_text(&line, "UDS session=0x");
    (void)line_put_hex8(&line, snapshot->uds_session);
    (void)line_put_text(&line, " reset=");
    if (snapshot->reset_pending) {
        (void)line_put_text(&line, "pending type=0x");
        (void)line_put_hex8(&line, snapshot->reset_type);
    } else {
        (void)line_put_text(&line, "none");
    }
    emit_line(&line, write_fn, write_context);
}

static void emit_vin(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    line_reset(&line);
    (void)line_put_text(&line, "VIN: ");
    if (snapshot != NULL && snapshot->vin != NULL)
        (void)line_put_text(&line, snapshot->vin);
    else
        (void)line_put_text(&line, "unavailable");
    emit_line(&line, write_fn, write_context);
}

static void emit_dtcs(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    size_t index;
    if (snapshot == NULL || snapshot->dtcs == NULL || snapshot->dtc_count == 0U) {
        console_write(write_fn, write_context, "No DTC records\r\n");
        return;
    }
    for (index = 0U; index < snapshot->dtc_count; ++index) {
        line_reset(&line);
        (void)line_put_hex24(&line, snapshot->dtcs[index].code);
        (void)line_put_text(&line, " status=0x");
        (void)line_put_hex8(&line, snapshot->dtcs[index].status);
        emit_line(&line, write_fn, write_context);
    }
}

static void emit_stats(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    if (snapshot == NULL) return;

    line_reset(&line);
    (void)line_put_text(&line, "UDS requests=");
    (void)line_put_u32(&line, snapshot->request_count);
    (void)line_put_text(&line, " positive=");
    (void)line_put_u32(&line, snapshot->positive_response_count);
    (void)line_put_text(&line, " negative=");
    (void)line_put_u32(&line, snapshot->negative_response_count);
    (void)line_put_text(&line, " suppressed=");
    (void)line_put_u32(&line, snapshot->suppressed_response_count);
    emit_line(&line, write_fn, write_context);

    line_reset(&line);
    (void)line_put_text(&line, "Transport completed=");
    (void)line_put_u32(&line, snapshot->completed_request_count);
    (void)line_put_text(&line, " CAN-dropped=");
    (void)line_put_u32(&line, snapshot->can_rx_dropped);
    (void)line_put_text(&line, " deferred-dropped=");
    (void)line_put_u32(&line, snapshot->deferred_rx_dropped);
    emit_line(&line, write_fn, write_context);
}

static void emit_last(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    if (snapshot == NULL) return;
    line_reset(&line);
    (void)line_put_text(&line, "Last service=0x");
    (void)line_put_hex8(&line, snapshot->last_service);
    (void)line_put_text(&line, " NRC=0x");
    (void)line_put_hex8(&line, snapshot->last_nrc);
    emit_line(&line, write_fn, write_context);
}

static void emit_reset(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    MblinkConsoleLine line;
    if (snapshot == NULL) return;
    line_reset(&line);
    (void)line_put_text(&line, "Reset: ");
    if (snapshot->reset_pending) {
        (void)line_put_text(&line, "pending type=0x");
        (void)line_put_hex8(&line, snapshot->reset_type);
    } else {
        (void)line_put_text(&line, "none pending");
    }
    emit_line(&line, write_fn, write_context);
}

static void emit_help(
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    console_write(write_fn, write_context,
        "Commands: help status version vin dtc stats last reset\r\n");
}

static void execute_line(
    MblinkEmbeddedConsole *console,
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    if (console == NULL) return;
    if (console->line_overflow) {
        console_write(write_fn, write_context, "ERROR line too long\r\n");
    } else if (console->line_length == 0U) {
        /* Empty line only redraws the prompt. */
    } else if (command_equals(console->line, console->line_length, "help")) {
        emit_help(write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "status")) {
        emit_status(snapshot, write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "version")) {
        emit_version(snapshot, write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "vin")) {
        emit_vin(snapshot, write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "dtc")) {
        emit_dtcs(snapshot, write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "stats")) {
        emit_stats(snapshot, write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "last")) {
        emit_last(snapshot, write_fn, write_context);
    } else if (command_equals(console->line, console->line_length, "reset")) {
        emit_reset(snapshot, write_fn, write_context);
    } else {
        console_write(write_fn, write_context, "Unknown command. Type help.\r\n");
    }
    console->line_length = 0U;
    console->line_overflow = false;
    console->line[0] = '\0';
    console_write(write_fn, write_context, "MBLINK> ");
}

void mblink_embedded_console_init(MblinkEmbeddedConsole *console)
{
    if (console == NULL) return;
    memset(console, 0, sizeof(*console));
}

void mblink_embedded_console_print_banner(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    console_write(write_fn, write_context, "\r\n");
    emit_version(snapshot, write_fn, write_context);
    console_write(write_fn, write_context,
        "Engineering console on USART2 115200 8-N-1\r\n"
        "Type help for commands. CAN/UDS continues headless.\r\n"
        "MBLINK> ");
}

void mblink_embedded_console_feed(
    MblinkEmbeddedConsole *console,
    uint8_t byte,
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context)
{
    char echo[1U];
    if (console == NULL || write_fn == NULL) return;

    if (byte == '\r') {
        console_write(write_fn, write_context, "\r\n");
        execute_line(console, snapshot, write_fn, write_context);
        console->ignore_next_lf = true;
        return;
    }
    if (byte == '\n') {
        if (console->ignore_next_lf) {
            console->ignore_next_lf = false;
            return;
        }
        console_write(write_fn, write_context, "\r\n");
        execute_line(console, snapshot, write_fn, write_context);
        return;
    }
    console->ignore_next_lf = false;

    if (byte == UINT8_C(0x08) || byte == UINT8_C(0x7f)) {
        if (console->line_length != 0U && !console->line_overflow) {
            --console->line_length;
            console->line[console->line_length] = '\0';
            console_write(write_fn, write_context, "\b \b");
        }
        return;
    }
    if (byte < UINT8_C(0x20) || byte > UINT8_C(0x7e)) return;

    echo[0] = (char)byte;
    write_fn(write_context, echo, sizeof(echo));
    if (console->line_overflow) return;
    if (console->line_length + 1U >= sizeof(console->line)) {
        console->line_overflow = true;
        return;
    }
    console->line[console->line_length++] = (char)byte;
    console->line[console->line_length] = '\0';
}
