#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "settings.h"
#include "z80Emulator.h"
#include "Screen.h"
#include "SpectrumScreen.h"

#define DEBUG_BAND_HEIGHT (DEBUG_ROWS * 8)
#define SPECTRUM_BAND_HEIGHT (SCREEN_HEIGHT - DEBUG_BAND_HEIGHT)

using namespace Display;

extern Screen DebugScreen;
extern SpectrumScreen MainScreen;
extern uint8_t _buffer16K_1[];
extern uint8_t _buffer16K_2[];

void EmulatorTaskMain(void *unused);

void showTitle(const char* title);
void saveState();
void restoreState(bool restoreScreen);

#endif /* __EMULATOR_H__ */
