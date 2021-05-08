
#include "z80input.h"
#include "ps2Input.h"

uint8_t indata[128];

const uint8_t keyaddr[ZX_KEY_LAST] = {
	0xFE, 0xFE, 0xFE, 0xFE, 0xFE, // ZX_KEY_SHIFT, ZX_KEY_Z,   ZX_KEY_X, ZX_KEY_C, ZX_KEY_V
	0xFD, 0xFD, 0xFD, 0xFD, 0xFD, // ZX_KEY_A,     ZX_KEY_S,   ZX_KEY_D, ZX_KEY_F, ZX_KEY_G
	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, // ZX_KEY_Q,     ZX_KEY_W,   ZX_KEY_E, ZX_KEY_R, ZX_KEY_T
	0xF7, 0xF7, 0xF7, 0xF7, 0xF7, // ZX_KEY_1,     ZX_KEY_2,   ZX_KEY_3, ZX_KEY_4, ZX_KEY_5
	0xEF, 0xEF, 0xEF, 0xEF, 0xEF, // ZX_KEY_0,     ZX_KEY_9,   ZX_KEY_8, ZX_KEY_7, ZX_KEY_6
	0xDF, 0xDF, 0xDF, 0xDF, 0xDF, // ZX_KEY_P,     ZX_KEY_O,   ZX_KEY_I, ZX_KEY_U, ZX_KEY_Y
	0xBF, 0xBF, 0xBF, 0xBF, 0xBF, // ZX_KEY_ENTER, ZX_KEY_L,   ZX_KEY_K, ZX_KEY_J, ZX_KEY_H
	0x7F, 0x7F, 0x7F, 0x7F, 0x7F, // ZX_KEY_SPACE, ZX_KEY_SYM, ZX_KEY_M, ZX_KEY_N, ZX_KEY_B
};

const uint8_t keybuf[ZX_KEY_LAST] = {
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_SHIFT, ZX_KEY_Z,   ZX_KEY_X, ZX_KEY_C, ZX_KEY_V
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_A,     ZX_KEY_S,   ZX_KEY_D, ZX_KEY_F, ZX_KEY_G
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_Q,     ZX_KEY_W,   ZX_KEY_E, ZX_KEY_R, ZX_KEY_T
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_1,     ZX_KEY_2,   ZX_KEY_3, ZX_KEY_4, ZX_KEY_5
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_0,     ZX_KEY_9,   ZX_KEY_8, ZX_KEY_7, ZX_KEY_6
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_P,     ZX_KEY_O,   ZX_KEY_I, ZX_KEY_U, ZX_KEY_Y
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_ENTER, ZX_KEY_L,   ZX_KEY_K, ZX_KEY_J, ZX_KEY_H
	0x01, 0x02, 0x04, 0x08, 0x10, // ZX_KEY_SPACE, ZX_KEY_SYM, ZX_KEY_M, ZX_KEY_N, ZX_KEY_B
};

#define ON_KEY(k, isKeyUp) isKeyUp ? indata[keyaddr[k] - 0x7F] |= keybuf[k] : indata[keyaddr[k] - 0x7F] &= ~keybuf[k]

