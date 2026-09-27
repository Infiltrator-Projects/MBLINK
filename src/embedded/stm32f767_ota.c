// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/stm32f767_ota.h"

#include <string.h>

static bool contains(
    MblinkStm32f767FlashRange outer, MblinkStm32f767FlashRange inner)
{
    return outer.size != 0U && inner.size != 0U &&
           inner.base >= outer.base &&
           (uint64_t)inner.base + inner.size <=
               (uint64_t)outer.base + outer.size &&
           (uint64_t)outer.base + outer.size <= UINT64_C(0x100000000);
}

static bool overlaps(
    MblinkStm32f767FlashRange a, MblinkStm32f767FlashRange b)
{
    return (uint64_t)a.base < (uint64_t)b.base + b.size &&
           (uint64_t)b.base < (uint64_t)a.base + a.size;
}

static bool read_version(void *context, uint32_t *version)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    return t->config.ops.read_monotonic_version(
        t->config.ops.context, version);
}

static bool select_slot(void *context, uint8_t *slot)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    uint8_t active;
    if (slot == NULL || !t->config.ops.read_active_slot(
            t->config.ops.context, &active) || active > 1U) return false;
    *slot = (uint8_t)(1U - active);
    return true;
}

static bool begin_image(
    void *context, uint8_t slot, uint32_t version, uint64_t size)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    uint8_t active;
    const MblinkStm32f767FlashRange *range;
    (void)version;
    if (slot > 1U || size == 0U || t->image_started ||
        !t->config.ops.read_active_slot(t->config.ops.context, &active) ||
        active > 1U || active == slot) return false;
    range = &t->config.slots[slot];
    if (size > range->size ||
        !t->config.ops.erase_slot(
            t->config.ops.context, slot, range->base, range->size))
        return false;
    t->selected_slot = slot;
    t->declared_size = size;
    t->image_started = true;
    return true;
}

static bool write_block(
    void *context, uint8_t slot, uint64_t offset,
    const uint8_t *data, size_t length)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    uint8_t active;
    if (!t->image_started || slot != t->selected_slot || data == NULL ||
        length == 0U || offset > t->declared_size ||
        (uint64_t)length > t->declared_size - offset ||
        !t->config.ops.read_active_slot(t->config.ops.context, &active) ||
        active > 1U || active == slot) return false;
    return t->config.ops.program(
        t->config.ops.context,
        t->config.slots[slot].base + (uint32_t)offset, data, length);
}

static bool finish_image(void *context, uint8_t slot, uint64_t size)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    return t->image_started && slot == t->selected_slot &&
           size == t->declared_size &&
           t->config.ops.finish_image(t->config.ops.context, slot, size);
}

static bool integrity(
    void *context, uint8_t slot, uint32_t version, uint64_t size)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    return t->image_started && slot == t->selected_slot &&
           size == t->declared_size &&
           t->config.ops.verify_integrity(
               t->config.ops.context, slot, version, size);
}

static bool authenticity(
    void *context, uint8_t slot, uint32_t version, uint64_t size)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    return t->image_started && slot == t->selected_slot &&
           size == t->declared_size &&
           t->config.ops.verify_authenticity(
               t->config.ops.context, slot, version, size);
}

static bool stage(void *context, uint8_t slot, uint32_t version)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    return t->image_started && slot == t->selected_slot &&
           t->config.ops.stage_slot(
               t->config.ops.context, slot, version, t->declared_size);
}

static bool secure_boot(void *context, uint8_t slot, uint32_t version)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    return t->image_started && slot == t->selected_slot &&
           t->config.ops.secure_boot_validate(
               t->config.ops.context, slot, version);
}

static bool commit_version(void *context, uint32_t version)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    uint8_t running;
    /* Never burn the rollback counter while still executing the old slot. */
    return t->image_started &&
           t->config.ops.read_active_slot(t->config.ops.context, &running) &&
           running == t->selected_slot &&
           t->config.ops.commit_monotonic_version(
               t->config.ops.context, version);
}

