// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_crd3.h
 * @brief Evidence-oriented Mercedes CRD3 identity decoding.
 *
 * The payload layouts here are limited to identity records published by the
 * open-source CaesarSuite Simulated_CRD3 model. They are not live-data
 * formulas, and a successful decode does not by itself prove a C207/OM651 ECU
 * variant until matched against a real trace fixture.
 */
#ifndef MBLINK_MERCEDES_CRD3_H
#define MBLINK_MERCEDES_CRD3_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_CRD3_DID_SESSION_VARIANT 0xf100U
#define MBLINK_MERCEDES_CRD3_DID_SUPPLIER_IDENTIFIER 0xf154U
#define MBLINK_MERCEDES_CRD3_DID_EROTAN 0xf196U
#define MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_FULL 0x1001U
#define MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_PARTIAL 0x1002U

typedef struct {
    uint8_t gateway_mode;
    uint16_t variant;
    uint8_t session;
} MblinkMercedesCrd3SessionVariant;

typedef struct {
    uint8_t supplier_identifier;
    const char *supplier_name;
} MblinkMercedesCrd3Supplier;

static inline const char *mblink_mercedes_crd3_supplier_name(uint8_t identifier)
{
    switch (identifier) {
    case 1U: return "Becker";
    case 2U: return "Blaupunkt";
    case 3U: return "Bosch";
    case 4U: return "Mercedes-Benz";
    case 8U: return "Siemens";
    case 17U: return "VDO";
    case 21U: return "Hella";
    case 32U: return "Borg";
    case 33U: return "Temic";
    case 35U: return "BorgWarner";
    case 37U: return "Denso";
    case 38U: return "ZF";
    case 39U: return "TRW";
    case 48U: return "Magneti Marelli";
    case 56U: return "Panasonic";
    case 60U: return "Conti Temic";
    case 64U: return "Delphi";
    case 71U: return "BERU";
    case 72U: return "Valeo";
    case 86U: return "Pierburg";
    case 98U: return "Eberspacher";
    case 119U: return "Autoliv";
    case 121U: return "Siemens VDO";
    case 126U: return "Hitachi";
    case 132U: return "Harman Becker";
    case 133U: return "Mitsubishi Electric";
    case 139U: return "Vector";
    case 151U: return "GETRAG";
    case 254U: return "Aftermarket supplier";
    case 255U: return "Unidentified";
    default: return "Unknown supplier";
    }
}

/**
 * Decode the four-byte F100 payload used by the published CRD3 simulator:
 * gateway-mode, big-endian 16-bit ECU variant, active session.
 */
static inline bool mblink_mercedes_crd3_decode_session_variant(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesCrd3SessionVariant *value)
{
    MblinkMercedesCrd3SessionVariant decoded;

    if (data == NULL || value == NULL || data_length != 4U) {
        return false;
    }

    decoded.gateway_mode = data[0];
    decoded.variant = (uint16_t)(((uint16_t)data[1] << 8U) | data[2]);
    decoded.session = data[3];
    *value = decoded;
    return true;
}

/** Decode the one-byte F154 supplier identifier. */
static inline bool mblink_mercedes_crd3_decode_supplier(
    const uint8_t *data,
    size_t data_length,
    MblinkMercedesCrd3Supplier *value)
{
    MblinkMercedesCrd3Supplier decoded;

    if (data == NULL || value == NULL || data_length != 1U) {
        return false;
    }

    decoded.supplier_identifier = data[0];
    decoded.supplier_name = mblink_mercedes_crd3_supplier_name(data[0]);
    *value = decoded;
    return true;
}

#ifdef __cplusplus
}
#endif

#endif
