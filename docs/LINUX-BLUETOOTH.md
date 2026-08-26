<!-- SPDX-License-Identifier: GPL-3.0-or-later -->
# Linux Bluetooth and Vgate iCar Pro

MBLINK 0.7.45 uses LINK 0.14.0's native Linux BlueZ transport. The Linux application now discovers four ELM-compatible connection classes in the same Adapter chooser:

- USB serial adapters exposed as `/dev/ttyUSB*` or `/dev/ttyACM*`.
- Classic Bluetooth SPP adapters exposed through `/dev/rfcomm*`.
- Native Bluetooth LE devices discovered through the BlueZ system D-Bus.

For BLE, no RFCOMM bridge is needed. Press **Refresh**, select the entry beginning with `BLE:` for the Vgate iCar Pro, then press **LINK UP**. LINK refreshes the LE bearer, connects through BlueZ, waits for GATT services, finds a writable/notifiable characteristic pair, enables notifications and performs an `ATI` handshake. Only an adapter that returns a complete ELM-style response is accepted. MBLINK then uses the same ELM327 parser, OBD-II flow and read-only Mercedes factory diagnostic extension as the serial path.

The BLE provider does not hard-code Vgate UUIDs. It supports both common split write/notify services and single UART-style characteristics that expose both capabilities. Writes are chunked at 20 bytes so longer text commands remain valid at the default ATT MTU.

The first hardware validation target is the Vgate iCar Pro BLE 4.0 / dual-mode adapter. If the same unit exposes Classic Bluetooth SPP on a Linux host, the existing `/dev/rfcomm*` route remains available as a second transport path.
