// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file parameter.c
 * @brief Protocol-neutral diagnostic parameter metadata, formatting and store.
 */
#include "mblink/parameter.h"
#include "mblink/parameter_store.h"

#include "infiltratr/core.h"
#include "infiltratr/format.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#define OBD_KEY(pid_value) \
    { MBLINK_PARAMETER_PROTOCOL_OBD2, \
      MBLINK_PARAMETER_MODULE_STANDARD_OBD2, (pid_value) }

typedef struct {
    MblinkParameterDefinition definition;
    MblinkObd2Unit expected_unit;
} MblinkObd2ParameterEntry;

static const MblinkObd2ParameterEntry mblink_obd2_parameters[] = {
    { { OBD_KEY(0x0cU), "obd2.engine.rpm", "RPM", "Engine speed", " rpm",
        0U, false, 0.0, 0.0 }, MBLINK_OBD2_UNIT_RPM },
    { { OBD_KEY(0x0dU), "obd2.vehicle.speed", "SPEED", "Vehicle speed", " km/h",
        0U, false, 0.0, 0.0 }, MBLINK_OBD2_UNIT_KMH },
    { { OBD_KEY(0x0bU), "obd2.engine.map", "MAP", "Manifold pressure", " kPa",
        0U, false, 0.0, 0.0 }, MBLINK_OBD2_UNIT_KPA },
    { { OBD_KEY(0x11U), "obd2.engine.throttle", "THROTTLE", "Throttle position", "%",
        0U, true, 0.0, 100.0 }, MBLINK_OBD2_UNIT_PERCENT },
    { { OBD_KEY(0x04U), "obd2.engine.load", "LOAD", "Calculated engine load", "%",
        0U, true, 0.0, 100.0 }, MBLINK_OBD2_UNIT_PERCENT },
    { { OBD_KEY(0x10U), "obd2.engine.maf", "MAF", "Mass air flow", " g/s",
        1U, false, 0.0, 0.0 }, MBLINK_OBD2_UNIT_GRAMS_PER_SECOND },
    { { OBD_KEY(0x05U), "obd2.engine.coolant", "ECT", "Coolant temperature", " °C",
        0U, false, 0.0, 0.0 }, MBLINK_OBD2_UNIT_CELSIUS },
    { { OBD_KEY(0x0fU), "obd2.engine.intake_air", "IAT", "Intake air temperature", " °C",
        0U, false, 0.0, 0.0 }, MBLINK_OBD2_UNIT_CELSIUS }
};

const char *mblink_parameter_protocol_name(MblinkParameterProtocol protocol)
{
    switch (protocol) {
    case MBLINK_PARAMETER_PROTOCOL_OBD2: return "obd2";
    case MBLINK_PARAMETER_PROTOCOL_UDS: return "uds";
    }
    return "unknown";
}

bool mblink_parameter_key_is_valid(const MblinkParameterKey *key)
{
    if (key == NULL) {
        return false;
    }

    switch (key->protocol) {
    case MBLINK_PARAMETER_PROTOCOL_OBD2:
        return key->module == MBLINK_PARAMETER_MODULE_STANDARD_OBD2 &&
               key->identifier <= UINT8_MAX;
    case MBLINK_PARAMETER_PROTOCOL_UDS:
        return key->identifier <= UINT16_MAX;
    }
    return false;
}

bool mblink_parameter_key_equal(const MblinkParameterKey *left,
                                const MblinkParameterKey *right)
{
    return left != NULL && right != NULL &&
           left->protocol == right->protocol &&
           left->module == right->module &&
           left->identifier == right->identifier;
}

bool mblink_parameter_definition_is_valid(
    const MblinkParameterDefinition *definition)
{
    if (definition == NULL ||
        !mblink_parameter_key_is_valid(&definition->key) ||
        definition->stable_key == NULL || definition->stable_key[0] == '\0' ||
        definition->short_name == NULL || definition->short_name[0] == '\0' ||
        definition->name == NULL || definition->name[0] == '\0' ||
        definition->suffix == NULL || definition->decimal_places > 9U) {
        return false;
    }

    if (definition->clamp &&
        (!isfinite(definition->minimum) || !isfinite(definition->maximum) ||
         definition->minimum > definition->maximum)) {
        return false;
    }
    return true;
}

