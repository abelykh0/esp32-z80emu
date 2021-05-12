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

void ScreenArea::SetPrintAttribute(uint16_t attribute)
{
	this->foreColor = attribute >> 8;
	this->backColor = attribute & 0xFF;
}

void ScreenArea::SetPrintAttribute(uint8_t foreColor, uint8_t backColor)
{
	this->foreColor = foreColor;
	this->backColor = backColor;
}

void ScreenArea::SetAttribute(uint8_t x, uint8_t y, uint8_t foreColor, uint8_t backColor)
{
	this->_videoController->SetAttribute(x + this->_xOffset, y + this->_yOffset, foreColor, backColor);
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

uint16_t ScreenArea::getX()
{
    if (this->_videoController->cursor_x < this->_xOffset)
    {
        return 0;
    }
    else
    {
        return this->_videoController->cursor_x - this->_xOffset;
    }
}

uint16_t ScreenArea::getY()
{
    if (this->_videoController->cursor_y < this->_yOffset)
    {
        return 0;
    }
    else
    {
        return this->_videoController->cursor_y - this->_yOffset;
    }    
}

void ScreenArea::ShowCursor()
{
    if (!this->_isCursorVisible)
    {
    	this->_isCursorVisible = true;
    	this->SetAttribute(this->getX(), this->getY(), this->backColor, this->foreColor);
    }
}

void ScreenArea::HideCursor()
{
    if (this->_isCursorVisible)
    {
    	this->_isCursorVisible = false;
    	this->SetAttribute(this->getX(), this->getY(), this->foreColor, this->backColor);
    }
}

void ScreenArea::Print(const char* str)
{
    Serial.printf("x=%d y=%d\r\n", this->getX(), this->getY());
    if (this->_isCursorVisible)
    {
    	this->SetAttribute(this->getX(), this->getY(), this->foreColor, this->backColor);
    }
	this->_videoController->print((char*)str, this->foreColor, this->backColor);
    if (this->_isCursorVisible)
    {
    	this->SetAttribute(this->getX(), this->getY(), this->backColor, this->foreColor);
    }
}

void ScreenArea::SetCursorPosition(uint8_t x, uint8_t y)
{
    if (this->_isCursorVisible)
    {
    	this->SetAttribute(this->getX(), this->getY(), this->foreColor, this->backColor);
    }
    this->_videoController->SetCursorPosition(x + this->_xOffset, y + this->_yOffset);
    if (this->_isCursorVisible)
    {
    	this->SetAttribute(this->getX(), this->getY(), this->backColor, this->foreColor);
    }
}