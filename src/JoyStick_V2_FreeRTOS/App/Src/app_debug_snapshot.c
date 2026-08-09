/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
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
    const AppHmiState *hmi,
    const ConfigurationRuntimeState *configuration,
    const RuntimeControlContext *runtime_context,
    const RuntimeControlOutput *runtime_output)
{
    uint32_t sequence;

    if ((raw_input == NULL) || (input_diagnostics == NULL) ||
        (motor_link == NULL) || (hmi == NULL) || (configuration == NULL) ||
        (runtime_context == NULL) || (runtime_output == NULL))
    {
        return;
    }

    /* Odd means an update is in progress; even means a coherent snapshot. */
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
    g_app_phase1_debug_snapshot.hmi = *hmi;
    g_app_phase1_debug_snapshot.configuration = *configuration;
    g_app_phase1_debug_snapshot.processed_joystick = runtime_output->joystick;
    g_app_phase1_debug_snapshot.requested_command = runtime_output->requested_command;
    g_app_phase1_debug_snapshot.active_mode = runtime_output->active_mode;
    g_app_phase1_debug_snapshot.safety_state = runtime_context->safety.state;
    g_app_phase1_debug_snapshot.active_faults = runtime_context->safety.active_faults;
    g_app_phase1_debug_snapshot.logical_authorized =
        runtime_output->authorization.logical_authorized;
    g_app_phase1_debug_snapshot.authorization_blocks =
        runtime_output->authorization.blocking_reasons;
    g_app_phase1_debug_snapshot.published_command =
        runtime_output->authorization.physical_command;

    g_app_phase1_debug_snapshot.snapshot_sequence = sequence + 1U;
}
