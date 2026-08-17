# Adapter Strategy

MBLINK is designed around **ELM327-compatible Bluetooth Low Energy adapters**, with the **Vgate iCar Pro BLE 4.0** as the first hardware target.

The project intentionally separates adapter transport from vehicle diagnostics. An adapter is a communication bridge; it must not define how the rest of MBLINK is structured.

## Primary development adapter

**Vgate iCar Pro BLE 4.0**

Reasons for choosing it as the first target:

- BLE support suitable for native iPhone development with CoreBluetooth.
- ELM327-compatible command interface.
- Significantly lower cost than premium OBD adapters in Australia.
- Appropriate capability for standard OBD-II development and initial Mercedes investigation.

The Vgate is the first target, not the only supported adapter MBLINK should ever use.

## BLE discovery

MBLINK must not assume that every adapter exposes the same BLE UUIDs.

The transport layer should:

1. Scan for nearby BLE peripherals.
2. Connect to the selected adapter.
3. Discover GATT services.
4. Discover characteristics for those services.
5. Identify writable and notification/indication characteristics suitable for the ELM327 command stream.
6. Record the detected adapter profile for future reconnects.
7. Fall back to known adapter-specific profiles when automatic discovery is insufficient.

The diagnostic core receives complete command responses and does not need to know which GATT characteristics were used.

## ELM327 compatibility

"ELM327-compatible" hardware varies widely. MBLINK therefore treats capabilities as discoverable rather than guaranteed.

During initialisation the adapter layer may probe commands such as:

```text
ATZ
ATE0
ATL0
ATS0
ATH0
ATSP0
ATI
```

Exact initialisation will be refined during hardware testing. Unsupported adapter commands must fail cleanly rather than preventing use of standard functionality that still works.

## Adapter capabilities

An adapter profile may eventually record:

- Display name and identifier.
- BLE service and characteristic UUIDs.
- Maximum write behaviour.
- Notification framing behaviour.
- Reported ELM/ST identity.
- Supported AT commands.
- Supported OBD protocols.
- CAN-header features.
- Known firmware quirks.
- Whether particular advanced Mercedes functions have been tested.

Capability data should describe observed behaviour, not make assumptions from a product name alone.

## Transport contract

All adapter implementations expose the same conceptual operations to `MBLINKCore`:

```text
connect()
disconnect()
send(command)
receive(response)
state
capabilities
```

The concrete Swift API may evolve, but the separation must remain.

This means adding another BLE adapter should not require changes to PID formulas, DTC decoding, ISO-TP, UDS or Mercedes data definitions.

## Cheap clone adapters

MBLINK may eventually work with inexpensive generic ELM327 BLE clones, but they are not the initial validation target. Clone hardware can differ in firmware behaviour, command support, BLE implementation and buffering.

Support should be based on actual testing and capability probing rather than a blanket claim that every product labelled "ELM327" will work.

## Future adapters

Future transport implementations may include:

- Additional BLE ELM327-compatible devices.
- Premium adapters if useful for comparison or advanced capabilities.
- Wi-Fi adapters.
- Other diagnostic interfaces where a clean command transport can be provided.

No future adapter should become a hard dependency of the diagnostic core.

## Development rule

If an adapter requires special handling, put that handling in its transport/profile. Do not contaminate generic OBD-II or Mercedes diagnostic code with Bluetooth-device quirks.
