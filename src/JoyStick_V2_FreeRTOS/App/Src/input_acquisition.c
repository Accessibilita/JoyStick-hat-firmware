/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "input_acquisition.h"

#include <stddef.h>

#include "app_build_config.h"
#include "app_rtos.h"
#include "board_io.h"
#include "adc.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"

#if defined(__GNUC__)
#define APP_ALIGN_4 __attribute__((aligned(4)))
#else
#define APP_ALIGN_4
#endif

static uint16_t s_adc_dma_buffer[APP_ADC_DMA_TOTAL_SAMPLE_COUNT] APP_ALIGN_4;
static volatile uint32_t s_completed_sequence = 0U;
static volatile uint8_t s_completed_half = 0U;
static uint32_t s_last_consumed_sequence = 0U;

static uint16_t InputAcquisition_AverageAxis(
    const uint16_t *samples,
    uint32_t axis_index)
{
    uint32_t sample_index;
    uint32_t sum = 0U;

    for (sample_index = 0U;
         sample_index < APP_ADC_SAMPLES_PER_AXIS_PER_BATCH;
         sample_index++)
    {
        const uint32_t buffer_index =
            (sample_index * APP_ADC_AXIS_COUNT) + axis_index;
        sum += samples[buffer_index];
    }

    return (uint16_t)(sum / APP_ADC_SAMPLES_PER_AXIS_PER_BATCH);
}

bool InputAcquisition_Start(void)
{
    HAL_StatusTypeDef adc_status;
    HAL_StatusTypeDef timer_status;

    adc_status = HAL_ADC_Start_DMA(
        &hadc1,
        (uint32_t *)(void *)s_adc_dma_buffer,
        APP_ADC_DMA_TOTAL_SAMPLE_COUNT);

    if (adc_status != HAL_OK)
    {
        return false;
    }

    timer_status = HAL_TIM_Base_Start(&htim2);
    if (timer_status != HAL_OK)
    {
        (void)HAL_ADC_Stop_DMA(&hadc1);
        return false;
    }

    return true;
}

bool InputAcquisition_CopyCompletedBatch(AppRawInputSnapshot *snapshot)
{
    uint16_t local_samples[APP_ADC_DMA_HALF_SAMPLE_COUNT];
    uint32_t sequence_before = 0U;
    uint32_t sequence_after = 0U;
    uint32_t sample_index;
    uint8_t half_index = 0U;
    bool copy_raced_dma = false;
    uint32_t attempt;

    if (snapshot == NULL)
    {
        return false;
    }

    /*
     * DMA alternates between two halves. Copying into a task-local array keeps
     * all later calculations independent of DMA. Rechecking the publication
     * sequence detects the case where the task was delayed long enough for DMA
     * to cycle back into the half being copied.
     */
    for (attempt = 0U; attempt < 2U; attempt++)
    {
        taskENTER_CRITICAL();
        sequence_before = s_completed_sequence;
        half_index = s_completed_half;
        taskEXIT_CRITICAL();

        if (sequence_before == s_last_consumed_sequence)
        {
            return false;
        }

        for (sample_index = 0U;
             sample_index < APP_ADC_DMA_HALF_SAMPLE_COUNT;
             sample_index++)
        {
            local_samples[sample_index] = s_adc_dma_buffer[
                ((uint32_t)half_index * APP_ADC_DMA_HALF_SAMPLE_COUNT) +
                sample_index];
        }

        taskENTER_CRITICAL();
        sequence_after = s_completed_sequence;
        taskEXIT_CRITICAL();

        if (sequence_after == sequence_before)
        {
            break;
        }

        copy_raced_dma = true;
    }

    if (sequence_after != sequence_before)
    {
        return false;
    }

    snapshot->sequence = sequence_before;
    snapshot->captured_at_ms = HAL_GetTick();
    snapshot->joystick_y_counts = InputAcquisition_AverageAxis(local_samples, 0U);
    snapshot->joystick_x_counts = InputAcquisition_AverageAxis(local_samples, 1U);
    snapshot->buttons_active_low = BoardIo_ReadButtonsActiveLow();
    snapshot->rotary_1_active_low = BoardIo_ReadRotary1ActiveLow();
    snapshot->rotary_2_active_low = BoardIo_ReadRotary2ActiveLow();
    snapshot->power_good = BoardIo_IsPowerGood();
    snapshot->dma_overrun_detected = copy_raced_dma ||
        ((sequence_before - s_last_consumed_sequence) > 1U);

    s_last_consumed_sequence = sequence_before;
    return true;
}

void InputAcquisition_OnDmaHalfCompleteFromIsr(void)
{
    s_completed_half = 0U;
    s_completed_sequence++;
    AppRtos_NotifyAdcBatchFromIsr();
}

void InputAcquisition_OnDmaCompleteFromIsr(void)
{
    s_completed_half = 1U;
    s_completed_sequence++;
    AppRtos_NotifyAdcBatchFromIsr();
}

uint16_t *InputAcquisition_GetDmaBuffer(void)
{
    return s_adc_dma_buffer;
}

uint32_t InputAcquisition_GetDmaBufferLength(void)
{
    return APP_ADC_DMA_TOTAL_SAMPLE_COUNT;
}
