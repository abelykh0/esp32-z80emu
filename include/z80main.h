#ifndef __ZXMAIN_INCLUDED__
#define __ZXMAIN_INCLUDED__

#include "z80Emulator.h"
#include "z80Environment.h"
#include "ay3-8912-state.h"

extern Sound::Ay3_8912_state _ay3_8912;
extern z80Emulator Z80cpu;

void zx_setup(Z80Environment* spectrumScreen);
int32_t zx_loop();
void zx_reset();

#endif
