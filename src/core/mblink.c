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

/*
 * Apple now compiles the pinned LINK implementation files as normal Xcode
 * translation units.  This keeps LINK file-local symbols isolated exactly as
 * they are in LINK::Core instead of amalgamating them into this product source.
 */

static const InfiltratrProjectInfo mblink_project_info_record = {
    .struct_size = sizeof(InfiltratrProjectInfo),
    .abi_version = INFILTRATR_PROJECT_INFO_ABI,
    .program_name = "MBLINK",
    .executable_name = "mblink",
    .application_id = "com.github.The-First-Infiltrator.MBLINK",
    .version = MBLINK_VERSION,
    .source_id = "Infiltrator-Projects/MBLINK",
    .build_profile = MBLINK_BUILD_PROFILE,
    .author = "Shannon Smith",
    .website = "https://github.com/Infiltrator-Projects/MBLINK",
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
