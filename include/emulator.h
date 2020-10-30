#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "Screen.h"
#include "SpectrumScreen.h"
//#include "fatfs.h"

using namespace Display;

/*
#define RESOLUTION QVGA_320x240_60Hz
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define DEBUG_COLUMNS 40
#define DEBUG_ROWS 5
*/

#define RESOLUTION VGA_400x300_60Hz
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 300
#define DEBUG_COLUMNS 50
#define DEBUG_ROWS 8

#define DEBUG_BAND_HEIGHT (DEBUG_ROWS * 8)
#define SPECTRUM_BAND_HEIGHT (SCREEN_HEIGHT - DEBUG_BAND_HEIGHT)

typedef struct
{
	uint8_t  Pixels[32 * 8 * 24];
	uint16_t Attributes[32 * 24];
	uint8_t  BorderColor;
} SpectrumScreenData;

extern Screen DebugScreen;
extern SpectrumScreen MainScreen;
extern uint8_t _buffer16K_1[0x4000];
extern uint8_t _buffer16K_2[0x4000];

void EmulatorTaskMain(void *unused);

void showTitle(const char* title);
void saveState();
void restoreState(bool restoreScreen);

#endif /* __EMULATOR_H__ */
