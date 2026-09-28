// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * Generated family-isolated EGS52 lookup.
 * Source: rnd-ash/ultimate-nag52-fw/lib/egs52_ecus/can_data.txt
 * Revision: 1b96089660e97c91811b3d9cda6ca6f82b458c69
 * Upstream can_data.txt declares the original ECU bit layout big endian.
 */
#include "mblink/mercedes_egs52_lookup.h"
#include <string.h>

static const MblinkMercedesEgs52EnumValue f0s10e[] = {
 { UINT64_C(0), "BREMSE_NBET", "Brake not actuated" },
 { UINT64_C(1), "BREMSE_BET", "brake actuated" },
 { UINT64_C(2), "UNKNOWN", "not defined" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f0s11e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f0s13e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f0s15e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f0s[] = {
 { "BRE_KL",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake defective control lamp (EBV_KL at 463/461 / NCV2)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BAS_KL",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Bas defective control lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_INFO_BL",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP Infolramp flashing light",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_INFO_DL",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP Info lamp permanent light",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_KL",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP defective control lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ABS_KL",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ABS defective control lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BBV_KL",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"brake pad wear control lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLS_UNT",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake light suppression (EBV_KL at 163 / T0 / T1N)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLS_PA",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"BLS Parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BZ200h",UINT16_C(10),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "BLS",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"brake light switch",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f0s10e,sizeof(f0s10e)/sizeof(f0s10e[0]) },
 { "DRTGVL",UINT16_C(16),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"rotary direction wheel front left",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f0s11e,sizeof(f0s11e)/sizeof(f0s11e[0]) },
 { "DVL",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"wheel speed front left",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGVR",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"direction of rotation wheel front right",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f0s13e,sizeof(f0s13e)/sizeof(f0s13e[0]) },
 { "DVR",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Right speed front right",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGTM",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Rad Left for Cruise",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f0s15e,sizeof(f0s15e)/sizeof(f0s15e[0]) },
 { "TM_DL",UINT16_C(50),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"wheel speed links for cruise control",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f1s2e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, upper limit = 1" },
 { UINT64_C(2), "G2", "Gear, upper limit = 2" },
 { UINT64_C(3), "G3", "Gear, upper limit = 3" },
 { UINT64_C(4), "G4", "Gear, upper limit = 4" },
 { UINT64_C(5), "G5", "Gear, upper limit = 5" },
 { UINT64_C(6), "G6", "Gear, upper limit = 6" },
 { UINT64_C(7), "G7", "Gear, upper limit = 7" },
};
static const MblinkMercedesEgs52EnumValue f1s3e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, lower limit = 1" },
 { UINT64_C(2), "G2", "Gear, lower limit = 2" },
 { UINT64_C(3), "G3", "Gear, lower limit = 3" },
 { UINT64_C(4), "G4", "Gear, lower limit = 4" },
 { UINT64_C(5), "G5", "Gear, lower limit = 5" },
 { UINT64_C(6), "G6", "Gear, lower limit = 6" },
 { UINT64_C(7), "G7", "Gear, lower limit = 7" },
};
static const MblinkMercedesEgs52EnumValue f1s5e[] = {
 { UINT64_C(0), "ERR", "system error" },
 { UINT64_C(1), "NORM", "normal operation" },
 { UINT64_C(2), "DIAG", "Diagnosis" },
 { UINT64_C(3), "ABGAS", "exhaust gas test" },
};
static const MblinkMercedesEgs52EnumValue f1s7e[] = {
 { UINT64_C(0), "SKL0", "Shift characteristic \"0\"" },
 { UINT64_C(1), "SKL1", "Shift characteristic \"1\"" },
 { UINT64_C(2), "SKL2", "Shift characteristic \"2\"" },
 { UINT64_C(3), "SKL3", "Shift characteristic \"3\"" },
 { UINT64_C(4), "SKL4", "Shift characteristic \"4\"" },
 { UINT64_C(5), "SKL5", "Shift characteristic \"5\"" },
 { UINT64_C(6), "SKL6", "Shift characteristic \"6\"" },
 { UINT64_C(7), "SKL7", "Shift characteristic \"7\"" },
 { UINT64_C(8), "SKL8", "Shift characteristic \"8\"" },
 { UINT64_C(9), "SKL9", "Shift characteristic \"9\"" },
 { UINT64_C(10), "SKL10", "Shift characteristic \"10\"" },
};
static const MblinkMercedesEgs52EnumValue f1s9e[] = {
 { UINT64_C(0), "UNKNOWN", "not defined" },
 { UINT64_C(1), "ANF_N", "requirement \"neutral\"" },
 { UINT64_C(2), "IDLE", "No requirement" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f1s12e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f1s14e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f1s[] = {
 { "AKT_R_ESP",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP / Art-Wish: \"Active Retract\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MINMAX_ART",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gear requirement of art",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GMAX_ESP",UINT16_C(2),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, upper limit",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s2e,sizeof(f1s2e)/sizeof(f1s2e[0]) },
 { "GMIN_ESP",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, lower limit",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s3e,sizeof(f1s3e)/sizeof(f1s3e[0]) },
 { "DDYN_UNT",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Suppression Dynamic fully detection",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SZS",UINT16_C(9),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"system condition",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s5e,sizeof(f1s5e)/sizeof(f1s5e[0]) },
 { "TM_AUS",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomat operation",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SLV_ESP",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Switching Difference ESP",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s7e,sizeof(f1s7e)/sizeof(f1s7e[0]) },
 { "BRE_AKT_ESP",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP brake engagement active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANFN",UINT16_C(17),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"ESP request: \"N\" Insert",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s9e,sizeof(f1s9e)/sizeof(f1s9e[0]) },
 { "BRE_AKT_ART",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ART brake intervention active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MBRE_ESP",UINT16_C(20),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"set braking torque (BR240 factor 1.8 larger)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGHR",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"rotary direction wheel rear right",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s12e,sizeof(f1s12e)/sizeof(f1s12e[0]) },
 { "DHR",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Rear wheel speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGHL",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"rotary direction wheel rear left",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f1s14e,sizeof(f1s14e)/sizeof(f1s14e[0]) },
 { "DHL",UINT16_C(50),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Rear wheel speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f2s2e[] = {
 { UINT64_C(0), "OK", "No warning" },
 { UINT64_C(1), "WARN_OHNE", "Tire pressure warning without position specification" },
 { UINT64_C(2), "PRW_NV", "PRW not available" },
 { UINT64_C(3), "PRW_START", "Restart PRW" },
 { UINT64_C(4), "WARN_VL", "Tire pressure warning front left" },
 { UINT64_C(5), "WARN_VR", "Tire pressure warning front right" },
 { UINT64_C(6), "WARN_HL", "Tire pressure warning rear left" },
 { UINT64_C(7), "WARN_HR", "Tire pressure warning rear right" },
 { UINT64_C(8), "UNKNOWN_1", "not defined" },
 { UINT64_C(14), "UNKNOWN_2", "not defined" },
 { UINT64_C(15), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f2s3e[] = {
 { UINT64_C(0), "EIN", "PRW active, no warning" },
 { UINT64_C(1), "WARN", "PRW active, warning is available" },
 { UINT64_C(2), "AUS", "PRW inactive or not available" },
 { UINT64_C(3), "INIT", "PRW is initialized" },
 { UINT64_C(4), "UNKNOWN_1", "not defined" },
 { UINT64_C(5), "UNKNOWN_2", "not defined" },
 { UINT64_C(6), "PRW_NV", "PRW not available" },
 { UINT64_C(7), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f2s[] = {
 { "RIZ_HL",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Impulse ring counter wheel rear left (48 per revolution)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "RIZ_HR",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Impulse ring counter wheel rear right (48 per revolution)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "PRW_WARN",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Alerts PlatRollwarner",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f2s2e,sizeof(f2s2e)/sizeof(f2s2e[0]) },
 { "PRW_ST",UINT16_C(21),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Status flat tyre warner",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f2s3e,sizeof(f2s3e)/sizeof(f2s3e[0]) },
};
static const MblinkMercedesEgs52EnumValue f3s11e[] = {
 { UINT64_C(0), "UNKNOWN", "not defined" },
 { UINT64_C(1), "T20_0", "send cycle time 20 ms" },
 { UINT64_C(2), "T23_1", "send cycle time 23.1 ms" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f3s13e[] = {
 { UINT64_C(0), "BREMSE_NEIN", "driver does not slower" },
 { UINT64_C(1), "BREMSE_JA", "driver brakes" },
 { UINT64_C(2), "UNKNOWN", "not defined" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f3s[] = {
 { "DMPAR_ART",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque Request Parity (just parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DMDYN_ART",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request dynamic",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BAS_AKT",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Bas-control active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VOLLBRE",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"full braking (ABS regulates all 4 wheels)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_E",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Art",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_GIER_AKT",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP giermom control active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWS_INI_OK",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Initialization Steering Angle Sensor O.K.",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWS_INI_EIN",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Initialization steering angle sensor possible",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MPAR_ESP",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque Request Parity (just parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MDYN_ESP",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request dynamic",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AMR_AKT_ESP",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"drive torque control active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "T_Z",UINT16_C(11),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Send cycle time",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f3s11e,sizeof(f3s11e)/sizeof(f3s11e[0]) },
 { "SFB_PA",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"driver brakes parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SFB",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"driver brakes",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f3s13e,sizeof(f3s13e)/sizeof(f3s13e[0]) },
 { "DMTGL_ART",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor torque toggle 40ms + -10",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DMMIN_ART",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request min",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DMMAX_ART",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request max",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DM_ART",UINT16_C(19),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Ford.Engine torque",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "MTGL_ESP",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor torque toggle 40ms + -10",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MMIN_ESP",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request min",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MMAX_ESP",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request max",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_ESP",UINT16_C(35),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Ford.Engine torque",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "GIER_ROH",UINT16_C(48),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"raw signal yaw rate without reconciliation / filtering (+ = left)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f4s3e[] = {
 { UINT64_C(0), "OK", "No error" },
 { UINT64_C(1), "ESP_DEF", "ESP defective" },
 { UINT64_C(2), "ESP_OFF", "ESP not available" },
 { UINT64_C(3), "ESP_OFF_DS", "ESP not available, DifferentialSp. active (only 463/461)" },
 { UINT64_C(4), "BAS_DEF", "Bas defective" },
 { UINT64_C(5), "ABS_DEF", "ABS defective (only 463/461)" },
 { UINT64_C(6), "ABS_OFF", "ABS Not available (463/461 only)" },
 { UINT64_C(7), "BKV_DEF_GBV", "BKV defective (463/461 only)" },
 { UINT64_C(8), "ALL_OFF_DS", "ABS, BAS u. ESP Not available (463/461 only)" },
 { UINT64_C(9), "SBCS_DEF", "Stop & Roll defective" },
 { UINT64_C(10), "SBCS_ON", "Stop & Roll" },
 { UINT64_C(11), "SBCH_N_AKT", "SBC HOLD not activatable" },
 { UINT64_C(13), "ESP_BAS_DEF", "BAS u. ESP defective" },
 { UINT64_C(14), "SBCH_OFF", "SBC HOLD OFF" },
 { UINT64_C(15), "SBCH", "SBC HOLD" },
 { UINT64_C(16), "ALL_DEF", "ABS, BAS u. ESP defective" },
 { UINT64_C(17), "ALL_DEF_GBV", "ABS, BAS, ESP u. BKV defective" },
 { UINT64_C(19), "ALL_DIAG", "ABS, BAS u. ESP DIAG. Test." },
 { UINT64_C(20), "ALL_DIAG_GBV", "ABS, BAS, ESP u. BKV DIAG. Test" },
 { UINT64_C(22), "ALL_OFF", "ABS, BAS u. ESP not available" },
 { UINT64_C(23), "ALL_OFF_GBV", "ABS, BAS, ESP u. BKV not available" },
 { UINT64_C(24), "SBCS", "SBC stop" },
 { UINT64_C(25), "SBCS_OFF", "SBC stop out" },
 { UINT64_C(26), "BRAKE", "Brake immediately!" },
 { UINT64_C(27), "SBCS_N_AKT", "SBC stop not activatable" },
 { UINT64_C(28), "SBCH_DEF", "SBC HOLD defective" },
 { UINT64_C(29), "SBCS_DEF2", "SBC Stop defective" },
 { UINT64_C(30), "GWH_P", "selector lever according to P" },
 { UINT64_C(31), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f4s[] = {
 { "WMS_PA",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"WMS Parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WMS",UINT16_C(1),UINT16_C(15),false,UINT64_C(0),UINT8_C(0),"target wobble moment change",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "AY_S",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Vehicle lateral acceleration. The focus (+ = left)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ESP_DSPL",UINT16_C(35),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"ESP display messages",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f4s3e,sizeof(f4s3e)/sizeof(f4s3e[0]) },
 { "NOTBRE",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Emergency braking (brake light blink)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KPL_OEF",UINT16_C(44),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Open clutch",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BZ328h",UINT16_C(45),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "RIZ_VL",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Impulse ring counter wheel front left (48 per revolution)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "RIZ_VR",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Impulse ring counter wheel front right (48 per revolution)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f5s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f6s[] = {
 { "SD_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"system diagnostic response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f7s[] = {
 { "APPL2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f8s[] = {
 { "FBS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS Embassy to EZS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f9s[] = {
 { "MS_FBS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS Embassy to EZS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f10s1e[] = {
 { UINT64_C(0), "SKL0", "Shift characteristic \"0\"" },
 { UINT64_C(1), "SKL1", "Shift characteristic \"1\"" },
 { UINT64_C(2), "SKL2", "Shift characteristic \"2\"" },
 { UINT64_C(3), "SKL3", "Shift characteristic \"3\"" },
 { UINT64_C(4), "SKL4", "Shift characteristic \"4\"" },
 { UINT64_C(5), "SKL5", "Shift characteristic \"5\"" },
 { UINT64_C(6), "SKL6", "Shift characteristic \"6\"" },
 { UINT64_C(7), "SKL7", "Shift characteristic \"7\"" },
 { UINT64_C(8), "SKL8", "Shift characteristic \"8\"" },
 { UINT64_C(9), "SKL9", "Shift characteristic \"9\"" },
 { UINT64_C(10), "SKL10", "Shift characteristic \"10\"" },
};
static const MblinkMercedesEgs52EnumValue f10s6e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, upper limit = 1" },
 { UINT64_C(2), "G2", "Gear, upper limit = 2" },
 { UINT64_C(3), "G3", "Gear, upper limit = 3" },
 { UINT64_C(4), "G4", "Gear, upper limit = 4" },
 { UINT64_C(5), "G5", "Gear, upper limit = 5" },
 { UINT64_C(6), "G6", "Gear, upper limit = 6" },
 { UINT64_C(7), "G7", "Gear, upper limit = 7" },
};
static const MblinkMercedesEgs52EnumValue f10s7e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, lower limit = 1" },
 { UINT64_C(2), "G2", "Gear, lower limit = 2" },
 { UINT64_C(3), "G3", "Gear, lower limit = 3" },
 { UINT64_C(4), "G4", "Gear, lower limit = 4" },
 { UINT64_C(5), "G5", "Gear, lower limit = 5" },
 { UINT64_C(6), "G6", "Gear, lower limit = 6" },
 { UINT64_C(7), "G7", "Gear, lower limit = 7" },
};
static const MblinkMercedesEgs52SignalDefinition f10s[] = {
 { "KOMP_NOTAUS",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air compressor Emergency Shutdown",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SLV_MS",UINT16_C(1),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"switching line shift MS",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f10s1e,sizeof(f10s1e)/sizeof(f10s1e[0]) },
 { "KRIECH_AUS",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch KSG-creep",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANF1",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"MS-wish: \"Approach 1.Gang\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AKT_R_MS",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"MS-wish: \"Active downshift\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZH_AUS_MS",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn off heater",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GMAX_MS",UINT16_C(10),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, upper limit",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f10s6e,sizeof(f10s6e)/sizeof(f10s6e[0]) },
 { "GMIN_MS",UINT16_C(13),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, lower limit",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f10s7e,sizeof(f10s7e)/sizeof(f10s7e[0]) },
 { "PW",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"pedal",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "V_DSPL_NEU",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"retrigger minimum display time in the display: S",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LL_STBL",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"idle is stable",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VGL_ST",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Vorglühstatus",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MSS_DEF",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Start / Stop system is defective",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MSS_KL",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"engine start / stop system warning",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MSS_AKT",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"engine start / stop system active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KOMP_BAUS",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"turn air compressor:: S acceleration",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRASH_MS",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Crash signal from motor control",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PWG_ERR",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"error pedal sensor",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LL",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"idle",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KUEB_S_A",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"beg. \"Slip\" lock-up clutch",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TM_REG",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control regulates",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_MAX_EIN",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"activated speed limit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KD_MS",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Kick Down (changeover scenario open!)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NOTL",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"emergency operation",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_MAX_SUM",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Warning buzzer",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FBS_SE",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"FBStart Error",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_PGB",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"\"achieved winter tires limitation\" Indicated on display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TM_EIN",UINT16_C(42),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"activated cruise control",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_MAX_REG",UINT16_C(43),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed ​​controls",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_LIM",UINT16_C(44),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display \"limit?\" on display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_ERR",UINT16_C(45),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"\"Error\" indicator on the display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_BL",UINT16_C(46),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"display flashes",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_EIN",UINT16_C(47),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Geschw.begrenzer- / cruise control display a",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FMMOTMAX",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"factor for altitude value. d. max. Mom with remo.. A.druck",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "V_MAX_TM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Set maximum or cruise control speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f11s[] = {
 { "NMOTS",UINT16_C(0),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Motorley roll speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "TM_MS",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Serial mpomat is variant encoded",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_ART_E",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable torque requirement type",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_FV",UINT16_C(19),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"default torque driver",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "SME_E",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ENABLE Fast torque setting",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_ESP_E",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable torque requirement ESP",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_FEV",UINT16_C(35),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"replacement feed torque driver",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CALID_CVN_E",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transfer Calid / CVN Enable",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_EGS_Q",UINT16_C(49),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"acknowledgment torque requirement EGS",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_EGS_E",UINT16_C(50),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable torque requirement EGS",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_ESPV",UINT16_C(51),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"default torque ESP",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f12s5e[] = {
 { UINT64_C(0), "ZU", "Heating shut-off valve is too" },
 { UINT64_C(1), "AUF", "Heating shut-off valve is up" },
 { UINT64_C(2), "TAKT", "Heating shut-off valve is clocked" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f12s[] = {
 { "IMIN_MS",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"target translation, lower border (FCVT)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "IMAX_MS",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Target Translation, Upper Border (FCVT)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "KL_61_EIN",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 61",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OEL_INFO_169",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Oil Info, reserved M266",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ASV_KKL_169",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"shut-off valve cooling circuit M266 ATL",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HZL_ST",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Status heating power",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f12s5e,sizeof(f12s5e)/sizeof(f12s5e[0]) },
 { "KID_MS",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Request for power-free in \"D\" (FCVT)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LRS_MODE",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Mode air control system",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LAST_GEN",UINT16_C(26),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Generator utilization (LIN generators only!)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "M_KOMP_MAX",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Max. Climate compressor torque",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "PW_F",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"pedal value driver (only 169)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f13s0e[] = {
 { UINT64_C(32), "BLANK", "Blank (\"\")" },
 { UINT64_C(49), "EINS", "Driving Level \"1\"" },
 { UINT64_C(50), "ZWEI", "Driving Level \"2\"" },
 { UINT64_C(51), "DREI", "Driving Level \"3\"" },
 { UINT64_C(52), "VIER", "Driving Level \"4\"" },
 { UINT64_C(53), "FUENF", "Driving Level \"5\"" },
 { UINT64_C(54), "SECHS", "Driving Level \"6\"" },
 { UINT64_C(55), "SIEBEN", "Driving stage \"7\"" },
 { UINT64_C(65), "A", "Driving stage \"A\"" },
 { UINT64_C(68), "D", "speed \"D\"" },
 { UINT64_C(70), "F", "Error Mark \"F\"" },
 { UINT64_C(78), "N", "Driving \"N\"" },
 { UINT64_C(80), "P", "Driving Level \"P\"" },
 { UINT64_C(82), "R", "Driving \"R\"" },
 { UINT64_C(255), "SNV", "passive value" },
};
static const MblinkMercedesEgs52EnumValue f13s1e[] = {
 { UINT64_C(1), "HOCH", "\"upshift\" / arrow" },
 { UINT64_C(2), "RUNTER", "\"downshift\" / arrow" },
 { UINT64_C(32), "BLANK_OR_PAS", "blank (\"\") / passive" },
 { UINT64_C(49), "EINS", "Driving Level \"1\"" },
 { UINT64_C(50), "ZWEI", "Driving Level \"2\"" },
 { UINT64_C(51), "DREI", "Driving Level \"3\"" },
 { UINT64_C(52), "VIER", "Driving Level \"4\"" },
 { UINT64_C(53), "FUENF", "Driving Level \"5\"" },
 { UINT64_C(54), "SECHS", "Driving Level \"6\"" },
 { UINT64_C(55), "SIEBEN", "Driving stage \"7\"" },
 { UINT64_C(65), "A", "Driving stage \"A\"" },
 { UINT64_C(68), "D", "speed \"D\"" },
 { UINT64_C(70), "F", "Error Mark \"F\"" },
 { UINT64_C(78), "N", "Driving \"N\"" },
 { UINT64_C(80), "P", "Driving Level \"P\"" },
 { UINT64_C(82), "R", "Driving \"R\"" },
 { UINT64_C(255), "SNV", "passive value" },
};
static const MblinkMercedesEgs52SignalDefinition f13s[] = {
 { "FSC_IST",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Drive Level Switching recommendation \"is\"",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f13s0e,sizeof(f13s0e)/sizeof(f13s0e[0]) },
 { "FSC_SOLL",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Drive Level Switching recommendation \"should\"",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f13s1e,sizeof(f13s1e)/sizeof(f13s1e[0]) },
};
static const MblinkMercedesEgs52SignalDefinition f14s[] = {
 { "KPL",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"clutch kicked",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KUEB_O_A",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"start.Convertible bridging clutch \"Open\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "N_MAX_BG",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limiting function active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SAST",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Partinal shutdown",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SASV",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"push shutdown full",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KSF_KL",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fuel filter clogs control lamp (CR2 US only)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WKS_KL",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Water in the fuel control lamp (CR2 US only)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZASBED",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Cylinder shutdown conditions fulfilled",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NMOT",UINT16_C(8),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"engine speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ELHP_WARN",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Warning message ECO steering helping pump",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EOH",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Ethanol operation detected",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LUFI_KL",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air filter dirty warning lamp (only diesel)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VGL_KL",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"pre-glow control lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OEL_KL",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"oil level / oil pressure control lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DIAG_KL",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Diagnosis Control Lamp (OBD II)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TANK_KL",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tank lid open check lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "UEHITZ",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine oil temperature too high (overheating)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZAS",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Cylinder shutdown",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADR_KL",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ADR check lamp (NFZ only)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADR_DEF_KL",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ADR defective control lamp (NFZ only)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANL_LFT",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"starter is running",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LUEFT_MOT_KL",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor Heater Defective Control Lamp",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DBAA",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation for display active (0 at CR)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TEMP_KL",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cooling water temperature too high",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "T_OEL",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Oil temperature",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "OEL_FS",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"oil level",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "OEL_QUAL",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"oil quality",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f15s[] = {
 { "M_STA",UINT16_C(3),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Motor torque static",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "M_MAX_ATL",UINT16_C(19),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Motor Torque Maximum incl. DYN.Turbocharger",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "M_MAX",UINT16_C(35),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Motor torque maximum",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "M_MIN",UINT16_C(51),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Motor torque minimal",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f16s[] = {
 { "FTK_BMI",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"indicator acceleration type (> 100: dynamic)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FTK_LMI",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Code of the transverse acceleration type (> 100: dynamic)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FTK_VMI",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"code number brake type (> 100: dynamic)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FTK_DPW",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Max. Diff.Pedal angle value per maneuver",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "AADKB",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Continuous driver watching",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "AADKBDYN",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Spontaneous dynamic requirement",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AADNT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"nervousness",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f17s2e[] = {
 { UINT64_C(0), "W", "W - Limousine (or G short BM1 / 3 at BR 463, G at 461)" },
 { UINT64_C(1), "V", "V - Limousine long (or VF at BR 210/211, G Lang BM6 at BR 463)" },
 { UINT64_C(2), "C", "C - Coupé (or VV at BR 210/211/220)" },
 { UINT64_C(3), "S", "S - T model (or special protection B4 at BR W240, T at BR 245)" },
 { UINT64_C(4), "A", "A - Cabrio (or X at BR 164)" },
 { UINT64_C(5), "R", "R - Roadster (or special protection B4 at BR 210 / 211/220 / V240)" },
 { UINT64_C(6), "SS", "SS - Special protection B6 / 7 (or CL at BR 203)" },
 { UINT64_C(7), "SNV", "Code not available" },
};
static const MblinkMercedesEgs52EnumValue f17s3e[] = {
 { UINT64_C(0), "BR221", "BR 221 Od. BR 140" },
 { UINT64_C(1), "BR129", "BR 129" },
 { UINT64_C(2), "BR210", "BR 210 Od. BR 212" },
 { UINT64_C(3), "BR202", "BR 202 Od. BR 204" },
 { UINT64_C(4), "BR220", "BR 220" },
 { UINT64_C(5), "BR170", "BR 170" },
 { UINT64_C(6), "BR203", "BR 203" },
 { UINT64_C(7), "BR168", "BR 168" },
 { UINT64_C(8), "BR163", "BR 163" },
 { UINT64_C(9), "BR208", "BR 208" },
 { UINT64_C(10), "BR463", "BR 463" },
 { UINT64_C(11), "BR215", "BR 215" },
 { UINT64_C(12), "BR230", "BR 230" },
 { UINT64_C(13), "BR211", "BR 211" },
 { UINT64_C(14), "BR209", "BR 209" },
 { UINT64_C(15), "BR461", "BR 461" },
 { UINT64_C(16), "BR240", "BR 240" },
 { UINT64_C(17), "BR251", "BR 251" },
 { UINT64_C(18), "BR171", "BR 171" },
 { UINT64_C(19), "BR164", "BR 164" },
 { UINT64_C(20), "BR169", "BR 169 Od. BR 245" },
 { UINT64_C(21), "BR199", "BR 199" },
 { UINT64_C(22), "BR216", "BR 216" },
 { UINT64_C(23), "BR219", "BR 219" },
 { UINT64_C(24), "BR454", "BR 454 (Z-CAR)" },
 { UINT64_C(25), "NCV2", "NCV2" },
 { UINT64_C(26), "VITO", "V-Class / Vito" },
 { UINT64_C(27), "SPRINTER", "Sprinter" },
 { UINT64_C(28), "NCV3", "NCV3" },
 { UINT64_C(29), "NCV1", "NCV1" },
 { UINT64_C(30), "REST", "All other BR" },
 { UINT64_C(31), "SNV", "Code not available" },
};
static const MblinkMercedesEgs52EnumValue f17s6e[] = {
 { UINT64_C(0), "M272E35", "M272 E35" },
 { UINT64_C(1), "M271E18ML105", "M271 E18 ml Red. (105 kW)" },
 { UINT64_C(2), "M271E18ML120", "M271 E18 ml (120 kW)" },
 { UINT64_C(3), "M112E37", "M112 E37" },
 { UINT64_C(4), "M272E25", "M272 E25" },
 { UINT64_C(5), "M272E30", "M272 E30" },
 { UINT64_C(7), "M112E28", "M112 E28" },
 { UINT64_C(8), "M112E32", "M112 E32" },
 { UINT64_C(10), "M273E46", "M273 E46" },
 { UINT64_C(11), "M273E55", "M273 E55" },
 { UINT64_C(12), "M112E26", "M112 E26" },
 { UINT64_C(13), "M113E43", "M113 E43" },
 { UINT64_C(14), "M113E50", "M113 E50" },
 { UINT64_C(18), "M271E18ML140", "M271 E18 ML / 1 (140 kW)" },
 { UINT64_C(19), "M271DE18ML105", "M271 DE18 ml Red. (105 kW)" },
 { UINT64_C(20), "M271DE18ML125", "M271 DE18 ML (125 kW)" },
 { UINT64_C(22), "M111E_E23ML", "M111E E23 ML" },
 { UINT64_C(23), "M111E_E20", "M111E E20" },
 { UINT64_C(24), "M111E_E20ML", "M111E E20 ml" },
 { UINT64_C(25), "M112E32_140", "M112 E32 RED. (140 kW)" },
 { UINT64_C(26), "M266E20ATL", "M266 E20 ATL" },
 { UINT64_C(27), "M266E15", "M266 E15" },
 { UINT64_C(28), "M266E17", "M266 E17" },
 { UINT64_C(29), "M266E20", "M266 E20" },
 { UINT64_C(30), "M275E55", "M275 E55 Od. M285 E55" },
 { UINT64_C(31), "M137E58", "M137 E58" },
 { UINT64_C(32), "OM640DE20LA60", "OM 640 DE20 LA (60 kW)" },
 { UINT64_C(34), "OM640DE20LA80", "OM 640 DE20 LA (80 kW)" },
 { UINT64_C(35), "OM642DE30LA160", "OM642 DE30 LA (155/160 kW)" },
 { UINT64_C(36), "OM640DE20LA100", "OM 640 DE20 LA (100 kW)" },
 { UINT64_C(37), "OM613DE32LA", "OM613 DE32 La od. OM648 DE32 LA" },
 { UINT64_C(39), "OM628DE40LA", "OM628 DE40 LA" },
 { UINT64_C(40), "OM642DE30LA140", "OM642 DE30 LA (140 kW)" },
 { UINT64_C(43), "OM612DE27LA", "OM612 DE27 LA od. OM647 DE27 LA (120/130 kW)" },
 { UINT64_C(44), "OM611DE22LA100", "OM611 DE22 LA (105/100 kW) Od. OM646 DE22 LA (100/105/110 kW)" },
 { UINT64_C(45), "OM611DE22LA85", "OM611 DE22 LA (85 kW) Od. OM646 DE22 LA (90 kW)" },
 { UINT64_C(46), "OM611DE22LA75", "OM611 DE22 LA (75 kW) Od. OM646 DE22 LA (75 kW)" },
 { UINT64_C(64), "M134E11", "M134 E11 (3A91)" },
 { UINT64_C(65), "M135E13", "M135 E13 (4A90)" },
 { UINT64_C(66), "M135E15", "M135 E15 (4A91)" },
 { UINT64_C(67), "M135E15ATL", "M135 E15 ATL" },
 { UINT64_C(68), "M272DE25", "M272 DE25" },
 { UINT64_C(69), "M272DE30", "M272 DE30" },
 { UINT64_C(70), "M272DE35", "M272 DE35" },
 { UINT64_C(71), "M273DE46", "M273 DE46" },
 { UINT64_C(72), "M273DE55", "M273 DE55" },
 { UINT64_C(79), "M271E18MLATTR115", "M271 E18 ml Attr. (115kW)" },
 { UINT64_C(80), "M271E18MLATTR141", "M271 E18 ml Attr. (141kW)" },
 { UINT64_C(96), "OM629DE40LA", "OM629 DE40 LA" },
 { UINT64_C(99), "OM642DE30LARED140", "OM642 DE30 LA RED. (140kW)" },
};
static const MblinkMercedesEgs52EnumValue f17s10e[] = {
 { UINT64_C(0), "OK", "No warning" },
 { UINT64_C(1), "PFW1", "Warning filters too, level 1" },
 { UINT64_C(2), "PFW2", "Warning filters too, level 2" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f17s[] = {
 { "T_MOT",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"engine coolant temperature",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "T_LUFT",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"intake air temperature",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FCOD_KAR",UINT16_C(16),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Vehicle code body",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f17s2e,sizeof(f17s2e)/sizeof(f17s2e[0]) },
 { "FCOD_BR",UINT16_C(19),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Vehicle code series",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f17s3e,sizeof(f17s3e)/sizeof(f17s3e[0]) },
 { "FCOD_MOT6",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Vehicle code engine with 7 bit, bit 6",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GS_NVH",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission control not available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FCOD_MOT",UINT16_C(26),UINT16_C(6),true,UINT64_C(191),UINT8_C(8),"FZGCOD.Motor 7Bit, bit0-5 (bit6 -> signal fcod_mot6)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f17s6e,sizeof(f17s6e)/sizeof(f17s6e[0]) },
 { "V_MAX_FIX",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Fixed maximum speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "VB",UINT16_C(40),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"consumption",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ZWP_EIN_MS",UINT16_C(56),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn on auxiliary water pump",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PFW",UINT16_C(57),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Particle filter warning",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f17s10e,sizeof(f17s10e)/sizeof(f17s10e[0]) },
 { "ZVB_EIN_MS",UINT16_C(59),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on additional consumers",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PFKO",UINT16_C(60),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Particle Filter Correction Offset FMMOTMAX",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f18s[] = {
 { "D_RS_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f19s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f20s[] = {
 { "SD_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"system diagnostic response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f21s[] = {
 { "SG_APPL_MS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Control device to external application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f22s[] = {
 { "APPL2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f23s[] = {
 { "APPL4",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f24s[] = {
 { "APPL5",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f25s[] = {
 { "APPL6",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f26s[] = {
 { "APPL7",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f27s[] = {
 { "MESS1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f28s[] = {
 { "MESS2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f29s4e[] = {
 { UINT64_C(0), "G_N", "Destination \"N\"" },
 { UINT64_C(1), "G_D1", "Destination \"1\"" },
 { UINT64_C(2), "G_D2", "Destination \"2\"" },
 { UINT64_C(3), "G_D3", "Destination \"3\"" },
 { UINT64_C(4), "G_D4", "Destination \"4\"" },
 { UINT64_C(5), "G_D5", "Destination \"5\"" },
 { UINT64_C(6), "G_D6", "Destination \"6\"" },
 { UINT64_C(7), "G_D7", "Destination \"7\"" },
 { UINT64_C(8), "G_D_CVT", "Destination \"infinitely forward" },
 { UINT64_C(9), "G_R_CVT", "Goal \"infinitely reverse\"" },
 { UINT64_C(10), "G_R3", "Destination \"R3\"" },
 { UINT64_C(11), "G_R", "Destination \"R\"" },
 { UINT64_C(12), "G_R2", "Destination \"R2\"" },
 { UINT64_C(13), "G_P", "Destination \"P\"" },
 { UINT64_C(14), "G_ABBRUCH", "ABORT" },
 { UINT64_C(15), "G_SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f29s5e[] = {
 { UINT64_C(0), "G_N", "Actual rang \"N\"" },
 { UINT64_C(1), "G_D1", "actual gear \"1\"" },
 { UINT64_C(2), "G_D2", "actual gear \"2\"" },
 { UINT64_C(3), "G_D3", "Actual Rang \"3\"" },
 { UINT64_C(4), "G_D4", "Actual rang \"4\"" },
 { UINT64_C(5), "G_D5", "Actual rang \"5\"" },
 { UINT64_C(6), "G_D6", "actual gear \"6\"" },
 { UINT64_C(7), "G_D7", "Actual rang \"7\"" },
 { UINT64_C(8), "G_D_CVT", "Actual ranging \"infinitely forward" },
 { UINT64_C(9), "G_R_CVT", "Actual \"infinitely reverse\"" },
 { UINT64_C(10), "G_R3", "Actual ranging \"R3\"" },
 { UINT64_C(11), "G_R", "Actual rang \"R\"" },
 { UINT64_C(12), "G_R2", "Actual rang \"R2\"s" },
 { UINT64_C(13), "G_P", "Actual rang \"P\"" },
 { UINT64_C(14), "G_KRAFTFREI", "power-free" },
 { UINT64_C(15), "G_SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f29s20e[] = {
 { UINT64_C(0), "SPORT", "Sports (standard)" },
 { UINT64_C(1), "KOMFORT", "comfort" },
 { UINT64_C(2), "UNKNOWN", "not defined" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f29s28e[] = {
 { UINT64_C(0), "WAIT", "error check not completely run through" },
 { UINT64_C(1), "OK", "Completely traject error test, result 0" },
 { UINT64_C(2), "ERROR", "Error detected, enter current environmental data" },
 { UINT64_C(3), "UNKNOWN", "not defined" },
};
static const MblinkMercedesEgs52SignalDefinition f29s[] = {
 { "MTGL_EGS",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor moments Toggle 40ms + -10",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MMIN_EGS",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request min",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MMAX_EGS",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request max",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_EGS",UINT16_C(3),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Ford. Engine torque",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "GZC",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Goal Gang",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f29s4e,sizeof(f29s4e)/sizeof(f29s4e[0]) },
 { "GIC",UINT16_C(20),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"actual gear",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f29s5e,sizeof(f29s5e)/sizeof(f29s5e[0]) },
 { "K_S_B",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Best. (Transducer overbridge.-) clutch \"slip\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "K_O_B",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Best. (Transducer overbridders.-) clutch \"open\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "K_G_B",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Best. (Transducer overbridge.-) clutch \"closed\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "G_G",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"terrain",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GSP_OK",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Basic switch program O.K.",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FW_HOCH",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"driving resistance high",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SCHALT",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"circuit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HSM",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"hand switching mode",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GET_OK",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"gear ok",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KS",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Ball start",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ALF",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"reasonable release",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GS_NOTL",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"GS in the emergency",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "UEHITZ_GET",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Overtemperature gearbox",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KD",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Kickdown",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FPC_AAD",UINT16_C(38),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Driving program for AAD",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f29s20e,sizeof(f29s20e)/sizeof(f29s20e[0]) },
 { "MPAR_EGS",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque Request Parity (just parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DYN1_EGS",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"engagement mode / drive torque control",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DYN0_AMR_EGS",UINT16_C(42),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"engagement mode / drive torque control",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "K_LSTFR",UINT16_C(45),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Convertible bridging clutch load-free",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MOT_NAUS_CNF",UINT16_C(46),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"MOT_NAUS-ConfirmMbit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MOT_NAUS",UINT16_C(47),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Emergency Switch Off",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MKRIECH",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Kriech torque (FFH at EGS, CVT) or Calid / CVN",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FEHLPRF_ST",UINT16_C(56),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Status Error Check",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f29s28e,sizeof(f29s28e)/sizeof(f29s28e[0]) },
 { "CALID_CVN_AKT",UINT16_C(58),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"CALID / CVN transmission active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FEHLER",UINT16_C(59),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"error number or counter for calid / CVN transmission",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f30s0e[] = {
 { UINT64_C(255), "SNV", "unknown" },
};
static const MblinkMercedesEgs52EnumValue f30s5e[] = {
 { UINT64_C(0), "OFF", "unknown" },
 { UINT64_C(1), "ON", "unknown" },
 { UINT64_C(2), "ACTV", "unknown" },
 { UINT64_C(3), "SNA", "unknown" },
};
static const MblinkMercedesEgs52EnumValue f30s6e[] = {
 { UINT64_C(255), "SNV", "unknown" },
};
static const MblinkMercedesEgs52EnumValue f30s7e[] = {
 { UINT64_C(255), "SNV", "unknown" },
};
static const MblinkMercedesEgs52EnumValue f30s8e[] = {
 { UINT64_C(255), "SNV", "unknown" },
};
static const MblinkMercedesEgs52SignalDefinition f30s[] = {
 { "NAB",UINT16_C(0),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Transmission output speed (only 463/461, otherwise FFFFH)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f30s0e,sizeof(f30s0e)/sizeof(f30s0e[0]) },
 { "MIL_ANF_GS",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Request Mil through GS",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NAK_PA",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"NAK interface parity bit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NAK_TGL",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"NAK interface Togglebit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KID",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"function 'power-free in D' activated",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RACE_START",UINT16_C(20),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"start with maximum acceleration",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f30s5e,sizeof(f30s5e)/sizeof(f30s5e[0]) },
 { "M_VORSTR",UINT16_C(22),UINT16_C(10),false,UINT64_C(0),UINT8_C(0),"pilot torque (0h: passive value)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f30s6e,sizeof(f30s6e)/sizeof(f30s6e[0]) },
 { "MABS_SW_ANFB",UINT16_C(32),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Amount of the total side wave torque for starting area",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f30s7e,sizeof(f30s7e)/sizeof(f30s7e[0]) },
 { "NTURBINE",UINT16_C(48),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Turbine speed (EGS52-NAG, VGS-NAG2)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f30s8e,sizeof(f30s8e)/sizeof(f30s8e[0]) },
};
static const MblinkMercedesEgs52EnumValue f31s7e[] = {
 { UINT64_C(0), "GROSS", "Nag, big gear" },
 { UINT64_C(1), "KLEIN", "NAG, small gearbox" },
 { UINT64_C(2), "GROSS2", "NAG2, big gear" },
 { UINT64_C(3), "KLEIN2", "NAG2, small gearbox" },
};
static const MblinkMercedesEgs52EnumValue f31s10e[] = {
 { UINT64_C(0), "G_N", "Destination \"N\"" },
 { UINT64_C(1), "G_D1", "Destination \"1\"" },
 { UINT64_C(2), "G_D2", "Destination \"2\"" },
 { UINT64_C(3), "G_D3", "Destination \"3\"" },
 { UINT64_C(4), "G_D4", "Destination \"4\"" },
 { UINT64_C(5), "G_D5", "Destination \"5\"" },
 { UINT64_C(6), "G_D6", "Destination \"6\"" },
 { UINT64_C(7), "G_D7", "Destination \"7\"" },
 { UINT64_C(8), "G_D_CVT", "Destination \"infinitely forward" },
 { UINT64_C(9), "G_R_CVT", "Goal \"infinitely reverse\"" },
 { UINT64_C(10), "G_R3", "Destination \"R3\"" },
 { UINT64_C(11), "G_R", "Destination \"R\"" },
 { UINT64_C(12), "G_R2", "Destination \"R2\"" },
 { UINT64_C(13), "G_P", "Destination \"P\"" },
 { UINT64_C(14), "G_ABBRUCH", "circuit break" },
 { UINT64_C(15), "G_SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f31s11e[] = {
 { UINT64_C(0), "G_N", "Actual rang \"N\"" },
 { UINT64_C(1), "G_D1", "actual gear \"1\"" },
 { UINT64_C(2), "G_D2", "actual gear \"2\"" },
 { UINT64_C(3), "G_D3", "Actual Rang \"3\"" },
 { UINT64_C(4), "G_D4", "Actual rang \"4\"" },
 { UINT64_C(5), "G_D5", "Actual rang \"5\"" },
 { UINT64_C(6), "G_D6", "actual gear \"6\"" },
 { UINT64_C(7), "G_D7", "Actual rang \"7\"" },
 { UINT64_C(8), "G_D_CVT", "Actual ranging \"infinitely forward" },
 { UINT64_C(9), "G_R_CVT", "Actual \"infinitely reverse\"" },
 { UINT64_C(10), "G_R3", "Actual ranging \"R3\"" },
 { UINT64_C(11), "G_R", "Actual rang \"R\"" },
 { UINT64_C(12), "G_R2", "Actual rang \"R2\"" },
 { UINT64_C(13), "G_P", "Actual rang \"P\"" },
 { UINT64_C(14), "G_KRAFTFREI", "power-free" },
 { UINT64_C(15), "G_SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f31s15e[] = {
 { UINT64_C(0), "P", "Gear selector lever in position \"P\"" },
 { UINT64_C(1), "R", "gear selector lever in position \"R\"" },
 { UINT64_C(2), "N", "Gear selector lever in position \"N\"" },
 { UINT64_C(4), "D", "gear selector lever in position \"D\"" },
 { UINT64_C(7), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f31s[] = {
 { "FSC",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"drive",MBLINK_MERCEDES_EGS52_SIGNAL_CHAR,1.0,0.0,"",NULL,0U },
 { "FPC",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"drive",MBLINK_MERCEDES_EGS52_SIGNAL_CHAR,1.0,0.0,"",NULL,0U },
 { "T_GET",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Gear oil temperature",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ALLRAD",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"four-wheel drive",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FRONT",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Front drive [1], rear drive [0]",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SCHALT",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"circuit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CVT",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Stepless transmission [1], stage gear [0]",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MECH",UINT16_C(28),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Gear mechanics variant",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f31s7e,sizeof(f31s7e)/sizeof(f31s7e[0]) },
 { "ESV_BRE",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Create brake when switching on",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KD",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Kickdown",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GZC",UINT16_C(32),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"target gear",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f31s10e,sizeof(f31s10e)/sizeof(f31s10e[0]) },
 { "GIC",UINT16_C(36),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"actual gear",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f31s11e,sizeof(f31s11e)/sizeof(f31s11e[0]) },
 { "M_VERL",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Loss moment (FFH at KSG)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FMRADPAR",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Factor wheel torque parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FMRADTGL",UINT16_C(49),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Factor wheel torque Toggle 40ms + -10",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WHST",UINT16_C(50),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"gear selector lever position (NAG, KSG, CVT)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f31s15e,sizeof(f31s15e)/sizeof(f31s15e[0]) },
 { "FMRAD",UINT16_C(53),UINT16_C(11),false,UINT64_C(0),UINT8_C(0),"Factor wheel torque (7ffh at KSG)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f32s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f33s[] = {
 { "SD_RS_GS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"system diagnostic response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f34s[] = {
 { "APPL1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f35s[] = {
 { "HSA",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f36s[] = {
 { "HSB",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f37s[] = {
 { "HSC",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f38s[] = {
 { "HSD",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f39s[] = {
 { "HSE",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f40s4e[] = {
 { UINT64_C(5), "D", "selector lever in position \"D\"" },
 { UINT64_C(6), "N", "selector lever in position \"N\"" },
 { UINT64_C(7), "R", "selector lever in position \"R\"" },
 { UINT64_C(8), "P", "selector lever in position \"P\"" },
 { UINT64_C(9), "PLUS", "selector lever in position \"+\"" },
 { UINT64_C(10), "MINUS", "selector lever in position \"-\"" },
 { UINT64_C(11), "N_ZW_D", "selector lever in intermediate position \"N-D\"" },
 { UINT64_C(12), "R_ZW_N", "selector lever in intermediate position \"R-N\"" },
 { UINT64_C(13), "P_ZW_R", "selector lever in intermediate position \"P-R\"" },
 { UINT64_C(15), "SNV", "selector lever position unplausible" },
};
static const MblinkMercedesEgs52SignalDefinition f40s[] = {
 { "W_S",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Driving program",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FPT",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Driving program button actuated",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KD",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Kickdown",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPERR",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"barrier magnet energized",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WHC",UINT16_C(4),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"gear selector lever position (NAG only)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f40s4e,sizeof(f40s4e)/sizeof(f40s4e[0]) },
};
static const MblinkMercedesEgs52SignalDefinition f41s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f42s[] = {
 { "SD_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"system diagnostic response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f43s[] = {
 { "MESS1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f44s[] = {
 { "MESS2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f45s[] = {
 { "IFZ_ST",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS embassy to MS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f46s[] = {
 { "FBS_MS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS embassy to MS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f47s[] = {
 { "FBS_GS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS Embassy to GS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f48s[] = {
 { "FBS_EWM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS embassy to EWM (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f49s8e[] = {
 { UINT64_C(0), "UNKNOWN", "not defined" },
 { UINT64_C(1), "LL", "Left" },
 { UINT64_C(2), "RL", "RHD" },
 { UINT64_C(3), "SNV", "Code not available" },
};
static const MblinkMercedesEgs52EnumValue f49s19e[] = {
 { UINT64_C(0), "NBET", "Not operated (rocker and push-push)" },
 { UINT64_C(1), "AUS_BET", "ESP from operated (rocker) operated (Push Push)" },
 { UINT64_C(2), "EIN_NDEF", "ESP a pressed (rocker) is not defined (push-push)" },
 { UINT64_C(3), "SNV", "No signal (rocker and push-push)" },
};
static const MblinkMercedesEgs52EnumValue f49s24e[] = {
 { UINT64_C(0), "NBET", "Not operated (rocker and push-push)" },
 { UINT64_C(1), "UNBET_NDEF", "Down actuated (rocker) Not defined (push-push)" },
 { UINT64_C(2), "OBBET_BET", "Top operated (rocker), actuated (push-push)" },
 { UINT64_C(3), "UNKNOWN", "Not defined (rocker and push-push)" },
};
static const MblinkMercedesEgs52EnumValue f49s25e[] = {
 { UINT64_C(0), "NBET", "Not operated (rocker and push-push)" },
 { UINT64_C(1), "UNBET_NDEF", "Down actuated (rocker) Not defined (push-push)" },
 { UINT64_C(2), "OBBET_BET", "Top operated (rocker), actuated (push-push)" },
 { UINT64_C(3), "UNKNOWN", "Not defined (rocker and push-push)" },
};
static const MblinkMercedesEgs52EnumValue f49s26e[] = {
 { UINT64_C(0), "NDEF_NBET", "not defined (rocker), non-actuated (push-push)" },
 { UINT64_C(1), "AUS_NDEF", "distance warning (rocker) not defined (Push Push)" },
 { UINT64_C(2), "EIN_BET", "distance warning a (rocker) operated (Push Push)" },
 { UINT64_C(3), "SNV", "No signal (rocker and push-push)" },
};
static const MblinkMercedesEgs52EnumValue f49s32e[] = {
 { UINT64_C(0), "START", "Stand at launch of the respective series" },
 { UINT64_C(1), "V1", "BR 220: EJ 99 / X, C215: EJ 01/1, R230: EJ 02/1" },
 { UINT64_C(2), "V2", "BR 220: EJ 1.1, C215: EJ 02 / X, R230: EJ 03 / X" },
 { UINT64_C(3), "V3", "BR 220: EJ 02 / X, C215: EJ 03 / X, R230: not defined" },
 { UINT64_C(4), "V4", "BR 220: prohibited C215 / R230: undefined" },
 { UINT64_C(5), "V5", "BR 220: prohibited C215 / R230: undefined" },
 { UINT64_C(6), "V6", "BR 220: EJ 03 / X, C215 / R230: undefined" },
 { UINT64_C(7), "V7", "BR 220 / C215 / R230: undefined" },
};
static const MblinkMercedesEgs52EnumValue f49s33e[] = {
 { UINT64_C(0), "RDW", "Rest of the World" },
 { UINT64_C(1), "USA_CAN", "USA / Canada" },
 { UINT64_C(2), "UNKNOWN", "not defined" },
 { UINT64_C(3), "SNV", "Code not available" },
};
static const MblinkMercedesEgs52SignalDefinition f49s[] = {
 { "WH_UP",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever implausible",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VMAX_AKT",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Operation variable speed limit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_MINUS_B",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever: \"Sit and delay Stufe0\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_PLUS_B",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever: \"Sit and accelerating Stufe0\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WA",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever: \"resume\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AUS",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever \"off\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KG_KL_AKT",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyless Go terminal control active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KG_ALB_OK",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"meets Keyles Go annealing conditions",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LL_RLC",UINT16_C(10),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"LHD / RHD",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s8e,sizeof(f49s8e)/sizeof(f49s8e[0]) },
 { "RG_SCHALT",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Reverse gear engaged (manual transmission only)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BS_SL",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"brake switch for Shift Lock",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_15",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 15",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_50",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 50",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WH_PA",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever parity (even parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BZ240h",UINT16_C(20),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ASG_SPORT_BET",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ASG Sport mode on / off operated (ST2_LED_DL when ABC available)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRASH_CNF",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"CRASH Confirmbit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRASH",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Crash signal from airbag SG",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BN_NTLF",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Wiring emergency: Prio1- and Prio2-consumers, Second battery supports",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_BET",UINT16_C(33),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"ESP on / off operated",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s19e,sizeof(f49s19e)/sizeof(f49s19e[0]) },
 { "HAS_KL",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"attracted hand brake (control light)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_31B",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Wiper outside parking position",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLI_RE",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"directional blinking right",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLI_LI",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"directional blinking left",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ST2_BET",UINT16_C(40),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"LF / ABC 2-stage switch actuated",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s24e,sizeof(f49s24e)/sizeof(f49s24e[0]) },
 { "ST3_BET",UINT16_C(42),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"LF / ABC 3-position switch is actuated",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s25e,sizeof(f49s25e)/sizeof(f49s25e[0]) },
 { "ART_ABW_BET",UINT16_C(44),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"ART-distance warning actuated on / off",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s26e,sizeof(f49s26e)/sizeof(f49s26e[0]) },
 { "ABL_EIN",UINT16_C(46),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on low beam",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL54_RM",UINT16_C(47),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 54 Hardware enabled",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_ABSTAND",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"spacing factor",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ART_VH",UINT16_C(56),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ART available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GBL_AUS",UINT16_C(57),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"E-extractor: basic ventilation from",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FZGVERSN",UINT16_C(59),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Series addicts vehicle version (only 220/215/230)",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s32e,sizeof(f49s32e)/sizeof(f49s32e[0]) },
 { "LDC",UINT16_C(62),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"country code",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f49s33e,sizeof(f49s33e)/sizeof(f49s33e[0]) },
};
static const MblinkMercedesEgs52EnumValue f50s5e[] = {
 { UINT64_C(0), "KEIN", "Pendant not recognized" },
 { UINT64_C(1), "OK", "trailer recognized" },
 { UINT64_C(2), "UNKNOWN", "not defined" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f50s[] = {
 { "DIAG_X4_B",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Start Xenon4 diagnostic procedure passenger side",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DIAG_X4_F",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Start Xenon4 diagnostic procedure driver side",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ABL_EIN",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on low beam",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AFL_ABL_EIN",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"AFL requirement: Switch on low beam",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZWP_LFT",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Auxiliary water pump is running",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANH_ERK2",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"trailer operation recognized",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f50s5e,sizeof(f50s5e)/sizeof(f50s5e[0]) },
};
static const MblinkMercedesEgs52SignalDefinition f51s[] = {
 { "ABL_DEF_BF_R",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Low beam defective front passenger / right (depending on BR)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ABL_DEF_F_L",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Low beam defective driver / left (depending on BR)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f52s[] = {
 { "HZL_ANF",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Request heat output",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f53s[] = {
 { "ZH_EIN_OK",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn on a heater",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SENDE_NEU",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"signal version Compressor torque",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_KOMPPAR",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Climate Compressor Torque Parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_KOMPTGL",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Climate Compressor Tour Toggle",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_KOMP_NEU",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Climate Compressor Tour NEW",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "LL_DZA",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"idle speed lifting to the cooling power increase",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KOMP_EIN",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"climate compressor turned on",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "P_KAELTE8",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"refrigerant printing",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "M_KOMP",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Torque recording refrigeration compressor",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NLFTS",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Motor fan setpoint speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "T_AUSSEN_WM",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Outdoor air temperature for thermal management",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f54s[] = {
 { "APPL_SG_MS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"External application to control unit",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f55s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f56s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f57s[] = {
 { "D_RQ_CAS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f58s[] = {
 { "D_RQ_DTR",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f59s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f60s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f61s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f62s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f63s[] = {
 { "D_RQ_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f64s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f65s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f66s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f67s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f68s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f69s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f70s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f71s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f72s[] = {
 { "APPL2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f73s[] = {
 { "APPL1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f74s[] = {
 { "APPL3",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f75s[] = {
 { "MESS1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f76s[] = {
 { "D_RQ",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Request",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f77s0e[] = {
 { UINT64_C(0), "N_DEF", "not defined" },
 { UINT64_C(1), "LO", "Vin sign 1 - 7" },
 { UINT64_C(2), "MID", "Vin sign 8 - 14" },
 { UINT64_C(3), "HI", "Vin sign 15 - 17" },
};
static const MblinkMercedesEgs52SignalDefinition f77s[] = {
 { "VIN_MSG",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vin signal part",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f77s0e,sizeof(f77s0e)/sizeof(f77s0e[0]) },
 { "VIN_DATA",UINT16_C(8),UINT16_C(56),false,UINT64_C(0),UINT8_C(0),"Vin data",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f78s18e[] = {
 { UINT64_C(0), "UBG", "Unlimited" },
 { UINT64_C(1), "BG210", "210 km / h" },
 { UINT64_C(2), "BG190", "190 km / h" },
 { UINT64_C(3), "BG160", "160 km / h" },
 { UINT64_C(4), "BG240", "240 km / h" },
 { UINT64_C(5), "BG230", "230 km / h" },
 { UINT64_C(6), "BG220", "220 km / h" },
 { UINT64_C(7), "BG200", "200 km / h" },
 { UINT64_C(128), "BG180", "180 km / h" },
 { UINT64_C(129), "BG170", "170 km / h" },
 { UINT64_C(130), "BG150", "150 km / h" },
 { UINT64_C(131), "BG140", "140 km / h" },
 { UINT64_C(132), "BG130", "130 km / h" },
 { UINT64_C(133), "BG120", "120 km / h" },
 { UINT64_C(134), "BG110", "110 km / h" },
 { UINT64_C(135), "BG100", "100 km / h" },
};
static const MblinkMercedesEgs52SignalDefinition f78s[] = {
 { "TANK_FS",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Tank level",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "TF_AUF",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"driver's door",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_AUS",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed Limit / Tempose Display Not possible",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TACHO_SYM",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tacho oak",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_MPH",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"MPH instead of km / h (variable speed bends)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KLA_VH",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air conditioning available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VGL_KL_DEF",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"pre-glow control lamp defective",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TFSM",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tank level minimum",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_61E",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Clamp 61 decoupled",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "T_AUSSEN",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Outdoor air temperature raw value",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "KL_58D",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Terminal 58 dimmed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "MAZ",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Motor setting time (will be sent from Kl.15)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "KM16",UINT16_C(40),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"mileage",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "WRC3",UINT16_C(56),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Winter Tire Top Speed Bit 3",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_AKT",UINT16_C(57),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed Limit / Tempomat Display Active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SGT_VH",UINT16_C(58),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Segment tacho available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZH_FREIG",UINT16_C(59),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Release Heaters",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RT_EIN",UINT16_C(60),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on Roll Test Mode ESP",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WRC",UINT16_C(61),UINT16_C(3),true,UINT64_C(135),UINT8_C(8),"Winter tire maximum speed with 4 bits",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f78s18e,sizeof(f78s18e)/sizeof(f78s18e[0]) },
};
static const MblinkMercedesEgs52EnumValue f79s4e[] = {
 { UINT64_C(0), "SEHR_KLEIN", "very small" },
 { UINT64_C(1), "KLEIN", "small" },
 { UINT64_C(2), "MITTEL", "medium" },
 { UINT64_C(3), "GROSS", "big" },
 { UINT64_C(4), "SEHR_GROSS", "very big" },
 { UINT64_C(5), "UNKNOWN_1", "not defined" },
 { UINT64_C(6), "UNKNOWN_2", "not defined" },
 { UINT64_C(7), "UNKNOWN_3", "not defined" },
};
static const MblinkMercedesEgs52EnumValue f79s6e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f79s9e[] = {
 { UINT64_C(0), "IDLE", "No change" },
 { UINT64_C(1), "AUS", "switch off PRW" },
 { UINT64_C(2), "EIN", "re-enable PRW" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f79s[] = {
 { "AKU_WARN_AUS",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Acoustic warning out",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OPT_WARN_AUS",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Optical warning out",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ECO_WARN_ST",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Status Eco Warning",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ABST_S",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"distance unit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "IST_ABST",UINT16_C(9),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"set distance",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f79s4e,sizeof(f79s4e)/sizeof(f79s4e[0]) },
 { "V_ANZ",UINT16_C(12),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Speed displayed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGANZ",UINT16_C(24),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"wheel direction of rotation to V_ANZ",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f79s6e,sizeof(f79s6e)/sizeof(f79s6e[0]) },
 { "DANZ",UINT16_C(26),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"wheel speed calculated from V_ANZ",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ECO_AKT",UINT16_C(44),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Activation ECO in the combined menu",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PRW_ANF",UINT16_C(46),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Request PlatRollwarner",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f79s9e,sizeof(f79s9e)/sizeof(f79s9e[0]) },
 { "MAZ_NEU",UINT16_C(52),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Motor setting time",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f80s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f81s[] = {
 { "MESS1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f82s[] = {
 { "MESS2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f83s3e[] = {
 { UINT64_C(0), "INIT_PSBL", "LRW sensor is initializable" },
 { UINT64_C(1), "INIT_SELF", "LRW sensor initializes itself" },
 { UINT64_C(2), "INIT_MUST", "(LRW sensor must be initialized)" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f83s4e[] = {
 { UINT64_C(0), "OK", "Steering wheel angle sensor I.O." },
 { UINT64_C(1), "INI", "Steering wheel angle sensor not initialized" },
 { UINT64_C(2), "ERR", "steering wheel angle sensor faulty" },
 { UINT64_C(3), "ERR_INI", "steering wheel angle sensor faulty and not initialized" },
};
static const MblinkMercedesEgs52SignalDefinition f83s[] = {
 { "LRW",UINT16_C(2),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Steering wheel angle",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "VLRW",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"steering wheel angular velocity",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "BZ236h",UINT16_C(32),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "LRWS_ID",UINT16_C(36),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Identification steering wheel angle sensor",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f83s3e,sizeof(f83s3e)/sizeof(f83s3e[0]) },
 { "LRWS_ST",UINT16_C(38),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Status steering wheel angle sensor",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f83s4e,sizeof(f83s4e)/sizeof(f83s4e[0]) },
 { "CRC236h",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC checksum byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f84s[] = {
 { "WH_UP",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomat selector lever unplausible",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VMAX_AKT",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Operation variable speed limitation",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_MINUS_B",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomatwatch Lever: \"Setting and delaying Levo0\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_PLUS_B",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomat selector lever: \"Setting and Accelerating Level0\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WA",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Cruise control lever: \"Recovery\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AUS",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomat selector lever: \"Switch off\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLI_RE",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"directional flashing right",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLI_LI",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"direction flash left",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WH_PA",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomat selector lever Parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BZ238h",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "LW_PA",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Steering angle parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LW_OV",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Steering angle sensor: overflow",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LW_CF",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Steering angle sensor: Code error",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LW_INI",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Steering angle sensor: not initialized",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LW_VZ",UINT16_C(20),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Steering angle sign",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LW",UINT16_C(21),UINT16_C(11),false,UINT64_C(0),UINT8_C(0),"steering angle",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f85s[] = {
 { "CONF_CRASH",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Confirm bit for all crazy events, tox",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRASH_F",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Frontal event 2",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRASH_C",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Frontal event 5",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f86s0e[] = {
 { UINT64_C(0), "IDLE", "No status / warning" },
 { UINT64_C(1), "M1", "Message \"ASA inactive: warming up the engine\"" },
 { UINT64_C(2), "M2", "Message \"ASA Active driving\"" },
 { UINT64_C(3), "M3", "Message \"ASA Active stop mode\"" },
 { UINT64_C(4), "M4", "Message \"ASA switched off: electrical power supplies\"" },
 { UINT64_C(5), "M5", "Message \"ASA switched off: air conditioner\"" },
 { UINT64_C(6), "M6", "Message \"ASA not active: Fault\"" },
 { UINT64_C(7), "M7", "Message \"ASA Active electrical energy demand, Please start engine\"" },
 { UINT64_C(8), "M8", "Message \"ASA active: For starting clutch kick\"" },
 { UINT64_C(9), "M9", "Message \"ASA active: Air start Please Motor\"" },
 { UINT64_C(10), "M10", "Message \"ASA Active: When leaving ignition off!\"" },
 { UINT64_C(11), "M11", "Message \"ASA disabled\"" },
 { UINT64_C(12), "M12", "Message \"ASA activated\"" },
 { UINT64_C(13), "M13", "Message \"ASA: Display defective\"" },
 { UINT64_C(14), "M14", "not defined" },
 { UINT64_C(15), "M15", "not defined" },
};
static const MblinkMercedesEgs52EnumValue f86s1e[] = {
 { UINT64_C(0), "IDLE", "No status / warning" },
 { UINT64_C(1), "M1", "Message \"ASA inactive: warming up the engine\"" },
 { UINT64_C(2), "M2", "Message \"ASA Active driving\"" },
 { UINT64_C(3), "M3", "Message \"ASA Active stop mode\"" },
 { UINT64_C(4), "M4", "Message \"ASA switched off: electrical power supplies\"" },
 { UINT64_C(5), "M5", "Message \"ASA switched off: air conditioner\"" },
 { UINT64_C(6), "M6", "Message \"ASA not active: Fault\"" },
 { UINT64_C(7), "M7", "Message \"ASA Active electrical energy demand, Please start engine\"" },
 { UINT64_C(8), "M8", "Message \"ASA active: For starting clutch kick\"" },
 { UINT64_C(9), "M9", "Message \"ASA active: Air start Please Motor\"" },
 { UINT64_C(10), "M10", "Message \"ASA Active: When leaving ignition off!\"" },
 { UINT64_C(11), "M11", "Message \"ASA disabled\"" },
 { UINT64_C(12), "M12", "Message \"ASA activated\"" },
 { UINT64_C(13), "M13", "Message \"ASA: Display defective\"" },
 { UINT64_C(14), "M14", "not defined" },
 { UINT64_C(15), "M15", "not defined" },
};
static const MblinkMercedesEgs52SignalDefinition f86s[] = {
 { "ASS_WARN",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Number of ASA alert",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f86s0e,sizeof(f86s0e)/sizeof(f86s0e[0]) },
 { "ASS_DSPL",UINT16_C(20),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Number of ASA status message",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f86s1e,sizeof(f86s1e)/sizeof(f86s1e[0]) },
 { "ASS_LTEST_AUS",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"suppress lamp test during stop phase",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f87s[] = {
 { "GS_FBS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS Embassy to EZS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f88s[] = {
 { "EWM_FBS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"FBS Embassy to EZS (8 bytes)",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f89s[] = {
 { "I_IST_GET",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"is translation (only with FCVT, otherwise is / goverge)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f90s0e[] = {
 { UINT64_C(0), "EWM", "EWM" },
 { UINT64_C(1), "MRM", "MRM" },
 { UINT64_C(2), "UNKNOWN_1", "not defined" },
 { UINT64_C(3), "UNKNOWN_2", "not defined" },
};
static const MblinkMercedesEgs52EnumValue f90s1e[] = {
 { UINT64_C(0), "NBET", "not actuated" },
 { UINT64_C(1), "PLUS", "\"+\" actuated" },
 { UINT64_C(2), "MINUS", "\"-\" actuated" },
 { UINT64_C(3), "PLUS_MINUS", "\"+\" and \"-\" actuated" },
 { UINT64_C(4), "UNKNOWN_1", "not defined" },
 { UINT64_C(5), "UNKNOWN_2", "not defined" },
 { UINT64_C(6), "UNKNOWN_3", "not defined" },
 { UINT64_C(7), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f90s2e[] = {
 { UINT64_C(0), "GWHST_LR", "GWHST_LR valid on bit 0..7 (old signal)" },
 { UINT64_C(2), "RES_ALT_FEHLER", "Reserved Old Signal \"Error MRSM\"" },
 { UINT64_C(3), "SBWB_ST_P_RND", "SBWB_ST P, RND valid on bit 0..5 (new signals)" },
};
static const MblinkMercedesEgs52EnumValue f90s3e[] = {
 { UINT64_C(0), "IDLE", "P-button in rest position" },
 { UINT64_C(1), "P", "P button in \"P\" position" },
 { UINT64_C(2), "INIT", "P button initialization" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52EnumValue f90s4e[] = {
 { UINT64_C(0), "IDLE", "SBW control in rest position" },
 { UINT64_C(1), "R", "SBW control in \"R\"" },
 { UINT64_C(2), "N_OBEN", "SBW control in \"N above\"" },
 { UINT64_C(4), "N_UNTEN", "SBW control in \"N below\"" },
 { UINT64_C(6), "INIT", "SBW control in initialization" },
 { UINT64_C(8), "D", "SBW control in \"D\"" },
 { UINT64_C(15), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f90s[] = {
 { "SID_SBW",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"transmitter recognition",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f90s0e,sizeof(f90s0e)/sizeof(f90s0e[0]) },
 { "LRT_PM3",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Steering wheel keys \"+\", \"-\" actuated",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f90s1e,sizeof(f90s1e)/sizeof(f90s1e[0]) },
 { "SBWB_ID",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Shift-by-Wire control element ID",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f90s2e,sizeof(f90s2e)/sizeof(f90s2e[0]) },
 { "SBWB_ST_P",UINT16_C(10),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Shift-by-Wire control P-button",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f90s3e,sizeof(f90s3e)/sizeof(f90s3e[0]) },
 { "SBWB_ST_RND",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Shift-by-Wire control Status RND",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f90s4e,sizeof(f90s4e)/sizeof(f90s4e[0]) },
 { "BZ232h",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f91s0e[] = {
 { UINT64_C(0), "SKL0", "Shift characteristic \"0\"" },
 { UINT64_C(1), "SKL1", "Shift characteristic \"1\"" },
 { UINT64_C(2), "SKL2", "Shift characteristic \"2\"" },
 { UINT64_C(3), "SKL3", "Shift characteristic \"3\"" },
 { UINT64_C(4), "SKL4", "Shift characteristic \"4\"" },
 { UINT64_C(5), "SKL5", "Shift characteristic \"5\"" },
 { UINT64_C(6), "SKL6", "Shift characteristic \"6\"" },
 { UINT64_C(7), "SKL7", "Shift characteristic \"7\"" },
 { UINT64_C(8), "SKL8", "Shift characteristic \"8\"" },
 { UINT64_C(9), "SKL9", "Shift characteristic \"9\"" },
 { UINT64_C(10), "SKL10", "Shift characteristic \"10\"" },
};
static const MblinkMercedesEgs52EnumValue f91s14e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, upper limit = 1" },
 { UINT64_C(2), "G2", "Gear, upper limit = 2" },
 { UINT64_C(3), "G3", "Gear, upper limit = 3" },
 { UINT64_C(4), "G4", "Gear, upper limit = 4" },
 { UINT64_C(5), "G5", "Gear, upper limit = 5" },
 { UINT64_C(6), "G6", "Gear, upper limit = 6" },
 { UINT64_C(7), "G7", "Gear, upper limit = 7" },
};
static const MblinkMercedesEgs52EnumValue f91s15e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, lower limit = 1" },
 { UINT64_C(2), "G2", "Gear, lower limit = 2" },
 { UINT64_C(3), "G3", "Gear, lower limit = 3" },
 { UINT64_C(4), "G4", "Gear, lower limit = 4" },
 { UINT64_C(5), "G5", "Gear, lower limit = 5" },
 { UINT64_C(6), "G6", "Gear, lower limit = 6" },
 { UINT64_C(7), "G7", "Gear, lower limit = 7" },
};
static const MblinkMercedesEgs52SignalDefinition f91s[] = {
 { "SLV_ART",UINT16_C(0),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Switching Difference Art",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f91s0e,sizeof(f91s0e)/sizeof(f91s0e[0]) },
 { "ART_OK",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Type in order",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_BRE",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Type brakes",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BL_UNT",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake light suppression",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DYN_UNT",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Suppression Dynamic fully detection",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MPAR_ART",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque Request Parity (just parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MDYN_ART",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine torque request dynamic",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CAS_REG",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"City Assistant regulates",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LIM_REG",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Limiter regulates",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_REG",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Type regulates",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_ART",UINT16_C(19),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Ford. Engine torque",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "BZ250h",UINT16_C(32),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "MBRE_ART",UINT16_C(36),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"brake torque (0000h: passive value)",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "AKT_R_ART",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Art desire: \"Active recirculation\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GMAX_ART",UINT16_C(50),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, upper limit",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f91s14e,sizeof(f91s14e)/sizeof(f91s14e[0]) },
 { "GMIN_ART",UINT16_C(53),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, lower limit",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f91s15e,sizeof(f91s15e)/sizeof(f91s15e[0]) },
};
static const MblinkMercedesEgs52EnumValue f92s4e[] = {
 { UINT64_C(0), "OK", "No error available" },
 { UINT64_C(1), "SCHMUTZ", "ART disorder; Sensor dirty" },
 { UINT64_C(2), "ART_DEF", "Art defective" },
 { UINT64_C(3), "ART_LIM_DEF", "Art / Lim defective" },
 { UINT64_C(4), "EXT", "ART; External fault" },
 { UINT64_C(5), "DBC_DEF", "DBC defective" },
 { UINT64_C(6), "SCHMUTZ_DBC_DEF", "ART disorder; Sensor dirty and DBC defective" },
 { UINT64_C(7), "ART_DBC_DEF", "Type and DBC defective" },
 { UINT64_C(8), "ART_LIM_DBC_DEF", "Art / Lim and DBC defective" },
 { UINT64_C(9), "EXT_DBC", "Art external disorder and DBC defective" },
 { UINT64_C(15), "UNKNOWN", "not defined" },
};
static const MblinkMercedesEgs52EnumValue f92s23e[] = {
 { UINT64_C(0), "AUS", "out" },
 { UINT64_C(1), "AAS", "spacer wizard" },
 { UINT64_C(2), "ADTR", "Advanced DISTRONIC" },
 { UINT64_C(3), "DBC", "Downhill Brake Control" },
};
static const MblinkMercedesEgs52EnumValue f92s24e[] = {
 { UINT64_C(0), "IDLE", "No error" },
 { UINT64_C(1), "CAS_SFV_REINIGEN", "CAS Display \"Clean bumper front\"" },
 { UINT64_C(2), "CAS_SFV_SFH_REINIGEN", "CAS Display \"Clean bumper front and rear\"" },
 { UINT64_C(3), "CAS_ERR_W", "CAS Display \"Workshop\"" },
};
static const MblinkMercedesEgs52EnumValue f92s25e[] = {
 { UINT64_C(0), "IDLE", "basic picture according to active bit" },
 { UINT64_C(1), "DBC_LIM", "Message \"Turn on DBC / DBC XX km / h\"" },
 { UINT64_C(2), "DBC_AUS", "Message \"Turn off DBC\"" },
 { UINT64_C(3), "DBC_AUS_TON", "message \"Switch off DBC\" with sound" },
 { UINT64_C(4), "DBC_NV_AKT", "Message \"DBC Missing / DBC Not Activable\"" },
 { UINT64_C(5), "DBC_NV_LIM", "Message \"DBC Missing / DBC XX km / h\"" },
 { UINT64_C(6), "AAS_EIN", "Message \"Turn on AAS\"" },
 { UINT64_C(7), "AAS_AUS", "Message \"Turn off AAS\"" },
 { UINT64_C(8), "AAS_AUS_TON", "Message \"Turn off AAS\" with sound" },
 { UINT64_C(9), "AAS_NV_LIM", "Message \"Do not turn on AAS activateable / LIM\"" },
 { UINT64_C(10), "AAS_NV_OBJ", "Message \"AAS not activateable / no destination\"" },
 { UINT64_C(11), "AAS_NV_FBED", "Message \"AAS incorrect operation / not available\"" },
 { UINT64_C(12), "AAS_FOLGEN", "message \"AAS destination goes on / follow\"" },
 { UINT64_C(13), "AAS_OBJ_VERLUST", "message \"AAS object loss\"" },
 { UINT64_C(14), "AAS_OBJ_WECHSEL", "message \"AAS new object / object change\"" },
 { UINT64_C(15), "PAS_EIN", "message \"PAS switch on / PAS active\"" },
 { UINT64_C(16), "PAS_AUS", "Message \"Turn off PAS / PAS\"" },
 { UINT64_C(17), "PAS_NV", "message \"PAS not activatable\"" },
};
static const MblinkMercedesEgs52SignalDefinition f92s[] = {
 { "ART_DSPL_EIN",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn the display on type display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_OBJ",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"detection standing object",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_WT",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Art Warning",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_INFO",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Art Infolampe",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_ERR",UINT16_C(4),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Art error code",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f92s4e,sizeof(f92s4e)/sizeof(f92s4e[0]) },
 { "V_ART",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"set type speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ABST_R_OBJ",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"distance relevant object",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "SOLL_ABST",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"driver request",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ART_DSPL_PGB",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display \"Winter tire limitation achieved\" on the display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_VFBR",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display \"DTR OFF [0]\" on the display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_DSPL_LIM",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display \"---\" on the display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_EIN",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Spacer control mpomat turned on",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OBJ_ERK",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Relevant object recognized",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_SEG_EIN",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn on style segment display",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_DSPL_BL",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fluid indicator flash",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TM_EIN_ART",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Art Tempomat on",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_ZIEL",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Speed ​​recognized target vehicle",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ART_DSPL_NEU",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Minimum display time in the display new trigger",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_UEBERSP",UINT16_C(49),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Type is overplayed by the driver",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_REAKT",UINT16_C(50),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display of system availability after system error",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ART_ABW_AKT",UINT16_C(51),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Art distance warning is switched on",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OBJ_AGB",UINT16_C(52),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Object Offer Spacer Wizard",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AAS_LED_BL",UINT16_C(53),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LED spacer wizard flashing",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ASSIST_FKT_AKT",UINT16_C(54),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Active function",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f92s23e,sizeof(f92s23e)/sizeof(f92s23e[0]) },
 { "CAS_ERR_ANZ_V2",UINT16_C(56),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"CAS Display request",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f92s24e,sizeof(f92s24e)/sizeof(f92s24e[0]) },
 { "ASSIST_ANZ_V2",UINT16_C(59),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Assistance system Display request",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f92s25e,sizeof(f92s25e)/sizeof(f92s25e[0]) },
};
static const MblinkMercedesEgs52SignalDefinition f93s[] = {
 { "M_LAST",UINT16_C(60),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Load torque ABC pump",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f94s[] = {
 { "PSM_ADR_PAR",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Work Speed Control - ParityBit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_ADR_TGL",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Labor speed control - Togglebit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_ADR_AKT",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"working speed control active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_N_SOLL",UINT16_C(8),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Motoroll speed ADR",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "PSM_MOM_PAR",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tomentic limitation - parity bit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_MOM_TGL",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tomentic limitation - Togglebit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_MOM_AKT",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tomentic limitation active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_MOM_SOLL",UINT16_C(27),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Maximum engine torque",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "PSM_DZ_PAR",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation - parity bit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_DZ_TGL",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation - Togglebit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_DZ_AKT",UINT16_C(42),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_DZ_MAX",UINT16_C(48),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Maximum speed",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f95s[] = {
 { "PSM_V_PAR",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed Control - Parity Bit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_V_TGL",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation - Togglebit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_V_AKT",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_V_SOLL",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Speed limit",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "PSM_DZ_PAR",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation - parity bit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_DZ_TGL",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation - Togglebit",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_FERN_START",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor Remote Start active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_FERN_STOP",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor Remote Stop active",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PSM_FPM_SP",UINT16_C(20),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"lock accelerator pedal module",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f96s[] = {
 { "T_AUSSEN_K",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Filtered outside temperature Combi",MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52EnumValue f97s1e[] = {
 { UINT64_C(0), "SH_IPG", "Switching is running (Shift in progress)" },
 { UINT64_C(1), "LO", "terrain (low range)" },
 { UINT64_C(2), "HI", "Road speed (High Range)" },
 { UINT64_C(4), "N", "Neutralgang (Not High Or Low Range)" },
 { UINT64_C(7), "SNV", "signal not available (signal not available)" },
};
static const MblinkMercedesEgs52EnumValue f97s4e[] = {
 { UINT64_C(0), "UNKNOWN", "not defined" },
 { UINT64_C(1), "ANF_N", "requirement \"neutral\"" },
 { UINT64_C(2), "IDLE", "No requirement" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs52SignalDefinition f97s[] = {
 { "VG_ERR",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Error VG (ECU Failure Detected)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VG_GANG",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Current gear distribution gear",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f97s1e,sizeof(f97s1e)/sizeof(f97s1e[0]) },
 { "ANFNPAR_VG",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"VG - Request \"n\" Parity (straight parity)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANFNTGL_VG",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"VG - ANG.Load \"N\" Toggle 20ms (1 / Embassy)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANFN_VG",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"VG request \"N\"",MBLINK_MERCEDES_EGS52_SIGNAL_ENUM,1.0,0.0,"",f97s4e,sizeof(f97s4e)/sizeof(f97s4e[0]) },
};
static const MblinkMercedesEgs52SignalDefinition f98s[] = {
 { "LWR_M7",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 7: \"Baltic view currently not available\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWR_M6",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 6: \"Bolt match right\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWR_M5",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 5: \"Bolt view left\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWR_M4",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 4: \"Curve light currently not available\" (white / 5x flashing with 1Hz)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWR_M3",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 3: \"Curve light currently not available\" (white).",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWR_M2",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 2: \"Curve light, replacement light activated!\"(White)",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LWR_M1",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display message 1: \"Curve light defective! Drive to the workshop\"",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SUB_ABL_L",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Substitution lowlight left",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SUB_ABL_R",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Substitution low beam right",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f99s[] = {
 { "GBL_AUS",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"E-suction fan: basic ventilation",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KLA_VH",UINT16_C(50),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air conditioning available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DSH_VH",UINT16_C(60),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Differential lock behind available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DSM_VH",UINT16_C(61),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Differential lock center available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DSV_VH",UINT16_C(62),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Differential lock in front available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VG_VH",UINT16_C(63),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"distribution gear control available",MBLINK_MERCEDES_EGS52_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f100s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f101s[] = {
 { "D_RS_CAS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f102s[] = {
 { "D_RS_DTR",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f103s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f104s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f105s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f106s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f107s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f108s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f109s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f110s[] = {
 { "D_RS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"KWP2000 Diagnostic Response",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f111s[] = {
 { "APPL1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f112s[] = {
 { "APPL1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f113s[] = {
 { "APPL3",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Application",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f114s[] = {
 { "MESS2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Measured values",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f115s[] = {
 { "HS1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f116s[] = {
 { "HS2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f117s[] = {
 { "HS3",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f118s[] = {
 { "HS4",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Hand control at the test bench",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f119s[] = {
 { "HS5",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"S:",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52SignalDefinition f120s[] = {
 { "HS6",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"S:",MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs52FrameDefinition defs[] = {
 { "ESP_SBC","BS_200h",UINT32_C(0x200),f0s,sizeof(f0s)/sizeof(f0s[0]) },
 { "ESP_SBC","BS_208h",UINT32_C(0x208),f1s,sizeof(f1s)/sizeof(f1s[0]) },
 { "ESP_SBC","BS_270h",UINT32_C(0x270),f2s,sizeof(f2s)/sizeof(f2s[0]) },
 { "ESP_SBC","BS_300h",UINT32_C(0x300),f3s,sizeof(f3s)/sizeof(f3s[0]) },
 { "ESP_SBC","BS_328h",UINT32_C(0x328),f4s,sizeof(f4s)/sizeof(f4s[0]) },
 { "ESP_SBC","D_RS_BS",UINT32_C(0x785),f5s,sizeof(f5s)/sizeof(f5s[0]) },
 { "ESP_SBC","SD_RS_BS",UINT32_C(0x722),f6s,sizeof(f6s)/sizeof(f6s[0]) },
 { "ESP_SBC","BS_APPL2",UINT32_C(0x635),f7s,sizeof(f7s)/sizeof(f7s[0]) },
 { "MS","MS_100h",UINT32_C(0x100),f8s,sizeof(f8s)/sizeof(f8s[0]) },
 { "MS","MS_101h",UINT32_C(0x101),f9s,sizeof(f9s)/sizeof(f9s[0]) },
 { "MS","MS_210h",UINT32_C(0x210),f10s,sizeof(f10s)/sizeof(f10s[0]) },
 { "MS","MS_212h",UINT32_C(0x212),f11s,sizeof(f11s)/sizeof(f11s[0]) },
 { "MS","MS_268h",UINT32_C(0x268),f12s,sizeof(f12s)/sizeof(f12s[0]) },
 { "MS","MS_2F3h",UINT32_C(0x2f3),f13s,sizeof(f13s)/sizeof(f13s[0]) },
 { "MS","MS_308h",UINT32_C(0x308),f14s,sizeof(f14s)/sizeof(f14s[0]) },
 { "MS","MS_312h",UINT32_C(0x312),f15s,sizeof(f15s)/sizeof(f15s[0]) },
 { "MS","AAD_580h",UINT32_C(0x580),f16s,sizeof(f16s)/sizeof(f16s[0]) },
 { "MS","MS_608h",UINT32_C(0x608),f17s,sizeof(f17s)/sizeof(f17s[0]) },
 { "MS","D_RS_FSCM",UINT32_C(0x779),f18s,sizeof(f18s)/sizeof(f18s[0]) },
 { "MS","D_RS_MS",UINT32_C(0x7e8),f19s,sizeof(f19s)/sizeof(f19s[0]) },
 { "MS","SD_RS_MS",UINT32_C(0x720),f20s,sizeof(f20s)/sizeof(f20s[0]) },
 { "MS","SG_APPL_MS",UINT32_C(0x529),f21s,sizeof(f21s)/sizeof(f21s[0]) },
 { "MS","MS_APPL2",UINT32_C(0x4a9),f22s,sizeof(f22s)/sizeof(f22s[0]) },
 { "MS","MS_APPL4",UINT32_C(0x633),f23s,sizeof(f23s)/sizeof(f23s[0]) },
 { "MS","MS_APPL5",UINT32_C(0x6a8),f24s,sizeof(f24s)/sizeof(f24s[0]) },
 { "MS","MS_APPL6",UINT32_C(0x610),f25s,sizeof(f25s)/sizeof(f25s[0]) },
 { "MS","MS_APPL7",UINT32_C(0x618),f26s,sizeof(f26s)/sizeof(f26s[0]) },
 { "MS","EDC_MESS1",UINT32_C(0x670),f27s,sizeof(f27s)/sizeof(f27s[0]) },
 { "MS","EDC_MESS2",UINT32_C(0x671),f28s,sizeof(f28s)/sizeof(f28s[0]) },
 { "GS","GS_218h",UINT32_C(0x218),f29s,sizeof(f29s)/sizeof(f29s[0]) },
 { "GS","GS_338h",UINT32_C(0x338),f30s,sizeof(f30s)/sizeof(f30s[0]) },
 { "GS","GS_418h",UINT32_C(0x418),f31s,sizeof(f31s)/sizeof(f31s[0]) },
 { "GS","D_RS_GS",UINT32_C(0x7e9),f32s,sizeof(f32s)/sizeof(f32s[0]) },
 { "GS","SD_RS_GS",UINT32_C(0x723),f33s,sizeof(f33s)/sizeof(f33s[0]) },
 { "GS","GS_APPL1",UINT32_C(0x51c),f34s,sizeof(f34s)/sizeof(f34s[0]) },
 { "GS","GS_HSA",UINT32_C(0x50a),f35s,sizeof(f35s)/sizeof(f35s[0]) },
 { "GS","GS_HSB",UINT32_C(0x50b),f36s,sizeof(f36s)/sizeof(f36s[0]) },
 { "GS","GS_HSC",UINT32_C(0x50c),f37s,sizeof(f37s)/sizeof(f37s[0]) },
 { "GS","GS_HSD",UINT32_C(0x50d),f38s,sizeof(f38s)/sizeof(f38s[0]) },
 { "GS","GS_HSE",UINT32_C(0x50e),f39s,sizeof(f39s)/sizeof(f39s[0]) },
 { "EWM","EWM_230h",UINT32_C(0x230),f40s,sizeof(f40s)/sizeof(f40s[0]) },
 { "EWM","D_RS_EWM",UINT32_C(0x789),f41s,sizeof(f41s)/sizeof(f41s[0]) },
 { "EWM","SD_RS_EWM",UINT32_C(0x724),f42s,sizeof(f42s)/sizeof(f42s[0]) },
 { "EWM","EWM_MESS1",UINT32_C(0x6f0),f43s,sizeof(f43s)/sizeof(f43s[0]) },
 { "EWM","EWM_MESS2",UINT32_C(0x6f1),f44s,sizeof(f44s)/sizeof(f44s[0]) },
 { "EZS","FBS_110h",UINT32_C(0x110),f45s,sizeof(f45s)/sizeof(f45s[0]) },
 { "EZS","FBS_111h",UINT32_C(0x111),f46s,sizeof(f46s)/sizeof(f46s[0]) },
 { "EZS","FBS_112h",UINT32_C(0x112),f47s,sizeof(f47s)/sizeof(f47s[0]) },
 { "EZS","FBS_114h",UINT32_C(0x114),f48s,sizeof(f48s)/sizeof(f48s[0]) },
 { "EZS","EZS_240h",UINT32_C(0x240),f49s,sizeof(f49s)/sizeof(f49s[0]) },
 { "EZS","ZGW_248h",UINT32_C(0x248),f50s,sizeof(f50s)/sizeof(f50s[0]) },
 { "EZS","ZGW_24Ch",UINT32_C(0x24c),f51s,sizeof(f51s)/sizeof(f51s[0]) },
 { "EZS","KLA_40Eh",UINT32_C(0x40e),f52s,sizeof(f52s)/sizeof(f52s[0]) },
 { "EZS","KLA_410h",UINT32_C(0x410),f53s,sizeof(f53s)/sizeof(f53s[0]) },
 { "EZS","APPL_SG_MS",UINT32_C(0x74c),f54s,sizeof(f54s)/sizeof(f54s[0]) },
 { "EZS","D_RQ_ART",UINT32_C(0x78e),f55s,sizeof(f55s)/sizeof(f55s[0]) },
 { "EZS","D_RQ_BS",UINT32_C(0x784),f56s,sizeof(f56s)/sizeof(f56s[0]) },
 { "EZS","D_RQ_CAS",UINT32_C(0x77a),f57s,sizeof(f57s)/sizeof(f57s[0]) },
 { "EZS","D_RQ_DTR",UINT32_C(0x702),f58s,sizeof(f58s)/sizeof(f58s[0]) },
 { "EZS","D_RQ_EHB",UINT32_C(0x79a),f59s,sizeof(f59s)/sizeof(f59s[0]) },
 { "EZS","D_RQ_EHB2",UINT32_C(0x7b0),f60s,sizeof(f60s)/sizeof(f60s[0]) },
 { "EZS","D_RQ_EWM",UINT32_C(0x788),f61s,sizeof(f61s)/sizeof(f61s[0]) },
 { "EZS","D_RQ_FS",UINT32_C(0x78c),f62s,sizeof(f62s)/sizeof(f62s[0]) },
 { "EZS","D_RQ_FSCM",UINT32_C(0x778),f63s,sizeof(f63s)/sizeof(f63s[0]) },
 { "EZS","D_RQ_GS",UINT32_C(0x7e1),f64s,sizeof(f64s)/sizeof(f64s[0]) },
 { "EZS","D_RQ_ISM",UINT32_C(0x6ea),f65s,sizeof(f65s)/sizeof(f65s[0]) },
 { "EZS","D_RQ_KOMBI_C",UINT32_C(0x796),f66s,sizeof(f66s)/sizeof(f66s[0]) },
 { "EZS","D_RQ_LWR",UINT32_C(0x794),f67s,sizeof(f67s)/sizeof(f67s[0]) },
 { "EZS","D_RQ_MS",UINT32_C(0x7e0),f68s,sizeof(f68s)/sizeof(f68s[0]) },
 { "EZS","D_RQ_RGS_L",UINT32_C(0x7b2),f69s,sizeof(f69s)/sizeof(f69s[0]) },
 { "EZS","D_RQ_RGS_R",UINT32_C(0x7b4),f70s,sizeof(f70s)/sizeof(f70s[0]) },
 { "EZS","D_RQ_UP28",UINT32_C(0x7a2),f71s,sizeof(f71s)/sizeof(f71s[0]) },
 { "EZS","GS_APPL2",UINT32_C(0x6e4),f72s,sizeof(f72s)/sizeof(f72s[0]) },
 { "EZS","MS_APPL1",UINT32_C(0x74a),f73s,sizeof(f73s)/sizeof(f73s[0]) },
 { "EZS","MS_APPL3",UINT32_C(0x6e0),f74s,sizeof(f74s)/sizeof(f74s[0]) },
 { "EZS","EZS_MESS1",UINT32_C(0x60e),f75s,sizeof(f75s)/sizeof(f75s[0]) },
 { "EZS","DG_RQ_OBD",UINT32_C(0x7df),f76s,sizeof(f76s)/sizeof(f76s[0]) },
 { "EZS","VIN",UINT32_C(0x6fa),f77s,sizeof(f77s)/sizeof(f77s[0]) },
 { "KOMBI","KOMBI_408h",UINT32_C(0x408),f78s,sizeof(f78s)/sizeof(f78s[0]) },
 { "KOMBI","KOMBI_412h",UINT32_C(0x412),f79s,sizeof(f79s)/sizeof(f79s[0]) },
 { "KOMBI","D_RS_KOMBI_C",UINT32_C(0x797),f80s,sizeof(f80s)/sizeof(f80s[0]) },
 { "KOMBI","KOMBI_MESS1",UINT32_C(0x680),f81s,sizeof(f81s)/sizeof(f81s[0]) },
 { "KOMBI","KOMBI_MESS2",UINT32_C(0x681),f82s,sizeof(f82s)/sizeof(f82s[0]) },
 { "MRM","LRW_236h",UINT32_C(0x236),f83s,sizeof(f83s)/sizeof(f83s[0]) },
 { "MRM","MRM_238h",UINT32_C(0x238),f84s,sizeof(f84s)/sizeof(f84s[0]) },
 { "ANY_ECU","ARCADE_A2",UINT32_C(0x35),f85s,sizeof(f85s)/sizeof(f85s[0]) },
 { "ANY_ECU","MS_ANZ",UINT32_C(0x33d),f86s,sizeof(f86s)/sizeof(f86s[0]) },
 { "ANY_ECU","GS_102h",UINT32_C(0x102),f87s,sizeof(f87s)/sizeof(f87s[0]) },
 { "ANY_ECU","EWM_104h",UINT32_C(0x104),f88s,sizeof(f88s)/sizeof(f88s[0]) },
 { "ANY_ECU","GS_218h",UINT32_C(0x218),f89s,sizeof(f89s)/sizeof(f89s[0]) },
 { "ANY_ECU","SBW_232h",UINT32_C(0x232),f90s,sizeof(f90s)/sizeof(f90s[0]) },
 { "ANY_ECU","ART_250h",UINT32_C(0x250),f91s,sizeof(f91s)/sizeof(f91s[0]) },
 { "ANY_ECU","ART_258h",UINT32_C(0x258),f92s,sizeof(f92s)/sizeof(f92s[0]) },
 { "ANY_ECU","FS_340h",UINT32_C(0x340),f93s,sizeof(f93s)/sizeof(f93s[0]) },
 { "ANY_ECU","PSM_3B4h",UINT32_C(0x3b4),f94s,sizeof(f94s)/sizeof(f94s[0]) },
 { "ANY_ECU","PSM_3B8h",UINT32_C(0x3b8),f95s,sizeof(f95s)/sizeof(f95s[0]) },
 { "ANY_ECU","KOMBI_414h",UINT32_C(0x414),f96s,sizeof(f96s)/sizeof(f96s[0]) },
 { "ANY_ECU","VG_428h",UINT32_C(0x428),f97s,sizeof(f97s)/sizeof(f97s[0]) },
 { "ANY_ECU","LWR_530h",UINT32_C(0x530),f98s,sizeof(f98s)/sizeof(f98s[0]) },
 { "ANY_ECU","CONFIG_6FFh",UINT32_C(0x6ff),f99s,sizeof(f99s)/sizeof(f99s[0]) },
 { "ANY_ECU","D_RS_ART",UINT32_C(0x78f),f100s,sizeof(f100s)/sizeof(f100s[0]) },
 { "ANY_ECU","D_RS_CAS",UINT32_C(0x77b),f101s,sizeof(f101s)/sizeof(f101s[0]) },
 { "ANY_ECU","D_RS_DTR",UINT32_C(0x4a0),f102s,sizeof(f102s)/sizeof(f102s[0]) },
 { "ANY_ECU","D_RS_EHB",UINT32_C(0x79b),f103s,sizeof(f103s)/sizeof(f103s[0]) },
 { "ANY_ECU","D_RS_EHB2",UINT32_C(0x7b1),f104s,sizeof(f104s)/sizeof(f104s[0]) },
 { "ANY_ECU","D_RS_FS",UINT32_C(0x78d),f105s,sizeof(f105s)/sizeof(f105s[0]) },
 { "ANY_ECU","D_RS_ISM",UINT32_C(0x49d),f106s,sizeof(f106s)/sizeof(f106s[0]) },
 { "ANY_ECU","D_RS_LWR",UINT32_C(0x795),f107s,sizeof(f107s)/sizeof(f107s[0]) },
 { "ANY_ECU","D_RS_RGS_L",UINT32_C(0x7b3),f108s,sizeof(f108s)/sizeof(f108s[0]) },
 { "ANY_ECU","D_RS_RGS_R",UINT32_C(0x7b5),f109s,sizeof(f109s)/sizeof(f109s[0]) },
 { "ANY_ECU","D_RS_UP28",UINT32_C(0x7a3),f110s,sizeof(f110s)/sizeof(f110s[0]) },
 { "ANY_ECU","BS_APPL1",UINT32_C(0x634),f111s,sizeof(f111s)/sizeof(f111s[0]) },
 { "ANY_ECU","MS_APPL1",UINT32_C(0x630),f112s,sizeof(f112s)/sizeof(f112s[0]) },
 { "ANY_ECU","MS_APPL3",UINT32_C(0x632),f113s,sizeof(f113s)/sizeof(f113s[0]) },
 { "ANY_ECU","EZS_MESS2",UINT32_C(0x60f),f114s,sizeof(f114s)/sizeof(f114s[0]) },
 { "ANY_ECU","GS_HS1",UINT32_C(0x501),f115s,sizeof(f115s)/sizeof(f115s[0]) },
 { "ANY_ECU","GS_HS2",UINT32_C(0x502),f116s,sizeof(f116s)/sizeof(f116s[0]) },
 { "ANY_ECU","GS_HS3",UINT32_C(0x503),f117s,sizeof(f117s)/sizeof(f117s[0]) },
 { "ANY_ECU","GS_HS4",UINT32_C(0x504),f118s,sizeof(f118s)/sizeof(f118s[0]) },
 { "ANY_ECU","GS_HS5",UINT32_C(0x505),f119s,sizeof(f119s)/sizeof(f119s[0]) },
 { "ANY_ECU","GS_HS6",UINT32_C(0x506),f120s,sizeof(f120s)/sizeof(f120s[0]) },
};

static bool plain(const uint8_t *p,size_t n,uint16_t off,uint16_t len,uint64_t *v)
{
 uint64_t x=0U;uint16_t b;
 if(p==NULL||v==NULL||len==0U||len>64U||(size_t)off+(size_t)len>n*8U)return false;
 for(b=0U;b<len;++b){size_t sb=(size_t)off+b;uint8_t one=(uint8_t)((p[sb/8U]>>(7U-(sb%8U)))&1U);x=(x<<1U)|one;}
 *v=x;return true;
}
static bool masked(const MblinkMercedesEgs52SignalDefinition *s,const uint8_t *p,size_t n,uint64_t *v)
{
 uint64_t frame=0U;unsigned int shift;size_t i;
 if(s==NULL||p==NULL||v==NULL||n!=8U||!s->masked||s->mask_width==0U||s->mask_width>64U)return false;
 if((unsigned int)s->bit_offset+(unsigned int)s->bit_length>64U)return false;
 for(i=0U;i<8U;++i)frame=(frame<<8U)|p[i];
 shift=64U-((unsigned int)s->bit_offset+(unsigned int)s->bit_length);
 if(shift+s->mask_width>64U)return false;
 *v=(frame>>shift)&s->mask;return true;
}
size_t mblink_mercedes_egs52_frame_count(void){return sizeof(defs)/sizeof(defs[0]);}
size_t mblink_mercedes_egs52_signal_count(void){size_t t=0U,i;for(i=0U;i<mblink_mercedes_egs52_frame_count();++i)t+=defs[i].signal_count;return t;}
const MblinkMercedesEgs52FrameDefinition *mblink_mercedes_egs52_frame_at(size_t i){return i<mblink_mercedes_egs52_frame_count()?&defs[i]:NULL;}
size_t mblink_mercedes_egs52_frame_match_count(uint32_t id){size_t c=0U,i;for(i=0U;i<mblink_mercedes_egs52_frame_count();++i)if(defs[i].can_id==id)++c;return c;}
const MblinkMercedesEgs52FrameDefinition *mblink_mercedes_egs52_frame_match_at(uint32_t id,size_t m){size_t c=0U,i;for(i=0U;i<mblink_mercedes_egs52_frame_count();++i){if(defs[i].can_id!=id)continue;if(c++==m)return &defs[i];}return NULL;}
const MblinkMercedesEgs52SignalDefinition *mblink_mercedes_egs52_signal_find(const MblinkMercedesEgs52FrameDefinition *fr,const char *name){size_t i;if(fr==NULL||name==NULL||name[0]=='\0')return NULL;for(i=0U;i<fr->signal_count;++i)if(strcmp(fr->signals[i].name,name)==0)return &fr->signals[i];return NULL;}
bool mblink_mercedes_egs52_decode_signal(const MblinkMercedesEgs52SignalDefinition *s,const uint8_t *p,size_t n,MblinkMercedesEgs52DecodedSignal *d)
{
 MblinkMercedesEgs52DecodedSignal x;uint64_t r;size_t i;
 if(s==NULL||d==NULL)return false;
 if(s->masked){if(!masked(s,p,n,&r))return false;}else if(!plain(p,n,s->bit_offset,s->bit_length,&r))return false;
 memset(&x,0,sizeof(x));x.raw=r;
 switch(s->type){
 case MBLINK_MERCEDES_EGS52_SIGNAL_BOOL:x.boolean_available=true;x.boolean_value=r!=0U;break;
 case MBLINK_MERCEDES_EGS52_SIGNAL_NUMBER:x.physical_available=true;x.physical_value=(double)r*s->multiplier+s->offset;break;
 case MBLINK_MERCEDES_EGS52_SIGNAL_ENUM:for(i=0U;i<s->enum_count;++i)if(s->enum_values[i].raw==r){x.enum_available=true;x.enum_name=s->enum_values[i].name;x.enum_description=s->enum_values[i].description;break;}break;
 case MBLINK_MERCEDES_EGS52_SIGNAL_CHAR:if(r<=UINT8_MAX){x.char_available=true;x.char_value=(char)(uint8_t)r;}break;
 case MBLINK_MERCEDES_EGS52_SIGNAL_ISO_TP:break;
 }
 *d=x;return true;
}
const char *mblink_mercedes_egs52_source_revision(void){return "1b96089660e97c91811b3d9cda6ca6f82b458c69";}
const char *mblink_mercedes_egs52_source_path(void){return "lib/egs52_ecus/can_data.txt";}
