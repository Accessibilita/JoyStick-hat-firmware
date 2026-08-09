/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include "operating_mode.h"

#include <stddef.h>

static bool OperatingMode_IsDriveProfile(AppOperatingMode mode)
{
    return (mode == APP_OPERATING_MODE_NORMAL) ||
           (mode == APP_OPERATING_MODE_REDUCED_SPEED) ||
           (mode == APP_OPERATING_MODE_PRECISION);
}

void OperatingMode_Init(OperatingModeContext *context)
{
    if (context == NULL)
    {
        return;
    }
    context->active_mode = APP_OPERATING_MODE_BOOT;
    context->transition_sequence = 0U;
}

void OperatingMode_Step(
    OperatingModeContext *context,
    const OperatingModeInput *input)
{
    AppOperatingMode next_mode;

    if ((context == NULL) || (input == NULL))
    {
        return;
    }

    next_mode = context->active_mode;

    if (input->active_faults != APP_FAULT_NONE)
    {
        next_mode = APP_OPERATING_MODE_FAULT;
    }
    else if ((input->requested_mode == APP_OPERATING_MODE_CALIBRATION) ||
             (input->requested_mode == APP_OPERATING_MODE_SERVICE))
    {
        /* Service-like modes require an inhibited, neutral user input. */
        if (input->input_neutral && (!input->enable_request))
        {
            next_mode = input->requested_mode;
        }
    }
    else if (OperatingMode_IsDriveProfile(input->requested_mode))
    {
        /* Profile changes cannot rescue invalid configuration or moving input. */
        if (input->configuration_valid && input->input_neutral)
        {
            next_mode = input->requested_mode;
        }
    }
    else if (input->requested_mode == APP_OPERATING_MODE_FAULT)
    {
        next_mode = APP_OPERATING_MODE_FAULT;
    }
    else
    {
        /* BOOT or unknown transitions keep the current mode. */
    }

    if (next_mode != context->active_mode)
    {
        context->active_mode = next_mode;
        context->transition_sequence++;
    }
}

uint16_t OperatingMode_GetSpeedCeilingQ15(AppOperatingMode mode)
{
    switch (mode)
    {
        case APP_OPERATING_MODE_NORMAL:
            return HMI_MODEL_SPEED_Q15_FULL;

        case APP_OPERATING_MODE_REDUCED_SPEED:
            return OPERATING_MODE_REDUCED_SPEED_Q15;

        case APP_OPERATING_MODE_PRECISION:
            return OPERATING_MODE_PRECISION_SPEED_Q15;

        case APP_OPERATING_MODE_BOOT:
        case APP_OPERATING_MODE_CALIBRATION:
        case APP_OPERATING_MODE_SERVICE:
        case APP_OPERATING_MODE_FAULT:
        default:
            return 0U;
    }
}
