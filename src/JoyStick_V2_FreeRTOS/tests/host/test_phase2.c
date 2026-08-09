/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_build_config.h"
#include "command_authorization.h"
#include "fake_motor_controller.h"
#include "motor_link_state.h"
#include "motor_protocol.h"

#define TEST_HOST_ADDRESS                  (0x01U)
#define TEST_MOTOR_ADDRESS                 (0x10U)

static MotorProtocolCommand MakeCommand(
    uint32_t session_id,
    uint32_t sequence,
    bool logically_authorized)
{
    MotorProtocolCommand command;

    command.destination_address = TEST_MOTOR_ADDRESS;
    command.source_address = TEST_HOST_ADDRESS;
    command.command_session_id = session_id;
    command.sequence = sequence;
    command.generated_at_ms = 100U;
    command.forward_q15 = 1200;
    command.turn_q15 = -500;
    command.maximum_speed_q15 = 16000U;
    command.safety_state = APP_SAFETY_READY;
    command.command_flags = MOTOR_PROTOCOL_COMMAND_FLAG_ENABLE_REQUEST;
    if (logically_authorized)
    {
        command.command_flags |= MOTOR_PROTOCOL_COMMAND_FLAG_LOGICAL_AUTH;
    }
    command.active_faults = APP_FAULT_NONE;

    return command;
}

static MotorProtocolStatus MakeStatus(
    uint32_t controller_session,
    uint32_t command_session,
    uint32_t acknowledged_sequence)
{
    MotorProtocolStatus status;

    status.destination_address = TEST_HOST_ADDRESS;
    status.source_address = TEST_MOTOR_ADDRESS;
    status.status_flags = MOTOR_PROTOCOL_STATUS_FLAG_LINK_READY |
                          MOTOR_PROTOCOL_STATUS_FLAG_DRIVE_READY |
                          MOTOR_PROTOCOL_STATUS_FLAG_BRAKES_CONFIRMED;
    status.status = MOTOR_PROTOCOL_STATUS_OK;
    status.controller_session_id = controller_session;
    status.command_session_echo = command_session;
    status.acknowledged_sequence = acknowledged_sequence;
    status.controller_uptime_ms = 500U;
    status.remote_faults = APP_FAULT_NONE;

    return status;
}

static void RecomputeFrameCrc(uint8_t *frame)
{
    const uint16_t crc = MotorProtocol_Crc16CcittFalse(
        frame,
        MOTOR_PROTOCOL_CRC_DATA_SIZE);

    frame[MOTOR_PROTOCOL_CRC_OFFSET] = (uint8_t)(crc & 0x00FFU);
    frame[MOTOR_PROTOCOL_CRC_OFFSET + 1U] =
        (uint8_t)((crc >> 8U) & 0x00FFU);
}

static void Test_CrcGoldenVector(void)
{
    static const uint8_t input[] =
    {
        (uint8_t)'1', (uint8_t)'2', (uint8_t)'3',
        (uint8_t)'4', (uint8_t)'5', (uint8_t)'6',
        (uint8_t)'7', (uint8_t)'8', (uint8_t)'9'
    };

    assert(MotorProtocol_Crc16CcittFalse(input, sizeof(input)) == 0x29B1U);
}

