// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Extract read-only Mercedes Vediamo CBF diagnostic facts.
 *
 * This tool intentionally emits only direct executable read-only Data services:
 * UDS ReadDataByIdentifier (22 xxxx) and KWP2000 ReadDataByLocalIdentifier /
 * ReadECUIdentification (21 xx / 1A xx). It does not import routines, IO control, coding,
 * security access, writes, or inferred semantics.
 */
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CBF_STUB_SIZE UINT32_C(0x410)

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t pos;
    bool ok;
} Reader;

typedef struct {
    uint32_t cff_size;
    uint32_t base;
    int32_t caesar_version;
    int32_t ecu_count;
    int32_t ecu_offset;
    int32_t string_pool_size;
} CffHeader;

typedef struct {
    const char *qualifier;
    uint32_t diag_block;
    int32_t diag_count;
    int32_t diag_entry_size;
} EcuHeader;

typedef struct {
    const char *qualifier;
    uint16_t type;
    uint16_t executable;
    uint16_t client_access;
    uint16_t security_access;
    int16_t request_count;
    int32_t request_offset;
} DiagService;

typedef struct {
    const char *profile_key;
    uint8_t service;
    uint16_t identifier;
    const char *display_name;
} CbfDisplayName;

/*
 * Curated English labels for the pinned CBF Data services. The raw Vediamo
 * qualifier remains untouched in provenance; only the user-facing label is
 * translated. Keep this as data so regeneration cannot reintroduce German
 * source symbols or underscores into PID Setup.
 */
