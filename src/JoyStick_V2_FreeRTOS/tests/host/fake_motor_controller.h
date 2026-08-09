#ifndef FAKE_MOTOR_CONTROLLER_H
#define FAKE_MOTOR_CONTROLLER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "motor_protocol.h"

typedef enum
{
    FAKE_MOTOR_MODE_NORMAL = 0,
    FAKE_MOTOR_MODE_DROP_RESPONSE,
    FAKE_MOTOR_MODE_CORRUPT_CRC,
    FAKE_MOTOR_MODE_WRONG_SESSION_ECHO,
    FAKE_MOTOR_MODE_RESET_CONTROLLER,
    FAKE_MOTOR_MODE_STALE_ACK,
    FAKE_MOTOR_MODE_REMOTE_FAULT,
    FAKE_MOTOR_MODE_NOT_READY
} FakeMotorControllerMode;

typedef struct
{
    uint8_t node_address;
    uint8_t expected_host_address;
    uint32_t controller_session_id;
    uint32_t uptime_ms;
    AppFaultMask injected_remote_faults;
    FakeMotorControllerMode mode;
} FakeMotorController;

void FakeMotorController_Init(
    FakeMotorController *controller,
    uint8_t node_address,
    uint8_t expected_host_address,
    uint32_t controller_session_id);

void FakeMotorController_SetMode(
    FakeMotorController *controller,
    FakeMotorControllerMode mode);

bool FakeMotorController_Exchange(
    FakeMotorController *controller,
    const uint8_t *command_frame,
    size_t command_frame_length,
    uint8_t *status_frame,
    size_t status_frame_length);

#endif /* FAKE_MOTOR_CONTROLLER_H */
