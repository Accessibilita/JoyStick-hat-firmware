/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef HMI_MODEL_H
#define HMI_MODEL_H

#include <stdbool.h>
#include <stdint.h>

#define HMI_MODEL_DEBOUNCE_MS                    (40U)
#define HMI_MODEL_SPEED_Q15_FULL                 (32767U)

typedef enum
{
    APP_OPERATING_MODE_BOOT = 0,
    APP_OPERATING_MODE_NORMAL,
    APP_OPERATING_MODE_REDUCED_SPEED,
    APP_OPERATING_MODE_PRECISION,
    APP_OPERATING_MODE_CALIBRATION,
    APP_OPERATING_MODE_SERVICE,
    APP_OPERATING_MODE_FAULT
} AppOperatingMode;

typedef struct
{
    uint32_t sequence;
    uint32_t sampled_at_ms;
    uint16_t buttons_active_low;
    uint8_t rotary_1_active_low;
    uint8_t rotary_2_active_low;
    AppOperatingMode requested_mode;
    uint16_t maximum_speed_q15;
    bool enable_request;
    bool control_mapping_valid;
} AppHmiState;

typedef struct
{
    uint16_t stable_buttons_active_low;
    uint16_t candidate_buttons_active_low;
    uint8_t stable_rotary_1_active_low;
    uint8_t candidate_rotary_1_active_low;
    uint8_t stable_rotary_2_active_low;
    uint8_t candidate_rotary_2_active_low;
    uint32_t candidate_since_ms;
    uint32_t sequence;
    bool initialized;
} HmiModelContext;

/* Reset debounce state. No physical input is treated as drive authority here. */
void HmiModel_Init(HmiModelContext *context);

/* Qualify raw active-low controls into a stable latest-state HMI snapshot. */
void HmiModel_Step(
    HmiModelContext *context,
    uint16_t buttons_active_low,
    uint8_t rotary_1_active_low,
    uint8_t rotary_2_active_low,
    uint32_t now_ms,
    AppHmiState *state);

#endif /* HMI_MODEL_H */
