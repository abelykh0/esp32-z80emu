#include <string.h>
#include "VideoController.h"

namespace Display
{

VideoController::VideoController(Band* band1, Band* band2)
{
    this->_band1 = band1;
    this->_band2 = band2;

    if (this->_band2 == nullptr)
    {
        this->_band2Start = this->_band1->Height;
    }
    else
    {
        this->_band2Start = this->_band2->StartLine;
    }
}

void IRAM_ATTR VideoController::drawScanline(uint8_t* dest, int scanLine)
{
    if (scanLine >= this->_band2Start)
    {
        this->_band2->drawScanline(dest, scanLine);
    }
    else
    {
        this->_band1->drawScanline(dest, scanLine);
    }
}

void VideoController::StartVideo(char const* modeline)
{
    this->begin();
    this->setResolution(modeline);

    this->_band1->Initialize(this);
    if (this->_band2 != nullptr)
    {
        this->_band2->Initialize(this);
    }
}

uint8_t IRAM_ATTR VideoController::createRawPixel(uint8_t color)
{
    // HACK: should call createRawPixel() instead
    return m_HVSync | color;
}

}