bool mblink_parameter_sample_is_valid(const MblinkParameterSample *sample)
{
    return sample != NULL &&
           mblink_parameter_definition_is_valid(sample->definition) &&
           (!sample->available || isfinite(sample->value));
}

bool mblink_parameter_format_value(
    const MblinkParameterDefinition *definition,
    bool available,
    double value,
    char *buffer,
    size_t buffer_size)
{
    InfiltratrScalarFormatOptions options = INFILTRATR_SCALAR_FORMAT_OPTIONS_INIT;

    if (!mblink_parameter_definition_is_valid(definition) ||
        buffer == NULL || buffer_size == 0U) {
        return false;
    }

    options.decimal_places = definition->decimal_places;
    options.clamp = definition->clamp;
    options.minimum = (long double)definition->minimum;
    options.maximum = (long double)definition->maximum;
    options.suffix = definition->suffix;
    return infiltratr_format_scalar(
        available, (long double)value, &options, buffer, buffer_size);
}

bool mblink_parameter_format_sample(
    const MblinkParameterSample *sample,
    char *buffer,
    size_t buffer_size)
{
    if (sample == NULL ||
        !mblink_parameter_definition_is_valid(sample->definition)) {
        return false;
    }
    return mblink_parameter_format_value(
        sample->definition, sample->available, sample->value,
        buffer, buffer_size);
}

size_t mblink_parameter_obd2_definition_count(void)
{
    return INFILTRATR_ARRAY_LENGTH(mblink_obd2_parameters);
}

const MblinkParameterDefinition *mblink_parameter_obd2_definition_at(
    size_t index)
{
    if (index >= mblink_parameter_obd2_definition_count()) {
        return NULL;
    }
    return &mblink_obd2_parameters[index].definition;
}

const MblinkParameterDefinition *mblink_parameter_obd2_definition(uint8_t pid)
{
    size_t index;

    for (index = 0U; index < mblink_parameter_obd2_definition_count(); ++index) {
        if (mblink_obd2_parameters[index].definition.key.identifier ==
            (uint32_t)pid) {
            return &mblink_obd2_parameters[index].definition;
        }
    }
    return NULL;
}

const MblinkParameterDefinition *mblink_parameter_obd2_definition_for_stable_key(
    const char *stable_key)
{
    size_t index;

    if (stable_key == NULL || stable_key[0] == '\0') {
        return NULL;
    }

    for (index = 0U; index < mblink_parameter_obd2_definition_count(); ++index) {
        if (infiltratr_string_equal(
                mblink_obd2_parameters[index].definition.stable_key,
                stable_key)) {
            return &mblink_obd2_parameters[index].definition;
        }
    }
    return NULL;
}

bool mblink_parameter_from_obd2(
    const MblinkObd2Sample *sample,
    uint64_t timestamp_ms,
    MblinkParameterSample *parameter)
{
    size_t index;
    MblinkParameterSample converted;

    if (sample == NULL || parameter == NULL || !isfinite(sample->value)) {
        return false;
    }

    for (index = 0U; index < mblink_parameter_obd2_definition_count(); ++index) {
        const MblinkObd2ParameterEntry *entry = &mblink_obd2_parameters[index];
        if (entry->definition.key.identifier != (uint32_t)sample->pid) {
            continue;
        }
        if (entry->expected_unit != sample->unit) {
            return false;
        }

        converted.definition = &entry->definition;
        converted.timestamp_ms = timestamp_ms;
        converted.available = true;
        converted.value = sample->value;
        *parameter = converted;
        return true;
    }

    return false;
}

