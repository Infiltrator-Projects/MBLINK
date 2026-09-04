// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file obd2.c
 * @brief iOS bridge to LINK's shared OBD-II implementation.
 *
 * CMake consumers link LINK::Core. The Xcode target still lists this product
 * path, so iOS compiles LINK's canonical Apple OBD-II entry point while the
 * product retains only the real Swift-visible MBLINK facade symbols below.
 */
#include "mblink/obd2.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/platform/apple/LinkPortableObd2.c"
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
