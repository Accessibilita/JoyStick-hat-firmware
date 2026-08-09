#ifndef APP_BUILD_CONFIG_H
#define APP_BUILD_CONFIG_H

#include <stdint.h>

/*
 * Phase 1 is intentionally incapable of using the PCB's current RS-485 data wiring.
 * This macro is checked at compile time in rs485_link_task.c.
 */
#define APP_RS485_PHYSICAL_LINK_ENABLE          (0U)

#define APP_SAFETY_PERIOD_MS                    (5U)
#define APP_RS485_TASK_PERIOD_MS                (10U)
#define APP_HMI_TASK_PERIOD_MS                  (20U)
#define APP_DIAGNOSTICS_TASK_PERIOD_MS          (100U)

#define APP_ADC_AXIS_COUNT                      (2U)
#define APP_ADC_SAMPLES_PER_AXIS_PER_BATCH      (5U)
#define APP_ADC_DMA_HALF_SAMPLE_COUNT           \
    (APP_ADC_AXIS_COUNT * APP_ADC_SAMPLES_PER_AXIS_PER_BATCH)
#define APP_ADC_DMA_TOTAL_SAMPLE_COUNT          (2U * APP_ADC_DMA_HALF_SAMPLE_COUNT)

#define APP_ADC_ENGINEERING_MIN_COUNTS          (96U)
#define APP_ADC_ENGINEERING_MAX_COUNTS          (3999U)
#define APP_ADC_NEUTRAL_LOW_COUNTS              (1800U)
#define APP_ADC_NEUTRAL_HIGH_COUNTS             (2296U)
#define APP_NEUTRAL_QUALIFICATION_MS            (500U)
#define APP_INPUT_MAX_AGE_MS                    (15U)
#define APP_LINK_MAX_AGE_MS                     (50U)

#define APP_MOTOR_COMMAND_MAX_AGE_MS            (20U)
#define APP_WATCHDOG_STARTUP_GRACE_MS           (1000U)
#define APP_WATCHDOG_TIMEOUT_APPROX_MS          (2000U)

#if defined(DEBUG)
#define APP_DEBUG_BUILD                         (1U)
#else
#define APP_DEBUG_BUILD                         (0U)
#endif

#endif /* APP_BUILD_CONFIG_H */
