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
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_CRD3_DID_SESSION_VARIANT 0xf100U
#define MBLINK_MERCEDES_CRD3_DID_SUPPLIER_IDENTIFIER 0xf154U
#define MBLINK_MERCEDES_CRD3_DID_EROTAN 0xf196U
#define MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_FULL 0x1001U
#define MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_PARTIAL 0x1002U

/*
 * Cross-corroborated OM651/CDID3 signature. CaesarSuite's CRD3 simulator maps
 * F100 bytes 0..2 to gateway-mode + 16-bit ECU variant and F154 to a supplier
 * byte. Independent ScanDoc OM651/CDID3 records for E 250 CDI/S 250 CDI report
 * diagnostic version 02 21 31 and Delphi as supplier. This makes the pair a
 * useful offline family signature, but not a substitute for a C207 trace.
 */
#define MBLINK_MERCEDES_CRD3_OM651_CDID3_GATEWAY_MODE 0x02U
#define MBLINK_MERCEDES_CRD3_OM651_CDID3_VARIANT 0x2131U
#define MBLINK_MERCEDES_CRD3_OM651_CDID3_SUPPLIER 64U

typedef struct {
    uint8_t gateway_mode;
    uint16_t variant;
    uint8_t session;
} MblinkMercedesCrd3SessionVariant;

typedef struct {
    uint8_t supplier_identifier;
    const char *supplier_name;
} MblinkMercedesCrd3Supplier;

typedef enum {
    MBLINK_MERCEDES_CRD3_PROFILE_MATCH_NONE = 0,
    MBLINK_MERCEDES_CRD3_PROFILE_MATCH_FAMILY,
    MBLINK_MERCEDES_CRD3_PROFILE_MATCH_CORROBORATED,
    MBLINK_MERCEDES_CRD3_PROFILE_MATCH_STRONG
} MblinkMercedesCrd3ProfileMatch;

typedef struct {
    const char *key;
    const char *ecu_family;
    const char *microcontroller;
    const char *vehicle_scope;
    unsigned int rated_power_kw;
    const char *hardware_number;
    const char *software_number;
    const char *spare_part_number;
    const char *system_name_prefix;
    const char *provenance;
} MblinkMercedesCrd3HardwareProfile;

typedef struct {
    const char *hardware_number;
    const char *software_number;
    const char *spare_part_number;
    const char *system_name;
} MblinkMercedesCrd3ObservedIdentity;

static inline const char *mblink_mercedes_crd3_profile_match_name(
    MblinkMercedesCrd3ProfileMatch match)
{
    switch (match) {
    case MBLINK_MERCEDES_CRD3_PROFILE_MATCH_NONE: return "no-match";
    case MBLINK_MERCEDES_CRD3_PROFILE_MATCH_FAMILY: return "family";
    case MBLINK_MERCEDES_CRD3_PROFILE_MATCH_CORROBORATED:
        return "source-corroborated";
    case MBLINK_MERCEDES_CRD3_PROFILE_MATCH_STRONG:
        return "strong-source-match";
    }
    return "unknown";
}

static inline size_t mblink_mercedes_crd3_hardware_profile_count(void)
{
    return 1U;
}

static inline const MblinkMercedesCrd3HardwareProfile *
mblink_mercedes_crd3_hardware_profile_at(size_t index)
{
    static const MblinkMercedesCrd3HardwareProfile profiles[] = {
        {
            "e250-2011-crd3-10-6519040001",
            "Delphi CRD3.10",
            "Infineon TriCore TC1797",
            "Mercedes E 250 CDI 2.2 / 150 kW / 2011; C207/W212 OM651 family",
            150U,
            "6519040001",
            "6519020001",
            "6519011801",
            "CRD3-651-",
            "2026-08-27 source corroboration: public 2011 E250 CRD3.10 firmware catalogues identify HW 6519040001 and SW 6519020001; independent E250 catalogues corroborate spare 6519011801 and CRD3.10/CRD3.1R fitment; independent CRD3.10 tooling documentation identifies the family MCU as Infineon TriCore TC1797. Exact vehicle promotion still requires returned ECU identity data."
        }
    };
    return index < (sizeof(profiles) / sizeof(profiles[0]))
        ? &profiles[index] : NULL;
}

static inline bool mblink_mercedes_crd3_ascii_contains_case_insensitive(
    const char *text,
    const char *needle)
{
    size_t text_index;
    size_t needle_length;

    if (text == NULL || needle == NULL || needle[0] == '\0') return false;
    needle_length = strlen(needle);
    for (text_index = 0U; text[text_index] != '\0'; ++text_index) {
        size_t offset = 0U;
        while (offset < needle_length && text[text_index + offset] != '\0') {
            unsigned char left = (unsigned char)text[text_index + offset];
            unsigned char right = (unsigned char)needle[offset];
            if (left >= (unsigned char)'a' && left <= (unsigned char)'z')
                left = (unsigned char)(left - ((unsigned char)'a' - (unsigned char)'A'));
            if (right >= (unsigned char)'a' && right <= (unsigned char)'z')
                right = (unsigned char)(right - ((unsigned char)'a' - (unsigned char)'A'));
            if (left != right) break;
            ++offset;
        }
        if (offset == needle_length) return true;
    }
    return false;
}

