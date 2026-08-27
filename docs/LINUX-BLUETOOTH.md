<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
# Linux diagnostic adapters

Linux adapter discovery and transport are owned by LINK and shared with JAGLINK. MBLINK does not maintain a separate Vgate or Tactrix implementation.

The adapter chooser supports:

- ordinary ELM-compatible `/dev/ttyUSB*` and `/dev/ttyACM*` devices;
- existing `/dev/rfcomm*` ELM-compatible devices;
- native BlueZ BLE/GATT ELM-compatible adapters;
- native BlueZ Bluetooth Classic/SPP ELM-compatible adapters; and
- native USB Tactrix OpenPort 2.0.

For a Vgate iCar Pro dual-mode adapter, press **Refresh**, select the actual BLE or Classic entry exposed by the adapter, then press **LINK UP**. BLE uses BlueZ D-Bus/GATT directly; Classic uses SDP/RFCOMM directly. The advertising name is not used to hard-code an operating-system path.

A connected Tactrix appears as:

`OP2:Tactrix OpenPort 2.0`

LINK drives it directly through libusb. No Wine, Windows Tactrix DLL or separately installed Linux J2534 package is required. The current native bridge covers the CAN/ISO-15765 modes used by present C207 diagnostics, including 11/29-bit addressing, 500/250 kbit/s, functional OBD responders, directed requests, flow control and response-pending handling. K-line is not yet exposed by the LINK Linux bridge and must not be implied by MBLINK.

The package installs LINK's shared OpenPort udev rule for USB VID:PID `0403:cc4d` using `uaccess`, group `dialout` and mode `0660`. Replug the Tactrix after package installation if permissions were established while it was already connected.

The generic transport details and source-of-truth behaviour are documented in LINK's `docs/LINUX-BLUETOOTH.md`; this file records only the MBLINK-facing usage boundary.
