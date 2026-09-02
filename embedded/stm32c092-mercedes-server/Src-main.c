// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * MBLINK Mercedes-Benz STM32C092 product target.
 *
 * MBLINK owns the Mercedes product/personality. LINK is consumed only as the
 * shared CAN/FDCAN, ISO-TP and UDS library underneath this application.
 */
#include "main.h"
#include "fdcan.h"
#include "gpio.h"
#include "usart.h"

#include "mblink/embedded_console.h"
#include "mblink/mercedes_server.h"

#include "link/uds_server.h"
#include "link/version.h"
#include "link-stm32-can.h"
#include "link-stm32-uds-server.h"
#include "link-stm32c092-hal.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern FDCAN_HandleTypeDef hfdcan1;
extern UART_HandleTypeDef huart2;
void SystemClock_Config(void);
void Error_Handler(void);

#define MBLINK_STM32_PRODUCT_VERSION "0.7.141"
#define MBLINK_STM32_CONSOLE_UART_TIMEOUT_MS UINT32_C(20)
#define MBLINK_STM32_CONSOLE_RX_BUDGET 8U

static const char target_vin[] = "WDD2073031A000001";

static const MblinkUdsDtcRecord target_dtcs[] = {
    { UINT32_C(0x123456), LINK_UDS_DTC_STATUS_TEST_FAILED |
                           LINK_UDS_DTC_STATUS_CONFIRMED_DTC },
    { UINT32_C(0xabcdef), LINK_UDS_DTC_STATUS_CONFIRMED_DTC }
};

/*
 * Deterministic bench data for every ISO 14229-1:2013 0x19 response family.
 * These are demonstration records for the reporter's STM32C092/PCAN
 * qualification target, not claims about a production Mercedes ECU.
 */
static const uint8_t target_snapshot_1[] = {
    0x12U, 0x34U, 0x56U, 0x78U
};
static const uint8_t target_snapshot_2[] = {
    0x12U, 0x35U, 0x9aU
};
static const uint8_t target_stored_1[] = {
    0x22U, 0x01U, 0x55U
};
static const uint8_t target_stored_2[] = {
    0x22U, 0x02U, 0x66U
};
static const uint8_t target_ext_1[] = { 0x05U, 0x09U };
static const uint8_t target_ext_2[] = { 0x03U, 0x08U };

static const LinkUdsServerDtcDetail target_dtc_details[] = {
    {
        UINT32_C(0x123456), 0x20U, 0x01U, 0x20U,
        1U, 1U, true, true, true, 0x33U, 0x01U,
        0x01U, 0x01U, target_snapshot_1, sizeof(target_snapshot_1),
        0x01U, 0x01U, target_stored_1, sizeof(target_stored_1),
        0x01U, target_ext_1, sizeof(target_ext_1)
    },
    {
        UINT32_C(0xabcdef), 0x40U, 0x02U, 0x10U,
        0U, 2U, true, true, false, 0x33U, 0x01U,
        0x01U, 0x01U, target_snapshot_2, sizeof(target_snapshot_2),
        0x01U, 0x01U, target_stored_2, sizeof(target_stored_2),
        0x01U, target_ext_2, sizeof(target_ext_2)
    }
};

static MblinkMercedesServerState mercedes_state;
static LinkStm32C092Hal stm32_hal;
static LinkStm32Can stm32_can;
static LinkUdsServer uds_server;
static LinkStm32UdsServer uds_transport;
static uint8_t uds_rx_storage[512U];
static uint8_t uds_tx_storage[512U];

static bool reset_pending;
static uint8_t reset_type;
static uint32_t reset_requested_ms;

static MblinkEmbeddedConsole engineering_console;
static bool engineering_console_active;

static uint32_t mblink_stm32_clock_ms(void *context)
{
    (void)context;
    return HAL_GetTick();
}

static void mblink_stm32_console_write(
    void *context,
    const char *text,
    size_t length)
{
    uint16_t amount;
    (void)context;
    if (text == NULL || length == 0U) return;

    /*
     * Console output is deliberately bounded and only happens after a user
     * activates USART2. Normal headless CAN/UDS operation emits no UART data.
     */
    while (length != 0U) {
        amount = length > UINT16_MAX ? UINT16_MAX : (uint16_t)length;
        if (HAL_UART_Transmit(
                &huart2, (uint8_t *)(void *)text, amount,
                MBLINK_STM32_CONSOLE_UART_TIMEOUT_MS) != HAL_OK) {
            return;
        }
        text += amount;
        length -= amount;
    }
}

