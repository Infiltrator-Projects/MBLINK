// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.c
 * @brief Core project metadata and compatibility boundary for shared LINK code.
 */
#include "mblink/mblink.h"
#include "mblink/project_info.h"
#include "mblink/transport.h"

#include "infiltratr/core.h"

#include <stddef.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#ifndef MBLINK_VERSION
#error "MBLINK_VERSION must be supplied by the build system"
#endif

#ifndef MBLINK_BUILD_PROFILE
#define MBLINK_BUILD_PROFILE "source"
#endif

/*
 * Normal CMake builds consume shared engines through LINK::Core. The native
 * iPhone target still compiles the pinned LINK C sources into MBLINKCore; keep
 * this bridge complete until LINK's dedicated Apple static-library target
 * replaces the compatibility path.
 */
#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/src/core/workspace.c"
#include "../link/src/core/fuel_economy.c"
#include "../link/src/core/diagnostic_flow.c"
#include "../link/src/core/parameter.c"
#include "../link/src/core/scheduler.c"
#include "../link/src/core/telemetry.c"
#include "../link/src/core/transport.c"
#include "../link/src/elm327/elm327.c"
#include "../link/src/elm327/can.c"
#include "../link/src/elm327/probe.c"
#include "../link/src/elm327/session.c"
#include "../link/src/discover/safety.c"
#include "../link/src/discover/ecu_probe.c"
#endif

static const InfiltratrProjectInfo mblink_project_info_record = {
    .struct_size = sizeof(InfiltratrProjectInfo),
    .abi_version = INFILTRATR_PROJECT_INFO_ABI,
    .program_name = "MBLINK",
    .executable_name = "mblink",
    .application_id = "com.github.The-First-Infiltrator.MBLINK",
    .version = MBLINK_VERSION,
    .source_id = "The-First-Infiltrator/MBLINK",
    .build_profile = MBLINK_BUILD_PROFILE,
    .author = "Shannon Smith",
    .website = "https://github.com/The-First-Infiltrator/MBLINK",
    .license_id = "GPL-3.0-or-later",
    .comments = "Portable C Mercedes vehicle diagnostics face over LINK",
    .icon_name = "mblink",
    .copyright_text = "Copyright (C) 2026 Shannon Smith"
};

const InfiltratrProjectInfo *mblink_project_info(void)
{
    return &mblink_project_info_record;
}

const char *mblink_version(void)
{
    return MBLINK_VERSION;
}

const char *mblink_build_profile(void)
{
    return MBLINK_BUILD_PROFILE;
}

bool mblink_self_check(void)
{
    return infiltratr_project_info_is_valid(mblink_project_info());
}

bool mblink_transport_is_valid(const MblinkTransport *transport)
{
    return link_transport_is_valid(transport);
}
