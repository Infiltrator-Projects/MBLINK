// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file isotp.h
 * @brief Portable ISO-TP (ISO 15765-2) transport-layer foundation.
 *
 * Classical-CAN buffers are caller-owned and all protocol state is bounded.
 * Timing values use one caller-supplied monotonic microsecond clock.
 */
#ifndef MBLINK_ISOTP_H
#define MBLINK_ISOTP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH 8U
#define MBLINK_ISOTP_MAX_PDU_LENGTH 4095U

typedef enum {
    MBLINK_ISOTP_ADDRESSING_NORMAL = 0,
    MBLINK_ISOTP_ADDRESSING_EXTENDED,
    MBLINK_ISOTP_ADDRESSING_MIXED
} MblinkIsoTpAddressingMode;

typedef enum {
    MBLINK_ISOTP_TARGET_PHYSICAL = 0,
    MBLINK_ISOTP_TARGET_FUNCTIONAL
} MblinkIsoTpTargetType;

typedef enum {
    MBLINK_ISOTP_FLOW_CONTINUE_TO_SEND = 0,
    MBLINK_ISOTP_FLOW_WAIT = 1,
    MBLINK_ISOTP_FLOW_OVERFLOW = 2
} MblinkIsoTpFlowStatus;

typedef enum {
    MBLINK_ISOTP_RESULT_OK = 0,
    MBLINK_ISOTP_RESULT_COMPLETE,
    MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL,
    MBLINK_ISOTP_RESULT_WAIT_SEPARATION,
    MBLINK_ISOTP_RESULT_FLOW_CONTROL_WAIT,
    MBLINK_ISOTP_RESULT_INVALID_ARGUMENT,
    MBLINK_ISOTP_RESULT_INVALID_FRAME,
    MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME,
    MBLINK_ISOTP_RESULT_WRONG_SEQUENCE,
    MBLINK_ISOTP_RESULT_BUFFER_TOO_SMALL,
    MBLINK_ISOTP_RESULT_PAYLOAD_TOO_LARGE,
    MBLINK_ISOTP_RESULT_TIMEOUT,
    MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW,
    MBLINK_ISOTP_RESULT_WAIT_FRAME_LIMIT,
    MBLINK_ISOTP_RESULT_UNSUPPORTED
} MblinkIsoTpResult;

typedef enum {
    MBLINK_ISOTP_RX_IDLE = 0,
    MBLINK_ISOTP_RX_RECEIVING,
    MBLINK_ISOTP_RX_COMPLETE,
    MBLINK_ISOTP_RX_FAILED
} MblinkIsoTpRxState;

typedef enum {
    MBLINK_ISOTP_TX_IDLE = 0,
    MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL,
    MBLINK_ISOTP_TX_SENDING,
    MBLINK_ISOTP_TX_COMPLETE,
    MBLINK_ISOTP_TX_FAILED
} MblinkIsoTpTxState;

typedef struct {
    uint32_t tx_can_id;
    uint32_t rx_can_id;
    bool tx_extended_id;
    bool rx_extended_id;
    MblinkIsoTpAddressingMode addressing_mode;
    MblinkIsoTpTargetType target_type;
    uint8_t tx_address_extension;
    uint8_t rx_address_extension;
} MblinkIsoTpAddress;

typedef struct {
    uint32_t can_id;
    bool extended_id;
    uint8_t length;
    uint8_t data[MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH];
} MblinkIsoTpCanFrame;

typedef struct {
    MblinkIsoTpAddress address;
    uint8_t block_size;
    uint8_t stmin;
    uint64_t consecutive_timeout_us;
} MblinkIsoTpRxConfig;

typedef struct {
    MblinkIsoTpRxConfig config;
    uint8_t *buffer;
    size_t capacity;
    size_t expected_length;
    size_t received_length;
    uint8_t next_sequence;
    uint8_t block_counter;
    uint64_t deadline_us;
    MblinkIsoTpRxState state;
    MblinkIsoTpResult failure;
} MblinkIsoTpRx;

typedef struct {
    MblinkIsoTpAddress address;
    uint64_t flow_control_timeout_us;
    uint8_t max_wait_frames;
} MblinkIsoTpTxConfig;

typedef struct {
    MblinkIsoTpTxConfig config;
    const uint8_t *payload;
    size_t payload_length;
    size_t offset;
    uint8_t next_sequence;
    uint8_t block_size;
    uint8_t block_counter;
    uint8_t wait_frame_count;
    uint32_t separation_time_us;
    uint64_t next_send_us;
    uint64_t deadline_us;
    MblinkIsoTpTxState state;
    MblinkIsoTpResult failure;
} MblinkIsoTpTx;

const char *mblink_isotp_result_name(MblinkIsoTpResult result);
const char *mblink_isotp_rx_state_name(MblinkIsoTpRxState state);
const char *mblink_isotp_tx_state_name(MblinkIsoTpTxState state);

bool mblink_isotp_address_is_valid(const MblinkIsoTpAddress *address);
bool mblink_isotp_stmin_to_us(uint8_t stmin, uint32_t *microseconds);

/** Initialise RX; `buffer` is borrowed until the receiver is no longer used. */
MblinkIsoTpResult mblink_isotp_rx_init(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpRxConfig *config,
    uint8_t *buffer,
    size_t capacity);

/** Reset RX state and clear any latched terminal failure. */
void mblink_isotp_rx_reset(MblinkIsoTpRx *receiver);

/** Feed one addressed CAN frame at monotonic time `now_us`. */
MblinkIsoTpResult mblink_isotp_rx_feed(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpCanFrame *frame,
    uint64_t now_us,
    MblinkIsoTpCanFrame *flow_control_frame,
    bool *flow_control_ready);

MblinkIsoTpResult mblink_isotp_rx_tick(
    MblinkIsoTpRx *receiver,
    uint64_t now_us);

const uint8_t *mblink_isotp_rx_payload(
    const MblinkIsoTpRx *receiver,
    size_t *length);

/**
 * Initialise TX with a borrowed payload.
 *
 * The payload must remain valid until the transmitter is reinitialised or no
 * longer used. `mblink_isotp_tx_reset()` preserves it for retransmission.
 */
MblinkIsoTpResult mblink_isotp_tx_init(
    MblinkIsoTpTx *transmitter,
    const MblinkIsoTpTxConfig *config,
    const uint8_t *payload,
    size_t payload_length);

/** Reset TX progress/failure state while preserving configuration and payload. */
void mblink_isotp_tx_reset(MblinkIsoTpTx *transmitter);

MblinkIsoTpResult mblink_isotp_tx_start(
    MblinkIsoTpTx *transmitter,
    uint64_t now_us,
    MblinkIsoTpCanFrame *frame);

/**
 * Accept Flow Control addressed to this transmitter.
 *
 * Well-formed frames for another CAN/address-extension endpoint return
 * UNEXPECTED_FRAME without destroying the active transfer.
 */
MblinkIsoTpResult mblink_isotp_tx_accept_flow_control(
    MblinkIsoTpTx *transmitter,
    const MblinkIsoTpCanFrame *frame,
    uint64_t now_us);

MblinkIsoTpResult mblink_isotp_tx_next(
    MblinkIsoTpTx *transmitter,
    uint64_t now_us,
    MblinkIsoTpCanFrame *frame);

MblinkIsoTpResult mblink_isotp_tx_tick(
    MblinkIsoTpTx *transmitter,
    uint64_t now_us);

#ifdef __cplusplus
}
#endif

#endif
