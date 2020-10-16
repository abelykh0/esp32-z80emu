#include "SpectrumScreen.h"
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
		this->Settings.Attributes[i] = this->FromSpectrumColor(
				screenshot[this->_pixelCount + i]);
	}
}

uint16_t SpectrumScreen::FromSpectrumColor(uint8_t sinclairColor)
{
	// Sinclair: Flash-Bright-PaperG-PaperR-PaperB-InkG-InkR-InkB
	//               7      6      5      4      3    2    1    0
	// Our colors: 00-PaperB01-PaperG01-PaperR01 : 00-InkB01-InkG01-InkR01
	//                      54       32       10 :        54     32     10

	bool bright = ((sinclairColor & 0B01000000) != 0);

	uint16_t ink = ((sinclairColor & 0B00000100) << 8); // InkG
	ink |= ((sinclairColor & 0B00000010) << 7);         // InkR
	ink |= ((sinclairColor & 0B00000001) << 12);        // InkB
	if (bright)
	{
		ink |= (ink << 1);
	}

	uint16_t paper = ((sinclairColor & 0B00100000) >> 3); // PaperG
	paper |= ((sinclairColor & 0B00010000) >> 4);         // PaperR
	paper |= ((sinclairColor & 0B00001000) << 1);         // PaperB
	if (bright)
	{
		paper |= (paper << 1);
	}

	uint16_t result = ink | paper;

	if (bright)
	{
		// This is only needed to correctly read back "bright black" color
		result |= 0x4000;
	}

	if ((sinclairColor & 0B10000000) != 0)
	{
		// Blink
		result |= 0x8000;
	}

	return result;
}

uint8_t SpectrumScreen::ToSpectrumColor(uint16_t color)
{
	// Our colors: 00-PaperB01-PaperG01-PaperR01 : 00-InkB01-InkG01-InkR01
	//                      54       32       10 :        54     32     10
	// Sinclair: Flash-Bright-PaperG-PaperR-PaperB-InkG-InkR-InkB
	//               7      6      5      4      3    2    1    0

	uint8_t result = 0;

	if ((color & 0x0080) != 0)
	{
		// Flash, need to swap bytes
		color = __builtin_bswap16(color);
	}

	if ((color & 0x4000) != 0)
	{
		// Bright
		result |= 0B01000000;
	}

	if ((color & 0x8000) != 0)
	{
		// Flash
		result |= 0B10000000;
	}

	result |= ((color & 0B00010000) >> 1); // PaperB
	result |= ((color & 0B00000001) << 4); // PaperR
	result |= ((color & 0B00000100) << 3); // PaperG

	color >>= 8;
	result |= ((color & 0B00010000) >> 4); // InkB
	result |= ((color & 0B00000001) << 1); // InkR
	result |= (color & 0B00000100);        // InkG

	return result;
}

}
