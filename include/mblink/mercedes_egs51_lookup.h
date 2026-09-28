// SPDX-License-Identifier: GPL-3.0-or-later
/** @file mercedes_egs51_lookup.h @brief Family-isolated EGS51 CAN lookup. */
#ifndef MBLINK_MERCEDES_EGS51_LOOKUP_H
#define MBLINK_MERCEDES_EGS51_LOOKUP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum MblinkMercedesEgs51SignalType {
 MBLINK_MERCEDES_EGS51_SIGNAL_BOOL=0,
 MBLINK_MERCEDES_EGS51_SIGNAL_NUMBER,
 MBLINK_MERCEDES_EGS51_SIGNAL_ENUM,
 MBLINK_MERCEDES_EGS51_SIGNAL_CHAR,
 MBLINK_MERCEDES_EGS51_SIGNAL_ISO_TP
} MblinkMercedesEgs51SignalType;
typedef struct MblinkMercedesEgs51EnumValue { uint64_t raw; const char *name; const char *description; } MblinkMercedesEgs51EnumValue;
typedef struct MblinkMercedesEgs51SignalDefinition {
 const char *name; uint16_t bit_offset; uint16_t bit_length;
 bool masked; uint64_t mask; uint8_t mask_width;
 const char *description; MblinkMercedesEgs51SignalType type;
 double multiplier; double offset; const char *unit;
 const MblinkMercedesEgs51EnumValue *enum_values; size_t enum_count;
} MblinkMercedesEgs51SignalDefinition;
typedef struct MblinkMercedesEgs51FrameDefinition {
 const char *ecu; const char *name; uint32_t can_id;
 const MblinkMercedesEgs51SignalDefinition *signals; size_t signal_count;
} MblinkMercedesEgs51FrameDefinition;
typedef struct MblinkMercedesEgs51DecodedSignal {
 uint64_t raw; bool unavailable; const char *unit;
 bool boolean_available; bool boolean_value;
 bool physical_available; double physical_value;
 bool enum_available; const char *enum_name; const char *enum_description;
 bool char_available; char char_value;
} MblinkMercedesEgs51DecodedSignal;
size_t mblink_mercedes_egs51_frame_count(void);
size_t mblink_mercedes_egs51_signal_count(void);
const MblinkMercedesEgs51FrameDefinition *mblink_mercedes_egs51_frame_at(size_t index);
size_t mblink_mercedes_egs51_frame_match_count(uint32_t can_id);
const MblinkMercedesEgs51FrameDefinition *mblink_mercedes_egs51_frame_match_at(uint32_t can_id,size_t match_index);
const MblinkMercedesEgs51SignalDefinition *mblink_mercedes_egs51_signal_find(const MblinkMercedesEgs51FrameDefinition *frame,const char *signal_name);
bool mblink_mercedes_egs51_decode_signal(const MblinkMercedesEgs51SignalDefinition *signal,const uint8_t *payload,size_t payload_length,MblinkMercedesEgs51DecodedSignal *decoded);
const char *mblink_mercedes_egs51_source_revision(void);
const char *mblink_mercedes_egs51_source_path(void);
#ifdef __cplusplus
}
#endif
#endif
