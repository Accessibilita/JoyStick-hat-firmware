/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef SAFETY_STATE_H
#define SAFETY_STATE_H

#include "app_types.h"

void SafetyState_Init(AppSafetyContext *context, uint32_t now_ms);
void SafetyState_Step(
    AppSafetyContext *context,
    const AppSafetyObservation *observation);
bool SafetyState_IsDriveAuthorized(const AppSafetyContext *context);

#endif /* SAFETY_STATE_H */
