// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file transport.h
 * @brief Platform-neutral transport boundary for libmblink.
 *
 * Platform providers implement this interface without exposing native
 * framework types to the portable diagnostics core.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#ifndef MBLINK_TRANSPORT_H
#define MBLINK_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_TRANSPORT_ABI 1U

typedef enum {
    MBLINK_TRANSPORT_OK = 0,
    MBLINK_TRANSPORT_NOT_CONNECTED,
    MBLINK_TRANSPORT_BUSY,
    MBLINK_TRANSPORT_TIMEOUT,
    MBLINK_TRANSPORT_IO_ERROR,
    MBLINK_TRANSPORT_UNSUPPORTED,
    MBLINK_TRANSPORT_INVALID_ARGUMENT
} MblinkTransportStatus;

typedef void (*MblinkTransportReceiveFn)(void *context,
                                         const uint8_t *data,
                                         size_t size);

typedef struct {
    size_t struct_size;
    uint32_t abi_version;
    void *context;
    MblinkTransportStatus (*connect)(void *context);
    void (*disconnect)(void *context);
    bool (*is_connected)(void *context);
    MblinkTransportStatus (*write)(void *context,
                                   const uint8_t *data,
                                   size_t size);
    void (*set_receiver)(void *context,
                         MblinkTransportReceiveFn receiver,
                         void *receiver_context);
} MblinkTransport;

#define MBLINK_TRANSPORT_INIT \
    { .struct_size = sizeof(MblinkTransport), \
      .abi_version = MBLINK_TRANSPORT_ABI, \
      .context = NULL, .connect = NULL, .disconnect = NULL, \
      .is_connected = NULL, .write = NULL, .set_receiver = NULL }

/** Validate ABI metadata and all mandatory operations. */
bool mblink_transport_is_valid(const MblinkTransport *transport);

#ifdef __cplusplus
}
#endif

#endif
