#include <ctype.h>

#include "ps2keyboard.h"

static fabgl::Keyboard* _keyboard;

void Ps2_Initialize(fabgl::PS2Controller* controller)
{
	_keyboard = controller->keyboard();
}

int32_t Ps2_GetScancode()
{
	int scanCode = _keyboard->getNextScancode(1);
	if (scanCode == 0xF0)
	{
		scanCode <<= 8;
		scanCode |= _keyboard->getNextScancode(1);
	}
	return scanCode;
}

/*
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
*/