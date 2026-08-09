/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef APP_DEBUG_SNAPSHOT_H
#define APP_DEBUG_SNAPSHOT_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "configuration_runtime.h"
#include "hmi_model.h"
#include "input_diagnostics.h"
#include "runtime_control.h"

#define APP_PHASE1_DEBUG_SNAPSHOT_MAGIC         (0x50344A53UL) /* "P4JS" */

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
    AppHmiState hmi;
    ConfigurationRuntimeState configuration;
    JoystickProcessedSample processed_joystick;
    AppRequestedDriveCommand requested_command;
    AppOperatingMode active_mode;
    AppSafetyState safety_state;
    AppFaultMask active_faults;
    bool logical_authorized;
    CommandAuthorizationBlockMask authorization_blocks;
    AppAuthorizedDriveCommand published_command;
} AppPhase1DebugSnapshot;

/* Debugger mirror only. Application decisions must never read this object. */
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
    const AppHmiState *hmi,
    const ConfigurationRuntimeState *configuration,
    const RuntimeControlContext *runtime_context,
    const RuntimeControlOutput *runtime_output);

#endif /* APP_DEBUG_SNAPSHOT_H */
