// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_PARAMETER_H
#define MBLINK_PARAMETER_H

#include "mblink/obd2.h"
#include "link/parameter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef LinkParameterProtocol MblinkParameterProtocol;
typedef LinkParameterKey MblinkParameterKey;
typedef LinkParameterDefinition MblinkParameterDefinition;
typedef LinkParameterSample MblinkParameterSample;

#define MBLINK_PARAMETER_MODULE_STANDARD_OBD2 LINK_PARAMETER_MODULE_STANDARD_OBD2
#define MBLINK_PARAMETER_PROTOCOL_OBD2 LINK_PARAMETER_PROTOCOL_OBD2
#define MBLINK_PARAMETER_PROTOCOL_UDS LINK_PARAMETER_PROTOCOL_UDS

const char *mblink_parameter_protocol_name(MblinkParameterProtocol protocol);
bool mblink_parameter_key_is_valid(const MblinkParameterKey *key);
bool mblink_parameter_key_equal(const MblinkParameterKey *left,
                                const MblinkParameterKey *right);
bool mblink_parameter_definition_is_valid(
    const MblinkParameterDefinition *definition);
bool mblink_parameter_sample_is_valid(const MblinkParameterSample *sample);
bool mblink_parameter_format_value(
    const MblinkParameterDefinition *definition,
    bool available,
    double value,
    char *buffer,
    size_t buffer_size);
bool mblink_parameter_format_sample(
    const MblinkParameterSample *sample,
    char *buffer,
    size_t buffer_size);
size_t mblink_parameter_obd2_definition_count(void);
const MblinkParameterDefinition *mblink_parameter_obd2_definition_at(size_t index);
const MblinkParameterDefinition *mblink_parameter_obd2_definition(uint8_t pid);
const MblinkParameterDefinition *mblink_parameter_obd2_definition_for_stable_key(
    const char *stable_key);
bool mblink_parameter_from_obd2(const MblinkObd2Sample *sample,
                                uint64_t timestamp_ms,
                                MblinkParameterSample *parameter);

#ifdef __cplusplus
}
#endif
#endif
