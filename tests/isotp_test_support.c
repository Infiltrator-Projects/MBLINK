// SPDX-License-Identifier: GPL-3.0-or-later
#include "isotp_test_support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void isotp_test_require(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

MblinkIsoTpAddress isotp_test_address(void)
{
    MblinkIsoTpAddress address = MBLINK_ISOTP_ADDRESS_INIT;
    address.tx.can_id = 0x7E0U;
    address.rx.can_id = 0x7E8U;
    address.tx.extended_id = false;
    address.rx.extended_id = false;
    address.format = MBLINK_ISOTP_ADDRESS_NORMAL;
    address.target_type = MBLINK_ISOTP_TARGET_PHYSICAL;
    return address;
}

MblinkIsoTpOptions isotp_test_options(void)
{
    MblinkIsoTpOptions options = MBLINK_ISOTP_OPTIONS_INIT;
    options.receive_timeout_us = 100000U;
    options.flow_control_timeout_us = 100000U;
    return options;
}

MblinkCanFrame isotp_test_rx_frame(const uint8_t *data, size_t length)
{
    MblinkCanFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x7E8U;
    frame.data_length = (uint8_t)length;
    memcpy(frame.data, data, length);
    return frame;
}
