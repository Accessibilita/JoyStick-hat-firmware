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
