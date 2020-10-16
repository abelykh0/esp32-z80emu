#ifndef __ZXMAIN_INCLUDED__
#define __ZXMAIN_INCLUDED__

#include "z80user.h"
#include "main_ROM.h"
#include "SpectrumScreen.h"
//#include "ay3-8912-state.h"

using namespace Display;

extern SpectrumScreen* _spectrumScreen;
//extern Sound::Ay3_8912_state _ay3_8912;
extern Z80_STATE _zxCpu;
extern uint8_t RamBuffer[];

void zx_setup(SpectrumScreen* spectrumScreen);
int32_t zx_loop();
void zx_reset();

#endif
