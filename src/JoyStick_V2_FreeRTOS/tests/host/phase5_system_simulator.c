/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include "phase5_system_simulator.h"

#include <stddef.h>
#include <string.h>

#include "app_build_config.h"
#include "joystick_configuration.h"
#include "motor_protocol.h"

static JoystickConfigurationImage Phase5SystemSimulator_MakeConfiguration(void)
{
    JoystickConfigurationImage image = {0};

    /*
     * These are synthetic host-test values, not claimed CHC-104B-M2 bench
     * measurements.  They provide asymmetric-capable valid calibration spans
     * around nominal ADC center so system logic can be exercised deterministically.
     */
    image.generation = 5U;
    image.configuration.x_axis.minimum_counts = 200U;
    image.configuration.x_axis.center_counts = 2048U;
    image.configuration.x_axis.maximum_counts = 3900U;
    image.configuration.x_axis.deadband_counts = 96U;
    image.configuration.x_axis.inverted = false;
    image.configuration.y_axis = image.configuration.x_axis;
    image.configuration.maximum_speed_q15 = JOYSTICK_CONFIG_Q15_MAX;
    image.configuration.response_curve_q15 = 0U;
    image.configuration.reference_profile = JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2;

    return image;
}

static void Phase5SystemSimulator_CopyLinkStatus(
    Phase5SystemSimulator *simulator)
{
    MotorLinkState_CopyAppStatus(
        &simulator->motor_link,
        &simulator->motor_link_status);
}

static bool Phase5SystemSimulator_ExchangeLink(
    Phase5SystemSimulator *simulator)
{
    MotorProtocolCommand command = {0};
    MotorProtocolStatus status = {0};
    uint8_t command_frame[MOTOR_PROTOCOL_FRAME_SIZE] = {0};
    uint8_t status_frame[MOTOR_PROTOCOL_FRAME_SIZE] = {0};

    simulator->link_command_sequence++;
    simulator->link_exchange_count++;
    simulator->last_link_response_received = false;
    simulator->last_decode_result = MOTOR_PROTOCOL_DECODE_WRONG_LENGTH;
    simulator->last_link_result = MOTOR_LINK_PROCESS_STATUS_REJECTED;

    command.destination_address = PHASE5_SIM_MOTOR_ADDRESS;
    command.source_address = PHASE5_SIM_HOST_ADDRESS;
    command.command_flags = 0U;
    if (simulator->output.requested_command.enable_request)
    {
        command.command_flags |= MOTOR_PROTOCOL_COMMAND_FLAG_ENABLE_REQUEST;
    }
    if (simulator->output.authorization.logical_authorized)
    {
        command.command_flags |= MOTOR_PROTOCOL_COMMAND_FLAG_LOGICAL_AUTH;
    }
    command.safety_state = simulator->runtime.safety.state;
    command.command_session_id = simulator->command_session_id;
    command.sequence = simulator->link_command_sequence;
    command.generated_at_ms = simulator->now_ms;
    command.forward_q15 = simulator->output.requested_command.forward_q15;
    command.turn_q15 = simulator->output.requested_command.turn_q15;
    command.maximum_speed_q15 =
        simulator->output.requested_command.maximum_speed_q15;
    command.active_faults = simulator->runtime.safety.active_faults;

    if (!MotorProtocol_EncodeCommand(
            &command,
            command_frame,
            sizeof(command_frame)))
    {
        return false;
    }

    if (!FakeMotorController_Exchange(
            &simulator->motor_controller,
            command_frame,
            sizeof(command_frame),
            status_frame,
            sizeof(status_frame)))
    {
        /* A dropped response is modeled as silence; timeout owns invalidation. */
        return true;
    }

    simulator->last_link_response_received = true;
    simulator->last_decode_result = MotorProtocol_DecodeStatus(
        status_frame,
        sizeof(status_frame),
        &status);

    if (simulator->last_decode_result != MOTOR_PROTOCOL_DECODE_OK)
    {
        MotorLinkState_RecordDecodeFailure(
            &simulator->motor_link,
            simulator->last_decode_result);
        Phase5SystemSimulator_CopyLinkStatus(simulator);
        return true;
    }

    simulator->last_link_result = MotorLinkState_ProcessStatus(
        &simulator->motor_link,
        &status,
        simulator->link_command_sequence,
        simulator->output.authorization.logical_authorized,
        simulator->now_ms);
    Phase5SystemSimulator_CopyLinkStatus(simulator);

    return true;
}