static void mblink_stm32_console_snapshot(
    MblinkEmbeddedConsoleSnapshot *snapshot)
{
    if (snapshot == NULL) return;
    memset(snapshot, 0, sizeof(*snapshot));

    snapshot->product_version = MBLINK_STM32_PRODUCT_VERSION;
    snapshot->link_version = LINK_VERSION_STRING;
    snapshot->vin = mercedes_state.vin;
    snapshot->request_can_id = mercedes_state.endpoint->address.tx_can_id;
    snapshot->response_can_id = mercedes_state.endpoint->address.rx_can_id;
    snapshot->can_online = true;
    snapshot->uds_session = link_uds_server_active_session(&uds_server);
    snapshot->last_service = uds_server.last_service;
    snapshot->last_nrc =
        link_uds_server_last_negative_response_code(&uds_server);
    snapshot->request_count = uds_server.request_count;
    snapshot->positive_response_count = uds_server.positive_response_count;
    snapshot->negative_response_count = uds_server.negative_response_count;
    snapshot->suppressed_response_count = uds_server.suppressed_response_count;
    snapshot->completed_request_count =
        link_stm32_uds_server_completed_requests(&uds_transport);
    snapshot->can_rx_dropped = link_stm32_can_rx_dropped(&stm32_can);
    snapshot->deferred_rx_dropped =
        link_stm32_uds_server_deferred_rx_dropped(&uds_transport);
    snapshot->dtcs = target_dtcs;
    snapshot->dtc_count = sizeof(target_dtcs) / sizeof(target_dtcs[0]);
    snapshot->reset_pending = reset_pending;
    snapshot->reset_type = reset_type;
}

static void mblink_stm32_console_poll(void)
{
    MblinkEmbeddedConsoleSnapshot snapshot;
    uint8_t byte;
    unsigned int index;

    /*
     * Polling with a zero timeout avoids requiring an additional USART IRQ
     * configuration in the reporter's Cube project. Drain only a small bounded
     * number of characters per main-loop pass so CAN/UDS always gets priority.
     */
    for (index = 0U; index < MBLINK_STM32_CONSOLE_RX_BUDGET; ++index) {
        byte = 0U;
        if (HAL_UART_Receive(&huart2, &byte, 1U, 0U) != HAL_OK) return;

        mblink_stm32_console_snapshot(&snapshot);
        if (!engineering_console_active) {
            engineering_console_active = true;
            mblink_embedded_console_print_banner(
                &snapshot, mblink_stm32_console_write, NULL);
        }
        mblink_embedded_console_feed(
            &engineering_console, byte, &snapshot,
            mblink_stm32_console_write, NULL);
    }
}

