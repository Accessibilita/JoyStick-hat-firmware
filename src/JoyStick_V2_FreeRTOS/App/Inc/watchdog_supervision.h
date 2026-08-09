/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef WATCHDOG_SUPERVISION_H
#define WATCHDOG_SUPERVISION_H

#include <stdbool.h>

void WatchdogSupervision_Init(void);
void WatchdogSupervision_RefreshIfHealthy(
    bool mandatory_tasks_healthy,
    bool no_latched_fault);

#endif /* WATCHDOG_SUPERVISION_H */
