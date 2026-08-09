/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef BOARD_IO_H
#define BOARD_IO_H

#include <stdbool.h>
#include <stdint.h>

void BoardIo_ForceRs485SafeDisabled(void);
void BoardIo_ForceLedHudBlanked(void);
uint16_t BoardIo_ReadButtonsActiveLow(void);
uint8_t BoardIo_ReadRotary1ActiveLow(void);
uint8_t BoardIo_ReadRotary2ActiveLow(void);
bool BoardIo_IsPowerGood(void);

#endif /* BOARD_IO_H */
