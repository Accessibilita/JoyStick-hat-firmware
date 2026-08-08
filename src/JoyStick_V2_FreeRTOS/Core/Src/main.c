#include "main.h"
#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "app_rtos.h"
#include "FreeRTOS.h"
#include "task.h"

static void SystemClock_Config(void);

/*
 * FreeRTOS application hooks.
 *
 * These functions have external linkage because the FreeRTOS kernel calls
 * them by their defined hook names.  Explicit prototypes are kept here so
 * the project can retain -Wmissing-prototypes as an error for hand-owned
 * firmware code.
 */
void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name);
void vApplicationMallocFailedHook(void);
void vApplicationGetIdleTaskMemory(
    StaticTask_t **task_buffer,
    StackType_t **stack_buffer,
    uint32_t *stack_size);


int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_SPI1_Init();
    MX_TIM2_Init();

#if defined(DEBUG)
    /* Freeze IWDG before it is started so breakpoints are safe immediately. */
    __HAL_DBGMCU_FREEZE_IWDG();
#endif
    MX_IWDG_Init();

    if (!AppRtos_CreateStaticObjects())
    {
        Error_Handler();
    }

    vTaskStartScheduler();

    /* The scheduler must never return. */
    Error_Handler();
    return 0;
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscillator.HSEState = RCC_HSE_ON;
    oscillator.PLL.PLLState = RCC_PLL_ON;
    oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscillator.PLL.PLLM = 8U;
    oscillator.PLL.PLLN = 192U;
    oscillator.PLL.PLLP = RCC_PLLP_DIV2;
    oscillator.PLL.PLLQ = 4U;

    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK)
    {
        Error_Handler();
    }

    clocks.ClockType = RCC_CLOCKTYPE_HCLK |
                       RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_PCLK1 |
                       RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV4;
    clocks.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();

    /*
     * Only touch ports whose clocks are already enabled. Clock-start failures
     * can reach this function before GPIO initialization.
     */
    if (__HAL_RCC_GPIOC_IS_CLK_ENABLED())
    {
        HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_BLANK_1_GPIO_Port, LED_BLANK_1_Pin, GPIO_PIN_SET);
    }

    if (__HAL_RCC_GPIOA_IS_CLK_ENABLED())
    {
        HAL_GPIO_WritePin(LED_BLANK_2_GPIO_Port, LED_BLANK_2_Pin, GPIO_PIN_SET);
    }

    for (;;)
    {
        /* IWDG is intentionally not refreshed; reset returns to inhibited boot. */
        __NOP();
    }
}

void App_AssertFailed(const char *file, int line)
{
    (void)file;
    (void)line;
    Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    Error_Handler();
}

void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

static StaticTask_t s_idle_task_control_block;
static StackType_t s_idle_task_stack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(
    StaticTask_t **task_buffer,
    StackType_t **stack_buffer,
    uint32_t *stack_size)
{
    *task_buffer = &s_idle_task_control_block;
    *stack_buffer = s_idle_task_stack;
    *stack_size = configMINIMAL_STACK_SIZE;
}