static const CbfDisplayName cbf_display_names[] = {
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0x0026), "Bus watchdog events count" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0x0025), "Engine type" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0xD210), "Installed ECUs actual body" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0xD211), "Installed ECUs actual chassis" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0xD213), "Installed ECUs actual diagnostic" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0xD212), "Installed ECUs actual impact" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0x0310), "Installed ECUs target body" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0x0311), "Installed ECUs target chassis" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0x0313), "Installed ECUs target diagnostic" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0x0312), "Installed ECUs target impact" },
    { "gateway-cgw204", UINT8_C(0x22), UINT16_C(0xD243), "VIN odometer counter" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x022C), "Steering lock status" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x0210), "Terminal status H0 switch" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x024A), "Current odometer reading" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x0231), "Power management status H0 switch" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x0223), "Additional door lock status" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x0228), "Supply voltage Ubat" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x020C), "Central locking last external unlock cause" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x0233), "Global variant coding model series" },
    { "eis-ezs204", UINT8_C(0x22), UINT16_C(0x020E), "Transport mode status" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0402), "ASSYST WIA measured values hex dump data" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0408), "ASSYST maintenance 1 hex dump data" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0xF100), "Diagnostic ID status" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0012), "Log port data indicator clicker" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0011), "Production display text status" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0003), "RWS teach-in status" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0013), "ECU hardware identification number" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0002), "Trip distance" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0001), "Total distance" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0302), "ASSYST PLUS average daily kilometres" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0406), "ASSYST oil overfill threshold 1" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0027), "CAN ID IC A13 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0028), "CAN ID IC A14 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0020), "CAN ID IC A1 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0040), "CAN ID IC A1 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0041), "CAN ID IC A2 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0021), "CAN ID IC A3 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0042), "CAN ID IC A3 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0043), "CAN ID IC A4 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0022), "CAN ID IC A5 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0044), "CAN ID IC A5 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0023), "CAN ID IC A6 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0024), "CAN ID IC A7 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0025), "CAN ID IC A8 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0026), "CAN ID IC A9 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0029), "CAN ID IC CTRL U A2 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0045), "CAN ID IC CTRL U A2 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0030), "CAN ID MESS IC1 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0031), "CAN ID MESS IC2 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0032), "CAN ID MESS IC3 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0047), "CAN ID NV DISP CTRL Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0048), "CAN ID NV DISP RNG Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x002E), "CAN ID SD RS IC Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x002F), "CAN ID SG APPL IC Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x004B), "CAN ID SG APPL IC Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x002C), "CAN ID TP IC MPM3 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0049), "CAN ID TP IC RDU7 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x004A), "CAN ID TP IC TELEAID5 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x002D), "CAN ID TP IC TGW1 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x002A), "CAN ID WIM IC Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0046), "CAN ID WIM IC Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0014), "Global coding Advanced Distronic ADTR" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0306), "ASSYST PLUS workshop code" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0033), "CAN ID MESS IC4 Body data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x004D), "CAN ID IC A10 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x004E), "CAN ID SD RS IC Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x004F), "CAN ID TP IC PTS33 Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0051), "CAN ID TANK OBD Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x004C), "CAN ID CNG TC DIST Chassis data record" },
    { "cluster-ic204", UINT8_C(0x22), UINT16_C(0x0052), "CAN ID LDC DISP STAT Chassis data record" },
    { "steering-sccm204", UINT8_C(0x22), UINT16_C(0x0163), "Local battery voltage" },
    { "steering-sccm204", UINT8_C(0x22), UINT16_C(0x0002), "Steering angle data steering revolution" },
    { "steering-sccm204", UINT8_C(0x22), UINT16_C(0x0160), "Steering column adjustment lever switch states" },
    { "steering-sccm204", UINT8_C(0x22), UINT16_C(0x0003), "Maximum steering angular velocity" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0001), "Actual Peripheral Status Algorithm activated ready to trigger" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x000D), "Bussed ECU Power Voltage Input ECU Voltage Source" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0052), "Bussed Impact Events Output Any Inpact Event" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0051), "Bussed Seatbelt Output Driver Seat Belt Switch Status" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0031), "Deployment Data Record 1 AB BF 1 Airbag Passenger Stage 1" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0032), "Deployment Data Record 2 AB BF 1 Airbag Passenger Stage 1" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0033), "Deployment Data Record 3 AB BF 1 Airbag Passenger Stage 1" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0034), "Deployment Data Record 4 AB BF 1 Airbag Passenger Stage 1" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0035), "Deployment Data Record 5 AB BF 1 Airbag Passenger Stage 1" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0058), "ECU lock state Status" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0013), "ECU battery voltage" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0064), "Front-left pSat serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0065), "Front-right pSat serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0060), "Left ECS serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0062), "Left gSat serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0030), "Near Deploy Data Record Data Block Identifier" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0069), "PP Sensor software Version software Version Patch Level" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0068), "Passenger-presence sensor serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0066), "Rear-left pSat serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0067), "Rear-right pSat serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0061), "Right ECS serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0063), "Right gSat serial number" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0059), "Seat Mat Serial Number hardware version cw" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x000F), "Squib Resistances Driver Airbag Squib 1" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0011), "System time" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0024), "Read Squib Circuit Coupling Squib 01 GS F" },
    { "restraints-orc204", UINT8_C(0x21), UINT16_C(0x0023), "Read Squib Circuit Mismatch Squib 01 GS F" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0006), "AUX Level Status" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0028), "BT Device Entries Device entry number" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0030), "Bussed Input Signals CTRL C A1" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x00D3), "Central Registry DATA" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0026), "Cradle connection status" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0074), "Current DRT CAN ID Request pos 0" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0015), "Current Keypad test Key Code" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0001), "Current Most Configuration 2 Current Configuration" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x00D1), "Environment Data Temperature" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0078), "RMS Error Information Data" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0017), "Rotary Encoder Status" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0008), "Signal Strength Mobile Phone BTPC Signal strength" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0007), "Signal Strength Mobile Phone Cradle Signal strength" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0072), "Statistical Info Application Counters DATA" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0075), "Statistical Info Diag Counters Events DATA" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0071), "Statistical Info Diag Counters Startup DATA" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0073), "Statistical Info Stability Table DATA" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x004C), "Anti Theft PIN accepted DATA" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0019), "Disc drive status" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0012), "GPS Position Available satellites" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0014), "Status Post training critical speakers Status" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0079), "Actual Version Of Navi Data Base version number" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0020), "HDD Info Model Name" },
    { "headunit-hu204", UINT8_C(0x21), UINT16_C(0x0005), "AUX Level from RSE Status" },
    { "fuel-pump-fscu", UINT8_C(0x22), UINT16_C(0x000A), "Analogue values battery voltage" },
    { "fuel-pump-fscu", UINT8_C(0x22), UINT16_C(0x0009), "Fuel pressure target value" },
    { "fuel-pump-fscu", UINT8_C(0x22), UINT16_C(0x000B), "Physical values battery voltage" },
    { "fuel-pump-fscu", UINT8_C(0x22), UINT16_C(0x000C), "Fuel pump running status" },
    { "fuel-pump-fscu", UINT8_C(0x22), UINT16_C(0x001C), "Fuel pump development running status" },
    { "fuel-pump-fscu", UINT8_C(0x22), UINT16_C(0x1001), "Coding data set" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2005), "CAN Coding ALDW Signal" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2004), "CAN data ESP OFF switch" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2047), "DGR mode misuse" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2009), "ECO actual current" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x200E), "LRG output ART regulating" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x200D), "LRG input ART present" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2008), "PML actual current" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x200A), "PRE-SAFE actuator stage" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2014), "PRW learning status byte 1 range 1 learned" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2011), "PRW reactivation reason 1" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2015), "PRW Status 0x00 0x07" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2010), "PRW warning km 1" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2001), "Wheel information rear left" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2046), "Roller test mode activation type" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2007), "Sensor offset yaw rate" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2018), "Sensor cluster Ax/Ay calibration status" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2002), "Analogue data yaw rate" },
    { "esp-abr2xt", UINT8_C(0x22), UINT16_C(0x2003), "Digital data ASV1" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0x0026), "Bus watchdog events count" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0x0025), "Engine type" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0xD243), "VIN odometer counter" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0xD210), "Installed ECUs actual body" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0xD211), "Installed ECUs actual chassis" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0xD213), "Installed ECUs actual diagnostic" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0xD212), "Installed ECUs actual impact" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0x0310), "Installed ECUs target body" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0x0311), "Installed ECUs target chassis" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0x0313), "Installed ECUs target diagnostic" },
    { "gateway-cgw212", UINT8_C(0x22), UINT16_C(0x0312), "Installed ECUs target impact" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0201), "Current model-series information" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0215), "External CAN voltage" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0213), "Temperature histogram 00" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0212), "External CAN temperature" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0220), "Vehicle dynamics speed" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0232), "ALDW usage counters ALDW Function Prerequests fulfilled" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0230), "IHC usage counters IHC Function Prerequests fulfilled" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0231), "SLA usage counters SLA Function Prerequests fulfilled" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0210), "Windscreen heating activation status" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0233), "Heating active time in current cycle" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0240), "Post-crash bit PCB set" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0242), "Calibration result" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0250), "CAN vehicle information ADC Avl" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0209), "Windscreen heating availability" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0251), "CAN common vehicle information model series" },
    { "camera-mfk", UINT8_C(0x22), UINT16_C(0x0254), "Variant coding status VC ALDW" },
};

