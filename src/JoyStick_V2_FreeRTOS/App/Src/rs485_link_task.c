#include "app_tasks.h"

#include "stm32f4xx_hal.h"

#include "app_build_config.h"
#include "app_rtos.h"
#include "board_io.h"
#include "health_monitor.h"
#include "FreeRTOS.h"
#include "task.h"

#if APP_RS485_PHYSICAL_LINK_ENABLE != 0U
#error "Phase 1 must not enable the present PCB RS-485 data connection."
#endif

void Rs485LinkTask_Run(void *argument)
{
    TickType_t last_wake_time;
    AppMotorLinkStatus link_status = {0};

    (void)argument;
    last_wake_time = xTaskGetTickCount();

    BoardIo_ForceRs485SafeDisabled();

    for (;;)
    {
        const uint32_t now_ms = HAL_GetTick();

        /*
         * The physical data direction mismatch is a hardware blocker. Keep the
         * link explicitly invalid so the safety state machine cannot authorize
         * motion, while still exercising task scheduling and health monitoring.
         */
        link_status.receive_sequence++;
        link_status.last_valid_packet_ms = 0U;
        link_status.remote_faults = APP_FAULT_LINK_INVALID;
        link_status.link_valid = false;
        link_status.remote_drive_ready = false;
        link_status.brakes_confirmed = false;

        (void)AppRtos_PublishMotorLinkStatus(&link_status);
        HealthMonitor_RecordProgress(APP_HEALTH_LINK_TASK, now_ms);

        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(APP_RS485_TASK_PERIOD_MS));
    }
}
