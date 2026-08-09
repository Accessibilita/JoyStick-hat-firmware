#include "motor_protocol.h"

#include <limits.h>
#include <stddef.h>

#define FRAME_OFFSET_MAGIC_0                      (0U)
#define FRAME_OFFSET_MAGIC_1                      (1U)
#define FRAME_OFFSET_VERSION                      (2U)
#define FRAME_OFFSET_MESSAGE_TYPE                 (3U)
#define FRAME_OFFSET_DESTINATION                  (4U)
#define FRAME_OFFSET_SOURCE                       (5U)
#define FRAME_OFFSET_FLAGS                        (6U)
#define FRAME_OFFSET_STATE_OR_STATUS              (7U)

#define COMMAND_OFFSET_SESSION                    (8U)
#define COMMAND_OFFSET_SEQUENCE                   (12U)
#define COMMAND_OFFSET_GENERATED_MS               (16U)
#define COMMAND_OFFSET_FORWARD_Q15                (20U)
#define COMMAND_OFFSET_TURN_Q15                   (22U)
#define COMMAND_OFFSET_MAX_SPEED_Q15              (24U)
#define COMMAND_OFFSET_FAULTS                     (26U)

#define STATUS_OFFSET_CONTROLLER_SESSION          (8U)
#define STATUS_OFFSET_COMMAND_SESSION_ECHO        (12U)
#define STATUS_OFFSET_ACK_SEQUENCE                (16U)
#define STATUS_OFFSET_UPTIME_MS                   (20U)
#define STATUS_OFFSET_REMOTE_FAULTS               (24U)
#define STATUS_OFFSET_RESERVED                    (28U)

static void MotorProtocol_WriteU16Le(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)(value & 0x00FFU);
    destination[1] = (uint8_t)((value >> 8U) & 0x00FFU);
}

static void MotorProtocol_WriteU32Le(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)(value & 0x000000FFUL);
    destination[1] = (uint8_t)((value >> 8U) & 0x000000FFUL);
    destination[2] = (uint8_t)((value >> 16U) & 0x000000FFUL);
    destination[3] = (uint8_t)((value >> 24U) & 0x000000FFUL);
}

static uint16_t MotorProtocol_ReadU16Le(const uint8_t *source)
{
    return (uint16_t)((uint16_t)source[0] |
                      (uint16_t)((uint16_t)source[1] << 8U));
}

static uint32_t MotorProtocol_ReadU32Le(const uint8_t *source)
{
    return (uint32_t)source[0] |
           ((uint32_t)source[1] << 8U) |
           ((uint32_t)source[2] << 16U) |
           ((uint32_t)source[3] << 24U);
}

static int16_t MotorProtocol_ReadI16Le(const uint8_t *source)
{
    const uint16_t raw = MotorProtocol_ReadU16Le(source);
    int32_t signed_value;

    if (raw <= (uint16_t)INT16_MAX)
    {
        signed_value = (int32_t)raw;
    }
    else
    {
        signed_value = (int32_t)raw - 65536L;
    }

    return (int16_t)signed_value;
}

bool MotorProtocol_AddressIsUnicast(uint8_t address)
{
    return (address >= MOTOR_PROTOCOL_ADDRESS_MIN_UNICAST) &&
           (address <= MOTOR_PROTOCOL_ADDRESS_MAX_UNICAST);
}

static bool MotorProtocol_AddressesValid(uint8_t destination, uint8_t source)
{
    return MotorProtocol_AddressIsUnicast(destination) &&
           MotorProtocol_AddressIsUnicast(source) &&
           (destination != source);
}

static bool MotorProtocol_CommandFieldsValid(
    const MotorProtocolCommand *command)
{
    if (command == NULL)
    {
        return false;
    }

    if (!MotorProtocol_AddressesValid(
            command->destination_address,
            command->source_address))
    {
        return false;
    }

    if (command->command_session_id == 0U)
    {
        return false;
    }

    if (command->maximum_speed_q15 > MOTOR_PROTOCOL_Q15_MAX)
    {
        return false;
    }

    if ((uint32_t)command->safety_state >
        (uint32_t)APP_SAFETY_LATCHED_FAULT)
    {
        return false;
    }

    if ((command->command_flags &
         (uint8_t)(~(uint8_t)MOTOR_PROTOCOL_COMMAND_FLAG_MASK)) != 0U)
    {
        return false;
    }

    return true;
}