static void abort_image(void *context, uint8_t slot)
{
    MblinkStm32f767OtaTarget *t = (MblinkStm32f767OtaTarget *)context;
    if (t->image_started && slot == t->selected_slot) {
        t->config.ops.abort_slot(t->config.ops.context, slot);
        (void)t->config.ops.clear_pending_candidate(t->config.ops.context);
    }
    t->image_started = false;
    t->declared_size = 0U;
}

bool mblink_stm32f767_ota_bind(
    MblinkStm32f767OtaTarget *target,
    const MblinkStm32f767OtaConfig *config,
    MblinkUdsBootloaderConfig *bootloader_config)
{
    MblinkUdsBootloaderConfig bound = MBLINK_UDS_BOOTLOADER_CONFIG_INIT;
    const MblinkStm32f767OtaOps *ops;
    if (target == NULL || config == NULL || bootloader_config == NULL ||
        !contains(config->flash, config->bootloader) ||
        !contains(config->flash, config->slots[0]) ||
        !contains(config->flash, config->slots[1]) ||
        overlaps(config->bootloader, config->slots[0]) ||
        overlaps(config->bootloader, config->slots[1]) ||
        overlaps(config->slots[0], config->slots[1])) return false;
    ops = &config->ops;
    if (ops->read_active_slot == NULL ||
        ops->read_monotonic_version == NULL || ops->erase_slot == NULL ||
        ops->program == NULL || ops->finish_image == NULL ||
        ops->verify_integrity == NULL || ops->verify_authenticity == NULL ||
        ops->stage_slot == NULL || ops->secure_boot_validate == NULL ||
        ops->commit_monotonic_version == NULL ||
        ops->read_pending_candidate == NULL ||
        ops->clear_pending_candidate == NULL || ops->abort_slot == NULL)
        return false;
    memset(target, 0, sizeof(*target));
    target->config = *config;
    target->selected_slot = MBLINK_UDS_BOOTLOADER_SLOT_NONE;
    bound.allow_programming = true;
    bound.backend.read_monotonic_version = read_version;
    bound.backend.select_inactive_slot = select_slot;
    bound.backend.begin_image = begin_image;
    bound.backend.write_block = write_block;
    bound.backend.finish_image = finish_image;
    bound.backend.verify_integrity = integrity;
    bound.backend.verify_authenticity = authenticity;
    bound.backend.stage_inactive_slot = stage;
    bound.backend.secure_boot_validate_candidate = secure_boot;
    bound.backend.commit_monotonic_version = commit_version;
    bound.backend.abort_image = abort_image;
    bound.backend.context = target;
    *bootloader_config = bound;
    return true;
}

bool mblink_stm32f767_ota_confirm_after_boot(
    MblinkStm32f767OtaTarget *target)
{
    uint8_t running, pending_slot;
    uint32_t pending_version, protected_version;
    uint64_t pending_size;
    void *context;
    const MblinkStm32f767OtaOps *ops;
    if (target == NULL || !contains(target->config.flash,
            target->config.slots[0]) || !contains(target->config.flash,
            target->config.slots[1])) return false;
    ops = &target->config.ops;
    context = ops->context;
    if (!ops->read_pending_candidate(context, &pending_slot,
            &pending_version, &pending_size) ||
        !ops->read_active_slot(context, &running) ||
        !ops->read_monotonic_version(context, &protected_version) ||
        pending_slot > 1U || running != pending_slot ||
        pending_size == 0U ||
        pending_size > target->config.slots[pending_slot].size ||
        pending_version < protected_version) return false;
    if (pending_version == protected_version)
        return ops->clear_pending_candidate(context);
    if (!ops->verify_integrity(context, pending_slot,
            pending_version, pending_size) ||
        !ops->verify_authenticity(context, pending_slot,
            pending_version, pending_size) ||
        !ops->secure_boot_validate(context, pending_slot, pending_version) ||
        !ops->commit_monotonic_version(context, pending_version))
        return false;
    return ops->clear_pending_candidate(context);
}
