/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#ifndef CONFIGURATION_STORAGE_H
#define CONFIGURATION_STORAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "joystick_configuration.h"

/* Read one persistent record slot. Phase-4 target backend returns false. */
bool ConfigurationStorage_ReadSlot(
    uint8_t slot_index,
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE]);

/* Write one persistent record slot. Phase-4 target backend returns false. */
bool ConfigurationStorage_WriteSlot(
    uint8_t slot_index,
    const uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE]);

#endif /* CONFIGURATION_STORAGE_H */
