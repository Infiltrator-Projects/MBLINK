// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Regression built from chenyurong22's STM32C092 + PCAN reports and supplied
 * KEIL project. This intentionally runs the Mercedes MBLINK server binding
 * through LINK's STM32 CAN/ISO-TP/UDS transport rather than testing either
 * layer in isolation.
 */
#include "mblink/mercedes_server.h"
#include "mblink/uds_dtc.h"
#include "link-stm32-can.h"
#include "link-stm32-uds-server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do {     if (!(c)) {         fprintf(stderr, "CHECK failed %s:%d: %s\n", __FILE__, __LINE__, #c);         return 1;     } } while (0)

#define MOCK_MAX_FRAMES 512U
#define REPORTER_RACE_REQUEST_COUNT 6U

typedef struct {
    LinkIsoTpCanFrame rx[MOCK_MAX_FRAMES];
    size_t rx_head;
    size_t rx_tail;
    LinkIsoTpCanFrame tx[MOCK_MAX_FRAMES];
    size_t tx_count;
    uint32_t tick_ms;
    bool hw_pending;
} MockCan;

typedef struct {
    MockCan mock;
    LinkStm32Can channel;
    LinkUdsServer uds;
    MblinkMercedesServerState mercedes;
    LinkStm32UdsServer transport;
    uint8_t rx_storage[512U];
    uint8_t tx_storage[512U];
} ReporterFixture;

static const MblinkUdsDtcRecord reporter_dtcs[] = {
    { UINT32_C(0x123456), LINK_UDS_DTC_STATUS_TEST_FAILED |
                           LINK_UDS_DTC_STATUS_CONFIRMED_DTC },
    { UINT32_C(0xabcdef), LINK_UDS_DTC_STATUS_CONFIRMED_DTC }
};

static const uint8_t reporter_snapshot_1[] = {
    0x12U, 0x34U, 0x56U, 0x78U
};
static const uint8_t reporter_snapshot_2[] = {
    0x12U, 0x35U, 0x9aU
};
static const uint8_t reporter_stored_1[] = {
    0x22U, 0x01U, 0x55U
};
static const uint8_t reporter_stored_2[] = {
    0x22U, 0x02U, 0x66U
};
static const uint8_t reporter_ext_1[] = { 0x05U, 0x09U };
static const uint8_t reporter_ext_2[] = { 0x03U, 0x08U };

static const LinkUdsServerDtcDetail reporter_dtc_details[] = {
    {
        UINT32_C(0x123456), 0x20U, 0x01U, 0x20U,
        1U, 1U, true, true, true, 0x33U, 0x01U,
        0x01U, 0x01U, reporter_snapshot_1, sizeof(reporter_snapshot_1),
        0x01U, 0x01U, reporter_stored_1, sizeof(reporter_stored_1),
        0x01U, reporter_ext_1, sizeof(reporter_ext_1)
    },
    {
        UINT32_C(0xabcdef), 0x40U, 0x02U, 0x10U,
        0U, 2U, true, true, false, 0x33U, 0x01U,
        0x01U, 0x01U, reporter_snapshot_2, sizeof(reporter_snapshot_2),
        0x01U, 0x01U, reporter_stored_2, sizeof(reporter_stored_2),
        0x01U, reporter_ext_2, sizeof(reporter_ext_2)
    }
};

static bool mock_receive(void *context, LinkIsoTpCanFrame *frame)
{
    MockCan *mock = (MockCan *)context;
    if (mock->rx_tail == mock->rx_head) return false;
    *frame = mock->rx[mock->rx_tail++ % MOCK_MAX_FRAMES];
    return true;
}

static bool mock_tx_ready(void *context)
{
    MockCan *mock = (MockCan *)context;
    return !mock->hw_pending && mock->tx_count < MOCK_MAX_FRAMES;
}

static bool mock_send(void *context, const LinkIsoTpCanFrame *frame)
{
    MockCan *mock = (MockCan *)context;
    if (mock->hw_pending || mock->tx_count >= MOCK_MAX_FRAMES) return false;
    mock->tx[mock->tx_count++] = *frame;
    mock->hw_pending = true;
    return true;
}

