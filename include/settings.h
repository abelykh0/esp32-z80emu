
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define ZX128K

///////////////////////////////////////////////////////////////////////////////
// CPU core selection
//
// one of the following MUST be defined:
// - CPU_LINKEFONG: use Lin Ke-Fong's core           https://github.com/anotherlin/z80emu
// - CPU_JLSANCHEZ: use José Luis Sánchez's core     https://github.com/jsanchezv/z80cpp
// - CPU_STEVECHECKOWAY: use Steve Checkoway's core  https://github.com/stevecheckoway/libzel
// - CPU_ANDREWEISSFLOG: use Andre Weissflog's core  https://github.com/floooh/chips
///////////////////////////////////////////////////////////////////////////////
//#define CPU_LINKEFONG
//#define CPU_JLSANCHEZ
#define CPU_STEVECHECKOWAY
//#define CPU_ANDREWEISSFLOG

#define SDCARD
#define BEEPER

#define RESOLUTION VGA_640x480_60Hz
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 60

#define DEBUG_BAND_COLORS 0x2A10

#endif
