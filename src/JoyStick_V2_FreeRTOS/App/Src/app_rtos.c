#include "app_rtos.h"

#include <stddef.h>

#include "app_build_config.h"
#include "app_tasks.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#define APP_SAFETY_TASK_STACK_WORDS            (512U)
#define APP_LINK_TASK_STACK_WORDS              (384U)
#define APP_HMI_TASK_STACK_WORDS               (256U)
#define APP_DIAGNOSTICS_TASK_STACK_WORDS       (256U)

static StaticTask_t s_safety_task_control_block;
static StackType_t s_safety_task_stack[APP_SAFETY_TASK_STACK_WORDS];
static TaskHandle_t s_safety_task_handle = NULL;

static StaticTask_t s_link_task_control_block;
static StackType_t s_link_task_stack[APP_LINK_TASK_STACK_WORDS];

static StaticTask_t s_hmi_task_control_block;
static StackType_t s_hmi_task_stack[APP_HMI_TASK_STACK_WORDS];

static StaticTask_t s_diagnostics_task_control_block;
static StackType_t s_diagnostics_task_stack[APP_DIAGNOSTICS_TASK_STACK_WORDS];

static StaticQueue_t s_authorized_command_queue_control_block;
static uint8_t s_authorized_command_queue_storage[
    sizeof(AppAuthorizedDriveCommand)];
static QueueHandle_t s_authorized_command_queue = NULL;

static StaticQueue_t s_motor_link_status_queue_control_block;
static uint8_t s_motor_link_status_queue_storage[
    sizeof(AppMotorLinkStatus)];
static QueueHandle_t s_motor_link_status_queue = NULL;

bool AppRtos_CreateStaticObjects(void)
{
    s_authorized_command_queue = xQueueCreateStatic(
        1U,
        sizeof(AppAuthorizedDriveCommand),
        s_authorized_command_queue_storage,
        &s_authorized_command_queue_control_block);

    s_motor_link_status_queue = xQueueCreateStatic(
        1U,
        sizeof(AppMotorLinkStatus),
        s_motor_link_status_queue_storage,
        &s_motor_link_status_queue_control_block);

    if ((s_authorized_command_queue == NULL) ||
        (s_motor_link_status_queue == NULL))
    {
        return false;
    }

    s_safety_task_handle = xTaskCreateStatic(
        SafetyControlTask_Run,
        "SafetyControl",
        APP_SAFETY_TASK_STACK_WORDS,
        NULL,
        5U,
        s_safety_task_stack,
        &s_safety_task_control_block);

    if (s_safety_task_handle == NULL)
    {
        return false;
    }

    if (xTaskCreateStatic(
            Rs485LinkTask_Run,
            "Rs485Link",
            APP_LINK_TASK_STACK_WORDS,
            NULL,
            4U,
            s_link_task_stack,
            &s_link_task_control_block) == NULL)
    {
        return false;
    }

    if (xTaskCreateStatic(
            HmiTask_Run,
            "Hmi",
            APP_HMI_TASK_STACK_WORDS,
            NULL,
            2U,
            s_hmi_task_stack,
            &s_hmi_task_control_block) == NULL)
    {
        return false;
    }

    if (xTaskCreateStatic(
            DiagnosticsTask_Run,
            "Diagnostics",
            APP_DIAGNOSTICS_TASK_STACK_WORDS,
            NULL,
            1U,
            s_diagnostics_task_stack,
            &s_diagnostics_task_control_block) == NULL)
    {
        return false;
    }

    return true;
}

void AppRtos_NotifyAdcBatchFromIsr(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (s_safety_task_handle != NULL)
    {
        vTaskNotifyGiveFromISR(
            s_safety_task_handle,
            &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

bool AppRtos_PublishAuthorizedCommand(
    const AppAuthorizedDriveCommand *command)
{
    if ((command == NULL) || (s_authorized_command_queue == NULL))
    {
        return false;
    }

    return xQueueOverwrite(s_authorized_command_queue, command) == pdPASS;
}

bool AppRtos_ReadAuthorizedCommand(
    AppAuthorizedDriveCommand *command)
{
    if ((command == NULL) || (s_authorized_command_queue == NULL))
    {
        return false;
    }

    return xQueuePeek(s_authorized_command_queue, command, 0U) == pdPASS;
}

bool AppRtos_PublishMotorLinkStatus(
    const AppMotorLinkStatus *status)
{
    if ((status == NULL) || (s_motor_link_status_queue == NULL))
    {
        return false;
    }

    return xQueueOverwrite(s_motor_link_status_queue, status) == pdPASS;
}

bool AppRtos_ReadMotorLinkStatus(
    AppMotorLinkStatus *status)
{
    if ((status == NULL) || (s_motor_link_status_queue == NULL))
    {
        return false;
    }

    return xQueuePeek(s_motor_link_status_queue, status, 0U) == pdPASS;
}
