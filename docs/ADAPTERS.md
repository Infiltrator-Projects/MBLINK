<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Adapter Strategy

MBLINK begins with the **Vgate iCar Pro BLE 4.0**, but no physical adapter is allowed to define the diagnostic architecture.

## Responsibility split

Adapter communication crosses two boundaries:

1. `libmblink` owns logical ELM327 command/response behaviour in portable C.
2. A platform provider owns the physical BLE/GATT connection.

For iPhone, the platform provider is intended to be a small Objective-C/CoreBluetooth implementation of the public C transport interface.

The BLE provider does not decode PIDs, DTCs, ISO-TP, UDS or Mercedes-Benz data.

## Primary development adapter

**Vgate iCar Pro BLE 4.0** is the first validation target because it provides BLE suitable for iPhone development while remaining materially more affordable than premium adapters.

This is a development target, not a permanent hard dependency.

## Runtime discovery

MBLINK must not assume that every ELM327-compatible BLE adapter exposes identical UUIDs or buffering behaviour.

The Apple provider should:

1. scan for BLE peripherals;
2. connect to the selected peripheral;
3. discover GATT services;
4. discover characteristics;
5. identify usable write and notification/indication paths;
6. respect the negotiated maximum write size;
7. expose observed capabilities through platform-neutral status/capability data;
8. use an explicit adapter profile only where automatic discovery is insufficient.

No UUID is promoted to a built-in Vgate profile until it is verified against the actual purchased hardware.

## ELM327 compatibility

Products described as ELM327-compatible vary considerably. Capability probing is therefore preferred to assumptions.

Initial logical adapter setup may exercise commands such as:

```text
ATZ
ATE0
ATL0
ATS0
ATH0
ATSP0
ATI
```

Exact sequencing and optional commands are refined through hardware testing and captured-session fixtures. An unsupported optional command must not disable otherwise working standard functionality.

## Capability model

A platform/adapter profile may record:

- display identity;
- observed BLE service/characteristic UUIDs;
- write-with/without-response behaviour;
- maximum write behaviour;
- notification framing;
- reported ELM/ST identity;
- supported AT commands;
- supported OBD protocols;
- CAN-header capabilities;
- tested advanced diagnostic capabilities;
- known firmware quirks.

Observed behaviour takes precedence over marketing labels.

## C transport boundary

The public C transport ABI carries byte/command data and platform-neutral status. The implementation may be CoreBluetooth, another native transport or a deterministic test mock.

The diagnostic core must not import Apple framework types.

## Cheap clone adapters

MBLINK may eventually support inexpensive generic BLE clones, but support is earned by testing. Clone hardware can differ in firmware, command support, BLE implementation and buffering.

MBLINK should report degraded/unsupported capability clearly rather than claim blanket compatibility with every product carrying an ELM327 label.

## Development rule

Adapter quirks belong in the provider/profile. Vehicle and protocol code must remain unchanged when another adapter implements the same transport contract.
