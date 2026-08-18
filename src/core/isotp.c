// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file isotp.c
 * @brief ISO-TP (ISO 15765-2) over Classical CAN.
 */
#include "mblink/isotp.h"

#include "infiltratr/core.h"

#include <string.h>

static bool mblink_isotp_can_id_valid(uint32_t can_id, bool extended)
{
    return extended ? can_id <= 0x1fffffffU : can_id <= 0x7ffU;
}

static bool mblink_isotp_mode_has_address_extension(
    MblinkIsoTpAddressingMode mode)
{
    return mode == MBLINK_ISOTP_ADDRESSING_EXTENDED ||
           mode == MBLINK_ISOTP_ADDRESSING_MIXED;
}

static size_t mblink_isotp_pci_offset(const MblinkIsoTpAddress *address)
{
    return mblink_isotp_mode_has_address_extension(address->addressing_mode)
        ? 1U : 0U;
}

static size_t mblink_isotp_single_frame_capacity(
    const MblinkIsoTpAddress *address)
{
    return MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
           mblink_isotp_pci_offset(address) - 1U;
}

static size_t mblink_isotp_first_frame_capacity(
    const MblinkIsoTpAddress *address)
{
    return MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
           mblink_isotp_pci_offset(address) - 2U;
}

static size_t mblink_isotp_consecutive_frame_capacity(
    const MblinkIsoTpAddress *address)
{
    return MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
           mblink_isotp_pci_offset(address) - 1U;
}

static bool mblink_isotp_frame_valid(const MblinkIsoTpCanFrame *frame)
{
    return frame != NULL &&
           frame->length > 0U &&
           frame->length <= MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH &&
           mblink_isotp_can_id_valid(frame->can_id, frame->extended_id);
}

static bool mblink_isotp_frame_matches_receive_address(
    const MblinkIsoTpAddress *address,
    const MblinkIsoTpCanFrame *frame,
    size_t *pci_offset)
{
    size_t offset;

    if (address == NULL || !mblink_isotp_frame_valid(frame) ||
        frame->can_id != address->rx_can_id ||
        frame->extended_id != address->rx_extended_id) {
        return false;
    }

    offset = mblink_isotp_pci_offset(address);
    if (frame->length <= offset) {
        return false;
    }

    if (offset != 0U &&
        frame->data[0] != address->rx_address_extension) {
        return false;
    }

    if (pci_offset != NULL) {
        *pci_offset = offset;
    }
    return true;
}

static void mblink_isotp_prepare_transmit_frame(
    const MblinkIsoTpAddress *address,
    MblinkIsoTpCanFrame *frame,
    size_t *pci_offset)
{
    const size_t offset = mblink_isotp_pci_offset(address);

    memset(frame, 0, sizeof(*frame));
    frame->can_id = address->tx_can_id;
    frame->extended_id = address->tx_extended_id;

    if (offset != 0U) {
        frame->data[0] = address->tx_address_extension;
    }

    if (pci_offset != NULL) {
        *pci_offset = offset;
    }
}

static uint64_t mblink_isotp_deadline(uint64_t now_us, uint64_t timeout_us)
{
    return infiltratr_u64_add_saturating(now_us, timeout_us);
}

static MblinkIsoTpResult mblink_isotp_build_flow_control(
    const MblinkIsoTpAddress *address,
    MblinkIsoTpFlowStatus flow_status,
    uint8_t block_size,
    uint8_t stmin,
    MblinkIsoTpCanFrame *frame)
{
    size_t offset;

    if (!mblink_isotp_address_is_valid(address) || frame == NULL ||
        flow_status > MBLINK_ISOTP_FLOW_OVERFLOW) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }

    mblink_isotp_prepare_transmit_frame(address, frame, &offset);
    frame->data[offset] = (uint8_t)(0x30U | (uint8_t)flow_status);
    frame->data[offset + 1U] = block_size;
    frame->data[offset + 2U] = stmin;
    frame->length = (uint8_t)(offset + 3U);
    return MBLINK_ISOTP_RESULT_OK;
}

