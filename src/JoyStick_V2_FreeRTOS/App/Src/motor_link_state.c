#include "motor_link_state.h"

#include <stddef.h>

#define MOTOR_LINK_REQUIRED_VALID_FRAMES         (2U)

static bool MotorLinkStatus_HasFlag(uint8_t flags, uint8_t flag)
{
    return (flags & flag) != 0U;
}

static void MotorLinkState_EnterSynchronizing(MotorLinkContext *context)
{
    context->state = MOTOR_LINK_SYNCHRONIZING;
    context->consecutive_valid_frames = 0U;
}

static void MotorLinkState_RecordAcceptedStatus(
    MotorLinkContext *context,
    const MotorProtocolStatus *status,
    uint32_t now_ms)
{
    context->last_valid_packet_ms = now_ms;
    context->last_acknowledged_sequence = status->acknowledged_sequence;
    context->receive_sequence++;
    context->consecutive_valid_frames++;
    context->consecutive_invalid_frames = 0U;
    context->remote_faults = status->remote_faults;
    context->last_status_flags = status->status_flags;
    context->last_remote_status = status->status;
}

bool MotorLinkState_Init(
    MotorLinkContext *context,
    uint8_t local_node_address,
    uint8_t remote_node_address,
    uint32_t local_command_session_id)
{
    bool addresses_valid;
    bool session_valid;

    if (context == NULL)
    {
        return false;
    }

    context->state = MOTOR_LINK_OFFLINE;
    context->fault_history = MOTOR_LINK_FAULT_NONE;
    context->local_node_address = local_node_address;
    context->remote_node_address = remote_node_address;
    context->local_command_session_id = local_command_session_id;
    context->controller_session_id = 0U;
    context->receive_sequence = 0U;
    context->last_valid_packet_ms = 0U;
    context->last_acknowledged_sequence = 0U;
    context->consecutive_valid_frames = 0U;
    context->consecutive_invalid_frames = 0U;
    context->remote_faults = APP_FAULT_NONE;
    context->last_status_flags = 0U;
    context->last_remote_status = MOTOR_PROTOCOL_STATUS_INHIBITED;
    context->controller_session_known = false;

    addresses_valid =
        MotorProtocol_AddressIsUnicast(local_node_address) &&
        MotorProtocol_AddressIsUnicast(remote_node_address) &&
        (local_node_address != remote_node_address);
    session_valid = local_command_session_id != 0U;

    if (!addresses_valid)
    {
        context->fault_history |= MOTOR_LINK_FAULT_ADDRESS_MISMATCH;
    }
    if (!session_valid)
    {
        context->fault_history |= MOTOR_LINK_FAULT_SESSION_MISMATCH;
    }

    if (!(addresses_valid && session_valid))
    {
        context->state = MOTOR_LINK_FAULT;
        return false;
    }

    return true;
}

void MotorLinkState_RecordDecodeFailure(
    MotorLinkContext *context,
    MotorProtocolDecodeResult decode_result)
{
    if (context == NULL)
    {
        return;
    }

    if (decode_result != MOTOR_PROTOCOL_DECODE_OK)
    {
        context->fault_history |= MOTOR_LINK_FAULT_BAD_FRAME;
        context->consecutive_invalid_frames++;
        MotorLinkState_EnterSynchronizing(context);
    }
}

