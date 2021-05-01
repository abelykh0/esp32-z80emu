#include "z80Environment.h"
#include "z80input.h"
#include "ps2Input.h"
#include "ay3-8912-state.h"
#include "main_ROM.h"

Sound::Ay3_8912_state _ay3_8912;
static uint8_t zx_data = 0;

static uint8_t Ram0Buffer[0x4000];
static uint8_t Ram2Buffer[0x4000];
static uint8_t Ram5Buffer[0x2500];

Z80Environment::Z80Environment (SpectrumScreen* spectrumScreen)
{
    this->Screen = spectrumScreen;

    VideoSettings* settings = this->Screen->Settings;
    settings->TextColumns = 32;
    settings->TextRows = 24;
    settings->Attributes = this->_mainScreenData.Attributes;
    settings->Pixels = this->_mainScreenData.Pixels;
    settings->BorderColor = &this->BorderColor;

    // Due to a technical limitation, the maximum statically allocated DRAM usage is 160KB
    // The remaining 160KB (for a total of 320KB of DRAM) can only be allocated at runtime as heap

    this->_rom0 = (uint8_t*)ROM;
    this->_rom1 = (uint8_t*)ROM;

    this->_ram0 = Ram0Buffer;
    this->_ram2 = Ram2Buffer;
    this->_ram5.Initialize(&this->_mainScreenData, Ram5Buffer);

#ifdef ZX128K
    this->_ram1 = (uint8_t*)malloc(0x4000);
    this->_ram3 = (uint8_t*)malloc(0x4000);
    this->_ram4 = (uint8_t*)malloc(0x4000);
    this->_ram6 = (uint8_t*)malloc(0x4000);
    this->_ram7.Initialize(&this->_shadowScreenData, (uint8_t*)malloc(0x2500));
#endif

    this->Rom[0] = &this->_rom0;
    this->Rom[1] = &this->_rom1;

    this->Ram[0] = &this->_ram0;
    this->Ram[1] = &this->_ram1;
    this->Ram[2] = &this->_ram2;
    this->Ram[3] = &this->_ram3;
    this->Ram[4] = &this->_ram4;
    this->Ram[5] = &this->_ram5;
    this->Ram[6] = &this->_ram6;
    this->Ram[7] = &this->_ram7;    
}

uint8_t Z80Environment::ReadByte(uint16_t addr)
{
    uint8_t res;
    uint16_t offset;
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            res = this->Rom[this->MemoryState.RomSelect]->ReadByte(addr);
            break;
        case 0x4000 ... 0x7FFF:
            // Always bank 5
            offset = addr - (uint16_t)0x4000;
            res = this->_ram5[offset];
            break;
        case 0x8000 ... 0xBFFF:
            // Always bank 2
            offset = addr - (uint16_t)0x8000;
            res = this->_ram2[offset];
            break;
        case 0xC000 ... 0xFFFF:
            // Selected page
            offset = addr - (uint16_t)0xC000;
            res = this->Ram[this->MemoryState.RamBank]->ReadByte(offset);
            break;
    }

    return res;
}

uint16_t Z80Environment::ReadWord(uint16_t addr)
{
    return ((this->ReadByte(addr + 1) << 8) | this->ReadByte(addr));
}

void Z80Environment::WriteByte(uint16_t addr, uint8_t data)
{
    uint16_t offset;
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            // Cannot write to ROM
            break;
        case 0x4000 ... 0x7FFF:
            // Always bank 5
            offset = addr - (uint16_t)0x4000;
            this->_ram5.WriteByte(offset, data);
            break;
        case 0x8000 ... 0xBFFF:
            // Always bank 2
            offset = addr - (uint16_t)0x8000;
            this->_ram2.WriteByte(offset, data);
            break;
        case 0xC000 ... 0xFFFF:
            // Selected page
            offset = addr - (uint16_t)0xC000;
            this->Ram[this->MemoryState.RamBank]->WriteByte(offset, data);
            break;
    }
}

