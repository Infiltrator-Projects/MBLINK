// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_connection_lifecycle.h"

#include <assert.h>

int main(void)
{
    MblinkMercedesConnectionLifecycle lifecycle;
    mblink_mercedes_connection_lifecycle_init(&lifecycle);
    assert(!mblink_mercedes_connection_lifecycle_active(&lifecycle));
    assert(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_NONE);
    assert(lifecycle.stage == MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE);
    assert(lifecycle.module_index == 0U);
    assert(!lifecycle.disconnect_requested);

    mblink_mercedes_connection_lifecycle_begin(
        &lifecycle, MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    assert(mblink_mercedes_connection_lifecycle_active(&lifecycle));
    assert(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    lifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_CONTROL;
    lifecycle.module_index = 4U;
    lifecycle.disconnect_requested = true;

    const MblinkMercedesConnectionSessionMode finished =
        mblink_mercedes_connection_lifecycle_finish(&lifecycle);
    if (finished != MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH) return 1;
    assert(!mblink_mercedes_connection_lifecycle_active(&lifecycle));
    assert(lifecycle.stage == MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE);
    assert(lifecycle.module_index == 0U);
    /* Disconnect intent survives finishing an establishment operation. */
    assert(lifecycle.disconnect_requested);

    mblink_mercedes_connection_lifecycle_begin(
        &lifecycle, MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);
    assert(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);
    assert(lifecycle.disconnect_requested);
    return 0;
}
