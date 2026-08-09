#ifndef MOTOR_PROTOCOL_H
#define MOTOR_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_types.h"

#define MOTOR_PROTOCOL_FRAME_SIZE                   (32U)
#define MOTOR_PROTOCOL_CRC_DATA_SIZE                (30U)
#define MOTOR_PROTOCOL_CRC_OFFSET                   (30U)

#define MOTOR_PROTOCOL_VERSION                      (1U)

#define MOTOR_PROTOCOL_COMMAND_MAGIC_0              (0xA5U)
#define MOTOR_PROTOCOL_COMMAND_MAGIC_1              (0x5AU)
#define MOTOR_PROTOCOL_STATUS_MAGIC_0               (0x4DU)
#define MOTOR_PROTOCOL_STATUS_MAGIC_1               (0x43U)

#define MOTOR_PROTOCOL_MESSAGE_DRIVE_COMMAND        (0x10U)
#define MOTOR_PROTOCOL_MESSAGE_DRIVE_STATUS         (0x20U)

#define MOTOR_PROTOCOL_ADDRESS_UNASSIGNED           (0x00U)
#define MOTOR_PROTOCOL_ADDRESS_BROADCAST            (0xFFU)
#define MOTOR_PROTOCOL_ADDRESS_MIN_UNICAST          (0x01U)
#define MOTOR_PROTOCOL_ADDRESS_MAX_UNICAST          (0xFEU)

#define MOTOR_PROTOCOL_COMMAND_FLAG_LOGICAL_AUTH    (1U << 0)
#define MOTOR_PROTOCOL_COMMAND_FLAG_ENABLE_REQUEST  (1U << 1)
#define MOTOR_PROTOCOL_COMMAND_FLAG_MASK            \
    (MOTOR_PROTOCOL_COMMAND_FLAG_LOGICAL_AUTH |     \
     MOTOR_PROTOCOL_COMMAND_FLAG_ENABLE_REQUEST)

#define MOTOR_PROTOCOL_STATUS_FLAG_LINK_READY       (1U << 0)
#define MOTOR_PROTOCOL_STATUS_FLAG_DRIVE_READY      (1U << 1)
#define MOTOR_PROTOCOL_STATUS_FLAG_BRAKES_CONFIRMED (1U << 2)
#define MOTOR_PROTOCOL_STATUS_FLAG_COMMAND_ACCEPTED (1U << 3)
#define MOTOR_PROTOCOL_STATUS_FLAG_MASK             \
    (MOTOR_PROTOCOL_STATUS_FLAG_LINK_READY |        \
     MOTOR_PROTOCOL_STATUS_FLAG_DRIVE_READY |       \
     MOTOR_PROTOCOL_STATUS_FLAG_BRAKES_CONFIRMED |  \
     MOTOR_PROTOCOL_STATUS_FLAG_COMMAND_ACCEPTED)

#define MOTOR_PROTOCOL_Q15_MAX                      (32767U)

typedef enum
{
    MOTOR_PROTOCOL_STATUS_OK = 0,
    MOTOR_PROTOCOL_STATUS_INHIBITED,
    MOTOR_PROTOCOL_STATUS_REMOTE_FAULT,
    MOTOR_PROTOCOL_STATUS_BAD_SESSION,
    MOTOR_PROTOCOL_STATUS_BAD_SEQUENCE,
    MOTOR_PROTOCOL_STATUS_BAD_COMMAND,
    MOTOR_PROTOCOL_STATUS_UNSUPPORTED_VERSION
} MotorProtocolRemoteStatus;

typedef enum
{
    MOTOR_PROTOCOL_DECODE_OK = 0,
    MOTOR_PROTOCOL_DECODE_NULL_ARGUMENT,
    MOTOR_PROTOCOL_DECODE_WRONG_LENGTH,
    MOTOR_PROTOCOL_DECODE_BAD_MAGIC,
    MOTOR_PROTOCOL_DECODE_BAD_VERSION,
    MOTOR_PROTOCOL_DECODE_BAD_MESSAGE_TYPE,
    MOTOR_PROTOCOL_DECODE_BAD_CRC,
    MOTOR_PROTOCOL_DECODE_RESERVED_NONZERO,
    MOTOR_PROTOCOL_DECODE_UNKNOWN_FLAGS,
    MOTOR_PROTOCOL_DECODE_BAD_ADDRESS,
    MOTOR_PROTOCOL_DECODE_RANGE_ERROR,
    MOTOR_PROTOCOL_DECODE_BAD_STATUS
} MotorProtocolDecodeResult;

typedef struct
{
    uint8_t destination_address;
    uint8_t source_address;
    uint8_t command_flags;
    AppSafetyState safety_state;
    uint32_t command_session_id;
    uint32_t sequence;
    uint32_t generated_at_ms;
    int16_t forward_q15;
    int16_t turn_q15;
    uint16_t maximum_speed_q15;
    AppFaultMask active_faults;
} MotorProtocolCommand;

typedef struct
{
    uint8_t destination_address;
    uint8_t source_address;
    uint8_t status_flags;
    MotorProtocolRemoteStatus status;
    uint32_t controller_session_id;
    uint32_t command_session_echo;
    uint32_t acknowledged_sequence;
    uint32_t controller_uptime_ms;
    AppFaultMask remote_faults;
} MotorProtocolStatus;

bool MotorProtocol_AddressIsUnicast(uint8_t address);

uint16_t MotorProtocol_Crc16CcittFalse(
    const uint8_t *data,
    size_t length);

bool MotorProtocol_EncodeCommand(
    const MotorProtocolCommand *command,
    uint8_t *frame,
    size_t frame_length);

MotorProtocolDecodeResult MotorProtocol_DecodeCommand(
    const uint8_t *frame,
    size_t frame_length,
    MotorProtocolCommand *command);

bool MotorProtocol_EncodeStatus(
    const MotorProtocolStatus *status,
    uint8_t *frame,
    size_t frame_length);

MotorProtocolDecodeResult MotorProtocol_DecodeStatus(
    const uint8_t *frame,
    size_t frame_length,
    MotorProtocolStatus *status);

#endif /* MOTOR_PROTOCOL_H */
