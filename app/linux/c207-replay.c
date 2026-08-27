// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file c207-replay.c
 * @brief Deterministic offline Vgate/ELM327 replay for the captured C207.
 *
 * Real capture-derived responses are kept distinct from the one deliberately
 * synthetic restraint-controller injection used to prove the Linux UI path.
 * The real vehicle VIN is not embedded in the repository; the replay preserves
 * its C207/OM651 VIN shape with a synthetic serial suffix.
 */
#include "c207-replay.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MblinkC207ReplayTransport *provider_replay(void *context)
{
    return (MblinkC207ReplayTransport *)context;
}

static bool replay_queue(MblinkC207ReplayTransport *replay, const char *text)
{
    size_t length;
    if (replay == NULL || text == NULL) return false;
    if (replay->pending_length != 0U) return false;
    length = strlen(text);
    if (length >= sizeof(replay->pending)) return false;
    memcpy(replay->pending, text, length + 1U);
    replay->pending_length = length;
    return true;
}

static void normalize_command(
    const uint8_t *data,
    size_t size,
    char *command,
    size_t capacity)
{
    size_t used = 0U;
    size_t start = 0U;
    size_t end = size;

    if (command == NULL || capacity == 0U) return;
    command[0] = '\0';
    if (data == NULL || size == 0U) return;

    while (start < end && isspace((unsigned char)data[start])) ++start;
    while (end > start && isspace((unsigned char)data[end - 1U])) --end;
    while (start < end && used + 1U < capacity) {
        const unsigned char value = data[start++];
        command[used++] = (char)toupper(value);
    }
    command[used] = '\0';
}

static bool parse_hex_id(const char *text, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed;
    if (text == NULL || value == NULL || text[0] == '\0') return false;
    parsed = strtoul(text, &end, 16);
    if (end == text || *end != '\0' || parsed > UINT32_MAX) return false;
    *value = (uint32_t)parsed;
    return true;
}

static const char *replay_obd_response(const char *command)
{
    if (strcmp(command, "0100") == 0)
        return "410098180001\r4100983BA013\r>";
    if (strcmp(command, "0120") == 0)
        return "4120B003A005\r412080018001\r>";
    if (strcmp(command, "0140") == 0)
        return "41404CD80000\r414040800000\r>";

    /*
     * No standard VIN/DTC/live sample reply was present in the captured
     * session.  Returning NO DATA here is intentionally conservative; the
     * Mercedes F190 VIN and factory DTC paths are replayed below.
     */
    if (strcmp(command, "0902") == 0 ||
        strcmp(command, "03") == 0 ||
        strcmp(command, "07") == 0 ||
        strcmp(command, "0A") == 0)
        return "NO DATA\r>";
    if (command[0] == '0' && command[1] == '1')
        return "NO DATA\r>";
    return NULL;
}

