/* Receiver state machine; included after isotp_common.inc. */
bool mblink_isotp_receiver_init(MblinkIsoTpReceiver *receiver,
                                const MblinkIsoTpAddress *address,
                                const MblinkIsoTpOptions *options)
{
    if (receiver == NULL || !mblink_isotp_address_is_valid(address) ||
        !mblink_isotp_options_is_valid(options)) {
        return false;
    }

    memset(receiver, 0, sizeof(*receiver));
    receiver->address = *address;
    receiver->options = *options;
    receiver->state = MBLINK_ISOTP_RX_IDLE;
    return true;
}

void mblink_isotp_receiver_reset(MblinkIsoTpReceiver *receiver)
{
    MblinkIsoTpAddress address;
    MblinkIsoTpOptions options;

    if (receiver == NULL) {
        return;
    }

    address = receiver->address;
    options = receiver->options;
    memset(receiver, 0, sizeof(*receiver));
    receiver->address = address;
    receiver->options = options;
    receiver->state = MBLINK_ISOTP_RX_IDLE;
}

MblinkIsoTpStepResult mblink_isotp_receiver_feed(MblinkIsoTpReceiver *receiver,
                                                  const MblinkCanFrame *frame,
                                                  uint64_t now_us,
                                                  MblinkCanFrame *reply)
{
    MblinkIsoTpFrameType type;
    size_t offset;
    size_t payload_offset;
    size_t available;
    size_t copy_length;
    size_t remaining;
    size_t total_length;
    uint8_t sequence;

    if (receiver == NULL || frame == NULL) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (receiver->state == MBLINK_ISOTP_RX_ERROR) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (!mblink_isotp_can_id_matches(frame, &receiver->address, true)) {
        return MBLINK_ISOTP_STEP_IGNORED;
    }
    if (!mblink_isotp_frame_is_classic(frame)) {
        mblink_isotp_set_receiver_error(receiver, MBLINK_ISOTP_ERROR_INVALID_FRAME);
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (!mblink_isotp_frame_matches(frame, &receiver->address, true)) {
        return MBLINK_ISOTP_STEP_IGNORED;
    }
    if (receiver->state == MBLINK_ISOTP_RX_RECEIVING &&
        now_us >= receiver->deadline_us) {
        mblink_isotp_set_receiver_error(receiver, MBLINK_ISOTP_ERROR_TIMEOUT);
        return MBLINK_ISOTP_STEP_ERROR;
    }

    type = mblink_isotp_frame_type(frame, &receiver->address, true);
    offset = mblink_isotp_pci_offset(&receiver->address);

    if (type == MBLINK_ISOTP_FRAME_SINGLE) {
        if (receiver->state == MBLINK_ISOTP_RX_RECEIVING) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        total_length = (size_t)(frame->data[offset] & 0x0FU);
        payload_offset = offset + 1U;
        available = (size_t)frame->data_length - payload_offset;
        if (total_length == 0U ||
            total_length > mblink_isotp_single_capacity(&receiver->address) ||
            total_length > available) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_INVALID_LENGTH);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        memcpy(receiver->payload, frame->data + payload_offset, total_length);
        receiver->expected_length = total_length;
        receiver->received_length = total_length;
        receiver->state = MBLINK_ISOTP_RX_COMPLETE;
        receiver->error = MBLINK_ISOTP_ERROR_NONE;
        return MBLINK_ISOTP_STEP_COMPLETE;
    }

    if (type == MBLINK_ISOTP_FRAME_FIRST) {
        if (receiver->state == MBLINK_ISOTP_RX_RECEIVING) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        if ((size_t)frame->data_length < offset + 3U) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_INVALID_FRAME);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        total_length = ((size_t)(frame->data[offset] & 0x0FU) << 8U) |
                       (size_t)frame->data[offset + 1U];
        if (total_length <= mblink_isotp_single_capacity(&receiver->address) ||
            total_length > MBLINK_ISOTP_MAX_PDU_LENGTH) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_INVALID_LENGTH);
            if (reply != NULL) {
                (void)mblink_isotp_build_flow_control(
                    receiver, MBLINK_ISOTP_FLOW_OVERFLOW, reply);
                return MBLINK_ISOTP_STEP_FRAME_READY;
            }
            return MBLINK_ISOTP_STEP_ERROR;
        }
        payload_offset = offset + 2U;
        available = (size_t)frame->data_length - payload_offset;
        if (available == 0U) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_INVALID_FRAME);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        copy_length = available < total_length ? available : total_length;
        memcpy(receiver->payload, frame->data + payload_offset, copy_length);
        receiver->expected_length = total_length;
        receiver->received_length = copy_length;
        receiver->next_sequence = 1U;
        receiver->block_count = 0U;
        receiver->deadline_us = mblink_isotp_deadline(
            now_us, receiver->options.receive_timeout_us);
        receiver->state = MBLINK_ISOTP_RX_RECEIVING;
        receiver->error = MBLINK_ISOTP_ERROR_NONE;
        if (reply == NULL ||
            !mblink_isotp_build_flow_control(receiver,
                                             MBLINK_ISOTP_FLOW_CONTINUE,
                                             reply)) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_INVALID_ARGUMENT);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        return MBLINK_ISOTP_STEP_FRAME_READY;
    }

    if (type == MBLINK_ISOTP_FRAME_CONSECUTIVE) {
        if (receiver->state != MBLINK_ISOTP_RX_RECEIVING) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        sequence = (uint8_t)(frame->data[offset] & 0x0FU);
        if (sequence != receiver->next_sequence) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_SEQUENCE);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        payload_offset = offset + 1U;
        available = (size_t)frame->data_length - payload_offset;
        if (available == 0U) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_INVALID_FRAME);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        if (receiver->received_length > receiver->expected_length) {
            mblink_isotp_set_receiver_error(receiver,
                                             MBLINK_ISOTP_ERROR_BUFFER_OVERFLOW);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        remaining = receiver->expected_length - receiver->received_length;
        copy_length = available < remaining ? available : remaining;
        memcpy(receiver->payload + receiver->received_length,
               frame->data + payload_offset,
               copy_length);
        receiver->received_length += copy_length;
        receiver->next_sequence = (uint8_t)((receiver->next_sequence + 1U) & 0x0FU);
        receiver->block_count++;
        receiver->deadline_us = mblink_isotp_deadline(
            now_us, receiver->options.receive_timeout_us);

        if (receiver->received_length == receiver->expected_length) {
            receiver->state = MBLINK_ISOTP_RX_COMPLETE;
            receiver->deadline_us = 0U;
            return MBLINK_ISOTP_STEP_COMPLETE;
        }

        if (receiver->options.receive_block_size != 0U &&
            receiver->block_count >= receiver->options.receive_block_size) {
            receiver->block_count = 0U;
            if (reply == NULL ||
                !mblink_isotp_build_flow_control(receiver,
                                                 MBLINK_ISOTP_FLOW_CONTINUE,
                                                 reply)) {
                mblink_isotp_set_receiver_error(
                    receiver, MBLINK_ISOTP_ERROR_INVALID_ARGUMENT);
                return MBLINK_ISOTP_STEP_ERROR;
            }
            return MBLINK_ISOTP_STEP_FRAME_READY;
        }

        return MBLINK_ISOTP_STEP_IN_PROGRESS;
    }

    mblink_isotp_set_receiver_error(receiver, MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME);
    return MBLINK_ISOTP_STEP_ERROR;
}

MblinkIsoTpStepResult mblink_isotp_receiver_tick(MblinkIsoTpReceiver *receiver,
                                                  uint64_t now_us)
{
    if (receiver == NULL) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (receiver->state == MBLINK_ISOTP_RX_ERROR) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (receiver->state == MBLINK_ISOTP_RX_COMPLETE) {
        return MBLINK_ISOTP_STEP_COMPLETE;
    }
    if (receiver->state == MBLINK_ISOTP_RX_RECEIVING &&
        now_us >= receiver->deadline_us) {
        mblink_isotp_set_receiver_error(receiver, MBLINK_ISOTP_ERROR_TIMEOUT);
        return MBLINK_ISOTP_STEP_ERROR;
    }
    return MBLINK_ISOTP_STEP_IN_PROGRESS;
}

const uint8_t *mblink_isotp_receiver_payload(const MblinkIsoTpReceiver *receiver,
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
    return receiver->payload;
}

