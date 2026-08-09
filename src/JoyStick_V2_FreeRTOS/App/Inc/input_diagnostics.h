/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * Accessibilita JoyStick Interface Firmware
 *
 * Coding standard: GhostPCB firmware rules in docs/CODING_STANDARD.md,
 * informed by MISRA C:2023, CERT C, and JPL/NASA Power of Ten.
 */

#ifndef INPUT_DIAGNOSTICS_H
#define INPUT_DIAGNOSTICS_H

#include "app_types.h"

typedef struct
{
    bool valid;
    bool neutral;
    AppFaultMask faults;
} InputDiagnosticResult;

InputDiagnosticResult InputDiagnostics_Evaluate(
    const AppRawInputSnapshot *input,
    uint32_t now_ms);

#endif /* INPUT_DIAGNOSTICS_H */
