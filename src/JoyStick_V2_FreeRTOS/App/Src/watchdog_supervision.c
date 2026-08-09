/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "watchdog_supervision.h"

#include "iwdg.h"

void WatchdogSupervision_Init(void)
{
    /* Hardware initialization and Debug freeze are completed before scheduler. */
}

void WatchdogSupervision_RefreshIfHealthy(
    bool mandatory_tasks_healthy,
    bool no_latched_fault)
{
    if (mandatory_tasks_healthy && no_latched_fault)
    {
        (void)HAL_IWDG_Refresh(&hiwdg);
    }
}