static LinkStm32CanTxStatus mock_tx_status(
    void *context,
    uint32_t *completion_tick_ms)
{
    MockCan *mock = (MockCan *)context;
    if (!mock->hw_pending) return LINK_STM32_CAN_TX_IDLE;
    mock->hw_pending = false;
    if (completion_tick_ms != NULL) *completion_tick_ms = mock->tick_ms;
    return LINK_STM32_CAN_TX_COMPLETE;
}

static uint32_t mock_clock_ms(void *context)
{
    return ((MockCan *)context)->tick_ms;
}

static void mock_push(MockCan *mock, const LinkIsoTpCanFrame *frame)
{
    mock->rx[mock->rx_head++ % MOCK_MAX_FRAMES] = *frame;
}

static void push_pcan_single_frame(
    ReporterFixture *fixture,
    const uint8_t *pdu,
    size_t pdu_length)
{
    LinkIsoTpCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    memset(frame.data, 0xcc, 8U);
    frame.can_id = fixture->mercedes.endpoint->address.tx_can_id;
    frame.length = 8U;
    frame.data[0] = (uint8_t)pdu_length;
    memcpy(frame.data + 1U, pdu, pdu_length);
    mock_push(&fixture->mock, &frame);
    link_stm32_can_rx_isr(&fixture->channel);
}

static void push_pcan_flow_control(ReporterFixture *fixture)
{
    LinkIsoTpCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    memset(frame.data, 0xcc, 8U);
    frame.can_id = fixture->mercedes.endpoint->address.tx_can_id;
    frame.length = 8U;
    frame.data[0] = 0x30U;
    frame.data[1] = 0x00U;
    frame.data[2] = 0x00U;
    mock_push(&fixture->mock, &frame);
    link_stm32_can_rx_isr(&fixture->channel);
}

static int fixture_init(ReporterFixture *fixture)
{
    LinkStm32CanOps ops;
    MblinkMercedesServerConfig mercedes = MBLINK_MERCEDES_SERVER_CONFIG_INIT;
    LinkUdsServerConfig uds_config = LINK_UDS_SERVER_CONFIG_INIT;
    LinkStm32UdsServerConfig transport_config;

    memset(fixture, 0, sizeof(*fixture));
    memset(&ops, 0, sizeof(ops));

    mercedes.vin = "WDD2073031A000001";
    mercedes.module = MBLINK_MERCEDES_MODULE_ENGINE;
    mercedes.endpoint_key = "c207-om651-engine-eobd-11bit";
    mercedes.dtcs = reporter_dtcs;
    mercedes.dtc_count = sizeof(reporter_dtcs) / sizeof(reporter_dtcs[0]);
    mercedes.dtc_details = reporter_dtc_details;
    mercedes.dtc_detail_count =
        sizeof(reporter_dtc_details) / sizeof(reporter_dtc_details[0]);
    mercedes.wwh_dtc_format_identifier = UINT8_C(0x04);
    if (!mblink_mercedes_server_init(&fixture->mercedes, &mercedes)) return 1;

    ops.context = &fixture->mock;
    ops.receive = mock_receive;
    ops.tx_ready = mock_tx_ready;
    ops.send = mock_send;
    ops.tx_status = mock_tx_status;
    ops.clock_ms = mock_clock_ms;
    if (!link_stm32_can_init(&fixture->channel, &ops)) return 1;

    uds_config.enforce_session_sequence = true;
    uds_config.s3_server_timeout_ms = UINT32_C(5000);
    uds_config.clock_ms = mock_clock_ms;
    uds_config.clock_context = &fixture->mock;
    if (!link_uds_server_init(&fixture->uds, &uds_config) ||
        !mblink_mercedes_server_bind(&fixture->uds, &fixture->mercedes)) {
        return 1;
    }

    memset(&transport_config, 0, sizeof(transport_config));
    transport_config.address.tx_can_id =
        fixture->mercedes.endpoint->address.rx_can_id;
    transport_config.address.rx_can_id =
        fixture->mercedes.endpoint->address.tx_can_id;
    transport_config.address.addressing_mode = LINK_ISOTP_ADDRESSING_NORMAL;
    transport_config.address.target_type = LINK_ISOTP_TARGET_PHYSICAL;
    transport_config.consecutive_timeout_us = UINT64_C(1000000);
    transport_config.flow_control_timeout_us = UINT64_C(1000000);
    transport_config.max_wait_frames = 3U;
    transport_config.can_fd = false;
    transport_config.data_length = 8U;
    transport_config.pad_short_frames = true;
    transport_config.padding_byte = 0xccU;

    return link_stm32_uds_server_init(
        &fixture->transport, &fixture->channel, &fixture->uds,
        &transport_config,
        fixture->rx_storage, sizeof(fixture->rx_storage),
        fixture->tx_storage, sizeof(fixture->tx_storage)) ? 0 : 1;
}

