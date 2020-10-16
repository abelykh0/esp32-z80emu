#include "Band.h"
#include "VideoController.h"

namespace Display
{

Band::Band(uint16_t startLine, uint16_t height)
{
    this->StartLine = startLine;
    this->Height = height;
}

void Band::Initialize(VideoController* videoController)
{
    this->Controller = videoController;
}

}
