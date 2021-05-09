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
    // TODO
}

void ScreenArea::SetAttribute(uint16_t attribute)
{
	//this->_defaultAttribute
}

void ScreenArea::PrintAt(uint8_t x, uint8_t y, const char* str)
{
    this->SetCursorPosition(x, y);
    this->Print(str);
}

void ScreenArea::PrintAlignCenter(uint8_t y, const char *str)
{
    // TODO
    //uint8_t leftX = (this->Settings->TextColumns - strlen(str)) / 2;
    //this->PrintAt(leftX, y, str);
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
	this->_videoController->Print((char*)str);
}

void ScreenArea::SetCursorPosition(uint8_t x, uint8_t y)
{
    this->_videoController->SetCursorPosition(x + this->_xOffset, y + this->_yOffset);
}