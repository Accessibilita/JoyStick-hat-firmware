/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#include "joystick_configuration.h"

#include <string.h>

#define JOYSTICK_CONFIG_MAGIC_0                         ((uint8_t)'J')
#define JOYSTICK_CONFIG_MAGIC_1                         ((uint8_t)'S')
#define JOYSTICK_CONFIG_MAGIC_2                         ((uint8_t)'C')
#define JOYSTICK_CONFIG_MAGIC_3                         ((uint8_t)'3')
#define JOYSTICK_CONFIG_PAYLOAD_LENGTH                  (30U)
#define JOYSTICK_CONFIG_CRC_OFFSET                      (60U)

static uint16_t JoystickConfiguration_ReadU16Le(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] |
                      (uint16_t)((uint16_t)data[1] << 8U));
}

static uint32_t JoystickConfiguration_ReadU32Le(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8U) |
           ((uint32_t)data[2] << 16U) |
           ((uint32_t)data[3] << 24U);
}

static void JoystickConfiguration_WriteU16Le(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value & 0x00FFU);
    data[1] = (uint8_t)((value >> 8U) & 0x00FFU);
}

static void JoystickConfiguration_WriteU32Le(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)(value & 0x000000FFUL);
    data[1] = (uint8_t)((value >> 8U) & 0x000000FFUL);
    data[2] = (uint8_t)((value >> 16U) & 0x000000FFUL);
    data[3] = (uint8_t)((value >> 24U) & 0x000000FFUL);
}

static JoystickConfigStatus JoystickConfiguration_ValidateAxis(
    const JoystickAxisCalibration *axis)
{
    uint16_t negative_span;
    uint16_t positive_span;

    if (axis == NULL)
    {
        return JOYSTICK_CONFIG_STATUS_NULL;
    }

    if ((axis->maximum_counts > JOYSTICK_CONFIG_ADC_MAX_COUNTS) ||
        (axis->minimum_counts >= axis->center_counts) ||
        (axis->center_counts >= axis->maximum_counts))
    {
        return JOYSTICK_CONFIG_STATUS_AXIS;
    }

    negative_span = (uint16_t)(axis->center_counts - axis->minimum_counts);
    positive_span = (uint16_t)(axis->maximum_counts - axis->center_counts);

    if ((negative_span < JOYSTICK_CONFIG_MIN_SIDE_SPAN_COUNTS) ||
        (positive_span < JOYSTICK_CONFIG_MIN_SIDE_SPAN_COUNTS) ||
        (axis->deadband_counts == 0U) ||
        (axis->deadband_counts >= negative_span) ||
        (axis->deadband_counts >= positive_span))
    {
        return JOYSTICK_CONFIG_STATUS_AXIS;
    }

    return JOYSTICK_CONFIG_STATUS_OK;
}

JoystickConfigStatus JoystickConfiguration_Validate(
    const JoystickConfigurationImage *image)
{
    JoystickConfigStatus status;

    if (image == NULL)
    {
        return JOYSTICK_CONFIG_STATUS_NULL;
    }

    if (image->generation == 0U)
    {
        return JOYSTICK_CONFIG_STATUS_GENERATION;
    }

    status = JoystickConfiguration_ValidateAxis(&image->configuration.x_axis);
    if (status != JOYSTICK_CONFIG_STATUS_OK)
    {
        return status;
    }

    status = JoystickConfiguration_ValidateAxis(&image->configuration.y_axis);
    if (status != JOYSTICK_CONFIG_STATUS_OK)
    {
        return status;
    }

    if ((image->configuration.maximum_speed_q15 == 0U) ||
        (image->configuration.maximum_speed_q15 > JOYSTICK_CONFIG_Q15_MAX) ||
        (image->configuration.response_curve_q15 > JOYSTICK_CONFIG_Q15_MAX))
    {
        return JOYSTICK_CONFIG_STATUS_SHAPING;
    }

    return JOYSTICK_CONFIG_STATUS_OK;
}

uint32_t JoystickConfiguration_Crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    size_t index;

    if ((data == NULL) && (length != 0U))
    {
        return 0U;
    }

    for (index = 0U; index < length; index++)
    {
        uint8_t bit;

        crc ^= (uint32_t)data[index];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 1UL) != 0UL)
            {
                crc = (crc >> 1U) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc ^ 0xFFFFFFFFUL;
}

