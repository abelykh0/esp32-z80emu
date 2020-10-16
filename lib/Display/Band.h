#ifndef _BAND_H
#define _BAND_H

#include <arduino.h>

namespace Display
{

class VideoController;

class Band
{
public:
	uint16_t StartLine;
	uint16_t Height;

    Band(uint16_t startLine, uint16_t height);

    virtual void Initialize(VideoController* videoController);
	virtual void IRAM_ATTR drawScanline(uint8_t* dest, int scanLine) = 0;

protected:
    VideoController* Controller;

    void SetScreenSize(uint16_t width, uint16_t height);
};

}

#endif
