/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_build_config.h"
#include "command_authorization.h"
#include "motor_link_state.h"
#include "phase5_system_simulator.h"

#define PHASE5_CENTER_COUNTS                    (2048U)
#define PHASE5_FORWARD_COUNTS                   (3000U)
#define PHASE5_CAMPAIGN_STEPS                   (20000U)

static bool Test_Check(bool condition, const char *message)
{
    if (!condition)
    {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return false;
    }
    return true;
}

static bool Test_PhysicalOutputIsInhibited(
    const Phase5SystemSimulator *simulator,
    const char *message)
{
    return Test_Check(
        (!simulator->output.authorization.physical_command.drive_authorized) &&
            (simulator->output.authorization.physical_command.forward_q15 == 0) &&
            (simulator->output.authorization.physical_command.turn_q15 == 0) &&
            (simulator->output.authorization.physical_command.maximum_speed_q15 == 0U),
        message);
}

static bool Test_RunToReady(Phase5SystemSimulator *simulator)
{
    Phase5SystemSimulator_SetJoystick(
        simulator,
        PHASE5_CENTER_COUNTS,
        PHASE5_CENTER_COUNTS);
    Phase5SystemSimulator_SetEnable(simulator, false);
    Phase5SystemSimulator_SetMode(
        simulator,
        APP_OPERATING_MODE_NORMAL,
        HMI_MODEL_SPEED_Q15_FULL);
    Phase5SystemSimulator_SetMotorMode(simulator, FAKE_MOTOR_MODE_NORMAL);
    Phase5SystemSimulator_SetConfigurationValid(simulator, true);
    Phase5SystemSimulator_SetMandatoryTasksHealthy(simulator, true);
    Phase5SystemSimulator_SetPowerGood(simulator, true);
    Phase5SystemSimulator_SetDmaOverrun(simulator, false);
    Phase5SystemSimulator_FreezeInputTimestamp(simulator, false);

    if (!Phase5SystemSimulator_RunForMs(simulator, 700U))
    {
        return false;
    }

    return (simulator->motor_link_status.link_valid &&
            (simulator->runtime.safety.state == APP_SAFETY_READY) &&
            (!simulator->output.authorization.logical_authorized));
}

static bool Test_EnterLogicalDrive(Phase5SystemSimulator *simulator)
{
    if (!Test_RunToReady(simulator))
    {
        return false;
    }

    /* Enable is qualified while neutral; motion follows on a later control step. */
    Phase5SystemSimulator_SetEnable(simulator, true);
    if (!Phase5SystemSimulator_Step(simulator))
    {
        return false;
    }
    if (!(simulator->runtime.safety.state == APP_SAFETY_DRIVE_AUTHORIZED))
    {
        return false;
    }
    if (!simulator->output.authorization.logical_authorized)
    {
        return false;
    }

    Phase5SystemSimulator_SetJoystick(
        simulator,
        PHASE5_CENTER_COUNTS,
        PHASE5_FORWARD_COUNTS);
    if (!Phase5SystemSimulator_Step(simulator))
    {
        return false;
    }

    return simulator->output.authorization.logical_authorized &&
           (simulator->output.requested_command.forward_q15 > 0);
}

static bool Test_LogicalAuthorizationPhysicalInhibit(void)
{
    Phase5SystemSimulator simulator;

    if (!Test_Check(
            Phase5SystemSimulator_Init(&simulator),
            "initialize full-system simulator"))
    {
        return false;
    }
    if (!Test_Check(
            Test_EnterLogicalDrive(&simulator),
            "logical drive authorization is reachable after neutral qualification"))
    {
        return false;
    }
    if (!Test_Check(
            simulator.output.authorization.blocking_reasons == COMMAND_AUTH_BLOCK_NONE,
            "logical authorization has no hidden blocking reason"))
    {
        return false;
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "logical authorization never becomes physical output");
}

