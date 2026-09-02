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

#include "mblink/mercedes_server.h"

#include "link/uds_server.h"
#include "link-stm32-can.h"
#include "link-stm32-uds-server.h"
#include "link-stm32c092-hal.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern FDCAN_HandleTypeDef hfdcan1;

static const char target_vin[] = "WDD2073031A000001";

static const MblinkUdsDtcRecord target_dtcs[] = {
    { UINT32_C(0x123456), LINK_UDS_DTC_STATUS_TEST_FAILED |
                           LINK_UDS_DTC_STATUS_CONFIRMED_DTC },
    { UINT32_C(0xabcdef), LINK_UDS_DTC_STATUS_CONFIRMED_DTC }
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

static uint32_t mblink_stm32_clock_ms(void *context)
{
    (void)context;
    return HAL_GetTick();
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
        reset_type = requested_reset;
        reset_requested_ms = HAL_GetTick();
        reset_pending = true;
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
    MX_USART1_UART_Init();

    if (!mblink_stm32_server_init())
        Error_Handler();

    for (;;) {
        mblink_stm32_process();
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