static int run_transaction(
    ReporterFixture *fixture,
    const uint8_t *pdu,
    size_t pdu_length,
    size_t *tx_start_out,
    size_t *tx_end_out)
{
    size_t tx_start = fixture->mock.tx_count;
    bool first_frame_seen = false;
    bool flow_control_sent = false;
    unsigned int index;

    if (pdu == NULL || pdu_length == 0U || pdu_length > 7U) return 1;
    push_pcan_single_frame(fixture, pdu, pdu_length);

    for (index = 0U; index < 256U; ++index) {
        LinkStm32UdsServerResult result =
            link_stm32_uds_server_poll(&fixture->transport);

        if (fixture->mock.tx_count > tx_start) {
            const LinkIsoTpCanFrame *first = &fixture->mock.tx[tx_start];
            first_frame_seen = (first->data[0] >> 4U) == 0x1U;
        }

        if (first_frame_seen && !flow_control_sent &&
            !fixture->mock.hw_pending) {
            push_pcan_flow_control(fixture);
            flow_control_sent = true;
        }

        if (result == LINK_STM32_UDS_SERVER_RESULT_REQUEST_COMPLETE) {
            if (tx_start_out != NULL) *tx_start_out = tx_start;
            if (tx_end_out != NULL) *tx_end_out = fixture->mock.tx_count;
            return 0;
        }
        if (result != LINK_STM32_UDS_SERVER_RESULT_OK &&
            result != LINK_STM32_UDS_SERVER_RESULT_WAITING) {
            return 1;
        }
        fixture->mock.tick_ms++;
    }
    return 1;
}

static int reassemble_response(
    const ReporterFixture *fixture,
    size_t tx_start,
    size_t tx_end,
    uint8_t *pdu,
    size_t capacity,
    size_t *pdu_length)
{
    const LinkIsoTpCanFrame *first;
    size_t total;
    size_t copied = 0U;
    size_t frame_index;

    if (fixture == NULL || pdu == NULL || pdu_length == NULL ||
        tx_start >= tx_end) return 1;

    first = &fixture->mock.tx[tx_start];
    CHECK(first->can_id == fixture->mercedes.endpoint->address.rx_can_id);
    if ((first->data[0] >> 4U) == 0x0U) {
        total = (size_t)(first->data[0] & 0x0fU);
        if (total > capacity || total + 1U > first->length) return 1;
        memcpy(pdu, first->data + 1U, total);
        *pdu_length = total;
        return 0;
    }

    if ((first->data[0] >> 4U) != 0x1U) return 1;
    total = ((size_t)(first->data[0] & 0x0fU) << 8U) |
            (size_t)first->data[1];
    if (total > capacity) return 1;

    copied = total < 6U ? total : 6U;
    memcpy(pdu, first->data + 2U, copied);
    frame_index = tx_start + 1U;

    while (copied < total && frame_index < tx_end) {
        const LinkIsoTpCanFrame *cf = &fixture->mock.tx[frame_index++];
        size_t amount;
        if ((cf->data[0] >> 4U) != 0x2U) return 1;
        amount = total - copied;
        if (amount > 7U) amount = 7U;
        memcpy(pdu + copied, cf->data + 1U, amount);
        copied += amount;
    }

    if (copied != total) return 1;
    *pdu_length = total;
    return 0;
}

