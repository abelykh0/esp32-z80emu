#ifndef _SPECTRUMSCREEN_H
#define _SPECTRUMSCREEN_H

#include "VideoSettings.h"
#include "Screen.h"

namespace Display
{

class SpectrumScreen: public Screen
{
protected:
	uint8_t* GetPixelPointer(uint16_t line) override;
	uint8_t* GetPixelPointer(uint16_t line, uint8_t character) override;

public:
	SpectrumScreen(VideoSettings settings, uint16_t startLine, uint16_t height);

	void ShowScreenshot(const uint8_t* screenshot);
};

}

#endif
