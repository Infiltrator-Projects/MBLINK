// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_ISOTP_TEST_SUPPORT_H
#define MBLINK_ISOTP_TEST_SUPPORT_H

#include "mblink/isotp.h"

#include <stddef.h>

void isotp_test_require(bool condition, const char *message);
MblinkIsoTpAddress isotp_test_address(void);
MblinkIsoTpOptions isotp_test_options(void);
MblinkCanFrame isotp_test_rx_frame(const uint8_t *data, size_t length);

#endif
