#include "Emulator.h"
#include "VideoController.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "ps2Keyboard.h"
//#include "Emulator/z80main.h"
//#include "Emulator/z80snapshot.h"
//#include "Emulator/z80emu/z80emu.h"
#include "keyboard.h"

uint8_t _buffer16K_1[0x4000];
uint8_t _buffer16K_2[0x4000];

// Spectrum video RAM + border color
// 256x192 pixels (or 32x24 characters)
static SpectrumScreenData _spectrumScreenData;

// Used in saveState / restoreState
//static SpectrumScreenData* _savedScreenData = (SpectrumScreenData*)&_buffer16K_2[0x4000 - sizeof(SpectrumScreenData)];

// Spectrum screen band
static VideoSettings _spectrumVideoSettings {
	32, 24, 
	_spectrumScreenData.Pixels, _spectrumScreenData.Attributes, &_spectrumScreenData.BorderColor
};
SpectrumScreen MainScreen(_spectrumVideoSettings, 0, SPECTRUM_BAND_HEIGHT);

// Debug screen video RAM
// DEBUG_COLUMNS x DEBUG_ROWS characters
static uint8_t  _debugPixels[52 * 8 * DEBUG_ROWS]; // number of text columns must be divisible by 4
static uint16_t _debugAttributes[52 * DEBUG_ROWS]; // number of text columns must be divisible by 4
static uint8_t  _debugBorderColor;

// Debug band
static VideoSettings _videoSettings {
	DEBUG_COLUMNS, DEBUG_ROWS, 
	_debugPixels, _debugAttributes,	&_debugBorderColor
};
Screen DebugScreen(_videoSettings, SPECTRUM_BAND_HEIGHT, DEBUG_BAND_HEIGHT);

VideoController ScreenController(&MainScreen, &DebugScreen);

/*
static bool _showingKeyboard;
static bool _settingDateTime;
static uint32_t _frames;
static char* _newDateTime = (char*)_buffer16K_2;
static bool _helpShown;
*/

void startVideo()
{
	MainScreen.Clear();
	DebugScreen.Clear();

    ScreenController.StartVideo(QVGA_320x240_60Hz);
}

void showHelp()
{
	DebugScreen.HideCursor();
	DebugScreen.SetAttribute(0x3F10); // white on blue
	DebugScreen.Clear();

	DebugScreen.PrintAt(0, 0, "F1  - show / hide help");
	DebugScreen.PrintAt(0, 1, "F3  - load snapshot from flash");
	DebugScreen.PrintAt(0, 2, "F5  - reset");
	DebugScreen.PrintAt(0, 3, "F10 - show keyboard layout");
	DebugScreen.PrintAt(0, 4, "F12 - show registers");

	//_helpShown = true;
}