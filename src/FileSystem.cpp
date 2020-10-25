#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "Ff.h"

#include "FileSystem.h"
#include "Emulator.h"
#include "ps2Keyboard.h"
#include "z80snapshot.h"

using namespace zx;

#define FILE_COLUMNS 3
#define FILE_COLUMNWIDTH (DEBUG_COLUMNS / FILE_COLUMNS)

typedef TCHAR FileName[FF_MAX_LFN + 1];

static FileName* _fileNames = (FileName*) _buffer16K_2;
static int16_t _selectedFile = 0;
static int16_t _fileCount;
static bool _loadingSnapshot = false;
static bool _savingSnapshot = false;
static char* _snapshotName = (char*)&_buffer16K_1[2];

static fs::FS* _fileSystem;


void FileSystemInitialize(fs::FS* fileSystem)
{
    _fileSystem = fileSystem;
}

FRESULT mount()
{
	return FR_OK;
}

void unmount()
{
}

void GetFileCoord(uint8_t fileIndex, uint8_t* x, uint8_t* y)
{
	*x = fileIndex / (DEBUG_ROWS - 1) * (FILE_COLUMNWIDTH + 1);
	*y = 1 + fileIndex % (DEBUG_ROWS - 1);
}

TCHAR* TruncateFileName(TCHAR* fileName)
{
	int maxLength = DEBUG_COLUMNS / FILE_COLUMNS - 1;
	TCHAR* result = (TCHAR*) _buffer16K_1;
	strncpy(result, fileName, maxLength);

	result[maxLength - 1] = '\0';
	TCHAR* extension = strrchr(result, '.');
	if (extension != nullptr)
	{
		*extension = '\0';
	}

	return result;
}

void noScreenshot()
{
	MainScreen.Clear();
	MainScreen.SetAttribute(0x0310); // red on blue
	MainScreen.PrintAlignCenter(11, "Error reading selected file");
	MainScreen.SetAttribute(0x3F10); // white on blue
}

void SetSelection(uint8_t selectedFile)
{
	if (_fileCount == 0)
	{
		return;
	}

	_selectedFile = selectedFile;

	uint8_t x, y;
	GetFileCoord(selectedFile, &x, &y);
	DebugScreen.PrintAt(x, y, "\x10"); // ►

	FRESULT fr;

	// Show screenshot for the selected file
	fr = mount();
	if (fr == FR_OK)
	{
		File file;
		bool scrFileFound = false;

		TCHAR* fileName = _fileNames[selectedFile];

		// Try to open file with the same name and .SCR extension
		TCHAR* scrFileName = (TCHAR*) _buffer16K_1;
		strncpy(scrFileName, fileName, FF_MAX_LFN + 1);
		TCHAR* extension = strrchr(scrFileName, '.');
		if (extension != nullptr)
		{
			strncpy(extension, ".scr", 4);
			file = _fileSystem->open(scrFileName, FILE_READ);
			if (file)
			{
				if (!LoadScreenshot(file, _buffer16K_1))
				{
					noScreenshot();
				}
				file.close();
				scrFileFound = true;
			}
		}

		if (!scrFileFound)
		{
			file = _fileSystem->open(fileName, FILE_READ);
			if (file)
			{
				if (!LoadScreenFromZ80Snapshot(file, _buffer16K_1))
				{
					noScreenshot();
				}
				file.close();
			}
		}

		unmount();
	}
}

void loadSnapshot(const TCHAR* fileName)
{
	FRESULT fr = mount();
	if (fr == FR_OK)
	{
		File file = _fileSystem->open(fileName, FILE_READ);
		LoadZ80Snapshot(file, _buffer16K_1, _buffer16K_2);
		file.close();

		unmount();
	}
}

bool saveSnapshot(const TCHAR* fileName)
{
	bool result = false;
	FRESULT fr = mount();
	if (fr == FR_OK)
	{
		File file = _fileSystem->open(fileName, FILE_WRITE);
/*
		if (fr == FR_EXIST)
		{
			fr = f_unlink(fileName);
			if (fr == FR_OK)
			{
				fr = f_open(&file, fileName, FA_WRITE | FA_CREATE_NEW);
			}
		}
*/
		if (fr == FR_OK)
		{
			result = SaveZ80Snapshot(file, _buffer16K_1, _buffer16K_2);
			file.close();
		}

		unmount();
	}

	return result;
}

static int fileCompare(const void* a, const void* b)
{
	TCHAR* file1 = (TCHAR*)_buffer16K_1;
	for (int i = 0; i <= FF_MAX_LFN; i++){
		file1[i] = tolower(((TCHAR*)a)[i]);
	}

	TCHAR* file2 = (TCHAR*)&_buffer16K_1[FF_MAX_LFN + 2];
	for (int i = 0; i <= FF_MAX_LFN; i++){
		file2[i] = tolower(((TCHAR*)b)[i]);
	}

	return strncmp(file1, file2, FF_MAX_LFN + 1);
}

bool saveSnapshotSetup()
{
	DebugScreen.SetAttribute(0x3F10); // white on blue
	DebugScreen.Clear();

	showTitle("Save snapshot. ENTER, ESC, BS");

	FRESULT fr = mount();
	if (fr != FR_OK)
	{
		return false;
	}

	// Unmount file system
	unmount();

	DebugScreen.PrintAt(0, 2, "Enter file name:");
	DebugScreen.SetCursorPosition(0, 3);
	DebugScreen.ShowCursor();
	memset(_snapshotName, 0, FF_MAX_LFN + 1);
	_savingSnapshot = true;

	return true;
}