static int test_reporter_single_request_services(void)
{
    ReporterFixture fixture;
    uint8_t response[64U];
    size_t response_length;
    size_t tx_start;
    size_t tx_end;
    static const uint8_t session[] = { 0x10U, 0x01U };
    static const uint8_t tester_present[] = { 0x3eU, 0x00U };
    static const uint8_t vin[] = { 0x22U, 0xf1U, 0x90U };
    static const uint8_t expected_session[] =
        { 0x50U, 0x01U, 0x00U, 0x32U, 0x01U, 0xf4U };
    static const uint8_t expected_tp[] = { 0x7eU, 0x00U };

    CHECK(fixture_init(&fixture) == 0);

    /* Her report: one 10 01 must work; no second/third send should be needed. */
    CHECK(run_transaction(
              &fixture, session, sizeof(session), &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end, response, sizeof(response),
              &response_length) == 0);
    CHECK(response_length == sizeof(expected_session));
    CHECK(memcmp(response, expected_session, sizeof(expected_session)) == 0);

    CHECK(run_transaction(
              &fixture, tester_present, sizeof(tester_present),
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end, response, sizeof(response),
              &response_length) == 0);
    CHECK(response_length == sizeof(expected_tp));
    CHECK(memcmp(response, expected_tp, sizeof(expected_tp)) == 0);

    CHECK(run_transaction(
              &fixture, vin, sizeof(vin), &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end, response, sizeof(response),
              &response_length) == 0);
    CHECK(response_length == 20U);
    CHECK(response[0] == 0x62U && response[1] == 0xf1U &&
          response[2] == 0x90U);
    CHECK(memcmp(
              response + 3U, "WDD2073031A000001",
              MBLINK_MERCEDES_VIN_LENGTH) == 0);
    return 0;
}

