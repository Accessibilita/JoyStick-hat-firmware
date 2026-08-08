#include "app_tasks.h"

#include "stm32f4xx_hal.h"

#include "app_build_config.h"
#include "health_monitor.h"
#include "FreeRTOS.h"
#include "task.h"

void DiagnosticsTask_Run(void *argument)
{
    TickType_t last_wake_time;

    (void)argument;
    last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        const uint32_t now_ms = HAL_GetTick();

        /*
         * Future work: copy bounded diagnostic snapshots to SWO or a service
         * interface. No formatting or blocking transport belongs in a critical
         * task.
         */
        HealthMonitor_RecordProgress(APP_HEALTH_DIAGNOSTICS_TASK, now_ms);

        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(APP_DIAGNOSTICS_TASK_PERIOD_MS));
    }
}