static const char *replay_uds_response(
    const MblinkC207ReplayTransport *replay,
    const char *command)
{
    if (replay == NULL || command == NULL) return NULL;

    /*
     * Synthetic test-only ORC at 0x602.  It is deliberately early in the
     * forensic sweep so the actual Linux Faults view can show the injected
     * restraint fault while the remainder of the replay continues.
     */
    if (replay->tx_can_id == UINT32_C(0x602)) {
        if (strcmp(command, "3E00") == 0)
            return replay->headers_enabled ? "60A027E00\r>" : "7E00\r>";
        if (strcmp(command, "1902FF") == 0)
            return "7F1978\r5902FFB0001309\r>";
        if (strcmp(command, "22F197") == 0)
            return "62F1974F52435F323132\r>";
        if (strcmp(command, "22F187") == 0 ||
            strcmp(command, "22F188") == 0 ||
            strcmp(command, "22F191") == 0)
            return "NO DATA\r>";
    }

    /* Real engine-ECU response shapes captured at 0x7E0 -> 0x7E8. */
    if (replay->tx_can_id == UINT32_C(0x7e0)) {
        if (strcmp(command, "3E00") == 0)
            return replay->headers_enabled ? "7E8037E00\r>" : "7E00\r>";
        if (strcmp(command, "22F190") == 0)
            return "014\r0:62F190574444\r1:3230373330333246\r2:313233343536\r>";
        if (strcmp(command, "22F18C") == 0)
            return "00B\r0:62F18C333134\r1:3932333333FFFF\r>";
        if (strcmp(command, "22F187") == 0 ||
            strcmp(command, "22F188") == 0 ||
            strcmp(command, "22F189") == 0 ||
            strcmp(command, "22F191") == 0 ||
            strcmp(command, "22F197") == 0 ||
            strcmp(command, "221001") == 0 ||
            strcmp(command, "221002") == 0)
            return "7F2231\r>";
        if (strcmp(command, "22F100") == 0)
            return "62F10002110F01\r>";
        if (strcmp(command, "22F154") == 0)
            return "62F1540040\r>";
        if (strcmp(command, "22F196") == 0)
            return "009\r0:62F196454430\r1:353037FFFFFFFF\r>";
        if (strcmp(command, "1902FF") == 0)
            return "7F1978\r5902FF\r>";
    }

    /*
     * Real second diagnostic endpoint captured at 0x7E1 -> 0x7E9.
     * 7F 3E 12 is a valid UDS negative response and therefore presence
     * evidence, not a missing-module result.
     */
    if (replay->tx_can_id == UINT32_C(0x7e1)) {
        if (strcmp(command, "3E00") == 0)
            return replay->headers_enabled ? "7E9037F3E12\r>" : "7F3E12\r>";
        if (strcmp(command, "1902FF") == 0 ||
            strcmp(command, "22F190") == 0 ||
            strcmp(command, "22F197") == 0 ||
            strcmp(command, "22F187") == 0 ||
            strcmp(command, "22F188") == 0 ||
            strcmp(command, "22F191") == 0)
            return "NO DATA\r>";
    }

    /*
     * The real deep sweep produced long runs of NO DATA at 0x600 upward.
     * Every other unproven target stays silent in the offline replay.
     */
    if (strcmp(command, "3E00") == 0 ||
        strcmp(command, "1902FF") == 0 ||
        strncmp(command, "22", 2U) == 0)
        return "NO DATA\r>";

    return NULL;
}

static LinkTransportStatus replay_connect(void *context)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    if (replay == NULL) return LINK_TRANSPORT_INVALID_ARGUMENT;
    replay->connected = true;
    return LINK_TRANSPORT_OK;
}

static void replay_disconnect(void *context)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    if (replay == NULL) return;
    replay->connected = false;
    replay->pending_length = 0U;
}

static bool replay_is_connected(void *context)
{
    const MblinkC207ReplayTransport *replay =
        (const MblinkC207ReplayTransport *)context;
    return replay != NULL && replay->connected;
}