const char *mblink_isotp_result_name(MblinkIsoTpResult result)
{
    switch (result) {
    case MBLINK_ISOTP_RESULT_OK: return "ok";
    case MBLINK_ISOTP_RESULT_COMPLETE: return "complete";
    case MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL: return "wait-flow-control";
    case MBLINK_ISOTP_RESULT_WAIT_SEPARATION: return "wait-separation";
    case MBLINK_ISOTP_RESULT_FLOW_CONTROL_WAIT: return "flow-control-wait";
    case MBLINK_ISOTP_RESULT_INVALID_ARGUMENT: return "invalid-argument";
    case MBLINK_ISOTP_RESULT_INVALID_FRAME: return "invalid-frame";
    case MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME: return "unexpected-frame";
    case MBLINK_ISOTP_RESULT_WRONG_SEQUENCE: return "wrong-sequence";
    case MBLINK_ISOTP_RESULT_BUFFER_TOO_SMALL: return "buffer-too-small";
    case MBLINK_ISOTP_RESULT_PAYLOAD_TOO_LARGE: return "payload-too-large";
    case MBLINK_ISOTP_RESULT_TIMEOUT: return "timeout";
    case MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW:
        return "flow-control-overflow";
    case MBLINK_ISOTP_RESULT_WAIT_FRAME_LIMIT: return "wait-frame-limit";
    case MBLINK_ISOTP_RESULT_UNSUPPORTED: return "unsupported";
    }
    return "unknown";
}

const char *mblink_isotp_rx_state_name(MblinkIsoTpRxState state)
{
    switch (state) {
    case MBLINK_ISOTP_RX_IDLE: return "idle";
    case MBLINK_ISOTP_RX_RECEIVING: return "receiving";
    case MBLINK_ISOTP_RX_COMPLETE: return "complete";
    case MBLINK_ISOTP_RX_FAILED: return "failed";
    }
    return "unknown";
}

const char *mblink_isotp_tx_state_name(MblinkIsoTpTxState state)
{
    switch (state) {
    case MBLINK_ISOTP_TX_IDLE: return "idle";
    case MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL: return "wait-flow-control";
    case MBLINK_ISOTP_TX_SENDING: return "sending";
    case MBLINK_ISOTP_TX_COMPLETE: return "complete";
    case MBLINK_ISOTP_TX_FAILED: return "failed";
    }
    return "unknown";
}

bool mblink_isotp_address_is_valid(const MblinkIsoTpAddress *address)
{
    if (address == NULL ||
        !mblink_isotp_can_id_valid(address->tx_can_id,
                                   address->tx_extended_id) ||
        !mblink_isotp_can_id_valid(address->rx_can_id,
                                   address->rx_extended_id)) {
        return false;
    }

    return address->addressing_mode >= MBLINK_ISOTP_ADDRESSING_NORMAL &&
           address->addressing_mode <= MBLINK_ISOTP_ADDRESSING_MIXED &&
           address->target_type >= MBLINK_ISOTP_TARGET_PHYSICAL &&
           address->target_type <= MBLINK_ISOTP_TARGET_FUNCTIONAL;
}

bool mblink_isotp_stmin_to_us(uint8_t stmin, uint32_t *microseconds)
{
    if (microseconds == NULL) {
        return false;
    }

    if (stmin <= 0x7fU) {
        *microseconds = (uint32_t)stmin * 1000U;
        return true;
    }

    if (stmin >= 0xf1U && stmin <= 0xf9U) {
        *microseconds = (uint32_t)(stmin - 0xf0U) * 100U;
        return true;
    }

    return false;
}

MblinkIsoTpResult mblink_isotp_rx_init(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpRxConfig *config,
    uint8_t *buffer,
    size_t capacity)
{
    uint32_t stmin_us;

    if (receiver == NULL || config == NULL || buffer == NULL ||
        capacity == 0U ||
        !mblink_isotp_address_is_valid(&config->address) ||
        config->consecutive_timeout_us == 0U ||
        !mblink_isotp_stmin_to_us(config->stmin, &stmin_us)) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }

    (void)stmin_us;
    memset(receiver, 0, sizeof(*receiver));
    receiver->config = *config;
    receiver->buffer = buffer;
    receiver->capacity = capacity;
    receiver->state = MBLINK_ISOTP_RX_IDLE;
    receiver->failure = MBLINK_ISOTP_RESULT_OK;
    return MBLINK_ISOTP_RESULT_OK;
}

