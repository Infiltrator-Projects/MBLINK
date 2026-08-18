// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file isotp.h
 * @brief Portable ISO-TP (ISO 15765-2) transport foundation for Classical CAN.
 *
 * @author Shannon Smith
 * @copyright Copyright (C) 2026 Shannon Smith
 */
#ifndef MBLINK_ISOTP_H
#define MBLINK_ISOTP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_ISOTP_ABI 1U
#define MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH 8U
#define MBLINK_ISOTP_MAX_PDU_LENGTH 4095U
#define MBLINK_ISOTP_DEFAULT_TIMEOUT_US 1000000ULL
#define MBLINK_ISOTP_DEFAULT_MAX_WAIT_FRAMES 3U

typedef enum {
    MBLINK_ISOTP_ADDRESS_NORMAL = 0,
    MBLINK_ISOTP_ADDRESS_EXTENDED,
    MBLINK_ISOTP_ADDRESS_MIXED
} MblinkIsoTpAddressFormat;

typedef enum {
    MBLINK_ISOTP_TARGET_PHYSICAL = 0,
    MBLINK_ISOTP_TARGET_FUNCTIONAL
} MblinkIsoTpTargetType;

typedef struct {
    uint32_t can_id;
    bool extended_id;
} MblinkCanAddress;

typedef struct {
    size_t struct_size;
    uint32_t abi_version;
    MblinkCanAddress tx;
    MblinkCanAddress rx;
    MblinkIsoTpAddressFormat format;
    /* Target type of transmitted N-PDUs; responses are filtered by rx address. */
    MblinkIsoTpTargetType target_type;
    uint8_t tx_address_extension;
    uint8_t rx_address_extension;
} MblinkIsoTpAddress;

#define MBLINK_ISOTP_ADDRESS_INIT \
    { .struct_size = sizeof(MblinkIsoTpAddress), \
      .abi_version = MBLINK_ISOTP_ABI }

typedef struct {
    size_t struct_size;
    uint32_t abi_version;
    uint64_t receive_timeout_us;
    uint64_t flow_control_timeout_us;
    uint8_t receive_block_size;
    uint8_t receive_stmin;
    uint8_t max_wait_frames;
    bool tx_padding_enabled;
    uint8_t tx_padding_byte;
} MblinkIsoTpOptions;

#define MBLINK_ISOTP_OPTIONS_INIT \
    { .struct_size = sizeof(MblinkIsoTpOptions), \
      .abi_version = MBLINK_ISOTP_ABI, \
      .receive_timeout_us = MBLINK_ISOTP_DEFAULT_TIMEOUT_US, \
      .flow_control_timeout_us = MBLINK_ISOTP_DEFAULT_TIMEOUT_US, \
      .receive_block_size = 0U, \
      .receive_stmin = 0U, \
      .max_wait_frames = MBLINK_ISOTP_DEFAULT_MAX_WAIT_FRAMES, \
      .tx_padding_enabled = false, \
      .tx_padding_byte = 0U }

typedef struct {
    uint32_t can_id;
    bool extended_id;
    bool flexible_data_rate;
    uint8_t data_length;
    uint8_t data[64];
} MblinkCanFrame;

typedef enum {
    MBLINK_ISOTP_FRAME_SINGLE = 0,
    MBLINK_ISOTP_FRAME_FIRST,
    MBLINK_ISOTP_FRAME_CONSECUTIVE,
    MBLINK_ISOTP_FRAME_FLOW_CONTROL,
    MBLINK_ISOTP_FRAME_UNKNOWN
} MblinkIsoTpFrameType;

typedef enum {
    MBLINK_ISOTP_FLOW_CONTINUE = 0,
    MBLINK_ISOTP_FLOW_WAIT = 1,
    MBLINK_ISOTP_FLOW_OVERFLOW = 2
} MblinkIsoTpFlowStatus;

typedef enum {
    MBLINK_ISOTP_ERROR_NONE = 0,
    MBLINK_ISOTP_ERROR_INVALID_ARGUMENT,
    MBLINK_ISOTP_ERROR_INVALID_ADDRESS,
    MBLINK_ISOTP_ERROR_INVALID_FRAME,
    MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME,
    MBLINK_ISOTP_ERROR_INVALID_LENGTH,
    MBLINK_ISOTP_ERROR_SEQUENCE,
    MBLINK_ISOTP_ERROR_TIMEOUT,
    MBLINK_ISOTP_ERROR_FLOW_CONTROL_OVERFLOW,
    MBLINK_ISOTP_ERROR_WAIT_FRAME_LIMIT,
    MBLINK_ISOTP_ERROR_INVALID_STMIN,
    MBLINK_ISOTP_ERROR_FUNCTIONAL_MULTIFRAME,
    MBLINK_ISOTP_ERROR_BUFFER_OVERFLOW
} MblinkIsoTpError;

