// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file telemetry.h
 * @brief Portable typed sample history, favourites and CSV session export.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#ifndef MBLINK_TELEMETRY_H
#define MBLINK_TELEMETRY_H

#include "mblink/elm327.h"
#include "mblink/obd2.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_TELEMETRY_HISTORY_CAPACITY 512U
#define MBLINK_TELEMETRY_ADAPTER_TEXT_LENGTH 96U
#define MBLINK_TELEMETRY_VEHICLE_TEXT_LENGTH 64U
#define MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY 64U
#define MBLINK_TELEMETRY_TRANSCRIPT_COMMAND_LENGTH 64U
#define MBLINK_TELEMETRY_TRANSCRIPT_RESPONSE_LENGTH 192U

typedef struct {
    uint64_t sequence;
    uint64_t timestamp_ms;
    MblinkObd2Sample measurement;
} MblinkTelemetrySample;

typedef struct {
    uint64_t timestamp_ms;
    MblinkElm327Result result;
    char command[MBLINK_TELEMETRY_TRANSCRIPT_COMMAND_LENGTH];
    char response[MBLINK_TELEMETRY_TRANSCRIPT_RESPONSE_LENGTH];
} MblinkTelemetryTranscriptEntry;

typedef struct {
    MblinkTelemetrySample history[MBLINK_TELEMETRY_HISTORY_CAPACITY];
    MblinkTelemetrySample latest[256U];
    bool latest_valid[256U];
    bool favourite[256U];
    MblinkTelemetryTranscriptEntry transcript[MBLINK_TELEMETRY_TRANSCRIPT_CAPACITY];
    size_t transcript_head;
    size_t transcript_count;
    size_t history_head;
    size_t history_count;
    uint64_t next_sequence;
    uint64_t total_sample_count;
} MblinkTelemetryStore;

typedef struct {
    uint64_t started_epoch_ms;
    uint64_t ended_epoch_ms;
    char adapter_identifier[MBLINK_TELEMETRY_ADAPTER_TEXT_LENGTH];
    char vehicle_identifier[MBLINK_TELEMETRY_VEHICLE_TEXT_LENGTH];
} MblinkTelemetrySessionMetadata;

typedef bool (*MblinkTelemetryTextSink)(
    void *context, const char *bytes, size_t length);

typedef struct {
    MblinkTelemetryTextSink sink;
    void *context;
    bool started;
    bool finished;
} MblinkTelemetryRecorder;

void mblink_telemetry_store_init(MblinkTelemetryStore *store);
void mblink_telemetry_store_clear_samples(MblinkTelemetryStore *store);

bool mblink_telemetry_store_record(
    MblinkTelemetryStore *store,
    uint64_t timestamp_ms,
    const MblinkObd2Sample *measurement);

bool mblink_telemetry_store_latest(
    const MblinkTelemetryStore *store,
    uint8_t pid,
    MblinkTelemetrySample *sample);

size_t mblink_telemetry_store_history_count(
    const MblinkTelemetryStore *store);

uint64_t mblink_telemetry_store_total_sample_count(
    const MblinkTelemetryStore *store);

bool mblink_telemetry_store_history_at(
    const MblinkTelemetryStore *store,
    size_t chronological_index,
    MblinkTelemetrySample *sample);

void mblink_telemetry_store_set_favourite(
    MblinkTelemetryStore *store, uint8_t pid, bool favourite);

bool mblink_telemetry_store_is_favourite(
    const MblinkTelemetryStore *store, uint8_t pid);

bool mblink_telemetry_store_record_transcript(
    MblinkTelemetryStore *store,
    uint64_t timestamp_ms,
    const char *command,
    const MblinkElm327Response *response);

size_t mblink_telemetry_store_transcript_count(
    const MblinkTelemetryStore *store);

bool mblink_telemetry_store_transcript_at(
    const MblinkTelemetryStore *store,
    size_t chronological_index,
    MblinkTelemetryTranscriptEntry *entry);

void mblink_telemetry_session_metadata_init(
    MblinkTelemetrySessionMetadata *metadata,
    uint64_t started_epoch_ms,
    const char *adapter_identifier,
    const char *vehicle_identifier);

void mblink_telemetry_session_metadata_set_adapter(
    MblinkTelemetrySessionMetadata *metadata,
    const char *adapter_identifier);

void mblink_telemetry_session_metadata_set_vehicle(
    MblinkTelemetrySessionMetadata *metadata,
    const char *vehicle_identifier);

void mblink_telemetry_session_metadata_finish(
    MblinkTelemetrySessionMetadata *metadata,
    uint64_t ended_epoch_ms);

void mblink_telemetry_recorder_init(MblinkTelemetryRecorder *recorder);

bool mblink_telemetry_recorder_begin(
    MblinkTelemetryRecorder *recorder,
    const MblinkTelemetrySessionMetadata *metadata,
    MblinkTelemetryTextSink sink,
    void *context);

bool mblink_telemetry_recorder_record_sample(
    MblinkTelemetryRecorder *recorder,
    const MblinkTelemetrySample *sample,
    bool favourite);

bool mblink_telemetry_recorder_record_response(
    MblinkTelemetryRecorder *recorder,
    uint64_t timestamp_ms,
    const char *command,
    const MblinkElm327Response *response);

bool mblink_telemetry_recorder_finish(
    MblinkTelemetryRecorder *recorder, uint64_t ended_epoch_ms);

bool mblink_telemetry_export_csv(
    const MblinkTelemetryStore *store,
    const MblinkTelemetrySessionMetadata *metadata,
    MblinkTelemetryTextSink sink,
    void *context);

#ifdef __cplusplus
}
#endif

#endif
