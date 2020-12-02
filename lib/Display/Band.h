#ifndef _BAND_H
#define _BAND_H

#include <arduino.h>
#include "VideoSettings.h"

namespace Display
{

class VideoController;

class Band
{
public:
	uint16_t StartLine;
	uint16_t Height;
    uint8_t VerticalBorder;
    uint16_t HorizontalResolution;
    uint16_t HorizontalBorder;
    VideoSettings* Settings;

    volatile uint32_t Frames = 0;
    uint8_t* (*getPixelPointer)(VideoSettings* settings, uint16_t line);

    Band(uint16_t startLine, uint16_t height);

    virtual void Initialize(VideoController* videoController);

protected:
    VideoController* Controller;

    void SetScreenSize(uint16_t width, uint16_t height);
};

}

#endif
