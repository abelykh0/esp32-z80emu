#include <string.h>
#include <stdio.h>
#include "ff.h"

#include "z80snapshot.h"
#include "z80main.h"
#include "z80Emulator.h"
#include "z80Environment.h"

/*
 Offset  Length  Description
 ---------------------------
 0       1       A register
 1       1       F register
 2       2       BC register pair (LSB, i.e. C, first)
 4       2       HL register pair
 6       2       PC (version 1) or 0 to signal a version 2 or 3
 8       2       Stack pointer
 10      1       Interrupt register
 11      1       Refresh register (Bit 7 is not significant!)
 12      1       Bit 0  : Bit 7 of the R-register
                 Bit 1-3: Border color
                 Bit 4-7: No meaning
                 If byte 12 is 255, it has to be regarded as being 1
 13      2       DE register pair
 15      2       BC' register pair
 17      2       DE' register pair
 19      2       HL' register pair
 21      1       A' register
 22      1       F' register
 23      2       IY register (Again LSB first)
 25      2       IX register
 27      1       Interrupt flipflop, 0=DI, otherwise EI
 28      1       IFF2
 29      1       Bit 0-1: Interrupt mode (0, 1 or 2)
                 Bit 2  : 1=Issue 2 emulation
                 Bit 3  : 1=Double interrupt frequency
                 Bit 4-5: unused
                 Bit 6-7: 0=Cursor/Protek/AGF joystick
                 1=Kempston joystick
                 2=Sinclair 2 Left joystick (or user
                 defined, for version 3 .z80 files)
                 3=Sinclair 2 Right joystick
 30 0x1E 2       Length of additional header block (see below)
                 23 for version 2
                 54 or 55 for version 3
 ===========================
 32 0x20 2       Program counter
 34 0x22 1       Hardware mode, 0 for ZX Spectrum 48K
 35 0x23 1       If in 128 mode, contains last OUT to 0x7ffd
 ...

 Hereafter a number of memory blocks follow, each containing the compressed data of a 16K block.
 The compression method is very simple: it replaces repetitions of at least five equal bytes by
 a four-byte code ED ED xx yy, which stands for "byte yy repeated xx times".
 Only sequences of length at least 5 are coded. The exception is sequences consisting of ED's;
 if they are encountered, even two ED's are encoded into ED ED 02 ED.
 Finally, every byte directly following a single ED is not taken into a block, for example ED 6*00
 is not encoded into ED ED ED 06 00 but into ED 00 ED ED 05 00.

 The structure of a memory block is:
 Byte    Length  Description
 ---------------------------
 0       2       Length of compressed data (without this 3-byte header)
                 If length=0xffff, data is 16384 bytes long and not compressed
 2       1       Page number, for ZX Spectrum 48K:
                 8: 4000-7fff
                 4: 8000-bfff
                 5: c000-ffff
 3       [0]     Data

 */

extern Z80Environment Environment;

struct FileHeader
{
	uint8_t A;
	uint8_t F;
	uint16_t BC;
	uint16_t HL;
	uint16_t PC;
	uint16_t SP;
	uint8_t InterruptRegister;
	uint8_t RefreshRegister;
	uint8_t Flags1;
	uint16_t DE;
	uint16_t BC_Dash;
	uint16_t DE_Dash;
	uint16_t HL_Dash;
	uint8_t A_Dash;
	uint8_t F_Dash;
	uint16_t IY;
	uint16_t IX;
	uint8_t InterruptFlipFlop;
	uint8_t IFF2;
	uint8_t Flags2;
	uint16_t AdditionalBlockLength;
	uint16_t PCVersion2;
	uint8_t HardwareMode;
	uint8_t PagingState;
}__attribute__((packed));

uint16_t DecompressPage(uint8_t *page, uint16_t pageLength, bool isCompressed,
		uint16_t maxSize, uint8_t* destMemory);
uint16_t CompressPage(uint8_t* page, uint8_t* destMemory);
void ReadState(FileHeader* header);
void SaveState(FileHeader* header);
void GetPageInfo(uint8_t* buffer, bool is128Mode, uint8_t pagingState, int8_t* pageNumber, uint16_t* pageSize);
void ShowScreenshot(uint8_t* buffer);

