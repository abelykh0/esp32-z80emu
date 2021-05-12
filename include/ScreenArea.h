#ifndef _SCREENAREA_H
#define _SCREENAREA_H

#include "VideoController.h"

class ScreenArea
{
private:
    VideoController* _videoController;
    uint16_t _xOffset;
    uint16_t _yOffset;
    uint16_t _Width;
    uint16_t _Height;
    uint8_t foreColor = 0xFF;
    uint8_t backColor = 0xFF;

public:
    uint16_t getX();
    uint16_t getY();

    ScreenArea(VideoController* videoController, 
        uint16_t xOffset, uint16_t width,
        uint16_t yOffset, uint16_t height);

	void Clear();
    void SetAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor);
	void SetCursorPosition(uint8_t x, uint8_t y);
	void ShowCursor();
	void HideCursor();
    void SetPrintAttribute(uint16_t attribute);
	void SetPrintAttribute(uint8_t foreColor, uint8_t backColor);
	void Print(const char* str);
	void PrintAt(uint8_t x, uint8_t y, const char* str);
	void PrintAlignRight(uint8_t y, const char *str);
	void PrintAlignCenter(uint8_t y, const char *str);
};

#endif
