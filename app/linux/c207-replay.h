// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_C207_REPLAY_H
#define MBLINK_C207_REPLAY_H

#include "link-gtk-shell.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_C207_REPLAY_DEVICE     "REPLAY: C207 / Vgate iCar Pro capture + injected ORC fault"

typedef struct MblinkC207ReplayTransport {
    LinkTransportReceiveFn receiver;
    void *receiver_context;
    bool connected;
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool extended_id;
    bool headers_enabled;
    char pending[1024];
    size_t pending_length;
    unsigned long command_count;
} MblinkC207ReplayTransport;

void mblink_c207_replay_init(MblinkC207ReplayTransport *replay);
const LinkGtkTransportProvider *mblink_c207_replay_provider(void);

#ifdef __cplusplus
}
#endif

#endif