bool zx::SaveZ80Snapshot(File file, uint8_t buffer1[0x4000], uint8_t buffer2[0x4000])
{
	// Note: this requires little-endian processor
	FileHeader* header = (FileHeader*)buffer1;
	SaveState(header);

	memset(&buffer1[sizeof(FileHeader)], 0, header->AdditionalBlockLength - 4);

    uint8_t pageCount;
    uint8_t pagesToSave[8];
    if (header->HardwareMode == 0)
    {
        // Save as 48K snaphot

        pageCount = 3;
        pagesToSave[0] = 5;
        pagesToSave[1] = 2;
        pagesToSave[2] = Environment.MemoryState.RamBank;
    }
    else
    {
        // Save as 128K snaphot

        pageCount = 8;
    	for (int i = 0; i < pageCount; i++)
        {
            pagesToSave[i] = i;
        }
    }

	UINT bytesWritten;
	UINT bytesToWrite = sizeof(FileHeader) + header->AdditionalBlockLength - 4;
	bytesWritten = file.write(buffer1, bytesToWrite);
	if (bytesWritten != bytesToWrite)
	{
		return false;
	}

	for (int i = 0; i < pageCount; i++)
	{
        uint8_t pageNumber = pagesToSave[i];
		uint8_t* buffer = buffer2;

        switch (pageNumber)
        {
            case 0:
            case 2:
            case 5:
                Environment.Ram[pageNumber]->ToBuffer(buffer);
                break;
#ifdef ZX128K
            case 1:
            case 3:
            case 4:
            case 6:
            case 7:
                Environment.Ram[pageNumber]->ToBuffer(buffer);
                break;
#endif
        }

		uint16_t pageSize = CompressPage(buffer, buffer1);

		buffer = (uint8_t*)buffer2;
		if (pageSize == 0x4000)
		{
			*buffer = 0xFF;
			buffer++;
			*buffer = 0xFF;
		}
		else
		{
			*buffer = pageSize & 0xFF;
			buffer++;
			*buffer = (pageSize & 0xFF00) >> 8;
		}
		buffer++;

        if (pageCount == 3)
        {
            switch (pageNumber)
            {
            case 2:
    		    *buffer = 4;
                break;
            case 5:
    		    *buffer = 8;
                break;
            default:
    		    *buffer = 5;
                break;
            }
        }
        else
        {
		    *buffer = pageNumber + 3;
        }

		bytesWritten = file.write(buffer2, 3);
		if (bytesWritten != 3)
		{
			return false;
		}

		buffer = buffer1;

        bytesToWrite = pageSize;
        bytesWritten = file.write(buffer, bytesToWrite);
        if (bytesWritten != bytesToWrite)
        {
            return false;
        }
	}

	return true;
}

