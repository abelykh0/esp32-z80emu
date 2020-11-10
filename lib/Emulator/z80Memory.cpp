#include "z80Memory.h"
#include "main_ROM.h"

ZxSpectrumMemory SpectrumMemory;

void ZxSpectrumMemory::Initialize()
{
    // Due to a technical limitation, the maximum statically allocated DRAM usage is 160KB
    // The remaining 160KB (for a total of 320KB of DRAM) can only be allocated at runtime as heap

    this->Rom0 = (uint8_t*)ROM;

#ifdef ZX128K
    this->Rom1 = this->Rom0;
    this->Ram1 = (uint8_t*)malloc(0x4000);
    this->Ram3 = (uint8_t*)malloc(0x4000);
    this->Ram4 = (uint8_t*)malloc(0x4000);
    this->Ram6 = (uint8_t*)malloc(0x4000);
    this->Ram7 = (uint8_t*)malloc(0x2500); // the rest in _shadowScreenData
#endif
}

void ZxSpectrumMemory::WriteByte(uint16_t addr, uint8_t data)
{
    this->WriteByte(this->MemoryState.RamBank, addr, data);
}

uint8_t ZxSpectrumMemory::ReadByte(uint16_t addr)
{
    return this->ReadByte(this->MemoryState.RamBank, addr);
}

void ZxSpectrumMemory::SetState(uint8_t memoryState)
{
    if (this->MemoryState.PagingLock != 0)
    {
        // Locked, don't change
        return;
    }

    this->MemoryState.Bits = memoryState;
}

void ZxSpectrumMemory::WriteByte(uint8_t bank, uint16_t addr, uint8_t data)
{
    uint16_t offset;

    switch (bank)
    {
    case 5:
        switch (addr)
        {
            case 0x0000 ... 0x17FF:
                // Screen pixels
                this->MainScreenData.Pixels[addr] = data;
                break;
            case 0x1800 ... 0x1AFF:
                // Screen Attributes
                this->MainScreenData.Attributes[addr - (uint16_t)0x1800] = ZxSpectrumMemory::FromSpectrumColor(data);
                break;
            case 0x1B00 ... 0x3FFF:
                offset = addr - (uint16_t)0x1B00;
                SpectrumMemory.Ram5[offset] = data;
                break;
        }
        break;

    case 2:
        SpectrumMemory.Ram2[addr] = data;
        break;

#ifdef ZX128K
    case 0:
        SpectrumMemory.Ram0[addr] = data;
        break;

    case 1:
        SpectrumMemory.Ram1[addr] = data;
        break;

    case 3:
        SpectrumMemory.Ram3[addr] = data;
        break;

    case 4:
        SpectrumMemory.Ram4[addr] = data;
        break;

    case 6:
        SpectrumMemory.Ram6[addr] = data;
        break;

    case 7:
        switch (addr)
        {
            case 0x0000 ... 0x17FF:
                // Screen pixels
                this->ShadowScreenData.Pixels[addr] = data;
                break;
            case 0x1800 ... 0x1AFF:
                // Screen Attributes
                this->ShadowScreenData.Attributes[addr - (uint16_t)0x1800] = this->FromSpectrumColor(data);
                break;
            case 0x1B00 ... 0x3FFF:
                offset = addr - (uint16_t)0x1B00;
                SpectrumMemory.Ram7[offset] = data;
                break;
        }
        break;
#endif
    }
}

uint8_t ZxSpectrumMemory::ReadByte(uint8_t bank, uint16_t addr)
{
    uint16_t offset;
    uint8_t result;

    switch (bank)
    {
    case 5:
        switch (addr)
        {
            case 0x0000 ... 0x17FF:
                // Screen pixels
                result = this->MainScreenData.Pixels[addr];
                break;
            case 0x1800 ... 0x1AFF:
                // Screen Attributes
                result = ZxSpectrumMemory::ToSpectrumColor(this->MainScreenData.Attributes[addr - (uint16_t)0x1800]);
                break;
            case 0x1B00 ... 0x3FFF:
                offset = addr - (uint16_t)0x1B00;
                result = SpectrumMemory.Ram5[offset];
                break;
            default:
                result = 0xFF;
                break;
        }
        break;

    case 2:
        result = SpectrumMemory.Ram2[addr];
        break;

#ifdef ZX128K
    case 0:
        result = SpectrumMemory.Ram0[addr];
        break;

    case 1:
        result = SpectrumMemory.Ram1[addr];
        break;

    case 3:
        result = SpectrumMemory.Ram3[addr];
        break;

    case 4:
        result = SpectrumMemory.Ram4[addr];
        break;

    case 6:
        result = SpectrumMemory.Ram6[addr];
        break;

    case 7:
        switch (addr)
        {
            case 0x0000 ... 0x17FF:
                // Screen pixels
                result = this->ShadowScreenData.Pixels[addr];
                break;
            case 0x1800 ... 0x1AFF:
                // Screen Attributes
                result = ZxSpectrumMemory::ToSpectrumColor(this->ShadowScreenData.Attributes[addr - (uint16_t)0x1800]);
                break;
            case 0x1B00 ... 0x3FFF:
                offset = addr - (uint16_t)0x1B00;
                result = SpectrumMemory.Ram7[offset];
                break;
            default:
                result = 0xFF;
                break;
        }
        break;
#endif
        default:
            result = 0xFF;
            break;
    }

    return result;
}

