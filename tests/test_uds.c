// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/uds.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_generic_response_decode(void)
{
    const uint8_t positive[] = { 0x62U, 0xf1U, 0x90U, 0x41U, 0x42U };
    const uint8_t negative[] = { 0x7fU, 0x22U, 0x31U };
    const uint8_t pending[] = { 0x7fU, 0x22U, 0x78U };
    const uint8_t malformed[] = { 0x7fU, 0x22U };
    const uint8_t wrong_service[] = { 0x63U, 0x00U };
    MblinkUdsResponse response;

    memset(&response, 0xa5, sizeof(response));
    CHECK(mblink_uds_decode_response(
              0x22U, positive, sizeof(positive), &response) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(response.kind == MBLINK_UDS_RESPONSE_POSITIVE);
    CHECK(response.request_service == 0x22U);
    CHECK(response.response_service == 0x62U);
    CHECK(response.negative_response_code == 0U);
    CHECK(response.data == positive + 1U);
    CHECK(response.data_length == sizeof(positive) - 1U);

    CHECK(mblink_uds_decode_response(
              0x22U, negative, sizeof(negative), &response) ==
          MBLINK_UDS_RESULT_NEGATIVE_RESPONSE);
    CHECK(response.kind == MBLINK_UDS_RESPONSE_NEGATIVE);
    CHECK(response.negative_response_code == 0x31U);
    CHECK(strcmp(mblink_uds_negative_response_code_name(0x31U),
                 "request-out-of-range") == 0);

    CHECK(mblink_uds_decode_response(
              0x22U, pending, sizeof(pending), &response) ==
          MBLINK_UDS_RESULT_NEGATIVE_RESPONSE);
    CHECK(response.negative_response_code == MBLINK_UDS_NRC_RESPONSE_PENDING);
    CHECK(strcmp(mblink_uds_negative_response_code_name(0x78U),
                 "response-pending") == 0);
    CHECK(strcmp(mblink_uds_negative_response_code_name(0x99U),
                 "unknown-nrc") == 0);

    memset(&response, 0x5a, sizeof(response));
    {
        MblinkUdsResponse snapshot = response;
        CHECK(mblink_uds_decode_response(
                  0x22U, malformed, sizeof(malformed), &response) ==
              MBLINK_UDS_RESULT_MALFORMED_PDU);
        CHECK(memcmp(&response, &snapshot, sizeof(response)) == 0);
    }

    memset(&response, 0x5a, sizeof(response));
    {
        MblinkUdsResponse snapshot = response;
        CHECK(mblink_uds_decode_response(
                  0x22U, wrong_service, sizeof(wrong_service), &response) ==
              MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
        CHECK(memcmp(&response, &snapshot, sizeof(response)) == 0);
    }

    CHECK(mblink_uds_decode_response(
              0x7fU, positive, sizeof(positive), &response) ==
          MBLINK_UDS_RESULT_INVALID_ARGUMENT);
    CHECK(strcmp(mblink_uds_result_name(MBLINK_UDS_RESULT_TIMEOUT),
                 "timeout") == 0);
    CHECK(strcmp(mblink_uds_client_state_name(MBLINK_UDS_CLIENT_COMPLETE),
                 "complete") == 0);
    return 0;
}

static int test_session_control(void)
{
    uint8_t request[4] = { 0xa5U, 0xa5U, 0xa5U, 0xa5U };
    size_t written = 99U;
    const uint8_t response_pdu[] = { 0x50U, 0x03U, 0x00U, 0x32U, 0x01U, 0xf4U };
    const uint8_t minimal_pdu[] = { 0x50U, 0x03U };
    const uint8_t wrong_echo[] = { 0x50U, 0x02U };
    const uint8_t zero_timing[] = { 0x50U, 0x03U, 0x00U, 0x00U, 0x01U, 0x00U };
    MblinkUdsSessionResponse response;

    CHECK(mblink_uds_build_session_control_request(
              MBLINK_UDS_SESSION_EXTENDED, false,
              request, sizeof(request), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(written == 2U && request[0] == 0x10U && request[1] == 0x03U);

    CHECK(mblink_uds_build_session_control_request(
              MBLINK_UDS_SESSION_EXTENDED, true,
              request, sizeof(request), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(written == 2U && request[1] == 0x83U);

    request[0] = 0xa5U;
    written = 99U;
    CHECK(mblink_uds_build_session_control_request(
              MBLINK_UDS_SESSION_EXTENDED, false,
              request, 1U, &written) == MBLINK_UDS_RESULT_BUFFER_TOO_SMALL);
    CHECK(written == 0U && request[0] == 0U);

    memset(&response, 0, sizeof(response));
    CHECK(mblink_uds_decode_session_control_response(
              response_pdu, sizeof(response_pdu), MBLINK_UDS_SESSION_EXTENDED,
              &response) == MBLINK_UDS_RESULT_OK);
    CHECK(response.session_type == MBLINK_UDS_SESSION_EXTENDED);
    CHECK(response.timing_present);
    CHECK(response.p2_server_max_ms == 50U);
    CHECK(response.p2_star_server_max_10ms == 500U);

    memset(&response, 0, sizeof(response));
    CHECK(mblink_uds_decode_session_control_response(
              minimal_pdu, sizeof(minimal_pdu), MBLINK_UDS_SESSION_EXTENDED,
              &response) == MBLINK_UDS_RESULT_OK);
    CHECK(response.session_type == MBLINK_UDS_SESSION_EXTENDED);
    CHECK(!response.timing_present);

    memset(&response, 0x5a, sizeof(response));
    {
        MblinkUdsSessionResponse snapshot = response;
        CHECK(mblink_uds_decode_session_control_response(
                  wrong_echo, sizeof(wrong_echo), MBLINK_UDS_SESSION_EXTENDED,
                  &response) == MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
        CHECK(memcmp(&response, &snapshot, sizeof(response)) == 0);
    }

    CHECK(mblink_uds_decode_session_control_response(
              zero_timing, sizeof(zero_timing), MBLINK_UDS_SESSION_EXTENDED,
              &response) == MBLINK_UDS_RESULT_MALFORMED_PDU);
    return 0;
}

static int test_tester_present(void)
{
    uint8_t request[3] = { 0xa5U, 0xa5U, 0xa5U };
    size_t written = 99U;
    const uint8_t response_pdu[] = { 0x7eU, 0x00U };
    const uint8_t wrong_subfunction[] = { 0x7eU, 0x01U };
    const uint8_t extra_data[] = { 0x7eU, 0x00U, 0x00U };

    CHECK(mblink_uds_build_tester_present_request(
              false, request, sizeof(request), &written) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(written == 2U && request[0] == 0x3eU && request[1] == 0x00U);

    CHECK(mblink_uds_build_tester_present_request(
              true, request, sizeof(request), &written) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(written == 2U && request[1] == 0x80U);

    CHECK(mblink_uds_decode_tester_present_response(
              response_pdu, sizeof(response_pdu)) == MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_decode_tester_present_response(
              wrong_subfunction, sizeof(wrong_subfunction)) ==
          MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
    CHECK(mblink_uds_decode_tester_present_response(
              extra_data, sizeof(extra_data)) == MBLINK_UDS_RESULT_MALFORMED_PDU);
    return 0;
}

static int test_read_did(void)
{
    uint8_t request[4] = { 0 };
    size_t written = 0U;
    const uint8_t response_pdu[] = {
        0x62U, 0xf1U, 0x90U, 0x41U, 0x42U, 0x43U
    };
    const uint8_t wrong_did[] = { 0x62U, 0xf1U, 0x91U, 0x00U };
    const uint8_t truncated[] = { 0x62U, 0xf1U };
    MblinkUdsDidRecord record;

    CHECK(mblink_uds_build_read_did_request(
              0xf190U, request, sizeof(request), &written) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(written == 3U && request[0] == 0x22U &&
          request[1] == 0xf1U && request[2] == 0x90U);

    CHECK(mblink_uds_decode_read_did_response(
              response_pdu, sizeof(response_pdu), 0xf190U, &record) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(record.identifier == 0xf190U);
    CHECK(record.data == response_pdu + 3U);
    CHECK(record.data_length == 3U);
    CHECK(record.data[0] == 0x41U && record.data[2] == 0x43U);

    memset(&record, 0x5a, sizeof(record));
    {
        MblinkUdsDidRecord snapshot = record;
        CHECK(mblink_uds_decode_read_did_response(
                  wrong_did, sizeof(wrong_did), 0xf190U, &record) ==
              MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
        CHECK(memcmp(&record, &snapshot, sizeof(record)) == 0);
    }

    CHECK(mblink_uds_decode_read_did_response(
              truncated, sizeof(truncated), 0xf190U, &record) ==
          MBLINK_UDS_RESULT_MALFORMED_PDU);
    return 0;
}

static int test_did_definitions(void)
{
    const MblinkUdsDidDefinition definition = {
        .identifier = 0x1234U,
        .key = "ecu-identity",
        .name = "ECU identity",
        .minimum_length = 2U,
        .maximum_length = 4U
    };
    MblinkUdsDidDefinition invalid = definition;
    uint8_t request[4] = { 0 };
    size_t written = 0U;
    const uint8_t response_pdu[] = { 0x62U, 0x12U, 0x34U, 0xaaU, 0xbbU, 0xccU };
    const uint8_t too_short[] = { 0x62U, 0x12U, 0x34U, 0xaaU };
    MblinkUdsDidValue value;

    CHECK(mblink_uds_did_definition_is_valid(&definition));
    invalid.key = "";
    CHECK(!mblink_uds_did_definition_is_valid(&invalid));
    invalid = definition;
    invalid.minimum_length = 5U;
    CHECK(!mblink_uds_did_definition_is_valid(&invalid));

    CHECK(mblink_uds_build_defined_did_request(
              &definition, request, sizeof(request), &written) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(written == 3U && request[0] == 0x22U &&
          request[1] == 0x12U && request[2] == 0x34U);

    CHECK(mblink_uds_decode_defined_did_response(
              response_pdu, sizeof(response_pdu), &definition, &value) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(value.definition == &definition);
    CHECK(value.data == response_pdu + 3U);
    CHECK(value.data_length == 3U);

    memset(&value, 0x5a, sizeof(value));
    {
        MblinkUdsDidValue snapshot = value;
        CHECK(mblink_uds_decode_defined_did_response(
                  too_short, sizeof(too_short), &definition, &value) ==
              MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
        CHECK(memcmp(&value, &snapshot, sizeof(value)) == 0);
    }
    return 0;
}

static int test_client_session_and_timing(void)
{
    MblinkUdsClient client;
    const MblinkUdsClientConfig config = {
        .p2_timeout_us = UINT64_C(100000),
        .p2_star_timeout_us = UINT64_C(1000000)
    };
    const uint8_t request[] = { 0x10U, 0x03U };
    const uint8_t response_pdu[] = { 0x50U, 0x03U, 0x00U, 0x32U, 0x01U, 0xf4U };
    const uint8_t did_request[] = { 0x22U, 0xf1U, 0x90U };
    MblinkUdsResponse response;

    CHECK(mblink_uds_client_init(&client, &config) == MBLINK_UDS_RESULT_OK);
    CHECK(client.state == MBLINK_UDS_CLIENT_IDLE);
    CHECK(client.active_session == MBLINK_UDS_SESSION_DEFAULT);

    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1000)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(client.state == MBLINK_UDS_CLIENT_WAITING_RESPONSE);
    CHECK(client.deadline_us == UINT64_C(101000));
    CHECK(mblink_uds_client_tick(&client, UINT64_C(100000)) ==
          MBLINK_UDS_RESULT_WAITING);

    CHECK(mblink_uds_client_accept(
              &client, response_pdu, sizeof(response_pdu), UINT64_C(50000),
              &response) == MBLINK_UDS_RESULT_COMPLETE);
    CHECK(client.state == MBLINK_UDS_CLIENT_COMPLETE);
    CHECK(client.active_session == MBLINK_UDS_SESSION_EXTENDED);
    CHECK(client.p2_timeout_us == UINT64_C(50000));
    CHECK(client.p2_star_timeout_us == UINT64_C(5000000));

    CHECK(mblink_uds_client_begin(
              &client, did_request, sizeof(did_request), UINT64_C(200000)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(client.deadline_us == UINT64_C(250000));
    return 0;
}

static int test_client_tester_present(void)
{
    MblinkUdsClient client;
    const MblinkUdsClientConfig config = {
        .p2_timeout_us = UINT64_C(100000),
        .p2_star_timeout_us = UINT64_C(1000000)
    };
    const uint8_t request[] = { 0x3eU, 0x00U };
    const uint8_t suppressed[] = { 0x3eU, 0x80U };
    const uint8_t response_pdu[] = { 0x7eU, 0x00U };
    MblinkUdsResponse response;

    CHECK(mblink_uds_client_init(&client, &config) == MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_begin(
              &client, suppressed, sizeof(suppressed), 0U) ==
          MBLINK_UDS_RESULT_UNSUPPORTED);
    CHECK(client.state == MBLINK_UDS_CLIENT_IDLE);

    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1000)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_accept(
              &client, response_pdu, sizeof(response_pdu), UINT64_C(1001),
              &response) == MBLINK_UDS_RESULT_COMPLETE);
    CHECK(client.state == MBLINK_UDS_CLIENT_COMPLETE);
    return 0;
}

static int test_client_pending_timeout_and_reset(void)
{
    MblinkUdsClient client;
    const MblinkUdsClientConfig config = {
        .p2_timeout_us = UINT64_C(100000),
        .p2_star_timeout_us = UINT64_C(1000000)
    };
    const uint8_t request[] = { 0x22U, 0xf1U, 0x90U };
    const uint8_t pending[] = { 0x7fU, 0x22U, 0x78U };
    MblinkUdsResponse response;

    CHECK(mblink_uds_client_init(&client, &config) == MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1000)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_accept(
              &client, pending, sizeof(pending), UINT64_C(50000), &response) ==
          MBLINK_UDS_RESULT_RESPONSE_PENDING);
    CHECK(client.state == MBLINK_UDS_CLIENT_RESPONSE_PENDING);
    CHECK(client.deadline_us == UINT64_C(1050000));

    CHECK(mblink_uds_client_accept(
              &client, pending, sizeof(pending), UINT64_C(500000), &response) ==
          MBLINK_UDS_RESULT_RESPONSE_PENDING);
    CHECK(client.deadline_us == UINT64_C(1500000));
    CHECK(mblink_uds_client_tick(&client, UINT64_C(1499999)) ==
          MBLINK_UDS_RESULT_WAITING);
    CHECK(mblink_uds_client_tick(&client, UINT64_C(1500000)) ==
          MBLINK_UDS_RESULT_TIMEOUT);
    CHECK(client.state == MBLINK_UDS_CLIENT_FAILED);
    CHECK(client.failure == MBLINK_UDS_RESULT_TIMEOUT);
    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(2000000)) ==
          MBLINK_UDS_RESULT_FAILED_STATE);
    CHECK(mblink_uds_client_accept(
              &client, pending, sizeof(pending), UINT64_C(2000000), &response) ==
          MBLINK_UDS_RESULT_FAILED_STATE);

    client.active_session = MBLINK_UDS_SESSION_EXTENDED;
    client.p2_timeout_us = UINT64_C(1234);
    client.p2_star_timeout_us = UINT64_C(5678);
    mblink_uds_client_reset(&client);
    CHECK(client.state == MBLINK_UDS_CLIENT_IDLE);
    CHECK(client.failure == MBLINK_UDS_RESULT_OK);
    CHECK(client.active_session == MBLINK_UDS_SESSION_DEFAULT);
    CHECK(client.p2_timeout_us == config.p2_timeout_us);
    CHECK(client.p2_star_timeout_us == config.p2_star_timeout_us);
    return 0;
}

static int test_client_negative_reuse_and_wrong_response(void)
{
    MblinkUdsClient client;
    const MblinkUdsClientConfig config = {
        .p2_timeout_us = UINT64_C(100000),
        .p2_star_timeout_us = UINT64_C(1000000)
    };
    const uint8_t request[] = { 0x22U, 0xf1U, 0x90U };
    const uint8_t negative[] = { 0x7fU, 0x22U, 0x31U };
    const uint8_t wrong_did[] = { 0x62U, 0xf1U, 0x91U, 0x00U };
    MblinkUdsResponse response;

    CHECK(mblink_uds_client_init(&client, &config) == MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(0)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_accept(
              &client, negative, sizeof(negative), UINT64_C(1), &response) ==
          MBLINK_UDS_RESULT_NEGATIVE_RESPONSE);
    CHECK(client.state == MBLINK_UDS_CLIENT_COMPLETE);
    CHECK(response.negative_response_code == 0x31U);

    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1000)) ==
          MBLINK_UDS_RESULT_OK);
    memset(&response, 0x5a, sizeof(response));
    {
        MblinkUdsResponse snapshot = response;
        CHECK(mblink_uds_client_accept(
                  &client, wrong_did, sizeof(wrong_did), UINT64_C(1001),
                  &response) == MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
        CHECK(memcmp(&response, &snapshot, sizeof(response)) == 0);
    }
    CHECK(client.state == MBLINK_UDS_CLIENT_FAILED);
    CHECK(client.failure == MBLINK_UDS_RESULT_UNEXPECTED_RESPONSE);
    return 0;
}

static int test_client_busy_suppress_and_deadline(void)
{
    MblinkUdsClient client;
    const MblinkUdsClientConfig config = {
        .p2_timeout_us = UINT64_C(100),
        .p2_star_timeout_us = UINT64_C(1000)
    };
    const uint8_t request[] = { 0x22U, 0x12U, 0x34U };
    const uint8_t session_suppressed[] = { 0x10U, 0x83U };
    const uint8_t response_pdu[] = { 0x62U, 0x12U, 0x34U };
    MblinkUdsResponse response;

    CHECK(mblink_uds_client_init(&client, &config) == MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_begin(
              &client, session_suppressed, sizeof(session_suppressed), 0U) ==
          MBLINK_UDS_RESULT_UNSUPPORTED);
    CHECK(client.state == MBLINK_UDS_CLIENT_IDLE);

    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1000)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1001)) ==
          MBLINK_UDS_RESULT_BUSY);
    CHECK(mblink_uds_client_accept(
              &client, response_pdu, sizeof(response_pdu), UINT64_C(1100),
              &response) == MBLINK_UDS_RESULT_TIMEOUT);
    CHECK(client.state == MBLINK_UDS_CLIENT_FAILED);
    return 0;
}

static int test_deadline_saturates(void)
{
    MblinkUdsClient client;
    const MblinkUdsClientConfig config = {
        .p2_timeout_us = UINT64_MAX,
        .p2_star_timeout_us = UINT64_MAX
    };
    const uint8_t request[] = { 0x22U, 0x12U, 0x34U };

    CHECK(mblink_uds_client_init(&client, &config) == MBLINK_UDS_RESULT_OK);
    CHECK(mblink_uds_client_begin(
              &client, request, sizeof(request), UINT64_C(1000)) ==
          MBLINK_UDS_RESULT_OK);
    CHECK(client.deadline_us == UINT64_MAX);
    CHECK(mblink_uds_client_tick(&client, UINT64_MAX - 1U) ==
          MBLINK_UDS_RESULT_WAITING);
    CHECK(mblink_uds_client_tick(&client, UINT64_MAX) ==
          MBLINK_UDS_RESULT_TIMEOUT);
    return 0;
}

int main(void)
{
    if (test_generic_response_decode() != 0) return 1;
    if (test_session_control() != 0) return 1;
    if (test_tester_present() != 0) return 1;
    if (test_read_did() != 0) return 1;
    if (test_did_definitions() != 0) return 1;
    if (test_client_session_and_timing() != 0) return 1;
    if (test_client_tester_present() != 0) return 1;
    if (test_client_pending_timeout_and_reset() != 0) return 1;
    if (test_client_negative_reuse_and_wrong_response() != 0) return 1;
    if (test_client_busy_suppress_and_deadline() != 0) return 1;
    if (test_deadline_saturates() != 0) return 1;
    return 0;
}
