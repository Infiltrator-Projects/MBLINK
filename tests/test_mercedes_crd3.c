// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_crd3.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_session_variant(void)
{
    const uint8_t payload[] = { 0x02U, 0x21U, 0x31U, 0x01U };
    MblinkMercedesCrd3SessionVariant value;

    CHECK(mblink_mercedes_crd3_decode_session_variant(
        payload, sizeof(payload), &value));
    CHECK(value.gateway_mode == 0x02U);
    CHECK(value.variant == 0x2131U);
    CHECK(value.session == 0x01U);

    memset(&value, 0xa5, sizeof(value));
    {
        MblinkMercedesCrd3SessionVariant snapshot = value;
        CHECK(!mblink_mercedes_crd3_decode_session_variant(
            payload, 3U, &value));
        CHECK(memcmp(&value, &snapshot, sizeof(value)) == 0);
    }
    return 0;
}

static int test_supplier(void)
{
    const uint8_t payload[] = { 64U };
    const uint8_t unknown_payload[] = { 200U };
    MblinkMercedesCrd3Supplier value;

    CHECK(mblink_mercedes_crd3_decode_supplier(
        payload, sizeof(payload), &value));
    CHECK(value.supplier_identifier == 64U);
    CHECK(strcmp(value.supplier_name, "Delphi") == 0);

    /* Real C207/OM651 capture: F154 returned 00 40 for Delphi. */
    {
        const uint8_t c207_payload[] = { 0x00U, 0x40U };
        CHECK(mblink_mercedes_crd3_decode_supplier(
            c207_payload, sizeof(c207_payload), &value));
        CHECK(value.supplier_identifier == 64U);
        CHECK(strcmp(value.supplier_name, "Delphi") == 0);
    }
    {
        const uint8_t ambiguous_payload[] = { 0x01U, 0x40U };
        CHECK(!mblink_mercedes_crd3_decode_supplier(
            ambiguous_payload, sizeof(ambiguous_payload), &value));
    }

    CHECK(mblink_mercedes_crd3_decode_supplier(
        unknown_payload, sizeof(unknown_payload), &value));
    CHECK(value.supplier_identifier == 200U);
    CHECK(strcmp(value.supplier_name, "Unknown supplier") == 0);
    CHECK(strcmp(mblink_mercedes_crd3_supplier_name(3U), "Bosch") == 0);
    CHECK(strcmp(mblink_mercedes_crd3_supplier_name(4U), "Mercedes-Benz") == 0);
    return 0;
}

static int test_corroborated_om651_signature(void)
{
    const uint8_t variant_payload[] = { 0x02U, 0x21U, 0x31U, 0x01U };
    const uint8_t supplier_payload[] = { 64U };
    const uint8_t wrong_supplier_payload[] = { 3U };
    MblinkMercedesCrd3SessionVariant variant;
    MblinkMercedesCrd3Supplier supplier;

    CHECK(mblink_mercedes_crd3_decode_session_variant(
        variant_payload, sizeof(variant_payload), &variant));
    CHECK(mblink_mercedes_crd3_decode_supplier(
        supplier_payload, sizeof(supplier_payload), &supplier));
    CHECK(mblink_mercedes_crd3_matches_om651_cdid3_delphi_signature(
        &variant, &supplier));

    CHECK(mblink_mercedes_crd3_decode_supplier(
        wrong_supplier_payload, sizeof(wrong_supplier_payload), &supplier));
    CHECK(!mblink_mercedes_crd3_matches_om651_cdid3_delphi_signature(
        &variant, &supplier));
    CHECK(!mblink_mercedes_crd3_matches_om651_cdid3_delphi_signature(
        NULL, &supplier));
    return 0;
}

static int test_hardware_profiles(void)
{
    const MblinkMercedesCrd3HardwareProfile *profile;
    const MblinkMercedesCrd3HardwareProfile *matched = NULL;
    MblinkMercedesCrd3ObservedIdentity observed;
    MblinkMercedesCrd3ProfileMatch match;

    CHECK(mblink_mercedes_crd3_hardware_profile_count() == 1U);
    profile = mblink_mercedes_crd3_hardware_profile_at(0U);
    CHECK(profile != NULL);
    CHECK(strcmp(profile->ecu_family, "Delphi CRD3.10") == 0);
    CHECK(strcmp(profile->microcontroller, "Infineon TriCore TC1797") == 0);
    CHECK(profile->rated_power_kw == 150U);
    CHECK(strcmp(profile->hardware_number, "6519040001") == 0);
    CHECK(strcmp(profile->software_number, "6519020001") == 0);
    CHECK(strcmp(profile->spare_part_number, "6519011801") == 0);
    CHECK(mblink_mercedes_crd3_hardware_profile_at(1U) == NULL);

    observed.hardware_number = "A 651 904 00 01";
    observed.software_number = "6519020001";
    observed.spare_part_number = "A6519011801";
    observed.system_name = "CRD3-651-WMA4BD3-212WA";
    match = mblink_mercedes_crd3_match_hardware_profile(&observed, &matched);
    CHECK(match == MBLINK_MERCEDES_CRD3_PROFILE_MATCH_STRONG);
    CHECK(matched == profile);

    observed.hardware_number = "6519040001";
    observed.software_number = "6519999999";
    observed.spare_part_number = NULL;
    observed.system_name = NULL;
    match = mblink_mercedes_crd3_match_hardware_profile(&observed, &matched);
    CHECK(match == MBLINK_MERCEDES_CRD3_PROFILE_MATCH_CORROBORATED);
    CHECK(matched == profile);

    observed.hardware_number = NULL;
    observed.software_number = NULL;
    observed.spare_part_number = NULL;
    observed.system_name = "crd3-651-unknown-calibration";
    match = mblink_mercedes_crd3_match_hardware_profile(&observed, &matched);
    CHECK(match == MBLINK_MERCEDES_CRD3_PROFILE_MATCH_FAMILY);
    CHECK(matched == profile);

    observed.system_name = "BOSCH EDC17";
    match = mblink_mercedes_crd3_match_hardware_profile(&observed, &matched);
    CHECK(match == MBLINK_MERCEDES_CRD3_PROFILE_MATCH_NONE);
    CHECK(matched == NULL);
    return 0;
}

static int test_identifiers(void)
{
    CHECK(MBLINK_MERCEDES_CRD3_DID_SESSION_VARIANT == 0xf100U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_SUPPLIER_IDENTIFIER == 0xf154U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_EROTAN == 0xf196U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_FULL == 0x1001U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_PARTIAL == 0x1002U);
    CHECK(MBLINK_MERCEDES_CRD3_OM651_CDID3_GATEWAY_MODE == 0x02U);
    CHECK(MBLINK_MERCEDES_CRD3_OM651_CDID3_VARIANT == 0x2131U);
    CHECK(MBLINK_MERCEDES_CRD3_OM651_CDID3_SUPPLIER == 64U);
    return 0;
}

int main(void)
{
    if (test_session_variant() != 0) return 1;
    if (test_supplier() != 0) return 1;
    if (test_corroborated_om651_signature() != 0) return 1;
    if (test_hardware_profiles() != 0) return 1;
    if (test_identifiers() != 0) return 1;
    puts("Mercedes CRD3 identity tests passed");
    return 0;
}
