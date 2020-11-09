#include "Emulator.h"
#include "VideoController.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include "ps2Input.h"
#include "z80main.h"
#include "FileSystem.h"
#include "keyboard.h"
#include "z80snapshot.h"
#include "z80Memory.h"

uint8_t _buffer16K_1[0x4000];
uint8_t _buffer16K_2[0x4000];

// Used in saveState / restoreState
static SpectrumScreenData* _savedScreenData = (SpectrumScreenData*)&_buffer16K_2[0x4000 - sizeof(SpectrumScreenData)];

// Spectrum screen band
static VideoSettings _spectrumVideoSettings {
	32, 24, 
	SpectrumMemory.MainScreenData.Pixels, 
    SpectrumMemory.MainScreenData.Attributes, 
    &SpectrumMemory.BorderColor
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

static PS2Controller* InputController;

static bool _showingKeyboard;
static bool _helpShown;

void startKeyboard()
{
	Mouse::quickCheckHardware();
	InputController = new PS2Controller();
	InputController->begin(PS2Preset::KeyboardPort0_MousePort1);
	Ps2_Initialize(InputController);
}

void saveState()
{
	*_savedScreenData = SpectrumMemory.MainScreenData;
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

    int y = 0;
	DebugScreen.PrintAt(0, y++, "F1  - show / hide help");
#ifdef SDCARD
	DebugScreen.PrintAt(0, y++, "F2  - save snapshot to SD card");
	DebugScreen.PrintAt(0, y++, "F3  - load snapshot from SD card");
#else
	DebugScreen.PrintAt(0, y++, "F3  - load snapshot from flash");
#endif
	DebugScreen.PrintAt(0, y++, "F5  - reset");
	DebugScreen.PrintAt(0, y++, "F10 - show keyboard layout");
	DebugScreen.PrintAt(0, y++, "F12 - show registers");

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
		SpectrumMemory.MainScreenData = *_savedScreenData;
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

	SpectrumMemory.BorderColor = 0; // Black
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

void EmulatorTaskMain(void *unused)
{

#ifdef SDCARD
    SPI.begin(14, 2, 12);
    if (!SD.begin(13)) 
    {
        Serial.println("Card Mount Failed");
        return;
    }
    FileSystemInitialize(&SD);
#else
    if (!FFat.begin())
    {
        Serial.println("FFat Mount Failed");
        return;
    }
    FileSystemInitialize(&FFat);
#endif

	// Setup
	startKeyboard();
	showHelp();

	zx_setup(&MainScreen);

    ScreenController.StartVideo(RESOLUTION);

	// Loop
	while (true)
	{
		delay(1);

		if (showKeyboardLoop())
		{
			continue;
		}

		if (loadSnapshotLoop())
		{
			continue;
		}

        if (saveSnapshotLoop())
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

#ifdef SDCARD
	case KEY_F2:
		if (!saveSnapshotSetup())
		{
#ifdef SDCARD
			showErrorMessage("Cannot initialize SD card");
#else
			showErrorMessage("Cannot initialize flash file system");
#endif
		}
		break;
#endif

		case KEY_F3:
			if (!loadSnapshotSetup("/"))
			{
#ifdef SDCARD
				showErrorMessage("Error when loading from SD card");
#else
				showErrorMessage("Error when loading from flash");
#endif
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
