#ifndef APP_RTOS_H
#define APP_RTOS_H

#include <stdbool.h>

#include "app_types.h"

bool AppRtos_CreateStaticObjects(void);
void AppRtos_NotifyAdcBatchFromIsr(void);
bool AppRtos_PublishAuthorizedCommand(
    const AppAuthorizedDriveCommand *command);
bool AppRtos_ReadAuthorizedCommand(
    AppAuthorizedDriveCommand *command);
bool AppRtos_PublishMotorLinkStatus(
    const AppMotorLinkStatus *status);
bool AppRtos_ReadMotorLinkStatus(
    AppMotorLinkStatus *status);

#endif /* APP_RTOS_H */
