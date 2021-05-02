#include <string.h>
#include <stdio.h>

#include "z80main.h"
#include "z80input.h"
#include "z80Environment.h"
#include "ps2Input.h"
#include "ay3-8912-state.h"

//#define BEEPER

#ifdef ZX128K
#define CYCLES_PER_STEP 70908
#else
#define CYCLES_PER_STEP 69888
#endif

z80Emulator Z80cpu;
extern Sound::Ay3_8912_state _ay3_8912;

static uint16_t _attributeCount;
static int _total;
static int _next_total = 0;
static uint8_t frames = 0;
static uint32_t _ticks = 0;
static SpectrumScreen* _spectrumScreen;

void zx_setup(Z80Environment* environment)
{
	_spectrumScreen = environment->Screen;
	_attributeCount = _spectrumScreen->Settings->TextColumns * _spectrumScreen->Settings->TextRows;

#ifdef BEEPER
    // Sound
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
    _ay3_8912.Initialize();

    Z80cpu.setup(environment);
    Serial.write("before zx_reset()\r\n");
    zx_reset();
}

void zx_reset()
{
    _ay3_8912.Clear();
    memset(indata, 0xFF, 128);
    *_spectrumScreen->Settings->BorderColor = 0x15;
    Z80cpu.reset();
}

int32_t zx_loop()
{
    int32_t result = -1;

    _total += Z80cpu.emulate(_next_total - _total);

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
                uint16_t color = _spectrumScreen->Settings->Attributes[i];
                if ((color & 0x8080) != 0)
                {
                	_spectrumScreen->Settings->Attributes[i] = __builtin_bswap16(color);
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

        Z80cpu.interrupt();

        // delay
        while (_spectrumScreen->Frames < _ticks)
        {
        }

		_ticks = _spectrumScreen->Frames + 1;
    }

    return result;
}
