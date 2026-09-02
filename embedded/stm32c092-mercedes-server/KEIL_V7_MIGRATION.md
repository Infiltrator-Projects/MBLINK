# STM32C092 V6/V7 Keil migration

Use this manifest when moving the reporter's STM32C092RCT6 Cube/Keil project to
current MBLINK.

## Keep the MCU platform

Keep the vendor-generated platform/startup pieces, including GPIO, USART2,
interrupt handlers, CMSIS/startup, STM32 HAL drivers and the existing linker
configuration.

Replace:
- project `Src/main.c` with `Src-main.c` from this directory;
- project `Src/fdcan.c` with `Src-fdcan.c` from this directory.

If CubeMX regeneration is required, use
`FDCAN_Classic_Frame_Networking-MBLINK.ioc`. The old IOC contains nominal
SJW=12; the verified setting is SJW=2.

## Do not retain the old diagnostic stack

Remove the inherited diagnostic application/transport objects from the Keil
target:
- `library/src/endpoint.c`
- `library/src/isotp.c`
- `library/src/uds.c`
- `library/src/uds_did.c`
- `library/src/uds_download.c`
- `library/src/uds_dtc.c`
- `library/src/uds_services.c`
- `stm32c092/can_transport_fdcan.c`
- `stm32c092/uds_app_fdcan.c`
- `stm32c092/uds_platform_fdcan.c`

The supplied fixed-project map measured 30,664 bytes RW/ZI in a 30,720-byte
configured region; `uds_app_fdcan.o` accounted for 29,272 bytes ZI. That old
stack embeds very large ISO-TP/response buffers and is not intended to coexist
with MBLINK/LINK.

## Add MBLINK

Add:
- `src/mercedes/mercedes.c`
- `src/mercedes/vin.c`
- `src/mercedes/server.c`
- `src/embedded/console.c`

## Add the pinned LINK STM32 subset

Add the exact sources from MBLINK's pinned LINK revision:
- `src/link/src/core/isotp.c`
- `src/link/src/uds/uds.c`
- `src/link/src/uds/uds_services.c`
- `src/link/src/uds/uds_server.c`
- `src/link/src/infiltratr-common/src/core.c`
- `src/link/platform/stm32/link-stm32-can.c`
- `src/link/platform/stm32/link-stm32-uds-server.c`
- `src/link/examples/stm32c092/link-stm32c092-hal.c`

MBLINK v0.7.144 pins LINK v0.14.79.

## Bench contract

- 48 MHz FDCAN kernel
- 500 kbit/s Classical CAN
- prescaler 6, TSEG1 13, TSEG2 2, SJW 2
- one FDCAN standard filter element in DUAL mode
- physical request 0x7E0
- functional request 0x7DF
- physical response 0x7E8
- ISO-TP padding 0xCC
- USART2 115200 8-N-1, no flow control