MotorLinkProcessResult MotorLinkState_ProcessStatus(
    MotorLinkContext *context,
    const MotorProtocolStatus *status,
    uint32_t expected_command_sequence,
    bool logical_authorization_active,
    uint32_t now_ms)
{
    bool new_controller_session = false;
    bool controller_restarted = false;
    bool link_ready;
    bool drive_ready;
    bool brakes_confirmed;
    bool command_accepted;

    if ((context == NULL) || (status == NULL))
    {
        return MOTOR_LINK_PROCESS_NULL_ARGUMENT;
    }

    if ((status->destination_address != context->local_node_address) ||
        (status->source_address != context->remote_node_address))
    {
        context->fault_history |= MOTOR_LINK_FAULT_ADDRESS_MISMATCH;
        context->consecutive_invalid_frames++;
        MotorLinkState_EnterSynchronizing(context);
        return MOTOR_LINK_PROCESS_ADDRESS_MISMATCH;
    }

    if (status->controller_session_id == 0U)
    {
        context->fault_history |= MOTOR_LINK_FAULT_BAD_CONTROLLER_SESSION;
        context->consecutive_invalid_frames++;
        MotorLinkState_EnterSynchronizing(context);
        return MOTOR_LINK_PROCESS_BAD_CONTROLLER_SESSION;
    }

    if ((context->local_command_session_id == 0U) ||
        (status->command_session_echo != context->local_command_session_id))
    {
        context->fault_history |= MOTOR_LINK_FAULT_SESSION_MISMATCH;
        context->consecutive_invalid_frames++;
        MotorLinkState_EnterSynchronizing(context);
        return MOTOR_LINK_PROCESS_SESSION_MISMATCH;
    }

    /*
     * A status frame belongs to exactly one command transaction in V1. Check
     * the acknowledgment before learning or changing controller-session state,
     * otherwise the first packet after startup could qualify using stale data.
     */
    if (status->acknowledged_sequence != expected_command_sequence)
    {
        context->fault_history |= MOTOR_LINK_FAULT_ACK_MISMATCH;
        context->consecutive_invalid_frames++;
        MotorLinkState_EnterSynchronizing(context);
        return MOTOR_LINK_PROCESS_ACK_MISMATCH;
    }

    if (!context->controller_session_known)
    {
        context->controller_session_id = status->controller_session_id;
        context->controller_session_known = true;
        context->consecutive_valid_frames = 0U;
        new_controller_session = true;
    }
    else if (status->controller_session_id != context->controller_session_id)
    {
        context->fault_history |= MOTOR_LINK_FAULT_CONTROLLER_RESTART;
        context->controller_session_id = status->controller_session_id;
        context->consecutive_valid_frames = 0U;
        controller_restarted = true;
    }
    else
    {
        /* Existing controller session remains current. */
    }

    MotorLinkState_RecordAcceptedStatus(context, status, now_ms);

    if ((status->remote_faults != APP_FAULT_NONE) ||
        (status->status == MOTOR_PROTOCOL_STATUS_REMOTE_FAULT))
    {
        context->fault_history |= MOTOR_LINK_FAULT_REMOTE;
        context->state = MOTOR_LINK_FAULT;
        return MOTOR_LINK_PROCESS_REMOTE_FAULT;
    }

    if (status->status != MOTOR_PROTOCOL_STATUS_OK)
    {
        context->fault_history |= MOTOR_LINK_FAULT_STATUS_REJECTED;
        context->state = MOTOR_LINK_SYNCHRONIZING;
        return MOTOR_LINK_PROCESS_STATUS_REJECTED;
    }

    link_ready = MotorLinkStatus_HasFlag(
        status->status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_LINK_READY);
    drive_ready = MotorLinkStatus_HasFlag(
        status->status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_DRIVE_READY);
    brakes_confirmed = MotorLinkStatus_HasFlag(
        status->status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_BRAKES_CONFIRMED);
    command_accepted = MotorLinkStatus_HasFlag(
        status->status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_COMMAND_ACCEPTED);

    if (!link_ready)
    {
        context->fault_history |= MOTOR_LINK_FAULT_REMOTE_NOT_READY;
        context->state = MOTOR_LINK_SYNCHRONIZING;
        return MOTOR_LINK_PROCESS_REMOTE_NOT_READY;
    }

    if (context->consecutive_valid_frames < MOTOR_LINK_REQUIRED_VALID_FRAMES)
    {
        context->state = MOTOR_LINK_SYNCHRONIZING;
        if (controller_restarted)
        {
            return MOTOR_LINK_PROCESS_CONTROLLER_RESTARTED;
        }
        if (new_controller_session)
        {
            return MOTOR_LINK_PROCESS_SYNC_STARTED;
        }
        return MOTOR_LINK_PROCESS_ACCEPTED;
    }

    if (!(drive_ready && brakes_confirmed))
    {
        context->fault_history |= MOTOR_LINK_FAULT_REMOTE_NOT_READY;
        context->state = MOTOR_LINK_READY;
        return MOTOR_LINK_PROCESS_REMOTE_NOT_READY;
    }

    if (logical_authorization_active && command_accepted)
    {
        context->state = MOTOR_LINK_ACTIVE;
    }
    else
    {
        context->state = MOTOR_LINK_READY;
    }

    return MOTOR_LINK_PROCESS_ACCEPTED;
}

void MotorLinkState_UpdateTimeout(
    MotorLinkContext *context,
    uint32_t now_ms,
    uint32_t timeout_ms)
{
    uint32_t elapsed_ms;

    if ((context == NULL) || (context->last_valid_packet_ms == 0U))
    {
        return;
    }

    elapsed_ms = now_ms - context->last_valid_packet_ms;
    if (elapsed_ms > timeout_ms)
    {
        context->fault_history |= MOTOR_LINK_FAULT_TIMEOUT;
        context->state = MOTOR_LINK_OFFLINE;
        context->consecutive_valid_frames = 0U;
        context->last_status_flags = 0U;
    }
}

bool MotorLinkState_IsLinkValid(const MotorLinkContext *context)
{
    if (context == NULL)
    {
        return false;
    }

    return (context->state == MOTOR_LINK_READY) ||
           (context->state == MOTOR_LINK_ACTIVE);
}

void MotorLinkState_CopyAppStatus(
    const MotorLinkContext *context,
    AppMotorLinkStatus *status)
{
    if (status == NULL)
    {
        return;
    }

    if (context == NULL)
    {
        status->receive_sequence = 0U;
        status->last_valid_packet_ms = 0U;
        status->remote_faults = APP_FAULT_INTERNAL_INVARIANT;
        status->link_fault_history = MOTOR_LINK_FAULT_BAD_FRAME;
        status->controller_session_id = 0U;
        status->acknowledged_command_sequence = 0U;
        status->local_node_address = MOTOR_PROTOCOL_ADDRESS_UNASSIGNED;
        status->remote_node_address = MOTOR_PROTOCOL_ADDRESS_UNASSIGNED;
        status->protocol_state = (uint8_t)MOTOR_LINK_FAULT;
        status->link_valid = false;
        status->remote_drive_ready = false;
        status->brakes_confirmed = false;
        status->command_accepted = false;
        return;
    }

    status->receive_sequence = context->receive_sequence;
    status->last_valid_packet_ms = context->last_valid_packet_ms;
    status->remote_faults = context->remote_faults;
    status->link_fault_history = context->fault_history;
    status->controller_session_id = context->controller_session_id;
    status->acknowledged_command_sequence =
        context->last_acknowledged_sequence;
    status->local_node_address = context->local_node_address;
    status->remote_node_address = context->remote_node_address;
    status->protocol_state = (uint8_t)context->state;
    status->link_valid = MotorLinkState_IsLinkValid(context);
    status->remote_drive_ready = MotorLinkStatus_HasFlag(
        context->last_status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_DRIVE_READY);
    status->brakes_confirmed = MotorLinkStatus_HasFlag(
        context->last_status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_BRAKES_CONFIRMED);
    status->command_accepted = MotorLinkStatus_HasFlag(
        context->last_status_flags,
        MOTOR_PROTOCOL_STATUS_FLAG_COMMAND_ACCEPTED);
}
