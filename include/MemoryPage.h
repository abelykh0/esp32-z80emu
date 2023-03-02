#ifndef __MEMORYPAGE_INCLUDED__
#define __MEMORYPAGE_INCLUDED__

#include <stdint.h>

class MemoryPage
{
public:
    uint8_t virtual ReadByte(uint16_t addr) = 0;
    uint8_t operator[](uint16_t addr)
    {
        return this->ReadByte(addr);
    }
    void virtual WriteByte(uint16_t addr, uint8_t data) = 0;

    uint16_t virtual ReadWord(uint16_t addr)
    {
        return (this->ReadByte(addr + 1) << 8) | this->ReadByte(addr);
    }

    void virtual WriteWord(uint16_t addr, uint16_t data)
    {
        this->WriteByte(addr + 1, data >> 8);
        this->WriteByte(addr, data & 0xFF);
    }

    void virtual FromBuffer(void* buffer) = 0;
    void virtual ToBuffer(void* buffer) = 0;
};

#endif
