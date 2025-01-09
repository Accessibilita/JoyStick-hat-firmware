/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for JoyStick_Task */
osThreadId_t JoyStick_TaskHandle;
const osThreadAttr_t JoyStick_Task_attributes = {
  .name = "JoyStick_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for LED_HUD_Task */
osThreadId_t LED_HUD_TaskHandle;
const osThreadAttr_t LED_HUD_Task_attributes = {
  .name = "LED_HUD_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Rotary_Switch */
osThreadId_t Rotary_SwitchHandle;
const osThreadAttr_t Rotary_Switch_attributes = {
  .name = "Rotary_Switch",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime2,
};
/* Definitions for Button_Task */
osThreadId_t Button_TaskHandle;
const osThreadAttr_t Button_Task_attributes = {
  .name = "Button_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for RS485_Task */
osThreadId_t RS485_TaskHandle;
const osThreadAttr_t RS485_Task_attributes = {
  .name = "RS485_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime1,
};
/* Definitions for debug_LED_Task */
osThreadId_t debug_LED_TaskHandle;
const osThreadAttr_t debug_LED_Task_attributes = {
  .name = "debug_LED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for EXP_1_Task */
osThreadId_t EXP_1_TaskHandle;
const osThreadAttr_t EXP_1_Task_attributes = {
  .name = "EXP_1_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for EXP_2_Task */
osThreadId_t EXP_2_TaskHandle;
const osThreadAttr_t EXP_2_Task_attributes = {
  .name = "EXP_2_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};
/* Definitions for myTimer01 */
osTimerId_t myTimer01Handle;
const osTimerAttr_t myTimer01_attributes = {
  .name = "myTimer01"
};
/* Definitions for myTimer02 */
osTimerId_t myTimer02Handle;
const osTimerAttr_t myTimer02_attributes = {
  .name = "myTimer02"
};
/* Definitions for myTimer03 */
osTimerId_t myTimer03Handle;
const osTimerAttr_t myTimer03_attributes = {
  .name = "myTimer03"
};
/* Definitions for USART2_Sem01 */
osSemaphoreId_t USART2_Sem01Handle;
const osSemaphoreAttr_t USART2_Sem01_attributes = {
  .name = "USART2_Sem01"
};
/* Definitions for ADC_Sem02 */
osSemaphoreId_t ADC_Sem02Handle;
const osSemaphoreAttr_t ADC_Sem02_attributes = {
  .name = "ADC_Sem02"
};
/* Definitions for UART5_Sem03 */
osSemaphoreId_t UART5_Sem03Handle;
const osSemaphoreAttr_t UART5_Sem03_attributes = {
  .name = "UART5_Sem03"
};
/* Definitions for USART1_Sem04 */
osSemaphoreId_t USART1_Sem04Handle;
const osSemaphoreAttr_t USART1_Sem04_attributes = {
  .name = "USART1_Sem04"
};
/* Definitions for SPI1_Sem05 */
osSemaphoreId_t SPI1_Sem05Handle;
const osSemaphoreAttr_t SPI1_Sem05_attributes = {
  .name = "SPI1_Sem05"
};
/* Definitions for I2C1_Sem06 */
osSemaphoreId_t I2C1_Sem06Handle;
const osSemaphoreAttr_t I2C1_Sem06_attributes = {
  .name = "I2C1_Sem06"
};
/* Definitions for GPIO_Sem07 */
osSemaphoreId_t GPIO_Sem07Handle;
const osSemaphoreAttr_t GPIO_Sem07_attributes = {
  .name = "GPIO_Sem07"
};
/* Definitions for myCountingSem01 */
osSemaphoreId_t myCountingSem01Handle;
const osSemaphoreAttr_t myCountingSem01_attributes = {
  .name = "myCountingSem01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void Start_Joystick_Task(void *argument);
void StartLED_HUD_Task(void *argument);
void Start_Rotary_Switch_Task(void *argument);
void Start_Button_Task(void *argument);
void StartTask06(void *argument);
void Start_debug_LED_Task(void *argument);
void Start_EXP_1_Task(void *argument);
void Start_EXP_2_Task(void *argument);
void Callback01(void *argument);
void Callback02(void *argument);
void Callback03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of USART2_Sem01 */
  USART2_Sem01Handle = osSemaphoreNew(1, 1, &USART2_Sem01_attributes);

  /* creation of ADC_Sem02 */
  ADC_Sem02Handle = osSemaphoreNew(1, 1, &ADC_Sem02_attributes);

  /* creation of UART5_Sem03 */
  UART5_Sem03Handle = osSemaphoreNew(1, 1, &UART5_Sem03_attributes);

  /* creation of USART1_Sem04 */
  USART1_Sem04Handle = osSemaphoreNew(1, 1, &USART1_Sem04_attributes);

  /* creation of SPI1_Sem05 */
  SPI1_Sem05Handle = osSemaphoreNew(1, 1, &SPI1_Sem05_attributes);

  /* creation of I2C1_Sem06 */
  I2C1_Sem06Handle = osSemaphoreNew(1, 1, &I2C1_Sem06_attributes);

  /* creation of GPIO_Sem07 */
  GPIO_Sem07Handle = osSemaphoreNew(1, 1, &GPIO_Sem07_attributes);

  /* creation of myCountingSem01 */
  myCountingSem01Handle = osSemaphoreNew(2, 0, &myCountingSem01_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of myTimer01 */
  myTimer01Handle = osTimerNew(Callback01, osTimerPeriodic, NULL, &myTimer01_attributes);

  /* creation of myTimer02 */
  myTimer02Handle = osTimerNew(Callback02, osTimerPeriodic, NULL, &myTimer02_attributes);

  /* creation of myTimer03 */
  myTimer03Handle = osTimerNew(Callback03, osTimerPeriodic, NULL, &myTimer03_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of JoyStick_Task */
  JoyStick_TaskHandle = osThreadNew(Start_Joystick_Task, NULL, &JoyStick_Task_attributes);

  /* creation of LED_HUD_Task */
  LED_HUD_TaskHandle = osThreadNew(StartLED_HUD_Task, NULL, &LED_HUD_Task_attributes);

  /* creation of Rotary_Switch */
  Rotary_SwitchHandle = osThreadNew(Start_Rotary_Switch_Task, NULL, &Rotary_Switch_attributes);

  /* creation of Button_Task */
  Button_TaskHandle = osThreadNew(Start_Button_Task, NULL, &Button_Task_attributes);

  /* creation of RS485_Task */
  RS485_TaskHandle = osThreadNew(StartTask06, NULL, &RS485_Task_attributes);

  /* creation of debug_LED_Task */
  debug_LED_TaskHandle = osThreadNew(Start_debug_LED_Task, NULL, &debug_LED_Task_attributes);

  /* creation of EXP_1_Task */
  EXP_1_TaskHandle = osThreadNew(Start_EXP_1_Task, NULL, &EXP_1_Task_attributes);

  /* creation of EXP_2_Task */
  EXP_2_TaskHandle = osThreadNew(Start_EXP_2_Task, NULL, &EXP_2_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Start_Joystick_Task */
/**
* @brief Function implementing the JoyStick_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Joystick_Task */
void Start_Joystick_Task(void *argument)
{
  /* USER CODE BEGIN Start_Joystick_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Joystick_Task */
}

/* USER CODE BEGIN Header_StartLED_HUD_Task */
/**
* @brief Function implementing the LED_HUD_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLED_HUD_Task */
void StartLED_HUD_Task(void *argument)
{
  /* USER CODE BEGIN StartLED_HUD_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLED_HUD_Task */
}

/* USER CODE BEGIN Header_Start_Rotary_Switch_Task */
/**
* @brief Function implementing the Rotary_Switch thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Rotary_Switch_Task */
void Start_Rotary_Switch_Task(void *argument)
{
  /* USER CODE BEGIN Start_Rotary_Switch_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Rotary_Switch_Task */
}

/* USER CODE BEGIN Header_Start_Button_Task */
/**
* @brief Function implementing the Button_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Button_Task */
void Start_Button_Task(void *argument)
{
  /* USER CODE BEGIN Start_Button_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Button_Task */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
* @brief Function implementing the RS485_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask06 */
void StartTask06(void *argument)
{
  /* USER CODE BEGIN StartTask06 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask06 */
}

/* USER CODE BEGIN Header_Start_debug_LED_Task */
/**
* @brief Function implementing the debug_LED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_debug_LED_Task */
void Start_debug_LED_Task(void *argument)
{
  /* USER CODE BEGIN Start_debug_LED_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_debug_LED_Task */
}

/* USER CODE BEGIN Header_Start_EXP_1_Task */
/**
* @brief Function implementing the EXP_1_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_EXP_1_Task */
void Start_EXP_1_Task(void *argument)
{
  /* USER CODE BEGIN Start_EXP_1_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_EXP_1_Task */
}

/* USER CODE BEGIN Header_Start_EXP_2_Task */
/**
* @brief Function implementing the EXP_2_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_EXP_2_Task */
void Start_EXP_2_Task(void *argument)
{
  /* USER CODE BEGIN Start_EXP_2_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_EXP_2_Task */
}

/* Callback01 function */
void Callback01(void *argument)
{
  /* USER CODE BEGIN Callback01 */

  /* USER CODE END Callback01 */
}

/* Callback02 function */
void Callback02(void *argument)
{
  /* USER CODE BEGIN Callback02 */

  /* USER CODE END Callback02 */
}

/* Callback03 function */
void Callback03(void *argument)
{
  /* USER CODE BEGIN Callback03 */

  /* USER CODE END Callback03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

