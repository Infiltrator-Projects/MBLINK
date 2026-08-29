/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef MBLINK_DISCOVER_H
#define MBLINK_DISCOVER_H

#include "link/discover.h"

typedef link_safety_decision mblink_safety_decision;
#define MBLINK_SAFETY_BLOCK LINK_SAFETY_BLOCK
#define MBLINK_SAFETY_ALLOW_READ_ONLY LINK_SAFETY_ALLOW_READ_ONLY

typedef link_safety_reason mblink_safety_reason;
#define MBLINK_SAFETY_REASON_ALLOWED_OBD_READ LINK_SAFETY_REASON_ALLOWED_OBD_READ
#define MBLINK_SAFETY_REASON_ALLOWED_UDS_READ LINK_SAFETY_REASON_ALLOWED_UDS_READ
#define MBLINK_SAFETY_REASON_EMPTY_REQUEST LINK_SAFETY_REASON_EMPTY_REQUEST
#define MBLINK_SAFETY_REASON_WRITE_OR_CONTROL LINK_SAFETY_REASON_WRITE_OR_CONTROL
#define MBLINK_SAFETY_REASON_ECU_RESET LINK_SAFETY_REASON_ECU_RESET
#define MBLINK_SAFETY_REASON_SECURITY_ACCESS LINK_SAFETY_REASON_SECURITY_ACCESS
#define MBLINK_SAFETY_REASON_ROUTINE_CONTROL LINK_SAFETY_REASON_ROUTINE_CONTROL
#define MBLINK_SAFETY_REASON_DTC_CLEAR LINK_SAFETY_REASON_DTC_CLEAR
#define MBLINK_SAFETY_REASON_PROGRAMMING LINK_SAFETY_REASON_PROGRAMMING
#define MBLINK_SAFETY_REASON_DENY_BY_DEFAULT LINK_SAFETY_REASON_DENY_BY_DEFAULT

typedef link_safety_result mblink_safety_result;
#define mblink_safety_classify link_safety_classify
#define mblink_safety_reason_string link_safety_reason_string

typedef link_evidence_writer mblink_evidence_writer;
#define mblink_evidence_open link_evidence_open
#define mblink_evidence_write_frame link_evidence_write_frame
#define mblink_evidence_write_annotation link_evidence_write_annotation
#define mblink_evidence_flush link_evidence_flush
#define mblink_evidence_close link_evidence_close

typedef struct MblinkMercedesKnownRoute {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    const char *module_key;
    const char *qualifier;
    const char *provenance;
} MblinkMercedesKnownRoute;

/**
 * Return source-corroborated Mercedes 11-bit diagnostic routes whose response
 * identifier is not the generic request+8 mapping.
 */
size_t mblink_mercedes_known_route_count(void);
const MblinkMercedesKnownRoute *mblink_mercedes_known_route_at(size_t index);
const MblinkMercedesKnownRoute *mblink_mercedes_known_route_for_tx(
    uint32_t tx_can_id);

/*
 * Mercedes owns the address/probe strategy; LINK owns the sweep machinery.
 * Both Linux and Windows consume this same product plan.
 */
const link_discover_sweep_plan *mblink_discover_full_sweep_plan(void);

#endif
