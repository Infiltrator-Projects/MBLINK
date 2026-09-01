// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_server.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_c207_om651_server_profile(void)
{
    static const MblinkUdsDtcRecord dtcs[] = {
        { UINT32_C(0x123456), LINK_UDS_DTC_STATUS_TEST_FAILED |
                               LINK_UDS_DTC_STATUS_CONFIRMED_DTC },
        { UINT32_C(0xabcdef), LINK_UDS_DTC_STATUS_CONFIRMED_DTC }
    };
    MblinkMercedesServerConfig config = MBLINK_MERCEDES_SERVER_CONFIG_INIT;
    MblinkMercedesServerState state;
    LinkUdsServer server;
    LinkUdsServerConfig server_config = LINK_UDS_SERVER_CONFIG_INIT;
    uint8_t response[64U];
    size_t response_length = 0U;
    static const uint8_t vin_request[] = { 0x22U, 0xf1U, 0x90U };
    static const uint8_t dtc_request[] = { 0x19U, 0x02U, 0xffU };
    const char *vin = "WDD2073031A000001";

    config.vin = vin;
    config.module = MBLINK_MERCEDES_MODULE_ENGINE;
    config.endpoint_key = "c207-om651-engine-eobd-11bit";
    config.dtcs = dtcs;
    config.dtc_count = sizeof(dtcs) / sizeof(dtcs[0]);

    CHECK(mblink_mercedes_server_init(&state, &config));
    CHECK(state.profile == mblink_mercedes_c207_om651_profile());
    CHECK(state.endpoint != NULL);
    CHECK(state.endpoint->address.tx_can_id == UINT32_C(0x7e0));
    CHECK(state.endpoint->address.rx_can_id == UINT32_C(0x7e8));
    CHECK(strcmp(state.vin, vin) == 0);

    CHECK(link_uds_server_init(&server, &server_config));
    CHECK(mblink_mercedes_server_bind(&server, &state));

    CHECK(link_uds_server_handle(
              &server, vin_request, sizeof(vin_request),
              response, sizeof(response), &response_length) ==
          LINK_UDS_SERVER_RESULT_POSITIVE);
    CHECK(response_length == 20U);
    CHECK(response[0] == 0x62U);
    CHECK(response[1] == 0xf1U && response[2] == 0x90U);
    CHECK(memcmp(response + 3U, vin, MBLINK_MERCEDES_VIN_LENGTH) == 0);

    CHECK(link_uds_server_handle(
              &server, dtc_request, sizeof(dtc_request),
              response, sizeof(response), &response_length) ==
          LINK_UDS_SERVER_RESULT_POSITIVE);
    CHECK(response_length == 11U);
    CHECK(response[0] == 0x59U);
    CHECK(response[1] == 0x02U);
    CHECK(response[2] == LINK_UDS_DTC_STATUS_MASK_ALL);
    CHECK(response[3] == 0x12U && response[4] == 0x34U &&
          response[5] == 0x56U);
    CHECK(response[7] == 0xabU && response[8] == 0xcdU &&
          response[9] == 0xefU);
    return 0;
}

static int test_rejects_non_mercedes_identity(void)
{
    MblinkMercedesServerConfig config = MBLINK_MERCEDES_SERVER_CONFIG_INIT;
    MblinkMercedesServerState state;

    config.vin = "1HGCM82633A004352";
    CHECK(!mblink_mercedes_server_init(&state, &config));
    return 0;
}

int main(void)
{
    if (test_c207_om651_server_profile() != 0) return 1;
    if (test_rejects_non_mercedes_identity() != 0) return 1;
    puts("mblink mercedes server tests passed");
    return 0;
}
