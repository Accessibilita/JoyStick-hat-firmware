#ifndef SAFETY_STATE_H
#define SAFETY_STATE_H

#include "app_types.h"

void SafetyState_Init(AppSafetyContext *context, uint32_t now_ms);
void SafetyState_Step(
    AppSafetyContext *context,
    const AppSafetyObservation *observation);
bool SafetyState_IsDriveAuthorized(const AppSafetyContext *context);

#endif /* SAFETY_STATE_H */