void mblink_isotp_rx_reset(MblinkIsoTpRx *receiver)
{
    if (receiver == NULL) {
        return;
    }

    receiver->expected_length = 0U;
    receiver->received_length = 0U;
    receiver->next_sequence = 0U;
    receiver->block_counter = 0U;
    receiver->deadline_us = 0U;
    receiver->state = MBLINK_ISOTP_RX_IDLE;
    receiver->failure = MBLINK_ISOTP_RESULT_OK;
}

static MblinkIsoTpResult mblink_isotp_rx_fail(
    MblinkIsoTpRx *receiver,
    MblinkIsoTpResult result)
{
    receiver->state = MBLINK_ISOTP_RX_FAILED;
    receiver->failure = result;
    receiver->deadline_us = 0U;
    return result;
}

static MblinkIsoTpResult mblink_isotp_rx_accept_single(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpCanFrame *frame,
    size_t offset)
{
    const size_t payload_length = (size_t)(frame->data[offset] & 0x0fU);
    const size_t data_offset = offset + 1U;
    const size_t capacity = mblink_isotp_single_frame_capacity(
        &receiver->config.address);

    if (receiver->state != MBLINK_ISOTP_RX_IDLE) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME);
    }
    if (payload_length == 0U || payload_length > capacity ||
        data_offset + payload_length > frame->length) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }
    if (payload_length > receiver->capacity) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_BUFFER_TOO_SMALL);
    }

    memcpy(receiver->buffer, &frame->data[data_offset], payload_length);
    receiver->expected_length = payload_length;
    receiver->received_length = payload_length;
    receiver->deadline_us = 0U;
    receiver->state = MBLINK_ISOTP_RX_COMPLETE;
    return MBLINK_ISOTP_RESULT_COMPLETE;
}

static MblinkIsoTpResult mblink_isotp_rx_accept_first(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpCanFrame *frame,
    size_t offset,
    uint64_t now_us,
    MblinkIsoTpCanFrame *flow_control_frame,
    bool *flow_control_ready)
{
    size_t payload_length;
    size_t data_offset;
    size_t initial_length;
    MblinkIsoTpResult fc_result;

    if (receiver->state != MBLINK_ISOTP_RX_IDLE) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME);
    }
    if (frame->length < offset + 2U) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }

    payload_length =
        ((size_t)(frame->data[offset] & 0x0fU) << 8U) |
        (size_t)frame->data[offset + 1U];

    if (payload_length == 0U) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_UNSUPPORTED);
    }
    if (payload_length <=
            mblink_isotp_single_frame_capacity(&receiver->config.address) ||
        payload_length > MBLINK_ISOTP_MAX_PDU_LENGTH) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }
    if (payload_length > receiver->capacity) {
        fc_result = mblink_isotp_build_flow_control(
            &receiver->config.address,
            MBLINK_ISOTP_FLOW_OVERFLOW,
            0U,
            0U,
            flow_control_frame);
        if (fc_result == MBLINK_ISOTP_RESULT_OK) {
            *flow_control_ready = true;
        }
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_BUFFER_TOO_SMALL);
    }

    data_offset = offset + 2U;
    initial_length = frame->length - data_offset;
    if (initial_length == 0U ||
        initial_length > mblink_isotp_first_frame_capacity(
            &receiver->config.address) ||
        initial_length >= payload_length) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }

    memcpy(receiver->buffer, &frame->data[data_offset], initial_length);
    receiver->expected_length = payload_length;
    receiver->received_length = initial_length;
    receiver->next_sequence = 1U;
    receiver->block_counter = 0U;
    receiver->deadline_us = mblink_isotp_deadline(
        now_us, receiver->config.consecutive_timeout_us);
    receiver->state = MBLINK_ISOTP_RX_RECEIVING;

    fc_result = mblink_isotp_build_flow_control(
        &receiver->config.address,
        MBLINK_ISOTP_FLOW_CONTINUE_TO_SEND,
        receiver->config.block_size,
        receiver->config.stmin,
        flow_control_frame);
    if (fc_result != MBLINK_ISOTP_RESULT_OK) {
        return mblink_isotp_rx_fail(receiver, fc_result);
    }
    *flow_control_ready = true;
    return MBLINK_ISOTP_RESULT_OK;
}

