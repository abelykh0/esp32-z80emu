#include "VideoController.h"
#include "fabutils.h"
#include "font8x8.h"
#include "z80Environment.h"

extern uint8_t* GetPixelPointer(uint8_t* pixels, uint16_t line);

#define BACK_COLOR 0x10
#define FORE_COLOR 0x3F

extern "C" void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

VideoController::VideoController(SpectrumScreenData* screenData)
{
    this->Settings = screenData;
}

void VideoController::InitAttribute(uint32_t* attribute, uint8_t foreColor, uint8_t backColor)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint8_t value = i;
        uint32_t attributeValue;
		for (uint8_t bit = 0; bit < 4; bit++)
		{
            VGA_PIXELINROW(((uint8_t*)&attributeValue), bit) = 
                this->createRawPixel(value & 0x08 ?  foreColor : backColor);
			value <<= 1;
		}
        
        *attribute = attributeValue;
        attribute++;
	}
}

void VideoController::Start(char const* modeline, void* characterBuffer)
{
    int charDataSize = 256 * 8;
    this->_fontData = (uint8_t*)characterBuffer;
    memcpy(this->_fontData, font8x8, charDataSize);

    // "default" attribute (white on blue)
    this->_defaultAttribute = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);

    this->Attributes = (uint32_t**)heap_caps_malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4, MALLOC_CAP_32BIT);

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        this->Characters[i] = ' ';
        this->Attributes[i] = this->_defaultAttribute;
    }

    this->setDrawScanlineCallback(drawScanline, this);

    this->begin();
    this->setResolution(modeline);

    this->InitAttribute(this->_defaultAttribute, FORE_COLOR, BACK_COLOR);
}

void VideoController::Print(const char* str)
{
	this->print((char*)str);
}

void VideoController::setAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor)
{
    uint32_t* attribute;
    uint16_t colors = foreColor << 8 | backColor;

    if (colors == 0xFFFF)
    {
        attribute = this->_defaultAttribute;
    }
    else
    {

        auto iterator = this->_attrToAddr.find(colors);
        if (iterator == this->_attrToAddr.end())
        {
            // Missing
            attribute = this->CreateAttribute(foreColor, backColor);
            this->_attrToAddr.insert(make_pair(colors, attribute));
        }
        else
        {
            attribute = iterator->second;
        }
    }

    this->Attributes[y * SCREEN_WIDTH + x] = attribute;
}

void VideoController::Clear()
{
    // TODO
}

void VideoController::SetAttribute(uint16_t attribute)
{
	//this->_attribute = attribute;
}

void VideoController::PrintAt(uint8_t x, uint8_t y, const char* str)
{
    this->SetCursorPosition(x, y);
    this->Print(str);
}

void VideoController::PrintAlignCenter(uint8_t y, const char *str)
{
    // TODO
    //uint8_t leftX = (this->Settings->TextColumns - strlen(str)) / 2;
    //this->PrintAt(leftX, y, str);
}

void VideoController::ShowCursor()
{
    /*
    if (!this->_isCursorVisible)
    {
    	this->_isCursorVisible = true;
    	this->InvertColor();
    }
    */
}

void VideoController::HideCursor()
{
    /*
    if (this->_isCursorVisible)
    {
    	this->_isCursorVisible = false;
    	this->InvertColor();
    }
    */
}

void VideoController::printChar(uint16_t x, uint16_t y, uint16_t ch)
{
	VideoController::printChar(x, y, ch, 0xFF, 0xFF);
}
void VideoController::printChar(uint16_t x, uint16_t y, uint16_t ch, uint8_t foreColor, uint8_t backColor)
{
	if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
	{
		// Invalid
		return;
	}

	int offset = y * SCREEN_WIDTH + x;
	this->Characters[offset] = ch;
    this->setAttribute(x, y, foreColor, backColor);
}
void VideoController::print(const char* str, uint8_t foreColor, uint8_t backColor)
{
	print((char*)str, foreColor, backColor);
}

void VideoController::print(char* str)
{
	this->print(str, 0xFF, 0xFF);
}
void VideoController::print(char* str, uint8_t foreColor, uint8_t backColor)
{
    while (*str)
    {
    	printChar(cursor_x, cursor_y, *str++, foreColor, backColor);
    	cursorNext();
    }
}

void VideoController::cursorNext()
{
    uint8_t x = cursor_x;
    uint8_t y = cursor_y;
    if (x < SCREEN_WIDTH - 1)
    {
        x++;
    }
    else
    {
        if (y < SCREEN_HEIGHT - 1)
        {
            x = 0;
            y++;
        }
    }

    this->SetCursorPosition(x, y);
}

void VideoController::SetCursorPosition(uint8_t x, uint8_t y)
{
	cursor_x = x;
	cursor_y = y;
	if (cursor_x >= SCREEN_WIDTH)
	{
		cursor_x = SCREEN_WIDTH - 1;
	}
	if (cursor_y >= SCREEN_HEIGHT)
	{
		cursor_y = SCREEN_HEIGHT - 1;
	}
}