bool Phase5SystemSimulator_Init(Phase5SystemSimulator *simulator)
{
    if (simulator == NULL)
    {
        return false;
    }

    (void)memset(simulator, 0, sizeof(*simulator));

    simulator->now_ms = 0U;
    simulator->command_session_id = PHASE5_SIM_INITIAL_COMMAND_SESSION;
    simulator->mandatory_tasks_healthy = true;
    simulator->freeze_input_timestamp = false;

    ConfigurationRuntime_Init(&simulator->configuration);
    simulator->configuration.active_image =
        Phase5SystemSimulator_MakeConfiguration();
    simulator->configuration.status = JOYSTICK_CONFIG_STATUS_OK;
    simulator->configuration.selected_slot = 0U;
    simulator->configuration.valid = true;

    simulator->hmi.sequence = 1U;
    simulator->hmi.sampled_at_ms = 0U;
    simulator->hmi.buttons_active_low = UINT16_MAX;
    simulator->hmi.rotary_1_active_low = UINT8_MAX;
    simulator->hmi.rotary_2_active_low = UINT8_MAX;
    simulator->hmi.requested_mode = APP_OPERATING_MODE_NORMAL;
    simulator->hmi.maximum_speed_q15 = HMI_MODEL_SPEED_Q15_FULL;
    simulator->hmi.enable_request = false;

    /*
     * Phase-5 host simulation injects a qualified synthetic HMI mapping so the
     * logical path can be exercised.  The target HMI model still hard-codes
     * control_mapping_valid=false until physical characterization exists.
     */
    simulator->hmi.control_mapping_valid = true;

    simulator->raw_input.sequence = 1U;
    simulator->raw_input.captured_at_ms = 0U;
    simulator->raw_input.joystick_x_counts = 2048U;
    simulator->raw_input.joystick_y_counts = 2048U;
    simulator->raw_input.buttons_active_low = UINT16_MAX;
    simulator->raw_input.rotary_1_active_low = UINT8_MAX;
    simulator->raw_input.rotary_2_active_low = UINT8_MAX;
    simulator->raw_input.power_good = true;
    simulator->raw_input.dma_overrun_detected = false;

    RuntimeControl_Init(&simulator->runtime, simulator->now_ms);

    if (!MotorLinkState_Init(
            &simulator->motor_link,
            PHASE5_SIM_HOST_ADDRESS,
            PHASE5_SIM_MOTOR_ADDRESS,
            simulator->command_session_id))
    {
        return false;
    }

    FakeMotorController_Init(
        &simulator->motor_controller,
        PHASE5_SIM_MOTOR_ADDRESS,
        PHASE5_SIM_HOST_ADDRESS,
        PHASE5_SIM_CONTROLLER_SESSION);

    Phase5SystemSimulator_CopyLinkStatus(simulator);
    simulator->input_diagnostics = InputDiagnostics_Evaluate(
        &simulator->raw_input,
        simulator->now_ms);

    return true;
}

bool Phase5SystemSimulator_ResetFirmware(Phase5SystemSimulator *simulator)
{
    if (simulator == NULL)
    {
        return false;
    }

    /*
     * A reboot creates a new command session but preserves external reality:
     * stick position, HMI request, controller session, and configuration image.
     */
    simulator->command_session_id++;
    if (simulator->command_session_id == 0U)
    {
        simulator->command_session_id = 1U;
    }

    RuntimeControl_Init(&simulator->runtime, simulator->now_ms);
    simulator->link_command_sequence = 0U;
    simulator->link_period_accumulator_ms = 0U;
    simulator->last_link_response_received = false;

    if (!MotorLinkState_Init(
            &simulator->motor_link,
            PHASE5_SIM_HOST_ADDRESS,
            PHASE5_SIM_MOTOR_ADDRESS,
            simulator->command_session_id))
    {
        return false;
    }

    Phase5SystemSimulator_CopyLinkStatus(simulator);
    (void)memset(&simulator->output, 0, sizeof(simulator->output));
    return true;
}

