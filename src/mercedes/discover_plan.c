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

/*
 * Public Mercedes/Caesar traces prove that a number of 204/207/212-family
 * diagnostic ECUs do not use the legislated-OBD request+8 response mapping.
 * Vediamo/CAESAR stores CP_REQUEST_CANIDENTIFIER and
 * CP_RESPONSE_CANIDENTIFIER independently for exactly this reason.
 *
 * Keep only routes whose two CAN identifiers and ECU family are directly
 * published.  Unknown routes remain unknown rather than being extrapolated.
 */
static const MblinkMercedesKnownRoute mercedes_known_routes[] = {
    {
        UINT32_C(0x612), UINT32_C(0x482),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        "eis-ezs", "EIS_212 / EIS_204",
        "CaesarSuite discussion #5: EIS trace 0x612 request, 0x482 response"
    },
    {
        UINT32_C(0x632), UINT32_C(0x486),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        "esp", "ABR2XT",
        "CaesarSuite discussion #5: ABR2XT CP_REQUEST 0x632, CP_RESPONSE 0x486"
    },
    {
        UINT32_C(0x64a), UINT32_C(0x489),
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        "restraints-orc", "ORC_212",
        "Public Monaco trace: ORC_212 HSCAN_KW2C3PE_500, tester 0x64A, response 0x489"
    },
    {
        UINT32_C(0x652), UINT32_C(0x48a),
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        "audio-headunit", "HU_204",
        "Public HU_204 Monaco trace: HSCAN_KW2C3PE_500, tester 0x652, response 0x48A"
    },
    {
        UINT32_C(0x6b2), UINT32_C(0x496),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        "steering-column", "EPS212",
        "CaesarSuite discussion #5: EPS212 CP_REQUEST 0x6B2, CP_RESPONSE 0x496"
    }
};

size_t mblink_mercedes_known_route_count(void)
{
    return sizeof(mercedes_known_routes) / sizeof(mercedes_known_routes[0]);
}

const MblinkMercedesKnownRoute *mblink_mercedes_known_route_at(size_t index)
{
    return index < mblink_mercedes_known_route_count()
        ? &mercedes_known_routes[index] : NULL;
}

const MblinkMercedesKnownRoute *mblink_mercedes_known_route_for_tx(
    uint32_t tx_can_id)
{
    size_t index;
    for (index = 0U; index < mblink_mercedes_known_route_count(); ++index) {
        if (mercedes_known_routes[index].tx_can_id == tx_can_id)
            return &mercedes_known_routes[index];
    }
    return NULL;
}

static bool mercedes_tx_is_known(uint32_t tx)
{
    return mblink_mercedes_known_route_for_tx(tx) != NULL;
}

static int mercedes_generic_11_tx_at(size_t index, uint32_t *tx)
{
    uint32_t candidate;
    size_t seen = 0U;

    if (tx == NULL) return 0;
    for (candidate = MBLINK_SWEEP_11_FIRST;
         candidate <= MBLINK_SWEEP_11_LAST;
         ++candidate) {
        if (mercedes_tx_is_known(candidate)) continue;
        if (seen == index) {
            *tx = candidate;
            return 1;
        }
        ++seen;
    }
    return 0;
}

static int mercedes_target_at(
    size_t index, link_discover_sweep_target *target)
{
    const size_t known_count = mblink_mercedes_known_route_count();

    if (target == NULL || index >= MBLINK_SWEEP_TARGET_COUNT ||
        known_count > MBLINK_SWEEP_11_COUNT) {
        return 0;
    }
    memset(target, 0, sizeof(*target));
    target->bitrate = 500000U;

    /*
     * Probe published Mercedes physical routes first.  These are the highest
     * value targets because their independent request/response identifiers and
     * protocol families are source-backed.  The remainder of the 11-bit sweep
     * follows afterwards, skipping those TX identifiers so no ECU is probed
     * twice.  The overall target count and 29-bit portion remain unchanged.
     */
    if (index < known_count) {
        const MblinkMercedesKnownRoute *known =
            mblink_mercedes_known_route_at(index);
        if (known == NULL) return 0;
        target->tx_can_id = known->tx_can_id;
        target->rx_can_id = known->rx_can_id;
        target->extended_id = false;
        return 1;
    }

    if (index < MBLINK_SWEEP_11_COUNT) {
        uint32_t tx = 0U;
        if (!mercedes_generic_11_tx_at(index - known_count, &tx)) return 0;
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
    if (!target->extended_id) {
        const MblinkMercedesKnownRoute *known =
            mblink_mercedes_known_route_for_tx(target->tx_can_id);
        if (known != NULL) return known->qualifier;
    }
    return "Unidentified Mercedes ECU";
}

static int mercedes_target_probes(
    const link_discover_sweep_target *target,
    const link_discover_sweep_probe **presence_probes,
    size_t *presence_probe_count,
    const link_discover_sweep_probe **identity_probe,
    link_discover_sweep_decode_identity_fn *decode_identity)
{
    static const link_discover_sweep_probe kwp_presence_probes[] = {
        {
            {UINT8_C(0x3e), UINT8_C(0x01)},
            2U,
            "Mercedes KWP2000 TesterPresent (response required)"
        },
        {
            {UINT8_C(0x18), UINT8_C(0x02), UINT8_C(0xff), UINT8_C(0x00)},
            4U,
            "Mercedes KWP2000 ReadDiagnosticTroubleCodesByStatus"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0x00)},
            3U,
            "Mercedes KWP2000 common-identifier presence fallback"
        }
    };
    const MblinkMercedesKnownRoute *route;

    if (target == NULL || presence_probes == NULL ||
        presence_probe_count == NULL || identity_probe == NULL ||
        decode_identity == NULL) {
        return 0;
    }
    if (target->extended_id) return 1;

    route = mblink_mercedes_known_route_for_tx(target->tx_can_id);
    if (route == NULL || route->rx_can_id != target->rx_can_id ||
        route->protocol != MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) {
        return 1;
    }

    *presence_probes = kwp_presence_probes;
    *presence_probe_count =
        sizeof(kwp_presence_probes) / sizeof(kwp_presence_probes[0]);
    /*
     * Do not apply UDS F197 identity semantics to a KWP endpoint. Exact
     * KWP-family identity options remain product evidence-gated.
     */
    *identity_probe = NULL;
    *decode_identity = NULL;
    return 1;
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
        mercedes_fallback_label,
        mercedes_target_probes
    };
    return &plan;
}
