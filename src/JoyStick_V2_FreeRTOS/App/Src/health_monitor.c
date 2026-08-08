#include "health_monitor.h"

#include <stddef.h>

#include "app_build_config.h"

/*
 * Each task is the sole writer of its own aligned 32-bit record fields. The
 * safety task only performs aligned 32-bit reads. On the Cortex-M4 these
 * accesses are indivisible. Volatile prevents the compiler from caching a
 * value across health checks; this is deliberately not used as a substitute
 * for multi-field transactional synchronization.
 */
static volatile uint32_t s_progress_counter[APP_HEALTH_COUNT];
static volatile uint32_t s_last_progress_ms[APP_HEALTH_COUNT];
static const uint32_t s_deadline_ms[APP_HEALTH_COUNT] =
{
    [APP_HEALTH_SAFETY_TASK] = 20U,
    [APP_HEALTH_LINK_TASK] = 50U,
    [APP_HEALTH_HMI_TASK] = 100U,
    [APP_HEALTH_DIAGNOSTICS_TASK] = 500U
};
static const bool s_mandatory[APP_HEALTH_COUNT] =
{
    [APP_HEALTH_SAFETY_TASK] = true,
    [APP_HEALTH_LINK_TASK] = true,
    [APP_HEALTH_HMI_TASK] = true,
    [APP_HEALTH_DIAGNOSTICS_TASK] = false
};

void HealthMonitor_Init(uint32_t now_ms)
{
    uint32_t index;

    for (index = 0U; index < (uint32_t)APP_HEALTH_COUNT; index++)
    {
        s_progress_counter[index] = 0U;
        s_last_progress_ms[index] = now_ms;
    }
}

void HealthMonitor_RecordProgress(AppHealthChannel channel, uint32_t now_ms)
{
    const uint32_t index = (uint32_t)channel;

    if (index < (uint32_t)APP_HEALTH_COUNT)
    {
        s_progress_counter[index]++;
        s_last_progress_ms[index] = now_ms;
    }
}

bool HealthMonitor_AreMandatoryChannelsHealthy(uint32_t now_ms)
{
    uint32_t index;

    if (now_ms < APP_WATCHDOG_STARTUP_GRACE_MS)
    {
        return true;
    }

    for (index = 0U; index < (uint32_t)APP_HEALTH_COUNT; index++)
    {
        const uint32_t last_progress_ms = s_last_progress_ms[index];

        if (s_mandatory[index] &&
            ((now_ms - last_progress_ms) > s_deadline_ms[index]))
        {
            return false;
        }
    }

    return true;
}

bool HealthMonitor_CopyRecord(AppHealthChannel channel, AppHealthRecord *record)
{
    const uint32_t index = (uint32_t)channel;

    if ((index >= (uint32_t)APP_HEALTH_COUNT) || (record == NULL))
    {
        return false;
    }

    record->progress_counter = s_progress_counter[index];
    record->last_progress_ms = s_last_progress_ms[index];
    record->deadline_ms = s_deadline_ms[index];
    record->mandatory = s_mandatory[index];
    return true;
}
