/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include "hmi_model.h"

#include <stddef.h>

static bool HmiModel_CandidateMatches(
    const HmiModelContext *context,
    uint16_t buttons_active_low,
    uint8_t rotary_1_active_low,
    uint8_t rotary_2_active_low)
{
    return (context->candidate_buttons_active_low == buttons_active_low) &&
           (context->candidate_rotary_1_active_low == rotary_1_active_low) &&
           (context->candidate_rotary_2_active_low == rotary_2_active_low);
}

void HmiModel_Init(HmiModelContext *context)
{
    if (context == NULL)
    {
        return;
    }

    context->stable_buttons_active_low = UINT16_MAX;
    context->candidate_buttons_active_low = UINT16_MAX;
    context->stable_rotary_1_active_low = UINT8_MAX;
    context->candidate_rotary_1_active_low = UINT8_MAX;
    context->stable_rotary_2_active_low = UINT8_MAX;
    context->candidate_rotary_2_active_low = UINT8_MAX;
    context->candidate_since_ms = 0U;
    context->sequence = 0U;
    context->initialized = false;
}

void HmiModel_Step(
    HmiModelContext *context,
    uint16_t buttons_active_low,
    uint8_t rotary_1_active_low,
    uint8_t rotary_2_active_low,
    uint32_t now_ms,
    AppHmiState *state)
{
    bool stable_changed = false;

    if ((context == NULL) || (state == NULL))
    {
        return;
    }

    if (!context->initialized)
    {
        context->stable_buttons_active_low = buttons_active_low;
        context->candidate_buttons_active_low = buttons_active_low;
        context->stable_rotary_1_active_low = rotary_1_active_low;
        context->candidate_rotary_1_active_low = rotary_1_active_low;
        context->stable_rotary_2_active_low = rotary_2_active_low;
        context->candidate_rotary_2_active_low = rotary_2_active_low;
        context->candidate_since_ms = now_ms;
        context->initialized = true;
        stable_changed = true;
    }
    else if (!HmiModel_CandidateMatches(
                 context,
                 buttons_active_low,
                 rotary_1_active_low,
                 rotary_2_active_low))
    {
        /*
         * Start a new bounded debounce qualification window.  Nothing becomes
         * authoritative merely because one GPIO sample changed.
         */
        context->candidate_buttons_active_low = buttons_active_low;
        context->candidate_rotary_1_active_low = rotary_1_active_low;
        context->candidate_rotary_2_active_low = rotary_2_active_low;
        context->candidate_since_ms = now_ms;
    }
    else if ((now_ms - context->candidate_since_ms) >= HMI_MODEL_DEBOUNCE_MS)
    {
        if ((context->stable_buttons_active_low != buttons_active_low) ||
            (context->stable_rotary_1_active_low != rotary_1_active_low) ||
            (context->stable_rotary_2_active_low != rotary_2_active_low))
        {
            context->stable_buttons_active_low = buttons_active_low;
            context->stable_rotary_1_active_low = rotary_1_active_low;
            context->stable_rotary_2_active_low = rotary_2_active_low;
            stable_changed = true;
        }
    }
    else
    {
        /* Candidate is still qualifying. */
    }

    if (stable_changed)
    {
        context->sequence++;
    }

    state->sequence = context->sequence;
    state->sampled_at_ms = now_ms;
    state->buttons_active_low = context->stable_buttons_active_low;
    state->rotary_1_active_low = context->stable_rotary_1_active_low;
    state->rotary_2_active_low = context->stable_rotary_2_active_low;

    /*
     * Phase 4 intentionally does not guess final button/rotary semantics from
     * an unavailable bench.  Raw HMI state is debounced and published now,
     * while drive-enable semantics remain fail-closed until hardware testing.
     */
    state->requested_mode = APP_OPERATING_MODE_NORMAL;
    state->maximum_speed_q15 = HMI_MODEL_SPEED_Q15_FULL;
    state->enable_request = false;
    state->control_mapping_valid = false;
}
