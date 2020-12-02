#include "DrawScanLine.h"
#include "Screen.h"

namespace Display
{

void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine)
{

    auto controller = static_cast<VideoController*>(arg);

    auto band = controller->GetBand(scanLine);

	if (band == nullptr)
	{
		// Not initialized yet
		return;
	}

    unsigned scaledLine = (scanLine - band->StartLine);
    if (scaledLine == 0)
    {
    	band->Frames++;
    }

    uint8_t borderColor = *band->Settings->BorderColor;
    borderColor = controller->createRawPixel(borderColor);

    if (scaledLine < band->VerticalBorder
    	|| scaledLine >= (unsigned)(band->Height - band->VerticalBorder))
    {
        for (int x = 0; x < band->HorizontalResolution; x++)
        {
            dest[x] = borderColor;
        }
    }
    else
    {
        // Border to the left
        for (int column = 0; column < band->HorizontalBorder; column++)
        {
            VGA_PIXELINROW(dest, column) = borderColor;
        }

        // Screen pixels
        uint16_t vline = scaledLine - band->VerticalBorder;
        uint8_t* bitmap = (uint8_t*)band->getPixelPointer(band->Settings, vline);
        uint16_t* colors = (uint16_t*)&band->Settings->Attributes[vline / 8 * band->Settings->TextColumns];
        int column = band->HorizontalBorder;
		for (uint8_t* charBits = bitmap; charBits < bitmap + band->Settings->TextColumns; charBits++)
		{
			uint8_t colorValue;
			uint8_t pixels = *charBits;
			uint8_t foregroundColor = controller->createRawPixel(((uint8_t*)colors)[1]);
			uint8_t backgroundColor = controller->createRawPixel(((uint8_t*)colors)[0]);
			for (int bit = 0; bit < 8; bit++)
			{
				if ((pixels & 0x80) != 0)
				{
					colorValue = foregroundColor;
				}
				else
				{
					colorValue = backgroundColor;
				}

				VGA_PIXELINROW(dest, column) = colorValue;
				column++;

				pixels <<= 1;
			}
			colors++;
		}

        // Border to the right
        for (int column = band->HorizontalResolution - band->HorizontalBorder; column < band->HorizontalBorder; column++)
        {
            VGA_PIXELINROW(dest, column) = borderColor;
        }
    }
}

}