static void Test_CommandRoundTrip(void)
{
    const MotorProtocolCommand source = MakeCommand(0x12345678UL, 42U, true);
    MotorProtocolCommand decoded;
    uint8_t frame[MOTOR_PROTOCOL_FRAME_SIZE];

    assert(MotorProtocol_EncodeCommand(&source, frame, sizeof(frame)));
    assert(MotorProtocol_DecodeCommand(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_OK);

    assert(decoded.destination_address == source.destination_address);
    assert(decoded.source_address == source.source_address);
    assert(decoded.command_session_id == source.command_session_id);
    assert(decoded.sequence == source.sequence);
    assert(decoded.generated_at_ms == source.generated_at_ms);
    assert(decoded.forward_q15 == source.forward_q15);
    assert(decoded.turn_q15 == source.turn_q15);
    assert(decoded.maximum_speed_q15 == source.maximum_speed_q15);
    assert(decoded.safety_state == source.safety_state);
    assert(decoded.command_flags == source.command_flags);
    assert(decoded.active_faults == source.active_faults);
}

static void Test_StatusRoundTrip(void)
{
    const MotorProtocolStatus source =
        MakeStatus(0x13572468UL, 0x12345678UL, 42U);
    MotorProtocolStatus decoded;
    uint8_t frame[MOTOR_PROTOCOL_FRAME_SIZE];

    assert(MotorProtocol_EncodeStatus(&source, frame, sizeof(frame)));
    assert(MotorProtocol_DecodeStatus(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_OK);

    assert(decoded.destination_address == source.destination_address);
    assert(decoded.source_address == source.source_address);
    assert(decoded.controller_session_id == source.controller_session_id);
    assert(decoded.acknowledged_sequence == source.acknowledged_sequence);
    assert(decoded.controller_uptime_ms == source.controller_uptime_ms);
    assert(decoded.remote_faults == source.remote_faults);
    assert(decoded.status_flags == source.status_flags);
    assert(decoded.status == source.status);
    assert(decoded.command_session_echo == source.command_session_echo);
}

static void Test_DecoderRejectsCorruption(void)
{
    MotorProtocolCommand command = MakeCommand(0x12345678UL, 42U, false);
    MotorProtocolCommand decoded;
    uint8_t frame[MOTOR_PROTOCOL_FRAME_SIZE];

    assert(MotorProtocol_EncodeCommand(&command, frame, sizeof(frame)));

    frame[20] ^= 0x01U;
    assert(MotorProtocol_DecodeCommand(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_BAD_CRC);

    assert(MotorProtocol_EncodeCommand(&command, frame, sizeof(frame)));
    frame[0] = 0U;
    RecomputeFrameCrc(frame);
    assert(MotorProtocol_DecodeCommand(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_BAD_MAGIC);

    assert(MotorProtocol_EncodeCommand(&command, frame, sizeof(frame)));
    frame[2] = (uint8_t)(MOTOR_PROTOCOL_VERSION + 1U);
    RecomputeFrameCrc(frame);
    assert(MotorProtocol_DecodeCommand(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_BAD_VERSION);

    assert(MotorProtocol_EncodeCommand(&command, frame, sizeof(frame)));
    frame[6] = 0x80U;
    RecomputeFrameCrc(frame);
    assert(MotorProtocol_DecodeCommand(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_UNKNOWN_FLAGS);

    assert(MotorProtocol_EncodeCommand(&command, frame, sizeof(frame)));
    frame[4] = MOTOR_PROTOCOL_ADDRESS_BROADCAST;
    RecomputeFrameCrc(frame);
    assert(MotorProtocol_DecodeCommand(frame, sizeof(frame), &decoded) ==
           MOTOR_PROTOCOL_DECODE_BAD_ADDRESS);
}

static void Test_LinkInitRejectsInvalidIdentity(void)
{
    MotorLinkContext context;

    assert(!MotorLinkState_Init(
        &context,
        MOTOR_PROTOCOL_ADDRESS_UNASSIGNED,
        TEST_MOTOR_ADDRESS,
        0x12345678UL));
    assert(context.state == MOTOR_LINK_FAULT);

    assert(!MotorLinkState_Init(
        &context,
        TEST_HOST_ADDRESS,
        TEST_HOST_ADDRESS,
        0x12345678UL));
    assert(context.state == MOTOR_LINK_FAULT);

    assert(!MotorLinkState_Init(
        &context,
        TEST_HOST_ADDRESS,
        TEST_MOTOR_ADDRESS,
        0U));
    assert(context.state == MOTOR_LINK_FAULT);
}

static void Test_LinkRequiresSessionQualification(void)
{
    MotorLinkContext context;
    MotorProtocolStatus status = MakeStatus(0xAA55AA55UL, 0x12345678UL, 1U);

    assert(MotorLinkState_Init(
        &context, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));
    assert(!MotorLinkState_IsLinkValid(&context));

    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_SYNC_STARTED);
    assert(context.state == MOTOR_LINK_SYNCHRONIZING);
    assert(!MotorLinkState_IsLinkValid(&context));

    status.acknowledged_sequence = 2U;
    assert(MotorLinkState_ProcessStatus(&context, &status, 2U, false, 20U) ==
           MOTOR_LINK_PROCESS_ACCEPTED);
    assert(context.state == MOTOR_LINK_READY);
    assert(MotorLinkState_IsLinkValid(&context));
}

static void Test_LinkRejectsWrongSessionAndStaleAck(void)
{
    MotorLinkContext context;
    MotorProtocolStatus status = MakeStatus(0xAA55AA55UL, 0x12345678UL, 1U);

    assert(MotorLinkState_Init(
        &context, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));
    status.command_session_echo = 0x87654321UL;
    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_SESSION_MISMATCH);
    assert((context.fault_history & MOTOR_LINK_FAULT_SESSION_MISMATCH) != 0U);

    assert(MotorLinkState_Init(
        &context, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));
    status = MakeStatus(0xAA55AA55UL, 0x12345678UL, 1U);
    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_SYNC_STARTED);
    status.acknowledged_sequence = 1U;
    assert(MotorLinkState_ProcessStatus(&context, &status, 2U, false, 20U) ==
           MOTOR_LINK_PROCESS_ACK_MISMATCH);
    assert((context.fault_history & MOTOR_LINK_FAULT_ACK_MISMATCH) != 0U);
}

static void Test_LinkRejectsWrongAddress(void)
{
    MotorLinkContext context;
    MotorProtocolStatus status = MakeStatus(0xAA55AA55UL, 0x12345678UL, 1U);

    assert(MotorLinkState_Init(
        &context,
        TEST_HOST_ADDRESS,
        TEST_MOTOR_ADDRESS,
        0x12345678UL));

    status.source_address = 0x11U;
    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_ADDRESS_MISMATCH);
    assert((context.fault_history & MOTOR_LINK_FAULT_ADDRESS_MISMATCH) != 0U);
    assert(!context.controller_session_known);
    assert(!MotorLinkState_IsLinkValid(&context));
}

static void Test_FirstFrameCannotQualifyWithWrongAck(void)
{
    MotorLinkContext context;
    MotorProtocolStatus status = MakeStatus(0xAA55AA55UL, 0x12345678UL, 99U);

    assert(MotorLinkState_Init(
        &context, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));
    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_ACK_MISMATCH);
    assert(!context.controller_session_known);
    assert(context.receive_sequence == 0U);
    assert(!MotorLinkState_IsLinkValid(&context));
    assert((context.fault_history & MOTOR_LINK_FAULT_ACK_MISMATCH) != 0U);
}

static void Test_ControllerRestartForcesResynchronization(void)
{
    MotorLinkContext context;
    MotorProtocolStatus status = MakeStatus(0x11111111UL, 0x12345678UL, 1U);

    assert(MotorLinkState_Init(
        &context, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));
    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_SYNC_STARTED);
    status.acknowledged_sequence = 2U;
    assert(MotorLinkState_ProcessStatus(&context, &status, 2U, false, 20U) ==
           MOTOR_LINK_PROCESS_ACCEPTED);
    assert(context.state == MOTOR_LINK_READY);

    status.controller_session_id = 0x22222222UL;
    status.acknowledged_sequence = 3U;
    assert(MotorLinkState_ProcessStatus(&context, &status, 3U, false, 30U) ==
           MOTOR_LINK_PROCESS_CONTROLLER_RESTARTED);
    assert(context.state == MOTOR_LINK_SYNCHRONIZING);
    assert((context.fault_history & MOTOR_LINK_FAULT_CONTROLLER_RESTART) != 0U);
    assert(!MotorLinkState_IsLinkValid(&context));
}

