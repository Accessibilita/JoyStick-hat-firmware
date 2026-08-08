#ifndef INPUT_ACQUISITION_H
#define INPUT_ACQUISITION_H

#include <stdbool.h>
#include <stdint.h>

#include "app_types.h"

bool InputAcquisition_Start(void);
bool InputAcquisition_CopyCompletedBatch(AppRawInputSnapshot *snapshot);
void InputAcquisition_OnDmaHalfCompleteFromIsr(void);
void InputAcquisition_OnDmaCompleteFromIsr(void);
uint16_t *InputAcquisition_GetDmaBuffer(void);
uint32_t InputAcquisition_GetDmaBufferLength(void);

#endif /* INPUT_ACQUISITION_H */
