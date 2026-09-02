# STM32C092 Mercedes product target

This directory is the **MBLINK STM32C092 Mercedes target**.

MBLINK is the product. LINK is the shared library layer underneath it. A user
who wants to run the STM32C092 work against a Mercedes uses MBLINK and MBLINK
pulls its pinned LINK revision for CAN/FDCAN, ISO-TP and UDS.

Do not build LINK's demo server as the Mercedes product.

## Ownership

MBLINK owns:

- Mercedes VIN/profile selection;
- Mercedes ECU/module endpoint selection;
- Mercedes DID policy and DTC state;
- the STM32C092 Mercedes application entry point;
- the target's 500 kbit/s FDCAN setup;
- the actual MCU reset action after UDS 0x11 has replied.

LINK owns:

- FDCAN adapter primitives;
- bounded CAN RX queues;
- Classical CAN/CAN FD framing;
- ISO-TP;
- UDS dispatch/session handling;
- response segmentation and Flow Control;
- burst-request retention and transport regression tests.

The reporter's STM32C092/PCAN testing is used to improve those two layers in
their proper places; her submitted project is test evidence, not a separate
product fork.

## Files

The product-side target files are:

```text
embedded/stm32c092-mercedes-server/Src-main.c
embedded/stm32c092-mercedes-server/Src-fdcan.c
```

Use those with the Cube-generated startup/clock/GPIO files for the target.

Compile the MBLINK Mercedes product sources:

```text
src/mercedes/mercedes.c
src/mercedes/vin.c
src/mercedes/server.c
```

and the pinned LINK library sources required by the STM32 transport:

```text
src/link/src/core/isotp.c
src/link/src/uds/uds.c
src/link/src/uds/uds_services.c
src/link/src/uds/uds_server.c
src/link/src/infiltratr-common/src/core.c
src/link/platform/stm32/link-stm32-can.c
src/link/platform/stm32/link-stm32-uds-server.c
src/link/examples/stm32c092/link-stm32c092-hal.c
```

Notice that MBLINK no longer uses
`link-stm32c092-server-example.c` as its application. That file is a LINK
library example only. The Mercedes application now constructs the generic LINK
server/transport itself and binds the MBLINK Mercedes handlers.

## Reporter-derived hardware corrections

The product target incorporates the useful STM32C092/PCAN findings:

- for a 48 MHz FDCAN kernel clock and 500 kbit/s Classical CAN:
  `Prescaler=6, TSEG1=13, TSEG2=2, SJW=2`;
- the RX interrupt remains the normal path, but the main loop also drains the
  bounded LINK RX queue as a fallback;
- short Classical CAN transport frames use LINK's configured `0xCC` padding;
- FDCAN TX completion comes from the TX event FIFO;
- ordinary tester requests racing a segmented response are retained by LINK's
  bounded deferred FIFO while Flow Control is handled immediately;
- ECUReset `0x11` is capability-driven:
  - `0x01 hardReset` and `0x03 softReset` are supported on this bare target
    and execute only after the positive response has drained;
  - `0x02 keyOffOnReset` is not advertised because the board has no ignition
    or power-cycle controller;
  - `0x04 enableRapidPowerShutDown` and `0x05 disableRapidPowerShutDown`
    are not advertised because the reporter project has no real rapid-power
    shutdown path;
  - unsupported reset types return `7F 11 12` instead of being incorrectly
    converted into `NVIC_SystemReset()`.

## Mercedes endpoint

The demonstration target selects the existing C207/OM651 engine endpoint:

```text
tester -> ECU request : 0x7E0
ECU -> tester response: 0x7E8
```

The VIN and two DTC records in `Src-main.c` are explicit bench/demo data, not
claims about the reporter's vehicle. Production firmware must supply the real
vehicle/ECU data providers.

## PCAN checks

On CAN ID `0x7E0`:

```text
02 10 01 CC CC CC CC CC
03 19 02 FF CC CC CC CC
03 22 F1 90 CC CC CC CC
```

The `19 02 FF` response is multi-frame with the two demo DTCs, so the tester
must send Flow Control after the First Frame. LINK owns that ISO-TP exchange;
MBLINK owns the Mercedes data returned through it.