void ZxSpectrumMemory::FromBuffer(uint8_t bank, uint8_t* data)
{
    switch (bank)
    {
    case 5:
		// Video RAM
		memcpy(this->MainScreenData.Pixels, data, 0x1800);
		for (uint32_t i = 0x1800; i < 0x1AFF; i++)
		{
            this->WriteByte(bank, i, data[i]);
		}

        // The rest
		memcpy(this->Ram5, &data[0x1B00], 0x2500);

        break;

    case 2:
        memcpy(SpectrumMemory.Ram2, data, 0x4000);
        break;

    case 0:
        memcpy(SpectrumMemory.Ram0, data, 0x4000);
        break;

#ifdef ZX128K

    case 1:
        memcpy(SpectrumMemory.Ram1, data, 0x4000);
        break;

    case 3:
        memcpy(SpectrumMemory.Ram3, data, 0x4000);
        break;

    case 4:
        memcpy(SpectrumMemory.Ram4, data, 0x4000);
        break;

    case 6:
        memcpy(SpectrumMemory.Ram6, data, 0x4000);
        break;

    case 7:
		// Video RAM
		memcpy(this->ShadowScreenData.Pixels, data, 0x1800);
		for (uint32_t i = 0x1800; i < 0x1AFF; i++)
		{
            this->WriteByte(bank, i, data[i]);
		}

        // The rest
		memcpy(this->Ram7, &data[0x1B00], 0x2500);

        break;
    }
#endif
}

void ZxSpectrumMemory::ToBuffer(uint8_t bank, uint8_t* data)
{
    switch (bank)
    {
    case 5:
		// Video RAM
		memcpy(data, this->MainScreenData.Pixels, 0x1800);
		for (uint32_t i = 0x1800; i < 0x1AFF; i++)
		{
            this->WriteByte(bank, i, data[i]);
		}

        // The rest
		memcpy(this->Ram5, &data[0x1B00], 0x2500);

        break;

    case 2:
        memcpy(SpectrumMemory.Ram2, data, 0x4000);
        break;

    case 0:
        memcpy(SpectrumMemory.Ram0, data, 0x4000);
        break;

#ifdef ZX128K
    case 1:
        memcpy(SpectrumMemory.Ram1, data, 0x4000);
        break;

    case 3:
        memcpy(SpectrumMemory.Ram3, data, 0x4000);
        break;

    case 4:
        memcpy(SpectrumMemory.Ram4, data, 0x4000);
        break;

    case 6:
        memcpy(SpectrumMemory.Ram6, data, 0x4000);
        break;

    case 7:
		// Video RAM
		memcpy(this->ShadowScreenData.Pixels, data, 0x1800);
		for (uint32_t i = 0x1800; i < 0x1AFF; i++)
		{
            this->WriteByte(bank, i, data[i]);
		}

        // The rest
		memcpy(this->Ram7, &data[0x1B00], 0x2500);

        break;
    }
#endif
}

uint16_t ZxSpectrumMemory::FromSpectrumColor(uint8_t sinclairColor)
{
	// Sinclair: Flash-Bright-PaperG-PaperR-PaperB-InkG-InkR-InkB
	//               7      6      5      4      3    2    1    0
	// Our colors: 00-PaperB01-PaperG01-PaperR01 : 00-InkB01-InkG01-InkR01
	//                      54       32       10 :        54     32     10

	bool bright = ((sinclairColor & 0B01000000) != 0);

	uint16_t ink = ((sinclairColor & 0B00000100) << 8); // InkG
	ink |= ((sinclairColor & 0B00000010) << 7);         // InkR
	ink |= ((sinclairColor & 0B00000001) << 12);        // InkB
	if (bright)
	{
		ink |= (ink << 1);
	}

	uint16_t paper = ((sinclairColor & 0B00100000) >> 3); // PaperG
	paper |= ((sinclairColor & 0B00010000) >> 4);         // PaperR
	paper |= ((sinclairColor & 0B00001000) << 1);         // PaperB
	if (bright)
	{
		paper |= (paper << 1);
	}

	uint16_t result = ink | paper;

	if (bright)
	{
		// This is only needed to correctly read back "bright black" color
		result |= 0x4000;
	}

	if ((sinclairColor & 0B10000000) != 0)
	{
		// Blink
		result |= 0x8000;
	}

	return result;
}

uint8_t ZxSpectrumMemory::ToSpectrumColor(uint16_t color)
{
	// Our colors: 00-PaperB01-PaperG01-PaperR01 : 00-InkB01-InkG01-InkR01
	//                      54       32       10 :        54     32     10
	// Sinclair: Flash-Bright-PaperG-PaperR-PaperB-InkG-InkR-InkB
	//               7      6      5      4      3    2    1    0

	uint8_t result = 0;

	if ((color & 0x0080) != 0)
	{
		// Flash, need to swap bytes
		color = __builtin_bswap16(color);
	}

	if ((color & 0x4000) != 0)
	{
		// Bright
		result |= 0B01000000;
	}

	if ((color & 0x8000) != 0)
	{
		// Flash
		result |= 0B10000000;
	}

	result |= ((color & 0B00010000) >> 1); // PaperB
	result |= ((color & 0B00000001) << 4); // PaperR
	result |= ((color & 0B00000100) << 3); // PaperG

	color >>= 8;
	result |= ((color & 0B00010000) >> 4); // InkB
	result |= ((color & 0B00000001) << 1); // InkR
	result |= (color & 0B00000100);        // InkG

	return result;
}
