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

static MblinkParameterKey mblink_scheduler_obd2_key(uint8_t pid)
{
    MblinkParameterKey key = {
        MBLINK_PARAMETER_PROTOCOL_OBD2,
        MBLINK_PARAMETER_MODULE_STANDARD_OBD2,
        (uint32_t)pid
    };
    return key;
}

static bool mblink_scheduler_key_to_pid(
    const MblinkParameterKey *key,
    uint8_t *pid)
{
    if (!mblink_parameter_key_is_valid(key) || pid == NULL ||
        key->protocol != MBLINK_PARAMETER_PROTOCOL_OBD2 ||
        key->module != MBLINK_PARAMETER_MODULE_STANDARD_OBD2 ||
        key->identifier > UINT8_MAX) {
        return false;
    }
    *pid = (uint8_t)key->identifier;
    return true;
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

MblinkSchedulerResult mblink_scheduler_add_parameter(
    MblinkScheduler *scheduler,
    const MblinkParameterKey *key,
    uint32_t interval_ms,
    MblinkSchedulerPriority priority,
    uint64_t first_due_ms)
{
    size_t index;
    MblinkSchedulerItem item;
    uint8_t pid = 0U;

    if (scheduler == NULL || !mblink_parameter_key_is_valid(key) ||
        interval_ms == 0U || !mblink_scheduler_priority_valid(priority)) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    for (index = 0U; index < scheduler->count; ++index) {
        if (mblink_parameter_key_equal(&scheduler->items[index].key, key)) {
            return MBLINK_SCHEDULER_RESULT_DUPLICATE;
        }
    }

    if (scheduler->count >= MBLINK_SCHEDULER_MAX_ITEMS) {
        return MBLINK_SCHEDULER_RESULT_FULL;
    }

    memset(&item, 0, sizeof(item));
    item.key = *key;
    item.pid_valid = mblink_scheduler_key_to_pid(key, &pid);
    item.pid = pid;
    item.interval_ms = interval_ms;
    item.next_due_ms = first_due_ms;
    item.priority = priority;
    item.enabled = true;
    scheduler->items[scheduler->count] = item;
    scheduler->count++;
    return MBLINK_SCHEDULER_RESULT_OK;
}

MblinkSchedulerResult mblink_scheduler_set_parameter_enabled(
    MblinkScheduler *scheduler,
    const MblinkParameterKey *key,
    bool enabled)
{
    size_t index;

    if (scheduler == NULL || !mblink_parameter_key_is_valid(key)) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    for (index = 0U; index < scheduler->count; ++index) {
        if (mblink_parameter_key_equal(&scheduler->items[index].key, key)) {
            scheduler->items[index].enabled = enabled;
            return MBLINK_SCHEDULER_RESULT_OK;
        }
    }
    return MBLINK_SCHEDULER_RESULT_NOT_FOUND;
}

MblinkSchedulerResult mblink_scheduler_add(
    MblinkScheduler *scheduler,
    uint8_t pid,
    uint32_t interval_ms,
    MblinkSchedulerPriority priority,
    uint64_t first_due_ms)
{
    const MblinkParameterKey key = mblink_scheduler_obd2_key(pid);
    return mblink_scheduler_add_parameter(
        scheduler, &key, interval_ms, priority, first_due_ms);
}

MblinkSchedulerResult mblink_scheduler_set_enabled(
    MblinkScheduler *scheduler, uint8_t pid, bool enabled)
{
    const MblinkParameterKey key = mblink_scheduler_obd2_key(pid);
    return mblink_scheduler_set_parameter_enabled(scheduler, &key, enabled);
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
    size_t index;

    if (scheduler == NULL || supported == NULL) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    mblink_scheduler_init(scheduler);
    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(plan); ++index) {
        MblinkSchedulerResult result;

        if (!mblink_obd2_pid_set_contains(supported, plan[index].pid)) {
            continue;
        }
        result = mblink_scheduler_add(scheduler,
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
    size_t index;
    uint64_t pause_duration = 0U;

    if (scheduler == NULL || scheduler->paused == paused) {
        return;
    }

    if (paused) {
        scheduler->paused = true;
        scheduler->pause_started_ms = now_ms;
        return;
    }

    if (now_ms >= scheduler->pause_started_ms) {
        pause_duration = now_ms - scheduler->pause_started_ms;
    }
    for (index = 0U; index < scheduler->count; ++index) {
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
    bool have_enabled = false;
    bool have_due = false;
    size_t selected = 0U;
    size_t index;
    uint64_t earliest_due = UINT64_MAX;

    if (dispatch != NULL) {
        memset(dispatch, 0, sizeof(*dispatch));
    }
    if (scheduler == NULL || dispatch == NULL) {
        return MBLINK_SCHEDULER_NEXT_INVALID_ARGUMENT;
    }
    if (scheduler->paused) {
        return MBLINK_SCHEDULER_NEXT_PAUSED;
    }

    for (index = 0U; index < scheduler->count; ++index) {
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
            item->next_due_ms < scheduler->items[selected].next_due_ms ||
            (item->next_due_ms == scheduler->items[selected].next_due_ms &&
             item->priority > scheduler->items[selected].priority) ||
            (item->next_due_ms == scheduler->items[selected].next_due_ms &&
             item->priority == scheduler->items[selected].priority &&
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
    dispatch->key = scheduler->items[selected].key;
    dispatch->pid = scheduler->items[selected].pid;
    dispatch->pid_valid = scheduler->items[selected].pid_valid;
    dispatch->due_ms = scheduler->items[selected].next_due_ms;
    dispatch->wait_ms = 0U;
    return MBLINK_SCHEDULER_NEXT_READY;
}

MblinkSchedulerResult mblink_scheduler_mark_dispatched(
    MblinkScheduler *scheduler, size_t index, uint64_t now_ms)
{
    MblinkSchedulerItem *item;
    uint64_t interval;
    uint64_t late_by;
    uint64_t steps;
    uint64_t advance;

    if (scheduler == NULL || index >= scheduler->count) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    item = &scheduler->items[index];
    interval = (uint64_t)item->interval_ms;
    if (interval == 0U) {
        return MBLINK_SCHEDULER_RESULT_INVALID_ARGUMENT;
    }

    if (item->next_due_ms > now_ms) {
        item->next_due_ms =
            infiltratr_u64_add_saturating(item->next_due_ms, interval);
        return MBLINK_SCHEDULER_RESULT_OK;
    }

    late_by = now_ms - item->next_due_ms;
    steps = infiltratr_u64_add_saturating(late_by / interval, 1U);
    advance = infiltratr_u64_multiply_saturating(interval, steps);
    item->next_due_ms =
        infiltratr_u64_add_saturating(item->next_due_ms, advance);
    return MBLINK_SCHEDULER_RESULT_OK;
}