static MblinkIsoTpResult mblink_isotp_rx_accept_consecutive(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpCanFrame *frame,
    size_t offset,
    uint64_t now_us,
    MblinkIsoTpCanFrame *flow_control_frame,
    bool *flow_control_ready)
{
    const uint8_t sequence = (uint8_t)(frame->data[offset] & 0x0fU);
    const size_t data_offset = offset + 1U;
    size_t available;
    size_t remaining;
    size_t copy_length;
    MblinkIsoTpResult fc_result;

    if (receiver->state != MBLINK_ISOTP_RX_RECEIVING) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME);
    }
    if (sequence != receiver->next_sequence) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_WRONG_SEQUENCE);
    }
    if (frame->length <= data_offset) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }

    available = frame->length - data_offset;
    if (available > mblink_isotp_consecutive_frame_capacity(
            &receiver->config.address)) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }

    remaining = receiver->expected_length - receiver->received_length;
    copy_length = available < remaining ? available : remaining;
    memcpy(&receiver->buffer[receiver->received_length],
           &frame->data[data_offset],
           copy_length);
    receiver->received_length += copy_length;
    receiver->next_sequence =
        (uint8_t)((receiver->next_sequence + 1U) & 0x0fU);
    receiver->block_counter++;

    if (receiver->received_length == receiver->expected_length) {
        receiver->deadline_us = 0U;
        receiver->state = MBLINK_ISOTP_RX_COMPLETE;
        return MBLINK_ISOTP_RESULT_COMPLETE;
    }

    receiver->deadline_us = mblink_isotp_deadline(
        now_us, receiver->config.consecutive_timeout_us);

    if (receiver->config.block_size != 0U &&
        receiver->block_counter >= receiver->config.block_size) {
        receiver->block_counter = 0U;
        fc_result = mblink_isotp_build_flow_control(
            &receiver->config.address,
            MBLINK_ISOTP_FLOW_CONTINUE_TO_SEND,
            receiver->config.block_size,
            receiver->config.stmin,
            flow_control_frame);
        if (fc_result != MBLINK_ISOTP_RESULT_OK) {
            return mblink_isotp_rx_fail(receiver, fc_result);
        }
        *flow_control_ready = true;
    }

    return MBLINK_ISOTP_RESULT_OK;
}

MblinkIsoTpResult mblink_isotp_rx_feed(
    MblinkIsoTpRx *receiver,
    const MblinkIsoTpCanFrame *frame,
    uint64_t now_us,
    MblinkIsoTpCanFrame *flow_control_frame,
    bool *flow_control_ready)
{
    size_t offset;
    uint8_t frame_type;

    if (receiver == NULL || flow_control_frame == NULL ||
        flow_control_ready == NULL || receiver->buffer == NULL) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }
    *flow_control_ready = false;

    if (receiver->state == MBLINK_ISOTP_RX_FAILED) {
        return receiver->failure;
    }

    if (!mblink_isotp_frame_matches_receive_address(
            &receiver->config.address, frame, &offset)) {
        return MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME;
    }

    frame_type = (uint8_t)(frame->data[offset] >> 4U);
    switch (frame_type) {
    case 0x0U:
        return mblink_isotp_rx_accept_single(receiver, frame, offset);
    case 0x1U:
        return mblink_isotp_rx_accept_first(
            receiver, frame, offset, now_us,
            flow_control_frame, flow_control_ready);
    case 0x2U:
        return mblink_isotp_rx_accept_consecutive(
            receiver, frame, offset, now_us,
            flow_control_frame, flow_control_ready);
    case 0x3U:
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME);
    default:
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }
}

