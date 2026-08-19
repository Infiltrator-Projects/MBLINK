// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mblink.h
 * @brief Public portable C interface for the MBLINK diagnostics core.
 */
#ifndef MBLINK_MBLINK_H
#define MBLINK_MBLINK_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Stable top-level diagnostic workspace shared by every front end. */
typedef enum MblinkWorkspaceSection {
    MBLINK_WORKSPACE_VEHICLE = 0,
    MBLINK_WORKSPACE_MODULES,
    MBLINK_WORKSPACE_FAULTS,
    MBLINK_WORKSPACE_LIVE_DATA,
    MBLINK_WORKSPACE_TABLE,
    MBLINK_WORKSPACE_DASHBOARD,
    MBLINK_WORKSPACE_GRAPHS,
    MBLINK_WORKSPACE_LOG,
    MBLINK_WORKSPACE_SETTINGS,
    MBLINK_WORKSPACE_SECTION_COUNT
} MblinkWorkspaceSection;

/** Shared section metadata. Platform shells own only presentation details. */
typedef struct MblinkWorkspaceSectionDescriptor {
    MblinkWorkspaceSection section;
    const char *key;
    const char *title;
    const char *summary;
} MblinkWorkspaceSectionDescriptor;

/** Return the semantic version of the linked MBLINK core. */
const char *mblink_version(void);

/** Validate the core's shared project-metadata contract. */
bool mblink_self_check(void);

/** Return the number of stable top-level diagnostic workspace sections. */
size_t mblink_workspace_section_count(void);

/** Return shared metadata for a workspace section index, or NULL if invalid. */
const MblinkWorkspaceSectionDescriptor *mblink_workspace_section_at(size_t index);

/** Return shared metadata for a workspace section identifier, or NULL if invalid. */
const MblinkWorkspaceSectionDescriptor *mblink_workspace_section(
    MblinkWorkspaceSection section);

#ifdef __cplusplus
}
#endif

#endif
