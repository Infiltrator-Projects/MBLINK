// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.c
 * @brief Portable C foundation for MBLINK.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#include "mblink/mblink.h"
#include "mblink/transport.h"
#include "mblink/scheduler.h"
#include "mblink/telemetry.h"

#include "infiltratr/core.h"
#include "infiltratr/format.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef MBLINK_VERSION
#define MBLINK_VERSION "0.1.0"
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


/* Private implementation fragments remain part of this translation unit so the
 * portable core and Xcode static-library target compile the same C sources. */
#include "scheduler.inc"
#include "telemetry_store.inc"
#include "telemetry_csv.inc"
