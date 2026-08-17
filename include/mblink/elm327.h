// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327.h
 * @brief Portable ELM327 command and response engine.
 *
 * The ELM327 layer owns serial-style command framing, response accumulation,
 * prompt detection, normalisation, adapter-status classification and the
 * deterministic initialisation sequence. It does not know about BLE, Wi-Fi,
 * sockets or any other concrete transport.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#ifndef MBLINK_ELM327_H
#define MBLINK_ELM327_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ELM327_MAX_COMMAND 64U
#define MBLINK_ELM327_MAX_RESPONSE 4096U
#define MBLINK_ELM327_MAX_ADAPTER_ID 96U

typedef enum {
    MBLINK_ELM327_RESULT_OK = 0,
    MBLINK_ELM327_RESULT_MORE_DATA,
    MBLINK_ELM327_RESULT_INVALID_ARGUMENT,
    MBLINK_ELM327_RESULT_COMMAND_TOO_LONG,
    MBLINK_ELM327_RESULT_RESPONSE_TOO_LONG,
    MBLINK_ELM327_RESULT_NO_DATA,
    MBLINK_ELM327_RESULT_STOPPED,
    MBLINK_ELM327_RESULT_UNABLE_TO_CONNECT,
    MBLINK_ELM327_RESULT_BUS_INIT_ERROR,
    MBLINK_ELM327_RESULT_CAN_ERROR,
    MBLINK_ELM327_RESULT_BUFFER_FULL,
    MBLINK_ELM327_RESULT_UNSUPPORTED_COMMAND,
    MBLINK_ELM327_RESULT_ADAPTER_ERROR,
    MBLINK_ELM327_RESULT_MALFORMED_RESPONSE
} MblinkElm327Result;

typedef struct {
    MblinkElm327Result result;
    bool prompt_seen;
    bool echo_removed;
    bool searching_seen;
    bool ok_seen;
    size_t line_count;
    size_t length;
    char text[MBLINK_ELM327_MAX_RESPONSE];
} MblinkElm327Response;

typedef struct {
    char command[MBLINK_ELM327_MAX_COMMAND];
    uint8_t raw[MBLINK_ELM327_MAX_RESPONSE];
    size_t raw_length;
    bool prompt_seen;
    bool overflowed;
} MblinkElm327Parser;

typedef enum {
    MBLINK_ELM327_INIT_RESET = 0,
    MBLINK_ELM327_INIT_ECHO_OFF,
    MBLINK_ELM327_INIT_LINEFEEDS_OFF,
    MBLINK_ELM327_INIT_SPACES_OFF,
    MBLINK_ELM327_INIT_HEADERS_OFF,
    MBLINK_ELM327_INIT_PROTOCOL_AUTO,
    MBLINK_ELM327_INIT_IDENTIFY,
    MBLINK_ELM327_INIT_COMPLETE,
    MBLINK_ELM327_INIT_FAILED
} MblinkElm327InitStage;

typedef struct {
    MblinkElm327InitStage stage;
    MblinkElm327Result failure;
    char adapter_id[MBLINK_ELM327_MAX_ADAPTER_ID];
} MblinkElm327InitState;

/** Return a stable human-readable name for an ELM327 result. */
const char *mblink_elm327_result_name(MblinkElm327Result result);

/**
 * Validate and frame an ELM327/OBD command for transmission.
 *
 * Leading/trailing ASCII whitespace is removed and exactly one carriage
 * return is appended. Embedded CR/LF bytes, the ELM prompt character and
 * non-printable bytes are rejected.
 */
MblinkElm327Result mblink_elm327_build_command(const char *command,
                                                uint8_t *buffer,
                                                size_t buffer_size,
                                                size_t *written);

/** Initialise a response accumulator for one outstanding command. */
MblinkElm327Result mblink_elm327_parser_begin(MblinkElm327Parser *parser,
                                               const char *command);

/**
 * Feed arbitrarily fragmented transport bytes into the parser.
 *
 * Returns MORE_DATA until a '>' prompt is seen, OK when a complete response
 * has been accumulated, or RESPONSE_TOO_LONG if the bounded buffer overflows.
 * `consumed` reports bytes consumed from this fragment; bytes after the prompt
 * remain the caller's responsibility and are never silently discarded.
 */
MblinkElm327Result mblink_elm327_parser_feed(MblinkElm327Parser *parser,
                                              const uint8_t *data,
                                              size_t size,
                                              size_t *consumed);

/** Normalise and classify a complete accumulated ELM327 response. */
MblinkElm327Result mblink_elm327_parser_finish(const MblinkElm327Parser *parser,
                                                MblinkElm327Response *response);

/** Initialise the deterministic adapter setup state machine. */
void mblink_elm327_init_begin(MblinkElm327InitState *state);

/** Return the command required by the current initialisation stage. */
const char *mblink_elm327_init_command(const MblinkElm327InitState *state);

/**
 * Advance initialisation after receiving a parsed response.
 *
 * Reset/identity responses may contain text; configuration steps require OK.
 * Any classified adapter error moves the state to FAILED.
 */
MblinkElm327Result mblink_elm327_init_accept(MblinkElm327InitState *state,
                                              const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
