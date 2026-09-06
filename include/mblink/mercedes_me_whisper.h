// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBMBLINK_MERCEDES_ME_WHISPER_H
#define MBMBLINK_MERCEDES_ME_WHISPER_H

#include "link/diagnostic_request.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MblinkMercedesMeWhisperResponseSelection {
    MBLINK_MERCEDES_ME_WHISPER_SELECT_FIRST = 0,
    MBLINK_MERCEDES_ME_WHISPER_SELECT_LOWEST_CANID_CACHED,
    MBLINK_MERCEDES_ME_WHISPER_SELECT_MAXIMUM,
    MBLINK_MERCEDES_ME_WHISPER_MERGE_ELIMINATE_DUPLICATES,
    MBLINK_MERCEDES_ME_WHISPER_RESPONSE_SELECTION_UNKNOWN
} MblinkMercedesMeWhisperResponseSelection;

typedef enum MblinkMercedesMeWhisperDtcPresentation {
    MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_KWP_DAI = 0,
    MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_KWP_VW,
    MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_UDS_DAI,
    MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_UDS_VW,
    MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_OBD,
    MBLINK_MERCEDES_ME_WHISPER_DTC_PRESENTATION_UNKNOWN
} MblinkMercedesMeWhisperDtcPresentation;

typedef enum MblinkMercedesMeWhisperVocabularyKind {
    MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE = 0,
    MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY
} MblinkMercedesMeWhisperVocabularyKind;

typedef struct MblinkMercedesMeWhisperVocabularyEntry {
    const char *name;
    MblinkMercedesMeWhisperVocabularyKind kind;
} MblinkMercedesMeWhisperVocabularyEntry;

const char *mblink_mercedes_me_whisper_response_selection_name(
    MblinkMercedesMeWhisperResponseSelection selection);
MblinkMercedesMeWhisperResponseSelection
mblink_mercedes_me_whisper_response_selection_from_name(const char *name);

/** Map recovered Whisper policy names onto LINK's transport-neutral engine. */
bool mblink_mercedes_me_whisper_response_selection_policy(
    MblinkMercedesMeWhisperResponseSelection selection,
    LinkDiagnosticResponseSelectionPolicy *policy);
const char *mblink_mercedes_me_whisper_dtc_presentation_name(
    MblinkMercedesMeWhisperDtcPresentation presentation);
MblinkMercedesMeWhisperDtcPresentation
mblink_mercedes_me_whisper_dtc_presentation_from_name(const char *name);

/** Exact standalone configuration/resource strings recovered from libwhisper.so. */
size_t mblink_mercedes_me_whisper_vocabulary_count(void);
const MblinkMercedesMeWhisperVocabularyEntry *
mblink_mercedes_me_whisper_vocabulary_at(size_t index);
const MblinkMercedesMeWhisperVocabularyEntry *
mblink_mercedes_me_whisper_vocabulary_find(const char *name);

#ifdef __cplusplus
}
#endif
#endif
