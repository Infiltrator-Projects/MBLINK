/* Sender state machine; included after isotp_receive.inc. */
bool mblink_isotp_sender_init(MblinkIsoTpSender *sender,
                              const MblinkIsoTpAddress *address,
                              const MblinkIsoTpOptions *options)
{
    if (sender == NULL || !mblink_isotp_address_is_valid(address) ||
        !mblink_isotp_options_is_valid(options)) {
        return false;
    }

    memset(sender, 0, sizeof(*sender));
    sender->address = *address;
    sender->options = *options;
    sender->state = MBLINK_ISOTP_TX_IDLE;
    return true;
}

void mblink_isotp_sender_reset(MblinkIsoTpSender *sender)
{
    MblinkIsoTpAddress address;
    MblinkIsoTpOptions options;

    if (sender == NULL) {
        return;
    }
    address = sender->address;
    options = sender->options;
    memset(sender, 0, sizeof(*sender));
    sender->address = address;
    sender->options = options;
    sender->state = MBLINK_ISOTP_TX_IDLE;
}

MblinkIsoTpStepResult mblink_isotp_sender_start(MblinkIsoTpSender *sender,
                                                 const uint8_t *payload,
                                                 size_t length,
                                                 uint64_t now_us,
                                                 MblinkCanFrame *frame)
{
    size_t offset;
    size_t capacity;
    size_t used;

    if (sender == NULL || payload == NULL || frame == NULL || length == 0U ||
        length > MBLINK_ISOTP_MAX_PDU_LENGTH) {
        if (sender != NULL) {
            mblink_isotp_set_sender_error(sender,
                                          MBLINK_ISOTP_ERROR_INVALID_ARGUMENT);
        }
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL ||
        sender->state == MBLINK_ISOTP_TX_SENDING) {
        mblink_isotp_set_sender_error(sender, MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME);
        return MBLINK_ISOTP_STEP_ERROR;
    }

    sender->error = MBLINK_ISOTP_ERROR_NONE;
    sender->payload_length = length;
    sender->payload_offset = 0U;
    sender->next_sequence = 1U;
    sender->block_size = 0U;
    sender->block_count = 0U;
    sender->wait_frames = 0U;
    sender->separation_time_us = 0U;
    sender->next_send_us = 0U;
    sender->deadline_us = 0U;
    memcpy(sender->payload, payload, length);

    mblink_isotp_prepare_tx_frame(&sender->address, &sender->options, frame);
    offset = mblink_isotp_pci_offset(&sender->address);
    capacity = mblink_isotp_single_capacity(&sender->address);

    if (length <= capacity) {
        frame->data[offset] = (uint8_t)length;
        memcpy(frame->data + offset + 1U, sender->payload, length);
        used = offset + 1U + length;
        mblink_isotp_finish_tx_frame(&sender->options, frame, used);
        sender->payload_offset = length;
        sender->state = MBLINK_ISOTP_TX_COMPLETE;
        return MBLINK_ISOTP_STEP_FRAME_READY;
    }

    if (sender->address.target_type == MBLINK_ISOTP_TARGET_FUNCTIONAL) {
        mblink_isotp_set_sender_error(
            sender, MBLINK_ISOTP_ERROR_FUNCTIONAL_MULTIFRAME);
        return MBLINK_ISOTP_STEP_ERROR;
    }

    capacity = mblink_isotp_first_capacity(&sender->address);
    frame->data[offset] = (uint8_t)(0x10U | ((length >> 8U) & 0x0FU));
    frame->data[offset + 1U] = (uint8_t)(length & 0xFFU);
    memcpy(frame->data + offset + 2U, sender->payload, capacity);
    sender->payload_offset = capacity;
    used = offset + 2U + capacity;
    mblink_isotp_finish_tx_frame(&sender->options, frame, used);
    sender->deadline_us = mblink_isotp_deadline(
        now_us, sender->options.flow_control_timeout_us);
    sender->state = MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL;
    return MBLINK_ISOTP_STEP_FRAME_READY;
}

