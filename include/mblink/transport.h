// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file transport.h
 * @brief MBLINK compatibility facade for LINK's byte-stream transport ABI.
 *
 * LINK owns the transport contract.  MBLINK retains its historical public type
 * and function names so existing Mercedes-specific callers do not need to
 * change simply because the implementation moved down the dependency pyramid.
 */
#ifndef MBLINK_TRANSPORT_H
#define MBLINK_TRANSPORT_H

#include "link/transport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_TRANSPORT_ABI LINK_TRANSPORT_ABI

#define MBLINK_TRANSPORT_OK LINK_TRANSPORT_OK
#define MBLINK_TRANSPORT_NOT_CONNECTED LINK_TRANSPORT_NOT_CONNECTED
#define MBLINK_TRANSPORT_BUSY LINK_TRANSPORT_BUSY
#define MBLINK_TRANSPORT_TIMEOUT LINK_TRANSPORT_TIMEOUT
#define MBLINK_TRANSPORT_IO_ERROR LINK_TRANSPORT_IO_ERROR
#define MBLINK_TRANSPORT_UNSUPPORTED LINK_TRANSPORT_UNSUPPORTED
#define MBLINK_TRANSPORT_INVALID_ARGUMENT LINK_TRANSPORT_INVALID_ARGUMENT

typedef LinkTransportStatus MblinkTransportStatus;
typedef LinkTransportReceiveFn MblinkTransportReceiveFn;
typedef LinkTransport MblinkTransport;

#define MBLINK_TRANSPORT_INIT LINK_TRANSPORT_INIT

/** Forward ABI validation to the single implementation owned by LINK. */
bool mblink_transport_is_valid(const MblinkTransport *transport);

#ifdef __cplusplus
}
#endif

#endif
