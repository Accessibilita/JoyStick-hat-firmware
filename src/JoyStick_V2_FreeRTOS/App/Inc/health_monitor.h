/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_HEALTH_SAFETY_TASK = 0,
    APP_HEALTH_LINK_TASK,
    APP_HEALTH_HMI_TASK,
    APP_HEALTH_DIAGNOSTICS_TASK,
    APP_HEALTH_COUNT
} AppHealthChannel;

typedef struct
{
    uint32_t progress_counter;
    uint32_t last_progress_ms;
    uint32_t deadline_ms;
    bool mandatory;
} AppHealthRecord;

void HealthMonitor_Init(uint32_t now_ms);
void HealthMonitor_RecordProgress(AppHealthChannel channel, uint32_t now_ms);
bool HealthMonitor_AreMandatoryChannelsHealthy(uint32_t now_ms);
bool HealthMonitor_CopyRecord(AppHealthChannel channel, AppHealthRecord *record);

#endif /* HEALTH_MONITOR_H */
