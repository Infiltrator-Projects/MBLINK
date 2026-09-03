// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_vin.h"

#include <ctype.h>
#include <string.h>

#define ARRAY_LENGTH(values) (sizeof(values) / sizeof((values)[0]))

static const MblinkMercedesWmiDefinition mercedes_wmis[] = {
    {"WDB", "Mercedes-Benz / Daimler-Benz", "Germany", "Mercedes-Benz vehicles"},
    {"WDC", "Mercedes-Benz / DaimlerChrysler", "Germany", "Mercedes-Benz vehicles"},
    {"WDD", "Mercedes-Benz / Daimler AG", "Germany", "Mercedes-Benz passenger cars"},
    {"WDF", "Mercedes-Benz / Daimler", "Germany", "Mercedes-Benz vans/commercial vehicles"},
    {"W1K", "Mercedes-Benz Group AG", "Germany", "Mercedes-Benz passenger cars"},
    {"W1N", "Mercedes-Benz Group AG", "Germany", "Mercedes-Benz multipurpose/SUV vehicles"},
    {"VSA", "Mercedes-Benz España", "Spain", "Mercedes-Benz vehicles"},
    {"4JG", "Mercedes-Benz", "United States", "Mercedes-Benz US-market vehicles"},
    {"55S", "Mercedes-Benz U.S. International", "United States", "Mercedes-Benz US-built vehicles"}
};

static const MblinkMercedesPlantDefinition mercedes_plants[] = {
    {'A', "Sindelfingen", "Germany"},
    {'B', "Sindelfingen", "Germany"},
    {'C', "Sindelfingen", "Germany"},
    {'D', "Sindelfingen", "Germany"},
    {'E', "Sindelfingen", "Germany"},
    {'F', "Bremen", "Germany"},
    {'G', "Bremen", "Germany"},
    {'H', "Bremen", "Germany"},
    {'J', "Rastatt", "Germany"},
    {'L', "Pekan", "Malaysia"},
    {'M', "Woking", "United Kingdom"},
    {'N', "Kecskemet", "Hungary"},
    {'R', "East London", "South Africa"},
    {'T', "Karmann / Osnabruck", "Germany"},
    {'V', "Valmet Automotive", "Finland"},
    {'X', "Graz", "Austria"}
};

/*
 * Source-corroborated C207 coupe catalogue.
 *
 * This is deliberately data, not diagnostic policy. Additional Mercedes
 * Baumuster families can be added without changing the decoder or ECU probe.
 * Power is zero where public sources describe multiple calibrations under the
 * same Baumuster and a single number would be misleading.
 */
