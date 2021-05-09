#include "Emulator.h"
#include "VideoController.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "SD.h"
#include "z80Environment.h"
#include "ps2Input.h"
#include "z80main.h"
#include "FileSystem.h"
#include "keyboard.h"
#include "z80snapshot.h"
#include "main_ROM.h"
#include "ScreenArea.h"

using namespace fabgl;

// Temporary buffers
uint8_t _buffer16K_1[0x4000];
uint8_t _buffer16K_2[0x4000];

// Screen
static SpectrumScreenData _spectrumScreenData;
static VideoController _screen(&_spectrumScreenData);
VideoController* Screen = &_screen;
ScreenArea DebugScreen(Screen, 
	1, SPECTRUM_WIDTH_WITH_BORDER, 
	SPECTRUM_HEIGHT_WITH_BORDER + 2, SCREEN_HEIGHT - (SPECTRUM_HEIGHT_WITH_BORDER + 2));
ScreenArea DebugScreenRight(Screen, 
	SPECTRUM_WIDTH_WITH_BORDER + 2, SCREEN_WIDTH - (SPECTRUM_WIDTH_WITH_BORDER + 2), 
	1, SCREEN_HEIGHT - 2);

// Z80State
Z80Environment Environment(Screen);

// Data for the "debug" screen
static bool _showingKeyboard;
static bool _helpShown;

static PS2Controller* InputController;

void startKeyboard()
{
	Mouse::quickCheckHardware();
	InputController = new PS2Controller();
	InputController->begin(PS2Preset::KeyboardPort0_MousePort1);
	Ps2_Initialize(InputController);
}

void saveState()
{
	//Screen->ShowScreenshot();

	Screen->_mode = 1;
}

void clearHelp()
{
	DebugScreen.HideCursor();
	DebugScreen.SetAttribute(DEBUG_BAND_COLORS);
	DebugScreen.Clear();

	_helpShown = false;
}

void showHelp()
{
	DebugScreen.HideCursor();
	DebugScreen.SetAttribute(DEBUG_BAND_COLORS);
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
	Screen->_mode = 2;
	if (restoreScreen)
	{
		// not used currently
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
/*
	DebugScreen.SetAttribute(DEBUG_BAND_COLORS);
	DebugScreen.Clear();
	DebugScreen.PrintAlignCenter(2, "Press any key to return");
*/
	//Screen->ShowScreenshot(spectrumKeyboard, 0);
}

void showTitle(const char* title)
{
	DebugScreen.SetAttribute(0x3F00); // white on black
	DebugScreen.PrintAlignCenter(0, title);
	DebugScreen.SetAttribute(DEBUG_BAND_COLORS);
}

void showRegisters()
{
	DebugScreen.SetAttribute(DEBUG_BAND_COLORS);
	DebugScreen.Clear();
	showTitle("Registers. ESC - clear");

    char* buf = (char*)_buffer16K_1;

    sprintf(buf, "PC %04x  AF %04x  AF' %04x  I %02x",
        (uint16_t)Z80cpu.PC, (uint16_t)Z80cpu.AF, (uint16_t)Z80cpu.AFx, (uint16_t)Z80cpu.I);
    DebugScreen.PrintAlignCenter(2, buf);
    sprintf(buf, "SP %04x  BC %04x  BC' %04x  R %02x",
        (uint16_t)Z80cpu.SP, (uint16_t)Z80cpu.BC, (uint16_t)Z80cpu.BCx, (uint16_t)Z80cpu.R);
    DebugScreen.PrintAlignCenter(3, buf);
    sprintf(buf, "IX %04x  DE %04x  DE' %04x  IM %x",
        (uint16_t)Z80cpu.IX, (uint16_t)Z80cpu.DE, (uint16_t)Z80cpu.DEx, (uint16_t)Z80cpu.IM);
    DebugScreen.PrintAlignCenter(4, buf);
    sprintf(buf, "IY %04x  HL %04x  HL' %04x      ",
        (uint16_t)Z80cpu.IY, (uint16_t)Z80cpu.HL, (uint16_t)Z80cpu.HLx);
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
	DebugScreen.SetAttribute(DEBUG_BAND_COLORS);
}

bool ReadRomFromFiles()
{
    if ((uint8_t*)*Environment.Rom[0] == (uint8_t*)ROM)
    {
        *Environment.Rom[0] = (uint8_t*)malloc(0x4000);
#ifdef ZX128K
        *Environment.Rom[1] = (uint8_t*)malloc(0x4000);
#endif
    }

    bool result;
#ifdef ZX128K
    result = ReadFromFile("/roms/128-0.rom", (uint8_t*)*Environment.Rom[0], 0x4000);
    if (result)
    {
        result = ReadFromFile("/roms/128-1.rom", (uint8_t*)*Environment.Rom[1], 0x4000);
    }
#else
    result = ReadFromFile("/roms/48.rom", (uint8_t*)*Environment.Rom[0], 0x4000);
#endif

    if (!result)
    {
        free((uint8_t*)*Environment.Rom[0]);
#ifdef ZX128K
        free((uint8_t*)*Environment.Rom[1]);
#endif
        *Environment.Rom[0] = (uint8_t*)ROM;
        *Environment.Rom[1] = (uint8_t*)ROM;
    }

    return result;
}

void ResetSystem()
{
    if (*Environment.Rom[0] != (uint8_t*)ROM)
    {
        free((uint8_t*)*Environment.Rom[0]);
#ifdef ZX128K
        free((uint8_t*)*Environment.Rom[1]);
#endif
        *Environment.Rom[0] = (uint8_t*)ROM;
        *Environment.Rom[1] = (uint8_t*)ROM;
    }

    ReadRomFromFiles();
    zx_reset();
}

void EmulatorTaskMain(void *unused)
{
#ifdef SDCARD
    SPI.begin(14, 2, 12);
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
	zx_setup(&Environment);
	Environment.Initialize();

    Serial.write("before ReadRomFromFiles()\r\n");
    ReadRomFromFiles();
    Serial.write("after ReadRomFromFiles()\r\n");

	Screen->Start(RESOLUTION);

	showHelp();

	// Loop
	while (true)
	{
		vTaskDelay(1); // important to avoid task watchdog timeouts

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
		if (!saveSnapshotSetup("/"))
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
            ResetSystem();
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