static size_t mblink_parameter_store_find_key(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key)
{
    size_t index;

    if (store == NULL || !mblink_parameter_key_is_valid(key)) {
        return SIZE_MAX;
    }

    for (index = 0U; index < store->slot_count; ++index) {
        const MblinkParameterDefinition *definition = store->slots[index].definition;
        if (definition != NULL &&
            mblink_parameter_key_equal(&definition->key, key)) {
            return index;
        }
    }
    return SIZE_MAX;
}

static size_t mblink_parameter_store_find_stable_key(
    const MblinkParameterStore *store,
    const char *stable_key)
{
    size_t index;

    if (store == NULL || stable_key == NULL || stable_key[0] == '\0') {
        return SIZE_MAX;
    }

    for (index = 0U; index < store->slot_count; ++index) {
        const MblinkParameterDefinition *definition = store->slots[index].definition;
        if (definition != NULL &&
            infiltratr_string_equal(definition->stable_key, stable_key)) {
            return index;
        }
    }
    return SIZE_MAX;
}

const char *mblink_parameter_store_result_name(MblinkParameterStoreResult result)
{
    switch (result) {
    case MBLINK_PARAMETER_STORE_OK: return "ok";
    case MBLINK_PARAMETER_STORE_INVALID_ARGUMENT: return "invalid-argument";
    case MBLINK_PARAMETER_STORE_FULL: return "full";
    case MBLINK_PARAMETER_STORE_DUPLICATE_KEY: return "duplicate-key";
    case MBLINK_PARAMETER_STORE_DUPLICATE_STABLE_KEY:
        return "duplicate-stable-key";
    case MBLINK_PARAMETER_STORE_NOT_FOUND: return "not-found";
    case MBLINK_PARAMETER_STORE_DEFINITION_MISMATCH:
        return "definition-mismatch";
    }
    return "unknown";
}

void mblink_parameter_store_init(MblinkParameterStore *store)
{
    if (store != NULL) {
        memset(store, 0, sizeof(*store));
    }
}

void mblink_parameter_store_clear_samples(MblinkParameterStore *store)
{
    size_t index;

    if (store == NULL) {
        return;
    }

    for (index = 0U; index < store->slot_count; ++index) {
        memset(&store->slots[index].latest, 0,
               sizeof(store->slots[index].latest));
        store->slots[index].latest_valid = false;
    }
    memset(store->history, 0, sizeof(store->history));
    store->history_head = 0U;
    store->history_count = 0U;
    store->total_sample_count = 0U;
}

MblinkParameterStoreResult mblink_parameter_store_register(
    MblinkParameterStore *store,
    const MblinkParameterDefinition *definition)
{
    MblinkParameterStoreSlot slot;

    if (store == NULL || !mblink_parameter_definition_is_valid(definition)) {
        return MBLINK_PARAMETER_STORE_INVALID_ARGUMENT;
    }
    if (mblink_parameter_store_find_key(store, &definition->key) != SIZE_MAX) {
        return MBLINK_PARAMETER_STORE_DUPLICATE_KEY;
    }
    if (mblink_parameter_store_find_stable_key(
            store, definition->stable_key) != SIZE_MAX) {
        return MBLINK_PARAMETER_STORE_DUPLICATE_STABLE_KEY;
    }
    if (store->slot_count >= MBLINK_PARAMETER_STORE_DEFINITION_CAPACITY) {
        return MBLINK_PARAMETER_STORE_FULL;
    }

    memset(&slot, 0, sizeof(slot));
    slot.definition = definition;
    store->slots[store->slot_count] = slot;
    store->slot_count++;
    return MBLINK_PARAMETER_STORE_OK;
}

size_t mblink_parameter_store_definition_count(const MblinkParameterStore *store)
{
    return store != NULL ? store->slot_count : 0U;
}

const MblinkParameterDefinition *mblink_parameter_store_definition_at(
    const MblinkParameterStore *store,
    size_t index)
{
    if (store == NULL || index >= store->slot_count) {
        return NULL;
    }
    return store->slots[index].definition;
}

const MblinkParameterDefinition *mblink_parameter_store_definition(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key)
{
    const size_t index = mblink_parameter_store_find_key(store, key);
    return index != SIZE_MAX ? store->slots[index].definition : NULL;
}

