// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/parameter.h"

const char *mblink_parameter_protocol_name(MblinkParameterProtocol protocol)
{
    return link_parameter_protocol_name(protocol);
}

bool mblink_parameter_key_is_valid(const MblinkParameterKey *key)
{
    return link_parameter_key_is_valid(key);
}

bool mblink_parameter_key_equal(const MblinkParameterKey *left,
                                const MblinkParameterKey *right)
{
    return link_parameter_key_equal(left, right);
}

bool mblink_parameter_definition_is_valid(
    const MblinkParameterDefinition *definition)
{
    return link_parameter_definition_is_valid(definition);
}

bool mblink_parameter_sample_is_valid(const MblinkParameterSample *sample)
{
    return link_parameter_sample_is_valid(sample);
}

bool mblink_parameter_format_value(
    const MblinkParameterDefinition *definition,
    bool available,
    double value,
    char *buffer,
    size_t buffer_size)
{
    return link_parameter_format_value(
        definition, available, value, buffer, buffer_size);
}

bool mblink_parameter_format_sample(
    const MblinkParameterSample *sample,
    char *buffer,
    size_t buffer_size)
{
    return link_parameter_format_sample(sample, buffer, buffer_size);
}

size_t mblink_parameter_obd2_definition_count(void)
{
    return link_parameter_obd2_definition_count();
}

const MblinkParameterDefinition *mblink_parameter_obd2_definition_at(size_t index)
{
    return link_parameter_obd2_definition_at(index);
}

const MblinkParameterDefinition *mblink_parameter_obd2_definition(uint8_t pid)
{
    return link_parameter_obd2_definition(pid);
}

const MblinkParameterDefinition *mblink_parameter_obd2_definition_for_stable_key(
    const char *stable_key)
{
    return link_parameter_obd2_definition_for_stable_key(stable_key);
}

bool mblink_parameter_from_obd2(const MblinkObd2Sample *sample,
                                uint64_t timestamp_ms,
                                MblinkParameterSample *parameter)
{
    return link_parameter_from_obd2(sample, timestamp_ms, parameter);
}
