#ifndef DRAWSCANLINE_H
#define DRAWSCANLINE_H

#include "esp_attr.h"
#include "stdint.h"

namespace Display
{

void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

}

#endif