bool zx::LoadZ80Snapshot(File file, uint8_t buffer1[0x4000],
		uint8_t buffer2[0x4000])
{
	size_t bytesRead;
	UINT bytesToRead;

	bytesToRead = 30;
	bytesRead = file.read(buffer1, bytesToRead);
	if (bytesRead != bytesToRead)
	{
		return false;
	}

	// Note: this requires little-endian processor
	FileHeader* header = (FileHeader*)buffer1;

    bool is128Mode;
    uint8_t pagingState;
    bool isVersion1;
    if (header->PC != 0)
    {
        // version 1
        is128Mode = false;
        pagingState = 0;
        isVersion1 = true;

    	ReadState(header);
    }
    else
    {
        bytesToRead = 6;
        bytesRead = file.read(&buffer1[30], bytesToRead);
        if (bytesRead != bytesToRead)
        {
            return false;
        }

        if (header->AdditionalBlockLength == 23)
        {
            // version 2
            is128Mode = (header->HardwareMode >= 3);
        }
        else if (header->AdditionalBlockLength == 54 || header->AdditionalBlockLength == 55)
        {
            // version 3
            is128Mode = (header->HardwareMode >= 4);
        }
        else
        {
            // Invalid
            return false;
        }

        pagingState = header->PagingState;

    	ReadState(header);

        bytesToRead = header->AdditionalBlockLength - 4 + 3;
        bytesRead = file.read(buffer1, bytesToRead);
        if (bytesRead != bytesToRead)
        {
            return false;
        }
        isVersion1 = false;
    }

    if (is128Mode)
    {
        Environment.MemoryState.Bits = header->PagingState;
    }
    else
    {
        Environment.MemoryState.Bits = 0;
        Environment.MemoryState.RomSelect = 1;
        Environment.MemoryState.PagingLock = 1;
    }

    bool isCompressed;
    if (isVersion1)
    {
        isCompressed = (header->Flags1 & 0x20) != 0;

        uint8_t* buffer = buffer1;
        int bytesToRead = 0x4000;
        for (int pageIndex = 0; pageIndex < 3; pageIndex++)
        {
            uint8_t* memory= buffer2;

            bytesRead = file.read(buffer, bytesToRead);
            if (!isCompressed && bytesRead != bytesToRead)
            {
                return false;
            }
            buffer += bytesRead;

            uint16_t usedBytes = DecompressPage(buffer1, 0x4000, isCompressed, 0x4000, memory);

            if (isCompressed)
            {
                uint16_t unusedBytes = 0x4000 - usedBytes; // part of next page(s)
                bytesToRead = usedBytes;
                for (int i = 0; i < unusedBytes; i++)
                {
                    buffer1[i] = buffer1[i + usedBytes];
                }
                buffer = &buffer1[unusedBytes];
            }
            else
            {
                buffer = buffer1;
                bytesToRead = 0x4000;
            }

            switch (pageIndex)
            {
                case 0:
                    Environment.Ram[5]->FromBuffer(memory);
                    break;
                case 1:
                    Environment.Ram[2]->FromBuffer(memory);
                    break;
                case 2:
                    Environment.Ram[0]->FromBuffer(memory);
                    break;
            }
        }
    }
    else
    {
        // Get pageSize and pageNumber
        uint16_t pageSize;
        int8_t pageNumber;
        GetPageInfo(&buffer1[bytesToRead - 3], is128Mode, pagingState, &pageNumber, &pageSize);

        do
        {
            isCompressed = (pageSize != 0xFFFF);
            if (!isCompressed)
            {
                pageSize = 0x4000;
            }

            uint8_t* memory;
            switch (pageNumber)
            {
                case 0:
                case 2:
                case 5:
                    memory = buffer2;
                    break;
#ifdef ZX128K
                case 1:
                case 3:
                case 4:
                case 6:
                case 7:
                    memory = buffer2;
                    break;
#endif
                default:
                    memory = nullptr;
                    break;
            }

            if (memory != nullptr)
            {
                // Read page into tempBuffer
                uint8_t* buffer = buffer1;
                bytesToRead = pageSize;
                bytesRead = file.read(buffer, bytesToRead);
                if (bytesRead != bytesToRead)
                {
                    return false;
                }

                DecompressPage(buffer1, pageSize, isCompressed, 0, memory);
                Environment.Ram[pageNumber]->FromBuffer(memory);
            }
            else
            {
                // Move forward without reading
                bool readResult = file.seek(file.position() + pageSize);
                if (readResult != true)
                {
                    return false;
                }
            }

            bytesRead = file.read(buffer1, 3);
            if (bytesRead <= 0)
            {
                return false;
            }

            if (bytesRead == 3)
            {
                GetPageInfo(buffer1, is128Mode, pagingState, &pageNumber, &pageSize);
            }
            else
            {
                pageSize = 0;
            }

        } while (pageSize > 0);
    }

	return true;
}

bool zx::LoadScreenFromZ80Snapshot(File file, uint8_t buffer1[0x4000])
{
	size_t bytesRead;
	UINT bytesToRead;

	bytesToRead = 30;
	bytesRead = file.read(buffer1, bytesToRead);
	if (bytesRead != bytesToRead)
	{
		return false;
	}

	// Note: this requires little-endian processor
	FileHeader* header = (FileHeader*)buffer1;

    bool isCompressed;
    if (header->PC != 0)
    {
        // version 1

        isCompressed = (header->Flags1 & 0x20) != 0;
        int bytesToRead = 0x1B00;
        bytesRead = file.read(buffer1, bytesToRead);
        if (!isCompressed && bytesRead != bytesToRead)
        {
            return false;
        }

        uint8_t* buffer2 = &buffer1[0x2000];
        DecompressPage(buffer1, 0x1B00, isCompressed, 0x1B00, buffer2);
        ShowScreenshot(buffer2);
    }
    else
    {
        bool is128Mode;
        uint8_t pagingState;

        bytesToRead = 6;
        bytesRead = file.read(&buffer1[30], bytesToRead);
        if (bytesRead != bytesToRead)
        {
            return false;
        }

        if (header->AdditionalBlockLength == 23)
        {
            // version 2
            is128Mode = (header->HardwareMode >= 3);
        }
        else if (header->AdditionalBlockLength == 54 || header->AdditionalBlockLength == 55)
        {
            // version 3
            is128Mode = (header->HardwareMode >= 4);
        }
        else
        {
            // Invalid
            return false;
        }

        pagingState = header->PagingState;

        bytesToRead = header->AdditionalBlockLength - 4 + 3;
        bytesRead = file.read(buffer1, bytesToRead);
        if (bytesRead != bytesToRead)
        {
            return false;
        }

        // Get pageSize and pageNumber
        uint16_t pageSize;
        int8_t pageNumber;
        GetPageInfo(&buffer1[bytesToRead - 3], is128Mode, pagingState, &pageNumber, &pageSize);

        do
        {
            isCompressed = (pageSize != 0xFFFF);
            if (!isCompressed)
            {
                pageSize = 0x4000;
            }

            if (pageNumber == 5)
            {
                // This page contains screenshoot

                // Read page into buffer1
                uint8_t* buffer = buffer1;
                UINT bytesToRead = pageSize;
                bytesRead = file.read(buffer, bytesToRead);
                if (bytesRead != bytesToRead)
                {
                    return false;
                }

                uint8_t* buffer2 = &buffer1[0x2000];
                if (pageSize > 0x1B00)
                {
                    pageSize = 0x1B00;
                }

                DecompressPage(buffer1, pageSize, isCompressed, 0x1B00, buffer2);
                ShowScreenshot(buffer2);

                return true;
            }
            else
            {
                // Move forward without reading
                bool readResult = file.seek(file.position() + pageSize);
                if (readResult != true)
                {
                    return false;
                }
            }

            bytesRead = file.read(buffer1, 3);
            if (bytesRead <= 0)
            {
                return false;
            }

            if (bytesRead == 3)
            {
                GetPageInfo(buffer1, is128Mode, pagingState, &pageNumber, &pageSize);
            }
            else
            {
                pageSize = 0;
            }

        } while (pageSize > 0);
    }

	return true;
}

