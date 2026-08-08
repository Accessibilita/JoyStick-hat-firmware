#include "safety_state.h"

#include <stddef.h>

#include "app_build_config.h"

static void SafetyState_Enter(
    AppSafetyContext *context,
    AppSafetyState next_state,
    uint32_t now_ms)
{
    context->state = next_state;
    context->state_entered_at_ms = now_ms;

    if (next_state != APP_SAFETY_WAIT_FOR_NEUTRAL)
    {
        context->neutral_timer_running = false;
        context->neutral_started_at_ms = 0U;
    }
}

void SafetyState_Init(AppSafetyContext *context, uint32_t now_ms)
{
    if (context != NULL)
    {
        context->state = APP_SAFETY_BOOT_SELF_TEST;
        context->active_faults = APP_FAULT_CONFIGURATION_INVALID |
                                 APP_FAULT_LINK_INVALID;
        context->state_entered_at_ms = now_ms;
        context->neutral_started_at_ms = 0U;
        context->neutral_timer_running = false;
    }
}

void SafetyState_Step(
    AppSafetyContext *context,
    const AppSafetyObservation *observation)
{
    if ((context == NULL) || (observation == NULL))
    {
        if (context != NULL)
        {
            context->active_faults |= APP_FAULT_INTERNAL_INVARIANT;
            SafetyState_Enter(
                context,
                APP_SAFETY_LATCHED_FAULT,
                context->state_entered_at_ms);
        }
        return;
    }

    context->active_faults = observation->observed_faults;

    if (!observation->configuration_valid)
    {
        context->active_faults |= APP_FAULT_CONFIGURATION_INVALID;
    }

    if (!observation->input_valid &&
        ((observation->observed_faults &
          (APP_FAULT_INPUT_STALE |
           APP_FAULT_JOYSTICK_X_RANGE |
           APP_FAULT_JOYSTICK_Y_RANGE |
           APP_FAULT_ADC_OVERRUN |
           APP_FAULT_POWER_GOOD_LOST)) == APP_FAULT_NONE))
    {
        context->active_faults |= APP_FAULT_INPUT_INVALID;
    }

    if (!observation->link_valid)
    {
        context->active_faults |= APP_FAULT_LINK_INVALID;
    }

    if (!observation->link_fresh)
    {
        context->active_faults |= APP_FAULT_LINK_STALE;
    }

    if (!observation->mandatory_tasks_healthy)
    {
        context->active_faults |= APP_FAULT_TASK_HEALTH;
    }

    if ((context->active_faults & APP_FAULT_CRITICAL_MASK) != APP_FAULT_NONE)
    {
        SafetyState_Enter(
            context,
            APP_SAFETY_LATCHED_FAULT,
            observation->now_ms);
        return;
    }

    switch (context->state)
    {
        case APP_SAFETY_BOOT_SELF_TEST:
            SafetyState_Enter(
                context,
                APP_SAFETY_CONFIGURATION_CHECK,
                observation->now_ms);
            break;

        case APP_SAFETY_CONFIGURATION_CHECK:
            if (observation->configuration_valid)
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_WAIT_FOR_LINK,
                    observation->now_ms);
            }
            else
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_RECOVERABLE_INHIBIT,
                    observation->now_ms);
            }
            break;

        case APP_SAFETY_WAIT_FOR_LINK:
            if (!observation->configuration_valid)
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_RECOVERABLE_INHIBIT,
                    observation->now_ms);
            }
            else if (observation->input_valid &&
                     observation->link_valid &&
                     observation->link_fresh &&
                     observation->mandatory_tasks_healthy)
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_WAIT_FOR_NEUTRAL,
                    observation->now_ms);
            }
            else
            {
                /* Stay inhibited while waiting for all prerequisites. */
            }
            break;

        case APP_SAFETY_WAIT_FOR_NEUTRAL:
            if (!(observation->configuration_valid &&
                  observation->input_valid &&
                  observation->link_valid &&
                  observation->link_fresh &&
                  observation->mandatory_tasks_healthy))
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_WAIT_FOR_LINK,
                    observation->now_ms);
            }
            else if (observation->input_neutral)
            {
                if (!context->neutral_timer_running)
                {
                    context->neutral_timer_running = true;
                    context->neutral_started_at_ms = observation->now_ms;
                }
                else if ((observation->now_ms - context->neutral_started_at_ms) >=
                         APP_NEUTRAL_QUALIFICATION_MS)
                {
                    SafetyState_Enter(
                        context,
                        APP_SAFETY_READY,
                        observation->now_ms);
                }
                else
                {
                    /* Neutral dwell time has not yet completed. */
                }
            }
            else
            {
                context->neutral_timer_running = false;
                context->neutral_started_at_ms = 0U;
            }
            break;

        case APP_SAFETY_READY:
            if (!(observation->configuration_valid &&
                  observation->input_valid &&
                  observation->input_neutral &&
                  observation->link_valid &&
                  observation->link_fresh &&
                  observation->mandatory_tasks_healthy))
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_WAIT_FOR_NEUTRAL,
                    observation->now_ms);
            }
            else if (observation->enable_request)
            {
#if APP_RS485_PHYSICAL_LINK_ENABLE == 1U
                SafetyState_Enter(
                    context,
                    APP_SAFETY_DRIVE_AUTHORIZED,
                    observation->now_ms);
#else
                SafetyState_Enter(
                    context,
                    APP_SAFETY_RECOVERABLE_INHIBIT,
                    observation->now_ms);
#endif
            }
            else
            {
                /* Ready but not authorized. */
            }
            break;

        case APP_SAFETY_DRIVE_AUTHORIZED:
#if APP_RS485_PHYSICAL_LINK_ENABLE == 0U
            SafetyState_Enter(
                context,
                APP_SAFETY_RECOVERABLE_INHIBIT,
                observation->now_ms);
#else
            if (!(observation->configuration_valid &&
                  observation->input_valid &&
                  observation->link_valid &&
                  observation->link_fresh &&
                  observation->mandatory_tasks_healthy &&
                  observation->enable_request))
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_WAIT_FOR_NEUTRAL,
                    observation->now_ms);
            }
#endif
            break;

        case APP_SAFETY_RECOVERABLE_INHIBIT:
            if (observation->configuration_valid &&
                observation->input_valid &&
                observation->link_valid &&
                observation->link_fresh &&
                observation->mandatory_tasks_healthy)
            {
                SafetyState_Enter(
                    context,
                    APP_SAFETY_WAIT_FOR_NEUTRAL,
                    observation->now_ms);
            }
            break;

        case APP_SAFETY_LATCHED_FAULT:
            /* A latched fault requires reset in Phase 1. */
            break;

        case APP_SAFETY_RESET:
        default:
            context->active_faults |= APP_FAULT_INTERNAL_INVARIANT;
            SafetyState_Enter(
                context,
                APP_SAFETY_LATCHED_FAULT,
                observation->now_ms);
            break;
    }
}

bool SafetyState_IsDriveAuthorized(const AppSafetyContext *context)
{
    if (context == NULL)
    {
        return false;
    }

    return context->state == APP_SAFETY_DRIVE_AUTHORIZED;
}
