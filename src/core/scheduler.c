// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file scheduler.c
 * @brief Portable live diagnostic request scheduler.
 */
#include "mblink/scheduler.h"

#include "infiltratr/core.h"

#include <string.h>

static bool mblink_scheduler_priority_valid(MblinkSchedulerPriority priority)
{
    return priority >= MBLINK_SCHEDULER_PRIORITY_LOW &&
           priority <= MBLINK_SCHEDULER_PRIORITY_CRITICAL;
}

const char *mblink_scheduler_result_name(MblinkSchedulerResult result)
{
    switch (result) {
    case MBLINK_SCHEDULER_RESULT_OK: return "ok";
    case MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT: return "invalid-argument";
    case MBLINK_SCHEDULER_RESULT_FULL: return "full";
    case MBLINK_SCHEDULER_RESULT_DUPLICATE: return "duplicate";
    case MBLINK_SCHEDULER_RESULT_NOT_FOUND: return "not-found";
    }
    return "unknown";
}

const char *mblink_scheduler_next_result_name(MblinkSchedulerNextResult result)
{
    switch (result) {
    case MBLINK_SCHEDULER_NEXT_READY: return "ready";
    case MBLINK_SCHEDULER_NEXT_WAITING: return "waiting";
    case MBLINK_SCHEDULER_NEXT_PAUSED: return "paused";
    case MBLINK_SCHEDULER_NEXT_EMPTY: return "empty";
    case MBLINK_SCHEDULER_NEXT_INVALID_ARGUMENT: return "invalid-argument";
    }
    return "unknown";
}

void mblink_scheduler_init(MblinkScheduler *scheduler)
{
    if (scheduler != NULL) {
        memset(scheduler, 0, sizeof(*scheduler));
    }
}

MblinkSchedulerResult mblink_scheduler_add(
    MblinkScheduler *scheduler,
    uint8_t pid,
    uint32_t interval_ms,
    MblinkSchedulerPriority priority,
    uint64_t first_due_ms)
{
    if (scheduler == NULL || interval_ms == 0U ||
        !mblink_scheduler_priority_valid(priority)) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    for (size_t index = 0U; index < scheduler->count; ++index) {
        if (scheduler->items[index].pid == pid) {
            return MBLINK_SCHEDULER_RESULT_DUPLICATE;
        }
    }

    if (scheduler->count >= MBLINK_SCHEDULER_MAX_ITEMS) {
        return MBLINK_SCHEDULER_RESULT_FULL;
    }

    MblinkSchedulerItem *item = &scheduler->items[scheduler->count++];
    item->pid = pid;
    item->interval_ms = interval_ms;
    item->next_due_ms = first_due_ms;
    item->priority = priority;
    item->enabled = true;
    return MBLINK_SCHEDULER_RESULT_OK;
}

MblinkSchedulerResult mblink_scheduler_set_enabled(
    MblinkScheduler *scheduler, uint8_t pid, bool enabled)
{
    if (scheduler == NULL) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    for (size_t index = 0U; index < scheduler->count; ++index) {
        if (scheduler->items[index].pid == pid) {
            scheduler->items[index].enabled = enabled;
            return MBLINK_SCHEDULER_RESULT_OK;
        }
    }
    return MBLINK_SCHEDULER_RESULT_NOT_FOUND;
}

typedef struct {
    uint8_t pid;
    uint32_t interval_ms;
    MblinkSchedulerPriority priority;
} MblinkStandardSchedule;

MblinkSchedulerResult mblink_scheduler_configure_standard_obd2(
    MblinkScheduler *scheduler,
    const MblinkObd2PidSet *supported,
    uint64_t first_due_ms)
{
    static const MblinkStandardSchedule plan[] = {
        { 0x0cU, 250U,  MBLINK_SCHEDULER_PRIORITY_CRITICAL },
        { 0x0dU, 500U,  MBLINK_SCHEDULER_PRIORITY_HIGH },
        { 0x0bU, 500U,  MBLINK_SCHEDULER_PRIORITY_HIGH },
        { 0x11U, 500U,  MBLINK_SCHEDULER_PRIORITY_HIGH },
        { 0x04U, 750U,  MBLINK_SCHEDULER_PRIORITY_NORMAL },
        { 0x10U, 750U,  MBLINK_SCHEDULER_PRIORITY_NORMAL },
        { 0x05U, 1500U, MBLINK_SCHEDULER_PRIORITY_LOW },
        { 0x0fU, 1500U, MBLINK_SCHEDULER_PRIORITY_LOW }
    };

    if (scheduler == NULL || supported == NULL) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    mblink_scheduler_init(scheduler);
    for (size_t index = 0U;
         index < sizeof(plan) / sizeof(plan[0]);
         ++index) {
        if (!mblink_obd2_pid_set_contains(supported, plan[index].pid)) {
            continue;
        }
        MblinkSchedulerResult result =
            mblink_scheduler_add(scheduler,
                                 plan[index].pid,
                                 plan[index].interval_ms,
                                 plan[index].priority,
                                 first_due_ms);
        if (result != MBLINK_SCHEDULER_RESULT_OK) {
            mblink_scheduler_init(scheduler);
            return result;
        }
    }

    return MBLINK_SCHEDULER_RESULT_OK;
}

