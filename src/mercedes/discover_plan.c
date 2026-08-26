// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file discover_plan.c
 * @brief Mercedes-owned exhaustive diagnostic discovery plan.
 *
 * LINK deliberately contains no Mercedes address ranges or DID assumptions.
 * This file supplies the C207-oriented full-sweep target map and read-only
 * probe policy to LINK's generic discovery engine.
 */
#include "mblink/discover.h"

#include <string.h>

#define MBLINK_SWEEP_11_FIRST UINT32_C(0x600)
#define MBLINK_SWEEP_11_LAST  UINT32_C(0x7f7)
#define MBLINK_SWEEP_11_COUNT ((size_t)(MBLINK_SWEEP_11_LAST - MBLINK_SWEEP_11_FIRST + 1U))
#define MBLINK_SWEEP_29_COUNT ((size_t)255U)
#define MBLINK_SWEEP_TARGET_COUNT (MBLINK_SWEEP_11_COUNT + MBLINK_SWEEP_29_COUNT)

static int mercedes_target_at(
    size_t index, link_discover_sweep_target *target)
{
    if (target == NULL || index >= MBLINK_SWEEP_TARGET_COUNT) return 0;
    memset(target, 0, sizeof(*target));
    target->bitrate = 500000U;

    if (index < MBLINK_SWEEP_11_COUNT) {
        const uint32_t tx = MBLINK_SWEEP_11_FIRST + (uint32_t)index;
        target->tx_can_id = tx;
        target->rx_can_id = tx + UINT32_C(8);
        target->extended_id = false;
        return 1;
    }

    {
        size_t normal_index = index - MBLINK_SWEEP_11_COUNT;
        unsigned int diagnostic_target = (unsigned int)normal_index;
        if (diagnostic_target >= 0xf1U) ++diagnostic_target;
        target->tx_can_id =
            UINT32_C(0x18da00f1) | ((uint32_t)diagnostic_target << 8U);
        target->rx_can_id =
            UINT32_C(0x18daf100) | (uint32_t)diagnostic_target;
        target->extended_id = true;
    }
    return 1;
}

static int mercedes_decode_f197(
    const uint8_t *payload,
    size_t payload_length,
    char *label,
    size_t label_capacity)
{
    size_t start = SIZE_MAX;
    size_t index;
    size_t written = 0U;

    if (payload == NULL || label == NULL || label_capacity == 0U) return 0;
    label[0] = '\0';

    for (index = 0U; index + 2U < payload_length; ++index) {
        if (payload[index] == UINT8_C(0x62) &&
            payload[index + 1U] == UINT8_C(0xf1) &&
            payload[index + 2U] == UINT8_C(0x97)) {
            start = index + 3U;
            break;
        }
    }
    if (start == SIZE_MAX) return 0;

    for (index = start;
         index < payload_length && written + 1U < label_capacity;
         ++index) {
        const uint8_t value = payload[index];
        if (value == 0U || value == UINT8_C(0xff)) break;
        if (value < UINT8_C(0x20) || value > UINT8_C(0x7e)) return 0;
        label[written++] = (char)value;
    }
    while (written != 0U && label[written - 1U] == ' ') --written;
    label[written] = '\0';
    return written != 0U;
}

static const char *mercedes_fallback_label(
    const link_discover_sweep_target *target)
{
    if (target == NULL) return "Unidentified Mercedes ECU";
    if (!target->extended_id &&
        target->tx_can_id == UINT32_C(0x7e0)) {
        return "Engine ECU";
    }
    if (!target->extended_id &&
        target->tx_can_id == UINT32_C(0x7e1)) {
        return "Secondary EOBD powertrain ECU";
    }
    return "Unidentified Mercedes ECU";
}

const link_discover_sweep_plan *mblink_discover_full_sweep_plan(void)
{
    static const link_discover_sweep_probe presence_probes[] = {
        {
            {UINT8_C(0x3e), UINT8_C(0x00)},
            2U,
            "Mercedes full-sweep TesterPresent"
        },
        {
            {UINT8_C(0x19), UINT8_C(0x02), UINT8_C(0xff)},
            3U,
            "Mercedes full-sweep ReadDTCInformation"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0x90)},
            3U,
            "Mercedes full-sweep VIN identification fallback"
        }
    };
    static const link_discover_sweep_probe identity_probe = {
        {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0x97)},
        3U,
        "Mercedes full-sweep F197 system-name read"
    };
    static const link_discover_sweep_plan plan = {
        "Mercedes forensic diagnostic sweep",
        MBLINK_SWEEP_TARGET_COUNT,
        mercedes_target_at,
        presence_probes,
        sizeof(presence_probes) / sizeof(presence_probes[0]),
        &identity_probe,
        mercedes_decode_f197,
        mercedes_fallback_label
    };
    return &plan;
}
