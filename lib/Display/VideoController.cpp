#include <string.h>
#include "VideoController.h"
#include "DrawScanLine.h"

namespace Display
{

Band* IRAM_ATTR VideoController::GetBand(int scanLine)
{
    if (!this->_isInitialized)
    {
        return nullptr;
    }

    if (scanLine >= this->_band2Start)
    {
        return this->_band2;
    }
    else
    {
        return this->_band1;
    }
}

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

    this->setDrawScanlineCallback(drawScanline, this);
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

    this->_isInitialized = true;
}

uint8_t IRAM_ATTR VideoController::createRawPixel(uint8_t color)
{
    // HACK: should call createRawPixel() instead
    return m_HVSync | color;
}

}