MblinkIsoTpResult mblink_isotp_rx_tick(
    MblinkIsoTpRx *receiver,
    uint64_t now_us)
{
    if (receiver == NULL) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }

    if (receiver->state == MBLINK_ISOTP_RX_RECEIVING &&
        now_us >= receiver->deadline_us) {
        return mblink_isotp_rx_fail(
            receiver, MBLINK_ISOTP_RESULT_TIMEOUT);
    }

    if (receiver->state == MBLINK_ISOTP_RX_COMPLETE) {
        return MBLINK_ISOTP_RESULT_COMPLETE;
    }
    if (receiver->state == MBLINK_ISOTP_RX_FAILED) {
        return receiver->failure;
    }

    return MBLINK_ISOTP_RESULT_OK;
}

const uint8_t *mblink_isotp_rx_payload(
    const MblinkIsoTpRx *receiver,
    size_t *length)
{
    if (length != NULL) {
        *length = 0U;
    }
    if (receiver == NULL || receiver->state != MBLINK_ISOTP_RX_COMPLETE) {
        return NULL;
    }

    if (length != NULL) {
        *length = receiver->received_length;
    }
    return receiver->buffer;
}

MblinkIsoTpResult mblink_isotp_tx_init(
    MblinkIsoTpTx *transmitter,
    const MblinkIsoTpTxConfig *config,
    const uint8_t *payload,
    size_t payload_length)
{
    if (transmitter == NULL || config == NULL || payload == NULL ||
        payload_length == 0U ||
        !mblink_isotp_address_is_valid(&config->address) ||
        config->flow_control_timeout_us == 0U) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }

    if (payload_length > MBLINK_ISOTP_MAX_PDU_LENGTH) {
        return MBLINK_ISOTP_RESULT_PAYLOAD_TOO_LARGE;
    }

    memset(transmitter, 0, sizeof(*transmitter));
    transmitter->config = *config;
    transmitter->payload = payload;
    transmitter->payload_length = payload_length;
    transmitter->next_sequence = 1U;
    transmitter->state = MBLINK_ISOTP_TX_IDLE;
    transmitter->failure = MBLINK_ISOTP_RESULT_OK;
    return MBLINK_ISOTP_RESULT_OK;
}

void mblink_isotp_tx_reset(MblinkIsoTpTx *transmitter)
{
    if (transmitter == NULL) {
        return;
    }

    transmitter->offset = 0U;
    transmitter->next_sequence = 1U;
    transmitter->block_size = 0U;
    transmitter->block_counter = 0U;
    transmitter->wait_frame_count = 0U;
    transmitter->separation_time_us = 0U;
    transmitter->next_send_us = 0U;
    transmitter->deadline_us = 0U;
    transmitter->state = MBLINK_ISOTP_TX_IDLE;
    transmitter->failure = MBLINK_ISOTP_RESULT_OK;
}

static MblinkIsoTpResult mblink_isotp_tx_fail(
    MblinkIsoTpTx *transmitter,
    MblinkIsoTpResult result)
{
    transmitter->state = MBLINK_ISOTP_TX_FAILED;
    transmitter->failure = result;
    transmitter->deadline_us = 0U;
    return result;
}

