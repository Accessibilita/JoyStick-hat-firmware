#include "app_tasks.h"

#include "stm32f4xx_hal.h"

#include <stddef.h>

#include "app_build_config.h"
#include "app_debug_snapshot.h"
#include "app_rtos.h"
#include "health_monitor.h"
#include "input_acquisition.h"
#include "input_diagnostics.h"
#include "safety_state.h"
#include "watchdog_supervision.h"
#include "FreeRTOS.h"
#include "task.h"

static AppAuthorizedDriveCommand SafetyControl_MakeInhibitedCommand(
    const AppSafetyContext *context,
    const AppRawInputSnapshot *input,
    uint32_t now_ms)
{
    static uint32_t publication_sequence = 0U;
    AppAuthorizedDriveCommand command;

    publication_sequence++;
    command.publication_sequence = publication_sequence;
    command.input_sequence = (input != NULL) ? input->sequence : 0U;
    command.generated_at_ms = now_ms;
    command.forward_q15 = 0;
    command.turn_q15 = 0;
    command.maximum_speed_q15 = 0U;
    command.safety_state =
        (context != NULL) ? context->state : APP_SAFETY_LATCHED_FAULT;
    command.active_faults =
        (context != NULL) ? context->active_faults : APP_FAULT_INTERNAL_INVARIANT;
    command.drive_authorized = false;

    return command;
}

void SafetyControlTask_Run(void *argument)
{
    AppSafetyContext safety_context;
    AppRawInputSnapshot input_snapshot = {0};
    AppMotorLinkStatus link_status = {0};
    uint32_t now_ms;

    (void)argument;

    now_ms = HAL_GetTick();
    SafetyState_Init(&safety_context, now_ms);
    HealthMonitor_Init(now_ms);
    WatchdogSupervision_Init();
    AppDebugSnapshot_Init();

    if (!InputAcquisition_Start())
    {
        safety_context.active_faults |= APP_FAULT_INTERNAL_INVARIANT;
        safety_context.state = APP_SAFETY_LATCHED_FAULT;
    }

    for (;;)
    {
        InputDiagnosticResult diagnostic_result;
        AppSafetyObservation observation;
        AppAuthorizedDriveCommand command;
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

        diagnostic_result = InputDiagnostics_Evaluate(
            &input_snapshot,
            now_ms);

        if (!AppRtos_ReadMotorLinkStatus(&link_status))
        {
            link_status.link_valid = false;
            link_status.last_valid_packet_ms = 0U;
        }

        observation.configuration_valid = false;
        observation.input_valid = diagnostic_result.valid;
        observation.input_neutral = diagnostic_result.neutral;
        observation.link_valid = link_status.link_valid;
        observation.link_fresh = link_status.link_valid &&
            ((now_ms - link_status.last_valid_packet_ms) <= APP_LINK_MAX_AGE_MS);
        observation.enable_request = false;
        mandatory_tasks_healthy =
            HealthMonitor_AreMandatoryChannelsHealthy(now_ms);
        observation.mandatory_tasks_healthy = mandatory_tasks_healthy;
        observation.now_ms = now_ms;
        observation.observed_faults = diagnostic_result.faults;

        SafetyState_Step(&safety_context, &observation);

        command = SafetyControl_MakeInhibitedCommand(
            &safety_context,
            &input_snapshot,
            now_ms);
        (void)AppRtos_PublishAuthorizedCommand(&command);

        AppDebugSnapshot_Publish(
            now_ms,
            notification_count,
            adc_batch_copied,
            mandatory_tasks_healthy,
            &input_snapshot,
            &diagnostic_result,
            &link_status,
            &safety_context,
            &command);

        HealthMonitor_RecordProgress(APP_HEALTH_SAFETY_TASK, now_ms);

        WatchdogSupervision_RefreshIfHealthy(
            HealthMonitor_AreMandatoryChannelsHealthy(now_ms),
            safety_context.state != APP_SAFETY_LATCHED_FAULT);
    }
}
