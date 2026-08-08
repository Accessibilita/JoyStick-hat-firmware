#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED

#define HSE_VALUE               8000000U
#define HSE_STARTUP_TIMEOUT     100U
#define LSE_VALUE               32768U
#define LSE_STARTUP_TIMEOUT     5000U
#define EXTERNAL_CLOCK_VALUE    12288000U
#define HSI_VALUE               16000000U
#define LSI_VALUE               32000U
#define VDD_VALUE               3300U
#define TICK_INT_PRIORITY       15U
#define USE_RTOS                0U
#define PREFETCH_ENABLE         1U
#define INSTRUCTION_CACHE_ENABLE 1U
#define DATA_CACHE_ENABLE       1U


/*
 * HAL callback registration configuration.
 *
 * Runtime callback registration is intentionally disabled.  The application
 * uses the normal STM32 HAL weak-callback mechanism where callbacks are
 * required.  These definitions match the ST HAL configuration defaults and
 * are explicitly present so -Wundef remains useful as a project-wide check.
 */
#define USE_HAL_ADC_REGISTER_CALLBACKS         0U
#define USE_HAL_CAN_REGISTER_CALLBACKS         0U
#define USE_HAL_CEC_REGISTER_CALLBACKS         0U
#define USE_HAL_CRYP_REGISTER_CALLBACKS        0U
#define USE_HAL_DAC_REGISTER_CALLBACKS         0U
#define USE_HAL_DCMI_REGISTER_CALLBACKS        0U
#define USE_HAL_DFSDM_REGISTER_CALLBACKS       0U
#define USE_HAL_DMA2D_REGISTER_CALLBACKS       0U
#define USE_HAL_DSI_REGISTER_CALLBACKS         0U
#define USE_HAL_ETH_REGISTER_CALLBACKS         0U
#define USE_HAL_HASH_REGISTER_CALLBACKS        0U
#define USE_HAL_HCD_REGISTER_CALLBACKS         0U
#define USE_HAL_I2C_REGISTER_CALLBACKS         0U
#define USE_HAL_FMPI2C_REGISTER_CALLBACKS      0U
#define USE_HAL_FMPSMBUS_REGISTER_CALLBACKS    0U
#define USE_HAL_I2S_REGISTER_CALLBACKS         0U
#define USE_HAL_IRDA_REGISTER_CALLBACKS        0U
#define USE_HAL_LPTIM_REGISTER_CALLBACKS       0U
#define USE_HAL_LTDC_REGISTER_CALLBACKS        0U
#define USE_HAL_MMC_REGISTER_CALLBACKS         0U
#define USE_HAL_NAND_REGISTER_CALLBACKS        0U
#define USE_HAL_NOR_REGISTER_CALLBACKS         0U
#define USE_HAL_PCCARD_REGISTER_CALLBACKS      0U
#define USE_HAL_PCD_REGISTER_CALLBACKS         0U
#define USE_HAL_QSPI_REGISTER_CALLBACKS        0U
#define USE_HAL_RNG_REGISTER_CALLBACKS         0U
#define USE_HAL_RTC_REGISTER_CALLBACKS         0U
#define USE_HAL_SAI_REGISTER_CALLBACKS         0U
#define USE_HAL_SD_REGISTER_CALLBACKS          0U
#define USE_HAL_SMARTCARD_REGISTER_CALLBACKS   0U
#define USE_HAL_SDRAM_REGISTER_CALLBACKS       0U
#define USE_HAL_SRAM_REGISTER_CALLBACKS        0U
#define USE_HAL_SPDIFRX_REGISTER_CALLBACKS     0U
#define USE_HAL_SMBUS_REGISTER_CALLBACKS       0U
#define USE_HAL_SPI_REGISTER_CALLBACKS         0U
#define USE_HAL_TIM_REGISTER_CALLBACKS         0U
#define USE_HAL_UART_REGISTER_CALLBACKS        0U
#define USE_HAL_USART_REGISTER_CALLBACKS       0U
#define USE_HAL_WWDG_REGISTER_CALLBACKS        0U

#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_rcc_ex.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_adc_ex.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_iwdg.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_pwr_ex.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define assert_param(expr) ((void)0U)
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
