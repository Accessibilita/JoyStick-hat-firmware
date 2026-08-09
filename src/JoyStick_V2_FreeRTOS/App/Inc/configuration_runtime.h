/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#ifndef CONFIGURATION_RUNTIME_H
#define CONFIGURATION_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include "joystick_configuration.h"

typedef bool (*ConfigurationRuntimeReadSlot)(
    uint8_t slot_index,
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE]);

typedef struct
{
    JoystickConfigurationImage active_image;
    JoystickConfigStatus status;
    uint8_t selected_slot;
    bool valid;
} ConfigurationRuntimeState;

/* Start with no authoritative configuration. */
void ConfigurationRuntime_Init(ConfigurationRuntimeState *state);

/* Read both slots and activate only the newest Phase-3-valid record. */
bool ConfigurationRuntime_Load(
    ConfigurationRuntimeState *state,
    ConfigurationRuntimeReadSlot read_slot);

#endif /* CONFIGURATION_RUNTIME_H */
