#ifndef __Z80ENVIRONMENT_INCLUDED__
#define __Z80ENVIRONMENT_INCLUDED__

#define ZX128K

// Class for the Z80 enviroment (ROM, RAM, I/O)

#include <stdint.h>
#include "SpectrumScreen.h"
#include "MemoryPage.h"
#include "RamPage.h"
#include "RamVideoPage.h"
#include "SpectrumScreenData.h"
#include "ClassProperties.h"

using namespace Display;

typedef struct 
{
    union
    {
        uint8_t Bits;

        struct 
        {
            uint8_t RamBank : 3;
            uint8_t ShadowScreen : 1;
            uint8_t RomSelect : 1;
            uint8_t PagingLock : 1;
        } __attribute__((packed));
    };
} MemorySelect;

class Z80Environment
{
private:
    uint8_t _borderColor;
    uint8_t get_BorderColor(); void set_BorderColor(uint8_t);

    RamPage _rom0;
    RamPage _rom1;

    RamPage _ram0;
    RamPage _ram2;
	SpectrumScreenData _mainScreenData;
    RamVideoPage _ram5;

#ifdef ZX128K
    RamPage _ram1;
    RamPage _ram3;
    RamPage _ram4;
    RamPage _ram6;
	SpectrumScreenData _shadowScreenData;
    RamVideoPage _ram7;
#endif

public:
	SpectrumScreen* Screen;
    RamPage* Rom[2];
    MemoryPage* Ram[8];
    MemorySelect MemoryState;

    CLASS(Z80Environment);
    PROPERTY(uint8_t, BorderColor);

    Z80Environment(SpectrumScreen* spectrumScreen);

	void SetState(uint8_t memoryState);
    uint8_t ReadByte(uint16_t address);
	uint16_t ReadWord(uint16_t address);
	void WriteByte(uint16_t address, uint8_t data);
	void WriteWord(uint16_t address, uint16_t data);
	uint8_t Input(uint8_t portLow, uint8_t portHigh);
	void Output(uint8_t portLow, uint8_t portHigh, uint8_t data);

	static uint16_t FromSpectrumColor(uint8_t sinclairColor);
	static uint8_t ToSpectrumColor(uint16_t color);
};

#endif

