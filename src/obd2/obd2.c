// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file obd2.c
 * @brief iOS build bridge to LINK's shared OBD-II implementation.
 *
 * CMake consumers link LINK::Core directly and do not compile this file.
 * The existing Xcode target still lists this product path, so on iOS it
 * compiles the exact source from the pinned LINK submodule rather than a copy.
 */
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/src/obd2/obd2.c"
#include "../link/src/obd2/dtc_knowledge.c"
#else
typedef int mblink_obd2_compat_translation_unit;
#endif
