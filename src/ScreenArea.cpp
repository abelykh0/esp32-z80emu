#include "ScreenArea.h"    
    
ScreenArea::ScreenArea(VideoController* videoController,
        uint16_t xOffset, uint16_t width,
        uint16_t yOffset, uint16_t height)
{
    this->_videoController = videoController;
    this->_xOffset = xOffset;
    this->_Width = width;
    this->_yOffset = yOffset;
    this->_Height = height;
}

void ScreenArea::Clear()
{
    for (int y = this->_yOffset; y < this->_yOffset + this->_Height; y++)
    {
        for (int x = this->_xOffset; x < this->_xOffset + this->_Width; x++)
        {
            int i = y * SCREEN_WIDTH + x;
            this->_videoController->Characters[i] = ' ';
            this->_videoController->Attributes[i] = this->_videoController->_defaultAttribute;
        }
    }
}

void ScreenArea::SetAttribute(uint16_t attribute)
{
	this->backColor = attribute >> 8;
	this->foreColor = attribute & 0xFF;
}

void ScreenArea::PrintAt(uint8_t x, uint8_t y, const char* str)
{
    this->SetCursorPosition(x, y);
    this->Print(str);
}

void ScreenArea::PrintAlignCenter(uint8_t y, const char *str)
{
    uint8_t leftX = (this->_Width - strlen(str)) / 2;
    this->PrintAt(leftX, y, str);
}

void ScreenArea::ShowCursor()
{
    /*
    if (!this->_isCursorVisible)
    {
    	this->_isCursorVisible = true;
    	this->InvertColor();
    }
    */
}

void ScreenArea::HideCursor()
{
    /*
    if (this->_isCursorVisible)
    {
    	this->_isCursorVisible = false;
    	this->InvertColor();
    }
    */
}

void ScreenArea::Print(const char* str)
{
	this->_videoController->print((char*)str, this->foreColor, this->backColor);
}

void ScreenArea::SetCursorPosition(uint8_t x, uint8_t y)
{
    this->_videoController->SetCursorPosition(x + this->_xOffset, y + this->_yOffset);
}