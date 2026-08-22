// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/parameter.h"

bool mblink_parameter_from_obd2(const MblinkObd2Sample *sample,
                                uint64_t timestamp_ms,
                                MblinkParameterSample *parameter)
{
    if (sample == NULL) {
        return false;
    }
    return link_parameter_from_obd2_scalar(
        sample->pid,
        (LinkObd2UnitCode)sample->unit,
        sample->value,
        timestamp_ms,
        parameter);
}
