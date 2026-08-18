// SPDX-License-Identifier: GPL-3.0-or-later
/* Private ISO-TP implementation fragment compiled as part of mblink.c so the
 * CMake and Xcode core targets consume the exact same portable C source. */
#include "mblink/isotp.h"
#include "infiltratr/core.h"

#include <string.h>

static bool mblink_isotp_enum_values_valid(const MblinkIsoTpAddress *address)
{
    return address->format >= MBLINK_ISOTP_ADDRESS_NORMAL &&
           address->format <= MBLINK_ISOTP_ADDRESS_MIXED &&
           address->target_type >= MBLINK_ISOTP_TARGET_PHYSICAL &&
           address->target_type <= MBLINK_ISOTP_TARGET_FUNCTIONAL;
}

static bool mblink_can_address_is_valid(MblinkCanAddress address)
{
    const uint32_t maximum = address.extended_id ? 0x1FFFFFFFU : 0x7FFU;
    return address.can_id <= maximum;
}

static size_t mblink_isotp_prefix_length(const MblinkIsoTpAddress *address)
{
    return address->format == MBLINK_ISOTP_ADDRESS_NORMAL ? 0U : 1U;
}

static size_t mblink_isotp_single_capacity(const MblinkIsoTpAddress *address)
{
    return MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
           mblink_isotp_prefix_length(address) - 1U;
}

static size_t mblink_isotp_first_capacity(const MblinkIsoTpAddress *address)
{
    return MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
           mblink_isotp_prefix_length(address) - 2U;
}

static size_t mblink_isotp_consecutive_capacity(const MblinkIsoTpAddress *address)
{
    return MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
           mblink_isotp_prefix_length(address) - 1U;
}

static bool mblink_isotp_frame_is_classic(const MblinkCanFrame *frame)
{
    return frame != NULL && !frame->flexible_data_rate &&
           frame->data_length > 0U &&
           frame->data_length <= MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH;
}

static bool mblink_isotp_can_id_matches(const MblinkCanFrame *frame,
                                            const MblinkIsoTpAddress *address,
                                            bool receive_direction)
{
    MblinkCanAddress expected;

    if (frame == NULL || address == NULL) {
        return false;
    }
    expected = receive_direction ? address->rx : address->tx;
    return frame->can_id == expected.can_id &&
           frame->extended_id == expected.extended_id;
}

static bool mblink_isotp_frame_matches(const MblinkCanFrame *frame,
                                       const MblinkIsoTpAddress *address,
                                       bool receive_direction)
{
    if (!mblink_isotp_can_id_matches(frame, address, receive_direction) ||
        !mblink_isotp_frame_is_classic(frame)) {
        return false;
    }

    if (address->format != MBLINK_ISOTP_ADDRESS_NORMAL) {
        const uint8_t expected_extension = receive_direction
            ? address->rx_address_extension
            : address->tx_address_extension;
        if (frame->data_length < 2U || frame->data[0] != expected_extension) {
            return false;
        }
    }

    return true;
}

static size_t mblink_isotp_pci_offset(const MblinkIsoTpAddress *address)
{
    return mblink_isotp_prefix_length(address);
}

static void mblink_isotp_prepare_tx_frame(const MblinkIsoTpAddress *address,
                                          const MblinkIsoTpOptions *options,
                                          MblinkCanFrame *frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->can_id = address->tx.can_id;
    frame->extended_id = address->tx.extended_id;
    frame->flexible_data_rate = false;
    if (address->format != MBLINK_ISOTP_ADDRESS_NORMAL) {
        frame->data[0] = address->tx_address_extension;
    }
    if (options->tx_padding_enabled) {
        memset(frame->data + mblink_isotp_prefix_length(address),
               options->tx_padding_byte,
               MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH -
                   mblink_isotp_prefix_length(address));
    }
}

static void mblink_isotp_finish_tx_frame(const MblinkIsoTpOptions *options,
                                         MblinkCanFrame *frame,
                                         size_t used)
{
    frame->data_length = (uint8_t)(options->tx_padding_enabled
        ? MBLINK_ISOTP_CLASSIC_CAN_DATA_LENGTH
        : used);
}

