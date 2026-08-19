// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file parameter_store.h
 * @brief Bounded protocol-neutral diagnostic parameter state and history.
 *
 * Definitions are borrowed from caller-owned/static catalogs and must outlive
 * the store registration. Samples are copied by value; their definition
 * pointer is canonicalised to the registered definition.
 */
#ifndef MBLINK_PARAMETER_STORE_H
#define MBLINK_PARAMETER_STORE_H

#include "mblink/parameter.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_PARAMETER_STORE_DEFINITION_CAPACITY 256U
#define MBLINK_PARAMETER_STORE_HISTORY_CAPACITY 1024U

typedef enum {
    MBLINK_PARAMETER_STORE_OK = 0,
    MBLINK_PARAMETER_STORE_INVALID_ARGUMENT,
    MBLINK_PARAMETER_STORE_FULL,
    MBLINK_PARAMETER_STORE_DUPLICATE_KEY,
    MBLINK_PARAMETER_STORE_DUPLICATE_STABLE_KEY,
    MBLINK_PARAMETER_STORE_NOT_FOUND,
    MBLINK_PARAMETER_STORE_DEFINITION_MISMATCH
} MblinkParameterStoreResult;

typedef struct {
    const MblinkParameterDefinition *definition;
    MblinkParameterSample latest;
    bool latest_valid;
    bool favourite;
} MblinkParameterStoreSlot;

typedef struct {
    MblinkParameterStoreSlot slots[MBLINK_PARAMETER_STORE_DEFINITION_CAPACITY];
    MblinkParameterSample history[MBLINK_PARAMETER_STORE_HISTORY_CAPACITY];
    size_t slot_count;
    size_t history_head;
    size_t history_count;
    uint64_t total_sample_count;
} MblinkParameterStore;

const char *mblink_parameter_store_result_name(MblinkParameterStoreResult result);

void mblink_parameter_store_init(MblinkParameterStore *store);

/** Clear latest/history state while preserving definitions and favourites. */
void mblink_parameter_store_clear_samples(MblinkParameterStore *store);

/** Register one borrowed definition; keys and stable keys must both be unique. */
MblinkParameterStoreResult mblink_parameter_store_register(
    MblinkParameterStore *store,
    const MblinkParameterDefinition *definition);

size_t mblink_parameter_store_definition_count(const MblinkParameterStore *store);

const MblinkParameterDefinition *mblink_parameter_store_definition_at(
    const MblinkParameterStore *store,
    size_t index);

const MblinkParameterDefinition *mblink_parameter_store_definition(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key);

const MblinkParameterDefinition *mblink_parameter_store_definition_for_stable_key(
    const MblinkParameterStore *store,
    const char *stable_key);

MblinkParameterStoreResult mblink_parameter_store_set_favourite(
    MblinkParameterStore *store,
    const MblinkParameterKey *key,
    bool favourite);

bool mblink_parameter_store_is_favourite(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key);

/** Record one sample transactionally after validating its registered definition. */
MblinkParameterStoreResult mblink_parameter_store_record(
    MblinkParameterStore *store,
    const MblinkParameterSample *sample);

bool mblink_parameter_store_latest(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key,
    MblinkParameterSample *sample);

size_t mblink_parameter_store_history_count(const MblinkParameterStore *store);
uint64_t mblink_parameter_store_total_sample_count(const MblinkParameterStore *store);

bool mblink_parameter_store_history_at(
    const MblinkParameterStore *store,
    size_t chronological_index,
    MblinkParameterSample *sample);

#ifdef __cplusplus
}
#endif

#endif