bool OnKey(uint32_t scanCode, bool isKeyUp)
{
	switch (scanCode)
	{
	case KEY_LEFTSHIFT:
	case KEY_RIGHTSHIFT:
		ON_KEY(ZX_KEY_SHIFT, isKeyUp);
		break;
	case KEY_LEFTCONTROL:
	case KEY_RIGHTCONTROL:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		break;
	case KEY_ENTER:
    case KEY_KP_ENTER:
		ON_KEY(ZX_KEY_ENTER, isKeyUp);
		break;

	case KEY_SPACEBAR:
		ON_KEY(ZX_KEY_SPACE, isKeyUp);
		break;
	case KEY_0:
		ON_KEY(ZX_KEY_0, isKeyUp);
		break;
	case KEY_1:
		ON_KEY(ZX_KEY_1, isKeyUp);
		break;
	case KEY_2:
		ON_KEY(ZX_KEY_2, isKeyUp);
		break;
	case KEY_3:
		ON_KEY(ZX_KEY_3, isKeyUp);
		break;
	case KEY_4:
		ON_KEY(ZX_KEY_4, isKeyUp);
		break;
	case KEY_5:
		ON_KEY(ZX_KEY_5, isKeyUp);
		break;
	case KEY_6:
		ON_KEY(ZX_KEY_6, isKeyUp);
		break;
	case KEY_7:
		ON_KEY(ZX_KEY_7, isKeyUp);
		break;
	case KEY_8:
		ON_KEY(ZX_KEY_8, isKeyUp);
		break;
	case KEY_9:
		ON_KEY(ZX_KEY_9, isKeyUp);
		break;

	case KEY_A:
		ON_KEY(ZX_KEY_A, isKeyUp);
		break;
	case KEY_B:
		ON_KEY(ZX_KEY_B, isKeyUp);
		break;
	case KEY_C:
		ON_KEY(ZX_KEY_C, isKeyUp);
		break;
	case KEY_D:
		ON_KEY(ZX_KEY_D, isKeyUp);
		break;
	case KEY_E:
		ON_KEY(ZX_KEY_E, isKeyUp);
		break;
	case KEY_F:
		ON_KEY(ZX_KEY_F, isKeyUp);
		break;
	case KEY_G:
		ON_KEY(ZX_KEY_G, isKeyUp);
		break;
	case KEY_H:
		ON_KEY(ZX_KEY_H, isKeyUp);
		break;
	case KEY_I:
		ON_KEY(ZX_KEY_I, isKeyUp);
		break;
	case KEY_J:
		ON_KEY(ZX_KEY_J, isKeyUp);
		break;
	case KEY_K:
		ON_KEY(ZX_KEY_K, isKeyUp);
		break;
	case KEY_L:
		ON_KEY(ZX_KEY_L, isKeyUp);
		break;
	case KEY_M:
		ON_KEY(ZX_KEY_M, isKeyUp);
		break;
	case KEY_N:
		ON_KEY(ZX_KEY_N, isKeyUp);
		break;
	case KEY_O:
		ON_KEY(ZX_KEY_O, isKeyUp);
		break;
	case KEY_P:
		ON_KEY(ZX_KEY_P, isKeyUp);
		break;
	case KEY_Q:
		ON_KEY(ZX_KEY_Q, isKeyUp);
		break;
	case KEY_R:
		ON_KEY(ZX_KEY_R, isKeyUp);
		break;
	case KEY_S:
		ON_KEY(ZX_KEY_S, isKeyUp);
		break;
	case KEY_T:
		ON_KEY(ZX_KEY_T, isKeyUp);
		break;
	case KEY_U:
		ON_KEY(ZX_KEY_U, isKeyUp);
		break;
	case KEY_V:
		ON_KEY(ZX_KEY_V, isKeyUp);
		break;
	case KEY_W:
		ON_KEY(ZX_KEY_W, isKeyUp);
		break;
	case KEY_X:
		ON_KEY(ZX_KEY_X, isKeyUp);
		break;
	case KEY_Y:
		ON_KEY(ZX_KEY_Y, isKeyUp);
		break;
	case KEY_Z:
		ON_KEY(ZX_KEY_Z, isKeyUp);
		break;

    // "Convenience" buttons

    case KEY_MINUS:
    case KEY_KP_MINUS:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_J, isKeyUp);
		break;
    case KEY_EQUAL:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_L, isKeyUp);
		break;
    case KEY_COMMA:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_N, isKeyUp);
		break;
    case KEY_DOT:
    case KEY_KP_DOT:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_M, isKeyUp);
		break;
    case KEY_DIV:
    case KEY_KP_DIV:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_V, isKeyUp);
		break;
    case KEY_SEMI:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_O, isKeyUp);
		break;
    case KEY_KP_TIMES:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_B, isKeyUp);
		break;
    case KEY_KP_PLUS:
		ON_KEY(ZX_KEY_SYM, isKeyUp);
		ON_KEY(ZX_KEY_K, isKeyUp);
		break;
	case KEY_BACKSPACE:
		ON_KEY(ZX_KEY_SHIFT, isKeyUp);
		ON_KEY(ZX_KEY_0, isKeyUp);
		break;
    case KEY_LEFTARROW:
		ON_KEY(ZX_KEY_SHIFT, isKeyUp);
		ON_KEY(ZX_KEY_5, isKeyUp);
		break;
    case KEY_RIGHTARROW:
		ON_KEY(ZX_KEY_SHIFT, isKeyUp);
		ON_KEY(ZX_KEY_8, isKeyUp);
		break;
    case KEY_UPARROW:
		ON_KEY(ZX_KEY_SHIFT, isKeyUp);
		ON_KEY(ZX_KEY_7, isKeyUp);
		break;
    case KEY_DOWNARROW:
		ON_KEY(ZX_KEY_SHIFT, isKeyUp);
		ON_KEY(ZX_KEY_6, isKeyUp);
		break;
    default:
        return false;
	}

    return true;
}
