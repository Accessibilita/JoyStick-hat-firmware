/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "runtime_control.h"

#include <stddef.h>

#include "app_build_config.h"

static uint16_t RuntimeControl_MinU16(uint16_t a, uint16_t b)
{
    return (a < b) ? a : b;
}

/*
 * Operating-mode selection must be based on the state we can observe in this
 * control iteration, not on the previous SafetyState active-fault snapshot.
 *
 * SafetyState_Init() deliberately starts with configuration/link faults set so
 * startup is fail-closed.  Reusing that bootstrap mask before current inputs
 * are evaluated would make the first otherwise-valid Phase-4 iteration look
 * faulted and would incorrectly force the operating-mode model into FAULT.
 *
 * This helper mirrors the current externally observable prerequisites needed
 * for mode selection.  A latched SafetyState fault is also carried forward so
 * an internal invariant cannot be hidden merely because fresh inputs look good.
 */
static AppFaultMask RuntimeControl_DeriveCurrentModeFaults(
    const RuntimeControlContext *context,
    const RuntimeControlInput *input)
{
    AppFaultMask faults = input->input_diagnostics->faults;

    if (!input->configuration->valid)
    {
        faults |= APP_FAULT_CONFIGURATION_INVALID;
    }

    if (!input->input_diagnostics->valid)
    {
        faults |= APP_FAULT_INPUT_INVALID;
    }

    if (!input->motor_link->link_valid)
    {
        faults |= APP_FAULT_LINK_INVALID;
    }
    else if ((input->now_ms - input->motor_link->last_valid_packet_ms) >
             APP_LINK_MAX_AGE_MS)
    {
        faults |= APP_FAULT_LINK_STALE;
    }

    if (!input->mandatory_tasks_healthy)
    {
        faults |= APP_FAULT_TASK_HEALTH;
    }

    if (input->motor_link->remote_faults != APP_FAULT_NONE)
    {
        faults |= APP_FAULT_REMOTE_CONTROLLER;
    }

    if (context->safety.state == APP_SAFETY_LATCHED_FAULT)
    {
        faults |= context->safety.active_faults;
        faults |= APP_FAULT_INTERNAL_INVARIANT;
    }

    return faults;
}

static void RuntimeControl_MakeZeroRequest(
    const RuntimeControlInput *input,
    uint32_t request_sequence,
    AppRequestedDriveCommand *request)
{
    request->request_sequence = request_sequence;
    request->input_sequence = input->raw_input->sequence;
    request->generated_at_ms = input->now_ms;
    request->forward_q15 = 0;
    request->turn_q15 = 0;
    request->maximum_speed_q15 = 0U;
    request->enable_request = false;
}

void RuntimeControl_Init(RuntimeControlContext *context, uint32_t now_ms)
{
    if (context == NULL)
    {
        return;
    }

    SafetyState_Init(&context->safety, now_ms);
    OperatingMode_Init(&context->operating_mode);
    context->request_sequence = 0U;
}

bool RuntimeControl_Step(
    RuntimeControlContext *context,
    const RuntimeControlInput *input,
    RuntimeControlOutput *output)
{
    JoystickConfigurationImage effective_configuration;
    CommandAuthorizationInput authorization_input;
    OperatingModeInput mode_input;
    uint16_t mode_speed_ceiling_q15;
    bool request_built = false;

    if ((context == NULL) || (input == NULL) || (output == NULL) ||
        (input->raw_input == NULL) || (input->input_diagnostics == NULL) ||
        (input->configuration == NULL) || (input->hmi == NULL) ||
        (input->motor_link == NULL))
    {
        return false;
    }

    context->request_sequence++;
    output->joystick.x_q15 = 0;
    output->joystick.y_q15 = 0;
    output->joystick.shaped_x_q15 = 0;
    output->joystick.shaped_y_q15 = 0;
    output->joystick.neutral = false;
    output->configuration_valid = input->configuration->valid;
    output->hmi_mapping_valid = input->hmi->control_mapping_valid;
    output->joystick_processed = false;

    RuntimeControl_MakeZeroRequest(
        input,
        context->request_sequence,
        &output->requested_command);

    mode_input.requested_mode = input->hmi->requested_mode;
    mode_input.active_faults = RuntimeControl_DeriveCurrentModeFaults(
        context,
        input);
    mode_input.configuration_valid = input->configuration->valid;
    mode_input.input_neutral = input->input_diagnostics->neutral;
    mode_input.enable_request = input->hmi->enable_request;
    OperatingMode_Step(&context->operating_mode, &mode_input);
    output->active_mode = context->operating_mode.active_mode;
    mode_speed_ceiling_q15 = OperatingMode_GetSpeedCeilingQ15(output->active_mode);

    if (input->configuration->valid && input->input_diagnostics->valid)
    {
        effective_configuration = input->configuration->active_image;

        /*
         * Both HMI policy and operating mode may only reduce the calibrated
         * maximum. Neither can expand the stored speed ceiling.
         */
        effective_configuration.configuration.maximum_speed_q15 =
            RuntimeControl_MinU16(
                effective_configuration.configuration.maximum_speed_q15,
                input->hmi->maximum_speed_q15);
        effective_configuration.configuration.maximum_speed_q15 =
            RuntimeControl_MinU16(
                effective_configuration.configuration.maximum_speed_q15,
                mode_speed_ceiling_q15);

        request_built = JoystickProcessing_BuildRequestedDriveCommand(
            input->raw_input,
            &effective_configuration,
            context->request_sequence,
            input->now_ms,
            input->hmi->enable_request && input->hmi->control_mapping_valid,
            &output->requested_command,
            &output->joystick);
        output->joystick_processed = request_built;
    }

    output->observation.configuration_valid = input->configuration->valid;
    output->observation.input_valid =
        input->input_diagnostics->valid && request_built;
    output->observation.input_neutral = request_built && output->joystick.neutral;
    output->observation.link_valid = input->motor_link->link_valid;
    output->observation.link_fresh = input->motor_link->link_valid &&
        ((input->now_ms - input->motor_link->last_valid_packet_ms) <=
         APP_LINK_MAX_AGE_MS);
    output->observation.enable_request =
        output->requested_command.enable_request;
    output->observation.mandatory_tasks_healthy = input->mandatory_tasks_healthy;
    output->observation.now_ms = input->now_ms;
    output->observation.observed_faults = input->input_diagnostics->faults;

    SafetyState_Step(&context->safety, &output->observation);

    authorization_input.request = output->requested_command;
    authorization_input.safety_state = context->safety.state;
    authorization_input.active_faults = context->safety.active_faults;
    authorization_input.link_status = *input->motor_link;
    authorization_input.now_ms = input->now_ms;
    authorization_input.configuration_valid = input->configuration->valid;
    authorization_input.input_valid = output->observation.input_valid;
    authorization_input.mandatory_tasks_healthy = input->mandatory_tasks_healthy;

    output->authorization = CommandAuthorization_Evaluate(&authorization_input);

    /*
     * CommandAuthorization_Evaluate() is still the Phase-2 physical boundary.
     * Phase 4 may generate real requested motion and even prove logical
     * authorization on the host, but physical_command remains zero/unauthorized.
     */
    return true;
}
