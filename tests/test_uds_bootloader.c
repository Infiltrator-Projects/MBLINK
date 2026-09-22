// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/uds_bootloader.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; \
} } while (0)

typedef struct {
    uint32_t monotonic_version;
    uint32_t staged_version;
    uint8_t selected_slot;
    uint8_t image[32U];
    uint64_t image_size;
    uint64_t bytes_written;
    unsigned int begin_calls;
    unsigned int finish_calls;
    unsigned int integrity_calls;
    unsigned int authenticity_calls;
    unsigned int stage_calls;
    unsigned int secure_boot_calls;
    unsigned int commit_calls;
    unsigned int abort_calls;
} FakeTarget;

static bool fake_read_version(void *context, uint32_t *version)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || version == NULL) return false;
    *version = target->monotonic_version;
    return true;
}

static bool fake_select_slot(void *context, uint8_t *slot)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || slot == NULL) return false;
    target->selected_slot = UINT8_C(1);
    *slot = target->selected_slot;
    return true;
}

static bool fake_begin(
    void *context, uint8_t slot, uint32_t version, uint64_t image_size)
{
    FakeTarget *target = (FakeTarget *)context;
    (void)version;
    if (target == NULL || slot != target->selected_slot ||
        image_size == 0U || image_size > sizeof(target->image)) {
        return false;
    }
    memset(target->image, 0xff, sizeof(target->image));
    target->image_size = image_size;
    target->bytes_written = 0U;
    target->begin_calls++;
    return true;
}

static bool fake_write(
    void *context, uint8_t slot, uint64_t offset,
    const uint8_t *data, size_t length)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || data == NULL || slot != target->selected_slot ||
        offset != target->bytes_written ||
        offset + (uint64_t)length > target->image_size) {
        return false;
    }
    memcpy(target->image + (size_t)offset, data, length);
    target->bytes_written += (uint64_t)length;
    return true;
}

static bool fake_finish(void *context, uint8_t slot, uint64_t image_size)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || slot != target->selected_slot ||
        image_size != target->image_size ||
        target->bytes_written != image_size) {
        return false;
    }
    target->finish_calls++;
    return true;
}

static bool fake_integrity(
    void *context, uint8_t slot, uint32_t version, uint64_t image_size)
{
    FakeTarget *target = (FakeTarget *)context;
    (void)version;
    if (target == NULL || slot != target->selected_slot ||
        image_size != target->image_size) {
        return false;
    }
    target->integrity_calls++;
    return true;
}

static bool fake_authenticity(
    void *context, uint8_t slot, uint32_t version, uint64_t image_size)
{
    FakeTarget *target = (FakeTarget *)context;
    (void)version;
    if (target == NULL || slot != target->selected_slot ||
        image_size != target->image_size) {
        return false;
    }
    target->authenticity_calls++;
    return true;
}

static bool fake_stage(void *context, uint8_t slot, uint32_t version)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || slot != target->selected_slot) return false;
    target->staged_version = version;
    target->stage_calls++;
    return true;
}

static bool fake_secure_boot(void *context, uint8_t slot, uint32_t version)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || slot != target->selected_slot ||
        version != target->staged_version) {
        return false;
    }
    target->secure_boot_calls++;
    return true;
}

static bool fake_commit(void *context, uint32_t version)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target == NULL || version != target->staged_version) return false;
    target->monotonic_version = version;
    target->commit_calls++;
    return true;
}

static void fake_abort(void *context, uint8_t slot)
{
    FakeTarget *target = (FakeTarget *)context;
    if (target != NULL && slot == target->selected_slot) target->abort_calls++;
}

static MblinkUdsBootloaderConfig fake_config(FakeTarget *target)
{
    MblinkUdsBootloaderConfig config = MBLINK_UDS_BOOTLOADER_CONFIG_INIT;
    config.allow_programming = true;
    config.backend.read_monotonic_version = fake_read_version;
    config.backend.select_inactive_slot = fake_select_slot;
    config.backend.begin_image = fake_begin;
    config.backend.write_block = fake_write;
    config.backend.finish_image = fake_finish;
    config.backend.verify_integrity = fake_integrity;
    config.backend.verify_authenticity = fake_authenticity;
    config.backend.stage_inactive_slot = fake_stage;
    config.backend.secure_boot_validate_candidate = fake_secure_boot;
    config.backend.commit_monotonic_version = fake_commit;
    config.backend.abort_image = fake_abort;
    config.backend.context = target;
    return config;
}