static inline bool mblink_mercedes_crd3_number_equal(
    const char *left,
    const char *right)
{
    char left_digits[32];
    char right_digits[32];
    size_t left_count = 0U;
    size_t right_count = 0U;
    size_t index;

    if (left == NULL || right == NULL) return false;
    for (index = 0U; left[index] != '\0'; ++index) {
        if (left[index] >= '0' && left[index] <= '9') {
            if (left_count + 1U >= sizeof(left_digits)) return false;
            left_digits[left_count++] = left[index];
        }
    }
    for (index = 0U; right[index] != '\0'; ++index) {
        if (right[index] >= '0' && right[index] <= '9') {
            if (right_count + 1U >= sizeof(right_digits)) return false;
            right_digits[right_count++] = right[index];
        }
    }
    if (left_count == 0U || left_count != right_count) return false;
    return memcmp(left_digits, right_digits, left_count) == 0;
}

static inline MblinkMercedesCrd3ProfileMatch
mblink_mercedes_crd3_match_hardware_profile(
    const MblinkMercedesCrd3ObservedIdentity *identity,
    const MblinkMercedesCrd3HardwareProfile **matched_profile)
{
    MblinkMercedesCrd3ProfileMatch best =
        MBLINK_MERCEDES_CRD3_PROFILE_MATCH_NONE;
    const MblinkMercedesCrd3HardwareProfile *best_profile = NULL;
    size_t profile_index;

    if (matched_profile != NULL) *matched_profile = NULL;
    if (identity == NULL) return best;

    for (profile_index = 0U;
         profile_index < mblink_mercedes_crd3_hardware_profile_count();
         ++profile_index) {
        const MblinkMercedesCrd3HardwareProfile *profile =
            mblink_mercedes_crd3_hardware_profile_at(profile_index);
        unsigned int typed_matches = 0U;
        unsigned int typed_available = 0U;
        bool family_match = false;
        MblinkMercedesCrd3ProfileMatch current =
            MBLINK_MERCEDES_CRD3_PROFILE_MATCH_NONE;

        if (profile == NULL) continue;
        if (identity->hardware_number != NULL &&
            identity->hardware_number[0] != '\0') {
            ++typed_available;
            if (mblink_mercedes_crd3_number_equal(
                    identity->hardware_number, profile->hardware_number)) {
                ++typed_matches;
            }
        }
        if (identity->software_number != NULL &&
            identity->software_number[0] != '\0') {
            ++typed_available;
            if (mblink_mercedes_crd3_number_equal(
                    identity->software_number, profile->software_number)) {
                ++typed_matches;
            }
        }
        if (identity->spare_part_number != NULL &&
            identity->spare_part_number[0] != '\0') {
            ++typed_available;
            if (mblink_mercedes_crd3_number_equal(
                    identity->spare_part_number, profile->spare_part_number)) {
                ++typed_matches;
            }
        }
        if (identity->system_name != NULL &&
            identity->system_name[0] != '\0') {
            family_match =
                mblink_mercedes_crd3_ascii_contains_case_insensitive(
                    identity->system_name, profile->system_name_prefix) ||
                mblink_mercedes_crd3_ascii_contains_case_insensitive(
                    identity->system_name, "CRD3");
        }

        if (typed_available >= 2U && typed_matches == typed_available) {
            current = MBLINK_MERCEDES_CRD3_PROFILE_MATCH_STRONG;
        } else if (typed_matches != 0U) {
            current = MBLINK_MERCEDES_CRD3_PROFILE_MATCH_CORROBORATED;
        } else if (family_match) {
            current = MBLINK_MERCEDES_CRD3_PROFILE_MATCH_FAMILY;
        }

        if (current > best) {
            best = current;
            best_profile = profile;
        }
    }

    if (matched_profile != NULL) *matched_profile = best_profile;
    return best;
}

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

static inline bool mblink_mercedes_crd3_matches_om651_cdid3_delphi_signature(
    const MblinkMercedesCrd3SessionVariant *variant,
    const MblinkMercedesCrd3Supplier *supplier)
{
    return variant != NULL && supplier != NULL &&
           variant->gateway_mode ==
               MBLINK_MERCEDES_CRD3_OM651_CDID3_GATEWAY_MODE &&
           variant->variant == MBLINK_MERCEDES_CRD3_OM651_CDID3_VARIANT &&
           supplier->supplier_identifier ==
               MBLINK_MERCEDES_CRD3_OM651_CDID3_SUPPLIER;
}

#ifdef __cplusplus
}
#endif

#endif
