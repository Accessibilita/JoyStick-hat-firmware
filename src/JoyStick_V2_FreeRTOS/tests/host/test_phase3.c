/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_types.h"
#include "joystick_calibration.h"
#include "joystick_configuration.h"
#include "joystick_processing.h"

static JoystickConfigurationImage MakeNominalConfiguration(void)
{
    JoystickConfigurationImage image;

    image.generation = 7U;
    image.configuration.x_axis.minimum_counts = 320U;
    image.configuration.x_axis.center_counts = 2050U;
    image.configuration.x_axis.maximum_counts = 3770U;
    image.configuration.x_axis.deadband_counts = 40U;
    image.configuration.x_axis.inverted = false;
    image.configuration.y_axis.minimum_counts = 280U;
    image.configuration.y_axis.center_counts = 2020U;
    image.configuration.y_axis.maximum_counts = 3810U;
    image.configuration.y_axis.deadband_counts = 48U;
    image.configuration.y_axis.inverted = false;
    image.configuration.maximum_speed_q15 = JOYSTICK_CONFIG_Q15_MAX;
    image.configuration.response_curve_q15 = 0U;
    image.configuration.reference_profile =
        JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2;

    return image;
}

static AppRawInputSnapshot MakeInput(uint16_t x_counts, uint16_t y_counts)
{
    AppRawInputSnapshot input = {0};

    input.sequence = 42U;
    input.captured_at_ms = 100U;
    input.joystick_x_counts = x_counts;
    input.joystick_y_counts = y_counts;
    input.power_good = true;

    return input;
}

static void Test_Crc32GoldenVector(void)
{
    static const uint8_t golden[] =
    {
        (uint8_t)'1', (uint8_t)'2', (uint8_t)'3',
        (uint8_t)'4', (uint8_t)'5', (uint8_t)'6',
        (uint8_t)'7', (uint8_t)'8', (uint8_t)'9'
    };

    assert(JoystickConfiguration_Crc32(golden, sizeof(golden)) ==
           0xCBF43926UL);
}

static void Test_ConfigurationRoundTripAndCorruption(void)
{
    const JoystickConfigurationImage original = MakeNominalConfiguration();
    JoystickConfigurationImage decoded;
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE];

    assert(JoystickConfiguration_Validate(&original) ==
           JOYSTICK_CONFIG_STATUS_OK);
    assert(JoystickConfiguration_EncodeRecord(&original, record) ==
           JOYSTICK_CONFIG_STATUS_OK);
    assert(JoystickConfiguration_DecodeRecord(record, &decoded) ==
           JOYSTICK_CONFIG_STATUS_OK);
    assert(decoded.generation == original.generation);
    assert(decoded.configuration.x_axis.center_counts ==
           original.configuration.x_axis.center_counts);
    assert(decoded.configuration.y_axis.maximum_counts ==
           original.configuration.y_axis.maximum_counts);
    assert(decoded.configuration.reference_profile ==
           JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2);

    record[24] ^= 0x01U;
    assert(JoystickConfiguration_DecodeRecord(record, &decoded) ==
           JOYSTICK_CONFIG_STATUS_CRC);
}

static void Test_RedundantSlotSelectionSurvivesTornUpdate(void)
{
    JoystickConfigurationImage old_image = MakeNominalConfiguration();
    JoystickConfigurationImage new_image;
    JoystickConfigurationImage selected;
    uint8_t slot_a[JOYSTICK_CONFIG_RECORD_SIZE];
    uint8_t slot_b[JOYSTICK_CONFIG_RECORD_SIZE];
    uint8_t selected_slot = 0xFFU;

    old_image.generation = 100U;
    new_image = old_image;
    new_image.generation = 101U;
    new_image.configuration.maximum_speed_q15 = 20000U;

    assert(JoystickConfiguration_EncodeRecord(&old_image, slot_a) ==
           JOYSTICK_CONFIG_STATUS_OK);
    assert(JoystickConfiguration_EncodeRecord(&new_image, slot_b) ==
           JOYSTICK_CONFIG_STATUS_OK);

    assert(JoystickConfiguration_SelectNewestValid(
        slot_a, slot_b, &selected, &selected_slot) ==
        JOYSTICK_CONFIG_STATUS_OK);
    assert(selected_slot == 1U);
    assert(selected.generation == 101U);
    assert(selected.configuration.maximum_speed_q15 == 20000U);

    /* Model a power-loss/torn write by corrupting the newer record. */
    slot_b[40] = 0xA5U;
    assert(JoystickConfiguration_SelectNewestValid(
        slot_a, slot_b, &selected, &selected_slot) ==
        JOYSTICK_CONFIG_STATUS_OK);
    assert(selected_slot == 0U);
    assert(selected.generation == 100U);
}

static void Test_PrepareNextGeneration(void)
{
    const JoystickConfigurationImage current = MakeNominalConfiguration();
    JoystickConfiguration next_configuration = current.configuration;
    JoystickConfigurationImage next_image;

    next_configuration.maximum_speed_q15 = 24576U;
    assert(JoystickConfiguration_PrepareNext(
        &current, &next_configuration, &next_image) ==
        JOYSTICK_CONFIG_STATUS_OK);
    assert(next_image.generation == (current.generation + 1U));
    assert(next_image.configuration.maximum_speed_q15 == 24576U);
}