static void Test_LinkTimeout(void)
{
    MotorLinkContext context;
    MotorProtocolStatus status = MakeStatus(0xAA55AA55UL, 0x12345678UL, 1U);

    assert(MotorLinkState_Init(
        &context, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));
    assert(MotorLinkState_ProcessStatus(&context, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_SYNC_STARTED);
    status.acknowledged_sequence = 2U;
    assert(MotorLinkState_ProcessStatus(&context, &status, 2U, false, 20U) ==
           MOTOR_LINK_PROCESS_ACCEPTED);
    assert(MotorLinkState_IsLinkValid(&context));

    MotorLinkState_UpdateTimeout(&context, 71U, APP_LINK_MAX_AGE_MS);
    assert(context.state == MOTOR_LINK_OFFLINE);
    assert((context.fault_history & MOTOR_LINK_FAULT_TIMEOUT) != 0U);
}

static CommandAuthorizationInput MakeAuthorizationInput(void)
{
    CommandAuthorizationInput input = {0};

    input.request.request_sequence = 10U;
    input.request.input_sequence = 5U;
    input.request.generated_at_ms = 100U;
    input.request.forward_q15 = 1500;
    input.request.turn_q15 = -500;
    input.request.maximum_speed_q15 = 16000U;
    input.request.enable_request = true;
    input.safety_state = APP_SAFETY_READY;
    input.active_faults = APP_FAULT_NONE;
    input.link_status.receive_sequence = 10U;
    input.link_status.last_valid_packet_ms = 100U;
    input.link_status.remote_faults = APP_FAULT_NONE;
    input.link_status.link_valid = true;
    input.link_status.remote_drive_ready = true;
    input.link_status.brakes_confirmed = true;
    input.now_ms = 105U;
    input.configuration_valid = true;
    input.input_valid = true;
    input.mandatory_tasks_healthy = true;

    return input;
}

static void Test_LogicalAuthorizationCannotBecomePhysicalMotion(void)
{
    CommandAuthorizationInput input = MakeAuthorizationInput();
    CommandAuthorizationResult result = CommandAuthorization_Evaluate(&input);

    assert(result.logical_authorized);
    assert(result.blocking_reasons == COMMAND_AUTH_BLOCK_NONE);

    assert(!result.physical_command.drive_authorized);
    assert(result.physical_command.forward_q15 == 0);
    assert(result.physical_command.turn_q15 == 0);
    assert(result.physical_command.maximum_speed_q15 == 0U);
}

static void Test_AuthorizationFailsClosed(void)
{
    CommandAuthorizationInput input = MakeAuthorizationInput();
    CommandAuthorizationResult result;

    input.link_status.link_valid = false;
    result = CommandAuthorization_Evaluate(&input);
    assert(!result.logical_authorized);
    assert((result.blocking_reasons & COMMAND_AUTH_BLOCK_LINK) != 0U);

    input = MakeAuthorizationInput();
    input.link_status.remote_faults = APP_FAULT_REMOTE_CONTROLLER;
    result = CommandAuthorization_Evaluate(&input);
    assert(!result.logical_authorized);
    assert((result.blocking_reasons & COMMAND_AUTH_BLOCK_REMOTE_FAULT) != 0U);

    input = MakeAuthorizationInput();
    input.request.generated_at_ms = 0U;
    input.now_ms = APP_MOTOR_COMMAND_MAX_AGE_MS + 1U;
    result = CommandAuthorization_Evaluate(&input);
    assert(!result.logical_authorized);
    assert((result.blocking_reasons & COMMAND_AUTH_BLOCK_COMMAND_STALE) != 0U);

    result = CommandAuthorization_Evaluate(NULL);
    assert(!result.logical_authorized);
    assert(!result.physical_command.drive_authorized);
}

static void Test_FakeMotorControllerNormalAndFaults(void)
{
    FakeMotorController controller;
    MotorLinkContext link;
    MotorProtocolCommand command;
    MotorProtocolStatus status;
    uint8_t command_frame[MOTOR_PROTOCOL_FRAME_SIZE];
    uint8_t status_frame[MOTOR_PROTOCOL_FRAME_SIZE];

    FakeMotorController_Init(&controller, TEST_MOTOR_ADDRESS, TEST_HOST_ADDRESS, 0xABCDEF01UL);
    assert(MotorLinkState_Init(
        &link, TEST_HOST_ADDRESS, TEST_MOTOR_ADDRESS, 0x12345678UL));

    command = MakeCommand(0x12345678UL, 1U, false);
    assert(MotorProtocol_EncodeCommand(&command, command_frame, sizeof(command_frame)));
    assert(FakeMotorController_Exchange(
        &controller,
        command_frame,
        sizeof(command_frame),
        status_frame,
        sizeof(status_frame)));
    assert(MotorProtocol_DecodeStatus(status_frame, sizeof(status_frame), &status) ==
           MOTOR_PROTOCOL_DECODE_OK);
    assert(MotorLinkState_ProcessStatus(&link, &status, 1U, false, 10U) ==
           MOTOR_LINK_PROCESS_SYNC_STARTED);

    command = MakeCommand(0x12345678UL, 2U, true);
    assert(MotorProtocol_EncodeCommand(&command, command_frame, sizeof(command_frame)));
    assert(FakeMotorController_Exchange(
        &controller,
        command_frame,
        sizeof(command_frame),
        status_frame,
        sizeof(status_frame)));
    assert(MotorProtocol_DecodeStatus(status_frame, sizeof(status_frame), &status) ==
           MOTOR_PROTOCOL_DECODE_OK);
    assert(MotorLinkState_ProcessStatus(&link, &status, 2U, true, 20U) ==
           MOTOR_LINK_PROCESS_ACCEPTED);
    assert(link.state == MOTOR_LINK_ACTIVE);

    FakeMotorController_SetMode(&controller, FAKE_MOTOR_MODE_CORRUPT_CRC);
    command = MakeCommand(0x12345678UL, 3U, true);
    assert(MotorProtocol_EncodeCommand(&command, command_frame, sizeof(command_frame)));
    assert(FakeMotorController_Exchange(
        &controller,
        command_frame,
        sizeof(command_frame),
        status_frame,
        sizeof(status_frame)));
    assert(MotorProtocol_DecodeStatus(status_frame, sizeof(status_frame), &status) ==
           MOTOR_PROTOCOL_DECODE_BAD_CRC);
    MotorLinkState_RecordDecodeFailure(&link, MOTOR_PROTOCOL_DECODE_BAD_CRC);
    assert(!MotorLinkState_IsLinkValid(&link));

    FakeMotorController_SetMode(&controller, FAKE_MOTOR_MODE_REMOTE_FAULT);
    command = MakeCommand(0x12345678UL, 4U, true);
    assert(MotorProtocol_EncodeCommand(&command, command_frame, sizeof(command_frame)));
    assert(FakeMotorController_Exchange(
        &controller,
        command_frame,
        sizeof(command_frame),
        status_frame,
        sizeof(status_frame)));
    assert(MotorProtocol_DecodeStatus(status_frame, sizeof(status_frame), &status) ==
           MOTOR_PROTOCOL_DECODE_OK);
    assert(MotorLinkState_ProcessStatus(&link, &status, 4U, true, 40U) ==
           MOTOR_LINK_PROCESS_REMOTE_FAULT);
    assert(link.state == MOTOR_LINK_FAULT);
}

static uint32_t FuzzNext(uint32_t state)
{
    return (uint32_t)((state * UINT32_C(1664525)) + UINT32_C(1013904223));
}

static void Test_MalformedFramesNeverEscapeDecoderContract(void)
{
    uint8_t frame[MOTOR_PROTOCOL_FRAME_SIZE];
    MotorProtocolCommand command;
    MotorProtocolStatus status;
    uint32_t rng = 0xC001D00DUL;
    uint32_t iteration;

    for (iteration = 0U; iteration < 10000U; iteration++)
    {
        size_t index;

        for (index = 0U; index < sizeof(frame); index++)
        {
            rng = FuzzNext(rng);
            frame[index] = (uint8_t)(rng >> 24U);
        }

        (void)MotorProtocol_DecodeCommand(frame, sizeof(frame), &command);
        (void)MotorProtocol_DecodeStatus(frame, sizeof(frame), &status);
    }
}

int main(void)
{
    Test_CrcGoldenVector();
    Test_CommandRoundTrip();
    Test_StatusRoundTrip();
    Test_DecoderRejectsCorruption();
    Test_LinkInitRejectsInvalidIdentity();
    Test_LinkRequiresSessionQualification();
    Test_LinkRejectsWrongSessionAndStaleAck();
    Test_LinkRejectsWrongAddress();
    Test_FirstFrameCannotQualifyWithWrongAck();
    Test_ControllerRestartForcesResynchronization();
    Test_LinkTimeout();
    Test_LogicalAuthorizationCannotBecomePhysicalMotion();
    Test_AuthorizationFailsClosed();
    Test_FakeMotorControllerNormalAndFaults();
    Test_MalformedFramesNeverEscapeDecoderContract();

    puts("Phase 2 host tests passed");
    return 0;
}
