/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#ifndef OPERATING_MODE_H
#define OPERATING_MODE_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "hmi_model.h"

#define OPERATING_MODE_REDUCED_SPEED_Q15        (16384U)
#define OPERATING_MODE_PRECISION_SPEED_Q15      (8192U)

typedef struct
{
    AppOperatingMode active_mode;
    uint32_t transition_sequence;
} OperatingModeContext;

typedef struct
{
    AppOperatingMode requested_mode;
    AppFaultMask active_faults;
    bool configuration_valid;
    bool input_neutral;
    bool enable_request;
} OperatingModeInput;

/* Start inhibited in BOOT; mode does not itself grant drive authority. */
void OperatingMode_Init(OperatingModeContext *context);

/* Apply bounded transition rules; profile changes require neutral input. */
void OperatingMode_Step(
    OperatingModeContext *context,
    const OperatingModeInput *input);

/* Return the mode-specific upper speed bound; service-like modes return zero. */
uint16_t OperatingMode_GetSpeedCeilingQ15(AppOperatingMode mode);

#endif /* OPERATING_MODE_H */
