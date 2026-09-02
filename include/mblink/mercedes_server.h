// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file mercedes_server.h
 * @brief Mercedes-Benz ECU/server application layer over LINK UDS server.
 *
 * LINK owns CAN/CAN-FD, ISO-TP, UDS dispatch and STM32 transport. MBLINK owns
 * Mercedes identity/profile selection and the application data presented by
 * the ECU/server.
 */
#ifndef MBLINK_MERCEDES_SERVER_H
#define MBLINK_MERCEDES_SERVER_H

#include "mblink/mercedes.h"
#include "mblink/uds_dtc.h"
#include "link/uds_server.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_MERCEDES_SERVER_VIN_DID UINT16_C(0xf190)

typedef bool (*MblinkMercedesServerDidReadFn)(
    void *context,
    uint16_t identifier,
    uint8_t *data,
    size_t capacity,
    size_t *data_length);

typedef struct {
    const char *vin;
    MblinkMercedesModuleKind module;
    const char *endpoint_key;
    const MblinkUdsDtcRecord *dtcs;
    size_t dtc_count;

    /*
     * Optional rich ISO 14229 DTC metadata. The compact dtcs[] list remains
     * the authoritative DTC/status set; details supplies the additional
     * snapshot, extended-data, severity, memory and WWH-OBD fields required
     * to answer the complete ReadDTCInformation surface truthfully.
     */
    const LinkUdsServerDtcDetail *dtc_details;
    size_t dtc_detail_count;
    uint8_t wwh_dtc_format_identifier;

    MblinkMercedesServerDidReadFn read_did;
    void *did_context;
} MblinkMercedesServerConfig;

#define MBLINK_MERCEDES_SERVER_CONFIG_INIT \
    { NULL, MBLINK_MERCEDES_MODULE_ENGINE, NULL, NULL, 0U, \
      NULL, 0U, UINT8_C(0x04), NULL, NULL }

typedef struct {
    char vin[MBLINK_MERCEDES_VIN_LENGTH + 1U];
    MblinkMercedesVinDecode decoded_vin;
    const MblinkMercedesVehicleProfile *profile;
    const MblinkMercedesEcuEndpointDefinition *endpoint;
    MblinkMercedesModuleKind module;
    LinkUdsServerDtcStore dtc_store;
    MblinkMercedesServerDidReadFn read_did;
    void *did_context;
} MblinkMercedesServerState;

bool mblink_mercedes_server_init(
    MblinkMercedesServerState *state,
    const MblinkMercedesServerConfig *config);

bool mblink_mercedes_server_bind(
    LinkUdsServer *server,
    MblinkMercedesServerState *state);

LinkUdsServerHandlerResult mblink_mercedes_server_read_did_handler(
    void *context,
    const LinkUdsServerRequest *request,
    uint8_t *response_data,
    size_t response_data_capacity);

LinkUdsServerHandlerResult mblink_mercedes_server_read_dtc_handler(
    void *context,
    const LinkUdsServerRequest *request,
    uint8_t *response_data,
    size_t response_data_capacity);

#ifdef __cplusplus
}
#endif

#endif
