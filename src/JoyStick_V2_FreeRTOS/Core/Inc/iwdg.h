/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef __IWDG_H
#define __IWDG_H

#include "main.h"
extern IWDG_HandleTypeDef hiwdg;
void MX_IWDG_Init(void);

#endif /* __IWDG_H */
