// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_vin.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
    return 1; } } while (0)

static int test_official_fin_shape(void)
{
    MblinkMercedesVinDecode decoded;

    CHECK(mblink_mercedes_vin_decode("WDB2452081A123456", &decoded));
    CHECK(decoded.valid);
    CHECK(decoded.mercedes_wmi);
    CHECK(decoded.layout == MBLINK_MERCEDES_VIN_LAYOUT_BAUMUSTER_FIN);
    CHECK(strcmp(decoded.wmi, "WDB") == 0);
    CHECK(strcmp(decoded.baumuster, "245208") == 0);
    CHECK(strcmp(decoded.series_number, "245") == 0);
    CHECK(decoded.baumuster_definition == NULL);
    CHECK(decoded.steering == MBLINK_MERCEDES_STEERING_LEFT_HAND_DRIVE);
    CHECK(decoded.plant_code == 'A');
    CHECK(decoded.plant_definition != NULL);
    CHECK(strcmp(decoded.plant_definition->plant, "Sindelfingen") == 0);
    CHECK(strcmp(decoded.plant_definition->country, "Germany") == 0);
    CHECK(strcmp(decoded.serial_number, "123456") == 0);
    return 0;
}

static int test_c207_petrol_data_card_vin(void)
{
    MblinkMercedesVinDecode decoded;

    CHECK(mblink_mercedes_vin_decode("WDD2073472F126610", &decoded));
    CHECK(decoded.baumuster_definition != NULL);
    CHECK(strcmp(decoded.baumuster_definition->chassis_family, "C207") == 0);
    CHECK(strcmp(decoded.baumuster_definition->body_style, "Coupe") == 0);
    CHECK(strcmp(decoded.baumuster_definition->model, "E 250 CGI") == 0);
    CHECK(strcmp(decoded.baumuster_definition->engine_code, "M271.860") == 0);
    CHECK(strcmp(decoded.baumuster_definition->engine_family, "M271") == 0);
    CHECK(decoded.baumuster_definition->fuel == MBLINK_MERCEDES_FUEL_PETROL);
    CHECK(decoded.baumuster_definition->displacement_cc == 1796U);
    CHECK(decoded.baumuster_definition->rated_power_kw == 150U);
    CHECK(decoded.steering == MBLINK_MERCEDES_STEERING_RIGHT_HAND_DRIVE);
    CHECK(decoded.plant_definition != NULL);
    CHECK(strcmp(decoded.plant_definition->plant, "Bremen") == 0);
    CHECK(strcmp(decoded.plant_definition->country, "Germany") == 0);
    CHECK(strcmp(decoded.serial_number, "126610") == 0);
    return 0;
}

static int test_c207_m274_data_card_vin(void)
{
    MblinkMercedesVinDecode decoded;

    CHECK(mblink_mercedes_vin_decode("WDD2073362F347681", &decoded));
    CHECK(decoded.baumuster_definition != NULL);
    CHECK(strcmp(decoded.baumuster_definition->model, "E 250") == 0);
    CHECK(strcmp(decoded.baumuster_definition->engine_code, "M274.920") == 0);
    CHECK(decoded.baumuster_definition->fuel == MBLINK_MERCEDES_FUEL_PETROL);
    CHECK(strcmp(decoded.serial_number, "347681") == 0);
    return 0;
}

static int test_unknown_baumuster_still_decodes_structure(void)
{
    MblinkMercedesVinDecode decoded;

    CHECK(mblink_mercedes_vin_decode("WDD2073991F654321", &decoded));
    CHECK(decoded.baumuster_available);
    CHECK(decoded.baumuster_definition == NULL);
    CHECK(strcmp(decoded.baumuster, "207399") == 0);
    CHECK(strcmp(decoded.series_number, "207") == 0);
    CHECK(decoded.steering == MBLINK_MERCEDES_STEERING_LEFT_HAND_DRIVE);
    CHECK(strcmp(decoded.serial_number, "654321") == 0);
    return 0;
}

static int test_rejects_invalid_input(void)
{
    MblinkMercedesVinDecode decoded;

    CHECK(!mblink_mercedes_vin_decode(NULL, &decoded));
    CHECK(!mblink_mercedes_vin_decode("WDD2073472F12661", &decoded));
    CHECK(!mblink_mercedes_vin_decode("WDD2073472F12661I", &decoded));
    CHECK(!mblink_mercedes_vin_decode("JHM2073472F126610", &decoded));
    CHECK(!mblink_mercedes_vin_decode("WDD2073472F126610", NULL));
    return 0;
}

int main(void)
{
    if (test_official_fin_shape() != 0) return 1;
    if (test_c207_petrol_data_card_vin() != 0) return 1;
    if (test_c207_m274_data_card_vin() != 0) return 1;
    if (test_unknown_baumuster_still_decodes_structure() != 0) return 1;
    if (test_rejects_invalid_input() != 0) return 1;
    CHECK(mblink_mercedes_baumuster_count() >= 23U);
    CHECK(strcmp(mblink_mercedes_vin_layout_name(
                     MBLINK_MERCEDES_VIN_LAYOUT_BAUMUSTER_FIN),
                 "Mercedes FIN/Baumuster") == 0);
    return 0;
}
