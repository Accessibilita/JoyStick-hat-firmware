#include "joystick_calibration.h"

#include <stddef.h>

#define JOYSTICK_CAL_DEFAULT_CENTER_SAMPLES             (64U)
#define JOYSTICK_CAL_DEFAULT_SWEEP_SAMPLES              (128U)
#define JOYSTICK_CAL_DEFAULT_ENDPOINT_GUARD_COUNTS      (32U)
#define JOYSTICK_CAL_DEFAULT_MIN_SIDE_SPAN_COUNTS       (512U)
#define JOYSTICK_CAL_DEFAULT_DEADBAND_FLOOR_COUNTS      (32U)
#define JOYSTICK_CAL_DEFAULT_NOISE_MULTIPLIER           (2U)

static uint16_t JoystickCalibration_MaxU16(uint16_t left, uint16_t right)
{
    return (left > right) ? left : right;
}

static bool JoystickCalibration_SampleInAdcRange(
    uint16_t x_counts,
    uint16_t y_counts)
{
    return (x_counts <= JOYSTICK_CONFIG_ADC_MAX_COUNTS) &&
           (y_counts <= JOYSTICK_CONFIG_ADC_MAX_COUNTS);
}

static bool JoystickCalibration_BuildAxis(
    uint16_t observed_min,
    uint16_t center_counts,
    uint16_t observed_max,
    uint16_t center_noise_span,
    const JoystickCalibrationPolicy *policy,
    JoystickAxisCalibration *axis)
{
    uint16_t usable_min;
    uint16_t usable_max;
    uint16_t negative_span;
    uint16_t positive_span;
    uint32_t noise_deadband;
    uint16_t deadband;

    if ((policy == NULL) || (axis == NULL))
    {
        return false;
    }

    if ((observed_min >= center_counts) ||
        (observed_max <= center_counts) ||
        ((uint32_t)observed_min + (uint32_t)policy->endpoint_guard_counts >=
         (uint32_t)center_counts) ||
        ((uint32_t)observed_max <=
         ((uint32_t)center_counts + (uint32_t)policy->endpoint_guard_counts)))
    {
        return false;
    }

    usable_min = (uint16_t)(observed_min + policy->endpoint_guard_counts);
    usable_max = (uint16_t)(observed_max - policy->endpoint_guard_counts);
    negative_span = (uint16_t)(center_counts - usable_min);
    positive_span = (uint16_t)(usable_max - center_counts);

    if ((negative_span < policy->minimum_side_span_counts) ||
        (positive_span < policy->minimum_side_span_counts))
    {
        return false;
    }

    noise_deadband = (uint32_t)center_noise_span *
                     (uint32_t)policy->center_noise_multiplier;
    if (noise_deadband > (uint32_t)UINT16_MAX)
    {
        return false;
    }

    deadband = JoystickCalibration_MaxU16(
        policy->deadband_floor_counts,
        (uint16_t)noise_deadband);

    if ((deadband == 0U) ||
        (deadband >= negative_span) ||
        (deadband >= positive_span))
    {
        return false;
    }

    axis->minimum_counts = usable_min;
    axis->center_counts = center_counts;
    axis->maximum_counts = usable_max;
    axis->deadband_counts = deadband;
    axis->inverted = false;

    return true;
}

void JoystickCalibration_GetSoftwareDefaultPolicy(
    JoystickCalibrationPolicy *policy)
{
    if (policy != NULL)
    {
        policy->center_sample_target = JOYSTICK_CAL_DEFAULT_CENTER_SAMPLES;
        policy->sweep_sample_minimum = JOYSTICK_CAL_DEFAULT_SWEEP_SAMPLES;
        policy->endpoint_guard_counts = JOYSTICK_CAL_DEFAULT_ENDPOINT_GUARD_COUNTS;
        policy->minimum_side_span_counts = JOYSTICK_CAL_DEFAULT_MIN_SIDE_SPAN_COUNTS;
        policy->deadband_floor_counts = JOYSTICK_CAL_DEFAULT_DEADBAND_FLOOR_COUNTS;
        policy->center_noise_multiplier = JOYSTICK_CAL_DEFAULT_NOISE_MULTIPLIER;
    }
}

bool JoystickCalibration_Begin(
    JoystickCalibrationContext *context,
    const JoystickCalibrationPolicy *policy)
{
    if ((context == NULL) || (policy == NULL))
    {
        return false;
    }

    if ((policy->center_sample_target == 0U) ||
        (policy->sweep_sample_minimum == 0U) ||
        (policy->minimum_side_span_counts < JOYSTICK_CONFIG_MIN_SIDE_SPAN_COUNTS) ||
        (policy->deadband_floor_counts == 0U) ||
        (policy->center_noise_multiplier == 0U))
    {
        return false;
    }

    context->state = JOYSTICK_CALIBRATION_CAPTURE_CENTER;
    context->policy = *policy;
    context->center_x_sum = 0U;
    context->center_y_sum = 0U;
    context->center_samples = 0U;
    context->sweep_samples = 0U;
    context->center_x_min = JOYSTICK_CONFIG_ADC_MAX_COUNTS;
    context->center_x_max = 0U;
    context->center_y_min = JOYSTICK_CONFIG_ADC_MAX_COUNTS;
    context->center_y_max = 0U;
    context->sweep_x_min = JOYSTICK_CONFIG_ADC_MAX_COUNTS;
    context->sweep_x_max = 0U;
    context->sweep_y_min = JOYSTICK_CONFIG_ADC_MAX_COUNTS;
    context->sweep_y_max = 0U;
    context->center_x_counts = 0U;
    context->center_y_counts = 0U;

    return true;
}

