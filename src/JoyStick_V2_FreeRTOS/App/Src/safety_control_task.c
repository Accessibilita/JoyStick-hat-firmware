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
#include "app_debug_snapshot.h"
#include "app_rtos.h"
#include "configuration_runtime.h"
#include "configuration_storage.h"
#include "health_monitor.h"
#include "hmi_model.h"
#include "input_acquisition.h"
#include "input_diagnostics.h"
#include "runtime_control.h"
#include "watchdog_supervision.h"
#include "FreeRTOS.h"
#include "task.h"

void SafetyControlTask_Run(void *argument)
{
    RuntimeControlContext runtime_context;
    ConfigurationRuntimeState configuration;
    AppRawInputSnapshot input_snapshot = {0};
    AppMotorLinkStatus link_status = {0};
    AppHmiState hmi_state = {0};
    uint32_t now_ms;

    (void)argument;

    now_ms = HAL_GetTick();
    RuntimeControl_Init(&runtime_context, now_ms);
    ConfigurationRuntime_Init(&configuration);
    (void)ConfigurationRuntime_Load(&configuration, ConfigurationStorage_ReadSlot);
    HealthMonitor_Init(now_ms);
    WatchdogSupervision_Init();
    AppDebugSnapshot_Init();

    /*
     * The target HMI mapping is deliberately invalid until its physical
     * controls have been characterized.  Absence of a mailbox update therefore
     * cannot accidentally look like an enable request.
     */
    hmi_state.requested_mode = APP_OPERATING_MODE_BOOT;
    hmi_state.maximum_speed_q15 = 0U;
    hmi_state.enable_request = false;
    hmi_state.control_mapping_valid = false;

    if (!InputAcquisition_Start())
    {
        runtime_context.safety.active_faults |= APP_FAULT_INTERNAL_INVARIANT;
        runtime_context.safety.state = APP_SAFETY_LATCHED_FAULT;
    }

    for (;;)
    {
        InputDiagnosticResult diagnostic_result;
        RuntimeControlInput runtime_input;
        RuntimeControlOutput runtime_output = {0};
        bool adc_batch_copied = false;
        bool mandatory_tasks_healthy;
        const uint32_t notification_count = ulTaskNotifyTake(
            pdTRUE,
            pdMS_TO_TICKS(APP_SAFETY_PERIOD_MS));

        now_ms = HAL_GetTick();

        if (notification_count > 0U)
        {
            adc_batch_copied = InputAcquisition_CopyCompletedBatch(&input_snapshot);
        }

        diagnostic_result = InputDiagnostics_Evaluate(&input_snapshot, now_ms);

        if (!AppRtos_ReadMotorLinkStatus(&link_status))
        {
            link_status.link_valid = false;
            link_status.last_valid_packet_ms = 0U;
        }

        if (!AppRtos_ReadHmiState(&hmi_state))
        {
            hmi_state.maximum_speed_q15 = 0U;
            hmi_state.enable_request = false;
            hmi_state.control_mapping_valid = false;
        }

        mandatory_tasks_healthy =
            HealthMonitor_AreMandatoryChannelsHealthy(now_ms);

        runtime_input.raw_input = &input_snapshot;
        runtime_input.input_diagnostics = &diagnostic_result;
        runtime_input.configuration = &configuration;
        runtime_input.hmi = &hmi_state;
        runtime_input.motor_link = &link_status;
        runtime_input.now_ms = now_ms;
        runtime_input.mandatory_tasks_healthy = mandatory_tasks_healthy;

        if (!RuntimeControl_Step(
                &runtime_context,
                &runtime_input,
                &runtime_output))
        {
            runtime_context.safety.active_faults |= APP_FAULT_INTERNAL_INVARIANT;
            runtime_context.safety.state = APP_SAFETY_LATCHED_FAULT;
        }

        /*
         * Phase-2 authorization still owns the physical boundary.  The command
         * published here is the already-inhibited physical command, never the
         * requested joystick command.
         */
        (void)AppRtos_PublishAuthorizedCommand(
            &runtime_output.authorization.physical_command);

        AppDebugSnapshot_Publish(
            now_ms,
            notification_count,
            adc_batch_copied,
            mandatory_tasks_healthy,
            &input_snapshot,
            &diagnostic_result,
            &link_status,
            &hmi_state,
            &configuration,
            &runtime_context,
            &runtime_output);

        HealthMonitor_RecordProgress(APP_HEALTH_SAFETY_TASK, now_ms);
        WatchdogSupervision_RefreshIfHealthy(
            HealthMonitor_AreMandatoryChannelsHealthy(now_ms),
            runtime_context.safety.state != APP_SAFETY_LATCHED_FAULT);
    }
}
