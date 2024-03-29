#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "esp_vfs_fat.h"
#include "esp_log.h"

#include "settings.h"
#include "FileSystem.h"
#include "Emulator.h"
#include "ps2Input.h"
#include "z80main.h"
#include "z80snapshot.h"
#include "ScreenArea.h"
#include "errorReadingFile.h"
#include "File.h"

using namespace zx;

#define DEBUG_COLUMNS 42
#define DEBUG_ROWS 58
#define FILE_COLUMNS 3
#define FILE_COLUMNWIDTH (DEBUG_COLUMNS / FILE_COLUMNS)
#define MAX_LFN 90
#define FORE_COLOR (DEBUG_BAND_COLORS >> 8)
#define BACK_COLOR (DEBUG_BAND_COLORS & 0xFF)

extern VideoController* Screen;
extern ScreenArea DebugScreen;

typedef TCHAR FileName[MAX_LFN + 1];

static FileName* _fileNames = (FileName*)_buffer16K_2;
static int16_t _selectedFile = 0;
static int16_t _fileCount;
static bool _loadingSnapshot = false;
static bool _savingSnapshot = false;
static char* _snapshotName = ((char*)_buffer16K_1) + MAX_LFN;

static char _rootFolder[MAX_LFN];
static int _rootFolderLength;

static esp_vfs_fat_sdmmc_mount_config_t _mount_config;
static sdmmc_host_t _host = SDSPI_HOST_DEFAULT();
static sdspi_device_config_t _slot_config;
static sdmmc_card_t* _card = nullptr;

void FileSystemInitialize()
{
    ESP_LOGI(TAG, "Initializing SD card");

    _mount_config = {
        .format_if_mount_failed = false,
        .max_files = 1,
        .allocation_unit_size = 16 * 1024
    };

    spi_host_device_t hostID = spi_host_device_t(_host.slot);

    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = PIN_NUM_MOSI;
    bus_cfg.miso_io_num = PIN_NUM_MISO;
    bus_cfg.sclk_io_num = PIN_NUM_CLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
    }

    _slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    _slot_config.gpio_cs = PIN_NUM_CS;
    _slot_config.host_id = hostID;
}

static FRESULT mount()
{
	return esp_vfs_fat_sdspi_mount(SDCARD_PATH, &_host, &_slot_config, &_mount_config, &_card) == ESP_OK ? FR_OK : FR_NOT_READY;
}

static void unmount()
{
	if (_card != nullptr)
	{
	    esp_vfs_fat_sdcard_unmount(SDCARD_PATH, _card);
		_card = nullptr;
	}
}

static void GetFileCoord(uint16_t fileIndex, uint8_t* x, uint8_t* y)
{
	*x = fileIndex / (DEBUG_ROWS - 1) * (FILE_COLUMNWIDTH + 1);
	*y = 1 + fileIndex % (DEBUG_ROWS - 1);
}

static TCHAR* GetFileName(TCHAR* fileName)
{
	TCHAR* result = (TCHAR*)_buffer16K_1;
    strncpy(result, _rootFolder, _rootFolderLength);
	strncpy(result + _rootFolderLength, fileName, MAX_LFN);

    return result;
}

static TCHAR* FileExtension(TCHAR* fileName)
{
	TCHAR* result = (TCHAR*)_buffer16K_1;
	memcpy(result, fileName, MAX_LFN);
	result[MAX_LFN] = '\0';

	result[MAX_LFN - 1] = '\0';
	char* extension = strrchr(result, '.');
    if (extension != nullptr)
    {
        for(int i = 0; extension[i]; i++)
        {
            extension[i] = tolower(extension[i]);
        }
    }
    else
    {
    	result[0] = '\0';
        extension = result;
    }

    return extension;
}

