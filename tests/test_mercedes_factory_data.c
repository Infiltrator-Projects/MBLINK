// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_factory_data.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_evidence_precedence(void)
{
    CHECK(mblink_mercedes_evidence_should_replace(
        MBLINK_MERCEDES_EVIDENCE_GENERIC_METADATA,
        MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_OPTION));
    CHECK(mblink_mercedes_evidence_should_replace(
        MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_OPTION,
        MBLINK_MERCEDES_EVIDENCE_POSITIVE_DIAGNOSTIC));
    CHECK(mblink_mercedes_evidence_should_replace(
        MBLINK_MERCEDES_EVIDENCE_POSITIVE_DIAGNOSTIC,
        MBLINK_MERCEDES_EVIDENCE_ECU_IDENTITY));
    CHECK(!mblink_mercedes_evidence_should_replace(
        MBLINK_MERCEDES_EVIDENCE_ECU_IDENTITY,
        MBLINK_MERCEDES_EVIDENCE_GENERIC_METADATA));
    CHECK(!mblink_mercedes_evidence_should_replace(
        MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_OPTION,
        MBLINK_MERCEDES_EVIDENCE_VIN_FACTORY_OPTION));
    CHECK(strcmp(mblink_mercedes_evidence_rank_name(
        MBLINK_MERCEDES_EVIDENCE_POSITIVE_DIAGNOSTIC),
        "positive diagnostic evidence") == 0);
    return 0;
}

static int test_factory_option_hints(void)
{
    const MblinkMercedesFactoryOptionDefinition *option;

    CHECK(mblink_mercedes_factory_option_count() >= 14U);

    option = mblink_mercedes_factory_option_for_code("423");
    CHECK(option != NULL);
    CHECK(strcmp(option->description, "5-speed automatic transmission") == 0);
    CHECK(strcmp(option->observed_baumuster, "207303") == 0);
    CHECK(strcmp(option->module_key_hint, "transmission-vgs") == 0);
    CHECK(mblink_mercedes_factory_option_prioritises_module(
        "423", "transmission-vgs"));
    CHECK(!mblink_mercedes_factory_option_prioritises_module(
        "423", "esp"));

    option = mblink_mercedes_factory_option_for_code("580");
    CHECK(option != NULL);
    CHECK(strcmp(option->module_key_hint, "climate") == 0);

    option = mblink_mercedes_factory_option_for_code("230");
    CHECK(option != NULL);
    CHECK(option->module_key_hint == NULL);
    CHECK(!mblink_mercedes_factory_option_prioritises_module(
        "230", "central-gateway"));

    CHECK(mblink_mercedes_factory_option_for_code("DOES-NOT-EXIST") == NULL);
    return 0;
}

int main(void)
{
    if (test_evidence_precedence() != 0) return 1;
    if (test_factory_option_hints() != 0) return 1;
    return 0;
}
