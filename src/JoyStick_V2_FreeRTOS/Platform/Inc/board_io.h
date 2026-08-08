#ifndef BOARD_IO_H
#define BOARD_IO_H

#include <stdbool.h>
#include <stdint.h>

void BoardIo_ForceRs485SafeDisabled(void);
void BoardIo_ForceLedHudBlanked(void);
uint16_t BoardIo_ReadButtonsActiveLow(void);
uint8_t BoardIo_ReadRotary1ActiveLow(void);
uint8_t BoardIo_ReadRotary2ActiveLow(void);
bool BoardIo_IsPowerGood(void);

#endif /* BOARD_IO_H */
