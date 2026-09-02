// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/embedded_console.h"
#include "mblink/version.h"
#include "link/version.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

typedef struct {
    char output[8192U];
    size_t length;
} ConsoleSink;

static void sink_write(void *context, const char *text, size_t length)
{
    ConsoleSink *sink = (ConsoleSink *)context;
    size_t available;
    if (sink == NULL || text == NULL || sink->length >= sizeof(sink->output) - 1U)
        return;
    available = sizeof(sink->output) - sink->length - 1U;
    if (length > available) length = available;
    memcpy(sink->output + sink->length, text, length);
    sink->length += length;
    sink->output[sink->length] = '\0';
}

static void send_text(
    MblinkEmbeddedConsole *console,
    const char *text,
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    ConsoleSink *sink)
{
    while (text != NULL && *text != '\0') {
        mblink_embedded_console_feed(
            console, (uint8_t)*text, snapshot, sink_write, sink);
        ++text;
    }
}

static int test_console_commands(void)
{
    static const MblinkUdsDtcRecord dtcs[] = {
        { UINT32_C(0x123456), UINT8_C(0x09) },
        { UINT32_C(0xabcdef), UINT8_C(0x08) }
    };
    MblinkEmbeddedConsoleSnapshot snapshot = {
        MBLINK_VERSION_STRING, LINK_VERSION_STRING, "WDD2073031A000001",
        UINT32_C(0x7e0), UINT32_C(0x7e8), true,
        UINT8_C(0x01), UINT8_C(0x19), 0U,
        12U, 11U, 1U, 0U, 12U, 0U, 0U,
        dtcs, 2U, false, 0U
    };
    MblinkEmbeddedConsole console;
    ConsoleSink sink = {{0}, 0U};

    mblink_embedded_console_init(&console);
    mblink_embedded_console_print_banner(&snapshot, sink_write, &sink);
    CHECK(strstr(sink.output, "MBLINK STM32C092 v") != NULL);
    CHECK(strstr(sink.output, MBLINK_VERSION_STRING) != NULL);
    CHECK(strstr(sink.output, LINK_VERSION_STRING) != NULL);

    send_text(&console, "status\r\n", &snapshot, &sink);
    CHECK(strstr(
        sink.output, "CAN: ONLINE request=0x7E0 response=0x7E8") != NULL);
    CHECK(strstr(sink.output, "UDS session=0x01 reset=none") != NULL);

    send_text(&console, "DTC\n", &snapshot, &sink);
    CHECK(strstr(sink.output, "123456 status=0x09") != NULL);
    CHECK(strstr(sink.output, "ABCDEF status=0x08") != NULL);

    send_text(&console, "stats\n", &snapshot, &sink);
    CHECK(strstr(
        sink.output,
        "UDS requests=12 positive=11 negative=1 suppressed=0") != NULL);
    CHECK(strstr(
        sink.output,
        "Transport completed=12 CAN-dropped=0 deferred-dropped=0") != NULL);

    send_text(&console, "last\n", &snapshot, &sink);
    CHECK(strstr(sink.output, "Last service=0x19 NRC=0x00") != NULL);

    send_text(&console, "vin\n", &snapshot, &sink);
    CHECK(strstr(sink.output, "VIN: WDD2073031A000001") != NULL);

    send_text(&console, "wat\n", &snapshot, &sink);
    CHECK(strstr(sink.output, "Unknown command. Type help.") != NULL);
    return 0;
}

int main(void)
{
    if (test_console_commands() != 0) return 1;
    puts("mblink embedded console tests passed");
    return 0;
}
