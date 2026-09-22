// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/uds_authentication.h"

#include <stdio.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) {     fprintf(stderr, "CHECK failed %s:%d: %s\n", __FILE__, __LINE__, #c);     return 1; } } while (0)

static int test_all_nine_authentication_tasks(void)
{
    static const uint8_t cert[] = {0x30U,0x01U,0xaaU};
    static const uint8_t challenge[] = {0x10U,0x20U};
    static const uint8_t proof[] = {0x55U,0x66U,0x77U};
    static const uint8_t ephemeral[] = {0x04U,0x99U};
    static const uint8_t additional[] = {0xdeU,0xadU};
    uint8_t algorithm[MBLINK_UDS_AUTH_ALGORITHM_INDICATOR_BYTES];
    uint8_t buffer[128U];
    size_t written = 0U;
    size_t i;

    for (i = 0U; i < sizeof(algorithm); ++i) algorithm[i] = (uint8_t)(i + 1U);

    CHECK(mblink_uds_build_authentication_deauthenticate_request(
              false, buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(written == 2U && buffer[0] == 0x29U &&
          buffer[1] == MBLINK_UDS_AUTH_DEAUTHENTICATE);

    CHECK(mblink_uds_build_authentication_verify_certificate_request(
              MBLINK_UDS_AUTH_VERIFY_CERTIFICATE_UNIDIRECTIONAL,
              false, 0xa5U, cert, sizeof(cert), NULL, 0U,
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_VERIFY_CERTIFICATE_UNIDIRECTIONAL);

    CHECK(mblink_uds_build_authentication_verify_certificate_request(
              MBLINK_UDS_AUTH_VERIFY_CERTIFICATE_BIDIRECTIONAL,
              false, 0x01U, cert, sizeof(cert), challenge, sizeof(challenge),
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_VERIFY_CERTIFICATE_BIDIRECTIONAL);

    CHECK(mblink_uds_build_authentication_proof_of_ownership_request(
              false, proof, sizeof(proof), ephemeral, sizeof(ephemeral),
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_PROOF_OF_OWNERSHIP);

    CHECK(mblink_uds_build_authentication_transmit_certificate_request(
              false, 0x1234U, cert, sizeof(cert),
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_TRANSMIT_CERTIFICATE);

    CHECK(mblink_uds_build_authentication_request_challenge_request(
              false, 0x22U, algorithm,
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_REQUEST_CHALLENGE);

    CHECK(mblink_uds_build_authentication_verify_proof_request(
              MBLINK_UDS_AUTH_VERIFY_PROOF_UNIDIRECTIONAL, false,
              algorithm, proof, sizeof(proof), NULL, 0U,
              additional, sizeof(additional),
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_VERIFY_PROOF_UNIDIRECTIONAL);

    CHECK(mblink_uds_build_authentication_verify_proof_request(
              MBLINK_UDS_AUTH_VERIFY_PROOF_BIDIRECTIONAL, false,
              algorithm, proof, sizeof(proof), challenge, sizeof(challenge),
              additional, sizeof(additional),
              buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(buffer[1] == MBLINK_UDS_AUTH_VERIFY_PROOF_BIDIRECTIONAL);

    CHECK(mblink_uds_build_authentication_configuration_request(
              false, buffer, sizeof(buffer), &written) == MBLINK_UDS_RESULT_OK);
    CHECK(written == 2U && buffer[1] == MBLINK_UDS_AUTH_CONFIGURATION);
    return 0;
}

static int test_response_facade(void)
{
    const uint8_t deauth[] = {0x69U,0x00U,0x10U};
    const uint8_t config[] = {0x69U,0x08U,0x03U};
    const uint8_t malformed[] = {0x69U,0x01U,0x11U,0x00U,0x03U,0xaaU};
    MblinkUdsAuthenticationResponse response;

    CHECK(mblink_uds_decode_authentication_response(
              MBLINK_UDS_AUTH_DEAUTHENTICATE,
              deauth, sizeof(deauth), &response) == MBLINK_UDS_RESULT_OK);
    CHECK(response.return_parameter == MBLINK_UDS_AUTH_RETURN_DEAUTHENTICATED);

    CHECK(mblink_uds_decode_authentication_response(
              MBLINK_UDS_AUTH_CONFIGURATION,
              config, sizeof(config), &response) == MBLINK_UDS_RESULT_OK);
    CHECK(response.return_parameter ==
          MBLINK_UDS_AUTH_RETURN_CONFIGURATION_ACR_ASYMMETRIC);

    CHECK(mblink_uds_decode_authentication_response(
              MBLINK_UDS_AUTH_VERIFY_CERTIFICATE_UNIDIRECTIONAL,
              malformed, sizeof(malformed), &response) ==
          MBLINK_UDS_RESULT_MALFORMED_PDU);
    return 0;
}

int main(void)
{
    CHECK(test_all_nine_authentication_tasks() == 0);
    CHECK(test_response_facade() == 0);
    puts("MBLINK UDS Authentication facade tests passed");
    return 0;
}
