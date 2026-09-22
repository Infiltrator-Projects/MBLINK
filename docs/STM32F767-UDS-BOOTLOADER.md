# STM32F767 UDS OTA Bootloader Integration

MBLINK issue #56 asked for the standard UDS OTA flow on STM32F767. MBLINK now
exposes LINK's fail-closed bootloader core through
`include/mblink/uds_bootloader.h`.

The core follows the requested flow exactly at the state-machine boundary:

- 0x10/0x02 Programming Session
- 0x27 Security Access
- 0x85 DTC recording disable
- 0x28 communication quiesce
- 0x34 RequestDownload into an inactive A/B slot
- 0x36 TransferData with strict sequence and byte-count enforcement
- 0x37 RequestTransferExit only after the declared image size arrives
- 0x31 CheckMemory through integrity and authenticity callbacks
- 0x11 reset staging only after verification and secure-boot candidate checks
- post-boot confirmation before the monotonic anti-rollback version is committed

The STM32F767 transport already belongs to LINK's bxCAN family support. The
bootloader core deliberately does not hard-code STM32F767 flash sectors,
option-byte writes or security secrets. Those operations are supplied through
the target backend and the default configuration is locked:

`allow_programming = false`

A production F767 integration therefore has to provide an inactive-slot
selector, erase/write backend, integrity verifier, authenticity/HSM verifier,
secure-boot candidate validation and protected monotonic-version storage before
the core can arm.

This keeps the full OTA control flow in shared tested code while ensuring that
linking MBLINK alone cannot accidentally enable ECU reprogramming.
