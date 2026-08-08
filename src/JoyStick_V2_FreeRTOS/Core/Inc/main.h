#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void Error_Handler(void);

/* ADC joystick channels */
#define JOYSTICK_Y_Pin                 GPIO_PIN_0
#define JOYSTICK_Y_GPIO_Port           GPIOA
#define JOYSTICK_X_Pin                 GPIO_PIN_1
#define JOYSTICK_X_GPIO_Port           GPIOA

/* TLC5947-style LED HUD control */
#define LED_XLAT_1_Pin                 GPIO_PIN_4
#define LED_XLAT_1_GPIO_Port           GPIOA
#define LED_SCLK_Pin                   GPIO_PIN_5
#define LED_SCLK_GPIO_Port             GPIOA
#define LED_BLANK_2_Pin                GPIO_PIN_6
#define LED_BLANK_2_GPIO_Port          GPIOA
#define LED_MOSI_Pin                   GPIO_PIN_7
#define LED_MOSI_GPIO_Port             GPIOA
#define LED_BLANK_1_Pin                GPIO_PIN_4
#define LED_BLANK_1_GPIO_Port          GPIOC
#define LED_XLAT_2_Pin                 GPIO_PIN_5
#define LED_XLAT_2_GPIO_Port           GPIOC

/* MAX3535 controls. Schematic net labels are misleading; names reflect pin function. */
#define RS485_nRE_Pin                  GPIO_PIN_8
#define RS485_nRE_GPIO_Port            GPIOC
#define RS485_DE_Pin                   GPIO_PIN_9
#define RS485_DE_GPIO_Port             GPIOC
#define RS485_DI_SAFE_Pin              GPIO_PIN_7
#define RS485_DI_SAFE_GPIO_Port        GPIOE
#define RS485_RO_SENSE_Pin             GPIO_PIN_8
#define RS485_RO_SENSE_GPIO_Port       GPIOE

/* Four active-low momentary buttons */
#define BUTTON_1_Pin                   GPIO_PIN_9
#define BUTTON_1_GPIO_Port             GPIOE
#define BUTTON_2_Pin                   GPIO_PIN_10
#define BUTTON_2_GPIO_Port             GPIOE
#define BUTTON_3_Pin                   GPIO_PIN_11
#define BUTTON_3_GPIO_Port             GPIOE
#define BUTTON_4_Pin                   GPIO_PIN_12
#define BUTTON_4_GPIO_Port             GPIOE
#define POWER_GOOD_Pin                 GPIO_PIN_13
#define POWER_GOOD_GPIO_Port           GPIOE

/* Active-low rotary contacts */
#define ROTARY_1_POLE_1_Pin            GPIO_PIN_8
#define ROTARY_1_POLE_1_GPIO_Port      GPIOD
#define ROTARY_2_POLE_1_Pin            GPIO_PIN_9
#define ROTARY_2_POLE_1_GPIO_Port      GPIOD
#define ROTARY_1_POLE_2_Pin            GPIO_PIN_10
#define ROTARY_1_POLE_2_GPIO_Port      GPIOD
#define ROTARY_2_POLE_2_Pin            GPIO_PIN_11
#define ROTARY_2_POLE_2_GPIO_Port      GPIOD
#define ROTARY_1_POLE_3_Pin            GPIO_PIN_12
#define ROTARY_1_POLE_3_GPIO_Port      GPIOD
#define ROTARY_2_POLE_3_Pin            GPIO_PIN_13
#define ROTARY_2_POLE_3_GPIO_Port      GPIOD

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
