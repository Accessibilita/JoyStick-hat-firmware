/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "app_build_config.h"
#include "health_monitor.h"
#include "input_diagnostics.h"
#include "safety_state.h"

static AppRawInputSnapshot MakeNominalInput(uint32_t captured_at_ms)
{
    const AppRawInputSnapshot input =
    {
        .sequence = 1U,
        .captured_at_ms = captured_at_ms,
        .joystick_x_counts = 2048U,
        .joystick_y_counts = 2048U,
        .buttons_active_low = 0U,
        .rotary_1_active_low = 0U,
        .rotary_2_active_low = 0U,
        .power_good = true,
        .dma_overrun_detected = false
    };

    return input;
}

static AppSafetyObservation MakeNominalObservation(uint32_t now_ms)
{
    const AppSafetyObservation observation =
    {
        .configuration_valid = true,
        .input_valid = true,
        .input_neutral = true,
        .link_valid = true,
        .link_fresh = true,
        .enable_request = false,
        .mandatory_tasks_healthy = true,
        .now_ms = now_ms,
        .observed_faults = APP_FAULT_NONE
    };

    return observation;
}

static void Test_InputDiagnosticsNominalAndFaults(void)
{
    AppRawInputSnapshot input = MakeNominalInput(100U);
    InputDiagnosticResult result = InputDiagnostics_Evaluate(&input, 105U);

    assert(result.valid);
    assert(result.neutral);

    input.joystick_x_counts = 4095U;
    result = InputDiagnostics_Evaluate(&input, 105U);
    assert(!result.valid);
    assert((result.faults & APP_FAULT_JOYSTICK_X_RANGE) != 0U);

    input = MakeNominalInput(100U);
    input.power_good = false;
    result = InputDiagnostics_Evaluate(&input, 105U);
    assert(!result.valid);
    assert((result.faults & APP_FAULT_POWER_GOOD_LOST) != 0U);

    input = MakeNominalInput(100U);
    input.dma_overrun_detected = true;
    result = InputDiagnostics_Evaluate(&input, 105U);
    assert(!result.valid);
    assert((result.faults & APP_FAULT_ADC_OVERRUN) != 0U);

    input = MakeNominalInput(100U);
    result = InputDiagnostics_Evaluate(&input, 200U);
    assert(!result.valid);
    assert((result.faults & APP_FAULT_INPUT_STALE) != 0U);

    result = InputDiagnostics_Evaluate(NULL, 0U);
    assert(!result.valid);
    assert((result.faults & APP_FAULT_INTERNAL_INVARIANT) != 0U);
}

static void Test_InputAgeHandlesTickWrap(void)
{
    AppRawInputSnapshot input = MakeNominalInput(UINT32_MAX - 5U);
    const InputDiagnosticResult result = InputDiagnostics_Evaluate(&input, 3U);

    /* Unsigned subtraction gives an elapsed age of nine milliseconds. */
    assert(result.valid);
    assert(result.neutral);
}

static void AdvanceToReady(
    AppSafetyContext *context,
    AppSafetyObservation *observation)
{
    SafetyState_Init(context, 0U);

    observation->now_ms = 0U;
    SafetyState_Step(context, observation); /* BOOT -> CONFIGURATION_CHECK */
    observation->now_ms = 1U;
    SafetyState_Step(context, observation); /* CONFIGURATION_CHECK -> WAIT_LINK */
    observation->now_ms = 2U;
    SafetyState_Step(context, observation); /* WAIT_LINK -> WAIT_NEUTRAL */
    observation->now_ms = 3U;
    SafetyState_Step(context, observation); /* begin neutral qualification */
    observation->now_ms = 3U + APP_NEUTRAL_QUALIFICATION_MS;
    SafetyState_Step(context, observation); /* WAIT_NEUTRAL -> READY */

    assert(context->state == APP_SAFETY_READY);
}

