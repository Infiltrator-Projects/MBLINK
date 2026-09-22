// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/uds_bootloader.h"

#include <stdio.h>

#define CHECK(expr) do { if (!(expr)) {     fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__);     return 1; } } while (0)

int main(void)
{
    MblinkUdsBootloader bootloader;
    MblinkUdsBootloaderConfig config = MBLINK_UDS_BOOTLOADER_CONFIG_INIT;

    CHECK(mblink_uds_bootloader_init(&bootloader, &config));
    CHECK(bootloader.state == MBLINK_UDS_BOOTLOADER_STATE_DISARMED);
    CHECK(mblink_uds_bootloader_arm(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_LOCKED);

    /*
     * This is intentional product-level safety coverage: importing MBLINK or
     * LINK cannot make an ECU programmable. A target must explicitly enable
     * programming and supply every required flash/verification backend hook.
     */
    CHECK(!config.allow_programming);
    CHECK(config.require_security_access);
    CHECK(config.require_quiesce);
    CHECK(config.require_authenticity);
    CHECK(config.require_secure_boot_validation);

    puts("MBLINK UDS bootloader facade tests passed");
    return 0;
}
