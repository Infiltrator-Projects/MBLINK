// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MBLINK_LINUX_STYLE_H
#define MBLINK_LINUX_STYLE_H

#include <stdbool.h>

const char *mblink_linux_style_base_css(void);
const char *mblink_linux_style_metrics_css(void);
bool mblink_linux_style_register_fonts(void);

#endif
