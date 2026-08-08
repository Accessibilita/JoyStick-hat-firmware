#include "app_tasks.h"

#include "stm32f4xx_hal.h"

#include "app_build_config.h"
#include "board_io.h"
#include "health_monitor.h"
#include "FreeRTOS.h"
#include "task.h"

void HmiTask_Run(void *argument)
{
    TickType_t last_wake_time;

    (void)argument;
    last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        const uint32_t now_ms = HAL_GetTick();

        /* LED drivers remain blanked until their complete driver is verified. */
        BoardIo_ForceLedHudBlanked();
        HealthMonitor_RecordProgress(APP_HEALTH_HMI_TASK, now_ms);

        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(APP_HMI_TASK_PERIOD_MS));
    }
}
