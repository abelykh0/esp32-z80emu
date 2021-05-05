
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define ZX128K

///////////////////////////////////////////////////////////////////////////////
// CPU core selection
//
// one of the following MUST be defined:
// - CPU_LINKEFONG: use Lin Ke-Fong's core           https://github.com/anotherlin/z80emu
// - CPU_ANDREWEISSFLOG: use Andre Weissflog's core  https://github.com/floooh/chips/
// - CPU_JLSANCHEZ: use José Luis Sánchez's core     https://github.com/jsanchezv/z80cpp
///////////////////////////////////////////////////////////////////////////////
//#define CPU_LINKEFONG
//#define CPU_ANDREWEISSFLOG
#define CPU_JLSANCHEZ

#define SDCARD
#define BEEPER

/*
#define RESOLUTION QVGA_320x240_60Hz
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define DEBUG_COLUMNS 40
#define DEBUG_ROWS 5
*/

#define RESOLUTION VGA_400x300_60Hz
#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 300
#define DEBUG_COLUMNS 50
#define DEBUG_ROWS 8

#endif
