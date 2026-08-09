#ifndef JOYSTICK_PROCESSING_H
#define JOYSTICK_PROCESSING_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "command_authorization.h"
#include "joystick_configuration.h"

typedef struct
{
    int16_t x_q15;
    int16_t y_q15;
    int16_t shaped_x_q15;
    int16_t shaped_y_q15;
    bool neutral;
} JoystickProcessedSample;

bool JoystickProcessing_NormalizeAxis(
    uint16_t sample_counts,
    const JoystickAxisCalibration *calibration,
    int16_t *normalized_q15);

int16_t JoystickProcessing_ApplyResponseCurve(
    int16_t input_q15,
    uint16_t response_curve_q15);

int16_t JoystickProcessing_ApplySpeedLimit(
    int16_t input_q15,
    uint16_t maximum_speed_q15);

bool JoystickProcessing_Process(
    const AppRawInputSnapshot *input,
    const JoystickConfigurationImage *configuration,
    JoystickProcessedSample *processed);

bool JoystickProcessing_BuildRequestedDriveCommand(
    const AppRawInputSnapshot *input,
    const JoystickConfigurationImage *configuration,
    uint32_t request_sequence,
    uint32_t now_ms,
    bool enable_request,
    AppRequestedDriveCommand *request,
    JoystickProcessedSample *processed);

#endif /* JOYSTICK_PROCESSING_H */
