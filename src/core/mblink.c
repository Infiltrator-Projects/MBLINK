// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.c
 * @brief Core project metadata, workspace model and transport ABI validation.
 */
#include "mblink/mblink.h"
#include "mblink/transport.h"

#include "infiltratr/core.h"

#include <stddef.h>

#ifndef MBLINK_VERSION
#error "MBLINK_VERSION must be supplied by the build system"
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

static const MblinkWorkspaceSectionDescriptor mblink_workspace_sections[] = {
    {
        .section = MBLINK_WORKSPACE_VEHICLE,
        .key = "vehicle",
        .title = "Vehicle",
        .summary = "Vehicle identity, adapter and connection information"
    },
    {
        .section = MBLINK_WORKSPACE_MODULES,
        .key = "modules",
        .title = "Modules",
        .summary = "Discovered control modules and ECU identification"
    },
    {
        .section = MBLINK_WORKSPACE_FAULTS,
        .key = "faults",
        .title = "Faults",
        .summary = "Diagnostic trouble codes by control module"
    },
    {
        .section = MBLINK_WORKSPACE_LIVE_DATA,
        .key = "live-data",
        .title = "Live Data",
        .summary = "Search, select and favourite live diagnostic parameters"
    },
    {
        .section = MBLINK_WORKSPACE_TABLE,
        .key = "table",
        .title = "Table",
        .summary = "Dense live values for selected diagnostic parameters"
    },
    {
        .section = MBLINK_WORKSPACE_DASHBOARD,
        .key = "dashboard",
        .title = "Dashboard",
        .summary = "At-a-glance live diagnostic measurements"
    },
    {
        .section = MBLINK_WORKSPACE_GRAPHS,
        .key = "graphs",
        .title = "Graphs",
        .summary = "Time-series views for selected diagnostic parameters"
    },
    {
        .section = MBLINK_WORKSPACE_LOG,
        .key = "log",
        .title = "Log",
        .summary = "Diagnostic session history and exported telemetry"
    },
    {
        .section = MBLINK_WORKSPACE_SETTINGS,
        .key = "settings",
        .title = "Settings",
        .summary = "Display, adapter, build and application preferences"
    }
};

const char *mblink_version(void)
{
    return MBLINK_VERSION;
}

bool mblink_self_check(void)
{
    return infiltratr_project_info_is_valid(&mblink_project_info);
}

size_t mblink_workspace_section_count(void)
{
    return sizeof(mblink_workspace_sections) /
           sizeof(mblink_workspace_sections[0]);
}

const MblinkWorkspaceSectionDescriptor *mblink_workspace_section_at(size_t index)
{
    if (index >= mblink_workspace_section_count()) {
        return NULL;
    }
    return &mblink_workspace_sections[index];
}

const MblinkWorkspaceSectionDescriptor *mblink_workspace_section(
    MblinkWorkspaceSection section)
{
    size_t index;

    for (index = 0U; index < mblink_workspace_section_count(); ++index) {
        if (mblink_workspace_sections[index].section == section) {
            return &mblink_workspace_sections[index];
        }
    }
    return NULL;
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
