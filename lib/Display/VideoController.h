#ifndef _VIDEOCONTROLLER_H
#define _VIDEOCONTROLLER_H

#include "Band.h"
#include "fabgl.h"

using namespace fabgl;

namespace Display
{

class VideoController : public VGADirectController
{
public:
    VideoController(Band* band1, Band* band2);

    uint8_t IRAM_ATTR createRawPixel(uint8_t color);

    void StartVideo(char const* modeline);

    Band* IRAM_ATTR GetBand(int scanLine);

private:
    Band* _band1 = nullptr;
    Band* _band2 = nullptr;
    uint16_t _band2Start;
    bool _isInitialized = false;

};

}

#endif
