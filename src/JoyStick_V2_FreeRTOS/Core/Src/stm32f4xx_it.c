/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "main.h"
#include "stm32f4xx_it.h"
#include "adc.h"
#include "input_acquisition.h"
#include "FreeRTOS.h"
#include "task.h"

extern void xPortSysTickHandler(void);

void NMI_Handler(void)
{
    Error_Handler();
}

void HardFault_Handler(void)
{
    Error_Handler();
}

void MemManage_Handler(void)
{
    Error_Handler();
}

void BusFault_Handler(void)
{
    Error_Handler();
}

void UsageFault_Handler(void)
{
    Error_Handler();
}

void DebugMon_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *adc_handle)
{
    if (adc_handle->Instance == ADC1)
    {
        InputAcquisition_OnDmaHalfCompleteFromIsr();
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adc_handle)
{
    if (adc_handle->Instance == ADC1)
    {
        InputAcquisition_OnDmaCompleteFromIsr();
    }
}
