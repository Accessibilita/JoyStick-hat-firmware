/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
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