bool zx::LoadScreenshot(File file, uint8_t buffer1[0x4000])
{
	size_t bytesRead;
	uint8_t* buffer = buffer1;
    UINT bytesToRead = 0x1B00;
    bytesRead = file.read(buffer, bytesToRead);
    if (bytesRead != bytesToRead)
    {
        return false;
    }

    ShowScreenshot(buffer1);

	return true;
}

uint16_t DecompressPage(uint8_t *page, uint16_t pageLength, bool isCompressed,
		uint16_t maxSize, uint8_t* destMemory)
{
	uint16_t size = 0;
	uint8_t* memory = destMemory;
	for (int i = 0; i < pageLength; i++)
	{
		if (i < pageLength - 4)
		{
			if (page[i] == 0x00 && page[i + 1] == 0xED && page[i + 2] == 0xED
					&& page[i + 3] == 0x00)
			{
				return i + 4;
			}

			if (isCompressed && page[i] == 0xED && page[i + 1] == 0xED)
			{
				i += 2;
				int repeat = page[i++];
				uint8_t value = page[i];
				for (int j = 0; j < repeat; j++)
				{
					*memory = value;
					memory++;

					size++;
					if (maxSize > 0 && size >= maxSize)
					{
Serial.println("in the middle of repeat");
						return i + 1;
					}
				}

				continue;
			}
		}

		*memory = page[i];
		memory++;

		size++;
		if (maxSize > 0 && size >= maxSize)
		{
			return i + 1;
		}
	}

    return pageLength;
}

void ReadState(FileHeader* header)
{
	// If byte 12 is 255, it has to be regarded as being 1
	if (header->Flags1 == 255)
	{
		header->Flags1 = 1;
	}

	Z80cpu.A = header->A;
	Z80cpu.F = header->F;
	Z80cpu.BC = header->BC;
	Z80cpu.HL = header->HL;
	Z80cpu.SP = header->SP;
	Z80cpu.I = header->InterruptRegister;
	Z80cpu.R = (header->RefreshRegister & 0x7F)
			| ((header->Flags1 & 0x01) << 7);
	Z80cpu.IM = header->Flags2 & 0x3;
	Z80cpu.DE = header->DE;
	Z80cpu.BCx = header->BC_Dash;
	Z80cpu.DEx = header->DE_Dash;
	Z80cpu.HLx = header->HL_Dash;
	Z80cpu.AFx = header->F_Dash | (header->A_Dash << 8);
	Z80cpu.IY = header->IY;
	Z80cpu.IX = header->IX;
	Z80cpu.IFF1 = header->InterruptFlipFlop;
	Z80cpu.IFF2 = header->IFF2;
	Z80cpu.PC = header->PC == 0 ? header->PCVersion2 : header->PC;

	uint8_t borderColor = (header->Flags1 & 0x0E) >> 1;
    Environment.BorderColor = borderColor;
}

