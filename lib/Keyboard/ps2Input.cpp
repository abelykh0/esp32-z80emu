#include <ctype.h>
#include <map>

#include "ps2Input.h"

using namespace fabgl;

static Keyboard* _keyboard;
static Mouse* _mouse;

static bool _isLeftShiftPressed;
static bool _isRightShiftPressed;

// Used to convert virtual key back to scan code
static std::map<VirtualKey, uint32_t> _virtualKeyMap;

void Ps2_Initialize(PS2Controller* inputController)
{
	_keyboard = inputController->keyboard();
    _mouse = inputController->mouse();
    _mouse->setupAbsolutePositioner(256, 256, false);

	// Used to convert virtual key back to scan code
	const KeyboardLayout* layout = _keyboard->getLayout();
	for (VirtualKeyDef keyDef: layout->scancodeToVK)
	{
		_virtualKeyMap[keyDef.virtualKey] = keyDef.scancode;
	}
	for (VirtualKeyDef keyDef: layout->exScancodeToVK)
	{
		_virtualKeyMap[keyDef.virtualKey] = 0xE000 | keyDef.scancode;
	}
	for (AltVirtualKeyDef keyDef: layout->alternateVK)
	{
		_virtualKeyMap[keyDef.virtualKey] = _virtualKeyMap[keyDef.reqVirtualKey];
	}
}

bool Ps2_isMouseAvailable()
{
    bool result = _mouse != nullptr && _mouse->isMouseAvailable();
    if (result && _mouse->deltaAvailable())
    {
        MouseDelta delta;
        _mouse->getNextDelta(&delta);
        _mouse->updateAbsolutePosition(&delta);
    }
    return result;
}

uint8_t Ps2_getMouseButtons()
{
    uint8_t result = 0xFF;
    MouseButtons buttons = _mouse->status().buttons;
    result &= buttons.right ? 0xFE : 0xFF;
    result &= buttons.left ? 0xFD : 0xFF;
    result &= buttons.middle ? 0xFB : 0xFF;
    return result;
}

uint8_t Ps2_getMouseX()
{
    return (uint8_t)_mouse->status().X;
}

uint8_t Ps2_getMouseY()
{
    return 0xFF - (uint8_t)_mouse->status().Y;
}

int32_t Ps2_GetScancode()
{
	if (!_keyboard->virtualKeyAvailable())
	{
		return 0;
	}

	bool keyDown;
	VirtualKey virtualKey = _keyboard->getNextVirtualKey(&keyDown, 0);

/*
    Serial.print("virtualKey = ");
	Serial.print((int)virtualKey);
    Serial.print(",");
	Serial.println((int)keyDown);
*/

	auto pos = _virtualKeyMap.find(virtualKey);
	if (pos == _virtualKeyMap.end()) 
	{
		return 0;
	} 
	
	uint32_t result = pos->second;
	result = ((result << 8) & 0xFF0000) | (result & 0xFF);
	if (!keyDown)
	{
		result |= 0xF000;
	}

	return result;
}

char Ps2_ConvertScancode(int32_t scanCode)
{
	char result;

	switch (scanCode)
	{
	case KEY_A:
		result = 'a';
		break;
	case KEY_B:
		result = 'b';
		break;
	case KEY_C:
		result = 'c';
		break;
	case KEY_D:
		result = 'd';
		break;
	case KEY_E:
		result = 'e';
		break;
	case KEY_F:
		result = 'f';
		break;
	case KEY_G:
		result = 'g';
		break;
	case KEY_H:
		result = 'h';
		break;
	case KEY_I:
		result = 'i';
		break;
	case KEY_J:
		result = 'j';
		break;
	case KEY_K:
		result = 'k';
		break;
	case KEY_L:
		result = 'l';
		break;
	case KEY_M:
		result = 'm';
		break;
	case KEY_N:
		result = 'n';
		break;
	case KEY_O:
		result = 'o';
		break;
	case KEY_P:
		result = 'p';
		break;
	case KEY_Q:
		result = 'q';
		break;
	case KEY_R:
		result = 'r';
		break;
	case KEY_S:
		result = 's';
		break;
	case KEY_T:
		result = 't';
		break;
	case KEY_U:
		result = 'u';
		break;
	case KEY_V:
		result = 'v';
		break;
	case KEY_W:
		result = 'w';
		break;
	case KEY_X:
		result = 'x';
		break;
	case KEY_Y:
		result = 'y';
		break;
	case KEY_Z:
		result = 'z';
		break;
	case KEY_0:
		result = '0';
		break;
	case KEY_1:
		result = '1';
		break;
	case KEY_2:
		result = '2';
		break;
	case KEY_3:
		result = '3';
		break;
	case KEY_4:
		result = '4';
		break;
	case KEY_5:
		result = '5';
		break;
	case KEY_6:
		result = '6';
		break;
	case KEY_7:
		result = '7';
		break;
	case KEY_8:
		result = '8';
		break;
	case KEY_9:
		result = '9';
		break;
	case KEY_BACKSPACE:
		result = '\b';
		break;
	case KEY_SPACEBAR:
		result = ' ';
		break;
	case KEY_COMMA:
		result = ',';
		break;
	case KEY_MINUS:
		result = '-';
		break;
	case KEY_DOT:
		result = '.';
		break;
	case KEY_DIV:
		result = '/';
		break;
	case KEY_SINGLE:
		result = '`';
		break;
	case KEY_APOS:
		result = '\'';
		break;
	case KEY_SEMI:
		result = ';';
		break;
	case KEY_BACK:
		result = '\\';
		break;
	case KEY_OPEN_SQ:
		result = '[';
		break;
	case KEY_CLOSE_SQ:
		result = ']';
		break;
	case KEY_EQUAL:
		result = '=';
		break;
	default:
		result = '\0';
		break;
	}

	if (_isLeftShiftPressed || _isRightShiftPressed)
	{
		switch (scanCode)
		{
		case KEY_0:
			result = ')';
			break;
		case KEY_1:
			result = '!';
			break;
		case KEY_2:
			result = '@';
			break;
		case KEY_3:
			result = '#';
			break;
		case KEY_4:
			result = '$';
			break;
		case KEY_5:
			result = '%';
			break;
		case KEY_6:
			result = '^';
			break;
		case KEY_7:
			result = '&';
			break;
		case KEY_8:
			result = '*';
			break;
		case KEY_9:
			result = '(';
			break;
		case KEY_COMMA:
			result = '<';
			break;
		case KEY_MINUS:
			result = '_';
			break;
		case KEY_DOT:
			result = '>';
			break;
		case KEY_DIV:
			result = '?';
			break;
		case KEY_SINGLE:
			result = '~';
			break;
		case KEY_APOS:
			result = '"';
			break;
		case KEY_SEMI:
			result = ':';
			break;
		case KEY_BACK:
			result = '|';
			break;
		case KEY_OPEN_SQ:
			result = '{';
			break;
		case KEY_CLOSE_SQ:
			result = '}';
			break;
		case KEY_EQUAL:
			result = '+';
			break;
		default:
			result = toupper(result);
			break;
		}
	}

	return result;
}
