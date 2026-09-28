// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * Generated family-isolated EGS53 lookup.
 * Source: rnd-ash/ultimate-nag52-fw/lib/egs53_ecus/can_data.txt
 * Revision: 1b96089660e97c91811b3d9cda6ca6f82b458c69
 * Upstream can_data.txt declares the original ECU bit layout big endian.
 */
#include "mblink/mercedes_egs53_lookup.h"
#include <string.h>

static const MblinkMercedesEgs53SignalDefinition f0s[] = {
 { "SwIllLvl",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Switch Illumination Level (Term 58D) / Search Lighting (class 58D)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"%",NULL,0U },
 { "DispBrt_IC",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Display Brightness Instrument Cluster / Display Brightness Combination Strument",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"%",NULL,0U },
 { "DispBrt_HU_V2",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Display Brightness Headunit / Display Brightness Headunit",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"%",NULL,0U },
 { "DispBrt_NV",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Display Brightness Night View / Display Brightness Night View",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"%",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f1s[] = {
 { "EngCoolTemp",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine Coolant Temperature / Motor Coolant Temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,-40.0,"°C",NULL,0U },
 { "IntkAirTemp",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Intake Air Temperature / intake air temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,-40.0,"°C",NULL,0U },
 { "EngOilTemp",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine OIL Temperature / Engine Oil Temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,-40.0,"°C",NULL,0U },
 { "EngOilLvl",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine Oil Level / Engine Oil Level",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.31496062874794006,0.0,"mm",NULL,0U },
 { "EngOilQual",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine Oil Quality / Engine Oil Quality",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019685039296746254,1.0,"",NULL,0U },
 { "FuelCons",UINT16_C(40),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Fuel Consumption / Consumption",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.21668142080307007,0.0,"µl/250ms",NULL,0U },
 { "AirPress_Outsd",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Outside Air Pressure / outdoor air pressure",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,7.795275688171387,0.0,"hPa",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f2s1e[] = {
 { UINT64_C(0), "CLS", "Heating Cutoff Valve is closed" },
 { UINT64_C(1), "OPN", "Heating Cutoff Valve is open" },
 { UINT64_C(2), "CYC", "Heating Cutoff Valve is Cyclic Clocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f2s[] = {
 { "AddWtrPmp_On_Rq_ECM",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Additional Water Pump on Request (by Engine Control Module) / Turn on auxiliary water pump",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HtPwr_Stat",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Heating Power State / Status Heating power",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f2s1e,sizeof(f2s1e)/sizeof(f2s1e[0]) },
 { "HVAC_CompTrq_Max",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Maximum Aircon Compressor Crackish Torque / Limit Momper (Crankshaft) Climate Compressor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,0.0,"Nm",NULL,0U },
 { "Clutch_Disengg",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Clutch is disengaged / clutch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FuelPmp_On_Rq",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fuel Pump on Request / Switch-on request Fuel pump",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngIdleRPM_Dsr",UINT16_C(26),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Desired Engine Idle Speed / Motorle Read Rate",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "EngEff",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Engine Efficiency / Efficiency Combustion Engine",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"%",NULL,0U },
 { "FSCM_Alive",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fuel System Control Module Alive / FSCM Life Sign",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqInc_Enbl_TCM",UINT16_C(50),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Engine Torque Increase Request / Enable Engine Torque Increase TCM",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqIncChk_Avl",UINT16_C(51),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Torque Increase Plausibility Check Available / Plausibility Check for Engine Torque Increase available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ParticleFltrCorrOffset",UINT16_C(52),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Particle Filter Correction Offset on EngtrqMaxCorrfCTR / Particle FilterCorrection value",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.029999999329447746,0.0,"",NULL,0U },
 { "FuelPress_Rq",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Fuel Pressure Request / Request Fuel Pressure",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.05000000074505806,0.0,"bar",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f3s5e[] = {
 { UINT64_C(0), "IGN_LOCK", "Ignition Lock (0)" },
 { UINT64_C(1), "IGN_OFF", "Ignition Off (15c)" },
 { UINT64_C(2), "IGN_ACC", "Ignition Accessory (15R)" },
 { UINT64_C(4), "IGN_ON", "Ignition on (15)" },
 { UINT64_C(5), "IGN_START", "Ignition Start (50)" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s6e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLKS_INSD_SW", "Source IS Central Locking System Inside Switch" },
 { UINT64_C(2), "MENU", "Source Is Settings Menu" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s11e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "ON", "profiles on" },
 { UINT64_C(2), "OFF", "Profiles Off" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s12e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s13e[] = {
 { UINT64_C(0), "P1", "Profile 1" },
 { UINT64_C(1), "P2", "Profile 2" },
 { UINT64_C(2), "P3", "Profile 3" },
 { UINT64_C(3), "P4", "Profile 4" },
 { UINT64_C(4), "DEFAULT1", "Default Profiles" },
 { UINT64_C(5), "DEFAULT2", "Default Profiles" },
 { UINT64_C(6), "DEFAULT3", "Default Profiles" },
 { UINT64_C(7), "DEFAULT4", "Default Profiles" },
 { UINT64_C(8), "DEFAULT5", "Default Profiles" },
 { UINT64_C(9), "DEFAULT6", "Default Profiles" },
 { UINT64_C(10), "DEFAULT7", "Default Profiles" },
 { UINT64_C(11), "DEFAULT8", "Default Profiles" },
 { UINT64_C(12), "DEFAULT9", "Default Profiles" },
 { UINT64_C(13), "DEFAULT10", "Default Profiles" },
 { UINT64_C(14), "DEFAULT11", "Default Profiles" },
 { UINT64_C(15), "DEFAULT12", "Default Profiles" },
};
static const MblinkMercedesEgs53EnumValue f3s14e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LOCK", "Locked" },
 { UINT64_C(2), "UNLOCK", "Unlocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s15e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LOCK", "Locked" },
 { UINT64_C(2), "UNLOCK", "Unlocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s16e[] = {
 { UINT64_C(0), "UNLK", "Vehicle Unlocked" },
 { UINT64_C(1), "INT_LK", "Vehicle Internal Locked" },
 { UINT64_C(2), "EXT_LK", "Vehicle External Locked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s17e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LOCK", "Locked" },
 { UINT64_C(2), "UNLOCK", "Unlocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s18e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LOCK", "Locked" },
 { UINT64_C(2), "UNLOCK", "Unlocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s19e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LOCK", "Locked" },
 { UINT64_C(2), "UNLOCK", "Unlocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s20e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LOCK", "Locked" },
 { UINT64_C(2), "UNLOCK", "Unlocked" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f3s30e[] = {
 { UINT64_C(0), "UNLK", "Vehicle Unlocked" },
 { UINT64_C(1), "INT_LK", "Vehicle Internal Locked" },
 { UINT64_C(2), "EXT_LK", "Vehicle External Locked" },
 { UINT64_C(3), "SEL_UNLK", "Vehicle Selective Unlocked" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f3s[] = {
 { "TxPkPosn_Rq_SBC_Enbl",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Parking Position Request Enable / SBC request: \"P\" allowed",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxPkPosnAuto_Enbl",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Parking Position Auto Enable / Auto- \"P\" Enabled",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxPkPosn_Emg_Rq",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Parking Position Emergency Request / Request Not- \"P\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxPkPosn_Rq",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Parking Position Request / Ice Wish: \"P\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Ign_On_StProc_Inact",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Ignition is on and starting procedure IS INACTIVE (15x) / Ignition AN / motor start",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ISw_Stat",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Ignition Switch State / Terminal Status",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s5e,sizeof(f3s5e)/sizeof(f3s5e[0]) },
 { "AutoDrLk_Rq_Src",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Automatic Door Lock Request Source / Source of the requirement Automatic door lock",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s6e,sizeof(f3s6e)/sizeof(f3s6e[0]) },
 { "KG_SevKeysDet",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Several keys detected / multiple keys detected",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KG_StSw_Psd",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyless Go Start Switch Pressed / Keyless Go Start Button",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngSt_Enbl_Rq_KG",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Engine Start Request / Keyles Go Ready Conditions",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KG_IgnCtrl_Actv",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyless Go Ignition Control Active / Keyless Go Terminal Control Active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ProfMd_Stat",UINT16_C(16),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Profile Mode State / Profile Mode Actual value",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s11e,sizeof(f3s11e)/sizeof(f3s11e[0]) },
 { "AutoDrLk_Stat",UINT16_C(18),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Automatic Door Lock State / Status Automatic door lock",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s12e,sizeof(f3s12e)/sizeof(f3s12e[0]) },
 { "Prof_Stat",UINT16_C(20),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Actual Profile / News Profile",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s13e,sizeof(f3s13e)/sizeof(f3s13e[0]) },
 { "CLkS_Gas_Dr_Stat",UINT16_C(24),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Gas Door State / ZV Status Tank Flap",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s14e,sizeof(f3s14e)/sizeof(f3s14e[0]) },
 { "CLkS_DL_Stat",UINT16_C(26),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Deck Lid State / ZV Status Tail Cover",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s15e,sizeof(f3s15e)/sizeof(f3s15e[0]) },
 { "CLkS_Lk_Stat",UINT16_C(28),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Lock State / Condition Central Locking",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s16e,sizeof(f3s16e)/sizeof(f3s16e[0]) },
 { "CLkS_Dr_RR_Stat",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Door Rear Right State / ZV Status Door Rear Right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s17e,sizeof(f3s17e)/sizeof(f3s17e[0]) },
 { "CLkS_Dr_RL_Stat",UINT16_C(34),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Door Rear Left State / ZV Status Door Rear Left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s18e,sizeof(f3s18e)/sizeof(f3s18e[0]) },
 { "CLkS_Dr_FR_Stat",UINT16_C(36),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Door Front Right State / ZV Status Door Front Right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s19e,sizeof(f3s19e)/sizeof(f3s19e[0]) },
 { "CLkS_Dr_FL_Stat",UINT16_C(38),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Central Locking System Door Front Left State / ZV Status Door Front Left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s20e,sizeof(f3s20e)/sizeof(f3s20e[0]) },
 { "KeyLine8_Appr",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 8 Approved for Keyless Go / Keyline 8 Released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine7_Appr",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 7 Approved for Keyless GO / Keyline 7 released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine6_Appr",UINT16_C(42),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 6 Approved for Keyless Go / Keyline 6 Released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine5_Appr",UINT16_C(43),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 5 Approved for Keyless Go / Keyline 5 released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine4_Appr",UINT16_C(44),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 4 Approved for Keyless Go / Keyline 4 released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine3_Appr",UINT16_C(45),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 3 Approved for Keyless Go / Keyline 3 Released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine2_Appr",UINT16_C(46),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 2 Approved for Keyless Go / Keyline 2 Released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "KeyLine1_Appr",UINT16_C(47),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Keyline 1 Approved for Keyless Go / Keyline 1 released for kg",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MC_EIS_A1",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CLkS_Lk_Stat3",UINT16_C(53),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Central Locking System Lock State / Condition Central Locking",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f3s30e,sizeof(f3s30e)/sizeof(f3s30e[0]) },
 { "CRC_EIS_A1",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f4s2e[] = {
 { UINT64_C(0), "OK", "No Error" },
 { UINT64_C(1), "TIMEOUT", "Timeout" },
 { UINT64_C(2), "FATAL", "Fatal Error" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f4s3e[] = {
 { UINT64_C(0), "DSABL", "Disable" },
 { UINT64_C(1), "ENBL", "Enable" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f4s[] = {
 { "VehSpd_Disp",UINT16_C(4),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Displayed Vehicle Speed (Without Attenuation) / Displayed Speed (without pointer attenuation)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.10000000149011612,0.0,"km/h",NULL,0U },
 { "AirTemp_Outsd_Disp",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Displayed Outside Air Temperature / Displayed Outdoor Air Temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,-40.0,"°C",NULL,0U },
 { "PTS_Disp_Stat_IC",UINT16_C(24),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"PTS DISPLAY STATE FROM IC / PTS Display state",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f4s2e,sizeof(f4s2e)/sizeof(f4s2e[0]) },
 { "HiBm_Enbl",UINT16_C(26),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"HIGH BEAM ENABLE / TRANSPORT LIGHT",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f4s3e,sizeof(f4s3e)/sizeof(f4s3e[0]) },
 { "EngShutOffTm",UINT16_C(28),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Engine Shut-Off Time / Motor Storage Time",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"min",NULL,0U },
 { "Odo",UINT16_C(40),UINT16_C(24),false,UINT64_C(0),UINT8_C(0),"Odometer (for Everyone, Fffffeh: Signal Invalid) / Mileage (for all, Fffffeheh: signal invalid)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.10000000149011612,0.0,"km",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f5s0e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s1e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s2e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "ACTIVATE", "Activate" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s3e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "OFF", "FTW OFF" },
 { UINT64_C(2), "ON", "FTW RE-ACTIVATE" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s4e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s5e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s6e[] = {
 { UINT64_C(0), "PRIVATE", "Private" },
 { UINT64_C(1), "BUSINESS", "on business" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s7e[] = {
 { UINT64_C(0), "NA", "NOT AVAILABLE" },
 { UINT64_C(1), "AVL", "Available" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s8e[] = {
 { UINT64_C(0), "CELSIUS", "Celsius" },
 { UINT64_C(1), "FAHRENHEIT", "Fahrenheit" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s9e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s10e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s11e[] = {
 { UINT64_C(0), "NO_RQ", "No Request" },
 { UINT64_C(1), "TAX_RQ", "Tax Request" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s12e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s13e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s14e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s15e[] = {
 { UINT64_C(0), "BAR", "bar" },
 { UINT64_C(1), "PSI", "PSI" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s16e[] = {
 { UINT64_C(0), "IDLE", "No Warning" },
 { UINT64_C(1), "LVL1", "Warning Level 1" },
 { UINT64_C(2), "LVL2", "Warning Level 2" },
 { UINT64_C(3), "LVL3", "Warning Level 3" },
 { UINT64_C(4), "LVL4", "Warning Level 4" },
 { UINT64_C(5), "LVL5", "Warning Level 5" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s17e[] = {
 { UINT64_C(0), "OFF", "Speed ​​Limit Assist Off" },
 { UINT64_C(1), "ON_NO_WARN", "Speed ​​Limit Assist on Without Warning" },
 { UINT64_C(2), "ON_WARN", "Speed ​​Limit Assist on with Warning" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s18e[] = {
 { UINT64_C(0), "OFF", "ALDW OFF" },
 { UINT64_C(1), "EARLY", "Aldw on, Warning Level Early" },
 { UINT64_C(2), "MID", "Aldw on, Warning Level Mid" },
 { UINT64_C(3), "LATE", "Aldw on, Warning Level Late" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s19e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "PERM", "System Active (permanently)" },
 { UINT64_C(2), "AUTO", "System Active (Automatically)" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s20e[] = {
 { UINT64_C(0), "DSABL", "Disable" },
 { UINT64_C(1), "ENBL", "Enable" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s21e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s22e[] = {
 { UINT64_C(0), "ENBL", "Enable" },
 { UINT64_C(1), "DSABL", "Disable" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s23e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s24e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s25e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s26e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s27e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s28e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s29e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f5s30e[] = {
 { UINT64_C(0), "COMFORT", "Comfort Fashion" },
 { UINT64_C(1), "SPORT", "Sport Fashion" },
 { UINT64_C(2), "MANUAL", "Manual fashion" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f5s[] = {
 { "RadarSensMd_Rq",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Radar Sensor Mode Request / Request Radar Sensoric Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s0e,sizeof(f5s0e)/sizeof(f5s0e[0]) },
 { "DRLt_On_Rq",UINT16_C(2),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Daytime Running Lamps on Request / Request Turn on daytime running light",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s1e,sizeof(f5s1e)/sizeof(f5s1e[0]) },
 { "TPM_Actv_Rq_V2",UINT16_C(4),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Tire Pressure Module Activate Request / Request Tire Pressure Control Activate",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s2e,sizeof(f5s2e)/sizeof(f5s2e[0]) },
 { "FTW_On_Rq",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Flat Tire Warning On Request / Request PlatRollwarner",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s3e,sizeof(f5s3e)/sizeof(f5s3e[0]) },
 { "TaxiToneMd_Rq",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Taxi Tone Fashion Request / Request Taxiton Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s4e,sizeof(f5s4e)/sizeof(f5s4e[0]) },
 { "TaxiRoofLmpMd_Rq",UINT16_C(10),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Taxi Roof Lamps Fashion Request / Request Roof Sign Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s5e,sizeof(f5s5e)/sizeof(f5s5e[0]) },
 { "TaxiMd_Rq",UINT16_C(12),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Taxi Fashion Request / Request Taximodus",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s6e,sizeof(f5s6e)/sizeof(f5s6e[0]) },
 { "DataRadioMenu_Stat",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Data Radio Menu State / Status Data Feature Menu",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s7e,sizeof(f5s7e)/sizeof(f5s7e[0]) },
 { "UnitTemp_Rq",UINT16_C(16),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Temperature Unit / Temperature Unit",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s8e,sizeof(f5s8e)/sizeof(f5s8e[0]) },
 { "DRVM_AudioMd_Rq",UINT16_C(18),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"DRVM Audio Mode Request / Request DRVM Audiomodus",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s9e,sizeof(f5s9e)/sizeof(f5s9e[0]) },
 { "DRVM_SysMd_Rq",UINT16_C(20),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"DRVM System Mode Request / Request DRVM System Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s10e,sizeof(f5s10e)/sizeof(f5s10e[0]) },
 { "TAX_Rq",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Taximeter Request / Request Taximeter",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s11e,sizeof(f5s11e)/sizeof(f5s11e[0]) },
 { "IHC_Md_Rq",UINT16_C(24),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"IHC Mode Request / Request IHC Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s12e,sizeof(f5s12e)/sizeof(f5s12e[0]) },
 { "ECO_Md_Rq",UINT16_C(26),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Eco Mode Request / Request ECO Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s13e,sizeof(f5s13e)/sizeof(f5s13e[0]) },
 { "AFS_Md_Rq",UINT16_C(28),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"AFS Mode Request / Request AFS Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s14e,sizeof(f5s14e)/sizeof(f5s14e[0]) },
 { "UnitPress_Rq",UINT16_C(30),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Pressure Unit / Pressure Unit",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s15e,sizeof(f5s15e)/sizeof(f5s15e[0]) },
 { "SLA_WarnLvl_Rq",UINT16_C(32),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Speed ​​Limit Assist Warning Level Request / Warning Spring Request Speed ​​Limitation Assistant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s16e,sizeof(f5s16e)/sizeof(f5s16e[0]) },
 { "SLA_Md_Rq",UINT16_C(35),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"SPEED LIMIT ASSIST MODE REQUEST / MODE DESCRIPTION SPEED LIMITATION WATER",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s17e,sizeof(f5s17e)/sizeof(f5s17e[0]) },
 { "ALDW_Md_Rq",UINT16_C(37),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"ALDW Fashion Request / Request ALDW mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s18e,sizeof(f5s18e)/sizeof(f5s18e[0]) },
 { "BSM_Md_Rq",UINT16_C(40),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BSM Mode Request / Request BSM Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s19e,sizeof(f5s19e)/sizeof(f5s19e[0]) },
 { "BSM_AcustWarn_Enbl",UINT16_C(42),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BSM Acoustical Warning Enable / BSM Acoustic Warning allowed",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s20e,sizeof(f5s20e)/sizeof(f5s20e[0]) },
 { "PTS_Md_Rq",UINT16_C(44),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Parktronic Mode Request / Parktronic Mode Set",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s21e,sizeof(f5s21e)/sizeof(f5s21e[0]) },
 { "PTS_AcustWarn_Enbl",UINT16_C(46),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Parktronic Acoustic Warning Enable / PTS acoustic warning released",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s22e,sizeof(f5s22e)/sizeof(f5s22e[0]) },
 { "VehDrvProgSys4_Md_Rq",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request - Steering / Driving Program for Steering",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s23e,sizeof(f5s23e)/sizeof(f5s23e[0]) },
 { "VehDrvProgSys3_Md_Rq",UINT16_C(50),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request - Drive / Drive Program for powertrain",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s24e,sizeof(f5s24e)/sizeof(f5s24e[0]) },
 { "VehDrvProgSys2_Md_Rq",UINT16_C(52),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request - Brake / Driving program for brake",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s25e,sizeof(f5s25e)/sizeof(f5s25e[0]) },
 { "VehDrvProgSys1_Md_Rq",UINT16_C(54),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request - Suspension / Driving Program for Suspension",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s26e,sizeof(f5s26e)/sizeof(f5s26e[0]) },
 { "VehDrvProgSys8_Md_Rq",UINT16_C(56),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request / Driving Program for",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s27e,sizeof(f5s27e)/sizeof(f5s27e[0]) },
 { "VehDrvProgSys7_Md_Rq",UINT16_C(58),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request / Driving Program for",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s28e,sizeof(f5s28e)/sizeof(f5s28e[0]) },
 { "VehDrvProgSys6_Md_Rq",UINT16_C(60),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request - DTR / Driving program for DISTRONIC",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s29e,sizeof(f5s29e)/sizeof(f5s29e[0]) },
 { "VehDrvProgSys5_Md_Rq",UINT16_C(62),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Fashion Request - ESP / Driving Program for ESP",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f5s30e,sizeof(f5s30e)/sizeof(f5s30e[0]) },
};
static const MblinkMercedesEgs53EnumValue f6s10e[] = {
 { UINT64_C(0), "IHC_ACTV", "IHC Activated" },
 { UINT64_C(1), "IHC_FLT", "IHC Fault" },
 { UINT64_C(2), "TEMP_NAVL", "IHC Temporarily Not Available" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f6s[] = {
 { "NS_Ill_Actv",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Night Security Illumination Active / Headlight Cable",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADL_Actv",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Automatic Driving Light Activated by Light Sensor / AFL activated by light sensor",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FogLmp_R_On_Rq",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"FOG LAMPS REAR ON REQUEST / NUTLE FLIGHT LIGHT",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FogLmp_Ft_On_Rq",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Turn on Fog Lamps Front On Request / Fog Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LoBm_On_Rq",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on Low Beam On Request / Low Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PosnLmp_On_Rq",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Position Lamps on Request / Power Land",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PkLmp_Rt_On_Rq",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Parking Lamps Right Side On Request / Parking Light On the right",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PkLmp_Lt_On_Rq",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Parking LAMPS LEFT SIDE ON REQUEST / PARK LIGHT Turn on the left",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PosnLmp_IndLmp_On_Rq",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Position LAMP Indication Lamp On Request / Landlight Turn on Control Lamp",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FogLmp_R_IndLmp_On_Rq",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fog Lamps Rear Indication Lamp On Request / Nebula Ficklets Turn on Control Lamp",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "IHC_Stat",UINT16_C(11),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Intelligent Headlight Control State / State Intelligent Headlight Control",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f6s10e,sizeof(f6s10e)/sizeof(f6s10e[0]) },
 { "LoBm_IndLmp_On_Rq",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Low Beam Indication Lamp On Request / Switch on Control Lamp",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FogLmp_Ft_IndLmp_On_Rq",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fog Lamps Front Indication Lamp On Request / Fog Light Control Lamp",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SwIll_Off_Rq",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch Illumination Off Request / Request Search Lighting",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f7s8e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "CONT", "Continuous Light" },
 { UINT64_C(2), "BLINK", "Blinking Light" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f7s9e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "CONT", "Continuous Light" },
 { UINT64_C(2), "BLINK", "Blinking Light" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f7s[] = {
 { "SPC_Msg2_Disp_Rq",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display Message # 2: \"Level Selection Cleared\" / Message 2: \"Level selection deleted\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPC_Msg1_Disp_Rq",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display Message # 1: \"Vehicle Lifts Up\" / Message 1: \"Vehicle Lifts\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPC_ErrMsg4_Disp_Rq",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display Error Message # 4: \"Shut Down Vehicle\" / Error 4: \"Stopping vehicle\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPC_ErrMsg3_Disp_Rq",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display Error Message # 3: \"Seek Service Soon\" / Error 3: \"Check the workshop\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPC_ErrMsg2_Disp_Rq",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display Error Message # 2: \"Please wait, Vehicle lifts up\" (ASP), \"Steering Oil\" (ABC) / Error 2: \"Please wait, vehicle lifts\" (ASP), \"Steering Oil\" (ABC)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPC_ErrMsg1_Disp_Rq",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Display Error Message # 1: \"Stop, Vehicle to Low\" / Error 1: \"Stop, car too deep\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VehLvlCtrl_Stat",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Vehicle Level Control State / Status Level Regulation",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADC_Stat",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Active Damping Control State / Status Active Damping Control",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ADC_SwLED_Rq",UINT16_C(12),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Active Damping Control Switch LED Request / Request LED Button Active Damping Control",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f7s8e,sizeof(f7s8e)/sizeof(f7s8e[0]) },
 { "SuspLvlAdjSwLED_Rq",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Suspension Level Adjustment Switch LED Request / Request LED Vehicle Level Button",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f7s9e,sizeof(f7s9e)/sizeof(f7s9e[0]) },
};
static const MblinkMercedesEgs53EnumValue f8s2e[] = {
 { UINT64_C(0), "INIT_PSBL", "Steering Wheel Angle Sensor Can Be Initialized" },
 { UINT64_C(1), "INIT_SELF", "Steering Wheel Angle Sensor is self-initializing" },
 { UINT64_C(2), "INIT_MUST", "(Steering Wheel Angle Sensor Must Initialized)" },
};
static const MblinkMercedesEgs53EnumValue f8s3e[] = {
 { UINT64_C(0), "OK", "Steering Wheel Angle Sensor OK" },
 { UINT64_C(1), "INI", "Steering Wheel Angle Sensor Not Initialized" },
 { UINT64_C(2), "ERR", "Steering Wheel Angle Sensor Fault" },
 { UINT64_C(3), "ERR_INI", "Steering Wheel Angle Sensor Fault and Not Initialized" },
};
static const MblinkMercedesEgs53SignalDefinition f8s[] = {
 { "StW_Angl",UINT16_C(2),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Steering Wheel Angle / steering wheel angle",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,-2048.0,"°",NULL,0U },
 { "StW_AnglSpd",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Steering Wheel Angle Speed / Steering Wheel Speed",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,-2048.0,"°/s",NULL,0U },
 { "StW_AnglSens_Id",UINT16_C(36),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Steering Wheel Angle Sensor Identification / Identification Steering wheel angle sensor",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f8s2e,sizeof(f8s2e)/sizeof(f8s2e[0]) },
 { "StW_AnglSens_Stat",UINT16_C(38),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Steering Wheel Angle Sensor State / Status Steering wheel angle sensor",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f8s3e,sizeof(f8s3e)/sizeof(f8s3e[0]) },
 { "MC_STW_ANGL_STAT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_STW_ANGL_STAT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f9s0e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "ENGG", "Engage Request" },
 { UINT64_C(2), "RELS", "Release Request" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f9s1e[] = {
 { UINT64_C(0), "UPSTOP", "Pedal upstopped" },
 { UINT64_C(1), "PSD", "Pedal Pressed" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f9s2e[] = {
 { UINT64_C(0), "IDLE", "Vehicle Not Stoped, Or Not Held by Assistance System" },
 { UINT64_C(1), "STOP", "Vehicle Stopped and Held by Assistance System (Using Service Brake)" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f9s3e[] = {
 { UINT64_C(0), "IDLE", "No Braking" },
 { UINT64_C(1), "BRAKING", "Braking" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f9s10e[] = {
 { UINT64_C(0), "IDLE", "normal surgery" },
 { UINT64_C(1), "HOLD", "HOLD MODE ACTIVE" },
 { UINT64_C(2), "GO", "Go Mode Active" },
 { UINT64_C(4), "SLP", "Vehicle Slips" },
 { UINT64_C(8), "SPCR_ACTV", "Stop Coordinator Active" },
 { UINT64_C(9), "SPCR_ACTV_HOLD", "SPCR in Activity and Hold Mode Active" },
 { UINT64_C(10), "SPCR_ACTV_GO", "SPCR in Activity and Go Mode Active" },
 { UINT64_C(12), "SPCR_ACTV_SLP", "SPCR in Activity and Vehicle Slips" },
 { UINT64_C(15), "SPCR_PSV", "SPCR Passive" },
};
static const MblinkMercedesEgs53EnumValue f9s14e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "SPDCTRLLVR_DSABL", "Disable Speed ​​Control Lever Request (Driver Door Open)" },
 { UINT64_C(2), "AS_DSABL", "Disable Assistance System Request (Engine Hood / Deck Lid Open)" },
 { UINT64_C(3), "SPDCTRLLVR_AS_DSABLE", "Disable Speed ​​Control Lever and Assistance System Request" },
};
static const MblinkMercedesEgs53EnumValue f9s15e[] = {
 { UINT64_C(0), "HOLD", "NOT ENOUGH STARTING TORQUE AVAILABLE" },
 { UINT64_C(1), "RELS", "Enough Starting Torque Available" },
 { UINT64_C(2), "UNDET", "Starting Torque undetermined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f9s16e[] = {
 { UINT64_C(0), "ERR", "System Error" },
 { UINT64_C(1), "NORM", "normal surgery" },
 { UINT64_C(2), "DIAG", "Diagnostics" },
 { UINT64_C(3), "EMT", "Exhaust Emission Test" },
};
static const MblinkMercedesEgs53EnumValue f9s18e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "AS_TMP_OFF", "Assistance System Temporary Off, Enabling Not Allowed" },
 { UINT64_C(2), "AS_CNTS_OFF", "Assistance System Continously Off, Enabling Not Allowed" },
 { UINT64_C(3), "SPCR_NA", "Stop Coordinator Not Available, Enabling Not Allowed" },
};
static const MblinkMercedesEgs53EnumValue f9s19e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "AS_NOT_ENBL", "Enabling Assistance System Not Allowed (Temperature)" },
 { UINT64_C(2), "AS_DSABL", "Disable Assistance System Request (Temperature)" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f9s[] = {
 { "PkBrk_Rq_SBC",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Parking Brake Request / parking brake request",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s0e,sizeof(f9s0e)/sizeof(f9s0e[0]) },
 { "BrkPdl_Stat",UINT16_C(2),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Brake Pedal State / Status Brake Pedal",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s1e,sizeof(f9s1e)/sizeof(f9s1e[0]) },
 { "SPCR_Sp_Stat",UINT16_C(4),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Stop Coordinator Stop State / Status Standstill",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s2e,sizeof(f9s2e)/sizeof(f9s2e[0]) },
 { "Brk_Stat",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Brake State / Status Brake",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s3e,sizeof(f9s3e)/sizeof(f9s3e[0]) },
 { "FullBrk_Actv",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Full Braking (ABS Regulates All Wheels) / Full Braking (ABS regulates all 4 wheels)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EmgBrk_Actv",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Emergency Braking / Emergency Braking",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BrkIntrvntn_Enbl",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake Intervention enabled / brake intervention allowed",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BrkIntrvntn_Actv_EPKB",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake Intervention by EPKB Active / EPKB brake intervention active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BrkIntrvntn_Actv_AS",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake Intervention by Assistance System Active / Assistance System Brake Intervention",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BrkIntrvntn_Actv_ESP",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake Intervention by ESP Active / ESP brake intervention active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPCR_Md_V3",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Stop Coordinator Mode / Mode Standstill Coordinator",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s10e,sizeof(f9s10e)/sizeof(f9s10e[0]) },
 { "BrkTrq",UINT16_C(20),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Actual Brake Torque / set braking torque",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,3.0,0.0,"Nm",NULL,0U },
 { "TxPkPosn_Rq_SBC",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Parking Position Request / SBC request: \"P\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPCR_Veh_Immo",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Vehicle Immobilized by Stop Coordinator / Standstill Coordinator has secured vehicle",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SPCR_Excpt_Rq",UINT16_C(34),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Stop Coordinator Exception Request / Standstill Coordinator Exception Request",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s14e,sizeof(f9s14e)/sizeof(f9s14e[0]) },
 { "StTrq_Stat",UINT16_C(36),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Starting Torque State (to Release Parking Brake) / Status Tracking Torque (for loosening parking brake)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s15e,sizeof(f9s15e)/sizeof(f9s15e[0]) },
 { "ESP_Sys_Stat",UINT16_C(38),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"ESP System State / ESP system condition",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s16e,sizeof(f9s16e)/sizeof(f9s16e[0]) },
 { "MC_BRK_STAT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "SPCR_AS_Off_Rq",UINT16_C(52),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Assistance System Off Request / Request Assistance System",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s18e,sizeof(f9s18e)/sizeof(f9s18e[0]) },
 { "SPCR_AS_Dsabl",UINT16_C(54),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Disable Assistance System / Shutdown Assistance System",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f9s19e,sizeof(f9s19e)/sizeof(f9s19e[0]) },
 { "CRC_BRK_STAT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f10s1e[] = {
 { UINT64_C(0), "ACTIVE", "Source Can Active" },
 { UINT64_C(1), "DSABL_TO_MON", "Source Can Inactive, Disable Timeout Monitoring" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53SignalDefinition f10s[] = {
 { "BusFlt_PrmntActv",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Bus Faulty, permanent active / bus faulty, permanent active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CGW_Rout_Stat",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"CGW Routing Status / CGW Routing Status",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f10s1e,sizeof(f10s1e)/sizeof(f10s1e[0]) },
};
static const MblinkMercedesEgs53SignalDefinition f11s[] = {
 { "BrkTrq_D",UINT16_C(4),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Brake Torque Requested by Driver / Driver requested braking torque",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,3.0,0.0,"Nm",NULL,0U },
 { "BrkTrqGrdnt_D",UINT16_C(20),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Brake Torque Gradient Requested by Driver / Driver requested braking torque gradient",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,97.67977905273438,-200048.0,"Nm/s",NULL,0U },
 { "MC_BRK_STAT2",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_BRK_STAT2",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f12s0e[] = {
 { UINT64_C(0), "EWM", "EWM" },
 { UINT64_C(1), "SCCM", "SCCM" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f12s1e[] = {
 { UINT64_C(0), "NPSD", "Nothing pressed" },
 { UINT64_C(1), "PLUS", "\"+\" pressed" },
 { UINT64_C(2), "MINUS", "\"-\" pressed" },
 { UINT64_C(3), "PLUS_MINUS", "\"+\" and \"-\" pressed" },
 { UINT64_C(4), "NDEF4", "Not Defined" },
 { UINT64_C(5), "NDEF5", "Not Defined" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f12s2e[] = {
 { UINT64_C(0), "OLD", "OLD signal \"TSL_POSN_STW\"" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "OLD_ERR_RES", "Reserved for Old Signal: Errors in SCCM" },
 { UINT64_C(3), "NEW", "New Signals \"TSL_P_PSD_STW\" AND \"TSL_RND_POSN_STW\"" },
};
static const MblinkMercedesEgs53EnumValue f12s3e[] = {
 { UINT64_C(0), "IDLE", "Transmission Selector Lever \"P\" Not Pressed" },
 { UINT64_C(1), "PSD", "Transmission Selector Lever \"P\" pressed" },
 { UINT64_C(2), "INI", "Transmission Selector Lever \"P\" Switch Not Initialized" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f12s4e[] = {
 { UINT64_C(0), "IDLE", "Transmission Selector Lever in idle position" },
 { UINT64_C(1), "R", "Transmission Selector Lever in position \"R\"" },
 { UINT64_C(2), "N_UP", "Transmission Selector Lever in position \"N UP\"" },
 { UINT64_C(4), "N_DOWN", "Transmission Selector Lever in position \"N Down\"" },
 { UINT64_C(6), "INI", "Transmission Selector Lever Not Initialized" },
 { UINT64_C(8), "D", "Transmission Selector Lever in position \"D\"" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f12s[] = {
 { "MsgTxmtId",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Message Transmitter Identification / Transmitter ID",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f12s0e,sizeof(f12s0e)/sizeof(f12s0e[0]) },
 { "StW_Sw_Stat3",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"State Steering Wheel Switch (\"+\", \"-\") / steering wheel keys \"+\", \"-\" actuated",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f12s1e,sizeof(f12s1e)/sizeof(f12s1e[0]) },
 { "TSL_Sgnl_Id_StW",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Signal Identification (Steering Wheel) / Gear Select Lever Signal ID (steering wheel)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f12s2e,sizeof(f12s2e)/sizeof(f12s2e[0]) },
 { "TSL_P_Psd_StW",UINT16_C(10),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever \"P\" Switch Actuated (Steering Wheel) / Gear Select Lever \"P\"",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f12s3e,sizeof(f12s3e)/sizeof(f12s3e[0]) },
 { "TSL_RND_Posn_StW",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever RND Position (Steering Wheel) / Gear Selection Lever RND position (steering wheel)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f12s4e,sizeof(f12s4e)/sizeof(f12s4e[0]) },
 { "MC_SBW_RQ_SCCM",UINT16_C(16),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_SBW_RQ_SCCM",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 3 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 3 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f13s0e[] = {
 { UINT64_C(0), "IDLE", "Not pressed" },
 { UINT64_C(1), "ENGG", "Apply (Engage Pushed)" },
 { UINT64_C(2), "RELS", "Release (Release Pulled)" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f13s1e[] = {
 { UINT64_C(0), "ERR", "Fatal System Error" },
 { UINT64_C(1), "NORM", "normal surgery" },
 { UINT64_C(2), "DIAG", "Diagnostics" },
 { UINT64_C(3), "INIT", "Initialization" },
 { UINT64_C(4), "PART", "Partial surgery" },
 { UINT64_C(5), "NO_ENGG", "Partial Operation, Engaging Not Possible" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f13s2e[] = {
 { UINT64_C(0), "NO_END_POSN", "No end position" },
 { UINT64_C(1), "INC", "InCreasing Tensioning Force" },
 { UINT64_C(2), "DEC", "Decreasing Tensioning Force" },
 { UINT64_C(3), "ENGG", "Parking Brake is engaged" },
 { UINT64_C(4), "RELS", "Parking brake is released" },
 { UINT64_C(5), "NDEF5", "Not Defined" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f13s4e[] = {
 { UINT64_C(0), "IDLE", "No Dynamic Braking" },
 { UINT64_C(1), "SBC", "Dynamic Braking Via SBC" },
 { UINT64_C(2), "EPKB", "Dynamic Braking Via EPKB" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f13s6e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "CLOSE", "Close Battery Coupling Switch" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f13s7e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "P_ENGG", "Engage transmission parking position" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f13s8e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f13s[] = {
 { "PkBrkSw_Stat",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Parking brake switch state / parking brake switch status",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s0e,sizeof(f13s0e)/sizeof(f13s0e[0]) },
 { "EPkBrk_Stat",UINT16_C(2),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Electrical Parking Brake System State / Status EPKB",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s1e,sizeof(f13s1e)/sizeof(f13s1e[0]) },
 { "PkBrk_Stat",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Parking brake state / status parking brake",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s2e,sizeof(f13s2e)/sizeof(f13s2e[0]) },
 { "SBC_Enbl_Rq_EPKB",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable SBC Request / SBC request allowed",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DynBrkMd",UINT16_C(10),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Dynamic Braking Fashion / Mode Dynamic Brakes",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s4e,sizeof(f13s4e)/sizeof(f13s4e[0]) },
 { "BrkTrq_Rq_EPKB",UINT16_C(12),UINT16_C(12),false,UINT64_C(0),UINT8_C(0),"Brake torque request / requested by EPKB braking torque",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,3.0,0.0,"Nm",NULL,0U },
 { "BatCplSw_Rq_EPKB",UINT16_C(26),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Battery Coupling Switch Request / Request Battery Coupling Switch",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s6e,sizeof(f13s6e)/sizeof(f13s6e[0]) },
 { "TxPkPosn_Rq_EPKB",UINT16_C(28),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Parking Position Request / EPKB request: \"P\"",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s7e,sizeof(f13s7e)/sizeof(f13s7e[0]) },
 { "BrkLgt_On_Rq",UINT16_C(30),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Turn on Brake Light on Request / Brake light",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f13s8e,sizeof(f13s8e)/sizeof(f13s8e[0]) },
 { "MC_EPKB_STAT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_EPKB_STAT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f14s[] = {
 { "HVAC_Comp_Ft_On",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Aircon Compressor Front On / Climate Compressor for Front On",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_AuxHt_Enbl",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Auxiliary Heater Enabled / Switch on",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_EngIdleRPM_Inc_Rq",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Idle Speed ​​Increase Request / Idle Speed ​​Lifting to Cooling Elevation",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_Recup_Enbl_Rq",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"HVAC Enable Recuperation Request / Recuperation enable",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SSA_Enbl_Rq_HVAC",UINT16_C(4),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ENABLE STOP / START AUTOMATIC REQUEST / ASS ENABLE",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SSA_EngSt_Rq_HVAC",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"STOP / START AUTOMATIC Engine Start Request / ASS Motorstart Request",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_TxShftPointHt_Inc_Rq",UINT16_C(6),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Shift Point Increase Request for Thermal Comfort (Heating) / Raise of the gear switching point for climate summary increase (heating)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_TxShftPoint_Inc_Rq",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Shift Point Increase Request for Thermal Comfort (Cooling) / Raising of the gear switching point for climate domestic enhancement (cooling)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_CompTrq_Ft",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Front Aircon Compressor Crackish Torque / Climate Compressor Tour For Front",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,0.0,"Nm",NULL,0U },
 { "EngFanRPM_Rq_HVAC",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine Fan RPM Request / Motor Heater Setpoint Speed",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"%",NULL,0U },
 { "AirTemp_Outsd_HtMgt",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Outside Air Temperature for Heating Management / Outdoor air temperature for thermal management",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,-40.0,"°C",NULL,0U },
 { "HtPwr_Ft_Rq",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Heating Power Front Request / Request Heating Power Front",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"%",NULL,0U },
 { "MC_HVAC_RS1",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_HVAC_RS1",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f15s1e[] = {
 { UINT64_C(0), "PASSIVE", "Passive value" },
 { UINT64_C(1), "G1", "Requested gear, upper limit = 1" },
 { UINT64_C(2), "G2", "Requested gear, upper limit = 2" },
 { UINT64_C(3), "G3", "Requested gear, upper limit = 3" },
 { UINT64_C(4), "G4", "Requested gear, upper limit = 4" },
 { UINT64_C(5), "G5", "Requested gear, upper limit = 5" },
 { UINT64_C(6), "G6", "Requested gear, upper limit = 6" },
 { UINT64_C(7), "G7", "Requested gear, upper limit = 7" },
};
static const MblinkMercedesEgs53EnumValue f15s2e[] = {
 { UINT64_C(0), "PASSIVE", "Passive value" },
 { UINT64_C(1), "G1", "Requested gear, lower limit = 1" },
 { UINT64_C(2), "G2", "Requested gear, lower limit = 2" },
 { UINT64_C(3), "G3", "Requested gear, lower limit = 3" },
 { UINT64_C(4), "G4", "Requested gear, lower limit = 4" },
 { UINT64_C(5), "G5", "Requested gear, lower limit = 5" },
 { UINT64_C(6), "G6", "Requested gear, lower limit = 6" },
 { UINT64_C(7), "G7", "Requested gear, lower limit = 7" },
};
static const MblinkMercedesEgs53EnumValue f15s5e[] = {
 { UINT64_C(0), "SKL0", "Shift characteristic displacement \"0\"" },
 { UINT64_C(1), "SKL1", "Shift characteristic displacement \"1\"" },
 { UINT64_C(2), "SKL2", "Shift characteristic displacement \"2\"" },
 { UINT64_C(3), "SKL3", "Shift characteristic displacement \"3\"" },
 { UINT64_C(4), "SKL4", "Shift characteristic displacement \"4\"" },
 { UINT64_C(5), "SKL5", "Shift characteristic displacement \"5\"" },
 { UINT64_C(6), "SKL6", "Shift characteristic displacement \"6\"" },
 { UINT64_C(7), "SKL7", "Shift characteristic displacement \"7\"" },
 { UINT64_C(8), "SKL8", "Shift characteristic displacement \"8\"" },
 { UINT64_C(9), "SKL9", "Shift characteristic displacement \"9\"" },
 { UINT64_C(10), "SKL10", "Shift characteristic displacement \"10\"" },
};
static const MblinkMercedesEgs53EnumValue f15s7e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "RQ_N", "Request \"Neutral\"" },
 { UINT64_C(2), "IDLE", "No Request" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f15s[] = {
 { "GrMinMax_Rq_DTR",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Gear limit request from DTR / nominal gear requirement of DTR",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GrMax_Rq_SBC",UINT16_C(2),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Requested gear, upper limit / target gear, upper limit",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f15s1e,sizeof(f15s1e)/sizeof(f15s1e[0]) },
 { "GrMin_Rq_SBC",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Requested gear, lower limit / target gear, lower limit",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f15s2e,sizeof(f15s2e)/sizeof(f15s2e[0]) },
 { "DynFLDS_Supp_Rq_SBC",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Suppression of dynamic full load downshift request / suppression dynamic Vollastrückschaltung",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ActvDnShift_Rq_SBC",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Active downshift / Active downshift",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ShftChrDsp_Rq_SBC",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Shift characteristic displacement request / demand shift line shift",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f15s5e,sizeof(f15s5e)/sizeof(f15s5e[0]) },
 { "NoGrN_Rq",UINT16_C(21),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"No gear \"N\" request (only for AMT during SBC additional value active) / SBC-S / H active ASG must not switch to \"N\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GrN_Rq_SBC",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Request neutral gear by SBC / SBC request: \"Neutral\"",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f15s7e,sizeof(f15s7e)/sizeof(f15s7e[0]) },
 { "TxRatioMin_Rq_SBC",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Requested minimum transmission ratio (CVT) / target speed, lower limit (CVT)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,0.0,"",NULL,0U },
 { "TxRatioMax_Rq_SBC",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Requested maximum transmission ratio (CVT) / target gear, upper limit (CVT)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,0.0,"",NULL,0U },
 { "MC_TX_RQ_SBC",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_TX_RQ_SBC",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f16s11e[] = {
 { UINT64_C(0), "STOP", "Engine is stopped" },
 { UINT64_C(1), "START", "Engine starting" },
 { UINT64_C(2), "IDLE_UNSTBL", "Engine Idling, Unstable" },
 { UINT64_C(3), "IDLE_STBL", "Engine Idling, Stable" },
 { UINT64_C(4), "UNLIMITED", "Engine Running, Unlimited RPM" },
 { UINT64_C(5), "LIMITED", "Engine Running, Limited RPM" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f16s[] = {
 { "KickDnSw_Psd",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"KICKDOWN SWITCH PRESSED / KICKDOWN OPERATED",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PreHt_Stat",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Proheating State / Preheating Status",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngRPM",UINT16_C(2),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Actual Engine RPM / engine speed",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "EngTrqMaxCorrFctr",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Correction Factor of Maximum Engine Torque Depending On Falling Atmospheric Pressure / Factor for Abwert.d.Max. Mom. At Aufneh.A.Print",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.007795275654643774,0.0,"",NULL,0U },
 { "AccelPdlPosn",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Accelerator Pedal Position / Pedal Value",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.4000000059604645,0.0,"%",NULL,0U },
 { "AccelPdlPosn_Raw",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Accelerator Pedal Position Raw Value / Pedal Value Driver",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.4000000059604645,0.0,"%",NULL,0U },
 { "Term61_Actv",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 61 Active / Clamp 61 active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AddPwrCnsmr_On_Rq",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Additional Power Consumers on Request / Request Additional power consumers",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "GenLoad",UINT16_C(42),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Actual Generator Load / Generator utilization",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,3.225806474685669,0.0,"%",NULL,0U },
 { "MC_ENG_RS3_PT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "AccelPdlPosnSens_Flt",UINT16_C(52),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Accelerator Pedal Position Sensor Fault / Error pedal value transmitter",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngRun_Stat",UINT16_C(53),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Engine Running State / Status Engine Circulation",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f16s11e,sizeof(f16s11e)/sizeof(f16s11e[0]) },
 { "CRC_ENG_RS3_PT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f17s[] = {
 { "EngTrqStatic",UINT16_C(3),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Actual Static Engine Torque / Motor Tour Static",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "EngTrqMaxETC",UINT16_C(19),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"ACTUAL Maximum Engine Torque Including Dynamic Exhaust Turbocharger Torque / Maximum Moment Burner with Exhaust Turbocharger",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "FullOFC_Actv",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fully Overrun Fuel Cutoff Active / push shutdown full",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PartOFC_Actv",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Partly Overrun Fuel Cutoff Active / Part Number Shutdown",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqMinTTC",UINT16_C(35),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Actual Mimimum Engine Torque Including Trailing Throttle Component / Motor Torque Minimal including thrust",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "MC_ENG_RS2_PT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_ENG_RS2_PT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f18s0e[] = {
 { UINT64_C(0), "PASSIVE", "Passive Value" },
 { UINT64_C(1), "G1", "Requested Gear, Upper Limit = 1" },
 { UINT64_C(2), "G2", "Requested Gear, Upper Limit = 2" },
 { UINT64_C(3), "G3", "Requested Gear, Upper Limit = 3" },
 { UINT64_C(4), "G4", "Requested Gear, Upper Limit = 4" },
 { UINT64_C(5), "G5", "Requested Gear, Upper Limit = 5" },
 { UINT64_C(6), "G6", "Requested Gear, Upper Limit = 6" },
 { UINT64_C(7), "G7", "Requested Gear, Upper Limit = 7" },
};
static const MblinkMercedesEgs53EnumValue f18s1e[] = {
 { UINT64_C(0), "PASSIVE", "Passive Value" },
 { UINT64_C(1), "G1", "Requested Gear, Lower Limit = 1" },
 { UINT64_C(2), "G2", "Requested Gear, Lower Limit = 2" },
 { UINT64_C(3), "G3", "Requested Gear, Lower Limit = 3" },
 { UINT64_C(4), "G4", "Requested Gear, Lower Limit = 4" },
 { UINT64_C(5), "G5", "Requested Gear, Lower Limit = 5" },
 { UINT64_C(6), "G6", "Requested Gear, Lower Limit = 6" },
 { UINT64_C(7), "G7", "Requested Gear, Lower Limit = 7" },
};
static const MblinkMercedesEgs53EnumValue f18s6e[] = {
 { UINT64_C(0), "SKL0", "Shift Characteristic Displacement \"0\"" },
 { UINT64_C(1), "SKL1", "Shift Characteristic Displacement \"1\"" },
 { UINT64_C(2), "SKL2", "Shift Characteristic Displacement \"2\"" },
 { UINT64_C(3), "SKL3", "Shift Characteristic Displacement \"3\"" },
 { UINT64_C(4), "SKL4", "Shift Characteristic Displacement \"4\"" },
 { UINT64_C(5), "SKL5", "Shift Characteristic Displacement \"5\"" },
 { UINT64_C(6), "SKL6", "Shift Characteristic Displacement \"6\"" },
 { UINT64_C(7), "SKL7", "Shift Characteristic Displacement \"7\"" },
 { UINT64_C(8), "SKL8", "Shift Characteristic Displacement \"8\"" },
 { UINT64_C(9), "SKL9", "Shift Characteristic Displacement \"9\"" },
 { UINT64_C(10), "SKL10", "Shift Characteristic Displacement \"10\"" },
};
static const MblinkMercedesEgs53EnumValue f18s9e[] = {
 { UINT64_C(0), "IDLE", "No Request" },
 { UINT64_C(1), "DISENGG", "Lockup Clutch Disengage" },
 { UINT64_C(2), "ENGG", "Lockup Clutch Engage" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f18s[] = {
 { "GrMax_Rq_ECM",UINT16_C(2),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Requested Gear, Upper Limit / Sprocket, Upper Border",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f18s0e,sizeof(f18s0e)/sizeof(f18s0e[0]) },
 { "GrMin_Rq_ECM",UINT16_C(5),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Requested Gear, Lower Limit / Sprocket, Lower Border",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f18s1e,sizeof(f18s1e)/sizeof(f18s1e[0]) },
 { "SSA_EngSp",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"STOP / START AUTOMATIC HAS Engine Stopped / ass MotorStop, 1 = Stop, 0 = normal operation",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SSA_Sp_Warn",UINT16_C(9),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"STOP / START AUTOMATIC PRE-WARNING ENGINE STOP / ASS Pre-warning engine stop",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Creep_Off_Rq",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Creep Mode Off Request / Crawl Off",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Gr1_Rq_ECM",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Request Driveaway with 1st Gear by ECM / MS-Wish: \"Attraction 1st gear\"",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ShftChrDsp_Rq",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Shift Characteristic Displacement Request / Request Switching Shift",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f18s6e,sizeof(f18s6e)/sizeof(f18s6e[0]) },
 { "TxRatioMin_Rq_ECM",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Requested Minimum Transmission Ratio (CVT) / Target Translation, Lower Border (CVT)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,0.0,"",NULL,0U },
 { "TxRatioMax_Rq_ECM",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Requested Maximum Transmission Ratio (CVT) / Target Translation, Upper Border (CVT)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,0.0,"",NULL,0U },
 { "TCC_Rq",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Torque Converter Lockup Clutch Request / Kueb setpoint open / slipping",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f18s9e,sizeof(f18s9e)/sizeof(f18s9e[0]) },
 { "TxSlpRPM_Rq_ECM",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Transmission Slip RPM Request / Request Slip speed.",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "MC_TX_RQ_ECM",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "ECM_LHOM",UINT16_C(52),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Control Modules in Limp-Home Operation Fashion / ECM in emergency operation",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngExhstAfterTreat_Actv",UINT16_C(54),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Exhaust-Gas Aftertreatment Active (Diesel) / Engine Exhaust Aftertreatment Active (Diesel)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngRPM_Sens_LHOM",UINT16_C(55),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine RPM Sensor in LIMP-Home Operation Fashion / engine speed sensor in emergency operation",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRC_TX_RQ_ECM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f19s[] = {
 { "CC_Encode_ECM",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Control Module Has Cruise Control Encoded / SerentemPomat is Variant Coded",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrq_Enbl_Rq_AS",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Engine Torque Request / Enable Torque Request AS",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqSel_D_TTC",UINT16_C(3),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Selected Torque by Driver Including Trailing Throttle Component / Preset Tame Driver including Schubrank",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "EngTrqAdjFast_Enbl",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Fast Engine Torque Adjustment / Enable Fast torque setting",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrq_Enbl_Rq_SBC",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Engine Torque Request / Enable Torque Requirement SBC",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqSel_AS_TTC",UINT16_C(19),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Selected Torque By AS Including Trailing Throttle Component / Property Toment AS including thrust",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "EngTrq_Ack_ECM",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Torque Request AcknowledgeGement / acknowledgment Torque requirement",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrq_Enbl_Rq_TCM",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Engine Torque Request / Enable Tomentic Request TCM",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqSel_SBC_TTC",UINT16_C(35),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Selected Torque by SBC Including Trailing Throttle Component / Preset Tame SBC including thrust",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "MC_ENG_RS1_PT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_ENG_RS1_PT",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f20s0e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "NO_EXT", "DPM External Fashion Not Allowed" },
 { UINT64_C(2), "EXT", "DPM External Fashion Allowed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f20s1e[] = {
 { UINT64_C(0), "SLEEP", "DPM LIMIT SLEEP MODE" },
 { UINT64_C(1), "LOCAL", "DPM LIMIT Local Mode" },
 { UINT64_C(2), "EXT", "DPM Limit External Mode" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f20s[] = {
 { "DPM_ExtMd_Enbl_Rq",UINT16_C(4),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Decentral Power Management External Fashion Enable Request / Release External Fashion by Decentralized Power Management",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f20s0e,sizeof(f20s0e)/sizeof(f20s0e[0]) },
 { "DPM_MdLmt_Rq",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Decentral Power Management Fashion Limit Request / Mode Handle Decentralized Power Management",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f20s1e,sizeof(f20s1e)/sizeof(f20s1e[0]) },
};
static const MblinkMercedesEgs53SignalDefinition f21s[] = {
 { "WhlPlsCnt_FL",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wheel Pulse Counter Front Left (96 by rotation) / pulse ring counter wheel front left (96 per revolution)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"Impulses",NULL,0U },
 { "WhlPlsCnt_FR",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wheel Pulse Counter Front Right (96 Per Rotation) / Pulse Ring Counter Wheel Front Right (96 per Revolution)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"Impulses",NULL,0U },
 { "WhlPlsCnt_RL",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wheel Pulse Counter Rear Left (96 by rotation) / pulse ring counter wheel rear left (96 per revolution)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"Impulses",NULL,0U },
 { "WhlPlsCnt_RR",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wheel Pulse Counter Rear Right (96 by rotation) / pulse ring counter wheel rear right (96 per revolution)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"Impulses",NULL,0U },
 { "MC_WHL_STAT1",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_WHL_STAT1",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f22s0e[] = {
 { UINT64_C(0), "VOID", "NO DETECTION OF DIRECTION" },
 { UINT64_C(1), "FORWARD", "DIRECTION FORWARD" },
 { UINT64_C(2), "BACKWARD", "DIRECTION BACKWARD" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f22s2e[] = {
 { UINT64_C(0), "VOID", "NO DETECTION OF DIRECTION" },
 { UINT64_C(1), "FORWARD", "DIRECTION FORWARD" },
 { UINT64_C(2), "BACKWARD", "DIRECTION BACKWARD" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f22s4e[] = {
 { UINT64_C(0), "VOID", "NO DETECTION OF DIRECTION" },
 { UINT64_C(1), "FORWARD", "DIRECTION FORWARD" },
 { UINT64_C(2), "BACKWARD", "DIRECTION BACKWARD" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f22s6e[] = {
 { UINT64_C(0), "VOID", "NO DETECTION OF DIRECTION" },
 { UINT64_C(1), "FORWARD", "DIRECTION FORWARD" },
 { UINT64_C(2), "BACKWARD", "DIRECTION BACKWARD" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f22s[] = {
 { "WhlDir_FL_Stat",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"DIRECTION OF ROTATION OF FRONT LEFT Wheel / direction of rotation Wheel front left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f22s0e,sizeof(f22s0e)/sizeof(f22s0e[0]) },
 { "WhlRPM_FL",UINT16_C(2),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Wheel RPM Front Left / Wheel Speed Front Left",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"rpm",NULL,0U },
 { "WhlDir_FR_Stat",UINT16_C(16),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"DIRECTION OF ROTATION OF FRONT RIGHT Wheel / direction of rotation Wheel front right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f22s2e,sizeof(f22s2e)/sizeof(f22s2e[0]) },
 { "WhlRPM_FR",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Wheel RPM Front Right / wheel speed front right",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"rpm",NULL,0U },
 { "WhlDir_RL_Stat",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"DIRECTION OF ROTATION OF REAR LEFT Wheel / direction of rotation Wheel rear left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f22s4e,sizeof(f22s4e)/sizeof(f22s4e[0]) },
 { "WhlRPM_RL",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Wheel RPM REAR Left / wheel speed rear left",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"rpm",NULL,0U },
 { "WhlDir_RR_Stat",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"DIRECTION OF ROTATION OF REAR RIGHT Wheel / direction of rotation Wheel rear right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f22s6e,sizeof(f22s6e)/sizeof(f22s6e[0]) },
 { "WhlRPM_RR",UINT16_C(50),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Wheel RPM Rear Right / Rear Rear Right Right",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"rpm",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f23s5e[] = {
 { UINT64_C(0), "CLS", "Battery Coupling Switch Closed" },
 { UINT64_C(1), "PREOPN", "Battery Coupling Switch Open in 2 sec" },
 { UINT64_C(2), "OPN", "Battery Coupling Switch Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f23s7e[] = {
 { UINT64_C(0), "CLS", "Battery Cutoff Switch is closed" },
 { UINT64_C(1), "PREOPN", "Battery Cutoff Switch Opens in 300 sec" },
 { UINT64_C(2), "OPN", "Battery Cutoff Switch is open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f23s[] = {
 { "PN14_SPCR_AfterRun_Avl",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Stop Coordinator Afterrun Function Available / Navail Function Standstill Coordinator available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN14_SBC_Add_Off_Rq",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"SBC Additional Value Off Request / SBC Added value",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN14_RevBltTns_Off_Rq",UINT16_C(2),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Reversible Belt Tensioner Off Request / GurtStraffer Turn off",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN14_RemPerm",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Remote Functions Permission / Remote Control Functions allowed",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN14_BackupBat_Flt",UINT16_C(5),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Backup Battery Fault / Backup Battery faulty",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN14_BatCplSw_Stat",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Battery Coupling Switch State / State Battery Coupling Switch",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f23s5e,sizeof(f23s5e)/sizeof(f23s5e[0]) },
 { "PN14_SupBat_Volt",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Supply Battery Voltage / Supply Battery IST voltage",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.10000000149011612,0.0,"V",NULL,0U },
 { "PN14_SupBatCutSw_Stat",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Supply Battery Cutoff Switch State / Status Battery Separation Switch",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f23s7e,sizeof(f23s7e)/sizeof(f23s7e[0]) },
 { "PN14_LHC_Actv",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"14 V PowerNet Limp-Home Cutoff Active / 14V-BN emergency shutdown active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_IntFan_F_50_Rq",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff Front Interior Fan to 50% Request / Emergency Shot In front to 50%",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_IntFan_R_50_Rq",UINT16_C(33),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff Rear Interior Fan to 50% Request / Emergency Shot Inner fan back to 50%",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_PTC1_Rq",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff PTC 1st Branch Request / Notification PTC 1. Branch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_PTC2_Rq",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff PTC 2nd Branch Request / Notification PTC 2. Branch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_PTC3_Rq",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff PTC 3rd Branch Request / Notification PTC 3. Branch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_PTC4_Rq",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff PTC 4th Branch Request / Notification PTC 4. Branch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_PTC5_Rq",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff PTC 5th Branch Request / Notification PTC 5. Branch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_PTC6_Rq",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff PTC 6th Branch Request / Notification PTC 6. Branch",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_TrlrSock_Rq",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Trailer Socket Request / Emergency Shot Towers",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_Tlm_Rq",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Telematics / Audio / Phone / GPS Request / Notification Telematics / Audio / Phone / GPS",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_SeatVn_Rq",UINT16_C(42),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Seat Ventilation Request / Emergency Shot Seat Ventilation",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_SeatHtStg1_Rq",UINT16_C(43),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Seat Heating Stage 1 Request / Emergency Shot Seat Heating Level 1",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_SeatHtStg2_Rq",UINT16_C(44),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Seat Heating Stage 2 Request / Emergency Shot Seat Heating Level 2",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_SeatHtStg3_Rq",UINT16_C(45),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Seat Heating Stage 3 Request / Emergency Shot Seat Heating Level 3",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_DSH_Stg1_Rq",UINT16_C(46),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Door Surface Heating Stage 1 Request / Emergency Shot Door surface heating Level 1",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_DSH_Stg2_Rq",UINT16_C(47),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Door Surface Heating Stage 2 Request / Emergency Shot Door surface heating Level 2",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_FogLmp_Ft_Rq",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Fog Lamps Front Request / Emergency Shot Fog Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_EngFan_50_Rq",UINT16_C(49),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Engine Fan to 50% Request / Emergency Shot Engineer Fan to 50%",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_CoolBox_R_Rq",UINT16_C(50),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Rear Cooler Box Request / Emergency Shot Fund Cooler",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_MirrHt_Rq",UINT16_C(51),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Mirror Heater Request / emergency shutdown Exterior mirror heating",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_WShHt_Rq",UINT16_C(52),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Windshield Heater Request / Emergency Shutters Windscreen Heating",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_StW_Ht_Rq",UINT16_C(53),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff Steering Wheel Heater Request / Emergency Shot Steering Wheeling",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_EBL_Rq",UINT16_C(54),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Electric Backlite Heater Request / Emergency Shot Rear Disc Heating",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_WprPkHt_Rq",UINT16_C(55),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff Wiper Park Position Heater Request / Emergency Shuttle Wiper Dashing",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PN14_LHC_Taxi_Rq",UINT16_C(59),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Taxi RoofSign / Printer Request / Emergency Shot TaxidAchach",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_ICH_Rq",UINT16_C(60),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff Independant Car Heater Request / Emergency shutdown",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_RHU_Rq",UINT16_C(61),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Residual Heat Utilization Request / Notification Residual heat use",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_ComfLmp_Rq",UINT16_C(62),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-Home Cutoff Comfort Lamps Request / Notification for Comfort Lamps",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LHC_TrkSock_Rq",UINT16_C(63),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"LIMP-HOME Cutoff Trunk Socket / Cigarette Lighter Request / Emergency Shot Coffee Socket / Cigar Lighter",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f24s0e[] = {
 { UINT64_C(0), "ROW", "Remain of the world" },
 { UINT64_C(1), "USA", "United States" },
 { UINT64_C(2), "CAN", "Canada" },
 { UINT64_C(3), "JAP", "Japan" },
 { UINT64_C(4), "SWI", "Switzerland" },
 { UINT64_C(5), "AUS", "Australia" },
 { UINT64_C(6), "GULF", "Gulf states" },
 { UINT64_C(7), "UK", "United Kingdom (not 221/216)" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s1e[] = {
 { UINT64_C(0), "MB", "Mercedes Benz, Maybach" },
 { UINT64_C(1), "SMART", "smart" },
 { UINT64_C(2), "CG", "Chrysler Group" },
 { UINT64_C(3), "MMC", "Mitsubishi" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s2e[] = {
 { UINT64_C(0), "NO", "No armoring" },
 { UINT64_C(1), "B4", "Armoring class B4" },
 { UINT64_C(2), "B6", "Armoring class B6 / B7" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s3e[] = {
 { UINT64_C(0), "BR221", "BR 221" },
 { UINT64_C(1), "BR231", "BR 231" },
 { UINT64_C(2), "BR212", "BR 212" },
 { UINT64_C(3), "BR204", "BR 204" },
 { UINT64_C(9), "BR207", "BR 207" },
 { UINT64_C(17), "BR251", "BR 251" },
 { UINT64_C(19), "BR164", "BR 164" },
 { UINT64_C(22), "BR216", "BR 216" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s4e[] = {
 { UINT64_C(30), "NDEF30", "Not Defined" },
 { UINT64_C(31), "START", "Start of vehicle line" },
};
static const MblinkMercedesEgs53EnumValue f24s5e[] = {
 { UINT64_C(0), "PACK0", "Package \"/ 0\"" },
 { UINT64_C(1), "PACK1", "Package \"/ 1\"" },
 { UINT64_C(2), "PACK2", "Package \"/ 2\"" },
 { UINT64_C(3), "PACKX", "Package \"/ X\" or start of vehicle line" },
};
static const MblinkMercedesEgs53EnumValue f24s6e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "LHD", "Left hand drive" },
 { UINT64_C(2), "RHD", "Right hand drive" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s7e[] = {
 { UINT64_C(0), "W", "W - Sedan" },
 { UINT64_C(1), "V", "V - Stretched limo" },
 { UINT64_C(2), "C", "C - Coupe" },
 { UINT64_C(3), "S", "S - wagon station" },
 { UINT64_C(4), "A", "A - Convertible" },
 { UINT64_C(5), "R", "R - Roadster" },
 { UINT64_C(6), "CL", "CL - Sports coupe" },
 { UINT64_C(7), "VV", "VV - Extra streched limousine" },
 { UINT64_C(8), "VF", "VF - Stretched chassis" },
 { UINT64_C(9), "F", "F - Chassis" },
 { UINT64_C(10), "G", "G - Off-road vehicle" },
 { UINT64_C(11), "GV", "GV - Stretched off-road vehicle" },
 { UINT64_C(12), "T", "T - Multi Sports Tourer" },
 { UINT64_C(13), "X", "X - sports utility Tourer" },
 { UINT64_C(31), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s8e[] = {
 { UINT64_C(0), "M275E55", "M275 E55 [ME2.7.2]" },
 { UINT64_C(1), "M273E55", "M273 E55 [ME9.7]" },
 { UINT64_C(2), "M273E46", "M273 E46 [ME9.7]" },
 { UINT64_C(3), "M272E35", "M272 E35 [ME9.7]" },
 { UINT64_C(4), "M272E30", "M272 E30 [ME9.7]" },
 { UINT64_C(5), "M272E25", "M272 E25 [ME9.7]" },
 { UINT64_C(7), "M273E55DE", "M273 E55 DE [ME9.7]" },
 { UINT64_C(8), "M273E46DE", "M273 E46 DE [ME9.7]" },
 { UINT64_C(9), "M272E35DE", "M272 E35 DE [ME9.7]" },
 { UINT64_C(10), "M272E30DE", "M272 E30 DE [ME9.7]" },
 { UINT64_C(11), "M272E25DE", "M272 E25 DE [ME9.7]" },
 { UINT64_C(12), "M271E18ML135ATT", "attrac M271 E18 ML. (135 kW) [SIM271KE]" },
 { UINT64_C(13), "M271E18ML115ATT", "attrac M271 E18 ML. (115 kW) [SIM271KE]" },
 { UINT64_C(14), "M272E35_221", "M272 E35 (221 kW) [ME9.7]" },
 { UINT64_C(123), "AMGM156E63HP", "AMG M156 E63 HP [ME9.7]" },
 { UINT64_C(124), "AMGM275E60LA", "AMG M275 E60 LA [ME2.7.2]" },
 { UINT64_C(125), "AMGM157E60LA", "AMG M157 E60 LA [ME9.7]" },
 { UINT64_C(126), "AMGM156E63", "AMG M156 E63 [ME9.7]" },
 { UINT64_C(129), "OM642DE30LA160", "OM642 DE30 LA (155/160 kW) [CR5 / CR6]" },
 { UINT64_C(130), "OM629DE40LA", "OM629 DE40 LA [CR5]" },
 { UINT64_C(131), "OM642DE30LA140", "OM642 DE30 LA red. (140 kW) [CR6]" },
 { UINT64_C(132), "OM646EVODE22LA125", "OM646EVO DE22 LA (120/125 kW) [CRD]" },
 { UINT64_C(133), "OM646EVODE22LA100", "OM646EVO DE22 LA red. (100 kW) [CRD]" },
 { UINT64_C(134), "OM646EVODE22LA85", "OM646EVO DE22 LA (85 kW) [CRD]" },
 { UINT64_C(135), "OM651DE22LA150", "OM651 DE22 LA (150 kW) [CRD2]" },
 { UINT64_C(136), "OM651DE22LA120", "OM651 DE22 LA (120 kW) [CRD2]" },
 { UINT64_C(137), "OM651DE22LA100", "OM651 DE22 LA (100 kW) [CRD2]" },
 { UINT64_C(138), "OM651DE22LA80", "OM651 DE22 LA (80 kW) [CRD2]" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s17e[] = {
 { UINT64_C(0), "NORM", "Normal Roof" },
 { UINT64_C(1), "TSSR", "tilt / slide sunroof" },
 { UINT64_C(2), "EXTRUN_TSSR", "Exterior running tilt / slide sunroof" },
 { UINT64_C(3), "TSSR_RCLS", "tilt / slide sunroof with rain closure (not 221/216)" },
 { UINT64_C(4), "EXTRUN_TSSR_CLS", "Exterior running tilt / slide sunroof with rain closure" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s19e[] = {
 { UINT64_C(0), "CLASSIC", "Classic" },
 { UINT64_C(1), "ELEGANCE", "Elegance" },
 { UINT64_C(2), "AVANTGARDE", "Avantgarde" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f24s20e[] = {
 { UINT64_C(0), "ONE_ZONE", "One zone HVAC" },
 { UINT64_C(1), "TWO_ZONE", "Two zone HVAC" },
 { UINT64_C(2), "THREE_ZONE", "Three zone HVAC" },
 { UINT64_C(3), "FOUR_ZONE", "Four zone HVAC" },
};
static const MblinkMercedesEgs53SignalDefinition f24s[] = {
 { "Country",UINT16_C(0),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Country code / country code",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s0e,sizeof(f24s0e)/sizeof(f24s0e[0]) },
 { "Group",UINT16_C(4),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"DC group / DC group",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s1e,sizeof(f24s1e)/sizeof(f24s1e[0]) },
 { "Guard",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Guard level / Guard Level",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s2e,sizeof(f24s2e)/sizeof(f24s2e[0]) },
 { "VehLine",UINT16_C(10),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Vehicle line / Series",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s3e,sizeof(f24s3e)/sizeof(f24s3e[0]) },
 { "VehLineYear",UINT16_C(17),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Vehicle line version: year / year change: Year",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s4e,sizeof(f24s4e)/sizeof(f24s4e[0]) },
 { "VehLinePack",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle line version: package / change Year: Package",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s5e,sizeof(f24s5e)/sizeof(f24s5e[0]) },
 { "StStyle",UINT16_C(25),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Steering variant / Steering variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s6e,sizeof(f24s6e)/sizeof(f24s6e[0]) },
 { "BodyStyle",UINT16_C(27),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Vehicle body style / body variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s7e,sizeof(f24s7e)/sizeof(f24s7e[0]) },
 { "EngStyle",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine (Bit 7: Otto-engine => 0, diesel engine => 1) / motor (Bit 7: gasoline => 0, Diesel => 1)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s8e,sizeof(f24s8e)/sizeof(f24s8e[0]) },
 { "RainSens_Avl",UINT16_C(40),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rain sensor available / rain sensor available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LgtSens_Avl",UINT16_C(41),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Light sensor available / light sensor available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ICH_Avl",UINT16_C(42),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Independent car heater available / heater available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PCD_Avl",UINT16_C(43),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Passenger compartment detection available / interior protection available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VTA_Avl",UINT16_C(44),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Vehicle theft alarm available / EDW available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TrlrHtch_Avl",UINT16_C(45),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Trailer Hitch available / trailer coupling available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HVAC_Avl",UINT16_C(46),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Air Condition available / KLA available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TCM_Avl",UINT16_C(47),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Control available / transmission control available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RoofStyle",UINT16_C(48),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Vehicle roof style / roof version",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s17e,sizeof(f24s17e)/sizeof(f24s17e[0]) },
 { "Cplt_SL_Enbl",UINT16_C(51),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Complete substitution lights enabled / Complete replacement light allowed",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VehOPTPack",UINT16_C(52),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle options package / vehicle equipment package",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s19e,sizeof(f24s19e)/sizeof(f24s19e[0]) },
 { "HVACStyle",UINT16_C(54),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"HVAC style / KLA variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f24s20e,sizeof(f24s20e)/sizeof(f24s20e[0]) },
 { "CRC_CVI",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f25s[] = {
 { "VehYawRate_Raw",UINT16_C(0),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Vehicle Yaw Rate Unfiltered / Unadjusted (+ Means Left) / Rohsignal Gierrate Without Calculation / Filtering (+ = Left)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.009999999776482582,-327.67999267578125,"°/s",NULL,0U },
 { "VehYawRateOffset",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Offset of Vehicle Yaw Rate Unfiltered / Unadjusted (+ Means LEFT) / Offset of the Raw Signal Greeding rate without reconciliation / filtering (+ = left)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.07999999821186066,-10.239999771118164,"",NULL,0U },
 { "VehAccel_X_Offset",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Offset of Vehicle Longitudinal Acceleration (+ Means Forward) / Offset of vehicle longitudinal acceleration (+ = forward)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,-2.559999942779541,"m/s²",NULL,0U },
 { "VehAccel_X",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Vehicle Longitudinal Acceleration (+ Means Forward) / vehicle longitudinal acceleration (+ = forward)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.07999999821186066,-10.239999771118164,"m/s²",NULL,0U },
 { "VehAccel_Y",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Vehicle Lateral Acceleration (+ Means LEFT, Specific to Center of Gravity, Offset Corrected) / Vehicle Cross Acceleration in Focus (+ = Left, Focus-related, Offset Corrected)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.07999999821186066,-10.239999771118164,"m/s²",NULL,0U },
 { "MC_VEH_DYN_STAT",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "VehAccel_Y_Offset",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Offset of Vehicle Lateral Acceleration (+ Means Left) / Offset of vehicle cross-acceleration in focus (+ = left)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,-2.559999942779541,"m/s²",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f26s0e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLS", "Door Closed" },
 { UINT64_C(2), "OPN", "Door Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s1e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLS", "Door Closed" },
 { UINT64_C(2), "OPN", "Door Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s2e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLS", "Door Closed" },
 { UINT64_C(2), "OPN", "Door Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s3e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLS", "Door Closed" },
 { UINT64_C(2), "OPN", "Door Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s4e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ON", "ON" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s7e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLS", "Rotary Latch Closed" },
 { UINT64_C(2), "OPN", "Rotary Latch Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s8e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "CLS", "Rotary Latch Closed" },
 { UINT64_C(2), "OPN", "Rotary Latch Open" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s20e[] = {
 { UINT64_C(0), "NONE", "Trailer Not Detected" },
 { UINT64_C(1), "OK", "trailer detected" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s21e[] = {
 { UINT64_C(0), "DISENGG", "disengaged" },
 { UINT64_C(1), "ENGG", "engaged" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s29e[] = {
 { UINT64_C(0), "OFF", "LED Steering Wheel Conditioning Off" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "ON", "LED Steering Wheel Conditioning On" },
 { UINT64_C(3), "BLINK", "LED Steering Wheel Conditioning Blinks Cause of Fault" },
};
static const MblinkMercedesEgs53EnumValue f26s30e[] = {
 { UINT64_C(0), "OK", "Seatbelt Fasted" },
 { UINT64_C(1), "NOT", "Seatbelt Not Fasted" },
 { UINT64_C(2), "FLT", "BUCKLE SWITCH FAULT" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s31e[] = {
 { UINT64_C(0), "OK", "Seatbelt Fasted" },
 { UINT64_C(1), "NOT", "Seatbelt Not Fasted" },
 { UINT64_C(2), "FLT", "BUCKLE SWITCH FAULT" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f26s32e[] = {
 { UINT64_C(0), "OK", "Seatbelt Fasted" },
 { UINT64_C(1), "NOT", "Seatbelt Not Fasted" },
 { UINT64_C(2), "FLT", "BUCKLE SWITCH FAULT" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f26s[] = {
 { "DrRLtch_RR_Stat",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Rotary Latch Door Rear Right State / Status Swivel Fall Door Rear Right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s0e,sizeof(f26s0e)/sizeof(f26s0e[0]) },
 { "DrRLtch_RL_Stat",UINT16_C(2),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Rotary Latch Door Rear Left State / Status Swivel Fall Door Rear Left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s1e,sizeof(f26s1e)/sizeof(f26s1e[0]) },
 { "DrRLtch_FR_Stat",UINT16_C(4),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Rotary Latch Door Front Right State / Status Swivel Fall Door Front Right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s2e,sizeof(f26s2e)/sizeof(f26s2e[0]) },
 { "DrRLtch_FL_Stat",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Rotary Latch Door Front Left State / Status Swivel Fall Door Front Left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s3e,sizeof(f26s3e)/sizeof(f26s3e[0]) },
 { "EF_On_Rq",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Emergency Flasher on Request / Warning Blink Light",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s4e,sizeof(f26s4e)/sizeof(f26s4e[0]) },
 { "VTA_Alm_Actv",UINT16_C(10),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Alarm Activated by Vehicle Theft Alarm / EDW alarm triggered",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ICH_HtVn_Actv",UINT16_C(11),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Independent Car Heater / Ventilation Active / Stand heater / ventilation active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngHd_Stat",UINT16_C(12),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Engine Hood State / Status Bonnet",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s7e,sizeof(f26s7e)/sizeof(f26s7e[0]) },
 { "DL_RLtch_Stat",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Deck Lid Rotary Latch State / Status Swivel Fall Tail Cover",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s8e,sizeof(f26s8e)/sizeof(f26s8e[0]) },
 { "ADL_LoBm_On_Rq",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Low Beam On Request by Automatic Driving Light / AFL Request: Switch on low beam",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LgtSens_Night",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Light Sensor Detect's Night Fashion / Light Sensor Actual value",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LgtSens_Flt",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Light Sensor Fault / Light Sensor Defective",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LgtSens_Tunnel",UINT16_C(19),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Light sensor \"Tunnel\" Detected / Light Sensor: Tunnel detected",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LgtSens_SNA",UINT16_C(20),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Light Sensor Information Not available / Values ​​of light sensor not available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LgtSens_Twlgt",UINT16_C(21),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Twilight State Light Sensor / Twilight Light Sensor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"Steps",NULL,0U },
 { "Tire_LHOM",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Tire in Limp-Home Operation Fashion / Mature Need",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "WprOutsdPkPosn",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Wiper Outside Park Position / Wiper Outside Parking",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "MPkBrk_Stat",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Mech. Parking Brake State / Status Mech. Parking brake",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Term54_Actv",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Terminal 54 (Brake Light) Active (FROM SAM_R) / Terminal 54 (Brake light) Feedback from SAM_R",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Hrn_On",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Horn is on / signal horn is switched on",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Trlr_Stat",UINT16_C(30),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"trailer detected / trailer operation detected",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s20e,sizeof(f26s20e)/sizeof(f26s20e[0]) },
 { "RevGr_Engg",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Reverse Gear Engaged / reverse input",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s21e,sizeof(f26s21e)/sizeof(f26s21e[0]) },
 { "LoBm_P_Rt_Flt",UINT16_C(34),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Low Beam Passenger Side Or Right (Depending On Vehicle Line) Fault / Downlight passenger side or right (depending on the series) defective",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LoBm_D_Lt_Flt",UINT16_C(35),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Low Beam Driver Side Or Left (Depending On Vehicle Line) Fault / Dimensioned Driver's side or left (depending on the series) defective",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "IrLmp_P_Rt_Flt",UINT16_C(36),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Infrared Lamp Passenger Side Or Right (Depending On Vehicle Line) Fault / IR headlight passenger side or right (depending on the series) defective",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "IrLmp_D_Lt_Flt",UINT16_C(37),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Infrared Lamp Driver Side Or Right (Depending On Vehicle Line) Fault / IR headlight driver's side or left (depending on the series) defective",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "HiBm_On",UINT16_C(38),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"High beam is on / high beam is switched on",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "LoBm_On_Rq",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Switch on Low Beam On Request / Low Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "AirTemp_Insd",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Inside Air Temperature / Inn temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,0.0,"°C",NULL,0U },
 { "StW_Cond_Stat",UINT16_C(48),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Steering Wheel Conditioning State / Status Steering Cool Climatization",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s29e,sizeof(f26s29e)/sizeof(f26s29e[0]) },
 { "Bckl_Sw_RM_Stat_SAM_R",UINT16_C(50),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BUCKLE SWITCH REAR MIDDLE STATE (by SAM_R) / Status Belt Slip Rear Center",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s30e,sizeof(f26s30e)/sizeof(f26s30e[0]) },
 { "Bckl_Sw_RR_Stat_SAM_R",UINT16_C(52),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BUCKLE SWITCH REAR RIGHT STATE (by SAM_R) / status Belt lock right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s31e,sizeof(f26s31e)/sizeof(f26s31e[0]) },
 { "Bckl_Sw_RL_Stat_SAM_R",UINT16_C(54),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BUCKLE SWITCH REAR LEFT STATE (by SAM_R) / Status Curtle Rear Left",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f26s32e,sizeof(f26s32e)/sizeof(f26s32e[0]) },
 { "AirTemp_Outsd",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Outside Air Temperature / Outdoor Air Temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,-40.0,"°C",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f27s[] = {
 { "DAC_TCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Driving Authorization Checker / FBS Embassy to TCM",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f28s[] = {
 { "DAC_ISM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Driving Authorization Checker / FBS Embassy to ISM",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f29s0e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s1e[] = {
 { UINT64_C(0), "OFF", "Stearing Wheel Conditioning Off" },
 { UINT64_C(1), "HEAT", "Stearing Wheel Heating Active" },
 { UINT64_C(2), "COOL", "Stearing Wheel Cooling Active" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s2e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s3e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s4e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s5e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s9e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s18e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "OK", "BSM Warnunit OK" },
 { UINT64_C(2), "ERROR", "BSM Warnunit Error" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s19e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "OK", "BSM Warnunit OK" },
 { UINT64_C(2), "ERROR", "BSM Warnunit Error" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s21e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s22e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "PSD", "pressed" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s23e[] = {
 { UINT64_C(0), "INACT", "Inactive" },
 { UINT64_C(1), "ACTV", "Phone Call OR SDS Active" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s24e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "PSD", "pressed" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s25e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s26e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "PSD", "pressed" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f29s27e[] = {
 { UINT64_C(0), "NPSD", "Not pressed" },
 { UINT64_C(1), "NDEF1", "Not Defined" },
 { UINT64_C(2), "PSD", "pressed" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f29s[] = {
 { "VehDrvProgSw_Psd",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Driving Program Switch Pressed / Button Driving Program",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s0e,sizeof(f29s0e)/sizeof(f29s0e[0]) },
 { "StW_Cond_Actv",UINT16_C(2),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Steering Wheel Conditioning Active / steering wheel climatization active",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s1e,sizeof(f29s1e)/sizeof(f29s1e[0]) },
 { "SuspLvlAdjSw_Psd",UINT16_C(6),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Suspension Level Adjustment Switch Pressed / Vehicle Level Buttock",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s2e,sizeof(f29s2e)/sizeof(f29s2e[0]) },
 { "PTS_Sw_Psd",UINT16_C(8),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Switch Parktronic / City Assistant Pressed / Button Parktronic / CAS Actuated",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s3e,sizeof(f29s3e)/sizeof(f29s3e[0]) },
 { "NV_Sw_Psd",UINT16_C(12),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"NightView Switch Pressed / NightView Button",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s4e,sizeof(f29s4e)/sizeof(f29s4e[0]) },
 { "ESP_Sw_Psd",UINT16_C(14),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Electronic Stability Program Switch Pressed / ESP Button",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s5e,sizeof(f29s5e)/sizeof(f29s5e[0]) },
 { "CoolIndLmp_On_Rq",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Indication Lamp On Request, Coolant Level Too Low / Cooler Spring To Switch on to low control lamp",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DL_Sw_Psd",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Deck Lid Switch Pushed / Tail Cover Button",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TX_Actv",UINT16_C(18),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Active (Data / Voice) / Transmission Active (Data / Language)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "PKAS_Sw_Psd",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Parking Assistance Switch Pressed / Parking Assist Taster",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s9e,sizeof(f29s9e)/sizeof(f29s9e[0]) },
 { "RLS_PosnLmp_Posn",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch On \"Position LAMPS\" position / LDS contact parking light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_FogLmp_R_Posn",UINT16_C(25),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch On \"Fog Lamps Rear\" Position / LDS Contact Fog Light Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_FogLmp_Ft_Posn",UINT16_C(26),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch On \"Fog Lamps Front\" Position / LDS Contact Fog Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_Red",UINT16_C(27),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch Redundancy / LDS Contact Redundancy",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_LoBm_Posn",UINT16_C(28),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch On \"Low Beam\" Position / LDS Contact Driving Light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_PkLmp_Posn_Lt",UINT16_C(29),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch On \"Parking Lamps Left\" Position / LDS Contact Parking Left",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_PkLmp_Posn_Rt",UINT16_C(30),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch on \"Parking Lamps Right\" position / LDS contact parking light right",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "RLS_AutoPosn",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch on \"Automatic\" position / LDS contact autom.speed light",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BSM_WarnUnit_Rt_Stat",UINT16_C(32),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BSM War Unit Right State / Status BSM Warning Unit right",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s18e,sizeof(f29s18e)/sizeof(f29s18e[0]) },
 { "BSM_WarnUnit_Lt_Stat",UINT16_C(34),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"BSM Warn Unit Left State / Status BSM Warning Unit Links",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s19e,sizeof(f29s19e)/sizeof(f29s19e[0]) },
 { "RLS_Init_Actv",UINT16_C(39),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Rotary Light Switch Initialization Avive / LDS Initialization",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DSR_Sw_Psd",UINT16_C(40),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Downhill Speed ​​Regulation Switch Pressed / Downhave Speed ​​Control Button",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s21e,sizeof(f29s21e)/sizeof(f29s21e[0]) },
 { "Tlm_Sw_Psd",UINT16_C(42),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Telematics Switches pressed / telematics switch operated",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s22e,sizeof(f29s22e)/sizeof(f29s22e[0]) },
 { "Phonecall_SDS_Actv",UINT16_C(44),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Phone Call OR Speech Dialogue System Active / Telephone Disconnection or Language Conditioning System",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s23e,sizeof(f29s23e)/sizeof(f29s23e[0]) },
 { "ADC_Sw_Psd",UINT16_C(46),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Active Damping Control Switch Pressed / Button Active Damping Control",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s24e,sizeof(f29s24e)/sizeof(f29s24e[0]) },
 { "SuspLvlAdjSw_Psd_CTRL_L",UINT16_C(50),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Suspension Level Adjustment Switch Pressed / Vehicle Level Buttock",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s25e,sizeof(f29s25e)/sizeof(f29s25e[0]) },
 { "ADC_Sw_Psd_CTRL_L",UINT16_C(52),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Active Damping Control Switch Pressed / Button Active Damping Control",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s26e,sizeof(f29s26e)/sizeof(f29s26e[0]) },
 { "OffRoadSw_Psd",UINT16_C(54),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Off Road Switch Pressed / Off Road Button",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f29s27e,sizeof(f29s27e)/sizeof(f29s27e[0]) },
};
static const MblinkMercedesEgs53EnumValue f30s3e[] = {
 { UINT64_C(0), "INACT", "Conditions Not Fulfilled" },
 { UINT64_C(1), "ACTV", "Conditions Fulfilled" },
 { UINT64_C(2), "FLT", "Fault" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f30s4e[] = {
 { UINT64_C(0), "INACT", "Conditions Not Fulfilled" },
 { UINT64_C(1), "ACTV", "Conditions Fulfilled" },
 { UINT64_C(2), "FLT", "Fault" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53SignalDefinition f30s[] = {
 { "AccelPdlPosn_OBD",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Accelerator Pedal Position / Pedal Value",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.3921568691730499,0.0,"%",NULL,0U },
 { "EngLoad_OBD",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine Load / Motorload",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.3921568691730499,0.0,"%",NULL,0U },
 { "AirTemp_Outsd_Sens_Flt",UINT16_C(23),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Outside Air Temperature Sensor Fault / Error Outdoor Air Temperature Sensor",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "OBD_IgnCycCntCond_Stat",UINT16_C(27),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Conditions for Ignition Cycle Counter (RBM) State / Status Conditions for Ignition Cycle Counter (RBM)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f30s3e,sizeof(f30s3e)/sizeof(f30s3e[0]) },
 { "OBD_GnrlDenCond_Stat",UINT16_C(29),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Conditions for General Denominator (RBM) State / Status Conditions for General Denominator (RBM)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f30s4e,sizeof(f30s4e)/sizeof(f30s4e[0]) },
 { "OBD_WarmupCond_Actv",UINT16_C(31),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Conditions for warm-up fulfilled / conditions for warm-up fulfilled",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f31s[] = {
 { "APPL_SG_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"External Application to ECU / External application to control unit",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f32s[] = {
 { "APPL_SG_ISM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"External Application to ECU / External application to control unit",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f33s[] = {
 { "APPL_SG_TCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"External Application to ECU / External application to control unit",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f34s[] = {
 { "D_RQ_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Request / Diagnostic Request",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f35s[] = {
 { "D_RQ_ISM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Request / Diagnostic Request",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f36s[] = {
 { "D_RQ_SSP",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Request / Diagnostic Request",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f37s[] = {
 { "D_RQ_TCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Request / Diagnostic Request",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f38s[] = {
 { "D_RQ_TSLM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Request / Diagnostic Request",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f39s[] = {
 { "DG_RQ_GLOBAL",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Functional Diagnostic Request (KWP2000) / Functional Diagnostic Request (KWP2000)",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f40s[] = {
 { "DG_RQ_GLOBAL_UDS",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Functional Diagnostic Request (UDS) / Functional Diagnostic Request (UDS)",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f41s[] = {
 { "DG_RQ_OBD",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Functional Diagnostic Request for OBDII / Functional Diagnostic Request Obiodii",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f42s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f42s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f42s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f42s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(11), "CHASSIS", "Wakeup by Chassis Can" },
 { UINT64_C(13), "POWERTRAIN", "Wakeup by Powertrain Can" },
 { UINT64_C(130), "TERM_15", "Wakeup by Discrete Terminal 15 Signal" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f42s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f42s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f42s0e,sizeof(f42s0e)/sizeof(f42s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f42s4e,sizeof(f42s4e)/sizeof(f42s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f42s5e,sizeof(f42s5e)/sizeof(f42s5e[0]) },
 { "WakeupRsn_ECM",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f42s6e,sizeof(f42s6e)/sizeof(f42s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f42s8e,sizeof(f42s8e)/sizeof(f42s8e[0]) },
};
static const MblinkMercedesEgs53EnumValue f43s2e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "ENGG", "engaged" },
 { UINT64_C(2), "DISENGG", "disengaged" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
 { UINT64_C(4), "SLIP", "SLIPPING" },
 { UINT64_C(5), "SLIP_ENGG", "Between Slipping and Engaged" },
 { UINT64_C(6), "DISENGG_SLIP", "Between Disgaged and Slipping" },
 { UINT64_C(7), "NDEF7", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f43s8e[] = {
 { UINT64_C(0), "P", "TRANMMISSION SELECTOR LEVER IN PARKING POSITION" },
 { UINT64_C(1), "R", "TRANMMISSION SELECTOR LEVER IN REVERSE POSITION" },
 { UINT64_C(2), "N", "TRANMMISSION SELECTOR LEVER IN NEUTRAL POSITION" },
 { UINT64_C(4), "D", "TRANMMISSION SELECTOR LEVER IN NORMAL DRIVING POSITION" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f43s10e[] = {
 { UINT64_C(0), "SPORT", "Sports (standard)" },
 { UINT64_C(1), "COMFORT", "Comfort" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
 { UINT64_C(4), "NDEF4", "Not Defined" },
 { UINT64_C(5), "NDEF5", "Not Defined" },
 { UINT64_C(6), "OFFROAD", "off-road" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f43s[] = {
 { "TxOilTemp",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Transmission Oil Temperature / Gear Oil Temperature",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,-50.0,"°C",NULL,0U },
 { "TCC_NoLoad",UINT16_C(8),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Torque Converter Lockup Clutch No Load / Converter Bridging Clutch Load",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "Clutch_Stat",UINT16_C(9),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"State (Torque Converter Lockup) Clutch / Status (Converter Bridging) Clutch",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f43s2e,sizeof(f43s2e)/sizeof(f43s2e[0]) },
 { "TCM_LHOM",UINT16_C(12),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Control in Limp-Home Operation Fashion / transmission control in emergency",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "BasShftProg_Ok",UINT16_C(13),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Basic Shifting Program OK / Basic Switching Program O.K.",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "DrvRst_Hi",UINT16_C(14),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Drive Resistance High / driving resistance high",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxTemp_Excess",UINT16_C(15),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Excessive Transmission Temperature / Overtemperature Transmission",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxOffRd_Actv",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission off-road active / terrain active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TSL_Posn_TCM",UINT16_C(17),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Position / Transmission Logging",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f43s8e,sizeof(f43s8e)/sizeof(f43s8e[0]) },
 { "TxDrvProgMan_Actv",UINT16_C(20),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Driving Program \"Manual\" Active / Driving Program \"Manual\" active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "VehDrvProg_TCM_V2",UINT16_C(21),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Actual Vehicle Driving Program (ie Sent During Txdrvprogman_Activ = 1) / Driving Program Transmission (is also sent to TXDRVPROGMAN_ACTV = 1)",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f43s10e,sizeof(f43s10e)/sizeof(f43s10e[0]) },
 { "StBrk_Rq_TCM",UINT16_C(24),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Brake During Start Request / Create brake when switched on",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxDrvTrqAbsVal",UINT16_C(32),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Absolute Value of the Transmission Overall Drive Shaft Torque For Driveaway / Amount of Total Point Torque In The Startup Area",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"Nm",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f44s5e[] = {
 { UINT64_C(0), "WAIT", "Error Test Not Yet Finished" },
 { UINT64_C(1), "OK", "Error Test Finished, result is ok" },
 { UINT64_C(2), "ERROR", "Error Detected, Record Actual Data" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53SignalDefinition f44s[] = {
 { "CurrDtyCyc_Rq",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Requested Current Duty Cycle / Soll Current (duty cycle)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"%",NULL,0U },
 { "TxTurbineRPM",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Actual Transmission Turbine RPM / Current turbine speed",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "MIL_On_Rq_TCM",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Malfunction Indicator Lamp on Request (OBD II) / Diagnosis Control Lamp (OBD II)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxSlpRPM_Dsr",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Desired Transmission Slip RPM / Slip Speed should",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "TCM_Data",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Calid / CVN Data Byte / Calid / CVN DataByte",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "TCM_ErrChk_Stat",UINT16_C(56),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Error Check State / Status Error Check",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f44s5e,sizeof(f44s5e)/sizeof(f44s5e[0]) },
 { "TCM_CALID_CVN_Actv",UINT16_C(58),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"CALID / CVN Transmission Active / Calid / CVN transmission active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TCM_CALID_CVN_ErrNum",UINT16_C(59),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Error Number or Counter for Calid / CVN Transmission / Error Number or Counter for Calid / CVN Transfer",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f45s3e[] = {
 { UINT64_C(0), "MFC", "Intervention Fashion Minimum Fuel Consumption" },
 { UINT64_C(1), "FAST", "Intervention Fashion Fastest" },
 { UINT64_C(2), "RATE_DEC", "Intervention Fashion Fastest With Rate Action - Torque Decreasing" },
 { UINT64_C(3), "RATE_INC", "Intervention Fashion Fastest With Rate Action - Torque Increasing" },
};
static const MblinkMercedesEgs53EnumValue f45s4e[] = {
 { UINT64_C(0), "IDLE", "Downshift Without Engine Torque Increase" },
 { UINT64_C(1), "SINGLE", "Single Downshift With Engine Torque Increase" },
 { UINT64_C(2), "MULTI", "Multiple Downshift with Engine Torque Increase" },
 { UINT64_C(3), "INTRVNTN_MON", "Intervention Monitoring by TCM" },
};
static const MblinkMercedesEgs53SignalDefinition f45s[] = {
 { "EngTrqMin_Rq_TCM",UINT16_C(0),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Torque Request Minimum / Motor Moment Request Min",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrqMax_Rq_TCM",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Torque Request Maximum / Engine Motor Toment Request Max",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngTrq_Rq_TCM",UINT16_C(3),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Engine Torque Request / Ford. Engine torque",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "IntrvntnMd_TCM",UINT16_C(16),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Intervention fashion / intervention mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f45s3e,sizeof(f45s3e)/sizeof(f45s3e[0]) },
 { "TxDnShiftMd",UINT16_C(18),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Transmission Downshift Mode / Rewish Mode Transmission",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f45s4e,sizeof(f45s4e)/sizeof(f45s4e[0]) },
 { "EngRPM_SyncTm_Rq_TCM",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Engine RPM Synchronization Time Request / Synchronization Time for Target Speed ​​Engine",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,0.0,"s",NULL,0U },
 { "SSA_Enbl_Rq_TCM",UINT16_C(32),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"ENABLE STOP / START AUTOMATIC REQUEST / ASS ENABLE",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngRPM_Rq_TCM",UINT16_C(34),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Engine RPM Request / Target Speed ​​Engine",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "MC_ENG_RQ1_TCM",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "EngSt_Enbl_Rq_TCM",UINT16_C(52),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Enable Engine Start Request / Using Release Transmission",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EngEmgOff_Rq",UINT16_C(53),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Engine Emergency Off Request / Engine Emergency Off",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "JmpSt_Actv",UINT16_C(54),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"JUMP START ACTIVE / BARMSTART",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "CRC_ENG_RQ1_TCM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f46s0e[] = {
 { UINT64_C(0), "N", "Target Gear \"N\"" },
 { UINT64_C(1), "D1", "Target Gear \"1\"" },
 { UINT64_C(2), "D2", "Target Gear \"2\"" },
 { UINT64_C(3), "D3", "Target Gear \"3\"" },
 { UINT64_C(4), "D4", "Target Gear \"4\"" },
 { UINT64_C(5), "D5", "Target Gear \"5\"" },
 { UINT64_C(6), "D6", "Target Gear \"6\"" },
 { UINT64_C(7), "D7", "Target Gear \"7\"" },
 { UINT64_C(8), "D_CVT", "Target Gear \"Continuously Forward\"" },
 { UINT64_C(9), "R_CVT", "Target Gear \"Continuously Backward\"" },
 { UINT64_C(10), "R_3", "Target Gear \"R3\"" },
 { UINT64_C(11), "R", "Target Gear \"R\"" },
 { UINT64_C(12), "R_2", "Target Gear \"R2\"" },
 { UINT64_C(13), "P", "Target Gear \"P\"" },
 { UINT64_C(14), "ABORT", "Gear Shift Abortion" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f46s1e[] = {
 { UINT64_C(0), "N", "Actual Gear \"N\"" },
 { UINT64_C(1), "D1", "Actual Gear \"1\"" },
 { UINT64_C(2), "D2", "Actual Gear \"2\"" },
 { UINT64_C(3), "D3", "Actual Gear \"3\"" },
 { UINT64_C(4), "D4", "Actual Gear \"4\"" },
 { UINT64_C(5), "D5", "Actual Gear \"5\"" },
 { UINT64_C(6), "D6", "Actual Gear \"6\"" },
 { UINT64_C(7), "D7", "Actual Gear \"7\"" },
 { UINT64_C(8), "D_CVT", "Actual Gear \"Continuously Forward\"" },
 { UINT64_C(9), "R_CVT", "Actual Gear \"Continuously Backward\"" },
 { UINT64_C(10), "R_3", "Actual Gear \"R3\"" },
 { UINT64_C(11), "R", "Actual Gear \"R\"" },
 { UINT64_C(12), "R_2", "Actual Gear \"R2\"" },
 { UINT64_C(13), "P", "Actual Gear \"P\"" },
 { UINT64_C(14), "PWRFREE", "Power Free" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f46s5e[] = {
 { UINT64_C(0), "REAR", "Rear Wheel Driven" },
 { UINT64_C(1), "FRONT", "Front Wheel Driven" },
 { UINT64_C(2), "ALL2", "All Wheel Driven" },
 { UINT64_C(3), "ALL3", "All Wheel Driven" },
};
static const MblinkMercedesEgs53EnumValue f46s6e[] = {
 { UINT64_C(0), "SAT", "Stepped Automatic Transmission" },
 { UINT64_C(1), "CVT", "Continously Variable Automatic Transmission" },
 { UINT64_C(2), "AMT", "Automated Manual Transmission" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f46s7e[] = {
 { UINT64_C(0), "LARGE", "NAG, Large Gear" },
 { UINT64_C(1), "SMALL", "NAG, Small Gear" },
 { UINT64_C(2), "LARGE2", "NAG2, Large Gear" },
 { UINT64_C(3), "SMALL2", "NAG2, Small Gear" },
};
static const MblinkMercedesEgs53EnumValue f46s8e[] = {
 { UINT64_C(0), "NDEF0", "Not Defined" },
 { UINT64_C(1), "SBW", "Shift by Wire (ISM)" },
 { UINT64_C(2), "MS", "Mechanical Shifting (EWM)" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f46s[] = {
 { "Gr_Target",UINT16_C(0),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Target Gear / Target Gear",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f46s0e,sizeof(f46s0e)/sizeof(f46s0e[0]) },
 { "Gr",UINT16_C(4),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Actual Gear / Actual Gang",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f46s1e,sizeof(f46s1e)/sizeof(f46s1e[0]) },
 { "TxRatio",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Transmission Ratio (CVT) / Translation Translation (CVT)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.019999999552965164,0.0,"",NULL,0U },
 { "EngWhlTrqRatio_TCM",UINT16_C(18),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"Crackish Torque to Wheel Torque ratio / factor crankshaft torque to wheel torque",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.009999999776482582,0.0,"",NULL,0U },
 { "TxTrqLoss",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Transmission Crankly Torque Loss / Loss Torque",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,0.0,"Nm",NULL,0U },
 { "VehDrvStyle",UINT16_C(40),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Vehicle Drive Style / Drive Variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f46s5e,sizeof(f46s5e)/sizeof(f46s5e[0]) },
 { "TxStyle",UINT16_C(42),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Variant / Gear Variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f46s6e,sizeof(f46s6e)/sizeof(f46s6e[0]) },
 { "TxMechStyle",UINT16_C(44),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Mechanics Style / Gear Mechanic Variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f46s7e,sizeof(f46s7e)/sizeof(f46s7e[0]) },
 { "TxShiftStyle",UINT16_C(46),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Shifting Style / Transmission Circuit Variant",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f46s8e,sizeof(f46s8e)/sizeof(f46s8e[0]) },
 { "MC_ENG_RQ2_TCM",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_ENG_RQ2_TCM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f47s0e[] = {
 { UINT64_C(0), "OFF", "OFF" },
 { UINT64_C(1), "ENBL", "Enabled" },
 { UINT64_C(2), "ACTV", "Active" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f47s[] = {
 { "DrvAccelMax_Stat",UINT16_C(1),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Driveaway with Maximum Acceleration State / Status Attachment With Maximum Acceleration",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f47s0e,sizeof(f47s0e)/sizeof(f47s0e[0]) },
 { "WetDrvClutchTrq",UINT16_C(3),UINT16_C(13),false,UINT64_C(0),UINT8_C(0),"Actual Wet Driveaway Clutch Torque (0h = passive) / current torque wet starting coupling (0h = passive)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.25,-500.0,"Nm",NULL,0U },
 { "MC_ENG_RQ3_TCM",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_ENG_RQ3_TCM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f48s0e[] = {
 { UINT64_C(0), "VGS_NAG2", "VGS NAG2" },
 { UINT64_C(1), "EGS52", "EGS52" },
 { UINT64_C(2), "NDEF2", "Not Defined" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f48s2e[] = {
 { UINT64_C(5), "D", "Actual selector valve position \"D\"" },
 { UINT64_C(6), "N", "Actual selector valve position \"N\"" },
 { UINT64_C(7), "R", "Actual selector valve position \"R\"" },
 { UINT64_C(8), "P", "Actual selector valve position \"P\"" },
 { UINT64_C(11), "N_ZW_D", "Actual selector valve position \"N-D\"" },
 { UINT64_C(12), "R_ZW_N", "Actual selector valve position \"R-N\"" },
 { UINT64_C(13), "P_ZW_R", "Actual selector valve position \"P-R\"" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f48s3e[] = {
 { UINT64_C(0), "IDLE", "transmission selector lever passive request" },
 { UINT64_C(5), "D", "transmission selector lever request \"D\"" },
 { UINT64_C(6), "N", "transmission selector lever request \"N\"" },
 { UINT64_C(7), "R", "transmission selector lever request \"R\"" },
 { UINT64_C(8), "P", "transmission selector lever request \"P\"" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f48s[] = {
 { "SBW_MsgTxmtId",UINT16_C(0),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Shift by wire message transmitter identification / shift-by-wire transmitter identification",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f48s0e,sizeof(f48s0e)/sizeof(f48s0e[0]) },
 { "StartLkSw",UINT16_C(7),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Starter lockout switch (only EGS52) / Inhibitor contact (only EGS52)",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxSelVlvPosn",UINT16_C(8),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Actual position transmission selector valve / Actual position Wählbereichsschieber in gear",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f48s2e,sizeof(f48s2e)/sizeof(f48s2e[0]) },
 { "TSL_Posn_Rq",UINT16_C(12),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"transmission selector lever position request / requirement gear selector lever position",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f48s3e,sizeof(f48s3e)/sizeof(f48s3e[0]) },
 { "TxSelSensPosn",UINT16_C(16),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Transmission selector sensor position / value Wählbereichssensor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.4000000059604645,0.0,"%",NULL,0U },
 { "MC_SBW_RS_TCM",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_SBW_RS_TCM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f49s[] = {
 { "TCM_DAC",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Driving Authorization Checker / FBS Embassy to Ice",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f50s0e[] = {
 { UINT64_C(1), "M1", "Driving position \"M1\"" },
 { UINT64_C(2), "M2", "Driving position \"M2\"" },
 { UINT64_C(3), "M3", "Driving position \"M3\"" },
 { UINT64_C(4), "M4", "driving position \"M4\"" },
 { UINT64_C(5), "M5", "Driving position \"M5\"" },
 { UINT64_C(6), "M6", "Driving position \"M6\"" },
 { UINT64_C(7), "M7", "Driving position \"M7\"" },
 { UINT64_C(32), "BLANK", "blank (\"\")" },
 { UINT64_C(49), "D1", "driving position \"D1\"" },
 { UINT64_C(50), "D2", "Driving position \"D2\"" },
 { UINT64_C(51), "D3", "Driving position \"D3\"" },
 { UINT64_C(52), "D4", "Driving position \"D4\"" },
 { UINT64_C(53), "D5", "Driving position \"D5\"" },
 { UINT64_C(54), "D6", "Driving position \"D6\"" },
 { UINT64_C(55), "D7", "Driving position \"D7\"" },
 { UINT64_C(65), "A", "Driving position \"A\"" },
 { UINT64_C(68), "D", "Driving position \"D\"" },
 { UINT64_C(70), "F", "Fault Label \"F\"" },
 { UINT64_C(78), "N", "Driving position \"N\"" },
 { UINT64_C(80), "P", "driving position \"P\"" },
 { UINT64_C(82), "R", "Driving position \"R\"" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f50s1e[] = {
 { UINT64_C(32), "BLANK", "Driving Program \"\" (blank)" },
 { UINT64_C(65), "A", "Driving Program \"A\"" },
 { UINT64_C(67), "C", "Driving Program \"C\"" },
 { UINT64_C(70), "F", "Fault Label \"F\"" },
 { UINT64_C(77), "M", "Driving Program \"M\"" },
 { UINT64_C(83), "S", "Driving Program \"S\"" },
 { UINT64_C(87), "W", "Driving Program \"W\"" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f50s3e[] = {
 { UINT64_C(0), "IDLE", "No Recommendation" },
 { UINT64_C(1), "UP", "Upshift Recommendation" },
 { UINT64_C(2), "DOWN", "Downshift Recommendation" },
 { UINT64_C(3), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f50s4e[] = {
 { UINT64_C(0), "IDLE", "No Message" },
 { UINT64_C(1), "M1", "Message 1" },
 { UINT64_C(2), "M2", "Message 2" },
 { UINT64_C(3), "M3", "Message 3" },
 { UINT64_C(4), "M4", "Message 4" },
 { UINT64_C(5), "M5", "Message 5" },
 { UINT64_C(6), "M6", "Message 6" },
 { UINT64_C(7), "NDEF7", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f50s5e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f50s6e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f50s7e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f50s8e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f50s9e[] = {
 { UINT64_C(32), "BLANK", "blank (\"\")" },
 { UINT64_C(49), "G1", "Gear \"1\"" },
 { UINT64_C(50), "G2", "Gear \"2\"" },
 { UINT64_C(51), "G3", "Gear \"3\"" },
 { UINT64_C(52), "G4", "Gear \"4\"" },
 { UINT64_C(53), "G5", "Gear \"5\"" },
 { UINT64_C(54), "G6", "Gear \"6\"" },
 { UINT64_C(55), "G7", "Gear \"7\"" },
 { UINT64_C(70), "F", "Fault Label \"F\"" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f50s10e[] = {
 { UINT64_C(0), "IDLE", "Race Start Fashion Off OR Passive" },
 { UINT64_C(1), "AVL", "Race Start Fashion Available" },
 { UINT64_C(2), "ACTV", "Race Start Mode Active" },
 { UINT64_C(3), "ON", "Race Start Fashion On" },
 { UINT64_C(4), "CANCEL", "Race Start Fashion Cancel" },
 { UINT64_C(5), "NPOSBL", "Race Start Fashion Not Possible" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f50s[] = {
 { "TxDrvPosn_Disp_Rq_TCM",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Transmission Driving Position Display Request / Request Display Gearbox",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s0e,sizeof(f50s0e)/sizeof(f50s0e[0]) },
 { "TxDrvProg_Disp_Rq_TCM",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Transmission Driving Program Display Request / Request Display Gearbox Program",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s1e,sizeof(f50s1e)/sizeof(f50s1e[0]) },
 { "SBW_Beep_Rq_TCM",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Shift by Wire Beep Request / Request Shift by Wire Warnington",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TxShiftRcmmnd_Disp_Rq_TCM",UINT16_C(17),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Transmission Shift Recommendation Display Request / Request Display Transmission Description",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s3e,sizeof(f50s3e)/sizeof(f50s3e[0]) },
 { "SBW_Msg_Disp_Rq_TCM",UINT16_C(21),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Shift by Wire Message Display Request / Request Shift by Wire Show message",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s4e,sizeof(f50s4e)/sizeof(f50s4e[0]) },
 { "TSL_MtnLk2_Disp_Rq_TCM",UINT16_C(24),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 2 Display Request / Request Transmission Lock Lock 2 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s5e,sizeof(f50s5e)/sizeof(f50s5e[0]) },
 { "TSL_MtnLk1_Disp_Rq_TCM",UINT16_C(28),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 1 Display Request / Request Transmission Lock Lock 1 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s6e,sizeof(f50s6e)/sizeof(f50s6e[0]) },
 { "TSL_MtnLk4_Disp_Rq_TCM",UINT16_C(32),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 4 Display Request / Request Transmission Lock Lock 4 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s7e,sizeof(f50s7e)/sizeof(f50s7e[0]) },
 { "TSL_MtnLk3_Disp_Rq_TCM",UINT16_C(36),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 3 Display Request / Request Transmission Lock Lock 3 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s8e,sizeof(f50s8e)/sizeof(f50s8e[0]) },
 { "Gr_Target_Disp_Rq",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Target Gear Display Request / Request Display Destination",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s9e,sizeof(f50s9e)/sizeof(f50s9e[0]) },
 { "RaceStMd_Disp_Rq_AMG",UINT16_C(48),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Race Start Mode Display Request / Display Race Start Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f50s10e,sizeof(f50s10e)/sizeof(f50s10e[0]) },
};
static const MblinkMercedesEgs53SignalDefinition f51s[] = {
 { "D_RS_TCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Response / Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f52s[] = {
 { "SD_RS_TCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"SystemDiagnostic Response / System Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f53s[] = {
 { "SG_APPL_TCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"ECU TO External Application / Controller to External Application",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f54s[] = {
 { "TCM_HSA",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f55s[] = {
 { "TCM_HSB",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f56s[] = {
 { "TCM_HSC",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f57s[] = {
 { "TCM_HSD",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f58s[] = {
 { "TCM_HSE",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f59s[] = {
 { "TCM_HSF",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f60s[] = {
 { "TCM_HSG",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f61s[] = {
 { "TCM_HSH",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f62s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f62s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f62s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f62s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f62s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f62s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f62s0e,sizeof(f62s0e)/sizeof(f62s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f62s4e,sizeof(f62s4e)/sizeof(f62s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f62s5e,sizeof(f62s5e)/sizeof(f62s5e[0]) },
 { "WakeupRsn_TCM",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f62s6e,sizeof(f62s6e)/sizeof(f62s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f62s8e,sizeof(f62s8e)/sizeof(f62s8e[0]) },
};
static const MblinkMercedesEgs53EnumValue f63s4e[] = {
 { UINT64_C(0), "OK", "Normal Operation Mode" },
 { UINT64_C(1), "SHRT_GND", "Invalid Fuel Pressure Sensor Signal, Shortcut to Ground" },
 { UINT64_C(2), "SHRT_BAT", "Invalid Fuel Pressure Sensor Signal, Shortcut to Battery Voltage" },
 { UINT64_C(3), "IMPLSBL", "Implausible Fuel Pressure Sensor Signal" },
 { UINT64_C(4), "PWR_FLT", "Fuel Pressure Sensor Power Supply Fault" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f63s5e[] = {
 { UINT64_C(0), "OK", "Normal Operation Mode" },
 { UINT64_C(1), "SHRT", "Fuel Pump Shortcut" },
 { UINT64_C(2), "OPN", "Fuel Pump Open Circuit" },
 { UINT64_C(3), "LHOM", "LIMP-Home Operation Fashion" },
};
static const MblinkMercedesEgs53EnumValue f63s7e[] = {
 { UINT64_C(0), "OK", "normal surgery" },
 { UINT64_C(1), "WARN_TEMP", "Temperature High Warning" },
 { UINT64_C(2), "ALM_TEMP", "Temperature High Alarm" },
 { UINT64_C(3), "START_FAIL", "Fuel Pump Start Up Failed" },
 { UINT64_C(4), "FLT_GNRL", "General Fault" },
 { UINT64_C(5), "NDEF5", "Not Defined" },
 { UINT64_C(6), "NDEF6", "Not Defined" },
 { UINT64_C(7), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f63s[] = {
 { "FuelPress",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Fuel Pressure / Current fuel pressure",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.05000000074505806,0.0,"bar",NULL,0U },
 { "FuelPmpDtyCyc",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Fuel Pump Duty Cycle / Current duty cycle fuel pump",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"%",NULL,0U },
 { "FuelPress_Rq_NA",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fuel Pressure Request Not available / requirement Fuel pressure not available",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FuelPress_IncHydrRst",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Fuel Pressure: InCreased Hydraulic Resistance / Fuel pressure: Increased hydraulic resistance",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "FuelPressSens_Stat_V2",UINT16_C(19),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Fuel Pressure Sensor State / Status Fuel pressure sensor",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f63s4e,sizeof(f63s4e)/sizeof(f63s4e[0]) },
 { "FuelPmp_Stat_V2",UINT16_C(22),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Fuel Pump State / Status Fuel Pump",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f63s5e,sizeof(f63s5e)/sizeof(f63s5e[0]) },
 { "FuelPmp1_InDtyCyc",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Fuel Pump # 1 Input Duty Cycle / News Input Dewy Rating Fuel Pump 1",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"%",NULL,0U },
 { "FSCM_Stat_AMG",UINT16_C(45),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Fuel System Control Module State / Status FSCM",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f63s7e,sizeof(f63s7e)/sizeof(f63s7e[0]) },
 { "ESIM_Cntct_Stat_EngShutOff",UINT16_C(48),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Evaporative System Integrity Monitor Contact State at Engine Shut-Off / Contact Rate Integrity Monitoring Evaporation System for Engine Stop",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESIM_Cntct_Stat",UINT16_C(49),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Evaporative System Integrity Monitor Contact State / Contact State Integrity Monitoring Evaporation System",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "ESIM_Cntct_ClsTm",UINT16_C(54),UINT16_C(10),false,UINT64_C(0),UINT8_C(0),"Evaporative System Integrity Monitor Contact Closing Time / Conclusion of the Contact Integrity Monitoring Evaporation System",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"min",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f64s[] = {
 { "D_RS_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Response / Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f65s[] = {
 { "SD_RS_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"SystemDiagnostic Response / System Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f66s[] = {
 { "SG_APPL_FSCM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"ECU TO External Application / Controller to External Application",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f67s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f67s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f67s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f67s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(130), "TERM_15", "Wakeup by Discrete Terminal 15 Signal" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f67s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f67s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f67s0e,sizeof(f67s0e)/sizeof(f67s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f67s4e,sizeof(f67s4e)/sizeof(f67s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f67s5e,sizeof(f67s5e)/sizeof(f67s5e[0]) },
 { "WakeupRsn_FSCM",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f67s6e,sizeof(f67s6e)/sizeof(f67s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f67s8e,sizeof(f67s8e)/sizeof(f67s8e[0]) },
};
static const MblinkMercedesEgs53EnumValue f68s2e[] = {
 { UINT64_C(5), "D", "Transmission Selector Lever in position \"D\"" },
 { UINT64_C(6), "N", "Transmission Selector Lever in position \"N\"" },
 { UINT64_C(7), "R", "Transmission Selector Lever in position \"R\"" },
 { UINT64_C(8), "P", "Transmission Selector Lever in position \"P\"" },
 { UINT64_C(9), "PLUS", "Transmission Selector Lever in position \"+\"" },
 { UINT64_C(10), "MINUS", "Transmission Selector Lever in position \"-\"" },
 { UINT64_C(11), "N_ZW_D", "Transmission Selector Lever in Intermediate Position \"N-D\"" },
 { UINT64_C(12), "R_ZW_N", "Transmission Selector Lever in Intermediate Position \"R-N\"" },
 { UINT64_C(13), "P_ZW_R", "Transmission Selector Lever in Intermediate Position \"P-R\"" },
 { UINT64_C(15), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f68s[] = {
 { "TxDrvProgSw_Psd_V3",UINT16_C(1),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Driving Program Switch State / Status Driving program",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TSL_MtnLk_Actv",UINT16_C(3),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock Active / Gear Select Lock Active",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "TSL_Posn_ISM",UINT16_C(4),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Position / Transmission Logging",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f68s2e,sizeof(f68s2e)/sizeof(f68s2e[0]) },
 { "MC_SBW_RS_ISM",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_SBW_RS_ISM",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f69s[] = {
 { "D_RS_TSLM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Response / Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f70s[] = {
 { "SD_RS_TSLM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"SystemDiagnostic Response / System Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f71s[] = {
 { "SG_APPL_TSLM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"ECU TO External Application / Controller to External Application",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f72s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f72s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f72s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f72s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f72s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f72s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f72s0e,sizeof(f72s0e)/sizeof(f72s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f72s4e,sizeof(f72s4e)/sizeof(f72s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f72s5e,sizeof(f72s5e)/sizeof(f72s5e[0]) },
 { "WakeupRsn_TSLM",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f72s6e,sizeof(f72s6e)/sizeof(f72s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f72s8e,sizeof(f72s8e)/sizeof(f72s8e[0]) },
};
static const MblinkMercedesEgs53EnumValue f73s2e[] = {
 { UINT64_C(0), "IDLE", "No Reaction" },
 { UINT64_C(1), "AUTOEPSOFF_ENBL", "carpsof_enable" },
 { UINT64_C(2), "LHOM", "LIMP Home" },
 { UINT64_C(3), "TRQ_RED", "Torque Reduce" },
 { UINT64_C(4), "SPD_RED", "Speed Reduce" },
 { UINT64_C(5), "ASC_ACTV", "ASC Active" },
 { UINT64_C(6), "TPC_ACTV", "TPC Active" },
 { UINT64_C(7), "PREVENT", "Prevent Operation" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f73s3e[] = {
 { UINT64_C(0), "WAIT", "Error Test Not Yet Finished" },
 { UINT64_C(1), "OK", "Error Test Finished, result is ok" },
 { UINT64_C(2), "ERROR", "Error Detected, Record Actual Data" },
 { UINT64_C(3), "NDEF3", "Not Defined" },
};
static const MblinkMercedesEgs53SignalDefinition f73s[] = {
 { "SG_OutDC_Curr",UINT16_C(0),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Starter / Generator Output DC Current / DC Current",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.10000000149011612,-3276.60009765625,"A",NULL,0U },
 { "SG_OutDC_Volt",UINT16_C(16),UINT16_C(16),false,UINT64_C(0),UINT8_C(0),"Starter / Generator Output DC Voltage / DC Voltage",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.10000000149011612,0.0,"V",NULL,0U },
 { "EM1_Diag_Stat",UINT16_C(48),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Electric Machine # 1 Diagnostics State / Diagnostic Status E-Machine 1",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f73s2e,sizeof(f73s2e)/sizeof(f73s2e[0]) },
 { "SG_ErrChk_Stat",UINT16_C(56),UINT16_C(2),false,UINT64_C(0),UINT8_C(0),"Error Check State / Status Error Check",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f73s3e,sizeof(f73s3e)/sizeof(f73s3e[0]) },
 { "EM1_EnhCool_Rq",UINT16_C(58),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Electric Machine # 1 Enhanced Cooling Request / Request Advanced Cooling of the E-Machine 1",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "EM1_ErrNum",UINT16_C(59),UINT16_C(5),false,UINT64_C(0),UINT8_C(0),"Error Number / Error Number",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f74s0e[] = {
 { UINT64_C(1), "M1", "Driving position \"M1\"" },
 { UINT64_C(2), "M2", "Driving position \"M2\"" },
 { UINT64_C(3), "M3", "Driving position \"M3\"" },
 { UINT64_C(4), "M4", "driving position \"M4\"" },
 { UINT64_C(5), "M5", "Driving position \"M5\"" },
 { UINT64_C(6), "M6", "Driving position \"M6\"" },
 { UINT64_C(7), "M7", "Driving position \"M7\"" },
 { UINT64_C(32), "BLANK", "blank (\"\")" },
 { UINT64_C(49), "D1", "driving position \"D1\"" },
 { UINT64_C(50), "D2", "Driving position \"D2\"" },
 { UINT64_C(51), "D3", "Driving position \"D3\"" },
 { UINT64_C(52), "D4", "Driving position \"D4\"" },
 { UINT64_C(53), "D5", "Driving position \"D5\"" },
 { UINT64_C(54), "D6", "Driving position \"D6\"" },
 { UINT64_C(55), "D7", "Driving position \"D7\"" },
 { UINT64_C(65), "A", "Driving position \"A\"" },
 { UINT64_C(68), "D", "Driving position \"D\"" },
 { UINT64_C(70), "F", "Fault Label \"F\"" },
 { UINT64_C(78), "N", "Driving position \"N\"" },
 { UINT64_C(80), "P", "driving position \"P\"" },
 { UINT64_C(82), "R", "Driving position \"R\"" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f74s2e[] = {
 { UINT64_C(0), "IDLE", "No Message" },
 { UINT64_C(1), "M1", "Message 1" },
 { UINT64_C(2), "M2", "Message 2" },
 { UINT64_C(3), "M3", "Message 3" },
 { UINT64_C(4), "M4", "Message 4" },
 { UINT64_C(5), "M5", "Message 5" },
 { UINT64_C(6), "M6", "Message 6" },
 { UINT64_C(7), "NDEF7", "Not Defined" },
};
static const MblinkMercedesEgs53EnumValue f74s3e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f74s4e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f74s5e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53EnumValue f74s6e[] = {
 { UINT64_C(0), "IDLE", "No Lock icon display" },
 { UINT64_C(1), "ARR_UP", "Lock icon arrow up" },
 { UINT64_C(2), "ARR_DN", "Lock Symbol Arrow Down" },
 { UINT64_C(3), "ARR_LT", "Lock Symbol Arrow Left" },
 { UINT64_C(4), "ARR_UP_DN", "Lock Symbol Arrow Up and Down" },
 { UINT64_C(9), "ARR_UP_HLGT", "Lock icon arrow up highlighted" },
 { UINT64_C(10), "ARR_DN_HLGT", "Lock Symbol Arrow Down Highlighted" },
 { UINT64_C(11), "ARR_LT_HLGT", "Lock Symbol Arrow Left Highlighted" },
 { UINT64_C(12), "ARR_UP_DN_HLGT", "Lock Symbol Arrow Up and Down Highlighted" },
};
static const MblinkMercedesEgs53SignalDefinition f74s[] = {
 { "TxDrvPosn_Disp_Rq_ISM",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Transmission Driving Position Display Request / Request Display Gearbox",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f74s0e,sizeof(f74s0e)/sizeof(f74s0e[0]) },
 { "SBW_Beep_Rq_ISM",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Shift by Wire Beep Request / Request Shift by Wire Warnington",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "SBW_Msg_Disp_Rq_ISM",UINT16_C(21),UINT16_C(3),false,UINT64_C(0),UINT8_C(0),"Shift by Wire Message Display Request / Request Shift by Wire Show message",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f74s2e,sizeof(f74s2e)/sizeof(f74s2e[0]) },
 { "TSL_MtnLk2_Disp_Rq_ISM",UINT16_C(24),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 2 Display Request / Request Transmission Lock Lock 2 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f74s3e,sizeof(f74s3e)/sizeof(f74s3e[0]) },
 { "TSL_MtnLk1_Disp_Rq_ISM",UINT16_C(28),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 1 Display Request / Request Transmission Lock Lock 1 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f74s4e,sizeof(f74s4e)/sizeof(f74s4e[0]) },
 { "TSL_MtnLk4_Disp_Rq_ISM",UINT16_C(32),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 4 Display Request / Request Transmission Lock Lock 4 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f74s5e,sizeof(f74s5e)/sizeof(f74s5e[0]) },
 { "TSL_MtnLk3_Disp_Rq_ISM",UINT16_C(36),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Transmission Selector Lever Motion Lock 3 Display Request / Request Transmission Lock Lock 3 Display",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f74s6e,sizeof(f74s6e)/sizeof(f74s6e[0]) },
};
static const MblinkMercedesEgs53SignalDefinition f75s[] = {
 { "ISM_DAC",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Driving Authorization Checker / FBS Embassy to Ice",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f76s1e[] = {
 { UINT64_C(0), "OK", "No Fault" },
 { UINT64_C(1), "CURR", "Nominal Current Not Obtained" },
 { UINT64_C(2), "VOLT", "Voltage" },
 { UINT64_C(3), "TEMP", "Temperature" },
 { UINT64_C(4), "DEBOUNCE_CNT", "Debounce Counter" },
 { UINT64_C(5), "TIMEOUT", "Timeout" },
 { UINT64_C(128), "LK", "locking" },
 { UINT64_C(129), "IDLE", "idle" },
 { UINT64_C(130), "SHRT", "Short Circuit" },
 { UINT64_C(131), "SEMI", "Semiconductor Fault" },
 { UINT64_C(132), "CAN", "CAN SPECIFIC FAULT" },
};
static const MblinkMercedesEgs53SignalDefinition f76s[] = {
 { "CurrDtyCyc_Actl",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Actual Current Duty Cycle / Actual Electricity (duty cycle)",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,0.5,0.0,"%",NULL,0U },
 { "SSP_Diag_Stat",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"SSP Diagnostics State / SSP Diagnostic Status",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f76s1e,sizeof(f76s1e)/sizeof(f76s1e[0]) },
 { "SSP_RPM",UINT16_C(26),UINT16_C(14),false,UINT64_C(0),UINT8_C(0),"ACTUAL RPM SSP / Current SSP speed",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"rpm",NULL,0U },
 { "MC_SSP_RS_SSP",UINT16_C(48),UINT16_C(4),false,UINT64_C(0),UINT8_C(0),"Message Counter / Message Counter",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "CRC_SSP_RS_SSP",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"CRC Checksum Byte 1 to 7 Accordinging to SAE J1850 / CRC Checksum Byte 1 - 7 to SAE J1850",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f77s[] = {
 { "D_RS_ISM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Response / Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f78s[] = {
 { "D_RS_SSP",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Diagnostic Response / Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f79s[] = {
 { "SD_RS_ISM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"SystemDiagnostic Response / System Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f80s[] = {
 { "SD_RS_SSP",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"SystemDiagnostic Response / System Diagnostic Response",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f81s[] = {
 { "SG_APPL_ISM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"ECU TO External Application / Controller to External Application",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f82s[] = {
 { "SG_APPL_ISM2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"ECU TO External Application / Controller to External Application",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f83s[] = {
 { "APPL_SG_TSLM",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"External Application to ECU / External application to control unit",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f84s[] = {
 { "TCM_HS1",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f85s[] = {
 { "TCM_HS2",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f86s[] = {
 { "TCM_HS3",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f87s[] = {
 { "TCM_HS4",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f88s[] = {
 { "TCM_HS5",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f89s[] = {
 { "TCM_HS6",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f90s[] = {
 { "TCM_HS7",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53SignalDefinition f91s[] = {
 { "TCM_HS8",UINT16_C(0),UINT16_C(64),false,UINT64_C(0),UINT8_C(0),"Manual Control at Test Bench / Manual control at the test bench",MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP,1.0,0.0,"",NULL,0U },
};
static const MblinkMercedesEgs53EnumValue f92s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f92s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f92s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f92s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f92s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f92s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f92s0e,sizeof(f92s0e)/sizeof(f92s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f92s4e,sizeof(f92s4e)/sizeof(f92s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f92s5e,sizeof(f92s5e)/sizeof(f92s5e[0]) },
 { "WakeupRsn_FSCM2",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f92s6e,sizeof(f92s6e)/sizeof(f92s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f92s8e,sizeof(f92s8e)/sizeof(f92s8e[0]) },
};
static const MblinkMercedesEgs53EnumValue f93s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f93s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f93s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f93s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f93s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f93s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f93s0e,sizeof(f93s0e)/sizeof(f93s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f93s4e,sizeof(f93s4e)/sizeof(f93s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f93s5e,sizeof(f93s5e)/sizeof(f93s5e[0]) },
 { "WakeupRsn_ISM",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f93s6e,sizeof(f93s6e)/sizeof(f93s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f93s8e,sizeof(f93s8e)/sizeof(f93s8e[0]) },
};
static const MblinkMercedesEgs53EnumValue f94s0e[] = {
 { UINT64_C(252), "LHOM", "LIMP-HOME Fashion" },
 { UINT64_C(253), "RING", "ring fashion" },
 { UINT64_C(254), "ALIVE", "Alive mode" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f94s4e[] = {
 { UINT64_C(4), "BROADCAST", "Broadcast or Start Alive" },
 { UINT64_C(63), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f94s5e[] = {
 { UINT64_C(1), "DATA_OK_BC", "UserData Transmission OK (Broadcast)" },
 { UINT64_C(2), "WAKEUP_SA", "Wakeup status (start alive)" },
 { UINT64_C(5), "SBC_STAT_BC", "System Base Chip Status (Broadcast)" },
 { UINT64_C(15), "AWAKE_BC", "Stay Awake Reason (Broadcast)" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f94s6e[] = {
 { UINT64_C(0), "NETWORK", "Wakeup by Network" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53EnumValue f94s8e[] = {
 { UINT64_C(4), "BACKBONE", "Backbone CAN" },
 { UINT64_C(5), "DIAGNOSTICS", "Diagnostics CAN" },
 { UINT64_C(6), "BODY", "Body CAN" },
 { UINT64_C(7), "CHASSIS", "Chassis CAN" },
 { UINT64_C(8), "POWERTRAIN", "Powertrain Can" },
 { UINT64_C(9), "PT_SENSOR", "Powertrain Sensor CAN" },
 { UINT64_C(11), "DYNAMICS", "Dynamics CAN" },
 { UINT64_C(14), "HEADUNIT", "HeadUnit CAN" },
 { UINT64_C(15), "IMPACT", "Impact CAN" },
 { UINT64_C(16), "MULTIPURPOSE", "Multipurpose CAN" },
 { UINT64_C(255), "SNA", "Signal Not Available" },
};
static const MblinkMercedesEgs53SignalDefinition f94s[] = {
 { "NM_Mode",UINT16_C(0),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Mode / Network Management Mode",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f94s0e,sizeof(f94s0e)/sizeof(f94s0e[0]) },
 { "NM_Successor",UINT16_C(8),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management Logical Successor / Network Management Logical Successor",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ind",UINT16_C(16),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Indication / Network Management Sleep Indication",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Sleep_Ack",UINT16_C(17),UINT16_C(1),false,UINT64_C(0),UINT8_C(0),"Network Management Sleep Acknowledge / Network Management Sleep Acknowledge",MBLINK_MERCEDES_EGS53_SIGNAL_BOOL,1.0,0.0,"",NULL,0U },
 { "NM_Ud_Launch",UINT16_C(18),UINT16_C(6),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Launch Type / Network Management UserData Sendart",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f94s4e,sizeof(f94s4e)/sizeof(f94s4e[0]) },
 { "NM_Ud_Srv",UINT16_C(24),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Management UserData Service No./netzmanagement UserData service",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f94s5e,sizeof(f94s5e)/sizeof(f94s5e[0]) },
 { "WakeupRsn_SSP",UINT16_C(32),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Wakeup Reason / Wake-up",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f94s6e,sizeof(f94s6e)/sizeof(f94s6e[0]) },
 { "WakeupCnt",UINT16_C(40),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Counter for Module Wakeup States During Network Sleep / Counter for ECUs Internal Wachzustäustände during bus rest",MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,1.0,0.0,"",NULL,0U },
 { "Nw_Id",UINT16_C(56),UINT16_C(8),false,UINT64_C(0),UINT8_C(0),"Network Identification No./netzwerk-id",MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,1.0,0.0,"",f94s8e,sizeof(f94s8e)/sizeof(f94s8e[0]) },
};
static const MblinkMercedesEgs53FrameDefinition defs[] = {
 { "ECM","CTRL_U_A2",UINT32_C(0x15),f0s,sizeof(f0s)/sizeof(f0s[0]) },
 { "ECM","ECM_A1",UINT32_C(0x30d),f1s,sizeof(f1s)/sizeof(f1s[0]) },
 { "ECM","ECM_A2",UINT32_C(0x349),f2s,sizeof(f2s)/sizeof(f2s[0]) },
 { "ECM","EIS_A1",UINT32_C(0x1),f3s,sizeof(f3s)/sizeof(f3s[0]) },
 { "ECM","IC_A1",UINT32_C(0x19f),f4s,sizeof(f4s)/sizeof(f4s[0]) },
 { "ECM","IC_A3",UINT32_C(0x3e1),f5s,sizeof(f5s)/sizeof(f5s[0]) },
 { "ECM","LM_A1",UINT32_C(0x69),f6s,sizeof(f6s)/sizeof(f6s[0]) },
 { "ECM","SPC_A3",UINT32_C(0x379),f7s,sizeof(f7s)/sizeof(f7s[0]) },
 { "ECM","STW_ANGL_STAT",UINT32_C(0x3),f8s,sizeof(f8s)/sizeof(f8s[0]) },
 { "ECM","BRK_STAT",UINT32_C(0x5),f9s,sizeof(f9s)/sizeof(f9s[0]) },
 { "ECM","CGW_STAT",UINT32_C(0xf),f10s,sizeof(f10s)/sizeof(f10s[0]) },
 { "ECM","BRK_STAT2",UINT32_C(0x5f),f11s,sizeof(f11s)/sizeof(f11s[0]) },
 { "ECM","SBW_RQ_SCCM",UINT32_C(0x6d),f12s,sizeof(f12s)/sizeof(f12s[0]) },
 { "ECM","EPKB_STAT",UINT32_C(0xdd),f13s,sizeof(f13s)/sizeof(f13s[0]) },
 { "ECM","HVAC_RS1",UINT32_C(0xf9),f14s,sizeof(f14s)/sizeof(f14s[0]) },
 { "ECM","TX_RQ_SBC",UINT32_C(0x104),f15s,sizeof(f15s)/sizeof(f15s[0]) },
 { "ECM","ENG_RS3_PT",UINT32_C(0x105),f16s,sizeof(f16s)/sizeof(f16s[0]) },
 { "ECM","ENG_RS2_PT",UINT32_C(0x14b),f17s,sizeof(f17s)/sizeof(f17s[0]) },
 { "ECM","TX_RQ_ECM",UINT32_C(0x17d),f18s,sizeof(f18s)/sizeof(f18s[0]) },
 { "ECM","ENG_RS1_PT",UINT32_C(0x1cd),f19s,sizeof(f19s)/sizeof(f19s[0]) },
 { "ECM","DPM_STAT",UINT32_C(0x200),f20s,sizeof(f20s)/sizeof(f20s[0]) },
 { "ECM","WHL_STAT1",UINT32_C(0x201),f21s,sizeof(f21s)/sizeof(f21s[0]) },
 { "ECM","WHL_STAT2",UINT32_C(0x203),f22s,sizeof(f22s)/sizeof(f22s[0]) },
 { "ECM","PN14_STAT",UINT32_C(0x205),f23s,sizeof(f23s)/sizeof(f23s[0]) },
 { "ECM","CVI",UINT32_C(0x207),f24s,sizeof(f24s)/sizeof(f24s[0]) },
 { "ECM","VEH_DYN_STAT",UINT32_C(0x245),f25s,sizeof(f25s)/sizeof(f25s[0]) },
 { "ECM","BODY_R1",UINT32_C(0x283),f26s,sizeof(f26s)/sizeof(f26s[0]) },
 { "ECM","DAC_TCM",UINT32_C(0x2eb),f27s,sizeof(f27s)/sizeof(f27s[0]) },
 { "ECM","DAC_ISM",UINT32_C(0x2fe),f28s,sizeof(f28s)/sizeof(f28s[0]) },
 { "ECM","BODY_R2",UINT32_C(0x3c5),f29s,sizeof(f29s)/sizeof(f29s[0]) },
 { "ECM","ECM_OBD",UINT32_C(0x3d0),f30s,sizeof(f30s)/sizeof(f30s[0]) },
 { "ECM","APPL_SG_FSCM",UINT32_C(0x6fc),f31s,sizeof(f31s)/sizeof(f31s[0]) },
 { "ECM","APPL_SG_ISM",UINT32_C(0x6ec),f32s,sizeof(f32s)/sizeof(f32s[0]) },
 { "ECM","APPL_SG_TCM",UINT32_C(0x6e4),f33s,sizeof(f33s)/sizeof(f33s[0]) },
 { "ECM","D_RQ_FSCM",UINT32_C(0x6fa),f34s,sizeof(f34s)/sizeof(f34s[0]) },
 { "ECM","D_RQ_ISM",UINT32_C(0x6ea),f35s,sizeof(f35s)/sizeof(f35s[0]) },
 { "ECM","D_RQ_SSP",UINT32_C(0x6f2),f36s,sizeof(f36s)/sizeof(f36s[0]) },
 { "ECM","D_RQ_TCM",UINT32_C(0x7e1),f37s,sizeof(f37s)/sizeof(f37s[0]) },
 { "ECM","D_RQ_TSLM",UINT32_C(0x77a),f38s,sizeof(f38s)/sizeof(f38s[0]) },
 { "ECM","DG_RQ_GLOBAL",UINT32_C(0x440),f39s,sizeof(f39s)/sizeof(f39s[0]) },
 { "ECM","DG_RQ_GLOBAL_UDS",UINT32_C(0x441),f40s,sizeof(f40s)/sizeof(f40s[0]) },
 { "ECM","DG_RQ_OBD",UINT32_C(0x7df),f41s,sizeof(f41s)/sizeof(f41s[0]) },
 { "ECM","NM_ECM",UINT32_C(0x429),f42s,sizeof(f42s)/sizeof(f42s[0]) },
 { "TCM","TCM_A1",UINT32_C(0x2f1),f43s,sizeof(f43s)/sizeof(f43s[0]) },
 { "TCM","TCM_A2",UINT32_C(0x2e2),f44s,sizeof(f44s)/sizeof(f44s[0]) },
 { "TCM","ENG_RQ1_TCM",UINT32_C(0xf1),f45s,sizeof(f45s)/sizeof(f45s[0]) },
 { "TCM","ENG_RQ2_TCM",UINT32_C(0xf3),f46s,sizeof(f46s)/sizeof(f46s[0]) },
 { "TCM","ENG_RQ3_TCM",UINT32_C(0xf4),f47s,sizeof(f47s)/sizeof(f47s[0]) },
 { "TCM","SBW_RS_TCM",UINT32_C(0x1bd),f48s,sizeof(f48s)/sizeof(f48s[0]) },
 { "TCM","TCM_DAC",UINT32_C(0x2f0),f49s,sizeof(f49s)/sizeof(f49s[0]) },
 { "TCM","TCM_DISP_RQ",UINT32_C(0x2f3),f50s,sizeof(f50s)/sizeof(f50s[0]) },
 { "TCM","D_RS_TCM",UINT32_C(0x7e9),f51s,sizeof(f51s)/sizeof(f51s[0]) },
 { "TCM","SD_RS_TCM",UINT32_C(0x59c),f52s,sizeof(f52s)/sizeof(f52s[0]) },
 { "TCM","SG_APPL_TCM",UINT32_C(0x51c),f53s,sizeof(f53s)/sizeof(f53s[0]) },
 { "TCM","TCM_HSA",UINT32_C(0x50a),f54s,sizeof(f54s)/sizeof(f54s[0]) },
 { "TCM","TCM_HSB",UINT32_C(0x50b),f55s,sizeof(f55s)/sizeof(f55s[0]) },
 { "TCM","TCM_HSC",UINT32_C(0x50c),f56s,sizeof(f56s)/sizeof(f56s[0]) },
 { "TCM","TCM_HSD",UINT32_C(0x50d),f57s,sizeof(f57s)/sizeof(f57s[0]) },
 { "TCM","TCM_HSE",UINT32_C(0x50e),f58s,sizeof(f58s)/sizeof(f58s[0]) },
 { "TCM","TCM_HSF",UINT32_C(0x50f),f59s,sizeof(f59s)/sizeof(f59s[0]) },
 { "TCM","TCM_HSG",UINT32_C(0x500),f60s,sizeof(f60s)/sizeof(f60s[0]) },
 { "TCM","TCM_HSH",UINT32_C(0x509),f61s,sizeof(f61s)/sizeof(f61s[0]) },
 { "TCM","NM_TCM",UINT32_C(0x41c),f62s,sizeof(f62s)/sizeof(f62s[0]) },
 { "FSCM","FSCM_STAT",UINT32_C(0x2e5),f63s,sizeof(f63s)/sizeof(f63s[0]) },
 { "FSCM","D_RS_FSCM",UINT32_C(0x49f),f64s,sizeof(f64s)/sizeof(f64s[0]) },
 { "FSCM","SD_RS_FSCM",UINT32_C(0x59f),f65s,sizeof(f65s)/sizeof(f65s[0]) },
 { "FSCM","SG_APPL_FSCM",UINT32_C(0x51f),f66s,sizeof(f66s)/sizeof(f66s[0]) },
 { "FSCM","NM_FSCM",UINT32_C(0x41f),f67s,sizeof(f67s)/sizeof(f67s[0]) },
 { "TSLM","SBW_RS_ISM",UINT32_C(0x73),f68s,sizeof(f68s)/sizeof(f68s[0]) },
 { "TSLM","D_RS_TSLM",UINT32_C(0x4af),f69s,sizeof(f69s)/sizeof(f69s[0]) },
 { "TSLM","SD_RS_TSLM",UINT32_C(0x5af),f70s,sizeof(f70s)/sizeof(f70s[0]) },
 { "TSLM","SG_APPL_TSLM",UINT32_C(0x52f),f71s,sizeof(f71s)/sizeof(f71s[0]) },
 { "TSLM","NM_TSLM",UINT32_C(0x42f),f72s,sizeof(f72s)/sizeof(f72s[0]) },
 { "ANY_ECU","SG_A1",UINT32_C(0x2f7),f73s,sizeof(f73s)/sizeof(f73s[0]) },
 { "ANY_ECU","ISM_DISP_RQ",UINT32_C(0x2f5),f74s,sizeof(f74s)/sizeof(f74s[0]) },
 { "ANY_ECU","ISM_DAC",UINT32_C(0x2f6),f75s,sizeof(f75s)/sizeof(f75s[0]) },
 { "ANY_ECU","SSP_RS_SSP",UINT32_C(0x381),f76s,sizeof(f76s)/sizeof(f76s[0]) },
 { "ANY_ECU","D_RS_ISM",UINT32_C(0x49d),f77s,sizeof(f77s)/sizeof(f77s[0]) },
 { "ANY_ECU","D_RS_SSP",UINT32_C(0x49e),f78s,sizeof(f78s)/sizeof(f78s[0]) },
 { "ANY_ECU","SD_RS_ISM",UINT32_C(0x59d),f79s,sizeof(f79s)/sizeof(f79s[0]) },
 { "ANY_ECU","SD_RS_SSP",UINT32_C(0x59e),f80s,sizeof(f80s)/sizeof(f80s[0]) },
 { "ANY_ECU","SG_APPL_ISM",UINT32_C(0x51d),f81s,sizeof(f81s)/sizeof(f81s[0]) },
 { "ANY_ECU","SG_APPL_ISM2",UINT32_C(0x600),f82s,sizeof(f82s)/sizeof(f82s[0]) },
 { "ANY_ECU","APPL_SG_TSLM",UINT32_C(0x77c),f83s,sizeof(f83s)/sizeof(f83s[0]) },
 { "ANY_ECU","TCM_HS1",UINT32_C(0x501),f84s,sizeof(f84s)/sizeof(f84s[0]) },
 { "ANY_ECU","TCM_HS2",UINT32_C(0x502),f85s,sizeof(f85s)/sizeof(f85s[0]) },
 { "ANY_ECU","TCM_HS3",UINT32_C(0x503),f86s,sizeof(f86s)/sizeof(f86s[0]) },
 { "ANY_ECU","TCM_HS4",UINT32_C(0x504),f87s,sizeof(f87s)/sizeof(f87s[0]) },
 { "ANY_ECU","TCM_HS5",UINT32_C(0x505),f88s,sizeof(f88s)/sizeof(f88s[0]) },
 { "ANY_ECU","TCM_HS6",UINT32_C(0x506),f89s,sizeof(f89s)/sizeof(f89s[0]) },
 { "ANY_ECU","TCM_HS7",UINT32_C(0x507),f90s,sizeof(f90s)/sizeof(f90s[0]) },
 { "ANY_ECU","TCM_HS8",UINT32_C(0x508),f91s,sizeof(f91s)/sizeof(f91s[0]) },
 { "ANY_ECU","NM_FSCM2",UINT32_C(0x435),f92s,sizeof(f92s)/sizeof(f92s[0]) },
 { "ANY_ECU","NM_ISM",UINT32_C(0x41d),f93s,sizeof(f93s)/sizeof(f93s[0]) },
 { "ANY_ECU","NM_SSP",UINT32_C(0x41e),f94s,sizeof(f94s)/sizeof(f94s[0]) },
};

static bool plain(const uint8_t *p,size_t n,uint16_t off,uint16_t len,uint64_t *v)
{
 uint64_t x=0U;uint16_t b;
 if(p==NULL||v==NULL||len==0U||len>64U||(size_t)off+(size_t)len>n*8U)return false;
 for(b=0U;b<len;++b){size_t sb=(size_t)off+b;uint8_t one=(uint8_t)((p[sb/8U]>>(7U-(sb%8U)))&1U);x=(x<<1U)|one;}
 *v=x;return true;
}
static bool masked(const MblinkMercedesEgs53SignalDefinition *s,const uint8_t *p,size_t n,uint64_t *v)
{
 uint64_t frame=0U;unsigned int shift;size_t i;
 if(s==NULL||p==NULL||v==NULL||n!=8U||!s->masked||s->mask_width==0U||s->mask_width>64U)return false;
 if((unsigned int)s->bit_offset+(unsigned int)s->bit_length>64U)return false;
 for(i=0U;i<8U;++i)frame=(frame<<8U)|p[i];
 shift=64U-((unsigned int)s->bit_offset+(unsigned int)s->bit_length);
 if(shift+s->mask_width>64U)return false;
 *v=(frame>>shift)&s->mask;return true;
}
size_t mblink_mercedes_egs53_frame_count(void){return sizeof(defs)/sizeof(defs[0]);}
size_t mblink_mercedes_egs53_signal_count(void){size_t t=0U,i;for(i=0U;i<mblink_mercedes_egs53_frame_count();++i)t+=defs[i].signal_count;return t;}
const MblinkMercedesEgs53FrameDefinition *mblink_mercedes_egs53_frame_at(size_t i){return i<mblink_mercedes_egs53_frame_count()?&defs[i]:NULL;}
size_t mblink_mercedes_egs53_frame_match_count(uint32_t id){size_t c=0U,i;for(i=0U;i<mblink_mercedes_egs53_frame_count();++i)if(defs[i].can_id==id)++c;return c;}
const MblinkMercedesEgs53FrameDefinition *mblink_mercedes_egs53_frame_match_at(uint32_t id,size_t m){size_t c=0U,i;for(i=0U;i<mblink_mercedes_egs53_frame_count();++i){if(defs[i].can_id!=id)continue;if(c++==m)return &defs[i];}return NULL;}
const MblinkMercedesEgs53SignalDefinition *mblink_mercedes_egs53_signal_find(const MblinkMercedesEgs53FrameDefinition *fr,const char *name){size_t i;if(fr==NULL||name==NULL||name[0]=='\0')return NULL;for(i=0U;i<fr->signal_count;++i)if(strcmp(fr->signals[i].name,name)==0)return &fr->signals[i];return NULL;}
bool mblink_mercedes_egs53_decode_signal(const MblinkMercedesEgs53SignalDefinition *s,const uint8_t *p,size_t n,MblinkMercedesEgs53DecodedSignal *d)
{
 MblinkMercedesEgs53DecodedSignal x;uint64_t r;size_t i;
 if(s==NULL||d==NULL)return false;
 if(s->masked){if(!masked(s,p,n,&r))return false;}else if(!plain(p,n,s->bit_offset,s->bit_length,&r))return false;
 memset(&x,0,sizeof(x));x.raw=r;x.unit=s->unit;

 /* EGS53 temperature getters explicitly reject 0xFF as unavailable. */
 if ((strcmp(s->name,"EngCoolTemp")==0 ||
      strcmp(s->name,"EngOilTemp")==0 ||
      strcmp(s->name,"IntkAirTemp")==0) && r==UINT64_C(255)) {
  x.unavailable=true;*d=x;return true;
 }

 switch(s->type){
 case MBLINK_MERCEDES_EGS53_SIGNAL_BOOL:x.boolean_available=true;x.boolean_value=r!=0U;break;
 case MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER:x.physical_available=true;x.physical_value=(double)r*s->multiplier+s->offset;break;
 case MBLINK_MERCEDES_EGS53_SIGNAL_ENUM:for(i=0U;i<s->enum_count;++i)if(s->enum_values[i].raw==r){x.enum_available=true;x.enum_name=s->enum_values[i].name;x.enum_description=s->enum_values[i].description;break;}break;
 case MBLINK_MERCEDES_EGS53_SIGNAL_CHAR:if(r<=UINT8_MAX){x.char_available=true;x.char_value=(char)(uint8_t)r;}break;
 case MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP:break;
 }
 *d=x;return true;
}
const char *mblink_mercedes_egs53_source_revision(void){return "1b96089660e97c91811b3d9cda6ca6f82b458c69";}
const char *mblink_mercedes_egs53_source_path(void){return "lib/egs53_ecus/can_data.txt";}