static const char *cbf_display_name(
    const char *profile_key,
    uint8_t service,
    uint16_t identifier)
{
    size_t index;
    if (profile_key == NULL) return NULL;
    for (index = 0U;
         index < sizeof(cbf_display_names) / sizeof(cbf_display_names[0]);
         ++index) {
        const CbfDisplayName *entry = &cbf_display_names[index];
        if (entry->service == service &&
            entry->identifier == identifier &&
            strcmp(entry->profile_key, profile_key) == 0) {
            return entry->display_name;
        }
    }
    return NULL;
}


static bool can_read(const Reader *reader, size_t count)
{
    return reader != NULL && reader->ok &&
        reader->pos <= reader->size && count <= reader->size - reader->pos;
}

static uint16_t read_u16(Reader *reader)
{
    uint16_t value = 0U;
    if (!can_read(reader, 2U)) {
        if (reader != NULL) reader->ok = false;
        return 0U;
    }
    value = (uint16_t)reader->data[reader->pos] |
        (uint16_t)((uint16_t)reader->data[reader->pos + 1U] << 8U);
    reader->pos += 2U;
    return value;
}

static int16_t read_i16(Reader *reader)
{
    return (int16_t)read_u16(reader);
}

static uint32_t read_u32(Reader *reader)
{
    uint32_t value = 0U;
    if (!can_read(reader, 4U)) {
        if (reader != NULL) reader->ok = false;
        return 0U;
    }
    value = (uint32_t)reader->data[reader->pos] |
        ((uint32_t)reader->data[reader->pos + 1U] << 8U) |
        ((uint32_t)reader->data[reader->pos + 2U] << 16U) |
        ((uint32_t)reader->data[reader->pos + 3U] << 24U);
    reader->pos += 4U;
    return value;
}