static LinkTransportStatus replay_write(
    void *context,
    const uint8_t *data,
    size_t size)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    char command[96];
    const char *response = NULL;

    if (replay == NULL || data == NULL || size == 0U)
        return LINK_TRANSPORT_INVALID_ARGUMENT;
    if (!replay->connected) return LINK_TRANSPORT_NOT_CONNECTED;
    if (replay->pending_length != 0U) return LINK_TRANSPORT_BUSY;

    normalize_command(data, size, command, sizeof(command));
    if (command[0] == '\0') return LINK_TRANSPORT_INVALID_ARGUMENT;
    replay->command_count++;

    if (strcmp(command, "ATZ") == 0 || strcmp(command, "ATI") == 0) {
        response = "ELM327 v2.3\r>";
    } else if (strncmp(command, "ATSH", 4U) == 0) {
        if (!parse_hex_id(command + 4U, &replay->tx_can_id))
            return LINK_TRANSPORT_IO_ERROR;
        replay->extended_id = strlen(command + 4U) > 3U;
        response = "OK\r>";
    } else if (strcmp(command, "ATH0") == 0 || strcmp(command, "ATH1") == 0) {
        replay->headers_enabled = command[3] == '1';
        response = "OK\r>";
    } else if (strcmp(command, "ATCRA") == 0) {
        replay->rx_can_id = 0U;
        response = "OK\r>";
    } else if (strncmp(command, "ATCRA", 5U) == 0) {
        if (!parse_hex_id(command + 5U, &replay->rx_can_id))
            return LINK_TRANSPORT_IO_ERROR;
        response = "OK\r>";
    } else if (strcmp(command, "ATDP") == 0) {
        response = "ISO 15765-4 (CAN 11/500)\r>";
    } else if (strcmp(command, "ATDPN") == 0) {
        response = "A6\r>";
    } else if (strncmp(command, "AT", 2U) == 0) {
        response = "OK\r>";
    } else {
        response = replay_obd_response(command);
        if (response == NULL)
            response = replay_uds_response(replay, command);
    }

    if (response == NULL) response = "NO DATA\r>";
    return replay_queue(replay, response)
        ? LINK_TRANSPORT_OK : LINK_TRANSPORT_IO_ERROR;
}

static void replay_set_receiver(
    void *context,
    LinkTransportReceiveFn receiver,
    void *receiver_context)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    if (replay == NULL) return;
    replay->receiver = receiver;
    replay->receiver_context = receiver_context;
}

static size_t replay_discover(
    char paths[][256],
    size_t capacity,
    void *context)
{
    (void)context;
    if (paths == NULL || capacity == 0U) return 0U;
    (void)snprintf(paths[0], 256U, "%s", MBLINK_C207_REPLAY_DEVICE);
    return 1U;
}

static bool replay_configure(
    const char *device,
    unsigned int baud_rate,
    LinkTransport *transport,
    void *context)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    (void)baud_rate;
    if (replay == NULL || transport == NULL || device == NULL ||
        strcmp(device, MBLINK_C207_REPLAY_DEVICE) != 0) {
        return false;
    }

    *transport = (LinkTransport)LINK_TRANSPORT_INIT;
    transport->context = replay;
    transport->connect = replay_connect;
    transport->disconnect = replay_disconnect;
    transport->is_connected = replay_is_connected;
    transport->write = replay_write;
    transport->set_receiver = replay_set_receiver;
    return true;
}

static bool replay_probe_elm327(
    char *identity,
    size_t identity_capacity,
    void *context)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    int written;
    if (replay == NULL || !replay->connected ||
        identity == NULL || identity_capacity == 0U) {
        return false;
    }
    written = snprintf(identity, identity_capacity,
                       "Vgate iCar Pro replay · ELM327 v2.3");
    return written >= 0 && (size_t)written < identity_capacity;
}

static void replay_pump(void *context)
{
    MblinkC207ReplayTransport *replay = provider_replay(context);
    LinkTransportReceiveFn receiver;
    void *receiver_context;
    size_t length;

    if (replay == NULL || !replay->connected ||
        replay->pending_length == 0U || replay->receiver == NULL) {
        return;
    }

    receiver = replay->receiver;
    receiver_context = replay->receiver_context;
    length = replay->pending_length;
    replay->pending_length = 0U;
    receiver(receiver_context, (const uint8_t *)replay->pending, length);
}

void mblink_c207_replay_init(MblinkC207ReplayTransport *replay)
{
    if (replay == NULL) return;
    memset(replay, 0, sizeof(*replay));
}

const LinkGtkTransportProvider *mblink_c207_replay_provider(void)
{
    static const LinkGtkTransportProvider provider = {
        .discover = replay_discover,
        .configure = replay_configure,
        .probe_elm327 = replay_probe_elm327,
        .pump = replay_pump
    };
    return &provider;
}
