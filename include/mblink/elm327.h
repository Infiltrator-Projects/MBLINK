// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327.h
 * @brief MBLINK compatibility facade for LINK's shared ELM327 engine.
 *
 * The parser, framing and initialisation algorithms live in LINK.  This header
 * preserves MBLINK's public vocabulary and ABI while ensuring Mercedes-only
 * code remains above the shared automotive layer.
 */
#ifndef MBLINK_ELM327_H
#define MBLINK_ELM327_H

#include "link/elm327.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ELM327_MAX_COMMAND LINK_ELM327_MAX_COMMAND
#define MBLINK_ELM327_MAX_RESPONSE LINK_ELM327_MAX_RESPONSE
#define MBLINK_ELM327_MAX_ADAPTER_ID LINK_ELM327_MAX_ADAPTER_ID

#define MBLINK_ELM327_RESULT_OK LINK_ELM327_RESULT_OK
#define MBLINK_ELM327_RESULT_MORE_DATA LINK_ELM327_RESULT_MORE_DATA
#define MBLINK_ELM327_RESULT_INVALID_ARGUMENT LINK_ELM327_RESULT_INVALID_ARGUMENT
#define MBLINK_ELM327_RESULT_COMMAND_TOO_LONG LINK_ELM327_RESULT_COMMAND_TOO_LONG
#define MBLINK_ELM327_RESULT_RESPONSE_TOO_LONG LINK_ELM327_RESULT_RESPONSE_TOO_LONG
#define MBLINK_ELM327_RESULT_NO_DATA LINK_ELM327_RESULT_NO_DATA
#define MBLINK_ELM327_RESULT_STOPPED LINK_ELM327_RESULT_STOPPED
#define MBLINK_ELM327_RESULT_UNABLE_TO_CONNECT LINK_ELM327_RESULT_UNABLE_TO_CONNECT
#define MBLINK_ELM327_RESULT_BUS_INIT_ERROR LINK_ELM327_RESULT_BUS_INIT_ERROR
#define MBLINK_ELM327_RESULT_CAN_ERROR LINK_ELM327_RESULT_CAN_ERROR
#define MBLINK_ELM327_RESULT_BUFFER_FULL LINK_ELM327_RESULT_BUFFER_FULL
#define MBLINK_ELM327_RESULT_UNSUPPORTED_COMMAND LINK_ELM327_RESULT_UNSUPPORTED_COMMAND
#define MBLINK_ELM327_RESULT_ADAPTER_ERROR LINK_ELM327_RESULT_ADAPTER_ERROR
#define MBLINK_ELM327_RESULT_MALFORMED_RESPONSE LINK_ELM327_RESULT_MALFORMED_RESPONSE

typedef LinkElm327Result MblinkElm327Result;
typedef LinkElm327Response MblinkElm327Response;
typedef LinkElm327Parser MblinkElm327Parser;

#define MBLINK_ELM327_INIT_RESET LINK_ELM327_INIT_RESET
#define MBLINK_ELM327_INIT_ECHO_OFF LINK_ELM327_INIT_ECHO_OFF
#define MBLINK_ELM327_INIT_LINEFEEDS_OFF LINK_ELM327_INIT_LINEFEEDS_OFF
#define MBLINK_ELM327_INIT_SPACES_OFF LINK_ELM327_INIT_SPACES_OFF
#define MBLINK_ELM327_INIT_HEADERS_OFF LINK_ELM327_INIT_HEADERS_OFF
#define MBLINK_ELM327_INIT_PROTOCOL_AUTO LINK_ELM327_INIT_PROTOCOL_AUTO
#define MBLINK_ELM327_INIT_IDENTIFY LINK_ELM327_INIT_IDENTIFY
#define MBLINK_ELM327_INIT_COMPLETE LINK_ELM327_INIT_COMPLETE
#define MBLINK_ELM327_INIT_FAILED LINK_ELM327_INIT_FAILED

typedef LinkElm327InitStage MblinkElm327InitStage;
typedef LinkElm327InitState MblinkElm327InitState;

const char *mblink_elm327_result_name(MblinkElm327Result result);
MblinkElm327Result mblink_elm327_build_command(const char *command,
                                               uint8_t *buffer,
                                               size_t buffer_size,
                                               size_t *written);
MblinkElm327Result mblink_elm327_parser_begin(MblinkElm327Parser *parser,
                                              const char *command);
MblinkElm327Result mblink_elm327_parser_feed(MblinkElm327Parser *parser,
                                             const uint8_t *data,
                                             size_t size,
                                             size_t *consumed);
MblinkElm327Result mblink_elm327_parser_finish(const MblinkElm327Parser *parser,
                                               MblinkElm327Response *response);
void mblink_elm327_init_begin(MblinkElm327InitState *state);
const char *mblink_elm327_init_command(const MblinkElm327InitState *state);
MblinkElm327Result mblink_elm327_init_accept(MblinkElm327InitState *state,
                                             const MblinkElm327Response *response);

#ifdef __cplusplus
}
#endif

#endif