MblinkIsoTpStepResult mblink_isotp_sender_accept_flow_control(
    MblinkIsoTpSender *sender,
    const MblinkCanFrame *frame,
    uint64_t now_us)
{
    size_t offset;
    uint8_t flow_status;
    uint64_t stmin_us;

    if (sender == NULL || frame == NULL) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state == MBLINK_ISOTP_TX_ERROR) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (!mblink_isotp_can_id_matches(frame, &sender->address, true)) {
        return MBLINK_ISOTP_STEP_IGNORED;
    }
    if (!mblink_isotp_frame_is_classic(frame)) {
        mblink_isotp_set_sender_error(sender, MBLINK_ISOTP_ERROR_INVALID_FRAME);
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (!mblink_isotp_frame_matches(frame, &sender->address, true)) {
        return MBLINK_ISOTP_STEP_IGNORED;
    }
    if (sender->state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL &&
        now_us >= sender->deadline_us) {
        mblink_isotp_set_sender_error(sender, MBLINK_ISOTP_ERROR_TIMEOUT);
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state != MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL) {
        mblink_isotp_set_sender_error(sender,
                                      MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME);
        return MBLINK_ISOTP_STEP_ERROR;
    }

    offset = mblink_isotp_pci_offset(&sender->address);
    if ((size_t)frame->data_length < offset + 3U ||
        (frame->data[offset] >> 4U) != 0x3U) {
        mblink_isotp_set_sender_error(sender, MBLINK_ISOTP_ERROR_INVALID_FRAME);
        return MBLINK_ISOTP_STEP_ERROR;
    }

    flow_status = (uint8_t)(frame->data[offset] & 0x0FU);
    if (flow_status == (uint8_t)MBLINK_ISOTP_FLOW_CONTINUE) {
        if (!mblink_isotp_stmin_to_us(frame->data[offset + 2U], &stmin_us)) {
            mblink_isotp_set_sender_error(sender,
                                          MBLINK_ISOTP_ERROR_INVALID_STMIN);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        sender->block_size = frame->data[offset + 1U];
        sender->block_count = 0U;
        sender->wait_frames = 0U;
        sender->separation_time_us = stmin_us;
        sender->next_send_us = now_us;
        sender->deadline_us = 0U;
        sender->state = MBLINK_ISOTP_TX_SENDING;
        return MBLINK_ISOTP_STEP_IN_PROGRESS;
    }

    if (flow_status == (uint8_t)MBLINK_ISOTP_FLOW_WAIT) {
        sender->wait_frames++;
        if (sender->wait_frames > sender->options.max_wait_frames) {
            mblink_isotp_set_sender_error(sender,
                                          MBLINK_ISOTP_ERROR_WAIT_FRAME_LIMIT);
            return MBLINK_ISOTP_STEP_ERROR;
        }
        sender->deadline_us = mblink_isotp_deadline(
            now_us, sender->options.flow_control_timeout_us);
        return MBLINK_ISOTP_STEP_IN_PROGRESS;
    }

    if (flow_status == (uint8_t)MBLINK_ISOTP_FLOW_OVERFLOW) {
        mblink_isotp_set_sender_error(
            sender, MBLINK_ISOTP_ERROR_FLOW_CONTROL_OVERFLOW);
        return MBLINK_ISOTP_STEP_ERROR;
    }

    mblink_isotp_set_sender_error(sender, MBLINK_ISOTP_ERROR_INVALID_FRAME);
    return MBLINK_ISOTP_STEP_ERROR;
}

MblinkIsoTpStepResult mblink_isotp_sender_next(MblinkIsoTpSender *sender,
                                                uint64_t now_us,
                                                MblinkCanFrame *frame)
{
    size_t offset;
    size_t capacity;
    size_t remaining;
    size_t copy_length;
    size_t used;

    if (sender == NULL || frame == NULL) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state == MBLINK_ISOTP_TX_ERROR) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state == MBLINK_ISOTP_TX_COMPLETE) {
        return MBLINK_ISOTP_STEP_COMPLETE;
    }
    if (sender->state != MBLINK_ISOTP_TX_SENDING) {
        return MBLINK_ISOTP_STEP_NOT_DUE;
    }
    if (now_us < sender->next_send_us) {
        return MBLINK_ISOTP_STEP_NOT_DUE;
    }
    if (sender->payload_offset >= sender->payload_length) {
        sender->state = MBLINK_ISOTP_TX_COMPLETE;
        return MBLINK_ISOTP_STEP_COMPLETE;
    }

    mblink_isotp_prepare_tx_frame(&sender->address, &sender->options, frame);
    offset = mblink_isotp_pci_offset(&sender->address);
    capacity = mblink_isotp_consecutive_capacity(&sender->address);
    remaining = sender->payload_length - sender->payload_offset;
    copy_length = remaining < capacity ? remaining : capacity;

    frame->data[offset] = (uint8_t)(0x20U | (sender->next_sequence & 0x0FU));
    memcpy(frame->data + offset + 1U,
           sender->payload + sender->payload_offset,
           copy_length);
    sender->payload_offset += copy_length;
    used = offset + 1U + copy_length;
    mblink_isotp_finish_tx_frame(&sender->options, frame, used);
    sender->next_sequence = (uint8_t)((sender->next_sequence + 1U) & 0x0FU);
    sender->block_count++;

    if (sender->payload_offset == sender->payload_length) {
        sender->state = MBLINK_ISOTP_TX_COMPLETE;
        sender->deadline_us = 0U;
        return MBLINK_ISOTP_STEP_FRAME_READY;
    }

    if (sender->block_size != 0U && sender->block_count >= sender->block_size) {
        sender->block_count = 0U;
        sender->deadline_us = mblink_isotp_deadline(
            now_us, sender->options.flow_control_timeout_us);
        sender->state = MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL;
        return MBLINK_ISOTP_STEP_FRAME_READY;
    }

    sender->next_send_us = mblink_isotp_deadline(now_us,
                                                  sender->separation_time_us);
    return MBLINK_ISOTP_STEP_FRAME_READY;
}