static bool Test_ControllerRestartRequiresSafetyRequalification(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_SetMotorMode(
        &simulator,
        FAKE_MOTOR_MODE_RESET_CONTROLLER);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 10U))
    {
        return false;
    }
    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_NORMAL);

    if (!Test_Check(
            !simulator.motor_link_status.link_valid,
            "controller restart drops link qualification"))
    {
        return false;
    }

    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            !simulator.output.authorization.logical_authorized,
            "controller restart removes logical authorization on next control step"))
    {
        return false;
    }

    /* Link can recover while the stick is displaced, but Safety cannot. */
    if (!Phase5SystemSimulator_RunForMs(&simulator, 40U))
    {
        return false;
    }
    if (!Test_Check(
            simulator.motor_link_status.link_valid,
            "controller link requalifies after restart"))
    {
        return false;
    }
    if (!Test_Check(
            simulator.runtime.safety.state != APP_SAFETY_DRIVE_AUTHORIZED,
            "link recovery cannot bypass neutral qualification"))
    {
        return false;
    }

    Phase5SystemSimulator_SetEnable(&simulator, false);
    Phase5SystemSimulator_SetJoystick(
        &simulator,
        PHASE5_CENTER_COUNTS,
        PHASE5_CENTER_COUNTS);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 700U))
    {
        return false;
    }

    return Test_Check(
        simulator.runtime.safety.state == APP_SAFETY_READY,
        "neutral dwell restores READY after controller restart");
}

static bool Test_RemoteFaultRequiresFreshLinkFrames(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_REMOTE_FAULT);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 10U))
    {
        return false;
    }
    if (!Test_Check(
            simulator.motor_link.state == MOTOR_LINK_FAULT,
            "remote fault enters link FAULT"))
    {
        return false;
    }
    if (!Test_Check(
            simulator.motor_link.consecutive_valid_frames == 0U,
            "remote fault discards pre-fault link qualification"))
    {
        return false;
    }

    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_NORMAL);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 10U))
    {
        return false;
    }
    if (!Test_Check(
            !simulator.motor_link_status.link_valid,
            "one post-fault good frame cannot restore link validity"))
    {
        return false;
    }
    if (!Test_Check(
            simulator.motor_link.consecutive_valid_frames == 1U,
            "first post-fault frame begins fresh qualification"))
    {
        return false;
    }

    if (!Phase5SystemSimulator_RunForMs(&simulator, 10U))
    {
        return false;
    }
    if (!Test_Check(
            simulator.motor_link_status.link_valid,
            "second post-fault good frame can restore link validity"))
    {
        return false;
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "remote-fault recovery never bypasses physical inhibit");
}

static bool Test_ProtocolFaultsFailClosed(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_CORRUPT_CRC);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 10U))
    {
        return false;
    }
    if (!Test_Check(
            !simulator.motor_link_status.link_valid,
            "CRC failure invalidates qualified link"))
    {
        return false;
    }
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            !simulator.output.authorization.logical_authorized,
            "CRC failure removes logical authorization"))
    {
        return false;
    }

    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_NORMAL);
    Phase5SystemSimulator_SetEnable(&simulator, false);
    Phase5SystemSimulator_SetJoystick(
        &simulator,
        PHASE5_CENTER_COUNTS,
        PHASE5_CENTER_COUNTS);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 700U) ||
        (simulator.runtime.safety.state != APP_SAFETY_READY))
    {
        return false;
    }

    Phase5SystemSimulator_SetEnable(&simulator, true);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_STALE_ACK);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 10U))
    {
        return false;
    }
    if (!Test_Check(
            !simulator.motor_link_status.link_valid,
            "stale ACK invalidates qualified link"))
    {
        return false;
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "protocol faults cannot create physical command");
}

static bool Test_LinkSilenceTimesOut(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_DROP_RESPONSE);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 70U))
    {
        return false;
    }

    if (!Test_Check(
            !simulator.motor_link_status.link_valid,
            "silent controller exceeds link timeout and goes offline"))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.motor_link.fault_history & MOTOR_LINK_FAULT_TIMEOUT) != 0U,
            "link timeout remains visible in fault history"))
    {
        return false;
    }

    return Test_Check(
        !simulator.output.authorization.logical_authorized,
        "link silence blocks logical authorization");
}