static bool MotorProtocol_StatusFieldsValid(
    const MotorProtocolStatus *status)
{
    if (status == NULL)
    {
        return false;
    }

    if (!MotorProtocol_AddressesValid(
            status->destination_address,
            status->source_address))
    {
        return false;
    }

    if ((status->controller_session_id == 0U) ||
        (status->command_session_echo == 0U))
    {
        return false;
    }

    if ((status->status_flags &
         (uint8_t)(~(uint8_t)MOTOR_PROTOCOL_STATUS_FLAG_MASK)) != 0U)
    {
        return false;
    }

    if ((uint32_t)status->status >
        (uint32_t)MOTOR_PROTOCOL_STATUS_UNSUPPORTED_VERSION)
    {
        return false;
    }

    return true;
}

uint16_t MotorProtocol_Crc16CcittFalse(
    const uint8_t *data,
    size_t length)
{
    uint16_t crc = 0xFFFFU;
    size_t byte_index;

    if ((data == NULL) && (length > 0U))
    {
        return 0U;
    }

    for (byte_index = 0U; byte_index < length; byte_index++)
    {
        uint8_t bit_index;

        crc ^= (uint16_t)((uint16_t)data[byte_index] << 8U);

        for (bit_index = 0U; bit_index < 8U; bit_index++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((uint16_t)(crc << 1U) ^ 0x1021U);
            }
            else
            {
                crc = (uint16_t)(crc << 1U);
            }
        }
    }

    return crc;
}

bool MotorProtocol_EncodeCommand(
    const MotorProtocolCommand *command,
    uint8_t *frame,
    size_t frame_length)
{
    uint16_t crc;
    size_t index;

    if ((frame == NULL) ||
        (frame_length != MOTOR_PROTOCOL_FRAME_SIZE) ||
        !MotorProtocol_CommandFieldsValid(command))
    {
        return false;
    }

    for (index = 0U; index < frame_length; index++)
    {
        frame[index] = 0U;
    }

    frame[FRAME_OFFSET_MAGIC_0] = MOTOR_PROTOCOL_COMMAND_MAGIC_0;
    frame[FRAME_OFFSET_MAGIC_1] = MOTOR_PROTOCOL_COMMAND_MAGIC_1;
    frame[FRAME_OFFSET_VERSION] = MOTOR_PROTOCOL_VERSION;
    frame[FRAME_OFFSET_MESSAGE_TYPE] = MOTOR_PROTOCOL_MESSAGE_DRIVE_COMMAND;
    frame[FRAME_OFFSET_DESTINATION] = command->destination_address;
    frame[FRAME_OFFSET_SOURCE] = command->source_address;
    frame[FRAME_OFFSET_FLAGS] = command->command_flags;
    frame[FRAME_OFFSET_STATE_OR_STATUS] = (uint8_t)command->safety_state;
    MotorProtocol_WriteU32Le(&frame[COMMAND_OFFSET_SESSION],
                             command->command_session_id);
    MotorProtocol_WriteU32Le(&frame[COMMAND_OFFSET_SEQUENCE], command->sequence);
    MotorProtocol_WriteU32Le(&frame[COMMAND_OFFSET_GENERATED_MS],
                             command->generated_at_ms);
    MotorProtocol_WriteU16Le(&frame[COMMAND_OFFSET_FORWARD_Q15],
                             (uint16_t)command->forward_q15);
    MotorProtocol_WriteU16Le(&frame[COMMAND_OFFSET_TURN_Q15],
                             (uint16_t)command->turn_q15);
    MotorProtocol_WriteU16Le(&frame[COMMAND_OFFSET_MAX_SPEED_Q15],
                             command->maximum_speed_q15);
    MotorProtocol_WriteU32Le(&frame[COMMAND_OFFSET_FAULTS],
                             command->active_faults);

    crc = MotorProtocol_Crc16CcittFalse(
        frame,
        MOTOR_PROTOCOL_CRC_DATA_SIZE);
    MotorProtocol_WriteU16Le(&frame[MOTOR_PROTOCOL_CRC_OFFSET], crc);

    return true;
}

