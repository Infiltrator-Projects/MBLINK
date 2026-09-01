// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file obd2.c
 * @brief iOS build bridge to LINK's shared OBD-II implementation.
 *
 * CMake consumers link LINK::Core directly and do not compile this file.
 * The existing Xcode target still lists this product path, so on iOS it
 * compiles the exact source from the pinned LINK submodule rather than a copy.
 */
#include "mblink/obd2.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/src/obd2/obd2.c"
#include "../link/src/obd2/pid_catalogue.c"
#include "../link/src/obd2/j1979da.c"
#include "../link/src/obd2/dtc_knowledge.c"
#endif

size_t mblink_obd2_pid_definition_count(void)
{
    return link_obd2_pid_definition_count();
}

const MblinkObd2PidDefinition *mblink_obd2_pid_definition_at(size_t index)
{
    return link_obd2_pid_definition_at(index);
}

const MblinkObd2PidDefinition *mblink_obd2_pid_definition(
    uint8_t mode, uint8_t pid)
{
    return link_obd2_pid_definition(mode, pid);
}