static TCHAR* TruncateFileName(TCHAR* fileName)
{
	int maxLength = FILE_COLUMNWIDTH + 1;
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

static void noScreenshot()
{
	// Error reading selected file
	Screen->ShowScreenshot(errorReadingFile, 0);
}

static void SetSelection(uint8_t selectedFile)
{
	if (_fileCount == 0)
	{
		return;
	}

	_selectedFile = selectedFile;

	uint8_t x, y;
	GetFileCoord(selectedFile, &x, &y);
	for (uint8_t i = x; i < x + FILE_COLUMNWIDTH; i++)
	{
		DebugScreen.SetAttribute(i, y, BACK_COLOR, FORE_COLOR); // inverse
	}

	FRESULT fr;

	// Show screenshot for the selected file
	fr = mount();
	if (fr == FR_OK)
	{
		File file;
		bool scrFileFound = false;

		TCHAR* fileName = GetFileName(_fileNames[selectedFile]);

		// Try to open file with the same name and .SCR extension
		TCHAR* scrFileName = (TCHAR*)&_buffer16K_1[_rootFolderLength + MAX_LFN + 1];
		strncpy(scrFileName, fileName, MAX_LFN + 1);
		TCHAR* extension = strrchr(scrFileName, '.');
		if (extension != nullptr)
		{
			strncpy(extension, ".scr", 5);
			file.open(scrFileName, ios_base::in);
			if (file.is_open())
			{
				if (!LoadScreenshot(&file, _buffer16K_1))
				{
					noScreenshot();
				}
				file.close();
				scrFileFound = true;
			}
		}

		if (!scrFileFound)
		{
			file.open(fileName, ios_base::in);
			if (file.is_open())
			{
				if (!LoadScreenFromZ80Snapshot(&file, _buffer16K_1))
				{
					noScreenshot();
				}
				file.close();
                _ay3_8912.Clear();
			}
		}

		unmount();
	}
}

static void loadSnapshot(const TCHAR* fileName)
{
	FRESULT fr = mount();
	if (fr == FR_OK)
	{
		File file;
		file.open(fileName, ios_base::in);
		if (file.is_open())
		{
			LoadZ80Snapshot(&file, _buffer16K_1, _buffer16K_2);
			file.close();
		}

		unmount();
	}
}

static bool saveSnapshot(const TCHAR* fileName)
{
	bool result = false;
	FRESULT fr = mount();
	if (fr == FR_OK)
	{
		File file;
		file.open(fileName, ios_base::out);
		if (file.is_open())
		{
			result = SaveZ80Snapshot(&file, _buffer16K_1, _buffer16K_2);
			file.close();
		}

		unmount();
	}

	return result;
}

static int fileCompare(const void* a, const void* b)
{
	TCHAR* file1 = (TCHAR*)_buffer16K_1;
	for (int i = 0; i <= MAX_LFN; i++){
		file1[i] = tolower(((TCHAR*)a)[i]);
	}

	TCHAR* file2 = (TCHAR*)&_buffer16K_1[MAX_LFN + 2];
	for (int i = 0; i <= MAX_LFN; i++){
		file2[i] = tolower(((TCHAR*)b)[i]);
	}

	return strncmp(file1, file2, MAX_LFN + 1);
}

bool saveSnapshotSetup(const char* path)
{
	string rootFolder = string(SDCARD_PATH);
	rootFolder.append(path);
	strcpy(_rootFolder, rootFolder.c_str());
    _rootFolderLength = rootFolder.length();

	DebugScreen.SetPrintAttribute(FORE_COLOR, BACK_COLOR);
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
	memset(_snapshotName, 0, MAX_LFN + 1);
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
	uint8_t x = DebugScreen.getX();
    TCHAR* fileName;
	switch (scanCode)
	{
	case KEY_BACKSPACE:
		if (x > 0)
		{
			DebugScreen.PrintAt(x - 1, DebugScreen.getY(), " ");
			DebugScreen.SetCursorPosition(x - 1, DebugScreen.getY());
			_snapshotName[x] = '\0';
		}
		break;

	case KEY_ENTER:
	case KEY_KP_ENTER:
		DebugScreen.HideCursor();
		DebugScreen.PrintAt(0, 5, "Saving...                  ");
        fileName = GetFileName(_snapshotName);
		strcat(fileName,".z80");
		if (saveSnapshot(fileName))
		{
			_savingSnapshot = false;
			restoreState();
			return false;
		}
		else
		{
			DebugScreen.SetPrintAttribute(0x0310); // red on blue
			DebugScreen.PrintAt(0, 5, "Error saving file");
			DebugScreen.SetPrintAttribute(0x3F10); // white on blue
			DebugScreen.SetCursorPosition(x, 3);
			DebugScreen.ShowCursor();
		}
		break;

	case KEY_ESC:
		_savingSnapshot = false;
		restoreState();
		return false;

	default:
		char character = Ps2_ConvertScancode(scanCode);
		if (x < FILE_COLUMNWIDTH && character != '\0'
			&& character != '\\' && character != '/' && character != ':'
			&& character != '*' && character != '?' && character != '"'
			&& character != '<' && character != '>' && character != '|')
		{
			char* text = (char*)_buffer16K_1;
			text[0] = character;
			_snapshotName[x] = character;
			text[1] = '\0';
			DebugScreen.Print(text);
		}
		break;
	}

	return true;
}

bool loadSnapshotSetup(const char* path)
{
	string rootFolder = string(SDCARD_PATH);
	rootFolder.append(path);
	strcpy(_rootFolder, rootFolder.c_str());
    _rootFolderLength = rootFolder.length();

	saveState();

	DebugScreen.SetPrintAttribute(0x3F10); // white on blue
	DebugScreen.Clear();

	showTitle("Loading files, please wait...");

	FRESULT fr = mount();
	if (fr != FR_OK)
	{
		return false;
	}

	uint8_t maxFileCount = (DEBUG_ROWS - 1) * FILE_COLUMNS;
	_fileCount = 0;
	bool result = true;

	FF_DIR folder;
	FILINFO fileInfo;
	fr = f_opendir(&folder, (const TCHAR*) "/");
	if (fr == FR_OK)
	{
        int fileIndex = 0;
		while (fileIndex < maxFileCount)
		{
			fr = f_readdir(&folder, &fileInfo);
			if (fr != FR_OK || fileInfo.fname[0] == 0)
			{
				result = _fileCount > 0;
				break;
			}

            if (fileInfo.fattrib & AM_DIR)
            {
                continue;
            }

            // *.z80
            if (strncmp(FileExtension((TCHAR*)fileInfo.fname), ".z80", 4) != 0)
            {
                continue;
            }

			memcpy(_fileNames[fileIndex], fileInfo.fname, MAX_LFN + 1);

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
		qsort(_fileNames, _fileCount, MAX_LFN + 1, fileCompare);

        for (int y = 1; y < DEBUG_ROWS; y++)
        {
            DebugScreen.PrintAt(FILE_COLUMNWIDTH, y, "\xB3"); // │
            DebugScreen.PrintAt(FILE_COLUMNWIDTH * 2 + 1, y, "\xB3"); // │
        }

        uint8_t x, y;
        for (int fileIndex = 0; fileIndex < _fileCount; fileIndex++)
        {
            GetFileCoord(fileIndex, &x, &y);
            DebugScreen.PrintAt(x, y, TruncateFileName(_fileNames[fileIndex]));
        }

        SetSelection(_selectedFile);	
    }

	// Unmount file system
	unmount();

	if (result)
	{
		_loadingSnapshot = true;
	}

	showTitle("Load snapshot. ENTER, ESC, \x18, \x19, \x1A, \x1B"); // ↑, ↓, →, ←

	return result;
}

bool loadSnapshotLoop()
{
	if (!_loadingSnapshot)
	{
		return false;
	}

	int32_t scanCode = Ps2_GetScancode();
	if (scanCode == 0 || (scanCode & 0xFF00) == 0xF000)
	{
		return true;
	}

	uint8_t previousSelection = _selectedFile;

	scanCode &= 0xFFFF;
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
		loadSnapshot(GetFileName(_fileNames[_selectedFile]));
		_loadingSnapshot = false;
		restoreState();
		return false;

	case KEY_ESC:
		_loadingSnapshot = false;
		restoreState();
		return false;
	}

	if (previousSelection == _selectedFile)
	{
		return true;
	}

	uint8_t x, y;
	GetFileCoord(previousSelection, &x, &y);
	for (uint8_t i = x; i < x + FILE_COLUMNWIDTH; i++)
	{
		DebugScreen.SetAttribute(i, y, FORE_COLOR, BACK_COLOR);
	}

	SetSelection(_selectedFile);

	return true;
}

bool ReadFromFile(const char* fileName, uint8_t* buffer, size_t size)
{
	FRESULT fr = mount();
	if (fr != FR_OK)
	{
		return false;
	}

	string rootFolder = string(SDCARD_PATH);
	strcpy(_rootFolder, rootFolder.c_str());
    _rootFolderLength = rootFolder.length();
	TCHAR* filePath = GetFileName((TCHAR*)fileName);

    bool result = false;
    File file;
	file.open(filePath, ios_base::in);
    if (file.is_open())
    {
        size_t bytesRead = file.read(buffer, size);
        result = (bytesRead == size);
        file.close();
    }

	unmount();

    return result;
}