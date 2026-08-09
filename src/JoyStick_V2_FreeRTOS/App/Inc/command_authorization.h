#ifndef COMMAND_AUTHORIZATION_H
#define COMMAND_AUTHORIZATION_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"

typedef uint32_t CommandAuthorizationBlockMask;

#define COMMAND_AUTH_BLOCK_NONE                 ((CommandAuthorizationBlockMask)0U)
#define COMMAND_AUTH_BLOCK_CONFIGURATION        ((CommandAuthorizationBlockMask)(1UL << 0))
#define COMMAND_AUTH_BLOCK_INPUT                ((CommandAuthorizationBlockMask)(1UL << 1))
#define COMMAND_AUTH_BLOCK_TASK_HEALTH          ((CommandAuthorizationBlockMask)(1UL << 2))
#define COMMAND_AUTH_BLOCK_LINK                 ((CommandAuthorizationBlockMask)(1UL << 3))
#define COMMAND_AUTH_BLOCK_LINK_STALE           ((CommandAuthorizationBlockMask)(1UL << 4))
#define COMMAND_AUTH_BLOCK_REMOTE_NOT_READY     ((CommandAuthorizationBlockMask)(1UL << 5))
#define COMMAND_AUTH_BLOCK_REMOTE_FAULT         ((CommandAuthorizationBlockMask)(1UL << 6))
#define COMMAND_AUTH_BLOCK_SAFETY_STATE         ((CommandAuthorizationBlockMask)(1UL << 7))
#define COMMAND_AUTH_BLOCK_ENABLE_REQUEST       ((CommandAuthorizationBlockMask)(1UL << 8))
#define COMMAND_AUTH_BLOCK_COMMAND_STALE        ((CommandAuthorizationBlockMask)(1UL << 9))
#define COMMAND_AUTH_BLOCK_COMMAND_RANGE        ((CommandAuthorizationBlockMask)(1UL << 10))
#define COMMAND_AUTH_BLOCK_ACTIVE_FAULT         ((CommandAuthorizationBlockMask)(1UL << 11))

typedef struct
{
    uint32_t request_sequence;
    uint32_t input_sequence;
    uint32_t generated_at_ms;
    int16_t forward_q15;
    int16_t turn_q15;
    uint16_t maximum_speed_q15;
    bool enable_request;
} AppRequestedDriveCommand;

typedef struct
{
    AppRequestedDriveCommand request;
    AppSafetyState safety_state;
    AppFaultMask active_faults;
    AppMotorLinkStatus link_status;
    uint32_t now_ms;
    bool configuration_valid;
    bool input_valid;
    bool mandatory_tasks_healthy;
} CommandAuthorizationInput;

typedef struct
{
    bool logical_authorized;
    CommandAuthorizationBlockMask blocking_reasons;
    AppAuthorizedDriveCommand physical_command;
} CommandAuthorizationResult;

CommandAuthorizationResult CommandAuthorization_Evaluate(
    const CommandAuthorizationInput *input);

#endif /* COMMAND_AUTHORIZATION_H */
