/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef RUNTIME_CONTROL_H
#define RUNTIME_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

#include "command_authorization.h"
#include "configuration_runtime.h"
#include "hmi_model.h"
#include "input_diagnostics.h"
#include "joystick_processing.h"
#include "operating_mode.h"
#include "safety_state.h"

typedef struct
{
    AppSafetyContext safety;
    OperatingModeContext operating_mode;
    uint32_t request_sequence;
} RuntimeControlContext;

typedef struct
{
    const AppRawInputSnapshot *raw_input;
    const InputDiagnosticResult *input_diagnostics;
    const ConfigurationRuntimeState *configuration;
    const AppHmiState *hmi;
    const AppMotorLinkStatus *motor_link;
    uint32_t now_ms;
    bool mandatory_tasks_healthy;
} RuntimeControlInput;

typedef struct
{
    AppSafetyObservation observation;
    JoystickProcessedSample joystick;
    AppRequestedDriveCommand requested_command;
    CommandAuthorizationResult authorization;
    bool joystick_processed;
    bool configuration_valid;
    bool hmi_mapping_valid;
    AppOperatingMode active_mode;
} RuntimeControlOutput;

/* Initialize integrated safety/mode state without creating drive authority. */
void RuntimeControl_Init(RuntimeControlContext *context, uint32_t now_ms);

/* Execute one bounded integration step from current input state to authorization. */
bool RuntimeControl_Step(
    RuntimeControlContext *context,
    const RuntimeControlInput *input,
    RuntimeControlOutput *output);

#endif /* RUNTIME_CONTROL_H */