void SaveState(FileHeader* header)
{
	header->PC = 0;
	header->AdditionalBlockLength = 54;

    if (Environment.MemoryState.PagingLock == 1
        && Environment.MemoryState.RomSelect == 1)
    {
        // Save as 48K snapshot
        header->HardwareMode = 0;
        header->PagingState = 0;
    }
    else
    {
        // Save as 128K snapshot
        header->HardwareMode = 4;
        header->PagingState = Environment.MemoryState.Bits;
    }

	header->A = Z80cpu.A;
	header->F = Z80cpu.F;
	header->BC = Z80cpu.BC;
	header->HL = Z80cpu.HL;
	header->SP = Z80cpu.SP;
	header->InterruptRegister = Z80cpu.I;
	header->RefreshRegister = Z80cpu.R;
	header->DE = Z80cpu.DE;
	header->BC_Dash = Z80cpu.BCx;
	header->DE_Dash = Z80cpu.DEx;
	header->HL_Dash = Z80cpu.HLx;
	header->F_Dash = Z80cpu.AFx & 0xFF;
	header->A_Dash = (Z80cpu.AFx & 0xFF00) >> 8;
	header->IY = Z80cpu.IY;
	header->IX = Z80cpu.IX;
	header->InterruptFlipFlop = Z80cpu.IFF1;
	header->IFF2 = Z80cpu.IFF2;
	header->PCVersion2 = Z80cpu.PC;

	// Bit 0  : Bit 7 of the R-register
	// Bit 1-3: Border color
	header->Flags1 = (Z80cpu.R & 0x80) >> 7;
	uint8_t border = Environment.BorderColor;
	header->Flags1 |= (border & 0x38) >> 2;

	// Bit 0-1: Interrupt mode (0, 1 or 2)
	header->Flags2 = Z80cpu.IM & 0x03;
}

void GetPageInfo(uint8_t* buffer, bool is128Mode, uint8_t pagingState, int8_t* pageNumber, uint16_t* pageSize)
{
	*pageSize = buffer[0];
	*pageSize |= buffer[1] << 8;
	*pageNumber = buffer[2];

#ifdef ZX128K
	if (!is128Mode)
	{
        // 48K snapshot

		switch (*pageNumber)
		{
		case 4:
			// 48K : 0x8000..0xBFFF
            *pageNumber = 2;
			break;
		case 5:
			// 48K : 0xC000..0xFFFF
            *pageNumber = 0;
			break;
		case 8:
			// 48K : 0x4000..0x7FFF
            *pageNumber = 5;
			break;
		}
	}
    else
    {
        // 128K snapshot

        *pageNumber -= 3;
    }
#else
	if (is128Mode)
	{
		switch (*pageNumber)
		{
		case 8:
			// 0x4000..0x7FFF
            *pageNumber = 5;
			break;
		case 4:
			// 0x8000..0xBFFF
            *pageNumber = 2;
			break;
		default:
			if (*pageNumber == (pagingState & 0x03) + 3)
			{
				*pageNumber = 0; // 0xC000..0xFFFF
			}
			else
			{
				// skip it
				*pageNumber = -1;
			}
			break;
		}
	}
#endif
}

uint8_t CountEqualBytes(uint8_t* address, uint8_t* maxAddress)
{
	int result;
	uint8_t byteValue = *address;

	for (result = 1; result < 255; result++)
	{
		address++;
		if (byteValue != *address
			|| address >= maxAddress)
		{
			break;
		}
	}

	return (uint8_t)result;
}

uint16_t CompressPage(uint8_t* page, uint8_t* destMemory)
{
	uint16_t size = 0;
	uint8_t* maxAddress = page + 0x4000;
	bool isPrevoiusSingleED = false;

	for (uint8_t* memory = page; memory < maxAddress; memory++)
	{
		uint8_t byteValue = *memory;
		uint8_t equalBytes;

		if (isPrevoiusSingleED)
		{
			// A byte directly following a single 0xED is not taken into a block
			equalBytes = 1;
		}
		else
		{
			equalBytes = CountEqualBytes(memory, maxAddress);
		}

		uint8_t minRepeats;
		if (byteValue == 0xED)
		{
			minRepeats = 2;
		}
		else
		{
			minRepeats = 5;
		}

		if (equalBytes >= minRepeats)
		{
			*destMemory = 0xED;
			destMemory++;
			*destMemory = 0xED;
			destMemory++;
			*destMemory = equalBytes;
			destMemory++;

			memory += equalBytes - 1;
			size += 3;
		}

		*destMemory = byteValue;
		destMemory++;
		size++;

		isPrevoiusSingleED = (byteValue == 0xED && memory < maxAddress && *(memory + 1) != 0xED);
	}

	return size;
}

void ShowScreenshot(uint8_t* buffer)
{
    Environment.Screen->ShowScreenshot(buffer);
}