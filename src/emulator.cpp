#include "Emulator.h"
#include "VideoController.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include "ps2Keyboard.h"
#include "z80main.h"
#include "FileSystem.h"
//#include "Emulator/z80snapshot.h"
//#include "Emulator/z80emu/z80emu.h"
#include "keyboard.h"
#include "z80snapshot.h"

uint8_t _buffer16K_1[0x4000];
uint8_t _buffer16K_2[0x4000];

// Spectrum video RAM + border color
// 256x192 pixels (or 32x24 characters)
static SpectrumScreenData _spectrumScreenData;

// Used in saveState / restoreState
static SpectrumScreenData* _savedScreenData = (SpectrumScreenData*)&_buffer16K_2[0x4000 - sizeof(SpectrumScreenData)];

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

static PS2Controller* KeyboardController;

static bool _showingKeyboard;
static bool _helpShown;

/*
static bool _settingDateTime;
static uint32_t _frames;
static char* _newDateTime = (char*)_buffer16K_2;
*/


void startKeyboard()
{
	//Mouse::quickCheckHardware();
	KeyboardController = new PS2Controller();
	KeyboardController->begin(PS2Preset::KeyboardPort0);
	Ps2_Initialize(KeyboardController);
}

void saveState()
{
	*_savedScreenData = _spectrumScreenData;
}

void clearHelp()
{
	DebugScreen.HideCursor();
	DebugScreen.SetAttribute(0x3F10); // white on blue
	DebugScreen.Clear();

	_helpShown = false;
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

	_helpShown = true;
}

void restoreHelp()
{
	if (_helpShown)
	{
		showHelp();
	}
	else
	{
		clearHelp();
	}
}

void restoreState(bool restoreScreen)
{
	if (restoreScreen)
	{
		_spectrumScreenData = *_savedScreenData;
	}

	restoreHelp();
}

bool showKeyboardLoop()
{
	if (!_showingKeyboard)
	{
		return false;
	}

	int32_t scanCode = Ps2_GetScancode();
	if (scanCode == 0 || (scanCode & 0xFF00) != 0x00)
	{
		return true;
	}

	_showingKeyboard = false;
	restoreState(true);
	return false;
}

void showKeyboardSetup()
{
	saveState();
	_showingKeyboard = true;

	DebugScreen.SetAttribute(0x3F10); // white on blue
	DebugScreen.Clear();
	DebugScreen.PrintAlignCenter(2, "Press any key to return");

	MainScreen.ShowScreenshot(spectrumKeyboard);

	_spectrumScreenData.BorderColor = 0; // Black
}

void showTitle(const char* title)
{
	DebugScreen.SetAttribute(0x3F00); // white on black
	DebugScreen.PrintAlignCenter(0, title);
	DebugScreen.SetAttribute(0x3F10); // white on blue
}

void showRegisters()
{
	DebugScreen.SetAttribute(0x3F10); // white on blue
	DebugScreen.Clear();
	showTitle("Registers. ESC - clear");

    char* buf = (char*)_buffer16K_1;

    sprintf(buf, "PC %04x  AF %04x  AF' %04x  I %02x",
        _zxCpu.pc, _zxCpu.registers.word[Z80_AF],
        _zxCpu.alternates[Z80_AF], _zxCpu.i);
    DebugScreen.PrintAlignCenter(2, buf);
    sprintf(buf, "SP %04x  BC %04x  BC' %04x  R %02x",
        _zxCpu.registers.word[Z80_SP], _zxCpu.registers.word[Z80_BC],
        _zxCpu.alternates[Z80_BC], _zxCpu.r);
    DebugScreen.PrintAlignCenter(3, buf);
    sprintf(buf, "IX %04x  DE %04x  DE' %04x  IM %x",
        _zxCpu.registers.word[Z80_IX], _zxCpu.registers.word[Z80_DE],
        _zxCpu.alternates[Z80_DE], _zxCpu.im);
    DebugScreen.PrintAlignCenter(4, buf);
    sprintf(buf, "IY %04x  HL %04x  HL' %04x      ",
        _zxCpu.registers.word[Z80_IY], _zxCpu.registers.word[Z80_HL],
        _zxCpu.alternates[Z80_HL]);
    DebugScreen.PrintAlignCenter(5, buf);
}

void toggleHelp()
{
	if (_helpShown)
	{
		clearHelp();
	}
	else
	{
		showHelp();
	}
}

void showErrorMessage(const char* errorMessage)
{
	DebugScreen.SetAttribute(0x0310); // red on blue
	DebugScreen.PrintAlignCenter(2, errorMessage);
	DebugScreen.SetAttribute(0x3F10); // white on blue
}

union t
{
    int l;
    byte a[4];
    short s[2]; 
};

void EmulatorTaskMain(void *unused)
{
/*
    if (!FFat.begin())
    {
        Serial.println("FFat Mount Failed");
        return;
    }
    FileSystemInitialize(&FFat);
*/

    SPI.begin(14, 2, 12);
    if (!SD.begin(13)) 
    {
        Serial.println("Card Mount Failed");
        return;
    }
    FileSystemInitialize(&SD);

	// Setup
	startKeyboard();
	showHelp();

	zx_setup(&MainScreen);

    ScreenController.StartVideo(RESOLUTION);

	// Loop
	while (true)
	{
		delay(1);
//continue;

		if (showKeyboardLoop())
		{
			continue;
		}

		if (loadSnapshotLoop())
		{
			continue;
		}

		int32_t result = zx_loop();
		switch (result)
		{
		case KEY_ESC:
			showHelp();
			break;

		case KEY_F1:
			toggleHelp();
			break;

		case KEY_F3:
			if (!loadSnapshotSetup("/"))
			{
				showErrorMessage("Error when loading from SD card");
			}
            else
            {
                // stop sound
                _ay3_8912.Stop();
            }
			break;

		case KEY_F5:
			zx_reset();
			showHelp();
			break;

		case KEY_F10:
			showKeyboardSetup();
			break;

		case KEY_F12:
			showRegisters();
			break;
		}	
	}
}