static bool mblink_stm32_server_init(void)
{
    MblinkMercedesServerConfig mercedes = MBLINK_MERCEDES_SERVER_CONFIG_INIT;
    LinkUdsServerConfig uds_config = LINK_UDS_SERVER_CONFIG_INIT;
    LinkStm32UdsServerConfig transport_config;
    LinkStm32CanOps can_ops;

    mercedes.vin = target_vin;
    mercedes.module = MBLINK_MERCEDES_MODULE_ENGINE;
    mercedes.endpoint_key = "c207-om651-engine-eobd-11bit";
    mercedes.dtcs = target_dtcs;
    mercedes.dtc_count = sizeof(target_dtcs) / sizeof(target_dtcs[0]);
    mercedes.dtc_details = target_dtc_details;
    mercedes.dtc_detail_count =
        sizeof(target_dtc_details) / sizeof(target_dtc_details[0]);
    mercedes.wwh_dtc_format_identifier = UINT8_C(0x04);

    if (!mblink_mercedes_server_init(&mercedes_state, &mercedes))
        return false;

    link_stm32c092_hal_init(&stm32_hal, &hfdcan1, false);
    can_ops = link_stm32c092_hal_ops(&stm32_hal);
    if (!link_stm32_can_init(&stm32_can, &can_ops))
        return false;

    /*
     * MBLINK endpoint definitions are tester-oriented:
     * tx_can_id = tester -> Mercedes ECU, rx_can_id = ECU -> tester.
     * This STM32 target is the ECU/server, so receive the former and transmit
     * the latter.
     */
    if (!link_stm32c092_hal_start_standard(
            &stm32_hal, mercedes_state.endpoint->address.tx_can_id))
        return false;

    uds_config.enforce_session_sequence = true;
    uds_config.s3_server_timeout_ms = UINT32_C(5000);

    /*
     * This bare STM32C092 target can honestly perform processor/system reset
     * for hardReset and softReset. It has no ignition/key-cycle controller and
     * no real rapid-power-shutdown hardware path, so those subfunctions are
     * deliberately not advertised as supported.
     */
    uds_config.supported_ecu_reset_types =
        LINK_UDS_ECU_RESET_SUPPORT_HARD |
        LINK_UDS_ECU_RESET_SUPPORT_SOFT;
    uds_config.rapid_power_shutdown_supported = false;
    uds_config.rapid_power_shutdown_time_seconds = 0U;

    uds_config.clock_ms = mblink_stm32_clock_ms;
    uds_config.clock_context = NULL;
    if (!link_uds_server_init(&uds_server, &uds_config) ||
        !mblink_mercedes_server_bind(&uds_server, &mercedes_state))
        return false;

    memset(&transport_config, 0, sizeof(transport_config));
    transport_config.address.tx_can_id =
        mercedes_state.endpoint->address.rx_can_id;
    transport_config.address.rx_can_id =
        mercedes_state.endpoint->address.tx_can_id;
    transport_config.address.addressing_mode = LINK_ISOTP_ADDRESSING_NORMAL;
    transport_config.address.target_type = LINK_ISOTP_TARGET_PHYSICAL;
    transport_config.rx_block_size = 0U;
    transport_config.rx_stmin = 0U;
    transport_config.consecutive_timeout_us = UINT64_C(1000000);
    transport_config.flow_control_timeout_us = UINT64_C(1000000);
    transport_config.max_wait_frames = 3U;
    transport_config.can_fd = false;
    transport_config.data_length = 8U;
    transport_config.pad_short_frames = true;
    transport_config.padding_byte = UINT8_C(0xcc);

    reset_pending = false;
    reset_type = 0U;
    reset_requested_ms = 0U;
    mblink_embedded_console_init(&engineering_console);
    engineering_console_active = false;

    return link_stm32_uds_server_init(
        &uds_transport, &stm32_can, &uds_server, &transport_config,
        uds_rx_storage, sizeof(uds_rx_storage),
        uds_tx_storage, sizeof(uds_tx_storage));
}

static void mblink_stm32_process(void)
{
    LinkStm32UdsServerResult result;
    uint8_t requested_reset = 0U;

    /*
     * IRQ is the normal path. This main-loop drain is deliberate: the
     * reporter's STM32C092/PCAN testing showed that relying on the RX callback
     * alone could lose an otherwise valid request when the board/IRQ path was
     * not delivered as expected.
     */
    link_stm32_can_rx_isr(&stm32_can);
    link_uds_server_tick(&uds_server);
    result = link_stm32_uds_server_poll(&uds_transport);

    if (result == LINK_STM32_UDS_SERVER_RESULT_REQUEST_COMPLETE &&
        link_uds_server_take_pending_ecu_reset(
            &uds_server, &requested_reset)) {
        switch (requested_reset) {
        case LINK_UDS_ECU_RESET_HARD:
        case LINK_UDS_ECU_RESET_SOFT:
            reset_type = requested_reset;
            reset_requested_ms = HAL_GetTick();
            reset_pending = true;
            break;
        default:
            /*
             * Capability filtering in LINK should make this unreachable.
             * Never turn an unsupported key-cycle/rapid-shutdown request into
             * an accidental MCU reset.
             */
            reset_pending = false;
            break;
        }
    }

    /*
     * 0x11 requires the positive 0x51 response to get onto the bus before the
     * MCU resets. LINK owns the UDS state; this product target owns the actual
     * platform reset primitive.
     */
    if (reset_pending &&
        (uint32_t)(HAL_GetTick() - reset_requested_ms) >= UINT32_C(50)) {
        (void)reset_type;
        NVIC_SystemReset();
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_FDCAN1_Init();
    MX_USART2_UART_Init();

    if (!mblink_stm32_server_init())
        Error_Handler();

    for (;;) {
        mblink_stm32_process();
        mblink_stm32_console_poll();
    }
}

void HAL_FDCAN_RxFifo0Callback(
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t RxFifo0ITs)
{
    if (hfdcan == &hfdcan1 &&
        (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0U) {
        link_stm32_can_rx_isr(&stm32_can);
    }
}

void HAL_FDCAN_TxEventFifoCallback(
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t TxEventFifoITs)
{
    if (hfdcan == &hfdcan1)
        link_stm32c092_hal_tx_event_irq(&stm32_hal, TxEventFifoITs);
}


/*
 * Clock/error glue retained from the reporter's supplied STM32C092 Cube
 * project. These definitions lived in its original Src/main.c, so a replacement
 * MBLINK application must carry them rather than merely calling them.
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    for (;;) {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
