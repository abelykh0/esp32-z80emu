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

public:
    uint16_t cursor_x = 0;
    uint16_t cursor_y = 0;

    ScreenArea(VideoController* videoController, 
        uint16_t xOffset, uint16_t width,
        uint16_t yOffset, uint16_t height);

	void Clear();
	void SetAttribute(uint16_t attribute);
	void SetCursorPosition(uint8_t x, uint8_t y);
	void ShowCursor();
	void HideCursor();
	void Print(const char* str);
	void PrintAt(uint8_t x, uint8_t y, const char* str);
	void PrintAlignRight(uint8_t y, const char *str);
	void PrintAlignCenter(uint8_t y, const char *str);
};

#endif
