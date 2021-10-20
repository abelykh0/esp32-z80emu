#include "VideoController.h"
#include "fabutils.h"
#include "font8x8.h"
#include "z80Environment.h"

#define BACK_COLOR 0x10
#define FORE_COLOR 0x3F

static uint32_t _borderAttribute;

extern uint8_t* IRAM_ATTR GetPixelPointer(uint8_t* pixels, uint16_t line);
extern "C" void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine);

VideoController::VideoController(SpectrumScreenData* screenData)
{
    this->Settings = screenData;
	this->_spectrumAttributes = (uint32_t*)heap_caps_malloc(SPECTRUM_WIDTH * SPECTRUM_HEIGHT * 8 * 2 * 4, MALLOC_CAP_32BIT);
}

void VideoController::Start(char const* modeline)
{
#ifdef SDCARD
    this->_fontData = (uint8_t*)font8x8;
#else
    this->_fontData = (uint8_t*)heap_caps_malloc(256 * 8, MALLOC_CAP_8BIT);
    memcpy(this->_fontData, font8x8, 256 * 8);
#endif

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
    this->setScanlinesPerCallBack(2);
    this->setResolution(modeline);

    this->InitAttribute(this->_defaultAttribute, FORE_COLOR, BACK_COLOR);

    this->prepareDebugScreen();
}   

