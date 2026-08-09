/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef APP_RTOS_H
#define APP_RTOS_H

#include <stdbool.h>

#include "app_types.h"
#include "hmi_model.h"

bool AppRtos_CreateStaticObjects(void);
void AppRtos_NotifyAdcBatchFromIsr(void);

/* Latest-state mailboxes use overwrite semantics; stale control state is useless. */
bool AppRtos_PublishAuthorizedCommand(
    const AppAuthorizedDriveCommand *command);
bool AppRtos_ReadAuthorizedCommand(
    AppAuthorizedDriveCommand *command);
bool AppRtos_PublishMotorLinkStatus(
    const AppMotorLinkStatus *status);
bool AppRtos_ReadMotorLinkStatus(
    AppMotorLinkStatus *status);
bool AppRtos_PublishHmiState(const AppHmiState *state);
bool AppRtos_ReadHmiState(AppHmiState *state);

#endif /* APP_RTOS_H */
