#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "settings.h"
#include "z80Emulator.h"
#include "VideoController.h"

extern uint8_t _buffer16K_1[];
extern uint8_t _buffer16K_2[];

void EmulatorTaskMain(void *unused);

void showTitle(const char* title);
void saveState();
void restoreState();

#endif /* __EMULATOR_H__ */
