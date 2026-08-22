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

#define mblink_parameter_protocol_name link_parameter_protocol_name
#define mblink_parameter_key_is_valid link_parameter_key_is_valid
#define mblink_parameter_key_equal link_parameter_key_equal
#define mblink_parameter_definition_is_valid link_parameter_definition_is_valid
#define mblink_parameter_sample_is_valid link_parameter_sample_is_valid
#define mblink_parameter_format_value link_parameter_format_value
#define mblink_parameter_format_sample link_parameter_format_sample
#define mblink_parameter_obd2_definition_count link_parameter_obd2_definition_count
#define mblink_parameter_obd2_definition_at link_parameter_obd2_definition_at
#define mblink_parameter_obd2_definition link_parameter_obd2_definition
#define mblink_parameter_obd2_definition_for_stable_key link_parameter_obd2_definition_for_stable_key

bool mblink_parameter_from_obd2(const MblinkObd2Sample *sample,
                                uint64_t timestamp_ms,
                                MblinkParameterSample *parameter);

#ifdef __cplusplus
}
#endif
#endif
