// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file embedded_console.h
 * @brief Allocation-free engineering console for headless MBLINK targets.
 */
#ifndef MBLINK_EMBEDDED_CONSOLE_H
#define MBLINK_EMBEDDED_CONSOLE_H

#include "mblink/uds_dtc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_EMBEDDED_CONSOLE_LINE_CAPACITY 64U

typedef void (*MblinkEmbeddedConsoleWriteFn)(
    void *context,
    const char *text,
    size_t length);

typedef struct {
    const char *product_version;
    const char *link_version;
    const char *vin;
    uint32_t request_can_id;
    uint32_t response_can_id;
    bool can_online;
    uint8_t uds_session;
    uint8_t last_service;
    uint8_t last_nrc;
    uint32_t request_count;
    uint32_t positive_response_count;
    uint32_t negative_response_count;
    uint32_t suppressed_response_count;
    uint32_t completed_request_count;
    uint32_t can_rx_dropped;
    uint32_t deferred_rx_dropped;
    const MblinkUdsDtcRecord *dtcs;
    size_t dtc_count;
    bool reset_pending;
    uint8_t reset_type;
} MblinkEmbeddedConsoleSnapshot;

typedef struct {
    char line[MBLINK_EMBEDDED_CONSOLE_LINE_CAPACITY];
    size_t line_length;
    bool line_overflow;
    bool ignore_next_lf;
} MblinkEmbeddedConsole;

void mblink_embedded_console_init(MblinkEmbeddedConsole *console);
void mblink_embedded_console_print_banner(
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context);
void mblink_embedded_console_feed(
    MblinkEmbeddedConsole *console,
    uint8_t byte,
    const MblinkEmbeddedConsoleSnapshot *snapshot,
    MblinkEmbeddedConsoleWriteFn write_fn,
    void *write_context);

#ifdef __cplusplus
}
#endif

#endif
