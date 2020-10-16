#ifndef _PS2KEYBOARD_H_
#define _PS2KEYBOARD_H_

/* Single Byte Key Codes */
#define KEY_NUM      0x77
#define KEY_SCROLL   0x7E
#define KEY_CAPS     0x58
#define KEY_LEFTSHIFT 0x12
#define KEY_RIGHTSHIFT 0x59
#define KEY_LEFTCONTROL 0x14
#define KEY_ALT      0x11
#define KEY_ESC      0x76
#define KEY_BACKSPACE 0x66
#define KEY_TAB      0x0D
#define KEY_ENTER    0x5A
#define KEY_SPACEBAR 0x29
#define KEY_KP0      0x70
#define KEY_KP1      0x69
#define KEY_KP2      0x72
#define KEY_KP3      0x7A
#define KEY_KP4      0x6B
#define KEY_KP5      0x73
#define KEY_KP6      0x74
#define KEY_KP7      0x6C
#define KEY_KP8      0x75
#define KEY_KP9      0x7D
#define KEY_KP_DOT   0x71
#define KEY_KP_PLUS  0x79
#define KEY_KP_MINUS 0x7B
#define KEY_KP_TIMES 0x7C
#define KEY_0        0X45
#define KEY_1        0X16
#define KEY_2        0X1E
#define KEY_3        0X26
#define KEY_4        0X25
#define KEY_5        0X2E
#define KEY_6        0X36
#define KEY_7        0X3D
#define KEY_8        0X3E
#define KEY_9        0X46
#define KEY_APOS     0X52
#define KEY_COMMA    0X41
#define KEY_MINUS    0X4E
#define KEY_DOT      0X49
#define KEY_DIV      0X4A
/* Single quote or back apostrophe */
#define KEY_SINGLE   0X0E
#define KEY_A        0X1C
#define KEY_B        0X32
#define KEY_C        0X21
#define KEY_D        0X23
#define KEY_E        0X24
#define KEY_F        0X2B
#define KEY_G        0X34
#define KEY_H        0X33
#define KEY_I        0X43
#define KEY_J        0X3B
#define KEY_K        0X42
#define KEY_L        0X4B
#define KEY_M        0X3A
#define KEY_N        0X31
#define KEY_O        0X44
#define KEY_P        0X4D
#define KEY_Q        0X15
#define KEY_R        0X2D
#define KEY_S        0X1B
#define KEY_T        0X2C
#define KEY_U        0X3C
#define KEY_V        0X2A
#define KEY_W        0X1D
#define KEY_X        0X22
#define KEY_Y        0X35
#define KEY_Z        0X1A
#define KEY_SEMI     0X4C
#define KEY_BACK     0X5D
#define KEY_OPEN_SQ  0X54
#define KEY_CLOSE_SQ 0X5B
#define KEY_EQUAL    0X55
#define KEY_F1       0X05
#define KEY_F2       0X06
#define KEY_F3       0X04
#define KEY_F4       0X0C
#define KEY_F5       0X03
#define KEY_F6       0X0B
#define KEY_F7       0X83
#define KEY_F8       0X0A
#define KEY_F9       0X01
#define KEY_F10      0X09
#define KEY_F11      0X78
#define KEY_F12      0X07
#define KEY_KP_COMMA 0X6D

/* Extended key codes E0 table for two byte codes */
/* PS2_CTRL and PS2_ALT Need using in any table for the right keys */
/* first is special case for PRTSCR not always used so ignored by decoding */
#define KEY_IGNORE   0xE012
#define KEY_PRTSCR   0xE07C
#define KEY_RIGHTCONTROL 0XE014
/* Sometimes called windows key */
#define KEY_L_GUI    0xE01F
#define KEY_R_GUI    0xE027
#define KEY_MENU     0xE02F
/* Break is CTRL + PAUSE generated inside keyboard */
#define KEY_BREAK    0xE07E
#define KEY_HOME     0xE06C
#define KEY_END      0xE069
#define KEY_PGUP     0xE07D
#define KEY_PGDN     0xE07A
#define KEY_LEFTARROW 0xE06B
#define KEY_RIGHTARROW 0xE074
#define KEY_UPARROW 0xE075
#define KEY_DOWNARROW 0xE072
#define KEY_INSERT   0xE070
#define KEY_DELETE   0xE071
#define KEY_KP_ENTER 0xE05A
#define KEY_KP_DIV   0xE04A
#define KEY_NEXT_TR  0XE04D
#define KEY_PREV_TR  0XE015
#define KEY_STOP     0XE038
#define KEY_PLAY     0XE034
#define KEY_MUTE     0XE023
#define KEY_VOL_UP   0XE032
#define KEY_VOL_DN   0XE021
#define KEY_MEDIA    0XE050
#define KEY_EMAIL    0XE048
#define KEY_CALC     0XE02B
#define KEY_COMPUTER 0XE040

#include "fabgl.h"

void Ps2_Initialize(fabgl::PS2Controller* controller);

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
int32_t Ps2_GetScancode();
//char Ps2_ConvertScancode(int32_t scanCode);

#ifdef __cplusplus
}
#endif

#endif
