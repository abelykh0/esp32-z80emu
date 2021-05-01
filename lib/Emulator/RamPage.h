#ifndef __RAMPAGE_INCLUDED__
#define __RAMPAGE_INCLUDED__

#include "MemoryPage.h"

class RamPage: public MemoryPage
{
private:
    uint8_t* _data = nullptr;
public:
    RamPage& operator=(void* allocatedRam);

    uint8_t virtual ReadByte(uint16_t addr) override;
    void virtual WriteByte(uint16_t addr, uint8_t data) override;
    void virtual FromBuffer(void* buffer) override;
    void virtual ToBuffer(void* buffer) override;    
};

#endif
