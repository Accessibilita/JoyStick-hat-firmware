#include "fake_motor_controller.h"

#include <stddef.h>

void FakeMotorController_Init(
    FakeMotorController *controller,
    uint8_t node_address,
    uint8_t expected_host_address,
    uint32_t controller_session_id)
{
    if (controller != NULL)
    {
        controller->node_address = node_address;
        controller->expected_host_address = expected_host_address;
        controller->controller_session_id = controller_session_id;
        controller->uptime_ms = 0U;
        controller->injected_remote_faults = APP_FAULT_NONE;
        controller->mode = FAKE_MOTOR_MODE_NORMAL;
    }
}

void FakeMotorController_SetMode(
    FakeMotorController *controller,
    FakeMotorControllerMode mode)
{
    if (controller != NULL)
    {
        controller->mode = mode;
    }
}

bool FakeMotorController_Exchange(
    FakeMotorController *controller,
    const uint8_t *command_frame,
    size_t command_frame_length,
    uint8_t *status_frame,
    size_t status_frame_length)
{
    MotorProtocolCommand command;
    MotorProtocolStatus status;
    MotorProtocolDecodeResult decode_result;
    bool logically_authorized;

    if ((controller == NULL) ||
        (command_frame == NULL) ||
        (status_frame == NULL))
    {
        return false;
    }

    if (controller->mode == FAKE_MOTOR_MODE_DROP_RESPONSE)
    {
        return false;
    }

    decode_result = MotorProtocol_DecodeCommand(
        command_frame,
        command_frame_length,
        &command);
    if (decode_result != MOTOR_PROTOCOL_DECODE_OK)
    {
        return false;
    }

    if ((command.destination_address != controller->node_address) ||
        (command.source_address != controller->expected_host_address))
    {
        return false;
    }

    if (controller->mode == FAKE_MOTOR_MODE_RESET_CONTROLLER)
    {
        controller->controller_session_id++;
        if (controller->controller_session_id == 0U)
        {
            controller->controller_session_id = 1U;
        }
        controller->uptime_ms = 0U;
    }

    controller->uptime_ms += 10U;

    status.destination_address = command.source_address;
    status.source_address = controller->node_address;
    status.status_flags = MOTOR_PROTOCOL_STATUS_FLAG_LINK_READY |
                          MOTOR_PROTOCOL_STATUS_FLAG_DRIVE_READY |
                          MOTOR_PROTOCOL_STATUS_FLAG_BRAKES_CONFIRMED;
    status.status = MOTOR_PROTOCOL_STATUS_OK;
    status.controller_session_id = controller->controller_session_id;
    status.command_session_echo = command.command_session_id;
    status.acknowledged_sequence = command.sequence;
    status.controller_uptime_ms = controller->uptime_ms;
    status.remote_faults = controller->injected_remote_faults;

    logically_authorized =
        (command.command_flags & MOTOR_PROTOCOL_COMMAND_FLAG_LOGICAL_AUTH) != 0U;
    if (logically_authorized)
    {
        status.status_flags |= MOTOR_PROTOCOL_STATUS_FLAG_COMMAND_ACCEPTED;
    }

    if (controller->mode == FAKE_MOTOR_MODE_WRONG_SESSION_ECHO)
    {
        status.command_session_echo ^= 0x00000001UL;
        if (status.command_session_echo == 0U)
        {
            status.command_session_echo = 1U;
        }
    }
    else if (controller->mode == FAKE_MOTOR_MODE_STALE_ACK)
    {
        status.acknowledged_sequence--;
    }
    else if (controller->mode == FAKE_MOTOR_MODE_REMOTE_FAULT)
    {
        status.remote_faults = APP_FAULT_REMOTE_CONTROLLER;
        status.status = MOTOR_PROTOCOL_STATUS_REMOTE_FAULT;
        status.status_flags &=
            (uint8_t)(~(uint8_t)MOTOR_PROTOCOL_STATUS_FLAG_COMMAND_ACCEPTED);
    }
    else if (controller->mode == FAKE_MOTOR_MODE_NOT_READY)
    {
        status.status_flags = MOTOR_PROTOCOL_STATUS_FLAG_LINK_READY;
        status.status = MOTOR_PROTOCOL_STATUS_INHIBITED;
    }
    else
    {
        /* Normal response. */
    }

    if (!MotorProtocol_EncodeStatus(
            &status,
            status_frame,
            status_frame_length))
    {
        return false;
    }

    if (controller->mode == FAKE_MOTOR_MODE_CORRUPT_CRC)
    {
        status_frame[MOTOR_PROTOCOL_CRC_OFFSET] ^= 0x01U;
    }

    return true;
}