void Z80Environment::WriteWord(uint16_t addr, uint16_t data)
{
    this->WriteByte(addr, (uint8_t)data);
    this->WriteByte(addr + 1, (uint8_t)(data >> 8));
}

uint8_t Z80Environment::Input(uint8_t portLow, uint8_t portHigh)
{
    if (portLow == 0xFE)
    {
    	// Keyboard

        switch (portHigh)
        {
        case 0xFE:
        case 0xFD:
        case 0xFB:
        case 0xF7:
        case 0xEF:
        case 0xDF:
        case 0xBF:
        case 0x7F:
            return indata[portHigh - 0x7F];
        case 0x00:
			{
				uint8_t result = indata[0xFE - 0x7F];
				result &= indata[0xFD - 0x7F];
				result &= indata[0xFB - 0x7F];
				result &= indata[0xF7 - 0x7F];
				result &= indata[0xEF - 0x7F];
				result &= indata[0xDF - 0x7F];
				result &= indata[0xBF - 0x7F];
				result &= indata[0x7F - 0x7F];
				return result;
			}
        }
    }

    // Sound (AY-3-8912)
    if (portLow == 0xFD)
    {
        switch (portHigh)
        {
        case 0xFF:
        	return _ay3_8912.getRegisterData();
        }
    }

    // Kempston Mouse
    if (portLow == 0xDF && Ps2_isMouseAvailable())
    {
        switch (portHigh)
        {
        case 0xFA:
        	return Ps2_getMouseButtons();
        case 0xFB:
        	return Ps2_getMouseX();
        case 0xFF:
        	return Ps2_getMouseY();
        }
    }

    uint8_t data = zx_data;
    data |= (0xe0); /* Set bits 5-7 - as reset above */
    data &= ~0x40;
    return data;
}

void Z80Environment::SetState(uint8_t memoryState)
{
    if (this->MemoryState.PagingLock != 0)
    {
        // Locked, don't change
        return;
    }

    this->MemoryState.Bits = memoryState;
}

void Z80Environment::Output(uint8_t portLow, uint8_t portHigh, uint8_t data)
{
    switch (portLow)
    {
    case 0xFE:
    {
        // border color (no bright colors)
        uint8_t borderColor = (data & 0x07);
    	if ((indata[0x20] & 0x07) != borderColor)
    	{
            *this->Screen->Settings->BorderColor = Z80Environment::FromSpectrumColor(borderColor) >> 8;
    	}

#ifdef BEEPER
        uint8_t sound = (data & 0x10);
    	if ((indata[0x20] & 0x10) != sound)
    	{
			if (sound)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
			}
    	}
#endif

        indata[0x20] = data;
    }
    break;

    case 0xF5:
    {
        // Sound (AY-3-8912)
        switch (portHigh)
        {
        case 0xC0:
        	_ay3_8912.selectRegister(data);
        	break;
        }
    }
    break;

    case 0xFD:
    {
        switch (portHigh)
        {
        // Sound (AY-3-8912)
        case 0xFF:
        	// Not sure if this one is correct
        	_ay3_8912.selectRegister(data);
        	break;
        case 0xBF:
        	_ay3_8912.setRegisterData(data);
        	break;

        case 0x7F:
            MemorySelect originalState = this->MemoryState;
        	this->SetState(data);
            if (originalState.ShadowScreen != this->MemoryState.ShadowScreen)
            {
                if (this->MemoryState.ShadowScreen == 1)
                {
                    this->Screen->Settings->Pixels = this->_shadowScreenData.Pixels;
                    this->Screen->Settings->Attributes = this->_shadowScreenData.Attributes;
                }
                else
                {
                    this->Screen->Settings->Pixels = this->_mainScreenData.Pixels;
                    this->Screen->Settings->Attributes = this->_mainScreenData.Attributes;
                }
            }

        	break;
        }
    }
    break;

    default:
        zx_data = data;
        break;
    }
}

uint16_t Z80Environment::FromSpectrumColor(uint8_t sinclairColor)
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

uint8_t Z80Environment::ToSpectrumColor(uint16_t color)
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