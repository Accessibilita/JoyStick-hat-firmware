/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include "configuration_runtime.h"

#include <stddef.h>

void ConfigurationRuntime_Init(ConfigurationRuntimeState *state)
{
    if (state == NULL)
    {
        return;
    }

    state->active_image.generation = 0U;
    state->status = JOYSTICK_CONFIG_STATUS_NO_VALID_RECORD;
    state->selected_slot = UINT8_MAX;
    state->valid = false;
}

bool ConfigurationRuntime_Load(
    ConfigurationRuntimeState *state,
    ConfigurationRuntimeReadSlot read_slot)
{
    uint8_t slot_a[JOYSTICK_CONFIG_RECORD_SIZE] = {0};
    uint8_t slot_b[JOYSTICK_CONFIG_RECORD_SIZE] = {0};
    bool slot_a_read;
    bool slot_b_read;

    if ((state == NULL) || (read_slot == NULL))
    {
        return false;
    }

    ConfigurationRuntime_Init(state);

    /*
     * The storage driver decides whether a slot physically exists/read cleanly.
     * A successful storage read is still not enough: the Phase-3 decoder owns
     * version, CRC, reserved-field, generation, and semantic validation.
     */
    slot_a_read = read_slot(0U, slot_a);
    slot_b_read = read_slot(1U, slot_b);

    if (!(slot_a_read || slot_b_read))
    {
        return false;
    }

    if (!slot_a_read)
    {
        slot_a[0] = 0U; /* Guaranteed-invalid magic. */
    }
    if (!slot_b_read)
    {
        slot_b[0] = 0U; /* Guaranteed-invalid magic. */
    }

    state->status = JoystickConfiguration_SelectNewestValid(
        slot_a,
        slot_b,
        &state->active_image,
        &state->selected_slot);
    state->valid = state->status == JOYSTICK_CONFIG_STATUS_OK;

    return state->valid;
}
