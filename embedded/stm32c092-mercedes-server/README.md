# STM32C092 Mercedes ECU/server application

This directory is the MBLINK half of the STM32C092 work requested in MBLINK
issues #27 and #38.

The architecture is intentionally split:

- **LINK** owns FDCAN, Classical CAN/CAN FD, ISO-TP, extended FF_DL,
  allocation-free UDS server dispatch and the portable STM32C092 transport.
- **MBLINK** owns Mercedes-Benz VIN/profile selection, Mercedes ECU endpoint
  selection, DID policy, DTC state and the target application's ECU personality.

The generic LINK server has now been physically confirmed by the reporter in
LINK issue #18 ("link-stm32c092-server-example.c is working!!!!"). This MBLINK
layer therefore does not fork or duplicate the working transport.

## Build relationship

Clone MBLINK recursively. Its `src/link` gitlink is the exact LINK revision
used by this project.

For the STM32C092 KEIL/Cube target, keep the Cube-generated clock/GPIO/FDCAN
and startup files from the hardware project and compile these project-owned
pieces:

From LINK:

```text
src/link/src/core/isotp.c
src/link/src/uds/uds.c
src/link/src/uds/uds_services.c
src/link/src/uds/uds_server.c
src/link/platform/stm32/link-stm32-can.c
src/link/platform/stm32/link-stm32-uds-server.c
src/link/examples/stm32c092/link-stm32c092-hal.c
src/link/examples/stm32c092/link-stm32c092-server-example.c
```

From MBLINK:

```text
src/mercedes/mercedes.c
src/mercedes/vin.c
src/mercedes/server.c
embedded/stm32c092-mercedes-server/Src-main.c
```

The normal MBLINK headers and LINK include directories must be on the include
path.

## Mercedes behaviour

`MblinkMercedesServerState` validates the VIN as Mercedes-Benz, selects the
narrowest MBLINK vehicle profile justified by that VIN, and selects the
Mercedes ECU endpoint for the requested module.

The server binds:

- `0x22 ReadDataByIdentifier`: standardized `F190` returns the configured
  target Mercedes VIN. Other Mercedes DIDs are only served when MBLINK has a
  definition for that DID and the application supplies live bytes; MBLINK does
  not invent proprietary values.
- `0x19 ReadDTCInformation`: delegates all LINK-supported report types to the
  target Mercedes DTC store.

LINK continues to provide the built-in `0x10 DiagnosticSessionControl` and
`0x3E TesterPresent` behaviour and the generic callback mechanism for the
other UDS services.

## CAN direction

MBLINK's Mercedes endpoint definitions are tester-oriented:

```text
endpoint.address.tx_can_id  = tester -> ECU request
endpoint.address.rx_can_id  = ECU -> tester response
```

For the C207/OM651 profile currently captured in MBLINK that is `0x7E0` /
`0x7E8`. Other Mercedes profiles can select different endpoints without
changing LINK.

## About the example identity

`Src-main.c` contains an explicitly labelled demonstration C207/OM651 VIN
and two demonstration DTCs so the application is self-testing. They are not
claims about the reporter's vehicle. Replace them with the actual target ECU
identity/DTC provider for production firmware.

The purpose of keeping this application in MBLINK is exactly to keep
Mercedes-specific behaviour out of LINK while retaining one tested UDS/ISO-TP
implementation.