bool JoystickCalibration_AddCenterSample(
    JoystickCalibrationContext *context,
    uint16_t x_counts,
    uint16_t y_counts)
{
    if ((context == NULL) ||
        (context->state != JOYSTICK_CALIBRATION_CAPTURE_CENTER) ||
        !JoystickCalibration_SampleInAdcRange(x_counts, y_counts) ||
        (context->center_samples >= context->policy.center_sample_target))
    {
        return false;
    }

    context->center_x_sum += (uint32_t)x_counts;
    context->center_y_sum += (uint32_t)y_counts;
    context->center_samples++;

    if (x_counts < context->center_x_min)
    {
        context->center_x_min = x_counts;
    }
    if (x_counts > context->center_x_max)
    {
        context->center_x_max = x_counts;
    }
    if (y_counts < context->center_y_min)
    {
        context->center_y_min = y_counts;
    }
    if (y_counts > context->center_y_max)
    {
        context->center_y_max = y_counts;
    }

    if (context->center_samples == context->policy.center_sample_target)
    {
        context->center_x_counts = (uint16_t)(
            context->center_x_sum / (uint32_t)context->center_samples);
        context->center_y_counts = (uint16_t)(
            context->center_y_sum / (uint32_t)context->center_samples);
    }

    return true;
}

bool JoystickCalibration_BeginSweep(
    JoystickCalibrationContext *context)
{
    if ((context == NULL) ||
        (context->state != JOYSTICK_CALIBRATION_CAPTURE_CENTER) ||
        (context->center_samples != context->policy.center_sample_target))
    {
        return false;
    }

    context->state = JOYSTICK_CALIBRATION_CAPTURE_SWEEP;
    return true;
}

bool JoystickCalibration_AddSweepSample(
    JoystickCalibrationContext *context,
    uint16_t x_counts,
    uint16_t y_counts)
{
    if ((context == NULL) ||
        (context->state != JOYSTICK_CALIBRATION_CAPTURE_SWEEP) ||
        !JoystickCalibration_SampleInAdcRange(x_counts, y_counts) ||
        (context->sweep_samples == UINT16_MAX))
    {
        return false;
    }

    context->sweep_samples++;

    if (x_counts < context->sweep_x_min)
    {
        context->sweep_x_min = x_counts;
    }
    if (x_counts > context->sweep_x_max)
    {
        context->sweep_x_max = x_counts;
    }
    if (y_counts < context->sweep_y_min)
    {
        context->sweep_y_min = y_counts;
    }
    if (y_counts > context->sweep_y_max)
    {
        context->sweep_y_max = y_counts;
    }

    return true;
}

bool JoystickCalibration_Finalize(
    JoystickCalibrationContext *context,
    uint16_t reference_profile,
    JoystickConfiguration *configuration)
{
    JoystickConfiguration candidate;
    JoystickConfigurationImage validation_image;
    const uint16_t x_noise_span =
        (uint16_t)(context != NULL ?
            (uint16_t)(context->center_x_max - context->center_x_min) : 0U);
    const uint16_t y_noise_span =
        (uint16_t)(context != NULL ?
            (uint16_t)(context->center_y_max - context->center_y_min) : 0U);

    if ((context == NULL) || (configuration == NULL))
    {
        return false;
    }

    if ((context->state != JOYSTICK_CALIBRATION_CAPTURE_SWEEP) ||
        (context->sweep_samples < context->policy.sweep_sample_minimum))
    {
        context->state = JOYSTICK_CALIBRATION_FAILED;
        return false;
    }

    if (!JoystickCalibration_BuildAxis(
            context->sweep_x_min,
            context->center_x_counts,
            context->sweep_x_max,
            x_noise_span,
            &context->policy,
            &candidate.x_axis) ||
        !JoystickCalibration_BuildAxis(
            context->sweep_y_min,
            context->center_y_counts,
            context->sweep_y_max,
            y_noise_span,
            &context->policy,
            &candidate.y_axis))
    {
        context->state = JOYSTICK_CALIBRATION_FAILED;
        return false;
    }

    candidate.maximum_speed_q15 = JOYSTICK_CONFIG_Q15_MAX;
    candidate.response_curve_q15 = 0U;
    candidate.reference_profile = reference_profile;

    validation_image.generation = 1U;
    validation_image.configuration = candidate;
    if (JoystickConfiguration_Validate(&validation_image) !=
        JOYSTICK_CONFIG_STATUS_OK)
    {
        context->state = JOYSTICK_CALIBRATION_FAILED;
        return false;
    }

    *configuration = candidate;
    context->state = JOYSTICK_CALIBRATION_COMPLETE;
    return true;
}
