#ifndef __VIDEOCONTROLLER__
#define __VIDEOCONTROLLER__

#include <memory>
#include <map>
#include "fabgl.h"
#include "Settings.h"
#include "SpectrumScreenData.h"

using namespace std;

class VideoController : public fabgl::VGADirectController
{
public:
    uint8_t _mode = 1;

    // Mode 1
    uint8_t IRAM_ATTR createRawPixel(uint8_t color);
    uint8_t* _fontData;
    uint16_t Characters[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t* _defaultAttribute = nullptr;
    uint32_t** Attributes;
    uint16_t _leftOffset = 24; 
    uint16_t _topOffset = 16;
    uint16_t cursor_x = 0;
    uint16_t cursor_y = 0;

    // Mode 2
    SpectrumScreenData* Settings;
    uint8_t  BorderColor;
    uint16_t _borderWidth = 32; 
    uint16_t _borderHeight = 24;
    uint32_t Frames = 0;

    VideoController(SpectrumScreenData* screenData);
    void Start(char const* modeline);

    // Mode 1
	void Clear();
	void SetAttribute(uint16_t attribute);
	void SetCursorPosition(uint8_t x, uint8_t y);
	void ShowCursor();
	void HideCursor();
	void Print(const char* str);
	void PrintAt(uint8_t x, uint8_t y, const char* str);
	void PrintAlignRight(uint8_t y, const char *str);
	void PrintAlignCenter(uint8_t y, const char *str);

    // Mode 2
    void ShowScreenshot(const uint8_t* screenshot);

private:
    std::map<uint16_t, uint32_t*> _attrToAddr;
    std::map<uint32_t*, uint16_t> _addrToAttr;

    uint32_t* CreateAttribute(uint8_t foreColor, uint8_t backColor);
    void InitAttribute(uint32_t* attribute, uint8_t foreColor, uint8_t backColor);
    void cursorNext();
    void print(char* str, uint8_t foreColor, uint8_t backColor);
    void print(char* str);
    void print(const char* str, uint8_t foreColor, uint8_t backColor);
    void printChar(uint16_t x, uint16_t y, uint16_t ch);
    void printChar(uint16_t x, uint16_t y, uint16_t ch, uint8_t foreColor, uint8_t backColor);
    void setAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor);
    void freeUnusedAttributes();
};

#endif