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

    CHECK(mblink_mercedes_crd3_decode_supplier(
        unknown_payload, sizeof(unknown_payload), &value));
    CHECK(value.supplier_identifier == 200U);
    CHECK(strcmp(value.supplier_name, "Unknown supplier") == 0);
    CHECK(strcmp(mblink_mercedes_crd3_supplier_name(3U), "Bosch") == 0);
    CHECK(strcmp(mblink_mercedes_crd3_supplier_name(4U), "Mercedes-Benz") == 0);
    return 0;
}

static int test_identifiers(void)
{
    CHECK(MBLINK_MERCEDES_CRD3_DID_SESSION_VARIANT == 0xf100U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_SUPPLIER_IDENTIFIER == 0xf154U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_EROTAN == 0xf196U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_FULL == 0x1001U);
    CHECK(MBLINK_MERCEDES_CRD3_DID_VARIANT_CODING_PARTIAL == 0x1002U);
    return 0;
}

int main(void)
{
    if (test_session_variant() != 0) return 1;
    if (test_supplier() != 0) return 1;
    if (test_identifiers() != 0) return 1;
    puts("Mercedes CRD3 identity tests passed");
    return 0;
}