MblinkIsoTpStepResult mblink_isotp_sender_tick(MblinkIsoTpSender *sender,
                                                uint64_t now_us)
{
    if (sender == NULL) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state == MBLINK_ISOTP_TX_ERROR) {
        return MBLINK_ISOTP_STEP_ERROR;
    }
    if (sender->state == MBLINK_ISOTP_TX_COMPLETE) {
        return MBLINK_ISOTP_STEP_COMPLETE;
    }
    if (sender->state == MBLINK_ISOTP_TX_WAIT_FLOW_CONTROL &&
        now_us >= sender->deadline_us) {
        mblink_isotp_set_sender_error(sender, MBLINK_ISOTP_ERROR_TIMEOUT);
        return MBLINK_ISOTP_STEP_ERROR;
    }
    return MBLINK_ISOTP_STEP_IN_PROGRESS;
}

const char *mblink_isotp_error_name(MblinkIsoTpError error)
{
    switch (error) {
    case MBLINK_ISOTP_ERROR_NONE:
        return "none";
    case MBLINK_ISOTP_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case MBLINK_ISOTP_ERROR_INVALID_ADDRESS:
        return "invalid address";
    case MBLINK_ISOTP_ERROR_INVALID_FRAME:
        return "invalid frame";
    case MBLINK_ISOTP_ERROR_UNEXPECTED_FRAME:
        return "unexpected frame";
    case MBLINK_ISOTP_ERROR_INVALID_LENGTH:
        return "invalid length";
    case MBLINK_ISOTP_ERROR_SEQUENCE:
        return "sequence mismatch";
    case MBLINK_ISOTP_ERROR_TIMEOUT:
        return "timeout";
    case MBLINK_ISOTP_ERROR_FLOW_CONTROL_OVERFLOW:
        return "flow-control overflow";
    case MBLINK_ISOTP_ERROR_WAIT_FRAME_LIMIT:
        return "wait-frame limit";
    case MBLINK_ISOTP_ERROR_INVALID_STMIN:
        return "invalid separation time";
    case MBLINK_ISOTP_ERROR_FUNCTIONAL_MULTIFRAME:
        return "functional multi-frame unsupported";
    case MBLINK_ISOTP_ERROR_BUFFER_OVERFLOW:
        return "buffer overflow";
    default:
        return "unknown";
    }
}
