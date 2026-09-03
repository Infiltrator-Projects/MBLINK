// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_vin.h
 * @brief Offline Mercedes-Benz VIN/FIN and Baumuster decoding.
 *
 * Mercedes FIN-style VINs embed the six-digit vehicle Baumuster immediately
 * after the three-character WMI. The decoder keeps structural VIN facts
 * separate from live ECU identity: VIN tells us what Mercedes built; UDS tells
 * us what control unit is installed now.
 */
#ifndef MBLINK_MERCEDES_VIN_H
#define MBLINK_MERCEDES_VIN_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_VIN_LENGTH 17U

typedef enum {
    MBLINK_MERCEDES_VIN_LAYOUT_UNKNOWN = 0,
    MBLINK_MERCEDES_VIN_LAYOUT_BAUMUSTER_FIN,
    MBLINK_MERCEDES_VIN_LAYOUT_ISO_VDS
} MblinkMercedesVinLayout;

typedef enum {
    MBLINK_MERCEDES_FUEL_UNKNOWN = 0,
    MBLINK_MERCEDES_FUEL_PETROL,
    MBLINK_MERCEDES_FUEL_DIESEL,
    MBLINK_MERCEDES_FUEL_HYBRID,
    MBLINK_MERCEDES_FUEL_ELECTRIC
} MblinkMercedesFuelType;

typedef enum {
    MBLINK_MERCEDES_STEERING_UNKNOWN = 0,
    MBLINK_MERCEDES_STEERING_LEFT_HAND_DRIVE,
    MBLINK_MERCEDES_STEERING_RIGHT_HAND_DRIVE
} MblinkMercedesSteering;

typedef struct {
    const char *code;
    const char *manufacturer;
    const char *wmi_country;
    const char *vehicle_scope;
} MblinkMercedesWmiDefinition;

typedef struct {
    char code;
    const char *plant;
    const char *country;
} MblinkMercedesPlantDefinition;

typedef struct {
    const char *baumuster;
    const char *chassis_family;
    const char *series;
    const char *body_style;
    const char *model;
    const char *engine_code;
    const char *engine_family;
    MblinkMercedesFuelType fuel;
    unsigned int displacement_cc;
    unsigned int rated_power_kw;
    const char *production_years;
    const char *provenance;

    /*
     * Optional factory-spec corroboration. These fields are populated only
     * where Mercedes-Benz Vehicle Specification evidence is available and
     * safe to generalise to the Baumuster. VIN-specific option combinations
     * are deliberately not stored here.
     */
    unsigned int rated_torque_nm;
    const char *factory_spec_provenance;
} MblinkMercedesBaumusterDefinition;

typedef struct {
    bool valid;
    bool mercedes_wmi;
    MblinkMercedesVinLayout layout;
    char vin[MBLINK_MERCEDES_VIN_LENGTH + 1U];
    char wmi[4];
    const MblinkMercedesWmiDefinition *wmi_definition;

    bool baumuster_available;
    char baumuster[7];
    char series_number[4];
    const MblinkMercedesBaumusterDefinition *baumuster_definition;

    MblinkMercedesSteering steering;
    char plant_code;
    const MblinkMercedesPlantDefinition *plant_definition;
    char serial_number[7];

    /* ISO/VDS layout fields are retained when no Mercedes FIN Baumuster exists. */
    char vds[7];
    char check_digit;
    char model_year_code;
} MblinkMercedesVinDecode;

const char *mblink_mercedes_vin_layout_name(MblinkMercedesVinLayout layout);
const char *mblink_mercedes_fuel_type_name(MblinkMercedesFuelType fuel);
const char *mblink_mercedes_steering_name(MblinkMercedesSteering steering);

size_t mblink_mercedes_wmi_count(void);
const MblinkMercedesWmiDefinition *mblink_mercedes_wmi_at(size_t index);
const MblinkMercedesWmiDefinition *mblink_mercedes_find_wmi(const char *wmi);

size_t mblink_mercedes_plant_count(void);
const MblinkMercedesPlantDefinition *mblink_mercedes_plant_at(size_t index);
const MblinkMercedesPlantDefinition *mblink_mercedes_find_plant(char code);

size_t mblink_mercedes_baumuster_count(void);
const MblinkMercedesBaumusterDefinition *
mblink_mercedes_baumuster_at(size_t index);
const MblinkMercedesBaumusterDefinition *
mblink_mercedes_find_baumuster(const char *baumuster);

bool mblink_mercedes_vin_decode(
    const char *vin,
    MblinkMercedesVinDecode *decoded);

#ifdef __cplusplus
}
#endif

#endif
