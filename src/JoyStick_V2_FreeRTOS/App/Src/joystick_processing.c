/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "joystick_processing.h"

#include <stddef.h>

#define JOYSTICK_PROCESSING_Q15_MAX_SIGNED              (32767)

static int16_t JoystickProcessing_ClampQ15(int32_t value)
{
    if (value > JOYSTICK_PROCESSING_Q15_MAX_SIGNED)
    {
        return (int16_t)JOYSTICK_PROCESSING_Q15_MAX_SIGNED;
    }

    if (value < -JOYSTICK_PROCESSING_Q15_MAX_SIGNED)
    {
        return (int16_t)(-JOYSTICK_PROCESSING_Q15_MAX_SIGNED);
    }

    return (int16_t)value;
}

bool JoystickProcessing_NormalizeAxis(
    uint16_t sample_counts,
    const JoystickAxisCalibration *calibration,
    int16_t *normalized_q15)
{
    uint16_t lower_deadband_edge;
    uint16_t upper_deadband_edge;
    int32_t normalized;

    if ((calibration == NULL) || (normalized_q15 == NULL))
    {
        return false;
    }

    if ((calibration->minimum_counts >= calibration->center_counts) ||
        (calibration->center_counts >= calibration->maximum_counts) ||
        (calibration->deadband_counts == 0U) ||
        (calibration->deadband_counts >=
         (uint16_t)(calibration->center_counts - calibration->minimum_counts)) ||
        (calibration->deadband_counts >=
         (uint16_t)(calibration->maximum_counts - calibration->center_counts)))
    {
        *normalized_q15 = 0;
        return false;
    }

    lower_deadband_edge =
        (uint16_t)(calibration->center_counts - calibration->deadband_counts);
    upper_deadband_edge =
        (uint16_t)(calibration->center_counts + calibration->deadband_counts);

    if ((sample_counts >= lower_deadband_edge) &&
        (sample_counts <= upper_deadband_edge))
    {
        normalized = 0;
    }
    else if (sample_counts < lower_deadband_edge)
    {
        const uint16_t clamped_sample =
            (sample_counts < calibration->minimum_counts) ?
                calibration->minimum_counts : sample_counts;
        const uint32_t numerator =
            (uint32_t)(lower_deadband_edge - clamped_sample) *
            (uint32_t)JOYSTICK_PROCESSING_Q15_MAX_SIGNED;
        const uint16_t denominator =
            (uint16_t)(lower_deadband_edge - calibration->minimum_counts);

        normalized = -(int32_t)(numerator / (uint32_t)denominator);
    }
    else
    {
        const uint16_t clamped_sample =
            (sample_counts > calibration->maximum_counts) ?
                calibration->maximum_counts : sample_counts;
        const uint32_t numerator =
            (uint32_t)(clamped_sample - upper_deadband_edge) *
            (uint32_t)JOYSTICK_PROCESSING_Q15_MAX_SIGNED;
        const uint16_t denominator =
            (uint16_t)(calibration->maximum_counts - upper_deadband_edge);

        normalized = (int32_t)(numerator / (uint32_t)denominator);
    }

    if (calibration->inverted)
    {
        normalized = -normalized;
    }

    *normalized_q15 = JoystickProcessing_ClampQ15(normalized);
    return true;
}

int16_t JoystickProcessing_ApplyResponseCurve(
    int16_t input_q15,
    uint16_t response_curve_q15)
{
    int32_t signed_input = (int32_t)input_q15;
    int32_t sign = 1;
    int32_t magnitude;
    int64_t cubic_numerator;
    int32_t cubic_q15;
    int64_t blended_numerator;
    int32_t blended_q15;
    uint16_t curve = response_curve_q15;

    if (curve > JOYSTICK_CONFIG_Q15_MAX)
    {
        curve = JOYSTICK_CONFIG_Q15_MAX;
    }

    if (signed_input < 0)
    {
        sign = -1;
        magnitude = -signed_input;
    }
    else
    {
        magnitude = signed_input;
    }

    if (magnitude > JOYSTICK_PROCESSING_Q15_MAX_SIGNED)
    {
        magnitude = JOYSTICK_PROCESSING_Q15_MAX_SIGNED;
    }

    cubic_numerator = (int64_t)magnitude *
                      (int64_t)magnitude *
                      (int64_t)magnitude;
    cubic_q15 = (int32_t)(cubic_numerator /
        ((int64_t)JOYSTICK_PROCESSING_Q15_MAX_SIGNED *
         (int64_t)JOYSTICK_PROCESSING_Q15_MAX_SIGNED));

    blended_numerator =
        ((int64_t)(JOYSTICK_CONFIG_Q15_MAX - curve) * (int64_t)magnitude) +
        ((int64_t)curve * (int64_t)cubic_q15);
    blended_q15 = (int32_t)(blended_numerator /
                            (int64_t)JOYSTICK_CONFIG_Q15_MAX);

    return JoystickProcessing_ClampQ15(sign * blended_q15);
}

