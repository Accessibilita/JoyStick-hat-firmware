#include "input_diagnostics.h"

#include <stddef.h>

#include "app_build_config.h"

static bool InputDiagnostics_IsWithinRange(uint16_t value)
{
    return (value >= APP_ADC_ENGINEERING_MIN_COUNTS) &&
           (value <= APP_ADC_ENGINEERING_MAX_COUNTS);
}

InputDiagnosticResult InputDiagnostics_Evaluate(
    const AppRawInputSnapshot *input,
    uint32_t now_ms)
{
    InputDiagnosticResult result =
    {
        .valid = false,
        .neutral = false,
        .faults = APP_FAULT_NONE
    };

    if (input == NULL)
    {
        result.faults = APP_FAULT_INTERNAL_INVARIANT;
        return result;
    }

    if ((now_ms - input->captured_at_ms) > APP_INPUT_MAX_AGE_MS)
    {
        result.faults |= APP_FAULT_INPUT_STALE;
    }

    if (!InputDiagnostics_IsWithinRange(input->joystick_x_counts))
    {
        result.faults |= APP_FAULT_JOYSTICK_X_RANGE;
    }

    if (!InputDiagnostics_IsWithinRange(input->joystick_y_counts))
    {
        result.faults |= APP_FAULT_JOYSTICK_Y_RANGE;
    }

    if (input->dma_overrun_detected)
    {
        result.faults |= APP_FAULT_ADC_OVERRUN;
    }

    if (!input->power_good)
    {
        result.faults |= APP_FAULT_POWER_GOOD_LOST;
    }

    result.valid = (result.faults == APP_FAULT_NONE);

    result.neutral = result.valid &&
                     (input->joystick_x_counts >= APP_ADC_NEUTRAL_LOW_COUNTS) &&
                     (input->joystick_x_counts <= APP_ADC_NEUTRAL_HIGH_COUNTS) &&
                     (input->joystick_y_counts >= APP_ADC_NEUTRAL_LOW_COUNTS) &&
                     (input->joystick_y_counts <= APP_ADC_NEUTRAL_HIGH_COUNTS);

    return result;
}
