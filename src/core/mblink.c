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
 * Evidence exported by the native iPhone build must identify the exact source
 * stack that produced it.  Keep these pinned values beside the compatibility
 * boundary so a field CSV can never again leave the MBLINK/LINK revision
 * ambiguous.  MBLINK_IMPLEMENTATION_REVISION is advanced with each release
 * implementation; LINK values must match the src/link gitlink exactly.
 */
#ifndef MBLINK_IMPLEMENTATION_REVISION
#define MBLINK_IMPLEMENTATION_REVISION "pending-main"
#endif
#define MBLINK_EMBEDDED_LINK_VERSION "0.14.25"
#define MBLINK_EMBEDDED_LINK_REVISION \
    "4fceb6fa8fb18b15f71e3c8fe84d326c5445f509"

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

/*
 * Rename the two stream-start entry points while compiling LINK telemetry so
 * MBLINK can append product/build provenance to every session without forking
 * LINK's recorder or changing the stable telemetry ABI.
 */
#define link_telemetry_recorder_begin \
    mblink_link_telemetry_recorder_begin_without_build_metadata
#define link_telemetry_recorder_continue \
    mblink_link_telemetry_recorder_continue_without_build_metadata
#include "../link/src/core/telemetry.c"
#undef link_telemetry_recorder_begin
#undef link_telemetry_recorder_continue

static bool mblink_telemetry_emit_build_metadata(
    LinkTelemetryRecorder *recorder)
{
    return recorder != NULL && recorder->started && !recorder->failed &&
           emit_metadata(recorder->sink, recorder->context,
                         "mblink_version", MBLINK_VERSION) &&
           emit_metadata(recorder->sink, recorder->context,
                         "mblink_implementation_revision",
                         MBLINK_IMPLEMENTATION_REVISION) &&
           emit_metadata(recorder->sink, recorder->context,
                         "link_version", MBLINK_EMBEDDED_LINK_VERSION) &&
           emit_metadata(recorder->sink, recorder->context,
                         "link_revision", MBLINK_EMBEDDED_LINK_REVISION);
}

bool link_telemetry_recorder_begin(
    LinkTelemetryRecorder *recorder,
    const LinkTelemetrySessionMetadata *metadata,
    const char *product_slug,
    LinkTelemetryTextSink sink,
    void *context)
{
    if (!mblink_link_telemetry_recorder_begin_without_build_metadata(
            recorder, metadata, product_slug, sink, context)) {
        return false;
    }
    return mblink_telemetry_emit_build_metadata(recorder);
}

bool link_telemetry_recorder_continue(
    LinkTelemetryRecorder *recorder,
    const LinkTelemetrySessionMetadata *metadata,
    const char *product_slug,
    LinkTelemetryTextSink sink,
    void *context)
{
    if (!mblink_link_telemetry_recorder_continue_without_build_metadata(
            recorder, metadata, product_slug, sink, context)) {
        return false;
    }
    return mblink_telemetry_emit_build_metadata(recorder);
}

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
