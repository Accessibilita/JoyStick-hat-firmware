/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "command_authorization.h"

#include <stddef.h>

#include "app_build_config.h"
#include "motor_protocol.h"

#if APP_RS485_PHYSICAL_LINK_ENABLE != 0U
#error "Phase 2 software authorization model requires physical drive to remain disabled."
#endif

static bool CommandAuthorization_SafetyStateCanRequestDrive(
    AppSafetyState safety_state)
{
    return (safety_state == APP_SAFETY_READY) ||
           (safety_state == APP_SAFETY_DRIVE_AUTHORIZED);
}

static AppAuthorizedDriveCommand CommandAuthorization_MakePhysicalInhibit(
    const CommandAuthorizationInput *input)
{
    AppAuthorizedDriveCommand command = {0};

    if (input != NULL)
    {
        command.publication_sequence = input->request.request_sequence;
        command.input_sequence = input->request.input_sequence;
        command.generated_at_ms = input->now_ms;
        command.safety_state = input->safety_state;
        command.active_faults = input->active_faults;
    }
    else
    {
        command.safety_state = APP_SAFETY_LATCHED_FAULT;
        command.active_faults = APP_FAULT_INTERNAL_INVARIANT;
    }

    command.forward_q15 = 0;
    command.turn_q15 = 0;
    command.maximum_speed_q15 = 0U;
    command.drive_authorized = false;

    return command;
}

CommandAuthorizationResult CommandAuthorization_Evaluate(
    const CommandAuthorizationInput *input)
{
    CommandAuthorizationResult result;
    uint32_t command_age_ms;
    uint32_t link_age_ms;

    result.logical_authorized = false;
    result.blocking_reasons = COMMAND_AUTH_BLOCK_NONE;
    result.physical_command = CommandAuthorization_MakePhysicalInhibit(input);

    if (input == NULL)
    {
        result.blocking_reasons = COMMAND_AUTH_BLOCK_ACTIVE_FAULT;
        return result;
    }

    if (!input->configuration_valid)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_CONFIGURATION;
    }

    if (!input->input_valid)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_INPUT;
    }

    if (!input->mandatory_tasks_healthy)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_TASK_HEALTH;
    }

    if (!input->link_status.link_valid)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_LINK;
    }

    link_age_ms = input->now_ms - input->link_status.last_valid_packet_ms;
    if ((!input->link_status.link_valid) ||
        (link_age_ms > APP_LINK_MAX_AGE_MS))
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_LINK_STALE;
    }

    if (!(input->link_status.remote_drive_ready &&
          input->link_status.brakes_confirmed))
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_REMOTE_NOT_READY;
    }

    if (input->link_status.remote_faults != APP_FAULT_NONE)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_REMOTE_FAULT;
    }

    if (!CommandAuthorization_SafetyStateCanRequestDrive(input->safety_state))
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_SAFETY_STATE;
    }

    if (!input->request.enable_request)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_ENABLE_REQUEST;
    }

    command_age_ms = input->now_ms - input->request.generated_at_ms;
    if (command_age_ms > APP_MOTOR_COMMAND_MAX_AGE_MS)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_COMMAND_STALE;
    }

    if (input->request.maximum_speed_q15 > MOTOR_PROTOCOL_Q15_MAX)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_COMMAND_RANGE;
    }

    if (input->active_faults != APP_FAULT_NONE)
    {
        result.blocking_reasons |= COMMAND_AUTH_BLOCK_ACTIVE_FAULT;
    }

    result.logical_authorized =
        result.blocking_reasons == COMMAND_AUTH_BLOCK_NONE;

    /*
     * Phase 2 deliberately proves the logical decision and the physical
     * inhibition as separate things. The result may become logically true in
     * host tests, but the returned physical command stays zero and unauthorized.
     */
    return result;
}
