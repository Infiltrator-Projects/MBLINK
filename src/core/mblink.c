// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.c
 * @brief Core project metadata and transport ABI validation.
 */
#include "mblink/mblink.h"
#include "mblink/transport.h"

#include "infiltratr/core.h"

#include <stddef.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#ifndef MBLINK_VERSION
#error "MBLINK_VERSION must be supplied by the build system"
#endif

/*
 * Normal CMake builds consume workspace.c through LINK::Core. The native
 * iPhone project compiles portable C sources directly, so it compiles the
 * exact same workspace.c from the pinned LINK submodule here rather than
 * carrying an MBLINK implementation.
 */
#if defined(__APPLE__) && TARGET_OS_IOS
#include "../link/src/core/workspace.c"
#endif

static const InfiltratrProjectInfo mblink_project_info = {
    .struct_size = sizeof(InfiltratrProjectInfo),
    .abi_version = INFILTRATR_PROJECT_INFO_ABI,
    .program_name = "MBLINK",
    .executable_name = "mblink",
    .application_id = "com.github.The-First-Infiltrator.MBLINK",
    .version = MBLINK_VERSION,
    .source_id = "The-First-Infiltrator/MBLINK",
    .build_profile = "portable-c11",
    .author = "Shannon Smith",
    .website = "https://github.com/The-First-Infiltrator/MBLINK",
    .license_id = "GPL-3.0-or-later",
    .comments = "Portable C vehicle diagnostics core",
    .icon_name = "mblink",
    .copyright_text = "Copyright (C) 2026 Shannon Smith"
};

const char *mblink_version(void)
{
    return MBLINK_VERSION;
}

bool mblink_self_check(void)
{
    return infiltratr_project_info_is_valid(&mblink_project_info);
}

bool mblink_transport_is_valid(const MblinkTransport *transport)
{
    if (transport == NULL || transport->struct_size < sizeof(*transport) ||
        transport->abi_version != MBLINK_TRANSPORT_ABI) {
        return false;
    }

    return transport->connect != NULL &&
           transport->disconnect != NULL &&
           transport->is_connected != NULL &&
           transport->write != NULL &&
           transport->set_receiver != NULL;
}
