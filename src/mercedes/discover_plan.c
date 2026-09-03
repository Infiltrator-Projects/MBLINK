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
#define MBLINK_SWEEP_11_EXTERNAL_COUNT ((size_t)1U)
#define MBLINK_SWEEP_11_TARGET_COUNT \
    (MBLINK_SWEEP_11_COUNT + MBLINK_SWEEP_11_EXTERNAL_COUNT)
#define MBLINK_SWEEP_29_COUNT ((size_t)255U)
#define MBLINK_SWEEP_TARGET_COUNT \
    (MBLINK_SWEEP_11_TARGET_COUNT + MBLINK_SWEEP_29_COUNT)

/*
 * Series 204/207/212 diagnostic traces expose a compact gateway-addressed
 * request/response lattice used by the mobile census:
 *
 *   RX = 0x480 + slot
 *   TX = 0x602 + (slot * 8)
 *
 * Slots 0..46 therefore cover RX 0x480..0x4AE and TX 0x602..0x772.
 * Six source-backed 204/207/212 routes land exactly on this lattice.  The
 * 2026-09-03 C207 field capture independently observed five of the lattice
 * routes (0x602, 0x612, 0x632, 0x64A and 0x652) with their expected response
 * identifiers. Two known Daimler routes sit outside it (0x607->0x587 and
 * 0x4E0->0x5FF).
 * The eight ISO 15765-4 physical OBD slots are appended so powertrain
 * responders remain part of the saved whole-vehicle profile.
 */
#define MBLINK_MOBILE_GRID_COUNT ((size_t)47U)
#define MBLINK_MOBILE_EXCEPTION_COUNT ((size_t)2U)
#define MBLINK_MOBILE_OBD_COUNT ((size_t)8U)
#define MBLINK_MOBILE_TARGET_COUNT \
    (MBLINK_MOBILE_GRID_COUNT + MBLINK_MOBILE_EXCEPTION_COUNT + \
     MBLINK_MOBILE_OBD_COUNT)
#define MBLINK_MOBILE_TX_FIRST UINT32_C(0x602)
#define MBLINK_MOBILE_RX_FIRST UINT32_C(0x480)

/*
 * Public Mercedes/Caesar traces prove that a number of 204/207/212-family
 * diagnostic ECUs do not use the legislated-OBD request+8 response mapping.
 * Vediamo/CAESAR stores CP_REQUEST_CANIDENTIFIER and
 * CP_RESPONSE_CANIDENTIFIER independently for exactly this reason.
 *
 * Keep only routes whose request/response identifiers and protocol are
 * directly evidenced. A route may remain deliberately unclassified when the
 * production bootstrap proves connectivity but does not name the ECU family.
 * Unknown routes remain unknown rather than being extrapolated.
 */