static const MblinkMercedesBaumusterDefinition mercedes_baumuster[] = {
    {"207301","C207","207","Coupe","E 220 BlueTEC","OM651.911","OM651",MBLINK_MERCEDES_FUEL_DIESEL,2143U,130U,"2010-2016","Mercedes club/aftermarket catalogues"},
    {"207302","C207","207","Coupe","E 220 CDI / d","OM651.911","OM651",MBLINK_MERCEDES_FUEL_DIESEL,2143U,0U,"2010-2016","Mercedes club/aftermarket catalogues"},
    {"207303","C207","207","Coupe","E 250 CDI BlueEFFICIENCY","OM651.911","OM651",MBLINK_MERCEDES_FUEL_DIESEL,2143U,150U,"2009-2016","Mercedes club/aftermarket catalogues",500U,"Mercedes-Benz Vehicle Specification API: Baumuster 207303, E 250 CDI BlueEFFICIENCY Coupe, 2143 cc, 150 kW and 500 Nm; VIN-specific options intentionally not generalised"},
    {"207304","C207","207","Coupe","E 250 BlueTEC / d","OM651.911","OM651",MBLINK_MERCEDES_FUEL_DIESEL,2143U,150U,"2009-2016","Mercedes club/aftermarket catalogues"},
    {"207322","C207","207","Coupe","E 350 CDI","OM642.836","OM642",MBLINK_MERCEDES_FUEL_DIESEL,2987U,0U,"2009-2011","Mercedes club/aftermarket catalogues"},
    {"207323","C207","207","Coupe","E 350 CDI BlueEFFICIENCY","OM642.838","OM642",MBLINK_MERCEDES_FUEL_DIESEL,2987U,0U,"2011-2013","Mercedes club/aftermarket catalogues"},
    {"207326","C207","207","Coupe","E 350 BlueTEC / d","OM642.838","OM642",MBLINK_MERCEDES_FUEL_DIESEL,2987U,0U,"2013-2016","Mercedes club/aftermarket catalogues"},
    {"207334","C207","207","Coupe","E 200","M274.920","M274",MBLINK_MERCEDES_FUEL_PETROL,1991U,135U,"2013-2016","Mercedes club/aftermarket catalogues"},
    {"207336","C207","207","Coupe","E 250","M274.920","M274",MBLINK_MERCEDES_FUEL_PETROL,1991U,155U,"2013-2016","Mercedes club/aftermarket catalogues"},
    {"207347","C207","207","Coupe","E 250 CGI","M271.860","M271",MBLINK_MERCEDES_FUEL_PETROL,1796U,150U,"2009-2013","Mercedes club/vehicle data-card catalogues"},
    {"207348","C207","207","Coupe","E 200 CGI","M271.820 / M271.860","M271",MBLINK_MERCEDES_FUEL_PETROL,1796U,135U,"2010-2013","Mercedes club/aftermarket catalogues"},
    {"207355","C207","207","Coupe","E 300","M276.957","M276",MBLINK_MERCEDES_FUEL_PETROL,3498U,185U,"2011-2016","Mercedes club/aftermarket catalogues"},
    {"207356","C207","207","Coupe","E 350","M272.961 / M272.988","M272",MBLINK_MERCEDES_FUEL_PETROL,3498U,200U,"2010-2011","Mercedes club/aftermarket catalogues"},
    {"207357","C207","207","Coupe","E 350 CGI","M272.982 / M272.984","M272",MBLINK_MERCEDES_FUEL_PETROL,3498U,215U,"2009-2011","Mercedes club/aftermarket catalogues"},
    {"207359","C207","207","Coupe","E 350","M276.957","M276",MBLINK_MERCEDES_FUEL_PETROL,3498U,225U,"2011-2014","Mercedes club/aftermarket catalogues"},
    {"207360","C207","207","Coupe","E 350","M276.850","M276",MBLINK_MERCEDES_FUEL_PETROL,3498U,0U,"2011-2016","Mercedes club model guide"},
    {"207361","C207","207","Coupe","E 400","M276.850","M276",MBLINK_MERCEDES_FUEL_PETROL,3498U,245U,"2014-2016","Mercedes club/aftermarket catalogues"},
    {"207362","C207","207","Coupe","E 320","M276.820","M276",MBLINK_MERCEDES_FUEL_PETROL,2996U,200U,"2014-2016","Mercedes club/aftermarket catalogues"},
    {"207365","C207","207","Coupe","E 400","M276.820","M276",MBLINK_MERCEDES_FUEL_PETROL,2996U,245U,"2013-2016","Mercedes club/aftermarket catalogues"},
    {"207367","C207","207","Coupe","E 400 4MATIC","M276.820","M276",MBLINK_MERCEDES_FUEL_PETROL,2996U,245U,"2013-2016","Mercedes club model guide"},
    {"207372","C207","207","Coupe","E 500","M273.966","M273",MBLINK_MERCEDES_FUEL_PETROL,5461U,285U,"2009-2011","Mercedes club/aftermarket catalogues"},
    {"207373","C207","207","Coupe","E 500 / E 550 CGI","M278.922","M278",MBLINK_MERCEDES_FUEL_PETROL,4663U,300U,"2011-2016","Mercedes club/aftermarket catalogues"},
    {"207388","C207","207","Coupe","E 350 4MATIC","M276.957","M276",MBLINK_MERCEDES_FUEL_PETROL,3498U,225U,"2013-2016","Mercedes club/aftermarket catalogues"}
};

static bool vin_character_valid(char value)
{
    unsigned char c = (unsigned char)value;
    if (!isalnum(c)) return false;
    c = (unsigned char)toupper(c);
    return c != (unsigned char)'I' &&
           c != (unsigned char)'O' &&
           c != (unsigned char)'Q';
}

static void uppercase_copy(char *destination, const char *source, size_t length)
{
    size_t index;
    for (index = 0U; index < length; ++index)
        destination[index] = (char)toupper((unsigned char)source[index]);
    destination[length] = '\0';
}

const char *mblink_mercedes_vin_layout_name(MblinkMercedesVinLayout layout)
{
    switch (layout) {
    case MBLINK_MERCEDES_VIN_LAYOUT_UNKNOWN: return "unknown";
    case MBLINK_MERCEDES_VIN_LAYOUT_BAUMUSTER_FIN: return "Mercedes FIN/Baumuster";
    case MBLINK_MERCEDES_VIN_LAYOUT_ISO_VDS: return "ISO VIN/VDS";
    }
    return "unknown";
}

const char *mblink_mercedes_fuel_type_name(MblinkMercedesFuelType fuel)
{
    switch (fuel) {
    case MBLINK_MERCEDES_FUEL_UNKNOWN: return "unknown";
    case MBLINK_MERCEDES_FUEL_PETROL: return "petrol";
    case MBLINK_MERCEDES_FUEL_DIESEL: return "diesel";
    case MBLINK_MERCEDES_FUEL_HYBRID: return "hybrid";
    case MBLINK_MERCEDES_FUEL_ELECTRIC: return "electric";
    }
    return "unknown";
}

const char *mblink_mercedes_steering_name(MblinkMercedesSteering steering)
{
    switch (steering) {
    case MBLINK_MERCEDES_STEERING_UNKNOWN: return "unknown";
    case MBLINK_MERCEDES_STEERING_LEFT_HAND_DRIVE: return "left-hand drive";
    case MBLINK_MERCEDES_STEERING_RIGHT_HAND_DRIVE: return "right-hand drive";
    }
    return "unknown";
}

