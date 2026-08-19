// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file parameter.c
 * @brief Protocol-neutral diagnostic parameter metadata and formatting.
 */
#include "mblink/parameter.h"

#include "infiltratr/core.h"
#include "infiltratr/format.h"

#include <math.h>
#include <stdint.h>

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
    if (definition == NULL || definition->stable_key == NULL ||
        definition->stable_key[0] == '\0' || definition->short_name == NULL ||
        definition->short_name[0] == '\0' || definition->name == NULL ||
        definition->name[0] == '\0' || definition->suffix == NULL ||
        definition->decimal_places > 9U ||
        definition->key.protocol < MBLINK_PARAMETER_PROTOCOL_OBD2 ||
        definition->key.protocol > MBLINK_PARAMETER_PROTOCOL_UDS) {
        return false;
    }

    if (definition->key.protocol == MBLINK_PARAMETER_PROTOCOL_OBD2 &&
        (definition->key.module != MBLINK_PARAMETER_MODULE_STANDARD_OBD2 ||
         definition->key.identifier > UINT8_MAX)) {
        return false;
    }
    if (definition->key.protocol == MBLINK_PARAMETER_PROTOCOL_UDS &&
        definition->key.identifier > UINT16_MAX) {
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
