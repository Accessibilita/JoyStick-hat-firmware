#ifndef APP_DEBUG_SNAPSHOT_H
#define APP_DEBUG_SNAPSHOT_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "input_diagnostics.h"

#define APP_PHASE1_DEBUG_SNAPSHOT_MAGIC         (0x50314A53UL) /* "P1JS" */

typedef struct
{
    uint32_t magic;
    uint32_t snapshot_sequence;
    uint32_t safety_loop_count;
    uint32_t now_ms;
    uint32_t adc_notification_count;
    bool adc_batch_copied;
    bool mandatory_tasks_healthy;
    AppRawInputSnapshot raw_input;
    InputDiagnosticResult input_diagnostics;
    AppMotorLinkStatus motor_link;
    AppSafetyState safety_state;
    AppFaultMask active_faults;
    AppAuthorizedDriveCommand published_command;
} AppPhase1DebugSnapshot;

/*
 * Debugger-facing mirror only. Application decisions must never read this
 * object because a debugger can observe it mid-update.
 */
extern volatile AppPhase1DebugSnapshot g_app_phase1_debug_snapshot;

void AppDebugSnapshot_Init(void);
void AppDebugSnapshot_Publish(
    uint32_t now_ms,
    uint32_t adc_notification_count,
    bool adc_batch_copied,
    bool mandatory_tasks_healthy,
    const AppRawInputSnapshot *raw_input,
    const InputDiagnosticResult *input_diagnostics,
    const AppMotorLinkStatus *motor_link,
    const AppSafetyContext *safety_context,
    const AppAuthorizedDriveCommand *published_command);

#endif /* APP_DEBUG_SNAPSHOT_H */