static int32_t read_i32(Reader *reader)
{
    return (int32_t)read_u32(reader);
}

static bool seek_to(Reader *reader, uint64_t offset)
{
    if (reader == NULL || offset > reader->size) {
        if (reader != NULL) reader->ok = false;
        return false;
    }
    reader->pos = (size_t)offset;
    return true;
}

static bool add_relative_offset(
    uint64_t base, int32_t relative, size_t limit, uint64_t *absolute)
{
    int64_t resolved;

    if (absolute == NULL || base > (uint64_t)INT64_MAX)
        return false;
    resolved = (int64_t)base + (int64_t)relative;
    if (resolved < 0 || (uint64_t)resolved > (uint64_t)limit)
        return false;
    *absolute = (uint64_t)resolved;
    return true;
}

static bool next_flag(uint32_t *flags)
{
    bool set;
    if (flags == NULL) return false;
    set = (*flags & UINT32_C(1)) != 0U;
    *flags >>= 1U;
    return set;
}

static int32_t read_flag_i32(Reader *reader, uint32_t *flags, int32_t fallback)
{
    return next_flag(flags) ? read_i32(reader) : fallback;
}

static int16_t read_flag_i16(Reader *reader, uint32_t *flags, int16_t fallback)
{
    return next_flag(flags) ? read_i16(reader) : fallback;
}

static uint16_t read_flag_u16(Reader *reader, uint32_t *flags, uint16_t fallback)
{
    return next_flag(flags) ? read_u16(reader) : fallback;
}

static const char *string_at(const Reader *reader, uint64_t offset)
{
    size_t index;
    if (reader == NULL || offset >= reader->size) return NULL;
    for (index = (size_t)offset; index < reader->size; ++index) {
        if (reader->data[index] == 0U)
            return (const char *)&reader->data[(size_t)offset];
    }
    return NULL;
}

static const char *read_flag_string(
    Reader *reader, uint32_t *flags, uint32_t base)
{
    int32_t relative;
    int64_t absolute;
    if (!next_flag(flags)) return NULL;
    relative = read_i32(reader);
    absolute = (int64_t)base + (int64_t)relative;
    if (!reader->ok || absolute < 0) {
        reader->ok = false;
        return NULL;
    }
    return string_at(reader, (uint64_t)absolute);
}

static bool parse_cff_header(Reader *reader, CffHeader *header)
{
    uint32_t flags;
    if (reader == NULL || header == NULL ||
        !seek_to(reader, CBF_STUB_SIZE)) return false;

    memset(header, 0, sizeof(*header));
    header->cff_size = read_u32(reader);
    header->base = (uint32_t)reader->pos;
    flags = (uint32_t)read_u16(reader);

    header->caesar_version = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0); /* GPD version */
    header->ecu_count = read_flag_i32(reader, &flags, 0);
    header->ecu_offset = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0); /* CTF offset */
    header->string_pool_size = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0); /* DSC offset */
    (void)read_flag_i32(reader, &flags, 0); /* DSC count */
    (void)read_flag_i32(reader, &flags, 0); /* DSC entry size */
    (void)read_flag_string(reader, &flags, header->base);
    (void)read_flag_string(reader, &flags, header->base);

    return reader->ok && header->caesar_version >= 400 &&
        header->ecu_count > 0 && header->ecu_count < 4096 &&
        header->ecu_offset > 0 && header->string_pool_size > 0;
}

static bool skip_ecu_prefix(
    Reader *reader, uint32_t *flags, uint32_t base)
{
    (void)read_flag_i32(reader, flags, -1); /* name */
    (void)read_flag_i32(reader, flags, -1); /* description */
    (void)read_flag_string(reader, flags, base); /* XML version */
    (void)read_flag_i32(reader, flags, 0); /* interface count */
    (void)read_flag_i32(reader, flags, 0); /* interface offset */
    (void)read_flag_i32(reader, flags, 0); /* subtype count */
    (void)read_flag_i32(reader, flags, 0); /* subtype offset */
    (void)read_flag_string(reader, flags, base); /* class */
    (void)read_flag_string(reader, flags, base); /* unknown */
    (void)read_flag_string(reader, flags, base); /* unknown */
    (void)read_flag_i16(reader, flags, 0); /* ignition */
    (void)read_flag_i16(reader, flags, 0);
    (void)read_flag_i16(reader, flags, 0);
    (void)read_flag_i32(reader, flags, 0);
    (void)read_flag_i16(reader, flags, 0);
    (void)read_flag_i32(reader, flags, 0);
    return reader->ok;
}

