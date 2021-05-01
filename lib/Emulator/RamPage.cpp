#include "RamPage.h"

#include <string.h>

RamPage& RamPage::operator=(void* allocatedRam)
{
    this->_data = (uint8_t*)allocatedRam;
}

uint8_t RamPage::ReadByte(uint16_t addr)
{
    return this->_data[addr];
}

void RamPage::WriteByte(uint16_t addr, uint8_t data)
{
    this->_data[addr] = data;
}

void RamPage::FromBuffer(void* buffer)
{
    memcpy(this->_data, buffer, 0x4000);
}

void RamPage::ToBuffer(void* buffer)
{
    memcpy(buffer, this->_data, 0x4000);
}