static bool Test_LocalPrerequisiteFaultsFailClosed(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_SetConfigurationValid(&simulator, false);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            (!simulator.output.authorization.logical_authorized) &&
                (!simulator.output.joystick_processed) &&
                (simulator.output.requested_command.forward_q15 == 0),
            "invalid configuration removes requested and logical motion"))
    {
        return false;
    }

    Phase5SystemSimulator_SetConfigurationValid(&simulator, true);
    Phase5SystemSimulator_SetMandatoryTasksHealthy(&simulator, false);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.output.authorization.blocking_reasons &
             COMMAND_AUTH_BLOCK_TASK_HEALTH) != 0U,
            "mandatory-task failure is authorization-visible"))
    {
        return false;
    }

    Phase5SystemSimulator_SetMandatoryTasksHealthy(&simulator, true);
    Phase5SystemSimulator_SetPowerGood(&simulator, false);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.input_diagnostics.faults & APP_FAULT_POWER_GOOD_LOST) != 0U,
            "power-good loss enters input diagnostics"))
    {
        return false;
    }

    Phase5SystemSimulator_SetPowerGood(&simulator, true);
    Phase5SystemSimulator_SetDmaOverrun(&simulator, true);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.input_diagnostics.faults & APP_FAULT_ADC_OVERRUN) != 0U,
            "DMA overrun enters input diagnostics"))
    {
        return false;
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "local prerequisite faults cannot produce physical output");
}

static bool Test_InputStalenessAndRailFaults(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_RunToReady(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_FreezeInputTimestamp(&simulator, true);
    if (!Phase5SystemSimulator_RunForMs(&simulator, 20U))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.input_diagnostics.faults & APP_FAULT_INPUT_STALE) != 0U,
            "stale acquisition timestamp is detected"))
    {
        return false;
    }

    Phase5SystemSimulator_FreezeInputTimestamp(&simulator, false);
    Phase5SystemSimulator_SetJoystick(&simulator, 0U, PHASE5_CENTER_COUNTS);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.input_diagnostics.faults & APP_FAULT_JOYSTICK_X_RANGE) != 0U,
            "ADC rail excursion is rejected"))
    {
        return false;
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "stale or out-of-range input cannot produce physical output");
}

static bool Test_ServiceModesHaveZeroMotionCeiling(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_RunToReady(&simulator))
    {
        return false;
    }

    Phase5SystemSimulator_SetMode(
        &simulator,
        APP_OPERATING_MODE_CALIBRATION,
        HMI_MODEL_SPEED_Q15_FULL);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }
    if (!Test_Check(
            (simulator.output.active_mode == APP_OPERATING_MODE_CALIBRATION) &&
                (simulator.output.requested_command.maximum_speed_q15 == 0U) &&
                (simulator.output.requested_command.forward_q15 == 0) &&
                (simulator.output.requested_command.turn_q15 == 0),
            "calibration mode has zero motion ceiling"))
    {
        return false;
    }

    Phase5SystemSimulator_SetMode(
        &simulator,
        APP_OPERATING_MODE_SERVICE,
        HMI_MODEL_SPEED_Q15_FULL);
    if (!Phase5SystemSimulator_Step(&simulator))
    {
        return false;
    }

    return Test_Check(
        (simulator.output.active_mode == APP_OPERATING_MODE_SERVICE) &&
            (simulator.output.requested_command.maximum_speed_q15 == 0U),
        "service mode has zero motion ceiling");
}

static bool Test_RebootCannotResumeDisplacedStick(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    if (!Phase5SystemSimulator_ResetFirmware(&simulator))
    {
        return false;
    }

    /* External enable and displaced stick deliberately remain unchanged. */
    if (!Phase5SystemSimulator_RunForMs(&simulator, 1000U))
    {
        return false;
    }
    if (!Test_Check(
            simulator.runtime.safety.state != APP_SAFETY_DRIVE_AUTHORIZED,
            "reboot with displaced stick cannot resume logical drive"))
    {
        return false;
    }
    if (!Test_Check(
            !simulator.output.authorization.logical_authorized,
            "reboot requires new neutral qualification"))
    {
        return false;
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "reboot never resumes physical output");
}