static bool parse_ecu_header(
    Reader *reader, uint32_t base, const CffHeader *cff, EcuHeader *header)
{
    uint32_t flags;
    uint64_t data_base;
    int32_t diag_relative;

    if (reader == NULL || cff == NULL || header == NULL ||
        !seek_to(reader, base)) return false;

    memset(header, 0, sizeof(*header));
    flags = read_u32(reader);
    (void)read_u16(reader); /* extended flags begin after fields we need */
    (void)read_i32(reader); /* ECU header id */

    header->qualifier = read_flag_string(reader, &flags, base);
    if (!skip_ecu_prefix(reader, &flags, base)) return false;

    data_base = (uint64_t)(uint32_t)cff->string_pool_size +
        CBF_STUB_SIZE + (uint64_t)cff->cff_size + UINT64_C(4);
    if (data_base > UINT32_MAX) return false;

    /* Variant pool. */
    (void)read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);

    /* Diagnostic-service pool. */
    diag_relative = read_flag_i32(reader, &flags, 0);
    header->diag_count = read_flag_i32(reader, &flags, 0);
    header->diag_entry_size = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);

    if (!reader->ok || header->diag_count < 0 ||
        header->diag_entry_size < 14) return false;
    {
        uint64_t diag_block;
        if (!add_relative_offset(
                data_base, diag_relative, reader->size, &diag_block) ||
            diag_block > UINT32_MAX)
            return false;
        header->diag_block = (uint32_t)diag_block;
    }
    return header->qualifier != NULL;
}

static bool parse_service(
    Reader *reader, uint32_t base, DiagService *service)
{
    uint32_t flags;

    if (reader == NULL || service == NULL || !seek_to(reader, base))
        return false;

    memset(service, 0, sizeof(*service));
    flags = read_u32(reader);
    (void)read_u32(reader); /* extended flags not needed for request identity */

    service->qualifier = read_flag_string(reader, &flags, base);
    (void)read_flag_i32(reader, &flags, -1); /* name CTF */
    (void)read_flag_i32(reader, &flags, -1); /* description CTF */
    service->type = read_flag_u16(reader, &flags, 0U);
    service->executable = read_flag_u16(reader, &flags, 0U);
    service->client_access = read_flag_u16(reader, &flags, 0U);
    service->security_access = read_flag_u16(reader, &flags, 0U);

    (void)read_flag_i32(reader, &flags, 0); /* comparam count */
    (void)read_flag_i32(reader, &flags, 0); /* comparam offset */
    (void)read_flag_i32(reader, &flags, 0); /* Q count */
    (void)read_flag_i32(reader, &flags, 0); /* Q offset */
    (void)read_flag_i32(reader, &flags, 0); /* R count */
    (void)read_flag_i32(reader, &flags, 0); /* R offset */
    (void)read_flag_string(reader, &flags, base); /* input ref */
    (void)read_flag_i32(reader, &flags, 0); /* prep count */
    (void)read_flag_i32(reader, &flags, 0); /* prep offset */
    (void)read_flag_i32(reader, &flags, 0); /* V count */
    (void)read_flag_i32(reader, &flags, 0); /* V offset */
    service->request_count = read_flag_i16(reader, &flags, 0);
    service->request_offset = read_flag_i32(reader, &flags, 0);

    return reader->ok && service->qualifier != NULL;
}

static uint8_t *read_file(const char *path, size_t *size_out)
{
    FILE *file;
    long length;
    uint8_t *buffer;
    size_t read_count;

    if (path == NULL || size_out == NULL) return NULL;
    file = fopen(path, "rb");
    if (file == NULL) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length <= 0 || (uint64_t)length > SIZE_MAX) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    buffer = malloc((size_t)length);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    read_count = fread(buffer, 1U, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) {
        free(buffer);
        return NULL;
    }
    *size_out = (size_t)length;
    return buffer;
}

