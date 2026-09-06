// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_me_whisper.h"

#include <string.h>

static const char *const response_selection_names[] = {
    "SELECT_FIRST",
    "SELECT_LOWEST_CANID_CACHED",
    "SELECT_MAXIMUM",
    "MERGE_ELIMINATE_DUPLICATES"
};

static const char *const dtc_presentation_names[] = {
    "SAEDTC_KWP_DAI",
    "SAEDTC_KWP_VW",
    "SAEDTC_UDS_DAI",
    "SAEDTC_UDS_VW",
    "SAEDTC_OBD"
};

static const MblinkMercedesMeWhisperVocabularyEntry whisper_vocabulary[] = {
    { "config.properties", MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE },
    { "_configs", MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE },
    { "deviceproviders", MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE },
    { "actionproviders", MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE },
    { "activeconfiguration", MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE },
    { "vinmapping", MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE },
    { "alwaysAvailable", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "baudrate", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "p2star", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "readInterval", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "encoding", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "formula", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "unit", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "relevance", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "throttle", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "bitmask", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "responseselection", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "message_limit", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "channel", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "requestid", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "resultid", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "timeout", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "filter", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "extract", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "PaddingByte", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "Timeout", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "DATAID", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "DATAID.dataPoints", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "DATAID.dataPoints.children", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "REQUESTID", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY },
    { "PduTransceive", MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY }
};

const char *mblink_mercedes_me_whisper_response_selection_name(
    MblinkMercedesMeWhisperResponseSelection selection)
{
    const unsigned int index = (unsigned int)selection;
    if (index >=
        sizeof(response_selection_names) / sizeof(response_selection_names[0]))
        return "UNKNOWN";
    return response_selection_names[index];
}

MblinkMercedesMeWhisperResponseSelection
mblink_mercedes_me_whisper_response_selection_from_name(const char *name)
{
    unsigned int index;
    if (name == NULL)
        return MBLINK_MERCEDES_ME_WHISPER_RESPONSE_SELECTION_UNKNOWN;
    for (index = 0U;
         index <
             sizeof(response_selection_names) /
                 sizeof(response_selection_names[0]);
         ++index) {
        if (strcmp(name, response_selection_names[index]) == 0)
            return (MblinkMercedesMeWhisperResponseSelection)index;
    }
    return MBLINK_MERCEDES_ME_WHISPER_RESPONSE_SELECTION_UNKNOWN;
}

bool mblink_mercedes_me_whisper_response_selection_policy(
    MblinkMercedesMeWhisperResponseSelection selection,
    LinkDiagnosticResponseSelectionPolicy *policy)
{
    if (policy == NULL) return false;
    switch (selection) {
    case MBLINK_MERCEDES_ME_WHISPER_SELECT_FIRST:
        *policy = LINK_DIAGNOSTIC_RESPONSE_SELECT_FIRST;
        return true;
    case MBLINK_MERCEDES_ME_WHISPER_SELECT_LOWEST_CANID_CACHED:
        *policy = LINK_DIAGNOSTIC_RESPONSE_SELECT_LOWEST_CAN_ID_CACHED;
        return true;
    case MBLINK_MERCEDES_ME_WHISPER_SELECT_MAXIMUM:
        *policy = LINK_DIAGNOSTIC_RESPONSE_SELECT_MAXIMUM;
        return true;
    case MBLINK_MERCEDES_ME_WHISPER_MERGE_ELIMINATE_DUPLICATES:
        *policy = LINK_DIAGNOSTIC_RESPONSE_MERGE_ELIMINATE_DUPLICATES;
        return true;
    case MBLINK_MERCEDES_ME_WHISPER_RESPONSE_SELECTION_UNKNOWN:
        break;
    }
    return false;
}

const char *mblink_mercedes_me_whisper_dtc_presentation_name(
    MblinkMercedesMeWhisperDtcPresentation presentation)
{
    const unsigned int index = (unsigned int)presentation;
    if (index >=
        sizeof(dtc_presentation_names) / sizeof(dtc_presentation_names[0]))
        return "UNKNOWN";
    return dtc_presentation_names[index];
}

MblinkMercedesMeWhisperDtcPresentation
mblink_mercedes_me_whisper_dtc_presentation_from_name(const char *name)
{
    unsigned int index;
    if (name == NULL)
        return MBLINK_MERCEDES_ME_WHISPER_DTC_PRESENTATION_UNKNOWN;
    for (index = 0U;
         index <
             sizeof(dtc_presentation_names) /
                 sizeof(dtc_presentation_names[0]);
         ++index) {
        if (strcmp(name, dtc_presentation_names[index]) == 0)
            return (MblinkMercedesMeWhisperDtcPresentation)index;
    }
    return MBLINK_MERCEDES_ME_WHISPER_DTC_PRESENTATION_UNKNOWN;
}

size_t mblink_mercedes_me_whisper_vocabulary_count(void)
{
    return sizeof(whisper_vocabulary) / sizeof(whisper_vocabulary[0]);
}

const MblinkMercedesMeWhisperVocabularyEntry *
mblink_mercedes_me_whisper_vocabulary_at(size_t index)
{
    return index < mblink_mercedes_me_whisper_vocabulary_count()
        ? &whisper_vocabulary[index] : NULL;
}

const MblinkMercedesMeWhisperVocabularyEntry *
mblink_mercedes_me_whisper_vocabulary_find(const char *name)
{
    size_t index;
    if (name == NULL) return NULL;
    for (index = 0U; index < mblink_mercedes_me_whisper_vocabulary_count(); ++index) {
        if (strcmp(name, whisper_vocabulary[index].name) == 0)
            return &whisper_vocabulary[index];
    }
    return NULL;
}
