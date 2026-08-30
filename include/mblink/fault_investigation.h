// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file fault_investigation.h
 * @brief Presentation-safe fault scan outcome classification.
 *
 * A zero-length fault list is not proof of a clean vehicle.  These helpers
 * deliberately require a completed scan before returning CLEAN and preserve
 * failure/incomplete states even when some fault records were captured first.
 */
#ifndef MBLINK_FAULT_INVESTIGATION_H
#define MBLINK_FAULT_INVESTIGATION_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBLINK_FAULT_SCAN_NOT_SCANNED = 0,
    MBLINK_FAULT_SCAN_IN_PROGRESS,
    MBLINK_FAULT_SCAN_FAILED,
    MBLINK_FAULT_SCAN_CLEAN,
    MBLINK_FAULT_SCAN_FAULTS_PRESENT
} MblinkFaultScanPresentationState;

static inline MblinkFaultScanPresentationState
mblink_fault_scan_presentation_state(
    bool started,
    bool active,
    bool complete,
    bool failed,
    size_t fault_count)
{
    if (failed) return MBLINK_FAULT_SCAN_FAILED;
    if (complete) {
        return fault_count == 0U
            ? MBLINK_FAULT_SCAN_CLEAN
            : MBLINK_FAULT_SCAN_FAULTS_PRESENT;
    }
    if (started || active) return MBLINK_FAULT_SCAN_IN_PROGRESS;
    return MBLINK_FAULT_SCAN_NOT_SCANNED;
}

static inline const char *mblink_fault_scan_presentation_state_name(
    MblinkFaultScanPresentationState state)
{
    switch (state) {
    case MBLINK_FAULT_SCAN_NOT_SCANNED: return "not-scanned";
    case MBLINK_FAULT_SCAN_IN_PROGRESS: return "in-progress";
    case MBLINK_FAULT_SCAN_FAILED: return "failed";
    case MBLINK_FAULT_SCAN_CLEAN: return "clean";
    case MBLINK_FAULT_SCAN_FAULTS_PRESENT: return "faults-present";
    }
    return "unknown";
}

#ifdef __cplusplus
}
#endif

#endif