MotorProtocolDecodeResult MotorProtocol_DecodeCommand(
    const uint8_t *frame,
    size_t frame_length,
    MotorProtocolCommand *command)
{
    uint16_t expected_crc;
    uint16_t actual_crc;

    if ((frame == NULL) || (command == NULL))
    {
        return MOTOR_PROTOCOL_DECODE_NULL_ARGUMENT;
    }

    if (frame_length != MOTOR_PROTOCOL_FRAME_SIZE)
    {
        return MOTOR_PROTOCOL_DECODE_WRONG_LENGTH;
    }

    if ((frame[FRAME_OFFSET_MAGIC_0] != MOTOR_PROTOCOL_COMMAND_MAGIC_0) ||
        (frame[FRAME_OFFSET_MAGIC_1] != MOTOR_PROTOCOL_COMMAND_MAGIC_1))
    {
        return MOTOR_PROTOCOL_DECODE_BAD_MAGIC;
    }

    if (frame[FRAME_OFFSET_VERSION] != MOTOR_PROTOCOL_VERSION)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_VERSION;
    }

    if (frame[FRAME_OFFSET_MESSAGE_TYPE] !=
        MOTOR_PROTOCOL_MESSAGE_DRIVE_COMMAND)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_MESSAGE_TYPE;
    }

    expected_crc = MotorProtocol_ReadU16Le(&frame[MOTOR_PROTOCOL_CRC_OFFSET]);
    actual_crc = MotorProtocol_Crc16CcittFalse(
        frame,
        MOTOR_PROTOCOL_CRC_DATA_SIZE);
    if (expected_crc != actual_crc)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_CRC;
    }

    if (!MotorProtocol_AddressesValid(
            frame[FRAME_OFFSET_DESTINATION],
            frame[FRAME_OFFSET_SOURCE]))
    {
        return MOTOR_PROTOCOL_DECODE_BAD_ADDRESS;
    }

    if ((frame[FRAME_OFFSET_FLAGS] &
         (uint8_t)(~(uint8_t)MOTOR_PROTOCOL_COMMAND_FLAG_MASK)) != 0U)
    {
        return MOTOR_PROTOCOL_DECODE_UNKNOWN_FLAGS;
    }

    command->destination_address = frame[FRAME_OFFSET_DESTINATION];
    command->source_address = frame[FRAME_OFFSET_SOURCE];
    command->command_flags = frame[FRAME_OFFSET_FLAGS];
    command->safety_state = (AppSafetyState)frame[FRAME_OFFSET_STATE_OR_STATUS];
    command->command_session_id =
        MotorProtocol_ReadU32Le(&frame[COMMAND_OFFSET_SESSION]);
    command->sequence = MotorProtocol_ReadU32Le(&frame[COMMAND_OFFSET_SEQUENCE]);
    command->generated_at_ms =
        MotorProtocol_ReadU32Le(&frame[COMMAND_OFFSET_GENERATED_MS]);
    command->forward_q15 =
        MotorProtocol_ReadI16Le(&frame[COMMAND_OFFSET_FORWARD_Q15]);
    command->turn_q15 = MotorProtocol_ReadI16Le(&frame[COMMAND_OFFSET_TURN_Q15]);
    command->maximum_speed_q15 =
        MotorProtocol_ReadU16Le(&frame[COMMAND_OFFSET_MAX_SPEED_Q15]);
    command->active_faults =
        MotorProtocol_ReadU32Le(&frame[COMMAND_OFFSET_FAULTS]);

    if (!MotorProtocol_CommandFieldsValid(command))
    {
        return MOTOR_PROTOCOL_DECODE_RANGE_ERROR;
    }

    return MOTOR_PROTOCOL_DECODE_OK;
}

bool MotorProtocol_EncodeStatus(
    const MotorProtocolStatus *status,
    uint8_t *frame,
    size_t frame_length)
{
    uint16_t crc;
    size_t index;

    if ((frame == NULL) ||
        (frame_length != MOTOR_PROTOCOL_FRAME_SIZE) ||
        !MotorProtocol_StatusFieldsValid(status))
    {
        return false;
    }

    for (index = 0U; index < frame_length; index++)
    {
        frame[index] = 0U;
    }

    frame[FRAME_OFFSET_MAGIC_0] = MOTOR_PROTOCOL_STATUS_MAGIC_0;
    frame[FRAME_OFFSET_MAGIC_1] = MOTOR_PROTOCOL_STATUS_MAGIC_1;
    frame[FRAME_OFFSET_VERSION] = MOTOR_PROTOCOL_VERSION;
    frame[FRAME_OFFSET_MESSAGE_TYPE] = MOTOR_PROTOCOL_MESSAGE_DRIVE_STATUS;
    frame[FRAME_OFFSET_DESTINATION] = status->destination_address;
    frame[FRAME_OFFSET_SOURCE] = status->source_address;
    frame[FRAME_OFFSET_FLAGS] = status->status_flags;
    frame[FRAME_OFFSET_STATE_OR_STATUS] = (uint8_t)status->status;
    MotorProtocol_WriteU32Le(&frame[STATUS_OFFSET_CONTROLLER_SESSION],
                             status->controller_session_id);
    MotorProtocol_WriteU32Le(&frame[STATUS_OFFSET_COMMAND_SESSION_ECHO],
                             status->command_session_echo);
    MotorProtocol_WriteU32Le(&frame[STATUS_OFFSET_ACK_SEQUENCE],
                             status->acknowledged_sequence);
    MotorProtocol_WriteU32Le(&frame[STATUS_OFFSET_UPTIME_MS],
                             status->controller_uptime_ms);
    MotorProtocol_WriteU32Le(&frame[STATUS_OFFSET_REMOTE_FAULTS],
                             status->remote_faults);

    crc = MotorProtocol_Crc16CcittFalse(
        frame,
        MOTOR_PROTOCOL_CRC_DATA_SIZE);
    MotorProtocol_WriteU16Le(&frame[MOTOR_PROTOCOL_CRC_OFFSET], crc);

    return true;
}