static void Test_LogicalDriveStateRequiresQualifiedEnable(void)
{
    AppSafetyContext context;
    AppSafetyObservation observation = MakeNominalObservation(0U);

    AdvanceToReady(&context, &observation);
    observation.enable_request = true;
    observation.now_ms++;
    SafetyState_Step(&context, &observation);

    /*
     * Later phases use DRIVE_AUTHORIZED as a logical safety decision.  The
     * physical output lock is enforced below this state machine by the
     * authorization/transport boundary and is tested in Phase 2+ campaigns.
     */
    assert(SafetyState_IsDriveAuthorized(&context));
    assert(context.state == APP_SAFETY_DRIVE_AUTHORIZED);

    observation.enable_request = false;
    observation.now_ms++;
    SafetyState_Step(&context, &observation);
    assert(!SafetyState_IsDriveAuthorized(&context));
    assert(context.state == APP_SAFETY_WAIT_FOR_NEUTRAL);
}

static void Test_NeutralQualificationRestartsAfterMotion(void)
{
    AppSafetyContext context;
    AppSafetyObservation observation = MakeNominalObservation(0U);

    SafetyState_Init(&context, 0U);
    SafetyState_Step(&context, &observation);
    observation.now_ms = 1U;
    SafetyState_Step(&context, &observation);
    observation.now_ms = 2U;
    SafetyState_Step(&context, &observation);
    observation.now_ms = 3U;
    SafetyState_Step(&context, &observation);

    observation.input_neutral = false;
    observation.now_ms = 250U;
    SafetyState_Step(&context, &observation);
    assert(context.state == APP_SAFETY_WAIT_FOR_NEUTRAL);
    assert(!context.neutral_timer_running);

    observation.input_neutral = true;
    observation.now_ms = 251U;
    SafetyState_Step(&context, &observation);
    observation.now_ms = 251U + APP_NEUTRAL_QUALIFICATION_MS - 1U;
    SafetyState_Step(&context, &observation);
    assert(context.state == APP_SAFETY_WAIT_FOR_NEUTRAL);

    observation.now_ms++;
    SafetyState_Step(&context, &observation);
    assert(context.state == APP_SAFETY_READY);
}

static void Test_CriticalFaultLatches(void)
{
    AppSafetyContext context;
    AppSafetyObservation observation = MakeNominalObservation(0U);

    SafetyState_Init(&context, 0U);
    observation.observed_faults = APP_FAULT_INTERNAL_INVARIANT;
    SafetyState_Step(&context, &observation);

    assert(context.state == APP_SAFETY_LATCHED_FAULT);
    assert(!SafetyState_IsDriveAuthorized(&context));
}

static void Test_HealthMonitor(void)
{
    AppHealthRecord record;

    HealthMonitor_Init(0U);
    assert(HealthMonitor_AreMandatoryChannelsHealthy(500U));

    HealthMonitor_RecordProgress(APP_HEALTH_SAFETY_TASK, 1000U);
    HealthMonitor_RecordProgress(APP_HEALTH_LINK_TASK, 1000U);
    HealthMonitor_RecordProgress(APP_HEALTH_HMI_TASK, 1000U);
    assert(HealthMonitor_AreMandatoryChannelsHealthy(1010U));
    assert(!HealthMonitor_AreMandatoryChannelsHealthy(1200U));

    assert(HealthMonitor_CopyRecord(APP_HEALTH_LINK_TASK, &record));
    assert(record.progress_counter == 1U);
    assert(record.last_progress_ms == 1000U);
    assert(record.mandatory);
    assert(!HealthMonitor_CopyRecord(APP_HEALTH_COUNT, &record));
    assert(!HealthMonitor_CopyRecord(APP_HEALTH_LINK_TASK, NULL));
}

int main(void)
{
    Test_InputDiagnosticsNominalAndFaults();
    Test_InputAgeHandlesTickWrap();
    Test_LogicalDriveStateRequiresQualifiedEnable();
    Test_NeutralQualificationRestartsAfterMotion();
    Test_CriticalFaultLatches();
    Test_HealthMonitor();

    puts("Phase 1 host tests passed");
    return 0;
}
