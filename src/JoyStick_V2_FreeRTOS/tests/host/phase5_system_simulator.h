/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#ifndef PHASE5_SYSTEM_SIMULATOR_H
#define PHASE5_SYSTEM_SIMULATOR_H

#include <stdbool.h>
#include <stdint.h>

#include "configuration_runtime.h"
#include "fake_motor_controller.h"
#include "input_diagnostics.h"
#include "motor_link_state.h"
#include "runtime_control.h"

#define PHASE5_SIM_STEP_MS                    (5U)
#define PHASE5_SIM_LINK_PERIOD_MS             (10U)
#define PHASE5_SIM_MAX_RUN_MS                 (5000U)
#define PHASE5_SIM_HOST_ADDRESS               (0x01U)
#define PHASE5_SIM_MOTOR_ADDRESS              (0x10U)
#define PHASE5_SIM_INITIAL_COMMAND_SESSION     (0x13572468UL)
#define PHASE5_SIM_CONTROLLER_SESSION          (0x24681357UL)

typedef struct
{
    RuntimeControlContext runtime;
    ConfigurationRuntimeState configuration;
    AppHmiState hmi;
    AppRawInputSnapshot raw_input;
    InputDiagnosticResult input_diagnostics;
    MotorLinkContext motor_link;
    AppMotorLinkStatus motor_link_status;
    FakeMotorController motor_controller;
    RuntimeControlOutput output;
    MotorProtocolDecodeResult last_decode_result;
    MotorLinkProcessResult last_link_result;
    uint32_t now_ms;
    uint32_t command_session_id;
    uint32_t link_command_sequence;
    uint32_t link_exchange_count;
    uint32_t link_period_accumulator_ms;
    bool mandatory_tasks_healthy;
    bool freeze_input_timestamp;
    bool last_link_response_received;
} Phase5SystemSimulator;

/* Start a deterministic software-only system with valid synthetic host inputs. */
bool Phase5SystemSimulator_Init(Phase5SystemSimulator *simulator);

/* Reset firmware/session state while preserving external joystick/HMI conditions. */
bool Phase5SystemSimulator_ResetFirmware(Phase5SystemSimulator *simulator);

/* Advance one fixed 5 ms control interval and the 10 ms modeled link schedule. */
bool Phase5SystemSimulator_Step(Phase5SystemSimulator *simulator);

/* Run a bounded duration; intended only for deterministic host campaigns. */
bool Phase5SystemSimulator_RunForMs(
    Phase5SystemSimulator *simulator,
    uint32_t duration_ms);

void Phase5SystemSimulator_SetJoystick(
    Phase5SystemSimulator *simulator,
    uint16_t x_counts,
    uint16_t y_counts);
void Phase5SystemSimulator_SetEnable(
    Phase5SystemSimulator *simulator,
    bool enable_request);
void Phase5SystemSimulator_SetMode(
    Phase5SystemSimulator *simulator,
    AppOperatingMode requested_mode,
    uint16_t maximum_speed_q15);
void Phase5SystemSimulator_SetConfigurationValid(
    Phase5SystemSimulator *simulator,
    bool valid);
void Phase5SystemSimulator_SetMandatoryTasksHealthy(
    Phase5SystemSimulator *simulator,
    bool healthy);
void Phase5SystemSimulator_SetPowerGood(
    Phase5SystemSimulator *simulator,
    bool power_good);
void Phase5SystemSimulator_SetDmaOverrun(
    Phase5SystemSimulator *simulator,
    bool overrun_detected);
void Phase5SystemSimulator_FreezeInputTimestamp(
    Phase5SystemSimulator *simulator,
    bool freeze);
void Phase5SystemSimulator_SetMotorMode(
    Phase5SystemSimulator *simulator,
    FakeMotorControllerMode mode);

#endif /* PHASE5_SYSTEM_SIMULATOR_H */
