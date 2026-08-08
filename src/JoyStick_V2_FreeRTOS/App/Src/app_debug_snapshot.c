#include "app_debug_snapshot.h"

#include <stddef.h>

volatile AppPhase1DebugSnapshot g_app_phase1_debug_snapshot;

void AppDebugSnapshot_Init(void)
{
    g_app_phase1_debug_snapshot.magic = APP_PHASE1_DEBUG_SNAPSHOT_MAGIC;
    g_app_phase1_debug_snapshot.snapshot_sequence = 0U;
    g_app_phase1_debug_snapshot.safety_loop_count = 0U;
}

void AppDebugSnapshot_Publish(
    uint32_t now_ms,
    uint32_t adc_notification_count,
    bool adc_batch_copied,
    bool mandatory_tasks_healthy,
    const AppRawInputSnapshot *raw_input,
    const InputDiagnosticResult *input_diagnostics,
    const AppMotorLinkStatus *motor_link,
    const AppSafetyContext *safety_context,
    const AppAuthorizedDriveCommand *published_command)
{
    uint32_t sequence;

    if ((raw_input == NULL) ||
        (input_diagnostics == NULL) ||
        (motor_link == NULL) ||
        (safety_context == NULL) ||
        (published_command == NULL))
    {
        return;
    }

    /*
     * Odd sequence means an update is in progress; even means complete. This is
     * mainly useful when a live debugger reads memory while the core is running.
     */
    sequence = g_app_phase1_debug_snapshot.snapshot_sequence + 1U;
    if ((sequence & 1U) == 0U)
    {
        sequence++;
    }
    g_app_phase1_debug_snapshot.snapshot_sequence = sequence;

    g_app_phase1_debug_snapshot.safety_loop_count++;
    g_app_phase1_debug_snapshot.now_ms = now_ms;
    g_app_phase1_debug_snapshot.adc_notification_count = adc_notification_count;
    g_app_phase1_debug_snapshot.adc_batch_copied = adc_batch_copied;
    g_app_phase1_debug_snapshot.mandatory_tasks_healthy = mandatory_tasks_healthy;
    g_app_phase1_debug_snapshot.raw_input = *raw_input;
    g_app_phase1_debug_snapshot.input_diagnostics = *input_diagnostics;
    g_app_phase1_debug_snapshot.motor_link = *motor_link;
    g_app_phase1_debug_snapshot.safety_state = safety_context->state;
    g_app_phase1_debug_snapshot.active_faults = safety_context->active_faults;
    g_app_phase1_debug_snapshot.published_command = *published_command;

    g_app_phase1_debug_snapshot.snapshot_sequence = sequence + 1U;
}
