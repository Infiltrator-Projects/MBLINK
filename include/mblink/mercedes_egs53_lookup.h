// SPDX-License-Identifier: GPL-3.0-or-later
/** @file mercedes_egs53_lookup.h @brief Family-isolated EGS53 CAN lookup. */
#ifndef MBLINK_MERCEDES_EGS53_LOOKUP_H
#define MBLINK_MERCEDES_EGS53_LOOKUP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum MblinkMercedesEgs53SignalType {
 MBLINK_MERCEDES_EGS53_SIGNAL_BOOL=0,
 MBLINK_MERCEDES_EGS53_SIGNAL_NUMBER,
 MBLINK_MERCEDES_EGS53_SIGNAL_ENUM,
 MBLINK_MERCEDES_EGS53_SIGNAL_CHAR,
 MBLINK_MERCEDES_EGS53_SIGNAL_ISO_TP
} MblinkMercedesEgs53SignalType;
typedef struct MblinkMercedesEgs53EnumValue { uint64_t raw; const char *name; const char *description; } MblinkMercedesEgs53EnumValue;
typedef struct MblinkMercedesEgs53SignalDefinition {
 const char *name; uint16_t bit_offset; uint16_t bit_length;
 bool masked; uint64_t mask; uint8_t mask_width;
 const char *description; MblinkMercedesEgs53SignalType type;
 double multiplier; double offset; const char *unit;
 const MblinkMercedesEgs53EnumValue *enum_values; size_t enum_count;
} MblinkMercedesEgs53SignalDefinition;
typedef struct MblinkMercedesEgs53FrameDefinition {
 const char *ecu; const char *name; uint32_t can_id;
 const MblinkMercedesEgs53SignalDefinition *signals; size_t signal_count;
} MblinkMercedesEgs53FrameDefinition;
typedef struct MblinkMercedesEgs53DecodedSignal {
 uint64_t raw; bool boolean_available; bool boolean_value;
 bool physical_available; double physical_value;
 bool enum_available; const char *enum_name; const char *enum_description;
 bool char_available; char char_value;
} MblinkMercedesEgs53DecodedSignal;
size_t mblink_mercedes_egs53_frame_count(void);
size_t mblink_mercedes_egs53_signal_count(void);
const MblinkMercedesEgs53FrameDefinition *mblink_mercedes_egs53_frame_at(size_t index);
size_t mblink_mercedes_egs53_frame_match_count(uint32_t can_id);
const MblinkMercedesEgs53FrameDefinition *mblink_mercedes_egs53_frame_match_at(uint32_t can_id,size_t match_index);
const MblinkMercedesEgs53SignalDefinition *mblink_mercedes_egs53_signal_find(const MblinkMercedesEgs53FrameDefinition *frame,const char *signal_name);
bool mblink_mercedes_egs53_decode_signal(const MblinkMercedesEgs53SignalDefinition *signal,const uint8_t *payload,size_t payload_length,MblinkMercedesEgs53DecodedSignal *decoded);
const char *mblink_mercedes_egs53_source_revision(void);
const char *mblink_mercedes_egs53_source_path(void);
#ifdef __cplusplus
}
#endif
#endif
