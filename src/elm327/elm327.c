// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file elm327.c
 * @brief MBLINK ABI wrappers for LINK's shared ELM327 core.
 *
 * No protocol algorithm belongs here.  Each function forwards to LINK so
 * existing MBLINK callers keep stable symbol names while one implementation is
 * maintained and tested in the shared automotive layer.
 */
#include "mblink/elm327.h"

const char *mblink_elm327_result_name(MblinkElm327Result result)
{
    return link_elm327_result_name(result);
}

MblinkElm327Result mblink_elm327_build_command(const char *command,
                                               uint8_t *buffer,
                                               size_t buffer_size,
                                               size_t *written)
{
    return link_elm327_build_command(command, buffer, buffer_size, written);
}

MblinkElm327Result mblink_elm327_parser_begin(MblinkElm327Parser *parser,
                                              const char *command)
{
    return link_elm327_parser_begin(parser, command);
}

MblinkElm327Result mblink_elm327_parser_feed(MblinkElm327Parser *parser,
                                             const uint8_t *data,
                                             size_t size,
                                             size_t *consumed)
{
    return link_elm327_parser_feed(parser, data, size, consumed);
}

MblinkElm327Result mblink_elm327_parser_finish(const MblinkElm327Parser *parser,
                                               MblinkElm327Response *response)
{
    return link_elm327_parser_finish(parser, response);
}

void mblink_elm327_init_begin(MblinkElm327InitState *state)
{
    link_elm327_init_begin(state);
}

const char *mblink_elm327_init_command(const MblinkElm327InitState *state)
{
    return link_elm327_init_command(state);
}

MblinkElm327Result mblink_elm327_init_accept(MblinkElm327InitState *state,
                                             const MblinkElm327Response *response)
{
    return link_elm327_init_accept(state, response);
}
