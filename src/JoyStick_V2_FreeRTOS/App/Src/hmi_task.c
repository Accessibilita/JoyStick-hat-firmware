/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "app_tasks.h"

#include "stm32f4xx_hal.h"

#include "app_build_config.h"
#include "app_rtos.h"
#include "board_io.h"
#include "health_monitor.h"
#include "hmi_model.h"
#include "FreeRTOS.h"
#include "task.h"

void HmiTask_Run(void *argument)
{
    HmiModelContext hmi_context;
    AppHmiState hmi_state = {0};
    TickType_t last_wake_time;

    (void)argument;
    HmiModel_Init(&hmi_context);
    last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        const uint32_t now_ms = HAL_GetTick();

        /*
         * Phase 4 debounces and publishes the physical HMI inputs, but does not
         * invent final button/rotary drive semantics before bench validation.
         */
        HmiModel_Step(
            &hmi_context,
            BoardIo_ReadButtonsActiveLow(),
            BoardIo_ReadRotary1ActiveLow(),
            BoardIo_ReadRotary2ActiveLow(),
            now_ms,
            &hmi_state);
        (void)AppRtos_PublishHmiState(&hmi_state);

        /* LED drivers remain blanked until their complete driver is verified. */
        BoardIo_ForceLedHudBlanked();
        HealthMonitor_RecordProgress(APP_HEALTH_HMI_TASK, now_ms);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(APP_HMI_TASK_PERIOD_MS));
    }
}
