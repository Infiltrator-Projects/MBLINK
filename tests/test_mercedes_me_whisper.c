// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_me_whisper.h"

#include <stdio.h>
#include <string.h>

#define CHECK(e) do { \
    if (!(e)) { \
        fprintf(stderr, "check failed: %s at %s:%d\n", \
                #e, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

int main(void)
{
    const MblinkMercedesMeWhisperVocabularyEntry *entry;
    LinkDiagnosticResponseSelectionPolicy policy;

    CHECK(strcmp(
        mblink_mercedes_me_whisper_response_selection_name(
            MBLINK_MERCEDES_ME_WHISPER_SELECT_LOWEST_CANID_CACHED),
        "SELECT_LOWEST_CANID_CACHED") == 0);
    CHECK(mblink_mercedes_me_whisper_response_selection_from_name(
              "SELECT_MAXIMUM") ==
          MBLINK_MERCEDES_ME_WHISPER_SELECT_MAXIMUM);
    CHECK(mblink_mercedes_me_whisper_response_selection_policy(
              MBLINK_MERCEDES_ME_WHISPER_SELECT_LOWEST_CANID_CACHED,
              &policy));
    CHECK(policy == LINK_DIAGNOSTIC_RESPONSE_SELECT_LOWEST_CAN_ID_CACHED);
    CHECK(strcmp(
        mblink_mercedes_me_whisper_dtc_presentation_name(
            MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_UDS_DAI),
        "SAEDTC_UDS_DAI") == 0);
    CHECK(mblink_mercedes_me_whisper_dtc_presentation_from_name(
              "SAEDTC_OBD") ==
          MBLINK_MERCEDES_ME_WHISPER_SAE_DTC_OBD);
    CHECK(mblink_mercedes_me_whisper_response_selection_from_name(
              "bogus") ==
          MBLINK_MERCEDES_ME_WHISPER_RESPONSE_SELECTION_UNKNOWN);

    CHECK(mblink_mercedes_me_whisper_vocabulary_count() == 31U);
    entry = mblink_mercedes_me_whisper_vocabulary_find("config.properties");
    CHECK(entry != NULL);
    CHECK(entry->kind == MBLINK_MERCEDES_ME_WHISPER_VOCAB_RESOURCE);
    entry = mblink_mercedes_me_whisper_vocabulary_find("formula");
    CHECK(entry != NULL);
    CHECK(entry->kind == MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY);
    entry = mblink_mercedes_me_whisper_vocabulary_find("alwaysAvailable");
    CHECK(entry != NULL);
    CHECK(entry->kind == MBLINK_MERCEDES_ME_WHISPER_VOCAB_CONFIGURATION_KEY);
    CHECK(mblink_mercedes_me_whisper_vocabulary_find("responseselection") != NULL);
    CHECK(mblink_mercedes_me_whisper_vocabulary_find("DATAID.dataPoints") != NULL);
    CHECK(mblink_mercedes_me_whisper_vocabulary_find("DATAID.dataPoints.children") != NULL);
    CHECK(mblink_mercedes_me_whisper_vocabulary_find("PaddingByte") != NULL);
    CHECK(mblink_mercedes_me_whisper_vocabulary_find("PduTransceive") != NULL);
    CHECK(mblink_mercedes_me_whisper_vocabulary_find("not-recovered") == NULL);
    CHECK(mblink_mercedes_me_whisper_vocabulary_at(31U) == NULL);
    return 0;
}
