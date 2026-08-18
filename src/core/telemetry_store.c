// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file telemetry_store.c
 * @brief Bounded typed telemetry history and session metadata.
 */
#include "mblink/telemetry.h"

#include "infiltratr/core.h"

#include <math.h>
#include <string.h>

void mblink_telemetry_store_init(MblinkTelemetryStore *store)
{
    if (store != NULL) {
        memset(store, 0, sizeof(*store));
    }
}

void mblink_telemetry_store_clear_samples(MblinkTelemetryStore *store)
{
    if (store == NULL) {
        return;
    }

    memset(store->history, 0, sizeof(store->history));
    memset(store->latest, 0, sizeof(store->latest));
    memset(store->latest_valid, 0, sizeof(store->latest_valid));
    memset(store->transcript, 0, sizeof(store->transcript));
    store->transcript_head = 0U;
    store->transcript_count = 0U;
    store->history_head = 0U;
    store->history_count = 0U;
    store->next_sequence = 0U;
    store->total_sample_count = 0U;
}

bool mblink_telemetry_store_record(
    MblinkTelemetryStore *store,
    uint64_t timestamp_ms,
    const MblinkObd2Sample *measurement)
{
    if (store == NULL || measurement == NULL ||
        !isfinite(measurement->value)) {
        return false;
    }

    MblinkTelemetrySample sample = {
        .sequence = store->next_sequence,
        .timestamp_ms = timestamp_ms,
        .measurement = *measurement
    };

    if (store->next_sequence != UINT64_MAX) {
        store->next_sequence++;
    }
    store->total_sample_count =
        infiltratr_u64_add_saturating(store->total_sample_count, 1U);

    store->latest[measurement->pid] = sample;
    store->latest_valid[measurement->pid] = true;

    store->history[store->history_head] = sample;
    store->history_head =
        (store->history_head + 1U) % MBLINK_TELEMETRY_HISTORY_CAPACITY;
    if (store->history_count < MBLINK_TELEMETRY_HISTORY_CAPACITY) {
        store->history_count++;
    }
    return true;
}

bool mblink_telemetry_store_latest(
    const MblinkTelemetryStore *store,
    uint8_t pid,
    MblinkTelemetrySample *sample)
{
    if (store == NULL || sample == NULL || !store->latest_valid[pid]) {
        return false;
    }
    *sample = store->latest[pid];
    return true;
}

size_t mblink_telemetry_store_history_count(
    const MblinkTelemetryStore *store)
{
    return store != NULL ? store->history_count : 0U;
}

uint64_t mblink_telemetry_store_total_sample_count(
    const MblinkTelemetryStore *store)
{
    return store != NULL ? store->total_sample_count : 0U;
}

bool mblink_telemetry_store_history_at(
    const MblinkTelemetryStore *store,
    size_t chronological_index,
    MblinkTelemetrySample *sample)
{
    if (store == NULL || sample == NULL ||
        chronological_index >= store->history_count) {
        return false;
    }

    const size_t oldest =
        (store->history_head + MBLINK_TELEMETRY_HISTORY_CAPACITY -
         store->history_count) %
        MBLINK_TELEMETRY_HISTORY_CAPACITY;
    const size_t storage_index =
        (oldest + chronological_index) % MBLINK_TELEMETRY_HISTORY_CAPACITY;
    *sample = store->history[storage_index];
    return true;
}

void mblink_telemetry_store_set_favourite(
    MblinkTelemetryStore *store, uint8_t pid, bool favourite)
{
    if (store != NULL) {
        store->favourite[pid] = favourite;
    }
}

bool mblink_telemetry_store_is_favourite(
    const MblinkTelemetryStore *store, uint8_t pid)
{
    return store != NULL && store->favourite[pid];
}

bool mblink_telemetry_store_record_transcript(
    MblinkTelemetryStore *store,
    uint64_t timestamp_ms,
    const char *command,
    const MblinkElm327Response *response)
{
    if (store == NULL || command == NULL || response == NULL) {
        return false;
    }

    MblinkTelemetryTranscriptEntry *entry =
        &store->transcript[store->transcript_head];
    memset(entry, 0, sizeof(*entry));
    entry->timestamp_ms = timestamp_ms;
    entry->result = response->result;
    infiltratr_copy_string(entry->command, sizeof(entry->command), command);
    infiltratr_copy_string(entry->response, sizeof(entry->response),
                           response->text);

    store->transcript_head =
        (store->transcript_head + 1U) % MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY;
    if (store->transcript_count < MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY) {
        store->transcript_count++;
    }
    return true;
}

size_t mblink_telemetry_store_transcript_count(
    const MblinkTelemetryStore *store)
{
    return store != NULL ? store->transcript_count : 0U;
}

bool mblink_telemetry_store_transcript_at(
    const MblinkTelemetryStore *store,
    size_t chronological_index,
    MblinkTelemetryTranscriptEntry *entry)
{
    if (store == NULL || entry == NULL ||
        chronological_index >= store->transcript_count) {
        return false;
    }

    const size_t oldest =
        (store->transcript_head + MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY -
         store->transcript_count) %
        MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY;
    const size_t storage_index =
        (oldest + chronological_index) % MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY;
    *entry = store->transcript[storage_index];
    return true;
}

void mblink_telemetry_session_metadata_init(
    MblinkTelemetrySessionMetadata *metadata,
    uint64_t started_epoch_ms,
    const char *adapter_identifier,
    const char *vehicle_identifier)
{
    if (metadata == NULL) {
        return;
    }

    memset(metadata, 0, sizeof(*metadata));
    metadata->started_epoch_ms = started_epoch_ms;
    infiltratr_copy_string(metadata->adapter_identifier,
                           sizeof(metadata->adapter_identifier),
                           adapter_identifier);
    infiltratr_copy_string(metadata->vehicle_identifier,
                           sizeof(metadata->vehicle_identifier),
                           vehicle_identifier);
}

void mblink_telemetry_session_metadata_set_adapter(
    MblinkTelemetrySessionMetadata *metadata,
    const char *adapter_identifier)
{
    if (metadata != NULL) {
        infiltratr_copy_string(metadata->adapter_identifier,
                               sizeof(metadata->adapter_identifier),
                               adapter_identifier);
    }
}

void mblink_telemetry_session_metadata_set_vehicle(
    MblinkTelemetrySessionMetadata *metadata,
    const char *vehicle_identifier)
{
    if (metadata != NULL) {
        infiltratr_copy_string(metadata->vehicle_identifier,
                               sizeof(metadata->vehicle_identifier),
                               vehicle_identifier);
    }
}

void mblink_telemetry_session_metadata_finish(
    MblinkTelemetrySessionMetadata *metadata,
    uint64_t ended_epoch_ms)
{
    if (metadata != NULL) {
        metadata->ended_epoch_ms = ended_epoch_ms;
    }
}