static int test_fail_closed_defaults(void)
{
    MblinkUdsBootloader bootloader;
    MblinkUdsBootloaderConfig config = MBLINK_UDS_BOOTLOADER_CONFIG_INIT;

    CHECK(mblink_uds_bootloader_init(&bootloader, &config));
    CHECK(bootloader.state == MBLINK_UDS_BOOTLOADER_STATE_DISARMED);
    CHECK(mblink_uds_bootloader_arm(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_LOCKED);
    CHECK(!config.allow_programming);
    CHECK(config.require_security_access);
    CHECK(config.require_quiesce);
    CHECK(config.require_authenticity);
    CHECK(config.require_secure_boot_validation);
    return 0;
}

static int test_complete_ota_flow(void)
{
    static const uint8_t block_1[] = { 0x10U, 0x20U, 0x30U };
    static const uint8_t block_2[] = { 0x40U, 0x50U };
    static const uint8_t expected[] = { 0x10U, 0x20U, 0x30U, 0x40U, 0x50U };
    FakeTarget target;
    MblinkUdsBootloader bootloader;
    MblinkUdsBootloaderConfig config;

    memset(&target, 0, sizeof(target));
    target.monotonic_version = UINT32_C(7);
    config = fake_config(&target);

    CHECK(mblink_uds_bootloader_init(&bootloader, &config));
    CHECK(mblink_uds_bootloader_arm(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(bootloader.installed_version == UINT32_C(7));

    CHECK(mblink_uds_bootloader_enter_programming_session(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_grant_security(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_set_dtc_recording_disabled(
              &bootloader, true) == MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_set_communication_disabled(
              &bootloader, true) == MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(bootloader.state == MBLINK_UDS_BOOTLOADER_STATE_QUIESCED);

    CHECK(mblink_uds_bootloader_request_download(
              &bootloader, UINT32_C(8), sizeof(expected)) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_transfer_data(
              &bootloader, UINT8_C(2), block_1, sizeof(block_1)) ==
          MBLINK_UDS_BOOTLOADER_RESULT_SEQUENCE_ERROR);
    CHECK(mblink_uds_bootloader_transfer_data(
              &bootloader, UINT8_C(1), block_1, sizeof(block_1)) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_transfer_data(
              &bootloader, UINT8_C(2), block_2, sizeof(block_2)) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(memcmp(target.image, expected, sizeof(expected)) == 0);

    CHECK(mblink_uds_bootloader_request_transfer_exit(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_check_memory(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_stage_for_reset(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_confirm_boot(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(bootloader.state == MBLINK_UDS_BOOTLOADER_STATE_COMPLETE);
    CHECK(target.monotonic_version == UINT32_C(8));
    CHECK(target.begin_calls == 1U && target.finish_calls == 1U);
    CHECK(target.integrity_calls == 1U && target.authenticity_calls == 1U);
    CHECK(target.stage_calls == 1U && target.secure_boot_calls == 1U);
    CHECK(target.commit_calls == 1U && target.abort_calls == 0U);
    return 0;
}

static int test_rollback_rejected(void)
{
    FakeTarget target;
    MblinkUdsBootloader bootloader;
    MblinkUdsBootloaderConfig config;

    memset(&target, 0, sizeof(target));
    target.monotonic_version = UINT32_C(9);
    config = fake_config(&target);

    CHECK(mblink_uds_bootloader_init(&bootloader, &config));
    CHECK(mblink_uds_bootloader_arm(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_enter_programming_session(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_grant_security(&bootloader) ==
          MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_set_dtc_recording_disabled(
              &bootloader, true) == MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_set_communication_disabled(
              &bootloader, true) == MBLINK_UDS_BOOTLOADER_RESULT_OK);
    CHECK(mblink_uds_bootloader_request_download(
              &bootloader, UINT32_C(9), UINT64_C(1)) ==
          MBLINK_UDS_BOOTLOADER_RESULT_ROLLBACK_REJECTED);
    CHECK(target.begin_calls == 0U);
    return 0;
}

int main(void)
{
    CHECK(test_fail_closed_defaults() == 0);
    CHECK(test_complete_ota_flow() == 0);
    CHECK(test_rollback_rejected() == 0);
    puts("MBLINK UDS bootloader facade tests passed");
    return 0;
}