int16_t JoystickProcessing_ApplySpeedLimit(
    int16_t input_q15,
    uint16_t maximum_speed_q15)
{
    uint16_t limit = maximum_speed_q15;
    int32_t scaled;

    if (limit > JOYSTICK_CONFIG_Q15_MAX)
    {
        limit = JOYSTICK_CONFIG_Q15_MAX;
    }

    scaled = (int32_t)(((int64_t)input_q15 * (int64_t)limit) /
                       (int64_t)JOYSTICK_CONFIG_Q15_MAX);
    return JoystickProcessing_ClampQ15(scaled);
}

bool JoystickProcessing_Process(
    const AppRawInputSnapshot *input,
    const JoystickConfigurationImage *configuration,
    JoystickProcessedSample *processed)
{
    int16_t normalized_x;
    int16_t normalized_y;

    if ((input == NULL) || (configuration == NULL) || (processed == NULL))
    {
        return false;
    }

    if (JoystickConfiguration_Validate(configuration) !=
        JOYSTICK_CONFIG_STATUS_OK)
    {
        processed->x_q15 = 0;
        processed->y_q15 = 0;
        processed->shaped_x_q15 = 0;
        processed->shaped_y_q15 = 0;
        processed->neutral = true;
        return false;
    }

    if (!JoystickProcessing_NormalizeAxis(
            input->joystick_x_counts,
            &configuration->configuration.x_axis,
            &normalized_x) ||
        !JoystickProcessing_NormalizeAxis(
            input->joystick_y_counts,
            &configuration->configuration.y_axis,
            &normalized_y))
    {
        processed->x_q15 = 0;
        processed->y_q15 = 0;
        processed->shaped_x_q15 = 0;
        processed->shaped_y_q15 = 0;
        processed->neutral = true;
        return false;
    }

    processed->x_q15 = normalized_x;
    processed->y_q15 = normalized_y;
    processed->shaped_x_q15 = JoystickProcessing_ApplySpeedLimit(
        JoystickProcessing_ApplyResponseCurve(
            normalized_x,
            configuration->configuration.response_curve_q15),
        configuration->configuration.maximum_speed_q15);
    processed->shaped_y_q15 = JoystickProcessing_ApplySpeedLimit(
        JoystickProcessing_ApplyResponseCurve(
            normalized_y,
            configuration->configuration.response_curve_q15),
        configuration->configuration.maximum_speed_q15);
    processed->neutral = (normalized_x == 0) && (normalized_y == 0);

    return true;
}

bool JoystickProcessing_BuildRequestedDriveCommand(
    const AppRawInputSnapshot *input,
    const JoystickConfigurationImage *configuration,
    uint32_t request_sequence,
    uint32_t now_ms,
    bool enable_request,
    AppRequestedDriveCommand *request,
    JoystickProcessedSample *processed)
{
    JoystickProcessedSample local_processed;
    JoystickProcessedSample *processed_target = processed;

    if (request == NULL)
    {
        return false;
    }

    request->request_sequence = request_sequence;
    request->input_sequence = (input != NULL) ? input->sequence : 0U;
    request->generated_at_ms = now_ms;
    request->forward_q15 = 0;
    request->turn_q15 = 0;
    request->maximum_speed_q15 = 0U;
    request->enable_request = false;

    if (processed_target == NULL)
    {
        processed_target = &local_processed;
    }

    if (!JoystickProcessing_Process(input, configuration, processed_target))
    {
        return false;
    }

    request->forward_q15 = processed_target->shaped_y_q15;
    request->turn_q15 = processed_target->shaped_x_q15;
    request->maximum_speed_q15 = configuration->configuration.maximum_speed_q15;
    request->enable_request = enable_request;

    return true;
}
