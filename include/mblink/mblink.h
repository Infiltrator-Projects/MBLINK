// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.h
 * @brief Public portable C interface for the MBLINK diagnostics core.
 */
#ifndef MBLINK_MBLINK_H
#define MBLINK_MBLINK_H

#include <stdbool.h>
#include <stddef.h>

#include "link/workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Workspace ownership lives in LINK. These aliases preserve the MBLINK-facing
 * source API while every front end consumes the same shared model.
 */
typedef LinkWorkspaceSection MblinkWorkspaceSection;
typedef LinkWorkspaceSectionDescriptor MblinkWorkspaceSectionDescriptor;

#define MBLINK_WORKSPACE_VEHICLE LINK_WORKSPACE_VEHICLE
#define MBLINK_WORKSPACE_MODULES LINK_WORKSPACE_MODULES
#define MBLINK_WORKSPACE_FAULTS LINK_WORKSPACE_FAULTS
#define MBLINK_WORKSPACE_LIVE_DATA LINK_WORKSPACE_LIVE_DATA
#define MBLINK_WORKSPACE_TABLE LINK_WORKSPACE_TABLE
#define MBLINK_WORKSPACE_DASHBOARD LINK_WORKSPACE_DASHBOARD
#define MBLINK_WORKSPACE_GRAPHS LINK_WORKSPACE_GRAPHS
#define MBLINK_WORKSPACE_LOG LINK_WORKSPACE_LOG
#define MBLINK_WORKSPACE_SETTINGS LINK_WORKSPACE_SETTINGS
#define MBLINK_WORKSPACE_SECTION_COUNT LINK_WORKSPACE_SECTION_COUNT

#define mblink_workspace_section_count link_workspace_section_count
#define mblink_workspace_section_at link_workspace_section_at
#define mblink_workspace_section link_workspace_section

/** Return the semantic version of the linked MBLINK core. */
const char *mblink_version(void);

/** Validate the core's shared project-metadata contract. */
bool mblink_self_check(void);

#ifdef __cplusplus
}
#endif

#endif
