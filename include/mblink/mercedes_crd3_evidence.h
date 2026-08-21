// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_crd3_evidence.h
 * @brief Accumulator for decoded read-only CRD3 identity evidence.
 */
#ifndef MBLINK_MERCEDES_CRD3_EVIDENCE_H
#define MBLINK_MERCEDES_CRD3_EVIDENCE_H

#include "mblink/mercedes_crd3.h"
#include "mblink/uds.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool session_variant_available;
    MblinkMercedesCrd3SessionVariant session_variant;
    bool supplier_available;
    MblinkMercedesCrd3Supplier supplier;
    bool om651_cdid3_delphi_signature;
} MblinkMercedesCrd3Evidence;

static inline void mblink_mercedes_crd3_evidence_init(
    MblinkMercedesCrd3Evidence *evidence)
{
    if (evidence != NULL) {
        memset(evidence, 0, sizeof(*evidence));
    }
}

static inline void mblink_mercedes_crd3_evidence_update_signature(
    MblinkMercedesCrd3Evidence *evidence)
{
    if (evidence == NULL) {
        return;
    }
    evidence->om651_cdid3_delphi_signature =
        evidence->session_variant_available && evidence->supplier_available &&
        mblink_mercedes_crd3_matches_om651_cdid3_delphi_signature(
            &evidence->session_variant, &evidence->supplier);
}

/**
 * Consume one complete positive/negative UDS ReadDataByIdentifier PDU for a
 * known CRD3 fingerprint DID. Unknown fingerprint DIDs are unsupported.
 * Positive raw responses whose published payload shape cannot be decoded are
 * reported as unexpected rather than being coerced into an identity value.
 */
static inline MblinkUdsResult mblink_mercedes_crd3_evidence_accept(
    MblinkMercedesCrd3Evidence *evidence,
    uint16_t did,
    const uint8_t *pdu,
    size_t pdu_length)
{
    MblinkUdsDidRecord record;
    MblinkUdsResult result;

    if (evidence == NULL || pdu == NULL) {
        return MBLINK_UDS_RESULT_INVALID_ARGUMENT;
    }
    if (did != MBLINK_MERCEDES_CRD3_DID_SESSION_VARIANT &&
        did != MBLINK_MERCEDES_CRD3_DID_SUPPLIER_IDENTIFIER &&
        did != MBLINK_MERCEDES_CRD3_DID_EROTAN &&
        did != MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_FULL &&
        did != MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_PARTIAL) {
        return MBLINK_UDS_RESULT_UNSUPPORTED;
    }

    result = mblink_uds_decode_read_did_response(
        pdu, pdu_length, did, &record);
    if (result != MBLINK_UDS_RESULT_OK) {
        return result;
    }

    if (did == MBLINK_MERCEDES_CRD3_DID_SESSION_VARIANT) {
        MblinkMercedesCrd3SessionVariant decoded;
        if (!mblink_mercedes_crd3_decode_session_variant(
                record.data, record.data_length, &decoded)) {
            return MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE;
        }
        evidence->session_variant = decoded;
        evidence->session_variant_available = true;
    } else if (did == MBLINK_MERCEDES_CRD3_DID_SUPPLIER_IDENTIFIER) {
        MblinkMercedesCrd3Supplier decoded;
        if (!mblink_mercedes_crd3_decode_supplier(
                record.data, record.data_length, &decoded)) {
            return MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE;
        }
        evidence->supplier = decoded;
        evidence->supplier_available = true;
    }

    mblink_mercedes_crd3_evidence_update_signature(evidence);
    return MBLINK_UDS_RESULT_OK;
}

#ifdef __cplusplus
}
#endif

#endif