static void Test_CalibrationFromSyntheticReferenceSweep(void)
{
    JoystickCalibrationPolicy policy;
    JoystickCalibrationContext context;
    JoystickConfiguration configuration;
    uint16_t index;

    JoystickCalibration_GetSoftwareDefaultPolicy(&policy);
    assert(JoystickCalibration_Begin(&context, &policy));

    for (index = 0U; index < policy.center_sample_target; index++)
    {
        const uint16_t x = (uint16_t)(2048U + (index % 7U));
        const uint16_t y = (uint16_t)(2036U + (index % 5U));
        assert(JoystickCalibration_AddCenterSample(&context, x, y));
    }

    assert(JoystickCalibration_BeginSweep(&context));

    for (index = 0U; index < policy.sweep_sample_minimum; index++)
    {
        uint16_t x;
        uint16_t y;

        switch (index % 4U)
        {
            case 0U:
                x = 180U;
                y = 210U;
                break;
            case 1U:
                x = 3910U;
                y = 3870U;
                break;
            case 2U:
                x = 760U;
                y = 3320U;
                break;
            default:
                x = 3380U;
                y = 690U;
                break;
        }

        assert(JoystickCalibration_AddSweepSample(&context, x, y));
    }

    assert(JoystickCalibration_Finalize(
        &context,
        JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2,
        &configuration));
    assert(context.state == JOYSTICK_CALIBRATION_COMPLETE);
    assert(configuration.x_axis.minimum_counts ==
           (uint16_t)(180U + policy.endpoint_guard_counts));
    assert(configuration.x_axis.maximum_counts ==
           (uint16_t)(3910U - policy.endpoint_guard_counts));
    assert(configuration.y_axis.minimum_counts ==
           (uint16_t)(210U + policy.endpoint_guard_counts));
    assert(configuration.y_axis.maximum_counts ==
           (uint16_t)(3870U - policy.endpoint_guard_counts));
    assert(configuration.x_axis.deadband_counts >=
           policy.deadband_floor_counts);
    assert(configuration.y_axis.deadband_counts >=
           policy.deadband_floor_counts);
}

static void Test_CalibrationRejectsInsufficientTravel(void)
{
    JoystickCalibrationPolicy policy;
    JoystickCalibrationContext context;
    JoystickConfiguration configuration;
    uint16_t index;

    JoystickCalibration_GetSoftwareDefaultPolicy(&policy);
    assert(JoystickCalibration_Begin(&context, &policy));

    for (index = 0U; index < policy.center_sample_target; index++)
    {
        assert(JoystickCalibration_AddCenterSample(&context, 2048U, 2048U));
    }
    assert(JoystickCalibration_BeginSweep(&context));

    for (index = 0U; index < policy.sweep_sample_minimum; index++)
    {
        const uint16_t offset = (uint16_t)(index % 16U);
        assert(JoystickCalibration_AddSweepSample(
            &context,
            (uint16_t)(1900U + offset),
            (uint16_t)(1900U + offset)));
    }

    assert(!JoystickCalibration_Finalize(
        &context,
        JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2,
        &configuration));
    assert(context.state == JOYSTICK_CALIBRATION_FAILED);
}

static void Test_NormalizationDeadbandAndEndpoints(void)
{
    const JoystickConfigurationImage image = MakeNominalConfiguration();
    int16_t normalized = 123;

    assert(JoystickProcessing_NormalizeAxis(
        image.configuration.x_axis.center_counts,
        &image.configuration.x_axis,
        &normalized));
    assert(normalized == 0);

    assert(JoystickProcessing_NormalizeAxis(
        (uint16_t)(image.configuration.x_axis.center_counts + 20U),
        &image.configuration.x_axis,
        &normalized));
    assert(normalized == 0);

    assert(JoystickProcessing_NormalizeAxis(
        image.configuration.x_axis.minimum_counts,
        &image.configuration.x_axis,
        &normalized));
    assert(normalized == -32767);

    assert(JoystickProcessing_NormalizeAxis(
        image.configuration.x_axis.maximum_counts,
        &image.configuration.x_axis,
        &normalized));
    assert(normalized == 32767);
}

static void Test_AsymmetricCalibrationAndInversion(void)
{
    JoystickAxisCalibration axis;
    int16_t low_value;
    int16_t high_value;

    axis.minimum_counts = 500U;
    axis.center_counts = 1800U;
    axis.maximum_counts = 3900U;
    axis.deadband_counts = 50U;
    axis.inverted = false;

    assert(JoystickProcessing_NormalizeAxis(1150U, &axis, &low_value));
    assert(JoystickProcessing_NormalizeAxis(2850U, &axis, &high_value));
    assert(low_value < 0);
    assert(high_value > 0);

    axis.inverted = true;
    assert(JoystickProcessing_NormalizeAxis(1150U, &axis, &high_value));
    assert(high_value > 0);
}