bool saveSnapshotLoop()
{
	if (!_savingSnapshot)
	{
		return false;
	}

	int32_t scanCode = Ps2_GetScancode();
	if (scanCode == 0 || (scanCode & 0xFF00) == 0xF000)
	{
		return true;
	}

	scanCode = ((scanCode & 0xFF0000) >> 8 | (scanCode & 0xFF));
	uint8_t x = DebugScreen._cursor_x;
	switch (scanCode)
	{
	case KEY_BACKSPACE:
		if (DebugScreen._cursor_x > 0)
		{
			DebugScreen.PrintAt(DebugScreen._cursor_x - 1, DebugScreen._cursor_y, " ");
			DebugScreen.SetCursorPosition(DebugScreen._cursor_x - 1, DebugScreen._cursor_y);
			_snapshotName[DebugScreen._cursor_x] = '\0';
		}
		break;

	case KEY_ENTER:
	case KEY_KP_ENTER:
		DebugScreen.HideCursor();
		DebugScreen.PrintAt(0, 5, "Saving...                  ");
		strcat(_snapshotName,".z80");
		if (saveSnapshot(_snapshotName))
		{
			_savingSnapshot = false;
			restoreState(false);
			return false;
		}
		else
		{
			DebugScreen.SetAttribute(0x0310); // red on blue
			DebugScreen.PrintAt(0, 5, "Error saving file");
			DebugScreen.SetAttribute(0x3F10); // white on blue
			DebugScreen.SetCursorPosition(x, 3);
			DebugScreen.ShowCursor();
		}
		break;

	case KEY_ESC:
		_savingSnapshot = false;
		restoreState(false);
		return false;

	default:
		char character = Ps2_ConvertScancode(scanCode);
		if (DebugScreen._cursor_x < FILE_COLUMNWIDTH && character != '\0'
			&& character != '\\' && character != '/' && character != ':'
			&& character != '*' && character != '?' && character != '"'
			&& character != '<' && character != '>' && character != '|')
		{
			char* text = (char*)_buffer16K_1;
			text[0] = character;
			_snapshotName[DebugScreen._cursor_x] = character;
			text[1] = '\0';
			DebugScreen.Print(text);
		}
		break;
	}

	return true;
}

bool loadSnapshotSetup(const char* path)
{
	saveState();

	DebugScreen.SetAttribute(0x3F10); // white on blue
	DebugScreen.Clear();
	*MainScreen.Settings.BorderColor = 0x10;

	showTitle("Load snapshot. ENTER, ESC, \x18, \x19, \x1A, \x1B"); // ↑, ↓, →, ←

	FRESULT fr = mount();
	if (fr != FR_OK)
	{
		return false;
	}

	uint8_t maxFileCount = (DEBUG_ROWS - 1) * FILE_COLUMNS;
	_fileCount = 0;
	bool result = true;

    File root = _fileSystem->open(path);
	if (root)
	{
        int fileIndex = 0;
		while (fileIndex < maxFileCount)
		{
			File file = root.openNextFile();
			if (!file)
			{
				result = false;
				break;
			}

            // TODO *.z80
            if (file.isDirectory())
            {
                continue;
            }

			strncpy(_fileNames[fileIndex], file.name(), FF_MAX_LFN + 1);
			_fileCount++;
            fileIndex++;
		}
	}
	else
	{
		result = false;
	}

	// Sort files alphabetically
	if (_fileCount > 0)
	{
		qsort(_fileNames, _fileCount, FF_MAX_LFN + 1, fileCompare);
	}

	for (int y = 1; y < DEBUG_ROWS; y++)
	{
		DebugScreen.PrintAt(FILE_COLUMNWIDTH, y, "\xB3"); // │
		DebugScreen.PrintAt(FILE_COLUMNWIDTH * 2 + 1, y, "\xB3"); // │
	}

	uint8_t x, y;
	for (int fileIndex = 0; fileIndex < _fileCount; fileIndex++)
	{
		GetFileCoord(fileIndex, &x, &y);
		DebugScreen.PrintAt(x + 1, y, TruncateFileName(_fileNames[fileIndex]));
	}

	SetSelection(_selectedFile);

	// Unmount file system
	unmount();

	if (result)
	{
		_loadingSnapshot = true;
	}

	return result;
}

bool loadSnapshotLoop()
{
	if (!_loadingSnapshot)
	{
		return false;
	}

	int32_t scanCode = Ps2_GetScancode();
	if (scanCode == 0 || (scanCode & 0xFF00) != 0xF000)
	{
		return true;
	}

	uint8_t previousSelection = _selectedFile;

	scanCode = ((scanCode & 0xFF0000) >> 8 | (scanCode & 0xFF));
	switch (scanCode)
	{
	case KEY_UPARROW:
		if (_selectedFile > 0)
		{
			_selectedFile--;
		}
		break;

	case KEY_DOWNARROW:
		if (_selectedFile < _fileCount - 1)
		{
			_selectedFile++;
		}
		break;

	case KEY_LEFTARROW:
		if (_selectedFile >= DEBUG_ROWS - 1)
		{
			_selectedFile -= DEBUG_ROWS - 1;
		}
		break;

	case KEY_RIGHTARROW:
		if (_selectedFile + DEBUG_ROWS <= _fileCount)
		{
			_selectedFile += DEBUG_ROWS - 1;
		}
		break;

	case KEY_ENTER:
	case KEY_KP_ENTER:
		loadSnapshot(_fileNames[_selectedFile]);
		_loadingSnapshot = false;
		restoreState(false);
		return false;

	case KEY_ESC:
		_loadingSnapshot = false;
		restoreState(true);
		return false;
	}

	if (previousSelection == _selectedFile)
	{
		return true;
	}

	uint8_t x, y;
	GetFileCoord(previousSelection, &x, &y);
	DebugScreen.PrintAt(x, y, " ");

	SetSelection(_selectedFile);

	return true;
}
