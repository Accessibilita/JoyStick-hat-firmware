/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef __SPI_H
#define __SPI_H

#include "main.h"
extern SPI_HandleTypeDef hspi1;
void MX_SPI1_Init(void);

#endif /* __SPI_H */
