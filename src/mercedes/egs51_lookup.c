// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * Generated family-isolated EGS51 lookup.
 * Source: rnd-ash/ultimate-nag52-fw/lib/egs51_ecus/can_data.txt
 * Revision: 1b96089660e97c91811b3d9cda6ca6f82b458c69
 * Upstream can_data.txt declares the original ECU bit layout big endian.
 */
#include "mblink/mercedes_egs51_lookup.h"
#include <string.h>

static const MblinkMercedesEgs51EnumValue f0s10e[] = {
 { UINT64_C(0), "BREMSE_NBET", "Brake not actuated" },
 { UINT64_C(1), "BREMSE_BET", "brake actuated" },
 { UINT64_C(2), "UNKNOWN", "not defined" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51EnumValue f0s11e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51EnumValue f0s13e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51EnumValue f0s15e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51SignalDefinition f0s[] = {
 { "BRE_KL",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake defective control lamp (EBV_KL at 463/461 / NCV2)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BAS_KL",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Bas defective control lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_INFO_BL",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP Infolramp flashing light",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_INFO_DL",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP Info lamp permanent light",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESP_KL",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP defective control lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ABS_KL",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ABS defective control lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BBV_KL",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"brake pad wear control lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLS_UNT",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake light suppression (EBV_KL at 163 / T0 / T1N)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BLS_PA",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"BLS Parity (straight parity)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BZ200h",UINT16_C(10),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "BLS",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"brake light switch",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f0s10e,sizeof(f0s10e)/sizeof(f0s10e[0]) },
 { "DRTGVL",UINT16_C(16),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"rotary direction wheel front left",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f0s11e,sizeof(f0s11e)/sizeof(f0s11e[0]) },
 { "DVL",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"wheel speed front left",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGVR",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"direction of rotation wheel front right",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f0s13e,sizeof(f0s13e)/sizeof(f0s13e[0]) },
 { "DVR",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Right speed front right",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGTM",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Rad Left for Cruise",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f0s15e,sizeof(f0s15e)/sizeof(f0s15e[0]) },
 { "TM_DL",UINT16_C(50),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"wheel speed links for cruise control",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51EnumValue f1s2e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, upper limit = 1" },
 { UINT64_C(2), "G2", "Gear, upper limit = 2" },
 { UINT64_C(3), "G3", "Gear, upper limit = 3" },
 { UINT64_C(4), "G4", "Gear, upper limit = 4" },
 { UINT64_C(5), "G5", "Gear, upper limit = 5" },
 { UINT64_C(6), "G6", "Gear, upper limit = 6" },
 { UINT64_C(7), "G7", "Gear, upper limit = 7" },
};
static const MblinkMercedesEgs51EnumValue f1s3e[] = {
 { UINT64_C(0), "PASSIVE", "passive value" },
 { UINT64_C(1), "G1", "Gear, lower limit = 1" },
 { UINT64_C(2), "G2", "Gear, lower limit = 2" },
 { UINT64_C(3), "G3", "Gear, lower limit = 3" },
 { UINT64_C(4), "G4", "Gear, lower limit = 4" },
 { UINT64_C(5), "G5", "Gear, lower limit = 5" },
 { UINT64_C(6), "G6", "Gear, lower limit = 6" },
 { UINT64_C(7), "G7", "Gear, lower limit = 7" },
};
static const MblinkMercedesEgs51EnumValue f1s5e[] = {
 { UINT64_C(0), "ERR", "system error" },
 { UINT64_C(1), "NORM", "normal operation" },
 { UINT64_C(2), "DIAG", "Diagnosis" },
 { UINT64_C(3), "ABGAS", "exhaust gas test" },
};
static const MblinkMercedesEgs51EnumValue f1s7e[] = {
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
static const MblinkMercedesEgs51EnumValue f1s9e[] = {
 { UINT64_C(0), "UNKNOWN", "not defined" },
 { UINT64_C(1), "ANF_N", "requirement \"neutral\"" },
 { UINT64_C(2), "IDLE", "No requirement" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51EnumValue f1s12e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51EnumValue f1s14e[] = {
 { UINT64_C(0), "PASSIVE", "No rotation detection" },
 { UINT64_C(1), "FWD", "direction of rotation forward" },
 { UINT64_C(2), "REV", "direction of rotation backwards" },
 { UINT64_C(3), "SNV", "signal not available" },
};
static const MblinkMercedesEgs51SignalDefinition f1s[] = {
 { "AKT_R_ESP",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP / Art-Wish: \"Active Retract\"",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MINMAX_ART",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gear requirement of art",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GMAX_ESP",UINT16_C(2),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, upper limit",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s2e,sizeof(f1s2e)/sizeof(f1s2e[0]) },
 { "GMIN_ESP",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Gear, lower limit",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s3e,sizeof(f1s3e)/sizeof(f1s3e[0]) },
 { "DDYN_UNT",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Suppression Dynamic fully detection",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SZS",UINT16_C(9),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"system condition",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s5e,sizeof(f1s5e)/sizeof(f1s5e[0]) },
 { "TM_AUS",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tempomat operation",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SLV_ESP",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Switching Difference ESP",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s7e,sizeof(f1s7e)/sizeof(f1s7e[0]) },
 { "BRE_AKT_ESP",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ESP brake engagement active",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANFN",UINT16_C(17),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"ESP request: \"N\" Insert",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s9e,sizeof(f1s9e)/sizeof(f1s9e[0]) },
 { "BRE_AKT_ART",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ART brake intervention active",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MBRE_ESP",UINT16_C(20),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"set braking torque (BR240 factor 1.8 larger)",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGHR",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"rotary direction wheel rear right",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s12e,sizeof(f1s12e)/sizeof(f1s12e[0]) },
 { "DHR",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Rear wheel speed",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "DRTGHL",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"rotary direction wheel rear left",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f1s14e,sizeof(f1s14e)/sizeof(f1s14e[0]) },
 { "DHL",UINT16_C(50),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Rear wheel speed",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51SignalDefinition f2s[] = {
 { "KPL",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"clutch kicked",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KUEB_O_A",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"start.Convertible bridging clutch \"Open\"",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "N_MAX_BG",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limiting function active",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SAST",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Partinal shutdown",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SASV",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"push shutdown full",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KSF_KL",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fuel filter clogs control lamp (CR2 US only)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WKS_KL",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Water in the fuel control lamp (CR2 US only)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZASBED",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Cylinder shutdown conditions fulfilled",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NMOT",UINT16_C(8),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"engine speed",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ELHP_WARN",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Warning message ECO steering helping pump",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EOH",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Ethanol operation detected",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LUFI_KL",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air filter dirty warning lamp (only diesel)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VGL_KL",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"pre-glow control lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OEL_KL",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"oil level / oil pressure control lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DIAG_KL",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Diagnosis Control Lamp (OBD II)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TANK_KL",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tank lid open check lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "UEHITZ",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine oil temperature too high (overheating)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZAS",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Cylinder shutdown",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADR_KL",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ADR check lamp (NFZ only)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADR_DEF_KL",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ADR defective control lamp (NFZ only)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ANL_LFT",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"starter is running",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LUEFT_MOT_KL",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Motor Heater Defective Control Lamp",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DBAA",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed limitation for display active (0 at CR)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TEMP_KL",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cooling water temperature too high",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "T_OEL",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Oil temperature",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "OEL_FS",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"oil level",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "OEL_QUAL",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"oil quality",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51SignalDefinition f3s[] = {
 { "IGN_ANG",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Ignition angle",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,0.35,0.0,"",NULL,0U },
 { "PW",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Pedal position",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,0.4,0.0,"%",NULL,0U },
 { "M_ESP",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Motor torque for ESP",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,3.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51SignalDefinition f4s[] = {
 { "MAX_TRQ_FACTOR",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"factor of max torque",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,0.0078,0.0,"",NULL,0U },
 { "IND_TORQUE",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"engine indicated torque",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,3.0,0.0,"",NULL,0U },
 { "DRG_TORQUE",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"engine drag torque",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,3.0,0.0,"",NULL,0U },
 { "MAX_TORQUE",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"engine max torque",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,3.0,0.0,"",NULL,0U },
 { "MIN_TORQUE",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"engine min torque",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,3.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51SignalDefinition f5s[] = {
 { "T_MOT",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"engine coolant temperature",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "T_LUFT",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"intake air temperature",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FCOD_KAR",UINT16_C(16),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Vehicle code body",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FCOD_BR",UINT16_C(19),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Vehicle code series",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "FCOD_MOT6",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Vehicle code engine with 7 bit, bit 6",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GS_NVH",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission control not available",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FCOD_MOT",UINT16_C(26),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"FZGCOD.Motor 7Bit, bit0-5 (bit6 -> signal fcod_mot6)",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "V_MAX_FIX",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Fixed maximum speed",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "VB",UINT16_C(40),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"consumption",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,0.5,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51EnumValue f6s8e[] = {
 { UINT64_C(0), "G_N", "Destination \"N\"" },
 { UINT64_C(1), "G_D1", "Destination \"1\"" },
 { UINT64_C(2), "G_D2", "Destination \"2\"" },
 { UINT64_C(3), "G_D3", "Destination \"3\"" },
 { UINT64_C(4), "G_D4", "Destination \"4\"" },
 { UINT64_C(5), "G_D5", "Destination \"5\"" },
 { UINT64_C(6), "G_R", "Destination \"R\"" },
 { UINT64_C(7), "G_R2", "Destination \"R2\"" },
 { UINT64_C(8), "G_P", "Destination \"P\"" },
 { UINT64_C(15), "G_SNV", "signal not available" },
};
static const MblinkMercedesEgs51EnumValue f6s9e[] = {
 { UINT64_C(0), "G_N", "Destination \"N\"" },
 { UINT64_C(1), "G_D1", "Destination \"1\"" },
 { UINT64_C(2), "G_D2", "Destination \"2\"" },
 { UINT64_C(3), "G_D3", "Destination \"3\"" },
 { UINT64_C(4), "G_D4", "Destination \"4\"" },
 { UINT64_C(5), "G_D5", "Destination \"5\"" },
 { UINT64_C(6), "G_R", "Destination \"R\"" },
 { UINT64_C(7), "G_R2", "Destination \"R2\"" },
 { UINT64_C(8), "G_P", "Destination \"P\"" },
 { UINT64_C(15), "G_SNV", "signal not available" },
};
static const MblinkMercedesEgs51SignalDefinition f6s[] = {
 { "TORQUE_REQ",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Torque request value. 0xFE when inactive",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,0.5,0.0,"",NULL,0U },
 { "GB_TYPE",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Manual gearbox (1), or automatic (0)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GB_OK",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gearbox program OK",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OFF_ROAD",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Offroad",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gear lever in P or N",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GARAGE_SHIFT",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Garage shifting",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CAN_START",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable starting",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TORQUE_REQ_EN",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable torque request",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GZC",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Target gear",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f6s8e,sizeof(f6s8e)/sizeof(f6s8e[0]) },
 { "GIC",UINT16_C(20),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"actual gear",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f6s9e,sizeof(f6s9e)/sizeof(f6s9e[0]) },
 { "TCC_SLIPPING",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Torque converter slipping",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TCC_OPEN",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Torque converter open",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TCC_CLOSED",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Torque converter closed",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GEARBOX_BIG",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gearbox is W5A580",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LIMP_MODE",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gearbox is in limp-home mode",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SE",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Shifting started",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KICKDOWN",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Kickdown pressed",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FWD",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Front wheel drive",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TCC_MULTI",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"TCC Torque multiplier",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FEHLER",UINT16_C(44),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"error number or counter for calid / CVN transmission",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51EnumValue f7s4e[] = {
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
static const MblinkMercedesEgs51SignalDefinition f7s[] = {
 { "W_S",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Driving program",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FPT",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Driving program button actuated",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KD",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Kickdown",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPERR",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"barrier magnet energized",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WHC",UINT16_C(4),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"gear selector lever position (NAG only)",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f7s4e,sizeof(f7s4e)/sizeof(f7s4e[0]) },
};
static const MblinkMercedesEgs51EnumValue f8s8e[] = {
 { UINT64_C(0), "UNKNOWN", "not defined" },
 { UINT64_C(1), "LL", "Left" },
 { UINT64_C(2), "RL", "RHD" },
 { UINT64_C(3), "SNV", "Code not available" },
};
static const MblinkMercedesEgs51SignalDefinition f8s[] = {
 { "WH_UP",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever implausible",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VMAX_AKT",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Operation variable speed limit",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_MINUS_B",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever: \"Sit and delay Stufe0\"",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "S_PLUS_B",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever: \"Sit and accelerating Stufe0\"",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WA",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever: \"resume\"",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AUS",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever \"off\"",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KG_KL_AKT",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyless Go terminal control active",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KG_ALB_OK",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"meets Keyles Go annealing conditions",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LL_RLC",UINT16_C(10),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"LHD / RHD",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f8s8e,sizeof(f8s8e)/sizeof(f8s8e[0]) },
 { "RG_SCHALT",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Reverse gear engaged (manual transmission only)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BS_SL",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"brake switch for Shift Lock",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_15",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 15",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_50",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 50",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WH_PA",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"cruise control lever parity (even parity)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BZ240h",UINT16_C(20),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message counter",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51SignalDefinition f9s[] = {
 { "ZH_EIN_OK",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn on a heater",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SENDE_NEU",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"signal version Compressor torque",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_KOMPPAR",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Climate Compressor Torque Parity (straight parity)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_KOMPTGL",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Climate Compressor Tour Toggle",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "M_KOMP_NEU",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Climate Compressor Tour NEW",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "LL_DZA",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"idle speed lifting to the cooling power increase",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KOMP_EIN",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"climate compressor turned on",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "P_KAELTE8",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"refrigerant printing",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "M_KOMP",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Torque recording refrigeration compressor",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NLFTS",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Motor fan setpoint speed",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs51EnumValue f10s18e[] = {
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
static const MblinkMercedesEgs51SignalDefinition f10s[] = {
 { "TANK_FS",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Tank level",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "TF_AUF",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"driver's door",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_AUS",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed Limit / Tempose Display Not possible",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TACHO_SYM",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tacho oak",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_MPH",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"MPH instead of km / h (variable speed bends)",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KLA_VH",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air conditioning available",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VGL_KL_DEF",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"pre-glow control lamp defective",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TFSM",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tank level minimum",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KL_61E",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Clamp 61 decoupled",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "T_AUSSEN",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Outdoor air temperature raw value",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "KL_58D",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Terminal 58 dimmed",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "MAZ",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Motor setting time (will be sent from Kl.15)",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "KM16",UINT16_C(40),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"mileage",MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "WRC3",UINT16_C(56),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Winter Tire Top Speed Bit 3",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "V_DSPL_AKT",UINT16_C(57),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Speed Limit / Tempomat Display Active",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SGT_VH",UINT16_C(58),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Segment tacho available",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ZH_FREIG",UINT16_C(59),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Release Heaters",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RT_EIN",UINT16_C(60),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on Roll Test Mode ESP",MBLINK_MERCEDES_EGS51_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WRC",UINT16_C(61),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Winter tire maximum speed with 4 bits",MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,1.0,0.0,"",f10s18e,sizeof(f10s18e)/sizeof(f10s18e[0]) },
};
static const MblinkMercedesEgs51FrameDefinition defs[] = {
 { "ESP51","BS_200h",UINT32_C(0x200),f0s,sizeof(f0s)/sizeof(f0s[0]) },
 { "ESP51","BS_208h",UINT32_C(0x208),f1s,sizeof(f1s)/sizeof(f1s[0]) },
 { "MS51","MS_308h",UINT32_C(0x308),f2s,sizeof(f2s)/sizeof(f2s[0]) },
 { "MS51","MS_210h",UINT32_C(0x210),f3s,sizeof(f3s)/sizeof(f3s[0]) },
 { "MS51","MS_310h",UINT32_C(0x310),f4s,sizeof(f4s)/sizeof(f4s[0]) },
 { "MS51","MS_608h",UINT32_C(0x608),f5s,sizeof(f5s)/sizeof(f5s[0]) },
 { "GS51","GS_218h",UINT32_C(0x218),f6s,sizeof(f6s)/sizeof(f6s[0]) },
 { "EWM51","EWM_230h",UINT32_C(0x230),f7s,sizeof(f7s)/sizeof(f7s[0]) },
 { "EZS51","EZS_240h",UINT32_C(0x240),f8s,sizeof(f8s)/sizeof(f8s[0]) },
 { "EZS51","KLA_410h",UINT32_C(0x410),f9s,sizeof(f9s)/sizeof(f9s[0]) },
 { "KOMBI51","KOMBI_408h",UINT32_C(0x408),f10s,sizeof(f10s)/sizeof(f10s[0]) },
};

static bool plain(const uint8_t *p,size_t n,uint16_t off,uint16_t len,uint64_t *v)
{
 uint64_t x=0U;uint16_t b;
 if(p==NULL||v==NULL||len==0U||len>64U||(size_t)off+(size_t)len>n*8U)return false;
 for(b=0U;b<len;++b){size_t sb=(size_t)off+b;uint8_t one=(uint8_t)((p[sb/8U]>>(7U-(sb%8U)))&1U);x=(x<<1U)|one;}
 *v=x;return true;
}
static bool masked(const MblinkMercedesEgs51SignalDefinition *s,const uint8_t *p,size_t n,uint64_t *v)
{
 uint64_t frame=0U;unsigned int shift;size_t i;
 if(s==NULL||p==NULL||v==NULL||n!=8U||!s->masked||s->mask_width==0U||s->mask_width>64U)return false;
 if((unsigned int)s->bit_offset+(unsigned int)s->bit_length>64U)return false;
 for(i=0U;i<8U;++i)frame=(frame<<8U)|p[i];
 shift=64U-((unsigned int)s->bit_offset+(unsigned int)s->bit_length);
 if(shift+s->mask_width>64U)return false;
 *v=(frame>>shift)&s->mask;return true;
}
size_t mblink_mercedes_egs51_frame_count(void){return sizeof(defs)/sizeof(defs[0]);}
size_t mblink_mercedes_egs51_signal_count(void){size_t t=0U,i;for(i=0U;i<mblink_mercedes_egs51_frame_count();++i)t+=defs[i].signal_count;return t;}
const MblinkMercedesEgs51FrameDefinition *mblink_mercedes_egs51_frame_at(size_t i){return i<mblink_mercedes_egs51_frame_count()?&defs[i]:NULL;}
size_t mblink_mercedes_egs51_frame_match_count(uint32_t id){size_t c=0U,i;for(i=0U;i<mblink_mercedes_egs51_frame_count();++i)if(defs[i].can_id==id)++c;return c;}
const MblinkMercedesEgs51FrameDefinition *mblink_mercedes_egs51_frame_match_at(uint32_t id,size_t m){size_t c=0U,i;for(i=0U;i<mblink_mercedes_egs51_frame_count();++i){if(defs[i].can_id!=id)continue;if(c++==m)return &defs[i];}return NULL;}
const MblinkMercedesEgs51SignalDefinition *mblink_mercedes_egs51_signal_find(const MblinkMercedesEgs51FrameDefinition *fr,const char *name){size_t i;if(fr==NULL||name==NULL||name[0]=='\0')return NULL;for(i=0U;i<fr->signal_count;++i)if(strcmp(fr->signals[i].name,name)==0)return &fr->signals[i];return NULL;}
bool mblink_mercedes_egs51_decode_signal(const MblinkMercedesEgs51SignalDefinition *s,const uint8_t *p,size_t n,MblinkMercedesEgs51DecodedSignal *d)
{
 MblinkMercedesEgs51DecodedSignal x;uint64_t r;size_t i;
 if(s==NULL||d==NULL)return false;
 if(s->masked){if(!masked(s,p,n,&r))return false;}else if(!plain(p,n,s->bit_offset,s->bit_length,&r))return false;
 memset(&x,0,sizeof(x));x.raw=r;x.unit=s->unit;

 /*
  * EGS51 read semantics from the same upstream revision as can_data.txt.
  * Keep these family-local: EGS52/53 use different encodings.
  */
 if ((strcmp(s->name,"T_MOT")==0 || strcmp(s->name,"T_OEL")==0 ||
      strcmp(s->name,"T_LUFT")==0) && r==UINT64_C(255)) {
  x.unavailable=true;*d=x;return true;
 }
 if ((strcmp(s->name,"DHR")==0 || strcmp(s->name,"DHL")==0) &&
     r==UINT64_C(0x3fff)) {
  x.unavailable=true;*d=x;return true;
 }
 if (strcmp(s->name,"PW")==0 && r>UINT64_C(250)) {
  x.unavailable=true;*d=x;return true;
 }
 if ((strcmp(s->name,"IND_TORQUE")==0 || strcmp(s->name,"MIN_TORQUE")==0 ||
      strcmp(s->name,"MAX_TORQUE")==0 || strcmp(s->name,"M_ESP")==0) &&
     r==UINT64_C(255)) {
  x.unavailable=true;*d=x;return true;
 }
 if (strcmp(s->name,"T_MOT")==0 || strcmp(s->name,"T_OEL")==0 ||
     strcmp(s->name,"T_LUFT")==0) {
  x.physical_available=true;x.physical_value=(double)r-40.0;x.unit="°C";
  *d=x;return true;
 }
 if (strcmp(s->name,"VB")==0) {
  x.physical_available=true;x.physical_value=(double)r*0.868;
  x.unit="µL/250ms";*d=x;return true;
 }

 switch(s->type){
 case MBLINK_MERCEDES_EGS51_SIGNAL_BOOL:x.boolean_available=true;x.boolean_value=r!=0U;break;
 case MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER:x.physical_available=true;x.physical_value=(double)r*s->multiplier+s->offset;break;
 case MBLINK_MERCEDES_EGS51_SIGNAL_ENUM:for(i=0U;i<s->enum_count;++i)if(s->enum_values[i].raw==r){x.enum_available=true;x.enum_name=s->enum_values[i].name;x.enum_description=s->enum_values[i].description;break;}break;
 case MBLINK_MERCEDES_EGS51_SIGNAL_CHAR:if(r<=UINT8_MAX){x.char_available=true;x.char_value=(char)(uint8_t)r;}break;
 case MBLINK_MERCEDES_EGS51_SIGNAL_ISO_TP:break;
 }
 *d=x;return true;
}
const char *mblink_mercedes_egs51_source_revision(void){return "1b96089660e97c91811b3d9cda6ca6f82b458c69";}
const char *mblink_mercedes_egs51_source_path(void){return "lib/egs51_ecus/can_data.txt";}
