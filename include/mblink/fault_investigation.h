// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file fault_investigation.h
 * @brief MBLINK compatibility facade for LINK's shared fault-scan state.
 */
#ifndef MBLINK_FAULT_INVESTIGATION_H
#define MBLINK_FAULT_INVESTIGATION_H

#include "link/fault_scan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef LinkFaultScanPresentationState MblinkFaultScanPresentationState;

#define MBLINK_FAULT_SCAN_NOT_SCANNED LINK_FAULT_SCAN_NOT_SCANNED
#define MBLINK_FAULT_SCAN_IN_PROGRESS LINK_FAULT_SCAN_IN_PROGRESS
#define MBLINK_FAULT_SCAN_FAILED LINK_FAULT_SCAN_FAILED
#define MBLINK_FAULT_SCAN_CLEAN LINK_FAULT_SCAN_CLEAN
#define MBLINK_FAULT_SCAN_FAULTS_PRESENT LINK_FAULT_SCAN_FAULTS_PRESENT

#define mblink_fault_scan_presentation_state \
    link_fault_scan_presentation_state
#define mblink_fault_scan_presentation_state_name \
    link_fault_scan_presentation_state_name

#ifdef __cplusplus
}
#endif

#endif
