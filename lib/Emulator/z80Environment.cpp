#include "z80Environment.h"
#include "z80input.h"
#include "z80Memory.h"
#include "ps2Input.h"
#include "ay3-8912-state.h"

Sound::Ay3_8912_state _ay3_8912;
static uint8_t zx_data = 0;

void Z80Environment::Setup(SpectrumScreen* spectrumScreen)
{
    this->_spectrumScreen = spectrumScreen;
}

uint8_t Z80Environment::ReadByte(uint16_t addr)
{
    uint8_t res;
    uint16_t offset;
    switch (addr)
    {
        case 0x0000 ... 0x3fff:
            if (SpectrumMemory.MemoryState.RomSelect == 0)
            {
                res = SpectrumMemory.Rom0[addr];
            }
            else
            {
                res = SpectrumMemory.Rom1[addr];
            }
            break;
        case 0x4000 ... 0x7FFF:
            // Always bank 5
            offset = addr - (uint16_t)0x4000;
            res = SpectrumMemory.ReadByte(5, offset);
            break;
        case 0x8000 ... 0xBFFF:
            // Always bank 2
            offset = addr - (uint16_t)0x8000;
            res = SpectrumMemory.ReadByte(2, offset);
            break;
        case 0xC000 ... 0xFFFF:
            // Selected page
            offset = addr - (uint16_t)0xC000;
            res = SpectrumMemory.ReadByte(offset);
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
            SpectrumMemory.WriteByte(5, offset, data);
            break;
        case 0x8000 ... 0xBFFF:
            // Always bank 2
            offset = addr - (uint16_t)0x8000;
            SpectrumMemory.WriteByte(2, offset, data);
            break;
        case 0xC000 ... 0xFFFF:
            // Selected page
            offset = addr - (uint16_t)0xC000;
            SpectrumMemory.WriteByte(offset, data);
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
            *_spectrumScreen->Settings->BorderColor = ZxSpectrumMemory::FromSpectrumColor(borderColor) >> 8;
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
            MemorySelect originalState = SpectrumMemory.MemoryState;
        	SpectrumMemory.SetState(data);
            if (originalState.ShadowScreen != SpectrumMemory.MemoryState.ShadowScreen)
            {
                if (SpectrumMemory.MemoryState.ShadowScreen == 1)
                {
                    _spectrumScreen->Settings->Pixels = SpectrumMemory.ShadowScreenData.Pixels;
                    _spectrumScreen->Settings->Attributes = SpectrumMemory.ShadowScreenData.Attributes;
                }
                else
                {
                    _spectrumScreen->Settings->Pixels = SpectrumMemory.MainScreenData.Pixels;
                    _spectrumScreen->Settings->Attributes = SpectrumMemory.MainScreenData.Attributes;
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


