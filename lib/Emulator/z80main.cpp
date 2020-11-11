#include <string.h>
#include <stdio.h>

#include "z80main.h"
#include "z80input.h"
#include "z80Memory.h"
#include "ps2Input.h"

//#define BEEPER

#ifdef ZX128K
#define CYCLES_PER_STEP 70908
#else
#define CYCLES_PER_STEP 69888
#endif

Sound::Ay3_8912_state _ay3_8912;
Z80_STATE _zxCpu;

static CONTEXT _zxContext;
static uint16_t _attributeCount;
static int _total;
static int _next_total = 0;
static uint8_t zx_data = 0;
static uint8_t frames = 0;
static uint32_t _ticks = 0;
static SpectrumScreen* _spectrumScreen;

extern "C"
{
    uint8_t readbyte(uint16_t addr);
    uint16_t readword(uint16_t addr);
    void writebyte(uint16_t addr, uint8_t data);
    void writeword(uint16_t addr, uint16_t data);
    uint8_t input(uint8_t portLow, uint8_t portHigh);
    void output(uint8_t portLow, uint8_t portHigh, uint8_t data);
}

void zx_setup(SpectrumScreen* spectrumScreen)
{
	_spectrumScreen = spectrumScreen;
	_attributeCount = spectrumScreen->Settings.TextColumns * spectrumScreen->Settings.TextRows;

    _zxContext.readbyte = readbyte;
    _zxContext.readword = readword;
    _zxContext.writeword = writeword;
    _zxContext.writebyte = writebyte;
    _zxContext.input = input;
    _zxContext.output = output;

#ifdef BEEPER
    // Sound
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
    _ay3_8912.Initialize();

    SpectrumMemory.Initialize();

    zx_reset();
}

void zx_reset()
{
    memset(indata, 0xFF, 128);
    *_spectrumScreen->Settings.BorderColor = 0x15;
    Z80Reset(&_zxCpu);
}

int32_t zx_loop()
{
    int32_t result = -1;

    _total += Z80Emulate(&_zxCpu, _next_total - _total, &_zxContext);

    if (_total >= _next_total)
    {
        _next_total += CYCLES_PER_STEP;

        // flash every 32 frames
        frames++;
        if (frames > 31)
        {
            frames = 0;
            for (int i = 0; i < _attributeCount; i++)
            {
                uint16_t color = _spectrumScreen->Settings.Attributes[i];
                if ((color & 0x8080) != 0)
                {
                	_spectrumScreen->Settings.Attributes[i] = __builtin_bswap16(color);
                }
            }
        }

        // Keyboard input
        int32_t scanCode = Ps2_GetScancode();
        if (scanCode > 0)
        {
            if ((scanCode & 0xFF00) == 0xF000)
            {
                // key up

                scanCode = ((scanCode & 0xFF0000) >> 8 | (scanCode & 0xFF));

                if (!OnKey(scanCode, true))
                {
                    result = scanCode;
                }
            }
            else
            {
                // key down

                OnKey(scanCode, false);
            }
        }

        Z80Interrupt(&_zxCpu, 0xff, &_zxContext);

        // delay
        while (_spectrumScreen->_frames < _ticks)
        {
        }

		_ticks = _spectrumScreen->_frames + 1;
    }

    return result;
}

extern "C" uint8_t readbyte(uint16_t addr)
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

extern "C" uint16_t readword(uint16_t addr)
{
    return ((readbyte(addr + 1) << 8) | readbyte(addr));
}

extern "C" void writebyte(uint16_t addr, uint8_t data)
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

extern "C" void writeword(uint16_t addr, uint16_t data)
{
    writebyte(addr, (uint8_t)data);
    writebyte(addr + 1, (uint8_t)(data >> 8));
}

extern "C" uint8_t input(uint8_t portLow, uint8_t portHigh)
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

extern "C" void output(uint8_t portLow, uint8_t portHigh, uint8_t data)
{
    switch (portLow)
    {
    case 0xFE:
    {
        // border color (no bright colors)
        uint8_t borderColor = (data & 0x07);
    	if ((indata[0x20] & 0x07) != borderColor)
    	{
            *_spectrumScreen->Settings.BorderColor = ZxSpectrumMemory::FromSpectrumColor(borderColor) >> 8;
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
                    _spectrumScreen->Settings.Pixels = SpectrumMemory.ShadowScreenData.Pixels;
                    _spectrumScreen->Settings.Attributes = SpectrumMemory.ShadowScreenData.Attributes;
                }
                else
                {
                    _spectrumScreen->Settings.Pixels = SpectrumMemory.MainScreenData.Pixels;
                    _spectrumScreen->Settings.Attributes = SpectrumMemory.MainScreenData.Attributes;
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