MblinkIsoTpResult mblink_isotp_tx_start(
    MblinkIsoTpTx *transmitter,
    uint64_t now_us,
    MblinkIsoTpCanFrame *frame)
{
    size_t offset;
    size_t sf_capacity;
    size_t ff_capacity;

    if (transmitter == NULL || frame == NULL ||
        transmitter->payload == NULL ||
        transmitter->payload_length == 0U) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_FAILED) {
        return transmitter->failure;
    }
    if (transmitter->state != MBLINK_ISOTP_TX_IDLE) {
        return MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME;
    }

    sf_capacity = mblink_isotp_single_frame_capacity(
        &transmitter->config.address);
    mblink_isotp_prepare_transmit_frame(
        &transmitter->config.address, frame, &offset);

    if (transmitter->payload_length <= sf_capacity) {
        frame->data[offset] = (uint8_t)transmitter->payload_length;
        memcpy(&frame->data[offset + 1U],
               transmitter->payload,
               transmitter->payload_length);
        frame->length = (uint8_t)(
            offset + 1U + transmitter->payload_length);
        transmitter->offset = transmitter->payload_length;
        transmitter->state = MBLINK_ISOTP_TX_COMPLETE;
        return MBLINK_ISOTP_RESULT_COMPLETE;
    }

    if (transmitter->config.address.target_type ==
        MBLINK_ISOTP_TARGET_FUNCTIONAL) {
        return mblink_isotp_tx_fail(
            transmitter, MBLINK_ISOTP_RESULT_UNSUPPORTED);
    }
    if (transmitter->payload_length > MBLINK_ISOTP_MAX_PDU_LENGTH) {
        return mblink_isotp_tx_fail(
            transmitter, MBLINK_ISOTP_RESULT_PAYLOAD_TOO_LARGE);
    }

    ff_capacity = mblink_isotp_first_frame_capacity(
        &transmitter->config.address);
    frame->data[offset] = (uint8_t)(
        0x10U | ((transmitter->payload_length >> 8U) & 0x0fU));
    frame->data[offset + 1U] =
        (uint8_t)(transmitter->payload_length & 0xffU);
    memcpy(&frame->data[offset + 2U],
           transmitter->payload,
           ff_capacity);
    frame->length = MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH;

    transmitter->offset = ff_capacity;
    transmitter->next_sequence = 1U;
    transmitter->block_size = 0U;
    transmitter->block_counter = 0U;
    transmitter->wait_frame_count = 0U;
    transmitter->separation_time_us = 0U;
    transmitter->deadline_us = mblink_isotp_deadline(
        now_us, transmitter->config.flow_control_timeout_us);
    transmitter->state = MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL;
    return MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL;
}

MblinkIsoTpResult mblink_isotp_tx_accept_flow_control(
    MblinkIsoTpTx *transmitter,
    const MblinkIsoTpCanFrame *frame,
    uint64_t now_us)
{
    size_t offset;
    uint8_t flow_status;
    uint32_t separation_time_us;

    if (transmitter == NULL) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_FAILED) {
        return transmitter->failure;
    }
    if (transmitter->state != MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL) {
        return MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME;
    }
    if (!mblink_isotp_frame_valid(frame)) {
        return mblink_isotp_tx_fail(
            transmitter, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }
    if (!mblink_isotp_frame_matches_receive_address(
            &transmitter->config.address, frame, &offset)) {
        return MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME;
    }
    if (frame->length < offset + 3U ||
        (frame->data[offset] >> 4U) != 0x3U) {
        return mblink_isotp_tx_fail(
            transmitter, MBLINK_ISOTP_RESULT_INVALID_FRAME);
    }

    flow_status = (uint8_t)(frame->data[offset] & 0x0fU);
    if (flow_status == MBLINK_ISOTP_FLOW_CONTINUE_TO_SEND) {
        if (!mblink_isotp_stmin_to_us(
                frame->data[offset + 2U],
                &separation_time_us)) {
            return mblink_isotp_tx_fail(
                transmitter, MBLINK_ISOTP_RESULT_INVALID_FRAME);
        }

        transmitter->block_size = frame->data[offset + 1U];
        transmitter->block_counter = 0U;
        transmitter->wait_frame_count = 0U;
        transmitter->separation_time_us = separation_time_us;
        transmitter->next_send_us = now_us;
        transmitter->deadline_us = 0U;
        transmitter->state = MBLINK_ISOTP_TX_SENDING;
        return MBLINK_ISOTP_RESULT_OK;
    }

    if (flow_status == MBLINK_ISOTP_FLOW_WAIT) {
        if (transmitter->wait_frame_count >=
            transmitter->config.max_wait_frames) {
            return mblink_isotp_tx_fail(
                transmitter, MBLINK_ISOTP_RESULT_WAIT_FRAME_LIMIT);
        }

        transmitter->wait_frame_count++;
        transmitter->deadline_us = mblink_isotp_deadline(
            now_us, transmitter->config.flow_control_timeout_us);
        return MBLINK_ISOTP_RESULT_FLOW_CONTROL_WAIT;
    }

    if (flow_status == MBLINK_ISOTP_FLOW_OVERFLOW) {
        return mblink_isotp_tx_fail(
            transmitter, MBLINK_ISOTP_RESULT_FLOW_CONTROL_OVERFLOW);
    }

    return mblink_isotp_tx_fail(
        transmitter, MBLINK_ISOTP_RESULT_INVALID_FRAME);
}

