#include "RamVideoPage.h"

#include <string.h>
#include "z80Environment.h"

void RamVideoPage::Initialize(SpectrumScreenData* videoRam, void* allocatedRam)
{
    this->_videoRam = videoRam;
    this->_data = (uint8_t*)allocatedRam;
}

uint8_t RamVideoPage::ReadByte(uint16_t addr)
{
    switch (addr)
    {
        case 0x0000 ... 0x17FF:
            // Screen pixels
            return this->_videoRam->Pixels[addr];
        case 0x1800 ... 0x1AFF:
            // Screen Attributes
            return Z80Environment::ToSpectrumColor(this->_videoRam->Attributes[addr - (uint16_t)0x1800]);
        case 0x1B00 ... 0x3FFF:
            return this->_data[addr - (uint16_t)0x1B00];
        default:
            return 0xFF;
    }
}

void RamVideoPage::WriteByte(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
        case 0x0000 ... 0x17FF:
            // Screen pixels
            this->_videoRam->Pixels[addr] = data;
            break;
        case 0x1800 ... 0x1AFF:
            // Screen Attributes
            this->_videoRam->Attributes[addr - (uint16_t)0x1800] = Z80Environment::FromSpectrumColor(data);
            break;
        case 0x1B00 ... 0x3FFF:
            this->_data[addr - (uint16_t)0x1B00] = data;
            break;
    }
}

void RamVideoPage::FromBuffer(void* data)
{
    // Screen pixels
    memcpy(this->_videoRam->Pixels, data, 0x1800);

    // Screen Attributes
    for (uint32_t i = 0x1800; i < 0x1AFF; i++)
    {
        this->WriteByte(i, ((uint8_t*)data)[i]);
    }

    // The rest
    memcpy(this->_data, &((uint8_t*)data)[0x1B00], 0x2500);
}

void RamVideoPage::ToBuffer(void* buffer)
{
    uint8_t* data = (uint8_t*)buffer;

    // Screen pixels
    memcpy(buffer, this->_videoRam->Pixels, 0x1800);

    // Screen Attributes
    for (uint32_t i = 0x1800; i < 0x1AFF; i++)
    {
        data[i] = this->ReadByte(i);
    }

    // The rest
    memcpy(&data[0x1B00], this->_data, 0x2500);
}