MotorProtocolDecodeResult MotorProtocol_DecodeStatus(
    const uint8_t *frame,
    size_t frame_length,
    MotorProtocolStatus *status)
{
    uint16_t expected_crc;
    uint16_t actual_crc;

    if ((frame == NULL) || (status == NULL))
    {
        return MOTOR_PROTOCOL_DECODE_NULL_ARGUMENT;
    }

    if (frame_length != MOTOR_PROTOCOL_FRAME_SIZE)
    {
        return MOTOR_PROTOCOL_DECODE_WRONG_LENGTH;
    }

    if ((frame[FRAME_OFFSET_MAGIC_0] != MOTOR_PROTOCOL_STATUS_MAGIC_0) ||
        (frame[FRAME_OFFSET_MAGIC_1] != MOTOR_PROTOCOL_STATUS_MAGIC_1))
    {
        return MOTOR_PROTOCOL_DECODE_BAD_MAGIC;
    }

    if (frame[FRAME_OFFSET_VERSION] != MOTOR_PROTOCOL_VERSION)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_VERSION;
    }

    if (frame[FRAME_OFFSET_MESSAGE_TYPE] != MOTOR_PROTOCOL_MESSAGE_DRIVE_STATUS)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_MESSAGE_TYPE;
    }

    expected_crc = MotorProtocol_ReadU16Le(&frame[MOTOR_PROTOCOL_CRC_OFFSET]);
    actual_crc = MotorProtocol_Crc16CcittFalse(
        frame,
        MOTOR_PROTOCOL_CRC_DATA_SIZE);
    if (expected_crc != actual_crc)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_CRC;
    }

    if (!MotorProtocol_AddressesValid(
            frame[FRAME_OFFSET_DESTINATION],
            frame[FRAME_OFFSET_SOURCE]))
    {
        return MOTOR_PROTOCOL_DECODE_BAD_ADDRESS;
    }

    if ((frame[FRAME_OFFSET_FLAGS] &
         (uint8_t)(~(uint8_t)MOTOR_PROTOCOL_STATUS_FLAG_MASK)) != 0U)
    {
        return MOTOR_PROTOCOL_DECODE_UNKNOWN_FLAGS;
    }

    if ((uint32_t)frame[FRAME_OFFSET_STATE_OR_STATUS] >
        (uint32_t)MOTOR_PROTOCOL_STATUS_UNSUPPORTED_VERSION)
    {
        return MOTOR_PROTOCOL_DECODE_BAD_STATUS;
    }

    if ((frame[STATUS_OFFSET_RESERVED] != 0U) ||
        (frame[STATUS_OFFSET_RESERVED + 1U] != 0U))
    {
        return MOTOR_PROTOCOL_DECODE_RESERVED_NONZERO;
    }

    status->destination_address = frame[FRAME_OFFSET_DESTINATION];
    status->source_address = frame[FRAME_OFFSET_SOURCE];
    status->status_flags = frame[FRAME_OFFSET_FLAGS];
    status->status =
        (MotorProtocolRemoteStatus)frame[FRAME_OFFSET_STATE_OR_STATUS];
    status->controller_session_id =
        MotorProtocol_ReadU32Le(&frame[STATUS_OFFSET_CONTROLLER_SESSION]);
    status->command_session_echo =
        MotorProtocol_ReadU32Le(&frame[STATUS_OFFSET_COMMAND_SESSION_ECHO]);
    status->acknowledged_sequence =
        MotorProtocol_ReadU32Le(&frame[STATUS_OFFSET_ACK_SEQUENCE]);
    status->controller_uptime_ms =
        MotorProtocol_ReadU32Le(&frame[STATUS_OFFSET_UPTIME_MS]);
    status->remote_faults =
        MotorProtocol_ReadU32Le(&frame[STATUS_OFFSET_REMOTE_FAULTS]);

    if (!MotorProtocol_StatusFieldsValid(status))
    {
        return MOTOR_PROTOCOL_DECODE_RANGE_ERROR;
    }

    return MOTOR_PROTOCOL_DECODE_OK;
}
