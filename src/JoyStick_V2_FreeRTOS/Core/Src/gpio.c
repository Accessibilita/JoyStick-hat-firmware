/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "gpio.h"

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* Establish safe output levels before switching pins to output mode. */
    HAL_GPIO_WritePin(GPIOA, LED_XLAT_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, LED_BLANK_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, LED_BLANK_1_Pin | LED_XLAT_2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, LED_BLANK_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, RS485_nRE_Pin | RS485_DE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOE, RS485_DI_SAFE_Pin, GPIO_PIN_RESET);

    gpio.Pin = LED_XLAT_1_Pin | LED_BLANK_2_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = LED_BLANK_1_Pin | LED_XLAT_2_Pin |
               RS485_nRE_Pin | RS485_DE_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

    gpio.Pin = RS485_DI_SAFE_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = RS485_RO_SENSE_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = BUTTON_1_Pin | BUTTON_2_Pin | BUTTON_3_Pin |
               BUTTON_4_Pin | POWER_GOOD_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = ROTARY_1_POLE_1_Pin | ROTARY_2_POLE_1_Pin |
               ROTARY_1_POLE_2_Pin | ROTARY_2_POLE_2_Pin |
               ROTARY_1_POLE_3_Pin | ROTARY_2_POLE_3_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &gpio);
}
