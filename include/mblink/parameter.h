// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file parameter.h
 * @brief Protocol-neutral live diagnostic parameter contracts.
 *
 * This layer gives OBD-II and manufacturer UDS data one stable C-facing model
 * for live-data, table, dashboard and graph presentation. Protocol decoding
 * remains in its owning layer; this module owns only parameter identity,
 * metadata and presentation-safe scalar formatting.
 */
#ifndef MBLINK_PARAMETER_H
#define MBLINK_PARAMETER_H

#include "mblink/obd2.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_PARAMETER_MODULE_STANDARD_OBD2 0U

typedef enum {
    MBLINK_PARAMETER_PROTOCOL_OBD2 = 0,
    MBLINK_PARAMETER_PROTOCOL_UDS
} MblinkParameterProtocol;

/** Stable machine identity for one diagnostic parameter. */
typedef struct {
    MblinkParameterProtocol protocol;
    uint32_t module;
    uint32_t identifier;
} MblinkParameterKey;

/** Portable metadata for one scalar diagnostic parameter. */
typedef struct {
    MblinkParameterKey key;
    const char *stable_key;
    const char *name;
    const char *suffix;
    unsigned int decimal_places;
    bool clamp;
    double minimum;
    double maximum;
} MblinkParameterDefinition;

typedef struct {
    const MblinkParameterDefinition *definition;
    uint64_t timestamp_ms;
    bool available;
    double value;
} MblinkParameterSample;

const char *mblink_parameter_protocol_name(MblinkParameterProtocol protocol);
bool mblink_parameter_key_equal(const MblinkParameterKey *left,
                                const MblinkParameterKey *right);
bool mblink_parameter_definition_is_valid(
    const MblinkParameterDefinition *definition);
bool mblink_parameter_sample_is_valid(const MblinkParameterSample *sample);

/** Format a scalar through Infiltratr Common's canonical scalar formatter. */
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

/** Standard OBD-II descriptors currently used by the live workspace. */
size_t mblink_parameter_obd2_definition_count(void);
const MblinkParameterDefinition *mblink_parameter_obd2_definition_at(
    size_t index);
const MblinkParameterDefinition *mblink_parameter_obd2_definition(
    uint8_t pid);

/** Convert one decoded OBD-II scalar into the shared parameter model. */
bool mblink_parameter_from_obd2(
    const MblinkObd2Sample *sample,
    uint64_t timestamp_ms,
    MblinkParameterSample *parameter);

#ifdef __cplusplus
}
#endif

#endif