static int test_reporter_ecu_reset_through_stm32_transport(void)
{
    ReporterFixture fixture;
    uint8_t response[16U];
    uint8_t pending_reset_type = 0U;
    size_t response_length;
    size_t tx_start;
    size_t tx_end;

    static const uint8_t hard_reset[] = { 0x11U, 0x01U };
    static const uint8_t key_off_on_reset[] = { 0x11U, 0x02U };
    static const uint8_t soft_reset[] = { 0x11U, 0x03U };
    static const uint8_t rapid_enable[] = { 0x11U, 0x04U };
    static const uint8_t rapid_disable[] = { 0x11U, 0x05U };

    CHECK(fixture_init(&fixture) == 0);

    /*
     * The reporter's bare STM32C092 target has an MCU reset primitive, but no
     * ignition/key-cycle controller and no rapid-power-shutdown hardware.
     * MBLINK therefore advertises only the reset modes it can really execute.
     */
    fixture.uds.config.supported_ecu_reset_types =
        LINK_UDS_ECU_RESET_SUPPORT_HARD |
        LINK_UDS_ECU_RESET_SUPPORT_SOFT;
    fixture.uds.config.rapid_power_shutdown_supported = false;

    CHECK(run_transaction(
              &fixture, hard_reset, sizeof(hard_reset),
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end, response, sizeof(response),
              &response_length) == 0);
    CHECK(response_length == 2U);
    CHECK(response[0] == 0x51U && response[1] == 0x01U);
    CHECK(link_uds_server_take_pending_ecu_reset(
              &fixture.uds, &pending_reset_type));
    CHECK(pending_reset_type == 0x01U);

    CHECK(run_transaction(
              &fixture, soft_reset, sizeof(soft_reset),
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end, response, sizeof(response),
              &response_length) == 0);
    CHECK(response_length == 2U);
    CHECK(response[0] == 0x51U && response[1] == 0x03U);
    CHECK(link_uds_server_take_pending_ecu_reset(
              &fixture.uds, &pending_reset_type));
    CHECK(pending_reset_type == 0x03U);

    /*
     * Do not lie to PCAN: the target cannot perform a genuine ignition
     * off/on cycle or rapid-power-shutdown state transition, so those
     * subfunctions are explicitly unsupported instead of being mapped to
     * NVIC_SystemReset().
     */
    {
        const uint8_t *unsupported[] = {
            key_off_on_reset, rapid_enable, rapid_disable
        };
        const size_t unsupported_lengths[] = {
            sizeof(key_off_on_reset), sizeof(rapid_enable),
            sizeof(rapid_disable)
        };
        size_t index;

        for (index = 0U; index < 3U; ++index) {
            CHECK(run_transaction(
                      &fixture, unsupported[index], unsupported_lengths[index],
                      &tx_start, &tx_end) == 0);
            CHECK(reassemble_response(
                      &fixture, tx_start, tx_end,
                      response, sizeof(response), &response_length) == 0);
            CHECK(response_length == 3U);
            CHECK(response[0] == 0x7fU && response[1] == 0x11U);
            CHECK(response[2] == LINK_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
            CHECK(!link_uds_server_take_pending_ecu_reset(
                       &fixture.uds, &pending_reset_type));
        }
    }

    return 0;
}

static int test_reporter_all_0x19_shapes_through_stm32_transport(void)
{
    static const uint8_t expected_subfunctions[] = {
        0x01U,0x02U,0x03U,0x04U,0x05U,0x06U,0x07U,0x08U,0x09U,
        0x0aU,0x0bU,0x0cU,0x0dU,0x0eU,0x0fU,0x10U,0x11U,0x12U,
        0x13U,0x14U,0x15U,0x16U,0x17U,0x18U,0x19U,0x42U,0x55U
    };
    static const uint8_t expected_request_lengths[] = {
        3U,3U,2U,6U,3U,6U,4U,4U,5U,
        2U,2U,2U,2U,2U,3U,6U,3U,3U,
        3U,2U,2U,3U,4U,7U,7U,5U,3U
    };
    ReporterFixture fixture;
    size_t index;

    CHECK(fixture_init(&fixture) == 0);
    CHECK(mblink_uds_dtc_report_definition_count() ==
          sizeof(expected_subfunctions) / sizeof(expected_subfunctions[0]));

    for (index = 0U;
         index < sizeof(expected_subfunctions) / sizeof(expected_subfunctions[0]);
         ++index) {
        MblinkUdsDtcInformationRequest request =
            MBLINK_UDS_DTC_INFORMATION_REQUEST_INIT;
        uint8_t request_pdu[8U] = {0U};
        uint8_t response_pdu[128U] = {0U};
        size_t request_length = 0U;
        size_t response_length = 0U;
        size_t tx_start = 0U;
        size_t tx_end = 0U;

        request.subfunction = expected_subfunctions[index];
        request.status_mask = LINK_UDS_DTC_STATUS_MASK_ALL;
        request.severity_mask = UINT8_C(0xff);
        request.dtc = UINT32_C(0x123456);
        request.record_number = UINT8_C(0x01);
        request.memory_selection = UINT8_C(0x01);
        request.functional_group_identifier = UINT8_C(0x33);
        if (expected_subfunctions[index] == 0x42U) {
            request.status_mask = UINT8_C(0x09);
            request.severity_mask = UINT8_C(0x20);
        }

        CHECK(mblink_uds_build_read_dtc_information_request(
                  &request, request_pdu, sizeof(request_pdu),
                  &request_length) == LINK_UDS_RESULT_OK);
        CHECK(request_pdu[0] == 0x19U);
        CHECK(request_pdu[1] == expected_subfunctions[index]);
        CHECK(request_length == expected_request_lengths[index]);
        if (expected_subfunctions[index] == 0x42U) {
            /* ISO 14229-1:2013: functional group, status mask, severity mask. */
            CHECK(request_pdu[2] == 0x33U);
            CHECK(request_pdu[3] == 0x09U);
            CHECK(request_pdu[4] == 0x20U);
        }

        CHECK(run_transaction(
                  &fixture, request_pdu, request_length,
                  &tx_start, &tx_end) == 0);
        CHECK(reassemble_response(
                  &fixture, tx_start, tx_end,
                  response_pdu, sizeof(response_pdu),
                  &response_length) == 0);

        {
            MblinkUdsDtcInformationResponse decoded;
            CHECK(response_length >= 2U);
            CHECK(response_pdu[0] == 0x59U);
            CHECK(response_pdu[1] == expected_subfunctions[index]);
            CHECK(mblink_uds_decode_read_dtc_information_response(
                      expected_subfunctions[index],
                      response_pdu, response_length, &decoded) ==
                  LINK_UDS_RESULT_OK);
        }

        /*
         * Pin the layouts that were missing from the old status-only demo.
         * These checks run after the real STM32 CAN/ISO-TP transport path.
         */
        switch (expected_subfunctions[index]) {
        case 0x03U:
            CHECK(response_length == 10U);
            CHECK(response_pdu[2] == 0x12U &&
                  response_pdu[3] == 0x34U &&
                  response_pdu[4] == 0x56U &&
                  response_pdu[5] == 0x01U);
            break;
        case 0x04U:
            CHECK(response_pdu[2] == 0x12U &&
                  response_pdu[3] == 0x34U &&
                  response_pdu[4] == 0x56U &&
                  response_pdu[5] == 0x09U &&
                  response_pdu[6] == 0x01U &&
                  response_pdu[7] == 0x01U);
            break;
        case 0x05U:
            CHECK(response_pdu[2] == 0x01U &&
                  response_pdu[3] == 0x12U &&
                  response_pdu[4] == 0x34U &&
                  response_pdu[5] == 0x56U &&
                  response_pdu[6] == 0x09U);
            break;
        case 0x06U:
        case 0x10U:
            CHECK(response_pdu[2] == 0x12U &&
                  response_pdu[3] == 0x34U &&
                  response_pdu[4] == 0x56U &&
                  response_pdu[5] == 0x09U &&
                  response_pdu[6] == 0x01U);
            break;
        case 0x08U:
        case 0x09U:
            CHECK(response_pdu[2] == 0xffU);
            CHECK(response_pdu[3] == 0x20U &&
                  response_pdu[4] == 0x01U &&
                  response_pdu[5] == 0x12U &&
                  response_pdu[6] == 0x34U &&
                  response_pdu[7] == 0x56U &&
                  response_pdu[8] == 0x09U);
            break;
        case 0x14U:
            CHECK(response_pdu[2] == 0x12U &&
                  response_pdu[3] == 0x34U &&
                  response_pdu[4] == 0x56U &&
                  response_pdu[5] == 0x20U);
            break;
        case 0x17U:
            CHECK(response_pdu[2] == 0x01U &&
                  response_pdu[3] == 0xffU);
            break;
        case 0x18U:
        case 0x19U:
            CHECK(response_pdu[2] == 0x01U &&
                  response_pdu[3] == 0x12U &&
                  response_pdu[4] == 0x34U &&
                  response_pdu[5] == 0x56U &&
                  response_pdu[6] == 0x09U);
            break;
        case 0x42U:
            CHECK(response_pdu[2] == 0x33U &&
                  response_pdu[3] == 0xffU &&
                  response_pdu[4] == 0xffU &&
                  response_pdu[5] == 0x04U &&
                  response_pdu[6] == 0x20U);
            break;
        case 0x55U:
            CHECK(response_pdu[2] == 0x33U &&
                  response_pdu[3] == 0xffU &&
                  response_pdu[4] == 0x04U);
            break;
        default:
            break;
        }
    }
    return 0;
}

static int test_reporter_0x19_edge_semantics_through_stm32_transport(void)
{
    static const MblinkUdsDtcRecord edge_records[] = {
        { UINT32_C(0x111111), UINT8_C(0x00) },
        { UINT32_C(0x222222), LINK_UDS_DTC_STATUS_CONFIRMED_DTC }
    };
    static const uint8_t obd_ext[] = { UINT8_C(0xaa) };
    static const LinkUdsServerDtcDetail edge_details[] = {
        {
            UINT32_C(0x111111),0x20U,0x01U,0x20U,1U,1U,
            true,true,true,0x33U,0x01U,
            0x01U,0U,NULL,0U,
            0x01U,0U,NULL,0U,
            0x01U,NULL,0U
        },
        {
            UINT32_C(0x222222),0x40U,0x02U,0U,2U,2U,
            false,true,false,0x33U,0x01U,
            0U,0U,NULL,0U,
            0U,0U,NULL,0U,
            0x90U,obd_ext,sizeof(obd_ext)
        }
    };
    static const uint8_t requests[][7] = {
        {0x19U,0x0aU},
        {0x19U,0x15U},
        {0x19U,0x0bU},
        {0x19U,0x04U,0x11U,0x11U,0x11U,0x01U},
        {0x19U,0x05U,0x01U},
        {0x19U,0x16U,0x01U},
        {0x19U,0x06U,0x11U,0x11U,0x11U,0xfeU},
        {0x19U,0x06U,0x22U,0x22U,0x22U,0xfeU}
    };
    static const uint8_t lengths[] = {2U,2U,2U,6U,3U,3U,6U,6U};
    ReporterFixture fixture;
    uint8_t response[96U];
    size_t response_length;
    size_t tx_start;
    size_t tx_end;
    size_t index;

    CHECK(fixture_init(&fixture) == 0);
    fixture.mercedes.dtc_store.records = edge_records;
    fixture.mercedes.dtc_store.record_count =
        sizeof(edge_records) / sizeof(edge_records[0]);
    fixture.mercedes.dtc_store.details = edge_details;
    fixture.mercedes.dtc_store.detail_count =
        sizeof(edge_details) / sizeof(edge_details[0]);

    for (index = 0U; index < 6U; ++index) {
        CHECK(run_transaction(
                  &fixture, requests[index], lengths[index],
                  &tx_start, &tx_end) == 0);
        CHECK(reassemble_response(
                  &fixture, tx_start, tx_end,
                  response, sizeof(response), &response_length) == 0);
        CHECK(response_length >= 2U);
        CHECK(response[0] == 0x59U);
        CHECK(response[1] == requests[index][1]);
    }

    /* reportSupportedDTC must retain a supported DTC at status 0x00. */
    CHECK(run_transaction(
              &fixture, requests[0], lengths[0],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 11U);
    CHECK(response[3] == 0x11U && response[4] == 0x11U &&
          response[5] == 0x11U && response[6] == 0x00U);

    /* 0x15 permanent and 0x0B historical first-failed survive status healing. */
    CHECK(run_transaction(
              &fixture, requests[1], lengths[1],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 7U && response[6] == 0x00U);

    CHECK(run_transaction(
              &fixture, requests[2], lengths[2],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 7U && response[6] == 0x00U);

    /* Supported records with no current payload return positive minimal forms. */
    CHECK(run_transaction(
              &fixture, requests[3], lengths[3],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 6U && response[1] == 0x04U &&
          response[2] == 0x11U && response[5] == 0x00U);

    CHECK(run_transaction(
              &fixture, requests[4], lengths[4],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 3U && response[1] == 0x05U &&
          response[2] == 0x01U);

    CHECK(run_transaction(
              &fixture, requests[5], lengths[5],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 3U && response[1] == 0x16U &&
          response[2] == 0x01U);

    /*
     * 0xFE means OBD extended records (0x90..0xEF). It must not match
     * ordinary record 0x01, but it must match the 0x90 record.
     */
    CHECK(run_transaction(
              &fixture, requests[6], lengths[6],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 3U);
    CHECK(response[0] == 0x7fU && response[1] == 0x19U &&
          response[2] == LINK_UDS_NRC_REQUEST_OUT_OF_RANGE);

    CHECK(run_transaction(
              &fixture, requests[7], lengths[7],
              &tx_start, &tx_end) == 0);
    CHECK(reassemble_response(
              &fixture, tx_start, tx_end,
              response, sizeof(response), &response_length) == 0);
    CHECK(response_length == 8U);
    CHECK(response[0] == 0x59U && response[1] == 0x06U &&
          response[2] == 0x22U && response[3] == 0x22U &&
          response[4] == 0x22U && response[5] == 0x08U &&
          response[6] == 0x90U && response[7] == 0xaaU);

    return 0;
}

static int test_reporter_pcan_burst_during_19_02(void)
{
    ReporterFixture fixture;
    LinkIsoTpCanFrame frame;
    unsigned int index;
    size_t dtc_tx_start;
    const uint8_t request_19_02[] = { 0x19U, 0x02U, 0xffU };
    const uint8_t tester_present[] = { 0x3eU, 0x00U };
    static const uint8_t expected_dtc_ff[] = {
        0x10U, 0x0bU, 0x59U, 0x02U, 0xffU, 0x12U, 0x34U, 0x56U
    };
    static const uint8_t expected_dtc_cf[] = {
        0x21U, 0x09U, 0xabU, 0xcdU, 0xefU, 0x08U, 0xccU, 0xccU
    };
    static const uint8_t expected_tp[] = {
        0x02U, 0x7eU, 0x00U, 0xccU, 0xccU, 0xccU, 0xccU, 0xccU
    };

    CHECK(fixture_init(&fixture) == 0);
    dtc_tx_start = fixture.mock.tx_count;

    push_pcan_single_frame(
        &fixture, request_19_02, sizeof(request_19_02));

    CHECK(link_stm32_uds_server_poll(&fixture.transport) ==
          LINK_STM32_UDS_SERVER_RESULT_WAITING);
    CHECK(fixture.mock.tx_count == dtc_tx_start + 1U);
    CHECK(memcmp(
              fixture.mock.tx[dtc_tx_start].data,
              expected_dtc_ff, sizeof(expected_dtc_ff)) == 0);

    /* Confirm the First Frame really left hardware before PCAN sends FC. */
    CHECK(link_stm32_uds_server_poll(&fixture.transport) ==
          LINK_STM32_UDS_SERVER_RESULT_WAITING);

    /*
     * Her intermittent report: send several requests in a row while the
     * segmented 0x19 response is still active. None may silently disappear.
     */
    for (index = 0U; index < REPORTER_RACE_REQUEST_COUNT; ++index) {
        memset(&frame, 0, sizeof(frame));
        memset(frame.data, 0xcc, 8U);
        frame.can_id = fixture.mercedes.endpoint->address.tx_can_id;
        frame.length = 8U;
        frame.data[0] = (uint8_t)sizeof(tester_present);
        memcpy(frame.data + 1U, tester_present, sizeof(tester_present));
        mock_push(&fixture.mock, &frame);
    }
    push_pcan_flow_control(&fixture);
    link_stm32_can_rx_isr(&fixture.channel);

    for (index = 0U; index < 64U; ++index) {
        LinkStm32UdsServerResult result =
            link_stm32_uds_server_poll(&fixture.transport);
        if (result == LINK_STM32_UDS_SERVER_RESULT_REQUEST_COMPLETE)
            break;
        CHECK(result == LINK_STM32_UDS_SERVER_RESULT_OK ||
              result == LINK_STM32_UDS_SERVER_RESULT_WAITING);
        fixture.mock.tick_ms++;
    }
    CHECK(index < 64U);
    CHECK(fixture.mock.tx_count == dtc_tx_start + 2U);
    CHECK(memcmp(
              fixture.mock.tx[dtc_tx_start + 1U].data,
              expected_dtc_cf, sizeof(expected_dtc_cf)) == 0);
    CHECK(link_stm32_uds_server_deferred_rx_dropped(
              &fixture.transport) == 0U);

    for (index = 0U; index < REPORTER_RACE_REQUEST_COUNT; ++index) {
        unsigned int poll_index;
        const size_t expected_tx_index = dtc_tx_start + 2U + index;
        for (poll_index = 0U; poll_index < 64U; ++poll_index) {
            LinkStm32UdsServerResult result =
                link_stm32_uds_server_poll(&fixture.transport);
            if (result == LINK_STM32_UDS_SERVER_RESULT_REQUEST_COMPLETE)
                break;
            CHECK(result == LINK_STM32_UDS_SERVER_RESULT_OK ||
                  result == LINK_STM32_UDS_SERVER_RESULT_WAITING);
            fixture.mock.tick_ms++;
        }
        CHECK(poll_index < 64U);
        CHECK(fixture.mock.tx_count == expected_tx_index + 1U);
        CHECK(memcmp(
                  fixture.mock.tx[expected_tx_index].data,
                  expected_tp, sizeof(expected_tp)) == 0);
    }
    CHECK(link_stm32_uds_server_deferred_rx_dropped(
              &fixture.transport) == 0U);
    return 0;
}

int main(void)
{
    if (test_reporter_single_request_services() != 0) return EXIT_FAILURE;
    if (test_reporter_ecu_reset_through_stm32_transport() != 0)
        return EXIT_FAILURE;
    if (test_reporter_all_0x19_shapes_through_stm32_transport() != 0)
        return EXIT_FAILURE;
    if (test_reporter_0x19_edge_semantics_through_stm32_transport() != 0)
        return EXIT_FAILURE;
    if (test_reporter_pcan_burst_during_19_02() != 0) return EXIT_FAILURE;
    puts("MBLINK STM32C092 reporter/PCAN regressions passed");
    return EXIT_SUCCESS;
}
