/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef APP_TASKS_H
#define APP_TASKS_H

void SafetyControlTask_Run(void *argument);
void Rs485LinkTask_Run(void *argument);
void HmiTask_Run(void *argument);
void DiagnosticsTask_Run(void *argument);

#endif /* APP_TASKS_H */