static bool Test_TickWraparound(void)
{
    Phase5SystemSimulator simulator;

    if (!Phase5SystemSimulator_Init(&simulator))
    {
        return false;
    }

    simulator.now_ms = UINT32_MAX - 100U;
    simulator.raw_input.captured_at_ms = simulator.now_ms;
    if (!Phase5SystemSimulator_ResetFirmware(&simulator))
    {
        return false;
    }

    if (!Test_RunToReady(&simulator))
    {
        return Test_Check(false, "neutral/link timing remains correct across uint32 tick wrap");
    }

    return Test_PhysicalOutputIsInhibited(
        &simulator,
        "tick wrap cannot create physical output");
}

static uint32_t Test_NextPrng(uint32_t *state)
{
    /* xorshift32: fixed-width deterministic input generation, never cryptographic. */
    uint32_t value = *state;

    value ^= value << 13U;
    value ^= value >> 17U;
    value ^= value << 5U;
    *state = value;
    return value;
}

static bool Test_CampaignInvariant(const Phase5SystemSimulator *simulator)
{
    const uint16_t configured_limit =
        simulator->configuration.active_image.configuration.maximum_speed_q15;

    if (simulator->output.authorization.physical_command.drive_authorized ||
        (simulator->output.authorization.physical_command.forward_q15 != 0) ||
        (simulator->output.authorization.physical_command.turn_q15 != 0) ||
        (simulator->output.authorization.physical_command.maximum_speed_q15 != 0U))
    {
        return false;
    }

    if (simulator->output.requested_command.maximum_speed_q15 > configured_limit)
    {
        return false;
    }

    if (simulator->output.authorization.logical_authorized)
    {
        if ((simulator->output.authorization.blocking_reasons != COMMAND_AUTH_BLOCK_NONE) ||
            (!simulator->configuration.valid) ||
            (!simulator->input_diagnostics.valid) ||
            (!simulator->mandatory_tasks_healthy) ||
            (!simulator->motor_link_status.link_valid) ||
            (!simulator->motor_link_status.remote_drive_ready) ||
            (!simulator->motor_link_status.brakes_confirmed) ||
            (!simulator->output.requested_command.enable_request) ||
            (simulator->runtime.safety.active_faults != APP_FAULT_NONE))
        {
            return false;
        }
    }

    if ((simulator->output.active_mode == APP_OPERATING_MODE_BOOT) ||
        (simulator->output.active_mode == APP_OPERATING_MODE_CALIBRATION) ||
        (simulator->output.active_mode == APP_OPERATING_MODE_SERVICE) ||
        (simulator->output.active_mode == APP_OPERATING_MODE_FAULT))
    {
        if ((simulator->output.requested_command.maximum_speed_q15 != 0U) ||
            (simulator->output.requested_command.forward_q15 != 0) ||
            (simulator->output.requested_command.turn_q15 != 0))
        {
            return false;
        }
    }

    if ((!simulator->configuration.valid) || (!simulator->input_diagnostics.valid))
    {
        if (simulator->output.authorization.logical_authorized)
        {
            return false;
        }
    }

    return true;
}

