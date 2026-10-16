// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_connection_lifecycle.h"

#include <stdlib.h>
#include <stdio.h>

// Checks remain executable under NDEBUG in optimized qualification builds.
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

int main(void)
{
    MblinkMercedesConnectionLifecycle lifecycle;
    mblink_mercedes_connection_lifecycle_init(&lifecycle);
    CHECK(!mblink_mercedes_connection_lifecycle_active(&lifecycle));
    CHECK(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_NONE);
    CHECK(lifecycle.stage == MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE);
    CHECK(lifecycle.module_index == 0U);
    CHECK(!lifecycle.disconnect_requested);

    mblink_mercedes_connection_lifecycle_begin(
        &lifecycle, MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    CHECK(mblink_mercedes_connection_lifecycle_active(&lifecycle));
    CHECK(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH);
    lifecycle.stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_CONTROL;
    lifecycle.module_index = 4U;
    lifecycle.disconnect_requested = true;

    const MblinkMercedesConnectionSessionMode finished =
        mblink_mercedes_connection_lifecycle_finish(&lifecycle);
    if (finished != MBLINK_MERCEDES_CONNECTION_SESSION_ESTABLISH) return 1;
    CHECK(!mblink_mercedes_connection_lifecycle_active(&lifecycle));
    CHECK(lifecycle.stage == MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE);
    CHECK(lifecycle.module_index == 0U);
    /* Disconnect intent survives finishing an establishment operation. */
    CHECK(lifecycle.disconnect_requested);

    mblink_mercedes_connection_lifecycle_begin(
        &lifecycle, MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);
    CHECK(lifecycle.mode == MBLINK_MERCEDES_CONNECTION_SESSION_TEARDOWN);
    CHECK(lifecycle.disconnect_requested);
    return 0;
}