void mblink_scheduler_set_paused(
    MblinkScheduler *scheduler, bool paused, uint64_t now_ms)
{
    if (scheduler == NULL || scheduler->paused == paused) {
        return;
    }

    if (paused) {
        scheduler->paused = true;
        scheduler->pause_started_ms = now_ms;
        return;
    }

    uint64_t pause_duration = 0U;
    if (now_ms >= scheduler->pause_started_ms) {
        pause_duration = now_ms - scheduler->pause_started_ms;
    }
    for (size_t index = 0U; index < scheduler->count; ++index) {
        scheduler->items[index].next_due_ms =
            infiltratr_u64_add_saturating(
                scheduler->items[index].next_due_ms, pause_duration);
    }
    scheduler->paused = false;
    scheduler->pause_started_ms = 0U;
}

MblinkSchedulerNextResult mblink_scheduler_next(
    const MblinkScheduler *scheduler,
    uint64_t now_ms,
    MblinkSchedulerDispatch *dispatch)
{
    if (dispatch != NULL) {
        memset(dispatch, 0, sizeof(*dispatch));
    }
    if (scheduler == NULL || dispatch == NULL) {
        return MBLINK_SCHEDULER_NEXT_INVALID_ARGUMENT;
    }
    if (scheduler->paused) {
        return MBLINK_SCHEDULER_NEXT_PAUSED;
    }

    bool have_enabled = false;
    bool have_due = false;
    size_t selected = 0U;
    uint64_t earliest_due = UINT64_MAX;

    for (size_t index = 0U; index < scheduler->count; ++index) {
        const MblinkSchedulerItem *item = &scheduler->items[index];
        if (!item->enabled) {
            continue;
        }

        if (!have_enabled || item->next_due_ms < earliest_due) {
            earliest_due = item->next_due_ms;
        }
        have_enabled = true;

        if (item->next_due_ms > now_ms) {
            continue;
        }

        if (!have_due ||
            item->priority > scheduler->items[selected].priority ||
            (item->priority == scheduler->items[selected].priority &&
             item->next_due_ms < scheduler->items[selected].next_due_ms) ||
            (item->priority == scheduler->items[selected].priority &&
             item->next_due_ms == scheduler->items[selected].next_due_ms &&
             index < selected)) {
            selected = index;
            have_due = true;
        }
    }

    if (!have_enabled) {
        return MBLINK_SCHEDULER_NEXT_EMPTY;
    }

    if (!have_due) {
        dispatch->due_ms = earliest_due;
        dispatch->wait_ms = earliest_due > now_ms ? earliest_due - now_ms : 0U;
        return MBLINK_SCHEDULER_NEXT_WAITING;
    }

    dispatch->index = selected;
    dispatch->pid = scheduler->items[selected].pid;
    dispatch->due_ms = scheduler->items[selected].next_due_ms;
    dispatch->wait_ms = 0U;
    return MBLINK_SCHEDULER_NEXT_READY;
}

MblinkSchedulerResult mblink_scheduler_mark_dispatched(
    MblinkScheduler *scheduler, size_t index, uint64_t now_ms)
{
    if (scheduler == NULL || index >= scheduler->count) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    MblinkSchedulerItem *item = &scheduler->items[index];
    const uint64_t interval = (uint64_t)item->interval_ms;
    if (interval == 0U) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    if (item->next_due_ms > now_ms) {
        item->next_due_ms =
            infiltratr_u64_add_saturating(item->next_due_ms, interval);
        return MBLINK_SCHEDULER_RESULT_OK;
    }

    const uint64_t late_by = now_ms - item->next_due_ms;
    const uint64_t steps = infiltratr_u64_add_saturating(
        late_by / interval, 1U);
    const uint64_t advance =
        infiltratr_u64_multiply_saturating(interval, steps);
    item->next_due_ms =
        infiltratr_u64_add_saturating(item->next_due_ms, advance);
    return MBLINK_SCHEDULER_RESULT_OK;
}
