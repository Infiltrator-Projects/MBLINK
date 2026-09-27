// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_STM32F767_OTA_H
#define MBLINK_STM32F767_OTA_H

#include "mblink/uds_bootloader.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A product must provide its real linker-matched flash layout and HAL hooks. */
typedef struct {
    uint32_t base;
    uint32_t size;
} MblinkStm32f767FlashRange;

typedef struct {
    void *context;
    bool (*read_active_slot)(void *, uint8_t *);
    bool (*read_monotonic_version)(void *, uint32_t *);
    bool (*erase_slot)(void *, uint8_t, uint32_t, uint32_t);
    bool (*program)(void *, uint32_t, const uint8_t *, size_t);
    bool (*finish_image)(void *, uint8_t, uint64_t);
    bool (*verify_integrity)(void *, uint8_t, uint32_t, uint64_t);
    bool (*verify_authenticity)(void *, uint8_t, uint32_t, uint64_t);
    /* Persist slot, version and image size as one pending candidate. */
    bool (*stage_slot)(void *, uint8_t, uint32_t, uint64_t);
    bool (*secure_boot_validate)(void *, uint8_t, uint32_t);
    bool (*commit_monotonic_version)(void *, uint32_t);
    /* Protected persistent pending metadata, surviving a processor reset. */
    bool (*read_pending_candidate)(void *, uint8_t *, uint32_t *, uint64_t *);
    bool (*clear_pending_candidate)(void *);
    void (*abort_slot)(void *, uint8_t);
} MblinkStm32f767OtaOps;

typedef struct {
    MblinkStm32f767FlashRange flash;
    MblinkStm32f767FlashRange bootloader;
    MblinkStm32f767FlashRange slots[2];
    MblinkStm32f767OtaOps ops;
} MblinkStm32f767OtaConfig;

typedef struct {
    MblinkStm32f767OtaConfig config;
    uint8_t selected_slot;
    uint64_t declared_size;
    bool image_started;
} MblinkStm32f767OtaTarget;

/* On failure, does not enable programming or install a partially bound backend.
 * Keep target and config.ops.context alive for the bootloader's entire life.
 */
bool mblink_stm32f767_ota_bind(
    MblinkStm32f767OtaTarget *target,
    const MblinkStm32f767OtaConfig *config,
    MblinkUdsBootloaderConfig *bootloader_config);

/* Call on the newly booted target, after binding but before normal operation. */
bool mblink_stm32f767_ota_confirm_after_boot(
    MblinkStm32f767OtaTarget *target);

#endif
