#ifndef JOYSTICK_CONFIGURATION_H
#define JOYSTICK_CONFIGURATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define JOYSTICK_CONFIG_RECORD_SIZE                    (64U)
#define JOYSTICK_CONFIG_RECORD_VERSION                 (1U)
#define JOYSTICK_CONFIG_REFERENCE_GENERIC              (0U)
#define JOYSTICK_CONFIG_REFERENCE_CHC_104B_M2          (1U)
#define JOYSTICK_CONFIG_Q15_MAX                        (32767U)
#define JOYSTICK_CONFIG_ADC_MAX_COUNTS                 (4095U)
#define JOYSTICK_CONFIG_MIN_SIDE_SPAN_COUNTS           (256U)

typedef enum
{
    JOYSTICK_CONFIG_STATUS_OK = 0,
    JOYSTICK_CONFIG_STATUS_NULL,
    JOYSTICK_CONFIG_STATUS_MAGIC,
    JOYSTICK_CONFIG_STATUS_VERSION,
    JOYSTICK_CONFIG_STATUS_LENGTH,
    JOYSTICK_CONFIG_STATUS_CRC,
    JOYSTICK_CONFIG_STATUS_GENERATION,
    JOYSTICK_CONFIG_STATUS_AXIS,
    JOYSTICK_CONFIG_STATUS_SHAPING,
    JOYSTICK_CONFIG_STATUS_RESERVED,
    JOYSTICK_CONFIG_STATUS_NO_VALID_RECORD
} JoystickConfigStatus;

typedef struct
{
    uint16_t minimum_counts;
    uint16_t center_counts;
    uint16_t maximum_counts;
    uint16_t deadband_counts;
    bool inverted;
} JoystickAxisCalibration;

typedef struct
{
    JoystickAxisCalibration x_axis;
    JoystickAxisCalibration y_axis;
    uint16_t maximum_speed_q15;
    uint16_t response_curve_q15;
    uint16_t reference_profile;
} JoystickConfiguration;

typedef struct
{
    uint32_t generation;
    JoystickConfiguration configuration;
} JoystickConfigurationImage;

JoystickConfigStatus JoystickConfiguration_Validate(
    const JoystickConfigurationImage *image);

uint32_t JoystickConfiguration_Crc32(
    const uint8_t *data,
    size_t length);

JoystickConfigStatus JoystickConfiguration_EncodeRecord(
    const JoystickConfigurationImage *image,
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE]);

JoystickConfigStatus JoystickConfiguration_DecodeRecord(
    const uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE],
    JoystickConfigurationImage *image);

JoystickConfigStatus JoystickConfiguration_SelectNewestValid(
    const uint8_t slot_a[JOYSTICK_CONFIG_RECORD_SIZE],
    const uint8_t slot_b[JOYSTICK_CONFIG_RECORD_SIZE],
    JoystickConfigurationImage *image,
    uint8_t *selected_slot);

JoystickConfigStatus JoystickConfiguration_PrepareNext(
    const JoystickConfigurationImage *current,
    const JoystickConfiguration *next_configuration,
    JoystickConfigurationImage *next_image);

#endif /* JOYSTICK_CONFIGURATION_H */