void VideoController::freeUnusedAttributes()
{
    for (auto it = this->_attrToAddr.begin(); it != this->_attrToAddr.end(); it++)
    {
        uint32_t* attribute = it->second;

        bool found = false;
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        {
            if (this->Attributes[i] == attribute)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {   
            free(attribute);
            this->_attrToAddr.erase(it->first);
        }
    }   
}

uint32_t* VideoController::CreateAttribute(uint8_t foreColor, uint8_t backColor)
{
    uint32_t* attribute = (uint32_t*)heap_caps_malloc(16 * 4, MALLOC_CAP_32BIT);
    this->InitAttribute(attribute, foreColor, backColor);
    return attribute;
}

uint8_t IRAM_ATTR VideoController::createRawPixel(uint8_t color)
{
    // HACK: should call createRawPixel() instead
    return m_HVSync | color;
}

void VideoController::ShowScreenshot(const uint8_t* screenshot)
{
	memcpy(this->Settings->Pixels, screenshot, SPECTRUM_WIDTH * SPECTRUM_HEIGHT * 8);
    uint8_t* attributes = (uint8_t*)screenshot + SPECTRUM_WIDTH * SPECTRUM_HEIGHT * 8;
	for (uint32_t i = 0; i < SPECTRUM_WIDTH * SPECTRUM_HEIGHT; i++)
	{
		this->Settings->Attributes[i] = Z80Environment::FromSpectrumColor(attributes[i]);
	}
}

uint8_t* IRAM_ATTR GetPixelPointer(uint8_t* pixels, uint16_t line)
{
	// ZX Sinclair addressing
	// 00-00-00-Y7-Y6-Y2-Y1-Y0 Y5-Y4-Y3-x4-x3-x2-x1-x0
	//          12 11 10  9  8  7  6  5  4  3  2  1  0

	uint32_t y012 = ((line & 0B00000111) << 8);
	uint32_t y345 = ((line & 0B00111000) << 2);
	uint32_t y67 =  ((line & 0B11000000) << 5);
	return &pixels[y012 | y345 | y67];
}

void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine)
{
    auto controller = static_cast<VideoController*>(arg);

    uint8_t mode = controller->_mode;
    if (mode == 1)
    {
        // "Debug" screen

        int y = scanLine / 8;
        int fontRow = scanLine % 8;
        int startCoord = y * SCREEN_WIDTH;

        uint16_t* characters = (uint16_t*)(controller->Characters + startCoord);
        uint32_t** attributes = controller->Attributes + startCoord;
        uint32_t* dest32 = (uint32_t*)dest;
        uint32_t** lastAttribute = attributes + SCREEN_WIDTH - 1;
        uint8_t* fontData = controller->_fontData + fontRow;

        do
        {
            uint16_t character = *characters;
            uint8_t fontPixels = fontData[character * 8];
            uint32_t* attribute = *attributes;
            dest32[0] = attribute[fontPixels >> 4];
            dest32[1] = attribute[fontPixels & 0x0F];

            dest32 += 2;
            attributes++;
            characters++;
        } while (attributes <= lastAttribute);
    }
    else
    {
        // Spectrum screen

        unsigned scaledLine = scanLine / 2;
        if (scaledLine < controller->_borderHeight
            || scaledLine > SPECTRUM_HEIGHT * 8 + controller->_borderHeight)
        {
            memset(dest, controller->createRawPixel(controller->BorderColor), SCREEN_WIDTH * 8);
        }
        else
        {
            uint16_t* dest16 = (uint16_t*)dest;

            // Border on the left
            memset(dest, controller->createRawPixel(controller->BorderColor), controller->_borderWidth);
            dest16 += controller->_borderWidth;

            // Screen pixels
            uint16_t vline = scaledLine - controller->_borderHeight;
            uint8_t* bitmap = (uint8_t*)GetPixelPointer(controller->Settings->Pixels, vline);
            uint16_t* colors = &controller->Settings->Attributes[vline / 8 * SPECTRUM_WIDTH];
            //int column = band->HorizontalBorder;
            for (uint8_t* charBits = bitmap; charBits < bitmap + SPECTRUM_WIDTH; charBits++)
            {
                uint8_t pixels = *charBits;
                uint16_t foregroundColor = controller->createRawPixel(((uint8_t*)colors)[1]);
                foregroundColor |= foregroundColor << 8;
                uint16_t backgroundColor = controller->createRawPixel(((uint8_t*)colors)[0]);
                backgroundColor |= backgroundColor << 8;
                for (int bit = 0; bit < 8; bit++)
                {
                    if ((bit & 0x01) == 0)
                    {
                        if ((pixels & 0x40) != 0)
                        {
                            *dest16 = foregroundColor;
                        }
                        else
                        {
                            *dest16 = backgroundColor;
                        }
                    }
                    else
                    {
                        if ((pixels & 0x80) != 0)
                        {
                            *dest16 = foregroundColor;
                        }
                        else
                        {
                            *dest16 = backgroundColor;
                        }
                        pixels <<= 2;
                    }

                    dest16++;
                }

                colors++;
            }

            // Border on the right
            memset(dest, controller->createRawPixel(controller->BorderColor), controller->_borderWidth);
        }
    }
}