MblinkIsoTpResult mblink_isotp_tx_next(
    MblinkIsoTpTx *transmitter,
    uint64_t now_us,
    MblinkIsoTpCanFrame *frame)
{
    size_t offset;
    size_t frame_capacity;
    size_t remaining;
    size_t copy_length;

    if (transmitter == NULL || frame == NULL) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_FAILED) {
        return transmitter->failure;
    }

    if (transmitter->state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL) {
        if (now_us >= transmitter->deadline_us) {
            return mblink_isotp_tx_fail(
                transmitter, MBLINK_ISOTP_RESULT_TIMEOUT);
        }
        return MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_COMPLETE) {
        return MBLINK_ISOTP_RESULT_COMPLETE;
    }
    if (transmitter->state != MBLINK_ISOTP_TX_SENDING) {
        return MBLINK_ISOTP_RESULT_UNEXPECTED_FRAME;
    }
    if (now_us < transmitter->next_send_us) {
        return MBLINK_ISOTP_RESULT_WAIT_SEPARATION;
    }

    mblink_isotp_prepare_transmit_frame(
        &transmitter->config.address, frame, &offset);
    frame->data[offset] = (uint8_t)(
        0x20U | (transmitter->next_sequence & 0x0fU));

    frame_capacity = mblink_isotp_consecutive_frame_capacity(
        &transmitter->config.address);
    remaining = transmitter->payload_length - transmitter->offset;
    copy_length = remaining < frame_capacity ? remaining : frame_capacity;
    memcpy(&frame->data[offset + 1U],
           &transmitter->payload[transmitter->offset],
           copy_length);
    frame->length = (uint8_t)(offset + 1U + copy_length);

    transmitter->offset += copy_length;
    transmitter->next_sequence =
        (uint8_t)((transmitter->next_sequence + 1U) & 0x0fU);
    transmitter->block_counter++;

    if (transmitter->offset == transmitter->payload_length) {
        transmitter->deadline_us = 0U;
        transmitter->state = MBLINK_ISOTP_TX_COMPLETE;
        return MBLINK_ISOTP_RESULT_COMPLETE;
    }

    transmitter->next_send_us = mblink_isotp_deadline(
        now_us, transmitter->separation_time_us);

    if (transmitter->block_size != 0U &&
        transmitter->block_counter >= transmitter->block_size) {
        transmitter->block_counter = 0U;
        transmitter->deadline_us = mblink_isotp_deadline(
            now_us, transmitter->config.flow_control_timeout_us);
        transmitter->state = MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL;
    }

    return MBLINK_ISOTP_RESULT_OK;
}

MblinkIsoTpResult mblink_isotp_tx_tick(
    MblinkIsoTpTx *transmitter,
    uint64_t now_us)
{
    if (transmitter == NULL) {
        return MBLINK_ISOTP_RESULT_INVALID_ARGUMENT;
    }

    if (transmitter->state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL &&
        now_us >= transmitter->deadline_us) {
        return mblink_isotp_tx_fail(
            transmitter, MBLINK_ISOTP_RESULT_TIMEOUT);
    }

    if (transmitter->state == MBLINK_ISOTP_TX_COMPLETE) {
        return MBLINK_ISOTP_RESULT_COMPLETE;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_FAILED) {
        return transmitter->failure;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL) {
        return MBLINK_ISOTP_RESULT_WAIT_FLOW_CONTROL;
    }
    if (transmitter->state == MBLINK_ISOTP_TX_SENDING &&
        now_us < transmitter->next_send_us) {
        return MBLINK_ISOTP_RESULT_WAIT_SEPARATION;
    }

    return MBLINK_ISOTP_RESULT_OK;
}