static const MblinkMercedesKnownRoute mercedes_known_routes[] = {
    {
        UINT32_C(0x612), UINT32_C(0x482),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0,
        "eis-ezs", "EIS_212 / EIS_204",
        "Daimler production VIN cascade and CaesarSuite EIS trace: 0x612 -> 0x482, UDS 22 F1 A0; independently observed on a 2026-09-03 C207 field capture"
    },
    {
        UINT32_C(0x632), UINT32_C(0x486),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_VIN_PROBE_UDS_F190,
        "esp", "ABR2XT",
        "CaesarSuite discussion #5: ABR2XT CP_REQUEST 0x632, CP_RESPONSE 0x486; independently observed on a 2026-09-03 C207 field capture"
    },
    {
        UINT32_C(0x64a), UINT32_C(0x489),
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        MBLINK_MERCEDES_VIN_PROBE_NONE,
        "restraints-orc", "ORC_212",
        "Public Monaco trace: ORC_212 HSCAN_KW2C3PE_500, tester 0x64A, response 0x489; independently observed on a 2026-09-03 C207 field capture"
    },
    {
        UINT32_C(0x652), UINT32_C(0x48a),
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        MBLINK_MERCEDES_VIN_PROBE_NONE,
        "audio-headunit", "HU_204",
        "Public HU_204 Monaco trace: HSCAN_KW2C3PE_500, tester 0x652, response 0x48A; independently observed on a 2026-09-03 C207 field capture"
    },
    {
        UINT32_C(0x6b2), UINT32_C(0x496),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_VIN_PROBE_UDS_F190,
        "steering-column", "EPS212",
        "CaesarSuite discussion #5: EPS212 CP_REQUEST 0x6B2, CP_RESPONSE 0x496"
    },
    {
        UINT32_C(0x602), UINT32_C(0x480),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0,
        NULL, "Daimler VIN-cascade ECU 602",
        "Daimler/T-Systems production MSA_VIN_cascade: Ecu602 0x602 -> 0x480, UDS 22 F1 A0; independently observed on a 2026-09-03 C207 field capture"
    },
    {
        UINT32_C(0x607), UINT32_C(0x587),
        MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0,
        NULL, "Daimler VIN-cascade ECU 607",
        "Daimler/T-Systems production MSA_VIN_cascade: Ecu607 0x607 -> 0x587, UDS 22 F1 A0"
    },
    {
        UINT32_C(0x4e0), UINT32_C(0x5ff),
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        MBLINK_MERCEDES_VIN_PROBE_KWP_2105,
        NULL, "Daimler VIN-cascade ECU 4E0",
        "Daimler/T-Systems production MSA_VIN_cascade: Ecu4e0 0x4E0 -> 0x5FF, KWP2000 21 05"
    },
    {
        UINT32_C(0x7e1), UINT32_C(0x7e9),
        MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        MBLINK_MERCEDES_VIN_PROBE_NONE,
        "transmission-vgs", "GS / VGS / EGS transmission control",
        "Mercedes CAN definition names D_RQ_GS 0x7E1 as KWP2000 diagnostic request to gearbox control and D_RS_GS 0x7E9 as its response; 0x7E9 independently observed on the 2026-09-03 C207 field capture"
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
        known_count > MBLINK_SWEEP_11_TARGET_COUNT) {
        return 0;
    }
    memset(target, 0, sizeof(*target));
    target->bitrate = 500000U;

    /*
     * Probe published Mercedes physical routes first.  These are the highest
     * value targets because their independent request/response identifiers and
     * protocol families are source-backed.  The remainder of the 11-bit sweep
     * follows afterwards, skipping those TX identifiers so no ECU is probed
     * twice. The source-backed 0x4E0 route sits outside the generic 0x600..0x7F7
     * range and therefore adds one target; the 29-bit portion remains
     * unchanged.
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

    if (index < MBLINK_SWEEP_11_TARGET_COUNT) {
        uint32_t tx = 0U;
        if (!mercedes_generic_11_tx_at(index - known_count, &tx)) return 0;
        target->tx_can_id = tx;
        target->rx_can_id = tx + UINT32_C(8);
        target->extended_id = false;
        return 1;
    }

    {
        size_t normal_index = index - MBLINK_SWEEP_11_TARGET_COUNT;
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

static bool mercedes_route_is_mobile_grid_route(
    const MblinkMercedesKnownRoute *route)
{
    uint32_t delta;
    uint32_t slot;

    if (route == NULL || route->tx_can_id < MBLINK_MOBILE_TX_FIRST)
        return false;
    delta = route->tx_can_id - MBLINK_MOBILE_TX_FIRST;
    if ((delta % UINT32_C(8)) != 0U) return false;
    slot = delta / UINT32_C(8);
    return slot < (uint32_t)MBLINK_MOBILE_GRID_COUNT &&
           route->rx_can_id == MBLINK_MOBILE_RX_FIRST + slot;
}

static const MblinkMercedesKnownRoute *mercedes_mobile_exception_at(
    size_t exception_index)
{
    size_t seen = 0U;
    for (size_t index = 0U;
         index < mblink_mercedes_known_route_count(); ++index) {
        const MblinkMercedesKnownRoute *route =
            mblink_mercedes_known_route_at(index);
        if (route == NULL || mercedes_route_is_mobile_grid_route(route))
            continue;
        if (seen == exception_index) return route;
        ++seen;
    }
    return NULL;
}

static int mercedes_mobile_target_at(
    size_t index, link_discover_sweep_target *target)
{
    if (target == NULL || index >= MBLINK_MOBILE_TARGET_COUNT) return 0;
    memset(target, 0, sizeof(*target));
    target->bitrate = 500000U;

    if (index < MBLINK_MOBILE_GRID_COUNT) {
        const uint32_t slot = (uint32_t)index;
        target->tx_can_id =
            MBLINK_MOBILE_TX_FIRST + slot * UINT32_C(8);
        target->rx_can_id = MBLINK_MOBILE_RX_FIRST + slot;
        target->extended_id = false;
        return 1;
    }

    index -= MBLINK_MOBILE_GRID_COUNT;
    if (index < MBLINK_MOBILE_EXCEPTION_COUNT) {
        const MblinkMercedesKnownRoute *route =
            mercedes_mobile_exception_at(index);
        if (route == NULL) return 0;
        target->tx_can_id = route->tx_can_id;
        target->rx_can_id = route->rx_can_id;
        target->extended_id = false;
        return 1;
    }

    index -= MBLINK_MOBILE_EXCEPTION_COUNT;
    if (index < MBLINK_MOBILE_OBD_COUNT) {
        target->tx_can_id = UINT32_C(0x7e0) + (uint32_t)index;
        target->rx_can_id = UINT32_C(0x7e8) + (uint32_t)index;
        target->extended_id = false;
        return 1;
    }
    return 0;
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
        return "Transmission ECU / GS";
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
        }
    };
    static const link_discover_sweep_probe kwp_2105_presence_probes[] = {
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
            {UINT8_C(0x21), UINT8_C(0x05)},
            2U,
            "Daimler MSA VIN cascade KWP2000 21 05"
        }
    };
    static const link_discover_sweep_probe uds_f1a0_presence_probes[] = {
        {
            {UINT8_C(0x3e), UINT8_C(0x00)},
            2U,
            "Mercedes UDS TesterPresent"
        },
        {
            {UINT8_C(0x19), UINT8_C(0x02), UINT8_C(0xff)},
            3U,
            "Mercedes UDS ReadDTCInformation"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0xa0)},
            3U,
            "Daimler MSA VIN cascade UDS F1A0"
        }
    };
    static const link_discover_sweep_probe motor_vin_cascade_probes[] = {
        {
            {UINT8_C(0x3e), UINT8_C(0x00)},
            2U,
            "Mercedes UDS TesterPresent"
        },
        {
            {UINT8_C(0x19), UINT8_C(0x02), UINT8_C(0xff)},
            3U,
            "Mercedes UDS ReadDTCInformation"
        },
        {
            {UINT8_C(0x09), UINT8_C(0x02)},
            2U,
            "Daimler MSA VIN cascade OBD Mode 09 PID 02"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0xa0)},
            3U,
            "Daimler MSA VIN cascade UDS F1A0"
        },
        {
            {UINT8_C(0x1a), UINT8_C(0x90)},
            2U,
            "Daimler MSA VIN cascade KWP2000 1A 90"
        }
    };
    static const link_discover_sweep_probe ecu4e0_vin_cascade_probes[] = {
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
            {UINT8_C(0x21), UINT8_C(0x05)},
            2U,
            "Daimler MSA VIN cascade KWP2000 21 05"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0xa0)},
            3U,
            "Daimler MSA VIN cascade UDS F1A0"
        },
        {
            {UINT8_C(0x1a), UINT8_C(0x90)},
            2U,
            "Daimler MSA VIN cascade KWP2000 1A 90"
        }
    };
    static const link_discover_sweep_probe ecu612_vin_cascade_probes[] = {
        {
            {UINT8_C(0x3e), UINT8_C(0x00)},
            2U,
            "Mercedes UDS TesterPresent"
        },
        {
            {UINT8_C(0x19), UINT8_C(0x02), UINT8_C(0xff)},
            3U,
            "Mercedes UDS ReadDTCInformation"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0xa0)},
            3U,
            "Daimler MSA VIN cascade UDS F1A0"
        },
        {
            {UINT8_C(0x21), UINT8_C(0x05)},
            2U,
            "Daimler MSA VIN cascade KWP2000 21 05"
        }
    };
    const MblinkMercedesKnownRoute *route;

    if (target == NULL || presence_probes == NULL ||
        presence_probe_count == NULL || identity_probe == NULL ||
        decode_identity == NULL) {
        return 0;
    }
    if (target->extended_id) return 1;

    /*
     * Reproduce the exact read-only production VIN cascade where the APK
     * supplied more than one protocol fallback for the same physical route.
     * These overrides are deliberately exact-TX/RX only.
     */
    if (target->tx_can_id == UINT32_C(0x7e0) &&
        target->rx_can_id == UINT32_C(0x7e8)) {
        *presence_probes = motor_vin_cascade_probes;
        *presence_probe_count = sizeof(motor_vin_cascade_probes) /
            sizeof(motor_vin_cascade_probes[0]);
        return 1;
    }

    route = mblink_mercedes_known_route_for_tx(target->tx_can_id);
    if (route == NULL || route->rx_can_id != target->rx_can_id) return 1;

    if (target->tx_can_id == UINT32_C(0x4e0) &&
        target->rx_can_id == UINT32_C(0x5ff)) {
        *presence_probes = ecu4e0_vin_cascade_probes;
        *presence_probe_count = sizeof(ecu4e0_vin_cascade_probes) /
            sizeof(ecu4e0_vin_cascade_probes[0]);
        *identity_probe = NULL;
        *decode_identity = NULL;
        return 1;
    }
    if (target->tx_can_id == UINT32_C(0x612) &&
        target->rx_can_id == UINT32_C(0x482)) {
        *presence_probes = ecu612_vin_cascade_probes;
        *presence_probe_count = sizeof(ecu612_vin_cascade_probes) /
            sizeof(ecu612_vin_cascade_probes[0]);
        return 1;
    }

    if (route->vin_probe == MBLINK_MERCEDES_VIN_PROBE_UDS_F1A0) {
        *presence_probes = uds_f1a0_presence_probes;
        *presence_probe_count = sizeof(uds_f1a0_presence_probes) /
            sizeof(uds_f1a0_presence_probes[0]);
        return 1;
    }

    if (route->protocol != MBLINK_MERCEDES_DIAGNOSTIC_KWP2000) return 1;

    if (route->vin_probe == MBLINK_MERCEDES_VIN_PROBE_KWP_2105) {
        *presence_probes = kwp_2105_presence_probes;
        *presence_probe_count = sizeof(kwp_2105_presence_probes) /
            sizeof(kwp_2105_presence_probes[0]);
    } else {
        *presence_probes = kwp_presence_probes;
        *presence_probe_count =
            sizeof(kwp_presence_probes) / sizeof(kwp_presence_probes[0]);
    }
    /*
     * Do not apply UDS F197 identity semantics to a KWP endpoint. Exact
     * KWP-family identity options remain product evidence-gated.
     */
    *identity_probe = NULL;
    *decode_identity = NULL;
    return 1;
}

const link_discover_sweep_plan *mblink_discover_mobile_census_plan(void)
{
    static const link_discover_sweep_probe presence_probes[] = {
        {
            {UINT8_C(0x3e), UINT8_C(0x00)},
            2U,
            "Mercedes mobile-census TesterPresent"
        },
        {
            {UINT8_C(0x19), UINT8_C(0x02), UINT8_C(0xff)},
            3U,
            "Mercedes mobile-census ReadDTCInformation"
        },
        {
            {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0x90)},
            3U,
            "Mercedes mobile-census VIN identification fallback"
        }
    };
    static const link_discover_sweep_probe identity_probe = {
        {UINT8_C(0x22), UINT8_C(0xf1), UINT8_C(0x97)},
        3U,
        "Mercedes mobile-census F197 system-name read"
    };
    static const link_discover_sweep_plan plan = {
        "Mercedes 47-slot mobile gateway census",
        MBLINK_MOBILE_TARGET_COUNT,
        mercedes_mobile_target_at,
        presence_probes,
        sizeof(presence_probes) / sizeof(presence_probes[0]),
        &identity_probe,
        mercedes_decode_f197,
        mercedes_fallback_label,
        mercedes_target_probes
    };
    return &plan;
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
