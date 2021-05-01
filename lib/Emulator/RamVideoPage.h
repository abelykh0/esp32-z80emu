#ifndef __RAMVIDEOPAGE_INCLUDED__
#define __RAMVIDEOPAGE_INCLUDED__

#include "MemoryPage.h"
#include "VideoSettings.h"
#include "SpectrumScreenData.h"

using namespace Display;

class RamVideoPage: public MemoryPage
{
private:
    SpectrumScreenData* _videoRam = nullptr;
    uint8_t* _data = nullptr;

public:
    void Initialize(SpectrumScreenData* _videoRam, void* allocatedRam);

    uint8_t virtual ReadByte(uint16_t addr) override;
    void virtual WriteByte(uint16_t addr, uint8_t data) override;
    void virtual FromBuffer(void* data) override;
    void virtual ToBuffer(void* buffer) override;    
};

#endif