static void mblink_isotp_set_receiver_error(MblinkIsoTpReceiver *receiver,
                                            MblinkIsoTpError error)
{
    receiver->state = MBLINK_ISOTP_RX_ERROR;
    receiver->error = error;
    receiver->deadline_us = 0U;
}

static void mblink_isotp_set_sender_error(MblinkIsoTpSender *sender,
                                          MblinkIsoTpError error)
{
    sender->state = MBLINK_ISOTP_TX_ERROR;
    sender->error = error;
    sender->deadline_us = 0U;
}

static uint64_t mblink_isotp_deadline(uint64_t now_us, uint64_t timeout_us)
{
    return infiltratr_u64_add_saturating(now_us, timeout_us);
}

static bool mblink_isotp_build_flow_control(const MblinkIsoTpReceiver *receiver,
                                            MblinkIsoTpFlowStatus flow_status,
                                            MblinkCanFrame *frame)
{
    size_t offset;
    size_t used;

    if (receiver == NULL || frame == NULL) {
        return false;
    }

    mblink_isotp_prepare_tx_frame(&receiver->address, &receiver->options, frame);
    offset = mblink_isotp_pci_offset(&receiver->address);
    frame->data[offset] = (uint8_t)(0x30U | ((uint8_t)flow_status & 0x0FU));
    frame->data[offset + 1U] = receiver->options.receive_block_size;
    frame->data[offset + 2U] = receiver->options.receive_stmin;
    used = offset + 3U;
    mblink_isotp_finish_tx_frame(&receiver->options, frame, used);
    return true;
}

bool mblink_isotp_address_is_valid(const MblinkIsoTpAddress *address)
{
    if (address == NULL || address->struct_size < sizeof(*address) ||
        address->abi_version != MBLINK_ISOTP_ABI ||
        !mblink_isotp_enum_values_valid(address)) {
        return false;
    }

    return mblink_can_address_is_valid(address->tx) &&
           mblink_can_address_is_valid(address->rx);
}

bool mblink_isotp_stmin_to_us(uint8_t stmin, uint64_t *microseconds)
{
    if (microseconds == NULL) {
        return false;
    }

    if (stmin <= 0x7FU) {
        *microseconds = infiltratr_u64_multiply_saturating((uint64_t)stmin,
                                                           1000U);
        return true;
    }

    if (stmin >= 0xF1U && stmin <= 0xF9U) {
        *microseconds = (uint64_t)(stmin - 0xF0U) * 100U;
        return true;
    }

    return false;
}

bool mblink_isotp_options_is_valid(const MblinkIsoTpOptions *options)
{
    uint64_t ignored;

    if (options == NULL || options->struct_size < sizeof(*options) ||
        options->abi_version != MBLINK_ISOTP_ABI ||
        options->receive_timeout_us == 0U ||
        options->flow_control_timeout_us == 0U) {
        return false;
    }

    return mblink_isotp_stmin_to_us(options->receive_stmin, &ignored);
}

MblinkIsoTpFrameType mblink_isotp_frame_type(const MblinkCanFrame *frame,
                                              const MblinkIsoTpAddress *address,
                                              bool receive_direction)
{
    size_t offset;
    uint8_t type;

    if (!mblink_isotp_address_is_valid(address) ||
        !mblink_isotp_frame_matches(frame, address, receive_direction)) {
        return MBLINK_ISOTP_FRAME_UNKNOWN;
    }

    offset = mblink_isotp_pci_offset(address);
    if (frame->data_length <= offset) {
        return MBLINK_ISOTP_FRAME_UNKNOWN;
    }

    type = (uint8_t)(frame->data[offset] >> 4U);
    switch (type) {
    case 0x0U:
        return MBLINK_ISOTP_FRAME_SINGLE;
    case 0x1U:
        return MBLINK_ISOTP_FRAME_FIRST;
    case 0x2U:
        return MBLINK_ISOTP_FRAME_CONSECUTIVE;
    case 0x3U:
        return MBLINK_ISOTP_FRAME_FLOW_CONTROL;
    default:
        return MBLINK_ISOTP_FRAME_UNKNOWN;
    }
}