JoystickConfigStatus JoystickConfiguration_EncodeRecord(
    const JoystickConfigurationImage *image,
    uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE])
{
    uint32_t crc;
    JoystickConfigStatus status;

    if ((image == NULL) || (record == NULL))
    {
        return JOYSTICK_CONFIG_STATUS_NULL;
    }

    status = JoystickConfiguration_Validate(image);
    if (status != JOYSTICK_CONFIG_STATUS_OK)
    {
        return status;
    }

    (void)memset(record, 0, JOYSTICK_CONFIG_RECORD_SIZE);
    record[0] = JOYSTICK_CONFIG_MAGIC_0;
    record[1] = JOYSTICK_CONFIG_MAGIC_1;
    record[2] = JOYSTICK_CONFIG_MAGIC_2;
    record[3] = JOYSTICK_CONFIG_MAGIC_3;
    record[4] = JOYSTICK_CONFIG_RECORD_VERSION;
    record[5] = JOYSTICK_CONFIG_PAYLOAD_LENGTH;
    JoystickConfiguration_WriteU32Le(&record[8], image->generation);

    JoystickConfiguration_WriteU16Le(&record[12], image->configuration.x_axis.minimum_counts);
    JoystickConfiguration_WriteU16Le(&record[14], image->configuration.x_axis.center_counts);
    JoystickConfiguration_WriteU16Le(&record[16], image->configuration.x_axis.maximum_counts);
    JoystickConfiguration_WriteU16Le(&record[18], image->configuration.x_axis.deadband_counts);
    record[20] = image->configuration.x_axis.inverted ? 1U : 0U;

    JoystickConfiguration_WriteU16Le(&record[22], image->configuration.y_axis.minimum_counts);
    JoystickConfiguration_WriteU16Le(&record[24], image->configuration.y_axis.center_counts);
    JoystickConfiguration_WriteU16Le(&record[26], image->configuration.y_axis.maximum_counts);
    JoystickConfiguration_WriteU16Le(&record[28], image->configuration.y_axis.deadband_counts);
    record[30] = image->configuration.y_axis.inverted ? 1U : 0U;

    JoystickConfiguration_WriteU16Le(&record[32], image->configuration.maximum_speed_q15);
    JoystickConfiguration_WriteU16Le(&record[34], image->configuration.response_curve_q15);
    JoystickConfiguration_WriteU16Le(&record[36], image->configuration.reference_profile);

    crc = JoystickConfiguration_Crc32(record, JOYSTICK_CONFIG_CRC_OFFSET);
    JoystickConfiguration_WriteU32Le(&record[JOYSTICK_CONFIG_CRC_OFFSET], crc);

    return JOYSTICK_CONFIG_STATUS_OK;
}

JoystickConfigStatus JoystickConfiguration_DecodeRecord(
    const uint8_t record[JOYSTICK_CONFIG_RECORD_SIZE],
    JoystickConfigurationImage *image)
{
    JoystickConfigurationImage decoded;
    uint32_t expected_crc;
    uint32_t actual_crc;
    size_t index;

    if ((record == NULL) || (image == NULL))
    {
        return JOYSTICK_CONFIG_STATUS_NULL;
    }

    if ((record[0] != JOYSTICK_CONFIG_MAGIC_0) ||
        (record[1] != JOYSTICK_CONFIG_MAGIC_1) ||
        (record[2] != JOYSTICK_CONFIG_MAGIC_2) ||
        (record[3] != JOYSTICK_CONFIG_MAGIC_3))
    {
        return JOYSTICK_CONFIG_STATUS_MAGIC;
    }

    if (record[4] != JOYSTICK_CONFIG_RECORD_VERSION)
    {
        return JOYSTICK_CONFIG_STATUS_VERSION;
    }

    if (record[5] != JOYSTICK_CONFIG_PAYLOAD_LENGTH)
    {
        return JOYSTICK_CONFIG_STATUS_LENGTH;
    }

    if ((record[6] != 0U) || (record[7] != 0U) ||
        (record[21] != 0U) || (record[31] != 0U) ||
        (record[20] > 1U) || (record[30] > 1U))
    {
        return JOYSTICK_CONFIG_STATUS_RESERVED;
    }

    for (index = 38U; index < JOYSTICK_CONFIG_CRC_OFFSET; index++)
    {
        if (record[index] != 0U)
        {
            return JOYSTICK_CONFIG_STATUS_RESERVED;
        }
    }

    expected_crc = JoystickConfiguration_ReadU32Le(&record[JOYSTICK_CONFIG_CRC_OFFSET]);
    actual_crc = JoystickConfiguration_Crc32(record, JOYSTICK_CONFIG_CRC_OFFSET);
    if (expected_crc != actual_crc)
    {
        return JOYSTICK_CONFIG_STATUS_CRC;
    }

    decoded.generation = JoystickConfiguration_ReadU32Le(&record[8]);
    decoded.configuration.x_axis.minimum_counts = JoystickConfiguration_ReadU16Le(&record[12]);
    decoded.configuration.x_axis.center_counts = JoystickConfiguration_ReadU16Le(&record[14]);
    decoded.configuration.x_axis.maximum_counts = JoystickConfiguration_ReadU16Le(&record[16]);
    decoded.configuration.x_axis.deadband_counts = JoystickConfiguration_ReadU16Le(&record[18]);
    decoded.configuration.x_axis.inverted = record[20] != 0U;
    decoded.configuration.y_axis.minimum_counts = JoystickConfiguration_ReadU16Le(&record[22]);
    decoded.configuration.y_axis.center_counts = JoystickConfiguration_ReadU16Le(&record[24]);
    decoded.configuration.y_axis.maximum_counts = JoystickConfiguration_ReadU16Le(&record[26]);
    decoded.configuration.y_axis.deadband_counts = JoystickConfiguration_ReadU16Le(&record[28]);
    decoded.configuration.y_axis.inverted = record[30] != 0U;
    decoded.configuration.maximum_speed_q15 = JoystickConfiguration_ReadU16Le(&record[32]);
    decoded.configuration.response_curve_q15 = JoystickConfiguration_ReadU16Le(&record[34]);
    decoded.configuration.reference_profile = JoystickConfiguration_ReadU16Le(&record[36]);

    {
        const JoystickConfigStatus validation_status =
            JoystickConfiguration_Validate(&decoded);

        if (validation_status != JOYSTICK_CONFIG_STATUS_OK)
        {
            return validation_status;
        }
    }

    *image = decoded;
    return JOYSTICK_CONFIG_STATUS_OK;
}

