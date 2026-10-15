// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_connection_lifecycle.h"

#include "mblink/elm327_can.h"
#include "mblink/mercedes_ecu_pack.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>

void mblink_mercedes_connection_lifecycle_init(
    MblinkMercedesConnectionLifecycle *lifecycle)
{
    if (lifecycle == NULL) return;
    *lifecycle = (MblinkMercedesConnectionLifecycle){0};
}

void mblink_mercedes_connection_lifecycle_begin(
    MblinkMercedesConnectionLifecycle *lifecycle,
    MblinkMercedesConnectionSessionMode mode)
{
    if (lifecycle == NULL) return;
    lifecycle->mode = mode;
    lifecycle->stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;
    lifecycle->module_index = 0U;
}

MblinkMercedesConnectionSessionMode mblink_mercedes_connection_lifecycle_finish(
    MblinkMercedesConnectionLifecycle *lifecycle)
{
    if (lifecycle == NULL)
        return MBLINK_MERCEDES_CONNECTION_SESSION_NONE;
    const MblinkMercedesConnectionSessionMode finished = lifecycle->mode;
    lifecycle->mode = MBLINK_MERCEDES_CONNECTION_SESSION_NONE;
    lifecycle->stage = MBLINK_MERCEDES_CONNECTION_SESSION_STAGE_NONE;
    lifecycle->module_index = 0U;
    return finished;
}

bool mblink_mercedes_connection_lifecycle_active(
    const MblinkMercedesConnectionLifecycle *lifecycle)
{
    return lifecycle != NULL &&
        lifecycle->mode != MBLINK_MERCEDES_CONNECTION_SESSION_NONE;
}

static bool simple_diagnostic_control_command(
    const char *source,
    char command[5])
{
    if (source == NULL || command == NULL) return false;
    if (source[0] == '0' && (source[1] == 'x' || source[1] == 'X'))
        source += 2;
    if (strlen(source) != 4U) return false;
    for (size_t index = 0U; index < 4U; ++index) {
        const unsigned char value = (unsigned char)source[index];
        if (!isxdigit(value)) return false;
        command[index] = (char)toupper(value);
    }
    command[4] = '\0';
    return true;
}

bool mblink_mercedes_connection_session_commands(
    const MblinkMercedesModuleScanEntry *module,
    char enter_command[5],
    char quit_command[5])
{
    MblinkMercedesEcuPack pack;
    if (module == NULL ||
        !mblink_mercedes_ecu_pack_resolve_module(module, &pack) ||
        pack.observed_protocol_conflict ||
        !simple_diagnostic_control_command(
            pack.session_command, enter_command) ||
        !simple_diagnostic_control_command(
            pack.quit_command, quit_command)) {
        return false;
    }

    /* Field evidence shows that automatic ESP extended-session entry can
     * affect steering behaviour while moving. Keep that safety exception in
     * the Mercedes policy layer, not in the Apple coordinator. */
    if (module->definition != NULL &&
        module->definition->key != NULL &&
        strcmp(module->definition->key, "esp") == 0) {
        return false;
    }
    return true;
}

bool mblink_mercedes_connection_session_response_positive(
    const MblinkElm327Response *response)
{
    uint8_t pdu[MBLINK_MERCEDES_MODULE_SCAN_PDU_CAPACITY];
    size_t length = 0U;
    return response != NULL &&
        response->result == MBLINK_ELM327_RESULT_OK &&
        mblink_elm327_can_decode_pdu(
            response, pdu, sizeof(pdu), &length) ==
            MBLINK_ELM327_CAN_RESULT_OK &&
        length >= 2U && pdu[0] == UINT8_C(0x50);
}