typedef enum {
    MBLINK_ISOTP_STEP_IGNORED = 0,
    MBLINK_ISOTP_STEP_IN_PROGRESS,
    MBLINK_ISOTP_STEP_FRAME_READY,
    MBLINK_ISOTP_STEP_COMPLETE,
    MBLINK_ISOTP_STEP_NOT_DUE,
    MBLINK_ISOTP_STEP_ERROR
} MblinkIsoTpStepResult;

typedef enum {
    MBLINK_ISOTP_RX_IDLE = 0,
    MBLINK_ISOTP_RX_RECEIVING,
    MBLINK_ISOTP_RX_COMPLETE,
    MBLINK_ISOTP_RX_ERROR
} MblinkIsoTpReceiverState;

typedef enum {
    MBLINK_ISOTP_TX_IDLE = 0,
    MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL,
    MBLINK_ISOTP_TX_SENDING,
    MBLINK_ISOTP_TX_COMPLETE,
    MBLINK_ISOTP_TX_ERROR
} MblinkIsoTpSenderState;

typedef struct {
    MblinkIsoTpAddress address;
    MblinkIsoTpOptions options;
    MblinkIsoTpReceiverState state;
    MblinkIsoTpError error;
    uint8_t payload[MBLINK_ISOTP_MAX_PDU_LENGTH];
    size_t expected_length;
    size_t received_length;
    uint8_t next_sequence;
    uint8_t block_count;
    uint64_t deadline_us;
} MblinkIsoTpReceiver;

typedef struct {
    MblinkIsoTpAddress address;
    MblinkIsoTpOptions options;
    MblinkIsoTpSenderState state;
    MblinkIsoTpError error;
    uint8_t payload[MBLINK_ISOTP_MAX_PDU_LENGTH];
    size_t payload_length;
    size_t payload_offset;
    uint8_t next_sequence;
    uint8_t block_size;
    uint8_t block_count;
    uint8_t wait_frames;
    uint64_t separation_time_us;
    uint64_t next_send_us;
    uint64_t deadline_us;
} MblinkIsoTpSender;

bool mblink_isotp_address_is_valid(const MblinkIsoTpAddress *address);
bool mblink_isotp_options_is_valid(const MblinkIsoTpOptions *options);
bool mblink_isotp_stmin_to_us(uint8_t stmin, uint64_t *microseconds);
MblinkIsoTpFrameType mblink_isotp_frame_type(const MblinkCanFrame *frame,
                                              const MblinkIsoTpAddress *address,
                                              bool receive_direction);

bool mblink_isotp_receiver_init(MblinkIsoTpReceiver *receiver,
                                const MblinkIsoTpAddress *address,
                                const MblinkIsoTpOptions *options);
void mblink_isotp_receiver_reset(MblinkIsoTpReceiver *receiver);
MblinkIsoTpStepResult mblink_isotp_receiver_feed(MblinkIsoTpReceiver *receiver,
                                                  const MblinkCanFrame *frame,
                                                  uint64_t now_us,
                                                  MblinkCanFrame *reply);
MblinkIsoTpStepResult mblink_isotp_receiver_tick(MblinkIsoTpReceiver *receiver,
                                                  uint64_t now_us);
const uint8_t *mblink_isotp_receiver_payload(const MblinkIsoTpReceiver *receiver,
                                             size_t *length);

bool mblink_isotp_sender_init(MblinkIsoTpSender *sender,
                              const MblinkIsoTpAddress *address,
                              const MblinkIsoTpOptions *options);
void mblink_isotp_sender_reset(MblinkIsoTpSender *sender);
MblinkIsoTpStepResult mblink_isotp_sender_start(MblinkIsoTpSender *sender,
                                                 const uint8_t *payload,
                                                 size_t length,
                                                 uint64_t now_us,
                                                 MblinkCanFrame *frame);
MblinkIsoTpStepResult mblink_isotp_sender_accept_flow_control(
    MblinkIsoTpSender *sender,
    const MblinkCanFrame *frame,
    uint64_t now_us);
MblinkIsoTpStepResult mblink_isotp_sender_next(MblinkIsoTpSender *sender,
                                                uint64_t now_us,
                                                MblinkCanFrame *frame);
MblinkIsoTpStepResult mblink_isotp_sender_tick(MblinkIsoTpSender *sender,
                                                uint64_t now_us);

const char *mblink_isotp_error_name(MblinkIsoTpError error);

#ifdef __cplusplus
}
#endif

#endif
