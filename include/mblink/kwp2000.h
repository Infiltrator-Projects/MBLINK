// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file kwp2000.h
 * @brief MBLINK compatibility aliases for LINK's shared ISO 14230-3 engine.
 */
#ifndef MBLINK_KWP2000_H
#define MBLINK_KWP2000_H

#include "link/kwp2000.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef LinkKwp2000Result MblinkKwp2000Result;
typedef LinkKwp2000ResponseKind MblinkKwp2000ResponseKind;
typedef LinkKwp2000Response MblinkKwp2000Response;
typedef LinkKwp2000LocalIdentifierRecord MblinkKwp2000LocalIdentifierRecord;
typedef LinkKwp2000CommonIdentifierRecord MblinkKwp2000CommonIdentifierRecord;

/*
 * LINK's generic ECU-identification decoder deliberately returns a zero-copy
 * view into the caller's PDU. MBLINK's Mercedes scanner decodes an ELM response
 * into a temporary PDU before interpreting Daimler 1A86/1A87/1A89 fields, so a
 * simple typedef would let that view outlive the PDU and leave record.data
 * dangling. Keep an MBLINK-owned copy for ECU identity records so callers can
 * safely consume the bytes after the decoder helper returns.
 */
#define MBLINK_KWP2000_ECU_IDENTIFICATION_DATA_CAPACITY 512U
typedef struct MblinkKwp2000EcuIdentificationRecord {
    uint8_t option;
    uint8_t data[MBLINK_KWP2000_ECU_IDENTIFICATION_DATA_CAPACITY];
    size_t data_length;
} MblinkKwp2000EcuIdentificationRecord;

typedef LinkKwp2000Dtc MblinkKwp2000Dtc;
typedef LinkKwp2000DtcList MblinkKwp2000DtcList;

#define MBLINK_KWP2000_SERVICE_READ_DTC_BY_STATUS LINK_KWP2000_SERVICE_READ_DTC_BY_STATUS
#define MBLINK_KWP2000_SERVICE_READ_ECU_IDENTIFICATION LINK_KWP2000_SERVICE_READ_ECU_IDENTIFICATION
#define MBLINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER LINK_KWP2000_SERVICE_READ_DATA_BY_LOCAL_IDENTIFIER
#define MBLINK_KWP2000_SERVICE_READ_DATA_BY_COMMON_IDENTIFIER LINK_KWP2000_SERVICE_READ_DATA_BY_COMMON_IDENTIFIER
#define MBLINK_KWP2000_SERVICE_TESTER_PRESENT LINK_KWP2000_SERVICE_TESTER_PRESENT
#define MBLINK_KWP2000_SERVICE_NEGATIVE_RESPONSE LINK_KWP2000_SERVICE_NEGATIVE_RESPONSE
#define MBLINK_KWP2000_DTC_REQUEST_STORED_AND_STATUS LINK_KWP2000_DTC_REQUEST_STORED_AND_STATUS
#define MBLINK_KWP2000_DTC_GROUP_ALL LINK_KWP2000_DTC_GROUP_ALL
#define MBLINK_KWP2000_RESULT_OK LINK_KWP2000_RESULT_OK
#define MBLINK_KWP2000_RESULT_NEGATIVE_RESPONSE LINK_KWP2000_RESULT_NEGATIVE_RESPONSE
#define MBLINK_KWP2000_RESULT_INVALID_ARGUMENT LINK_KWP2000_RESULT_INVALID_ARGUMENT
#define MBLINK_KWP2000_RESULT_MALFORMED_PDU LINK_KWP2000_RESULT_MALFORMED_PDU
#define MBLINK_KWP2000_RESULT_UNEXPECTED_RESPONSE LINK_KWP2000_RESULT_UNEXPECTED_RESPONSE
#define MBLINK_KWP2000_RESULT_TRUNCATED LINK_KWP2000_RESULT_TRUNCATED

#define mblink_kwp2000_result_name link_kwp2000_result_name
#define mblink_kwp2000_decode_response link_kwp2000_decode_response
#define mblink_kwp2000_build_tester_present_request link_kwp2000_build_tester_present_request
#define mblink_kwp2000_decode_tester_present_response link_kwp2000_decode_tester_present_response
#define mblink_kwp2000_build_read_local_identifier_request link_kwp2000_build_read_local_identifier_request
#define mblink_kwp2000_decode_read_local_identifier_response link_kwp2000_decode_read_local_identifier_response
#define mblink_kwp2000_build_read_common_identifier_request link_kwp2000_build_read_common_identifier_request
#define mblink_kwp2000_decode_read_common_identifier_response link_kwp2000_decode_read_common_identifier_response
#define mblink_kwp2000_build_read_ecu_identification_request link_kwp2000_build_read_ecu_identification_request
#define mblink_kwp2000_build_read_dtc_by_status_request link_kwp2000_build_read_dtc_by_status_request
#define mblink_kwp2000_decode_read_dtc_by_status_response link_kwp2000_decode_read_dtc_by_status_response

static inline MblinkKwp2000Result
mblink_kwp2000_decode_read_ecu_identification_response(
    const uint8_t *pdu,
    size_t pdu_length,
    uint8_t expected_option,
    MblinkKwp2000EcuIdentificationRecord *record)
{
    LinkKwp2000EcuIdentificationRecord shared;
    LinkKwp2000Result result;

    if (record == NULL)
        return MBLINK_KWP2000_RESULT_INVALID_ARGUMENT;

    record->option = 0U;
    record->data_length = 0U;
    result = link_kwp2000_decode_read_ecu_identification_response(
        pdu, pdu_length, expected_option, &shared);
    if (result != LINK_KWP2000_RESULT_OK)
        return result;
    if (shared.data_length >
        MBLINK_KWP2000_ECU_IDENTIFICATION_DATA_CAPACITY)
        return MBLINK_KWP2000_RESULT_TRUNCATED;
    if (shared.data_length != 0U && shared.data == NULL)
        return MBLINK_KWP2000_RESULT_MALFORMED_PDU;

    record->option = shared.option;
    record->data_length = shared.data_length;
    if (shared.data_length != 0U)
        memcpy(record->data, shared.data, shared.data_length);
    return MBLINK_KWP2000_RESULT_OK;
}

#ifdef __cplusplus
}
#endif

#endif