static bool Test_DeterministicLongFaultCampaign(void)
{
    Phase5SystemSimulator simulator;
    uint32_t prng = 0x5A17C3E1UL;
    uint32_t logical_authorized_count = 0U;
    uint32_t index;

    if (!Phase5SystemSimulator_Init(&simulator) ||
        !Test_EnterLogicalDrive(&simulator))
    {
        return false;
    }

    if (simulator.output.authorization.logical_authorized)
    {
        logical_authorized_count = 1U;
    }

    for (index = 0U; index < PHASE5_CAMPAIGN_STEPS; index++)
    {
        const uint32_t random = Test_NextPrng(&prng);
        const bool next_is_link_exchange =
            (simulator.link_period_accumulator_ms + PHASE5_SIM_STEP_MS) >=
            PHASE5_SIM_LINK_PERIOD_MS;
        FakeMotorControllerMode motor_mode = FAKE_MOTOR_MODE_NORMAL;
        uint16_t x_counts;
        uint16_t y_counts;

        /* Most samples are electrically plausible; some deliberately are not. */
        x_counts = (uint16_t)(128U + (random & 0x0DFFU));
        y_counts = (uint16_t)(128U + ((random >> 12U) & 0x0DFFU));
        if ((random & 0x000001FFUL) == 0U)
        {
            x_counts = 0U;
        }
        if ((random & 0x000003FFUL) == 1U)
        {
            y_counts = 4095U;
        }
        Phase5SystemSimulator_SetJoystick(&simulator, x_counts, y_counts);

        Phase5SystemSimulator_SetEnable(
            &simulator,
            (random & 0x00000008UL) != 0U);
        Phase5SystemSimulator_SetMandatoryTasksHealthy(
            &simulator,
            (random & 0x000007FFUL) != 0x000007FFUL);
        Phase5SystemSimulator_SetConfigurationValid(
            &simulator,
            (random & 0x00000FFFUL) != 0x00000AA5UL);
        Phase5SystemSimulator_SetPowerGood(
            &simulator,
            (random & 0x00001FFFUL) != 0x00001234UL);
        Phase5SystemSimulator_SetDmaOverrun(
            &simulator,
            (random & 0x00003FFFUL) == 0x00002000UL);

        if ((random & 0x0000007FUL) == 0U)
        {
            Phase5SystemSimulator_SetMode(
                &simulator,
                (AppOperatingMode)(random % 7U),
                (uint16_t)(random & 0x7FFFU));
        }

        if (next_is_link_exchange)
        {
            switch ((random >> 20U) & 0x003FU)
            {
                case 0U:
                    motor_mode = FAKE_MOTOR_MODE_DROP_RESPONSE;
                    break;
                case 1U:
                    motor_mode = FAKE_MOTOR_MODE_CORRUPT_CRC;
                    break;
                case 2U:
                    motor_mode = FAKE_MOTOR_MODE_WRONG_SESSION_ECHO;
                    break;
                case 3U:
                    motor_mode = FAKE_MOTOR_MODE_STALE_ACK;
                    break;
                case 4U:
                    motor_mode = FAKE_MOTOR_MODE_RESET_CONTROLLER;
                    break;
                case 5U:
                    motor_mode = FAKE_MOTOR_MODE_REMOTE_FAULT;
                    break;
                case 6U:
                    motor_mode = FAKE_MOTOR_MODE_NOT_READY;
                    break;
                default:
                    motor_mode = FAKE_MOTOR_MODE_NORMAL;
                    break;
            }
        }
        Phase5SystemSimulator_SetMotorMode(&simulator, motor_mode);

        if (!Phase5SystemSimulator_Step(&simulator))
        {
            return false;
        }
        Phase5SystemSimulator_SetMotorMode(&simulator, FAKE_MOTOR_MODE_NORMAL);

        if (!Test_CampaignInvariant(&simulator))
        {
            (void)fprintf(stderr, "FAIL: long campaign invariant at step %lu\n",
                          (unsigned long)index);
            return false;
        }

        if (simulator.output.authorization.logical_authorized)
        {
            logical_authorized_count++;
        }
    }

    return Test_Check(
        logical_authorized_count > 0U,
        "long campaign exercised logical authorization before/among faults");
}

int main(void)
{
    if (!Test_LogicalAuthorizationPhysicalInhibit())
    {
        return 1;
    }
    if (!Test_ControllerRestartRequiresSafetyRequalification())
    {
        return 1;
    }
    if (!Test_RemoteFaultRequiresFreshLinkFrames())
    {
        return 1;
    }
    if (!Test_ProtocolFaultsFailClosed())
    {
        return 1;
    }
    if (!Test_LinkSilenceTimesOut())
    {
        return 1;
    }
    if (!Test_LocalPrerequisiteFaultsFailClosed())
    {
        return 1;
    }
    if (!Test_InputStalenessAndRailFaults())
    {
        return 1;
    }
    if (!Test_ServiceModesHaveZeroMotionCeiling())
    {
        return 1;
    }
    if (!Test_RebootCannotResumeDisplacedStick())
    {
        return 1;
    }
    if (!Test_TickWraparound())
    {
        return 1;
    }
    if (!Test_DeterministicLongFaultCampaign())
    {
        return 1;
    }

    (void)printf("Phase 5 full-system simulation and fault campaign passed\n");
    return 0;
}
