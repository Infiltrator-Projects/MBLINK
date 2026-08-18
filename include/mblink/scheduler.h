// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file scheduler.h
 * @brief Portable bounded request scheduler for live diagnostic polling.
 */
#ifndef MBLINK_SCHEDULER_H
#define MBLINK_SCHEDULER_H

#include "mblink/obd2.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBLINK_SCHEDULER_MAX_ITEMS 16U

typedef enum {
    MBLINK_SCHEDULER_PRIORITY_LOW = 0,
    MBLINK_SCHEDULER_PRIORITY_NORMAL = 1,
    MBLINK_SCHEDULER_PRIORITY_HIGH = 2,
    MBLINK_SCHEDULER_PRIORITY_CRITICAL = 3
} MblinkSchedulerPriority;

typedef enum {
    MBLINK_SCHEDULER_RESULT_OK = 0,
    MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT,
    MBLINK_SCHEDULER_RESULT_FULL,
    MBLINK_SCHEDULER_RESULT_DUPLICATE,
    MBLINK_SCHEDULER_RESULT_NOT_FOUND
} MblinkSchedulerResult;

typedef enum {
    MBLINK_SCHEDULER_NEXT_READY = 0,
    MBLINK_SCHEDULER_NEXT_WAITING,
    MBLINK_SCHEDULER_NEXT_PAUSED,
    MBLINK_SCHEDULER_NEXT_EMPTY,
    MBLINK_SCHEDULER_NEXT_INVALID_ARGUMENT
} MblinkSchedulerNextResult;

typedef struct {
    uint8_t pid;
    uint32_t interval_ms;
    uint64_t next_due_ms;
    MblinkSchedulerPriority priority;
    bool enabled;
} MblinkSchedulerItem;

typedef struct {
    MblinkSchedulerItem items[MBLINK_SCHEDULER_MAX_ITEMS];
    size_t count;
    bool paused;
    uint64_t pause_started_ms;
} MblinkScheduler;

typedef struct {
    size_t index;
    uint8_t pid;
    uint64_t due_ms;
    uint64_t wait_ms;
} MblinkSchedulerDispatch;

const char *mblink_scheduler_result_name(MblinkSchedulerResult result);
const char *mblink_scheduler_next_result_name(MblinkSchedulerNextResult result);

void mblink_scheduler_init(MblinkScheduler *scheduler);

/** All scheduler timestamps use one caller-supplied monotonic millisecond clock. */
MblinkSchedulerResult mblink_scheduler_add(
    MblinkScheduler *scheduler,
    uint8_t pid,
    uint32_t interval_ms,
    MblinkSchedulerPriority priority,
    uint64_t first_due_ms);

MblinkSchedulerResult mblink_scheduler_set_enabled(
    MblinkScheduler *scheduler, uint8_t pid, bool enabled);

MblinkSchedulerResult mblink_scheduler_configure_standard_obd2(
    MblinkScheduler *scheduler,
    const MblinkObd2PidSet *supported,
    uint64_t first_due_ms);

void mblink_scheduler_set_paused(
    MblinkScheduler *scheduler, bool paused, uint64_t now_ms);

MblinkSchedulerNextResult mblink_scheduler_next(
    const MblinkScheduler *scheduler,
    uint64_t now_ms,
    MblinkSchedulerDispatch *dispatch);

MblinkSchedulerResult mblink_scheduler_mark_dispatched(
    MblinkScheduler *scheduler, size_t index, uint64_t now_ms);

#ifdef __cplusplus
}
#endif

#endif
