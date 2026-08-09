/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef MOTOR_LINK_STATE_H
#define MOTOR_LINK_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"
#include "motor_protocol.h"

typedef enum
{
    MOTOR_LINK_OFFLINE = 0,
    MOTOR_LINK_SYNCHRONIZING,
    MOTOR_LINK_READY,
    MOTOR_LINK_ACTIVE,
    MOTOR_LINK_FAULT
} MotorLinkState;

typedef enum
{
    MOTOR_LINK_PROCESS_ACCEPTED = 0,
    MOTOR_LINK_PROCESS_SYNC_STARTED,
    MOTOR_LINK_PROCESS_CONTROLLER_RESTARTED,
    MOTOR_LINK_PROCESS_NULL_ARGUMENT,
    MOTOR_LINK_PROCESS_ADDRESS_MISMATCH,
    MOTOR_LINK_PROCESS_BAD_CONTROLLER_SESSION,
    MOTOR_LINK_PROCESS_SESSION_MISMATCH,
    MOTOR_LINK_PROCESS_ACK_MISMATCH,
    MOTOR_LINK_PROCESS_REMOTE_FAULT,
    MOTOR_LINK_PROCESS_STATUS_REJECTED,
    MOTOR_LINK_PROCESS_REMOTE_NOT_READY
} MotorLinkProcessResult;

typedef uint32_t MotorLinkFaultMask;

#define MOTOR_LINK_FAULT_NONE                    ((MotorLinkFaultMask)0U)
#define MOTOR_LINK_FAULT_BAD_FRAME               ((MotorLinkFaultMask)(1UL << 0))
#define MOTOR_LINK_FAULT_ADDRESS_MISMATCH        ((MotorLinkFaultMask)(1UL << 1))
#define MOTOR_LINK_FAULT_SESSION_MISMATCH        ((MotorLinkFaultMask)(1UL << 2))
#define MOTOR_LINK_FAULT_CONTROLLER_RESTART      ((MotorLinkFaultMask)(1UL << 3))
#define MOTOR_LINK_FAULT_ACK_MISMATCH            ((MotorLinkFaultMask)(1UL << 4))
#define MOTOR_LINK_FAULT_REMOTE                  ((MotorLinkFaultMask)(1UL << 5))
#define MOTOR_LINK_FAULT_REMOTE_NOT_READY        ((MotorLinkFaultMask)(1UL << 6))
#define MOTOR_LINK_FAULT_TIMEOUT                 ((MotorLinkFaultMask)(1UL << 7))
#define MOTOR_LINK_FAULT_STATUS_REJECTED         ((MotorLinkFaultMask)(1UL << 8))
#define MOTOR_LINK_FAULT_BAD_CONTROLLER_SESSION  ((MotorLinkFaultMask)(1UL << 9))

typedef struct
{
    MotorLinkState state;
    MotorLinkFaultMask fault_history;
    uint8_t local_node_address;
    uint8_t remote_node_address;
    uint32_t local_command_session_id;
    uint32_t controller_session_id;
    uint32_t receive_sequence;
    uint32_t last_valid_packet_ms;
    uint32_t last_acknowledged_sequence;
    uint32_t consecutive_valid_frames;
    uint32_t consecutive_invalid_frames;
    AppFaultMask remote_faults;
    uint8_t last_status_flags;
    MotorProtocolRemoteStatus last_remote_status;
    bool controller_session_known;
} MotorLinkContext;

bool MotorLinkState_Init(
    MotorLinkContext *context,
    uint8_t local_node_address,
    uint8_t remote_node_address,
    uint32_t local_command_session_id);

void MotorLinkState_RecordDecodeFailure(
    MotorLinkContext *context,
    MotorProtocolDecodeResult decode_result);

MotorLinkProcessResult MotorLinkState_ProcessStatus(
    MotorLinkContext *context,
    const MotorProtocolStatus *status,
    uint32_t expected_command_sequence,
    bool logical_authorization_active,
    uint32_t now_ms);

void MotorLinkState_UpdateTimeout(
    MotorLinkContext *context,
    uint32_t now_ms,
    uint32_t timeout_ms);

bool MotorLinkState_IsLinkValid(const MotorLinkContext *context);

void MotorLinkState_CopyAppStatus(
    const MotorLinkContext *context,
    AppMotorLinkStatus *status);

#endif /* MOTOR_LINK_STATE_H */