size_t mblink_mercedes_wmi_count(void)
{
    return ARRAY_LENGTH(mercedes_wmis);
}

const MblinkMercedesWmiDefinition *mblink_mercedes_wmi_at(size_t index)
{
    return index < ARRAY_LENGTH(mercedes_wmis) ? &mercedes_wmis[index] : NULL;
}

const MblinkMercedesWmiDefinition *mblink_mercedes_find_wmi(const char *wmi)
{
    size_t index;
    if (wmi == NULL || strlen(wmi) != 3U) return NULL;
    for (index = 0U; index < ARRAY_LENGTH(mercedes_wmis); ++index) {
        if (strncmp(wmi, mercedes_wmis[index].code, 3U) == 0)
            return &mercedes_wmis[index];
    }
    return NULL;
}

size_t mblink_mercedes_plant_count(void)
{
    return ARRAY_LENGTH(mercedes_plants);
}

const MblinkMercedesPlantDefinition *mblink_mercedes_plant_at(size_t index)
{
    return index < ARRAY_LENGTH(mercedes_plants) ? &mercedes_plants[index] : NULL;
}

const MblinkMercedesPlantDefinition *mblink_mercedes_find_plant(char code)
{
    size_t index;
    code = (char)toupper((unsigned char)code);
    for (index = 0U; index < ARRAY_LENGTH(mercedes_plants); ++index) {
        if (mercedes_plants[index].code == code) return &mercedes_plants[index];
    }
    return NULL;
}

size_t mblink_mercedes_baumuster_count(void)
{
    return ARRAY_LENGTH(mercedes_baumuster);
}

const MblinkMercedesBaumusterDefinition *
mblink_mercedes_baumuster_at(size_t index)
{
    return index < ARRAY_LENGTH(mercedes_baumuster)
        ? &mercedes_baumuster[index] : NULL;
}

const MblinkMercedesBaumusterDefinition *
mblink_mercedes_find_baumuster(const char *baumuster)
{
    size_t index;
    if (baumuster == NULL || strlen(baumuster) != 6U) return NULL;
    for (index = 0U; index < ARRAY_LENGTH(mercedes_baumuster); ++index) {
        if (strncmp(baumuster, mercedes_baumuster[index].baumuster, 6U) == 0)
            return &mercedes_baumuster[index];
    }
    return NULL;
}

bool mblink_mercedes_vin_decode(
    const char *vin,
    MblinkMercedesVinDecode *decoded)
{
    size_t index;
    bool numeric_baumuster = true;

    if (decoded == NULL) return false;
    memset(decoded, 0, sizeof(*decoded));
    if (vin == NULL || strlen(vin) != MBLINK_MERCEDES_VIN_LENGTH)
        return false;
    for (index = 0U; index < MBLINK_MERCEDES_VIN_LENGTH; ++index) {
        if (!vin_character_valid(vin[index])) return false;
    }

    uppercase_copy(decoded->vin, vin, MBLINK_MERCEDES_VIN_LENGTH);
    uppercase_copy(decoded->wmi, vin, 3U);
    decoded->wmi_definition = mblink_mercedes_find_wmi(decoded->wmi);
    decoded->mercedes_wmi = decoded->wmi_definition != NULL;
    if (!decoded->mercedes_wmi) return false;

    for (index = 3U; index < 9U; ++index) {
        if (!isdigit((unsigned char)vin[index])) {
            numeric_baumuster = false;
            break;
        }
    }

    if (numeric_baumuster) {
        decoded->layout = MBLINK_MERCEDES_VIN_LAYOUT_BAUMUSTER_FIN;
        decoded->baumuster_available = true;
        uppercase_copy(decoded->baumuster, vin + 3U, 6U);
        uppercase_copy(decoded->series_number, vin + 3U, 3U);
        decoded->baumuster_definition =
            mblink_mercedes_find_baumuster(decoded->baumuster);
        if (decoded->vin[9] == '1')
            decoded->steering = MBLINK_MERCEDES_STEERING_LEFT_HAND_DRIVE;
        else if (decoded->vin[9] == '2')
            decoded->steering = MBLINK_MERCEDES_STEERING_RIGHT_HAND_DRIVE;
        decoded->plant_code = decoded->vin[10];
        decoded->plant_definition =
            mblink_mercedes_find_plant(decoded->plant_code);
        uppercase_copy(decoded->serial_number, vin + 11U, 6U);
    } else {
        decoded->layout = MBLINK_MERCEDES_VIN_LAYOUT_ISO_VDS;
        uppercase_copy(decoded->vds, vin + 3U, 6U);
        decoded->check_digit = decoded->vin[8];
        decoded->model_year_code = decoded->vin[9];
        decoded->plant_code = decoded->vin[10];
        decoded->plant_definition =
            mblink_mercedes_find_plant(decoded->plant_code);
        uppercase_copy(decoded->serial_number, vin + 11U, 6U);
    }

    decoded->valid = true;
    return true;
}
