#include "board_io.h"

#include "main.h"

void BoardIo_ForceRs485SafeDisabled(void)
{
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RS485_nRE_GPIO_Port, RS485_nRE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RS485_DI_SAFE_GPIO_Port, RS485_DI_SAFE_Pin, GPIO_PIN_RESET);
}

void BoardIo_ForceLedHudBlanked(void)
{
    HAL_GPIO_WritePin(LED_BLANK_1_GPIO_Port, LED_BLANK_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_BLANK_2_GPIO_Port, LED_BLANK_2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_XLAT_1_GPIO_Port, LED_XLAT_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_XLAT_2_GPIO_Port, LED_XLAT_2_Pin, GPIO_PIN_RESET);
}

uint16_t BoardIo_ReadButtonsActiveLow(void)
{
    uint16_t result = 0U;

    result |= (HAL_GPIO_ReadPin(BUTTON_1_GPIO_Port, BUTTON_1_Pin) == GPIO_PIN_RESET) ? (1U << 0) : 0U;
    result |= (HAL_GPIO_ReadPin(BUTTON_2_GPIO_Port, BUTTON_2_Pin) == GPIO_PIN_RESET) ? (1U << 1) : 0U;
    result |= (HAL_GPIO_ReadPin(BUTTON_3_GPIO_Port, BUTTON_3_Pin) == GPIO_PIN_RESET) ? (1U << 2) : 0U;
    result |= (HAL_GPIO_ReadPin(BUTTON_4_GPIO_Port, BUTTON_4_Pin) == GPIO_PIN_RESET) ? (1U << 3) : 0U;

    return result;
}

uint8_t BoardIo_ReadRotary1ActiveLow(void)
{
    uint8_t result = 0U;

    result |= (HAL_GPIO_ReadPin(ROTARY_1_POLE_1_GPIO_Port, ROTARY_1_POLE_1_Pin) == GPIO_PIN_RESET) ? (1U << 0) : 0U;
    result |= (HAL_GPIO_ReadPin(ROTARY_1_POLE_2_GPIO_Port, ROTARY_1_POLE_2_Pin) == GPIO_PIN_RESET) ? (1U << 1) : 0U;
    result |= (HAL_GPIO_ReadPin(ROTARY_1_POLE_3_GPIO_Port, ROTARY_1_POLE_3_Pin) == GPIO_PIN_RESET) ? (1U << 2) : 0U;

    return result;
}

uint8_t BoardIo_ReadRotary2ActiveLow(void)
{
    uint8_t result = 0U;

    result |= (HAL_GPIO_ReadPin(ROTARY_2_POLE_1_GPIO_Port, ROTARY_2_POLE_1_Pin) == GPIO_PIN_RESET) ? (1U << 0) : 0U;
    result |= (HAL_GPIO_ReadPin(ROTARY_2_POLE_2_GPIO_Port, ROTARY_2_POLE_2_Pin) == GPIO_PIN_RESET) ? (1U << 1) : 0U;
    result |= (HAL_GPIO_ReadPin(ROTARY_2_POLE_3_GPIO_Port, ROTARY_2_POLE_3_Pin) == GPIO_PIN_RESET) ? (1U << 2) : 0U;

    return result;
}

bool BoardIo_IsPowerGood(void)
{
    return HAL_GPIO_ReadPin(POWER_GOOD_GPIO_Port, POWER_GOOD_Pin) == GPIO_PIN_SET;
}