const MblinkParameterDefinition *mblink_parameter_store_definition_for_stable_key(
    const MblinkParameterStore *store,
    const char *stable_key)
{
    const size_t index = mblink_parameter_store_find_stable_key(store, stable_key);
    return index != SIZE_MAX ? store->slots[index].definition : NULL;
}

MblinkParameterStoreResult mblink_parameter_store_set_favourite(
    MblinkParameterStore *store,
    const MblinkParameterKey *key,
    bool favourite)
{
    size_t index;

    if (store == NULL || !mblink_parameter_key_is_valid(key)) {
        return MBLINK_PARAMETER_STORE_INVALID_ARGUMENT;
    }
    index = mblink_parameter_store_find_key(store, key);
    if (index == SIZE_MAX) {
        return MBLINK_PARAMETER_STORE_NOT_FOUND;
    }
    store->slots[index].favourite = favourite;
    return MBLINK_PARAMETER_STORE_OK;
}

bool mblink_parameter_store_is_favourite(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key)
{
    const size_t index = mblink_parameter_store_find_key(store, key);
    return index != SIZE_MAX && store->slots[index].favourite;
}

MblinkParameterStoreResult mblink_parameter_store_record(
    MblinkParameterStore *store,
    const MblinkParameterSample *sample)
{
    size_t slot_index;
    size_t history_index;
    MblinkParameterSample copied;

    if (store == NULL || !mblink_parameter_sample_is_valid(sample)) {
        return MBLINK_PARAMETER_STORE_INVALID_ARGUMENT;
    }

    slot_index = mblink_parameter_store_find_key(
        store, &sample->definition->key);
    if (slot_index == SIZE_MAX) {
        return MBLINK_PARAMETER_STORE_NOT_FOUND;
    }
    if (store->slots[slot_index].definition != sample->definition) {
        return MBLINK_PARAMETER_STORE_DEFINITION_MISMATCH;
    }

    copied = *sample;
    store->slots[slot_index].latest = copied;
    store->slots[slot_index].latest_valid = true;

    if (store->history_count < MBLINK_PARAMETER_STORE_HISTORY_CAPACITY) {
        history_index = (store->history_head + store->history_count) %
                        MBLINK_PARAMETER_STORE_HISTORY_CAPACITY;
        store->history_count++;
    } else {
        history_index = store->history_head;
        store->history_head = (store->history_head + 1U) %
                              MBLINK_PARAMETER_STORE_HISTORY_CAPACITY;
    }
    store->history[history_index] = copied;
    store->total_sample_count = infiltratr_u64_add_saturating(
        store->total_sample_count, 1U);
    return MBLINK_PARAMETER_STORE_OK;
}

bool mblink_parameter_store_latest(
    const MblinkParameterStore *store,
    const MblinkParameterKey *key,
    MblinkParameterSample *sample)
{
    size_t index;

    if (store == NULL || sample == NULL ||
        !mblink_parameter_key_is_valid(key)) {
        return false;
    }
    index = mblink_parameter_store_find_key(store, key);
    if (index == SIZE_MAX || !store->slots[index].latest_valid) {
        return false;
    }
    *sample = store->slots[index].latest;
    return true;
}

size_t mblink_parameter_store_history_count(const MblinkParameterStore *store)
{
    return store != NULL ? store->history_count : 0U;
}

uint64_t mblink_parameter_store_total_sample_count(const MblinkParameterStore *store)
{
    return store != NULL ? store->total_sample_count : 0U;
}

bool mblink_parameter_store_history_at(
    const MblinkParameterStore *store,
    size_t chronological_index,
    MblinkParameterSample *sample)
{
    size_t physical_index;

    if (store == NULL || sample == NULL ||
        chronological_index >= store->history_count) {
        return false;
    }

    physical_index = (store->history_head + chronological_index) %
                     MBLINK_PARAMETER_STORE_HISTORY_CAPACITY;
    *sample = store->history[physical_index];
    return true;
}