static void emit_c_string(const char *value)
{
    const unsigned char *cursor =
        (const unsigned char *)(value != NULL ? value : "");

    putchar('"');
    while (*cursor != 0U) {
        const unsigned char ch = *cursor++;
        if (ch == (unsigned char)'"' || ch == (unsigned char)'\\') {
            putchar('\\');
            putchar((int)ch);
        } else if (ch >= UINT8_C(0x20) && ch <= UINT8_C(0x7e)) {
            putchar((int)ch);
        } else {
            printf("\\%03o", (unsigned int)ch);
        }
    }
    putchar('"');
}

static void emit_fallback_display_name(uint8_t service, uint16_t identifier)
{
    char fallback[64];
    const int written = snprintf(
        fallback, sizeof(fallback), "Mercedes data %02" PRIX8 " %04" PRIX16,
        service, identifier);
    if (written < 0 || (size_t)written >= sizeof(fallback)) {
        emit_c_string("Mercedes documented data");
        return;
    }
    emit_c_string(fallback);
}

static int emit_direct_read_data(
    const uint8_t *data,
    size_t size,
    const char *profile_key,
    const char *source_name)
{
    Reader reader = { data, size, 0U, true };
    CffHeader cff;
    uint64_t ecu_table;
    int32_t ecu_index;
    unsigned int emitted = 0U;
    bool *seen = NULL;
    const bool c_profile =
        profile_key != NULL && source_name != NULL;
    const size_t identifiers_per_service = (size_t)UINT16_MAX + (size_t)1U;
    const size_t seen_count =
        ((size_t)UINT8_MAX + (size_t)1U) * identifiers_per_service;

    if (!parse_cff_header(&reader, &cff)) {
        fprintf(stderr, "invalid or unsupported CBF header\n");
        return 2;
    }

    ecu_table = (uint64_t)cff.base + (uint64_t)(uint32_t)cff.ecu_offset;
    if (ecu_table > size) {
        fprintf(stderr, "invalid ECU table offset\n");
        return 2;
    }

    if (c_profile) {
        seen = calloc(seen_count, sizeof(*seen));
        if (seen == NULL) {
            fprintf(stderr, "failed to allocate read-key deduplication table\n");
            return 2;
        }
    } else {
        puts("ecu\tservice\tidentifier\tqualifier\tclient_access\tsecurity_access");
    }

    for (ecu_index = 0; ecu_index < cff.ecu_count; ++ecu_index) {
        EcuHeader ecu;
        uint64_t table_entry =
            ecu_table + (uint64_t)(uint32_t)ecu_index * UINT64_C(4);
        int32_t ecu_relative;
        int32_t service_index;

        if (!seek_to(&reader, table_entry)) {
            free(seen);
            return 2;
        }
        ecu_relative = read_i32(&reader);
        {
            uint64_t ecu_base;
            if (!reader.ok ||
                !add_relative_offset(
                    ecu_table, ecu_relative, size, &ecu_base) ||
                ecu_base > UINT32_MAX) {
                free(seen);
                return 2;
            }
            if (!parse_ecu_header(
                    &reader, (uint32_t)ecu_base, &cff, &ecu)) {
                fprintf(stderr, "failed to parse ECU %" PRId32 "\n", ecu_index);
                free(seen);
                return 2;
            }
        }

        for (service_index = 0; service_index < ecu.diag_count;
             ++service_index) {
            uint64_t entry = (uint64_t)ecu.diag_block +
                (uint64_t)(uint32_t)service_index *
                (uint64_t)(uint32_t)ecu.diag_entry_size;
            int32_t service_relative;
            uint64_t service_base;
            DiagService service;
            uint64_t request_base;
            uint8_t wire_service;
            uint16_t identifier;

            if (entry + UINT64_C(14) > size || !seek_to(&reader, entry)) {
                free(seen);
                return 2;
            }
            service_relative = read_i32(&reader);
            (void)read_i32(&reader); /* entry size */
            (void)read_u32(&reader); /* CRC */
            (void)read_u16(&reader); /* config */
            if (!reader.ok ||
                !add_relative_offset(
                    (uint64_t)ecu.diag_block, service_relative,
                    size, &service_base) ||
                service_base > UINT32_MAX ||
                !parse_service(&reader, (uint32_t)service_base, &service)) {
                free(seen);
                return 2;
            }

            /* Caesar service class 5 is Data. Keep only direct safe reads. */
            if (service.type != 5U || service.executable == 0U ||
                service.request_count <= 0) {
                continue;
            }

            if (!add_relative_offset(
                    service_base, service.request_offset,
                    size, &request_base) ||
                request_base > (uint64_t)size ||
                (uint64_t)(uint16_t)service.request_count >
                    (uint64_t)size - request_base) {
                free(seen);
                return 2;
            }

            wire_service = data[(size_t)request_base];
            if (wire_service == UINT8_C(0x22) &&
                service.request_count == 3) {
                identifier = (uint16_t)(
                    (uint16_t)data[(size_t)request_base + 1U] << 8U) |
                    (uint16_t)data[(size_t)request_base + 2U];
            } else if ((wire_service == UINT8_C(0x21) ||
                        wire_service == UINT8_C(0x1a)) &&
                       service.request_count == 2) {
                identifier = (uint16_t)data[(size_t)request_base + 1U];
            } else {
                continue;
            }

            if (c_profile) {
                const size_t seen_index =
                    (size_t)wire_service * identifiers_per_service +
                    (size_t)identifier;
                if (seen[seen_index]) continue;
                seen[seen_index] = true;

                fputs("    { ", stdout);
                emit_c_string(profile_key);
                fputs(", ", stdout);
                if (wire_service == UINT8_C(0x22)) {
                    fputs("MBLINK_MERCEDES_DIAGNOSTIC_UDS,\n        ", stdout);
                } else {
                    fputs("MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,\n        ", stdout);
                }
                printf("UINT8_C(0x%02" PRIX8 "), UINT16_C(0x%04" PRIX16
                       "), true, ",
                    wire_service, identifier);
                {
                    const char *display_name = cbf_display_name(
                        profile_key, wire_service, identifier);
                    if (display_name != NULL)
                        emit_c_string(display_name);
                    else
                        emit_fallback_display_name(wire_service, identifier);
                }
                fputs(",\n"
                      "        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,\n"
                      "        ", stdout);
                {
                    char provenance[1024];
                    const int written = snprintf(
                        provenance, sizeof(provenance),
                        "Mercedes Vediamo %s · CBF Data service %s · %02" PRIX8
                        " %04" PRIX16,
                        source_name,
                        service.qualifier != NULL ? service.qualifier : "",
                        wire_service, identifier);
                    if (written < 0 ||
                        (size_t)written >= sizeof(provenance)) {
                        fprintf(stderr, "CBF provenance text is too long\n");
                        free(seen);
                        return 2;
                    }
                    emit_c_string(provenance);
                }
                fputs(" },\n", stdout);
            } else {
                printf("%s\t%02" PRIX8 "\t%04" PRIX16 "\t%s\t%" PRIu16
                       "\t%" PRIu16 "\n",
                    ecu.qualifier, wire_service, identifier, service.qualifier,
                    service.client_access, service.security_access);
            }
            ++emitted;
        }
    }

    free(seen);
    fprintf(stderr, "direct_read_rows=%u\n", emitted);
    return 0;
}

int main(int argc, char **argv)
{
    uint8_t *data;
    size_t size = 0U;
    int result;
    const char *profile_key = NULL;
    const char *source_name = NULL;
    const char *path = NULL;

    if (argc == 2) {
        path = argv[1];
    } else if (argc == 5 && strcmp(argv[1], "--profile") == 0) {
        profile_key = argv[2];
        source_name = argv[3];
        path = argv[4];
    } else {
        fprintf(stderr,
            "usage: %s FILE.cbf\n"
            "       %s --profile PROFILE_KEY SOURCE_NAME FILE.cbf\n",
            argv[0], argv[0]);
        return 64;
    }

    data = read_file(path, &size);
    if (data == NULL) {
        fprintf(stderr, "failed to read %s: %s\n",
            path, strerror(errno));
        return 66;
    }

    result = emit_direct_read_data(
        data, size, profile_key, source_name);
    free(data);
    return result;
}