static void Test_ResponseCurveAndSpeedLimit(void)
{
    const int16_t input = 16384;
    const int16_t linear = JoystickProcessing_ApplyResponseCurve(input, 0U);
    const int16_t cubic = JoystickProcessing_ApplyResponseCurve(
        input, JOYSTICK_CONFIG_Q15_MAX);
    const int16_t limited = JoystickProcessing_ApplySpeedLimit(
        32767, 16384U);

    assert(linear == input);
    assert(cubic > 0);
    assert(cubic < linear);
    assert((limited >= 16383) && (limited <= 16384));
    assert(JoystickProcessing_ApplyResponseCurve(-input, 0U) == -input);
}

static void Test_RequestedCommandPipeline(void)
{
    JoystickConfigurationImage image = MakeNominalConfiguration();
    AppRawInputSnapshot input = MakeInput(
        image.configuration.x_axis.maximum_counts,
        image.configuration.y_axis.minimum_counts);
    AppRequestedDriveCommand request;
    JoystickProcessedSample processed;

    image.configuration.maximum_speed_q15 = 16384U;
    image.configuration.response_curve_q15 = 0U;

    assert(JoystickProcessing_BuildRequestedDriveCommand(
        &input,
        &image,
        55U,
        1234U,
        true,
        &request,
        &processed));

    assert(request.request_sequence == 55U);
    assert(request.input_sequence == input.sequence);
    assert(request.generated_at_ms == 1234U);
    assert(request.turn_q15 > 16000);
    assert(request.forward_q15 < -16000);
    assert(request.maximum_speed_q15 == 16384U);
    assert(request.enable_request);
    assert(!processed.neutral);
}

static void Test_InvalidConfigurationFailsClosed(void)
{
    JoystickConfigurationImage image = MakeNominalConfiguration();
    const AppRawInputSnapshot input = MakeInput(3000U, 3000U);
    AppRequestedDriveCommand request;

    image.configuration.x_axis.center_counts =
        image.configuration.x_axis.minimum_counts;

    assert(!JoystickProcessing_BuildRequestedDriveCommand(
        &input,
        &image,
        1U,
        10U,
        true,
        &request,
        NULL));
    assert(request.forward_q15 == 0);
    assert(request.turn_q15 == 0);
    assert(request.maximum_speed_q15 == 0U);
    assert(!request.enable_request);
}

static uint32_t NextDeterministicRandom(uint32_t *state)
{
    *state = (*state * UINT32_C(1664525)) + UINT32_C(1013904223);
    return *state;
}

static void Test_BoundedProcessingAcrossAdcDomain(void)
{
    const JoystickConfigurationImage image = MakeNominalConfiguration();
    uint32_t random_state = UINT32_C(0x104B0003);
    uint32_t index;

    for (index = 0U; index < 20000U; index++)
    {
        const uint16_t x = (uint16_t)(NextDeterministicRandom(&random_state) & 0x0FFFU);
        const uint16_t y = (uint16_t)(NextDeterministicRandom(&random_state) & 0x0FFFU);
        const AppRawInputSnapshot input = MakeInput(x, y);
        JoystickProcessedSample processed;

        assert(JoystickProcessing_Process(&input, &image, &processed));
        assert(processed.x_q15 >= -32767);
        assert(processed.x_q15 <= 32767);
        assert(processed.y_q15 >= -32767);
        assert(processed.y_q15 <= 32767);
        assert(processed.shaped_x_q15 >= -32767);
        assert(processed.shaped_x_q15 <= 32767);
        assert(processed.shaped_y_q15 >= -32767);
        assert(processed.shaped_y_q15 <= 32767);
    }
}

static void Test_RandomRecordsNeverEscapeValidation(void)
{
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE];
    JoystickConfigurationImage image;
    uint32_t random_state = UINT32_C(0x5A17C0DE);
    uint32_t iteration;

    for (iteration = 0U; iteration < 10000U; iteration++)
    {
        size_t index;

        for (index = 0U; index < sizeof(record); index++)
        {
            record[index] = (uint8_t)(NextDeterministicRandom(&random_state) >> 24U);
        }

        if (JoystickConfiguration_DecodeRecord(record, &image) ==
            JOYSTICK_CONFIG_STATUS_OK)
        {
            assert(JoystickConfiguration_Validate(&image) ==
                   JOYSTICK_CONFIG_STATUS_OK);
        }
    }
}

int main(void)
{
    Test_Crc32GoldenVector();
    Test_ConfigurationRoundTripAndCorruption();
    Test_RedundantSlotSelectionSurvivesTornUpdate();
    Test_PrepareNextGeneration();
    Test_CalibrationFromSyntheticReferenceSweep();
    Test_CalibrationRejectsInsufficientTravel();
    Test_NormalizationDeadbandAndEndpoints();
    Test_AsymmetricCalibrationAndInversion();
    Test_ResponseCurveAndSpeedLimit();
    Test_RequestedCommandPipeline();
    Test_InvalidConfigurationFailsClosed();
    Test_BoundedProcessingAcrossAdcDomain();
    Test_RandomRecordsNeverEscapeValidation();

    puts("Phase 3 host tests passed");
    return 0;
}
