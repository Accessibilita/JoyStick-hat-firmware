/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef __TIM_H
#define __TIM_H

#include "main.h"
extern TIM_HandleTypeDef htim2;
void MX_TIM2_Init(void);

#endif /* __TIM_H */
