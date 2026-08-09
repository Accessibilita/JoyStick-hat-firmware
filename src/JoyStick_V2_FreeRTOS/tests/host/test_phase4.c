/*
 * Accessibilita JoyStick Interface Firmware
 *
 * License: No project license is declared in this repository at Phase 4.
 * Do not assume permission to redistribute this project-owned file.
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "configuration_runtime.h"
#include "hmi_model.h"
#include "joystick_configuration.h"
#include "runtime_control.h"

static uint8_t s_slots[2][JOYSTICK_CONFIG_RECORD_SIZE];
static bool s_slot_present[2];

static bool Test_ReadSlot(
    uint8_t slot_index,
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE])
{
    if ((slot_index >= 2U) || (!s_slot_present[slot_index]))
    {
        return false;
    }

    (void)memcpy(record, s_slots[slot_index], JOYSTICK_CONFIG_RECORD_SIZE);
    return true;
}

static bool Test_Check(bool condition, const char *message)
{
    if (!condition)
    {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return false;
    }
    return true;
}

static JoystickConfigurationImage Test_MakeValidConfiguration(void)
{
    JoystickConfigurationImage image = {0};

    image.generation = 7U;
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

static bool Test_ConfigurationRuntime(void)
{
    ConfigurationRuntimeState state;
    JoystickConfigurationImage image = Test_MakeValidConfiguration();

    (void)memset(s_slots, 0, sizeof(s_slots));
    s_slot_present[0] = true;
    s_slot_present[1] = false;

    if (!Test_Check(
            JoystickConfiguration_EncodeRecord(&image, s_slots[0]) ==
                JOYSTICK_CONFIG_STATUS_OK,
            "encode valid configuration"))
    {
        return false;
    }

    ConfigurationRuntime_Init(&state);
    if (!Test_Check(
            ConfigurationRuntime_Load(&state, Test_ReadSlot),
            "load one valid slot"))
    {
        return false;
    }

    return Test_Check(
        state.valid && (state.active_image.generation == 7U) &&
            (state.selected_slot == 0U),
        "selected valid configuration image");
}

static bool Test_HmiDebounce(void)
{
    HmiModelContext context;
    AppHmiState state = {0};

    HmiModel_Init(&context);
    HmiModel_Step(&context, 0x000FU, 0x07U, 0x07U, 0U, &state);

    /* A short glitch must not become stable state. */
    HmiModel_Step(&context, 0x000EU, 0x07U, 0x07U, 10U, &state);
    HmiModel_Step(&context, 0x000FU, 0x07U, 0x07U, 20U, &state);
    if (!Test_Check(state.buttons_active_low == 0x000FU, "reject button glitch"))
    {
        return false;
    }

    HmiModel_Step(&context, 0x000EU, 0x07U, 0x07U, 100U, &state);
    HmiModel_Step(
        &context,
        0x000EU,
        0x07U,
        0x07U,
        100U + HMI_MODEL_DEBOUNCE_MS,
        &state);

    if (!Test_Check(state.buttons_active_low == 0x000EU, "accept debounced button"))
    {
        return false;
    }

    return Test_Check(
        (!state.enable_request) && (!state.control_mapping_valid),
        "target HMI semantics remain fail-closed");
}

static bool Test_RuntimeIntegration(void)
{
    RuntimeControlContext context;
    RuntimeControlInput input;
    RuntimeControlOutput output;
    ConfigurationRuntimeState configuration;
    InputDiagnosticResult diagnostics = {0};
    AppRawInputSnapshot raw = {0};
    AppHmiState hmi = {0};
    AppMotorLinkStatus link = {0};

    ConfigurationRuntime_Init(&configuration);
    configuration.active_image = Test_MakeValidConfiguration();
    configuration.status = JOYSTICK_CONFIG_STATUS_OK;
    configuration.selected_slot = 0U;
    configuration.valid = true;

    raw.sequence = 42U;
    raw.captured_at_ms = 100U;
    raw.joystick_x_counts = 2048U;
    raw.joystick_y_counts = 2048U;
    raw.power_good = true;

    diagnostics.valid = true;
    diagnostics.neutral = true;
    diagnostics.faults = APP_FAULT_NONE;

    /* Host tests may inject a validated HMI mapping that target code cannot yet claim. */
    hmi.sequence = 3U;
    hmi.sampled_at_ms = 100U;
    hmi.requested_mode = APP_OPERATING_MODE_REDUCED_SPEED;
    hmi.maximum_speed_q15 = 8192U;
    hmi.enable_request = false;
    hmi.control_mapping_valid = true;

    link.link_valid = true;
    link.last_valid_packet_ms = 100U;
    link.remote_drive_ready = true;
    link.brakes_confirmed = true;
    link.remote_faults = APP_FAULT_NONE;

    RuntimeControl_Init(&context, 0U);
    input.raw_input = &raw;
    input.input_diagnostics = &diagnostics;
    input.configuration = &configuration;
    input.hmi = &hmi;
    input.motor_link = &link;
    input.now_ms = 100U;
    input.mandatory_tasks_healthy = true;

    if (!Test_Check(RuntimeControl_Step(&context, &input, &output), "neutral runtime step"))
    {
        return false;
    }
    /*
     * SafetyState_Init() starts with fail-closed bootstrap faults.  A valid
     * current iteration must not feed those stale startup bits into mode
     * selection; otherwise the first qualified profile request becomes FAULT.
     */
    if (!Test_Check(
            output.active_mode == APP_OPERATING_MODE_REDUCED_SPEED,
            "neutral-qualified operating-mode transition"))
    {
        return false;
    }

    raw.sequence++;
    raw.captured_at_ms = 105U;
    raw.joystick_y_counts = 3000U;
    diagnostics.neutral = false;
    hmi.enable_request = true;
    input.now_ms = 105U;

    if (!Test_Check(RuntimeControl_Step(&context, &input, &output), "moving runtime step"))
    {
        return false;
    }
    if (!Test_Check(output.joystick_processed, "joystick entered runtime pipeline"))
    {
        return false;
    }
    if (!Test_Check(output.requested_command.forward_q15 > 0, "forward request generated"))
    {
        return false;
    }
    if (!Test_Check(
            output.requested_command.maximum_speed_q15 == 8192U,
            "HMI speed ceiling cannot exceed effective configuration"))
    {
        return false;
    }

    /* The most important Phase-4 acceptance condition. */
    return Test_Check(
        (!output.authorization.physical_command.drive_authorized) &&
            (output.authorization.physical_command.forward_q15 == 0) &&
            (output.authorization.physical_command.turn_q15 == 0),
        "physical command remains inhibited");
}

int main(void)
{
    if (!Test_ConfigurationRuntime())
    {
        return 1;
    }
    if (!Test_HmiDebounce())
    {
        return 1;
    }
    if (!Test_RuntimeIntegration())
    {
        return 1;
    }

    (void)printf("Phase 4 host tests passed\n");
    return 0;
}
