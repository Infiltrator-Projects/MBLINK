// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mblink.h"
#include "mblink/project_info.h"
#include "mblink/transport.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MBLINK_TEST_EXPECTED_VERSION
#error "MBLINK_TEST_EXPECTED_VERSION must be supplied by the build system"
#endif

static MblinkTransportStatus mock_connect(void *context)
{
    (void)context;
    return MBLINK_TRANSPORT_OK;
}

static void mock_disconnect(void *context)
{
    (void)context;
}

static bool mock_is_connected(void *context)
{
    (void)context;
    return true;
}

static MblinkTransportStatus mock_write(void *context,
                                        const uint8_t *data,
                                        size_t size)
{
    (void)context;
    return (data != NULL && size > 0U) ? MBLINK_TRANSPORT_OK
                                       : MBLINK_TRANSPORT_INVALID_ARGUMENT;
}

static void mock_set_receiver(void *context,
                              MblinkTransportReceiveFn receiver,
                              void *receiver_context)
{
    (void)context;
    (void)receiver;
    (void)receiver_context;
}

static bool check(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "mblink-core-test: %s\n", message);
    }
    return condition;
}

static bool check_workspace(void)
{
    size_t index;
    bool passed = true;

    if (!check(mblink_workspace_section_count() ==
                   (size_t)MBLINK_WORKSPACE_SECTION_COUNT,
               "workspace section count mismatch")) {
        passed = false;
    }

    for (index = 0U; index < mblink_workspace_section_count(); ++index) {
        const MblinkWorkspaceSectionDescriptor *descriptor =
            mblink_workspace_section_at(index);
        if (!check(descriptor != NULL,
                   "workspace descriptor missing")) {
            passed = false;
            continue;
        }
        if (!check(descriptor->section == (MblinkWorkspaceSection)index,
                   "workspace section order is not stable") ||
            !check(descriptor->key != NULL && descriptor->key[0] != '\0',
                   "workspace key missing") ||
            !check(descriptor->title != NULL && descriptor->title[0] != '\0',
                   "workspace title missing") ||
            !check(descriptor->summary != NULL && descriptor->summary[0] != '\0',
                   "workspace summary missing") ||
            !check(mblink_workspace_section(descriptor->section) == descriptor,
                   "workspace identifier lookup mismatch")) {
            passed = false;
        }
    }

    if (!check(mblink_workspace_section_at(mblink_workspace_section_count()) == NULL,
               "out-of-range workspace index should fail") ||
        !check(mblink_workspace_section(MBLINK_WORKSPACE_SECTION_COUNT) == NULL,
               "out-of-range workspace identifier should fail")) {
        passed = false;
    }

    return passed;
}

int main(void)
{
    bool passed = true;
    MblinkTransport transport = MBLINK_TRANSPORT_INIT;
    static const uint8_t probe[] = { 'A', 'T', 'I', '\r' };

    if (!check(strcmp(mblink_version(), MBLINK_TEST_EXPECTED_VERSION) == 0,
               "linked MBLINK version does not match build version")) {
        passed = false;
    }
    if (!check(mblink_self_check(), "project identity validation failed")) {
        passed = false;
    }
    {
        const InfiltratrProjectInfo *info = mblink_project_info();
        if (!check(info != NULL && strcmp(info->source_id, "Infiltrator-Projects/MBLINK") == 0,
                   "project source identity mismatch") ||
            !check(strcmp(info->build_profile, mblink_build_profile()) == 0,
                   "project build profile mismatch")) {
            passed = false;
        }
    }
    if (!check_workspace()) {
        passed = false;
    }
    if (!check(!mblink_transport_is_valid(&transport),
               "empty transport should be invalid")) {
        passed = false;
    }

    transport.connect = mock_connect;
    transport.disconnect = mock_disconnect;
    transport.is_connected = mock_is_connected;
    transport.write = mock_write;
    transport.set_receiver = mock_set_receiver;

    if (!check(mblink_transport_is_valid(&transport),
               "complete transport should be valid")) {
        passed = false;
    }

    transport.abi_version = MBLINK_TRANSPORT_ABI + 1U;
    if (!check(!mblink_transport_is_valid(&transport),
               "unknown transport ABI should be rejected")) {
        passed = false;
    }
    transport.abi_version = MBLINK_TRANSPORT_ABI;

    if (!check(transport.connect(transport.context) == MBLINK_TRANSPORT_OK,
               "mock connect failed")) {
        passed = false;
    }
    if (!check(transport.is_connected(transport.context),
               "mock transport did not report connected")) {
        passed = false;
    }
    if (!check(transport.write(transport.context, probe, sizeof(probe)) ==
                   MBLINK_TRANSPORT_OK,
               "mock write failed")) {
        passed = false;
    }

    transport.disconnect(transport.context);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
