// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/scheduler.h"

MblinkSchedulerResult mblink_scheduler_configure_standard_obd2(
    MblinkScheduler *scheduler,
    const MblinkObd2PidSet *supported,
    uint64_t first_due_ms)
{
    return link_scheduler_configure_standard_obd2(
        scheduler, supported, first_due_ms);
}
