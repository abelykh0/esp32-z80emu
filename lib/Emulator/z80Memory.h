
#ifndef __ZXMEMORY_INCLUDED__
#define __ZXMEMORY_INCLUDED__

#include "SpectrumScreen.h"

#define ZX128K

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

typedef struct 
{
	uint8_t  Pixels[32 * 8 * 24];
	uint16_t Attributes[32 * 24];
	uint8_t  BorderColor;
} SpectrumScreenData;

class ZxSpectrumMemory
{
public:
    MemorySelect MemoryState;

    uint8_t Ram0[0x4000];
    uint8_t Ram2[0x4000];
    SpectrumScreenData MainScreenData;
    uint8_t Ram5[0x2500]; // the rest in _mainScreenData

#ifdef ZX128K
    // Due to a technical limitation, the maximum statically allocated DRAM usage is 160KB
    // The remaining 160KB (for a total of 320KB of DRAM) can only be allocated at runtime as heap
    uint8_t* Ram1;
    uint8_t* Ram3;
    uint8_t* Ram4;
    uint8_t* Ram6;
    SpectrumScreenData ShadowScreenData;
    uint8_t* Ram7;
#endif

    void Initialize();
    void SetState(uint8_t memoryState);
    void WriteByte(uint8_t bank, uint16_t addr, uint8_t data);
    uint8_t ReadByte(uint8_t bank, uint16_t addr);
    void WriteByte(uint16_t addr, uint8_t data);
    uint8_t ReadByte(uint16_t addr);
	static uint16_t FromSpectrumColor(uint8_t sinclairColor);
	static uint8_t ToSpectrumColor(uint16_t color);
};

extern ZxSpectrumMemory SpectrumMemory;

#endif