JoystickConfigStatus JoystickConfiguration_SelectNewestValid(
    const uint8_t slot_a[JOYSTICK_CONFIG_RECORD_SIZE],
    const uint8_t slot_b[JOYSTICK_CONFIG_RECORD_SIZE],
    JoystickConfigurationImage *image,
    uint8_t *selected_slot)
{
    JoystickConfigurationImage image_a;
    JoystickConfigurationImage image_b;
    JoystickConfigStatus status_a;
    JoystickConfigStatus status_b;

    if ((slot_a == NULL) || (slot_b == NULL) ||
        (image == NULL) || (selected_slot == NULL))
    {
        return JOYSTICK_CONFIG_STATUS_NULL;
    }

    status_a = JoystickConfiguration_DecodeRecord(slot_a, &image_a);
    status_b = JoystickConfiguration_DecodeRecord(slot_b, &image_b);

    if ((status_a != JOYSTICK_CONFIG_STATUS_OK) &&
        (status_b != JOYSTICK_CONFIG_STATUS_OK))
    {
        return JOYSTICK_CONFIG_STATUS_NO_VALID_RECORD;
    }

    if ((status_a == JOYSTICK_CONFIG_STATUS_OK) &&
        ((status_b != JOYSTICK_CONFIG_STATUS_OK) ||
         (image_a.generation >= image_b.generation)))
    {
        *image = image_a;
        *selected_slot = 0U;
    }
    else
    {
        *image = image_b;
        *selected_slot = 1U;
    }

    return JOYSTICK_CONFIG_STATUS_OK;
}

JoystickConfigStatus JoystickConfiguration_PrepareNext(
    const JoystickConfigurationImage *current,
    const JoystickConfiguration *next_configuration,
    JoystickConfigurationImage *next_image)
{
    JoystickConfigurationImage candidate;

    if ((current == NULL) ||
        (next_configuration == NULL) ||
        (next_image == NULL))
    {
        return JOYSTICK_CONFIG_STATUS_NULL;
    }

    if (JoystickConfiguration_Validate(current) != JOYSTICK_CONFIG_STATUS_OK)
    {
        return JOYSTICK_CONFIG_STATUS_GENERATION;
    }

    if (current->generation == UINT32_MAX)
    {
        return JOYSTICK_CONFIG_STATUS_GENERATION;
    }

    candidate.generation = current->generation + 1U;
    candidate.configuration = *next_configuration;

    {
        const JoystickConfigStatus validation_status =
            JoystickConfiguration_Validate(&candidate);

        if (validation_status != JOYSTICK_CONFIG_STATUS_OK)
        {
            return validation_status;
        }
    }

    *next_image = candidate;
    return JOYSTICK_CONFIG_STATUS_OK;
}
