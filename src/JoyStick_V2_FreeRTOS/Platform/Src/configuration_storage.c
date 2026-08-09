/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include "configuration_storage.h"

#include <stddef.h>

bool ConfigurationStorage_ReadSlot(
    uint8_t slot_index,
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE])
{
    (void)slot_index;
    (void)record;

    /*
     * No STM32 flash layout has been bench-validated for configuration yet.
     * Reporting a successful read here would turn invented persistence into a
     * safety input, so the target backend deliberately reports unavailable.
     */
    return false;
}

bool ConfigurationStorage_WriteSlot(
    uint8_t slot_index,
    const uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE])
{
    (void)slot_index;
    (void)record;

    /*
     * Phase 4 models transactional records but does not erase/program target
     * flash.  Storage writes remain disabled until the memory map, erase unit,
     * power-loss behavior, and read-back verification are validated on board.
     */
    return false;
}