bool Phase5SystemSimulator_Step(Phase5SystemSimulator *simulator)
{
    RuntimeControlInput input;

    if (simulator == NULL)
    {
        return false;
    }

    simulator->now_ms += PHASE5_SIM_STEP_MS;

    if (!simulator->freeze_input_timestamp)
    {
        simulator->raw_input.sequence++;
        simulator->raw_input.captured_at_ms = simulator->now_ms;
    }

    simulator->hmi.sampled_at_ms = simulator->now_ms;
    simulator->input_diagnostics = InputDiagnostics_Evaluate(
        &simulator->raw_input,
        simulator->now_ms);

    MotorLinkState_UpdateTimeout(
        &simulator->motor_link,
        simulator->now_ms,
        APP_LINK_MAX_AGE_MS);
    Phase5SystemSimulator_CopyLinkStatus(simulator);

    input.raw_input = &simulator->raw_input;
    input.input_diagnostics = &simulator->input_diagnostics;
    input.configuration = &simulator->configuration;
    input.hmi = &simulator->hmi;
    input.motor_link = &simulator->motor_link_status;
    input.now_ms = simulator->now_ms;
    input.mandatory_tasks_healthy = simulator->mandatory_tasks_healthy;

    if (!RuntimeControl_Step(
            &simulator->runtime,
            &input,
            &simulator->output))
    {
        return false;
    }

    /*
     * Model the periodic link task by elapsed time, not absolute tick modulo.
     * Absolute modulo is not wrap-safe when UINT32 rollover is not an exact
     * multiple of the task period.
     */
    simulator->link_period_accumulator_ms += PHASE5_SIM_STEP_MS;
    if (simulator->link_period_accumulator_ms >= PHASE5_SIM_LINK_PERIOD_MS)
    {
        simulator->link_period_accumulator_ms -= PHASE5_SIM_LINK_PERIOD_MS;
        if (!Phase5SystemSimulator_ExchangeLink(simulator))
        {
            return false;
        }
    }

    return true;
}

bool Phase5SystemSimulator_RunForMs(
    Phase5SystemSimulator *simulator,
    uint32_t duration_ms)
{
    uint32_t step_count;
    uint32_t index;

    if ((simulator == NULL) ||
        (duration_ms > PHASE5_SIM_MAX_RUN_MS) ||
        ((duration_ms % PHASE5_SIM_STEP_MS) != 0U))
    {
        return false;
    }

    step_count = duration_ms / PHASE5_SIM_STEP_MS;
    for (index = 0U; index < step_count; index++)
    {
        if (!Phase5SystemSimulator_Step(simulator))
        {
            return false;
        }
    }

    return true;
}

void Phase5SystemSimulator_SetJoystick(
    Phase5SystemSimulator *simulator,
    uint16_t x_counts,
    uint16_t y_counts)
{
    if (simulator != NULL)
    {
        simulator->raw_input.joystick_x_counts = x_counts;
        simulator->raw_input.joystick_y_counts = y_counts;
    }
}

void Phase5SystemSimulator_SetEnable(
    Phase5SystemSimulator *simulator,
    bool enable_request)
{
    if (simulator != NULL)
    {
        simulator->hmi.enable_request = enable_request;
        simulator->hmi.sequence++;
    }
}

void Phase5SystemSimulator_SetMode(
    Phase5SystemSimulator *simulator,
    AppOperatingMode requested_mode,
    uint16_t maximum_speed_q15)
{
    if (simulator != NULL)
    {
        simulator->hmi.requested_mode = requested_mode;
        simulator->hmi.maximum_speed_q15 = maximum_speed_q15;
        simulator->hmi.sequence++;
    }
}

void Phase5SystemSimulator_SetConfigurationValid(
    Phase5SystemSimulator *simulator,
    bool valid)
{
    if (simulator != NULL)
    {
        simulator->configuration.valid = valid;
        simulator->configuration.status = valid ?
            JOYSTICK_CONFIG_STATUS_OK : JOYSTICK_CONFIG_STATUS_NO_VALID_RECORD;
    }
}

void Phase5SystemSimulator_SetMandatoryTasksHealthy(
    Phase5SystemSimulator *simulator,
    bool healthy)
{
    if (simulator != NULL)
    {
        simulator->mandatory_tasks_healthy = healthy;
    }
}

void Phase5SystemSimulator_SetPowerGood(
    Phase5SystemSimulator *simulator,
    bool power_good)
{
    if (simulator != NULL)
    {
        simulator->raw_input.power_good = power_good;
    }
}

void Phase5SystemSimulator_SetDmaOverrun(
    Phase5SystemSimulator *simulator,
    bool overrun_detected)
{
    if (simulator != NULL)
    {
        simulator->raw_input.dma_overrun_detected = overrun_detected;
    }
}

void Phase5SystemSimulator_FreezeInputTimestamp(
    Phase5SystemSimulator *simulator,
    bool freeze)
{
    if (simulator != NULL)
    {
        simulator->freeze_input_timestamp = freeze;
    }
}

void Phase5SystemSimulator_SetMotorMode(
    Phase5SystemSimulator *simulator,
    FakeMotorControllerMode mode)
{
    if (simulator != NULL)
    {
        FakeMotorController_SetMode(&simulator->motor_controller, mode);
    }
}
