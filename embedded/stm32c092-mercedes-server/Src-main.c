// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * MBLINK Mercedes-Benz STM32C092 ECU/server integration.
 *
 * Keep the Cube-generated peripheral/startup files from the target project.
 * LINK supplies the working STM32 transport; MBLINK supplies Mercedes state.
 */
#include "main.h"
#include "fdcan.h"
#include "gpio.h"
#include "usart.h"

#include "mblink/mercedes_server.h"
#include "link-stm32c092-server-example.h"

extern FDCAN_HandleTypeDef hfdcan1;

/*
 * Demonstration identity only. Replace with the target Mercedes VIN/FIN.
 * This deliberately selects MBLINK's C207/OM651 profile so the application
 * exercises a Mercedes-specific endpoint instead of LINK's generic demo.
 */
static const char target_vin[] = "WDD2073031A000001";

static const MblinkUdsDtcRecord target_dtcs[] = {
    { UINT32_C(0x123456), LINK_UDS_DTC_STATUS_TEST_FAILED |
                           LINK_UDS_DTC_STATUS_CONFIRMED_DTC },
    { UINT32_C(0xabcdef), LINK_UDS_DTC_STATUS_CONFIRMED_DTC }
};

static MblinkMercedesServerState mercedes_state;

int main(void)
{
    MblinkMercedesServerConfig mercedes =
        MBLINK_MERCEDES_SERVER_CONFIG_INIT;
    LinkStm32C092ServerConfig transport =
        LINK_STM32C092_SERVER_CONFIG_INIT;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_FDCAN1_Init();
    MX_USART1_UART_Init();

    mercedes.vin = target_vin;
    mercedes.module = MBLINK_MERCEDES_MODULE_ENGINE;
    mercedes.endpoint_key = "c207-om651-engine-eobd-11bit";
    mercedes.dtcs = target_dtcs;
    mercedes.dtc_count = sizeof(target_dtcs) / sizeof(target_dtcs[0]);

    if (!mblink_mercedes_server_init(&mercedes_state, &mercedes))
        Error_Handler();

    transport.request_can_id = mercedes_state.endpoint->address.tx_can_id;
    transport.response_can_id = mercedes_state.endpoint->address.rx_can_id;
    transport.can_fd = false;
    transport.data_length = 8U;

    if (!link_stm32c092_server_example_init(&hfdcan1, &transport))
        Error_Handler();

    if (!link_stm32c092_server_example_set_handler(
            LINK_UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
            mblink_mercedes_server_read_did_handler,
            &mercedes_state) ||
        !link_stm32c092_server_example_set_handler(
            LINK_UDS_SERVICE_READ_DTC_INFORMATION,
            mblink_mercedes_server_read_dtc_handler,
            &mercedes_state)) {
        Error_Handler();
    }

    for (;;) {
        link_stm32c092_server_example_poll_rx(&hfdcan1);
        link_stm32c092_server_example_process();
    }
}

void HAL_FDCAN_RxFifo0Callback(
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0U)
        link_stm32c092_server_example_rx_fifo0_irq(hfdcan);
}

void HAL_FDCAN_TxEventFifoCallback(
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t TxEventFifoITs)
{
    link_stm32c092_server_example_tx_event_irq(hfdcan, TxEventFifoITs);
}