void VideoController::prepareDebugScreen()
{
    // Display frame
    this->printChar(0, 0, '\xC9'); // ╔
    this->printChar(SCREEN_WIDTH - 1, 0, '\xBB');  // ╗
    this->printChar(0, SCREEN_HEIGHT - 1, '\xC8'); // ╚
    this->printChar(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, '\xBC'); // ╝
    for (int i = 1; i < SCREEN_WIDTH - 1; i++)
    {
    	this->printChar(i, 0, '\x0CD'); // ═
    	this->printChar(i, SCREEN_HEIGHT - 1, '\x0CD'); // ═
    }
    for (int i = 1; i < SCREEN_HEIGHT - 1; i++)
    {
    	this->printChar(0, i, '\x0BA'); // ║
    	this->printChar(SCREEN_WIDTH - 1, i, '\x0BA'); // ║
    }

    // Border around Spectrum screen
    for (int i = 1; i <= SPECTRUM_WIDTH_WITH_BORDER; i++)
    {
    	this->printChar(i, SPECTRUM_HEIGHT_WITH_BORDER + 1, '\x0CD'); // ═
    }
    for (int i = 1; i < SCREEN_HEIGHT - 1; i++)
    {
    	this->printChar(SPECTRUM_WIDTH_WITH_BORDER + 1, i, '\x0BA'); // ║
    }
    this->printChar(SPECTRUM_WIDTH_WITH_BORDER + 1, 0, '\xCB');  // ╦
    this->printChar(0, SPECTRUM_HEIGHT_WITH_BORDER + 1, '\xCC'); // ╠
    this->printChar(SPECTRUM_WIDTH_WITH_BORDER + 1, SPECTRUM_HEIGHT_WITH_BORDER + 1, '\xB9'); // ╣
    this->printChar(SPECTRUM_WIDTH_WITH_BORDER + 1, SCREEN_HEIGHT - 1, '\xCA'); // ╩

    // Spectrum border
    for (int x = 1; x <= SPECTRUM_WIDTH_WITH_BORDER; x++)
    {
    	this->printChar(x, 1, ' ');
    	this->printChar(x, SPECTRUM_HEIGHT_WITH_BORDER, ' ');
    }
    for (int y = 1; y <= SPECTRUM_HEIGHT_WITH_BORDER; y++)
    {
    	this->printChar(1, y, ' ');
    	this->printChar(2, y, ' ');
    	this->printChar(SPECTRUM_WIDTH_WITH_BORDER - 1, y, ' ');
    	this->printChar(SPECTRUM_WIDTH_WITH_BORDER, y, ' ');
    }

    // Spectrum content
    for (int y = 2; y < SPECTRUM_HEIGHT_WITH_BORDER; y++)
    {
        for (int x = 3; x < SPECTRUM_WIDTH_WITH_BORDER - 1; x++)
        {
            this->printChar(x, y, 0);
        }
    }
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

void VideoController::Print(const char* str)
{
	this->print((char*)str);
}

void VideoController::SetAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor)
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
    this->SetAttribute(x, y, foreColor, backColor);
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
	this->cursor_x = x;
	this->cursor_y = y;
	if (this->cursor_x >= SCREEN_WIDTH)
	{
		this->cursor_x = SCREEN_WIDTH - 1;
	}
	if (this->cursor_y >= SCREEN_HEIGHT)
	{
		this->cursor_y = SCREEN_HEIGHT - 1;
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

void VideoController::ShowScreenshot(const uint8_t* screenshot, uint8_t borderColor)
{
    uint8_t border = Z80Environment::FromSpectrumColor(borderColor) >> 8;
    this->showScreenshot((uint8_t*)screenshot, nullptr, border);
}

void VideoController::ShowScreenshot()
{
    this->showScreenshot(this->Settings->Pixels, this->Settings->Attributes, *this->BorderColor);
}

void VideoController::showScreenshot(
    uint8_t* pixelData, 
    uint16_t* attributes,
    uint8_t borderColor)
{
    // Screenshot
    uint8_t* attrData8 = nullptr;
    if (attributes == nullptr)
    {
        attrData8 = (uint8_t*)pixelData + SPECTRUM_WIDTH * SPECTRUM_HEIGHT * 8;
    }

	uint32_t* spectrumAttributes = this->_spectrumAttributes;
    for (int y = 0; y < SPECTRUM_HEIGHT; y++)
    {
        for (int x = 0; x < SPECTRUM_WIDTH; x++)
        {
			this->Attributes[(y + this->_topOffset / 8) * SCREEN_WIDTH + x + (this->_leftOffset / 8)] = spectrumAttributes;
			uint16_t colors;
            if (attributes == nullptr)
            {
                colors = Z80Environment::FromSpectrumColor(*attrData8);
            }
            else
            {
                colors = *attributes;
            }
			uint8_t foreColor = colors >> 8;
			uint8_t backColor = colors & 0xFF;

            for (int y2 = 0; y2 < 8; y2++)
            {
				uint8_t* charPixels = GetPixelPointer(pixelData, y * 8 + y2) + x;
				uint8_t pixels = *charPixels;

				uint32_t attributeValue;
				for (uint8_t halfByte = 0; halfByte < 2; halfByte++)
				{
					for (uint8_t bit = 0; bit < 4; bit++)
					{
						VGA_PIXELINROW(((uint8_t*)&attributeValue), bit) = 
							this->createRawPixel(pixels & 0x80 ?  foreColor : backColor);
						pixels <<= 1;
					}

					*spectrumAttributes = attributeValue;
					spectrumAttributes++;
				}
			}

            if (attributes == nullptr)
            {
                attrData8++;
            }
            else
            {
                attributes++;
            }
        }
    }

    // Border
    uint8_t rawPixel = this->createRawPixel(borderColor);
    uint32_t attributeValue = rawPixel << 24 | rawPixel << 16 | rawPixel << 8 | rawPixel;
    _borderAttribute = attributeValue;
    for (int x = 1; x <= SPECTRUM_WIDTH_WITH_BORDER; x++)
    {
    	this->Attributes[SCREEN_WIDTH + x] = &_borderAttribute;
    	this->Attributes[SPECTRUM_HEIGHT_WITH_BORDER * SCREEN_WIDTH + x] = &_borderAttribute;
    }
    for (int y = 1; y <= SPECTRUM_HEIGHT_WITH_BORDER; y++)
    {
    	this->Attributes[y * SCREEN_WIDTH + 1] = &_borderAttribute;
    	this->Attributes[y * SCREEN_WIDTH + 2] = &_borderAttribute;
    	this->Attributes[y * SCREEN_WIDTH + SPECTRUM_WIDTH_WITH_BORDER - 1] = &_borderAttribute;
    	this->Attributes[y * SCREEN_WIDTH + SPECTRUM_WIDTH_WITH_BORDER] = &_borderAttribute;
    }
}

uint8_t IRAM_ATTR VideoController::createRawPixel(uint8_t color)
{
    // HACK: should call createRawPixel() instead
    return this->m_HVSync | color;
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

void VideoController::SetMode(uint8_t mode)
{
    //this->setScanlinesPerCallBack(mode == 1 ? 2 : 1);
    this->_mode = mode;
}

void IRAM_ATTR drawScanline(void* arg, uint8_t* dest, int scanLine)
{
    auto controller = static_cast<VideoController*>(arg);
    if (scanLine == 0)
    {
        controller->Frames++;
    }

    uint8_t mode = controller->_mode;
    if (mode == 1)
    {
        // "Debug" screen

        int y = scanLine / 8;
        int fontRow = scanLine % 8;
        int startCoord = y * SCREEN_WIDTH;

        uint8_t* characters = controller->Characters + startCoord;
        uint32_t** attributes = controller->Attributes + startCoord;
        uint32_t* dest32 = (uint32_t*)dest;
        uint32_t** lastAttribute = attributes + SCREEN_WIDTH - 1;
        uint8_t* fontData = controller->_fontData + fontRow;

        do
        {
            uint8_t character = *characters;
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
        uint16_t* dest16 = (uint16_t*)dest;

        if (scaledLine < controller->_borderHeight
            || scaledLine >= SPECTRUM_HEIGHT * 8 + controller->_borderHeight)
        {
            memset(dest16, controller->createRawPixel(*controller->BorderColor), SCREEN_WIDTH * 8);
        }
        else
        {
            // Border on the left
            memset(dest16, controller->createRawPixel(*controller->BorderColor), controller->_borderWidth);
            dest16 += controller->_borderWidth;

            // Screen pixels
            uint16_t vline = scaledLine - controller->_borderHeight;
            uint8_t* bitmap = (uint8_t*)GetPixelPointer(controller->Settings->Pixels, vline);
            uint16_t* colors = &controller->Settings->Attributes[vline / 8 * SPECTRUM_WIDTH];
            for (uint8_t* charBits = bitmap; charBits < bitmap + SPECTRUM_WIDTH; charBits++)
            {
                uint8_t pixels = *charBits;
                uint16_t foregroundColor = controller->createRawPixel(((uint8_t*)colors)[1]);
                foregroundColor |= foregroundColor << 8;
                uint16_t backgroundColor = controller->createRawPixel(((uint8_t*)colors)[0]);
                backgroundColor |= backgroundColor << 8;
                for (uint16_t* endDest16 = dest16 + 8; dest16 < endDest16; )
                {
                    if ((pixels & 0x40) != 0)
                    {
                        *dest16 = foregroundColor;
                    }
                    else
                    {
                        *dest16 = backgroundColor;
                    }

                    dest16++;

                    if ((pixels & 0x80) != 0)
                    {
                        *dest16 = foregroundColor;
                    }
                    else
                    {
                        *dest16 = backgroundColor;
                    }

                    pixels <<= 2;
                    dest16++;
                }

                colors++;
            }

            // Border on the right
            memset(dest16, controller->createRawPixel(*controller->BorderColor), controller->_borderWidth);
        }
    }
}
