#include "SpectrumScreen.h"
#include "z80Memory.h"
#include <string.h>

namespace Display
{

SpectrumScreen::SpectrumScreen(VideoSettings settings, uint16_t startLine, uint16_t height)
	: Screen(settings, startLine, height)
{
}

uint8_t* SpectrumScreen::GetPixelPointer(uint16_t line)
{
	// ZX Sinclair addressing
	// 00-00-00-Y7-Y6-Y2-Y1-Y0 Y5-Y4-Y3-x4-x3-x2-x1-x0
	//          12 11 10  9  8  7  6  5  4  3  2  1  0

	uint32_t y012 = ((line & 0B00000111) << 8);
	uint32_t y345 = ((line & 0B00111000) << 2);
	uint32_t y67 = ((line & 0B11000000) << 5);
	return &this->Settings.Pixels[y012 | y345 | y67];
}

uint8_t* SpectrumScreen::GetPixelPointer(uint16_t line, uint8_t character)
{
	character &= 0B00011111;
	return this->GetPixelPointer(line) + character;
}

void SpectrumScreen::ShowScreenshot(const uint8_t* screenshot)
{
	memcpy(this->Settings.Pixels, screenshot, this->_pixelCount);
	for (uint32_t i = 0; i < this->_attributeCount; i++)
	{
		this->Settings.Attributes[i] = ZxSpectrumMemory::FromSpectrumColor(
				screenshot[this->_pixelCount + i]);
	}
}

}
