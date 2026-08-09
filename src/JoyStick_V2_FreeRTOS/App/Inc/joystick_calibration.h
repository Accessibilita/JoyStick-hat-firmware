/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef JOYSTICK_CALIBRATION_H
#define JOYSTICK_CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

#include "joystick_configuration.h"

typedef enum
{
    JOYSTICK_CALIBRATION_IDLE = 0,
    JOYSTICK_CALIBRATION_CAPTURE_CENTER,
    JOYSTICK_CALIBRATION_CAPTURE_SWEEP,
    JOYSTICK_CALIBRATION_COMPLETE,
    JOYSTICK_CALIBRATION_FAILED
} JoystickCalibrationState;

typedef struct
{
    uint16_t center_sample_target;
    uint16_t sweep_sample_minimum;
    uint16_t endpoint_guard_counts;
    uint16_t minimum_side_span_counts;
    uint16_t deadband_floor_counts;
    uint16_t center_noise_multiplier;
} JoystickCalibrationPolicy;

typedef struct
{
    JoystickCalibrationState state;
    JoystickCalibrationPolicy policy;
    uint32_t center_x_sum;
    uint32_t center_y_sum;
    uint16_t center_samples;
    uint16_t sweep_samples;
    uint16_t center_x_min;
    uint16_t center_x_max;
    uint16_t center_y_min;
    uint16_t center_y_max;
    uint16_t sweep_x_min;
    uint16_t sweep_x_max;
    uint16_t sweep_y_min;
    uint16_t sweep_y_max;
    uint16_t center_x_counts;
    uint16_t center_y_counts;
} JoystickCalibrationContext;

void JoystickCalibration_GetSoftwareDefaultPolicy(
    JoystickCalibrationPolicy *policy);

bool JoystickCalibration_Begin(
    JoystickCalibrationContext *context,
    const JoystickCalibrationPolicy *policy);

bool JoystickCalibration_AddCenterSample(
    JoystickCalibrationContext *context,
    uint16_t x_counts,
    uint16_t y_counts);

bool JoystickCalibration_BeginSweep(
    JoystickCalibrationContext *context);

bool JoystickCalibration_AddSweepSample(
    JoystickCalibrationContext *context,
    uint16_t x_counts,
    uint16_t y_counts);

bool JoystickCalibration_Finalize(
    JoystickCalibrationContext *context,
    uint16_t reference_profile,
    JoystickConfiguration *configuration);

#endif /* JOYSTICK_CALIBRATION_H */
