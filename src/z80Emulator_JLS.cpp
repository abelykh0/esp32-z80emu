// Z80 Emulator by Lin Ke-Fong

#include "z80Emulator.h"

#ifdef CPU_JLSANCHEZ2

#include "z80.h"
#include "z80operations.h"

static Operations z80Operations;
static Z80 z80(&z80Operations);
static Z80Environment* env;

static bool interruptPending = false;

// CPU Tstates elapsed in current frame
static uint32_t tstates;

void z80Emulator::setup(Z80Environment* environment)
{
    env = environment;
}

void z80Emulator::reset()
{
    z80.reset();
}

int z80Emulator::emulate(int number_cycles)
{
    tstates = 0;
	while (tstates < number_cycles)
	{
		z80.execute();
	}  

    return number_cycles;
}

void z80Emulator::interrupt()
{
    interruptPending = true;
}

uint8_t z80Emulator::get_A() { return z80.getRegA(); }
void z80Emulator::set_A(uint8_t value) { z80.setRegA(value); }

uint8_t z80Emulator::get_F() { return z80.getFlags(); }
void z80Emulator::set_F(uint8_t value) { z80.setFlags(value); }

uint8_t z80Emulator::get_B() { return z80.getRegB(); }
void z80Emulator::set_B(uint8_t value) { z80.setRegB(value); }

uint8_t z80Emulator::get_C() { return z80.getRegC(); }
void z80Emulator::set_C(uint8_t value) { z80.setRegC(value); }

uint8_t z80Emulator::get_D() { return z80.getRegD(); }
void z80Emulator::set_D(uint8_t value) { z80.setRegD(value); }

uint8_t z80Emulator::get_H() { return z80.getRegH(); }
void z80Emulator::set_H(uint8_t value) { z80.setRegH(value); }

uint8_t z80Emulator::get_L() { return z80.getRegL(); }
void z80Emulator::set_L(uint8_t value) { z80.setRegL(value); }

uint8_t z80Emulator::get_I() { return z80.getRegI(); }
void z80Emulator::set_I(uint8_t value) { z80.setRegI(value); }

uint8_t z80Emulator::get_R() { return z80.getRegR(); }
void z80Emulator::set_R(uint8_t value) { z80.setRegR(value); }

uint16_t z80Emulator::get_AF() { return z80.getRegAF(); }
void z80Emulator::set_AF(uint16_t value) { z80.setRegAF(value); }

uint16_t z80Emulator::get_BC() { return z80.getRegBC(); }
void z80Emulator::set_BC(uint16_t value) { z80.setRegBC(value); }

uint16_t z80Emulator::get_DE() { return z80.getRegDE(); }
void z80Emulator::set_DE(uint16_t value) { z80.setRegDE(value); }

uint16_t z80Emulator::get_HL() { return z80.getRegHL(); }
void z80Emulator::set_HL(uint16_t value) { z80.setRegHL(value); }

uint16_t z80Emulator::get_AFx() { return z80.getRegAFx(); }
void z80Emulator::set_AFx(uint16_t value) { z80.setRegAFx(value); }

uint16_t z80Emulator::get_BCx() { return z80.getRegBCx(); }
void z80Emulator::set_BCx(uint16_t value) { z80.setRegBCx(value); }

uint16_t z80Emulator::get_DEx() { return z80.getRegDEx(); }
void z80Emulator::set_DEx(uint16_t value) { z80.setRegDEx(value); }

uint16_t z80Emulator::get_HLx() { return z80.getRegHLx(); }
void z80Emulator::set_HLx(uint16_t value) { z80.setRegHLx(value); }

uint16_t z80Emulator::get_IX() { return z80.getRegIX(); }
void z80Emulator::set_IX(uint16_t value) { z80.setRegIX(value); }

uint16_t z80Emulator::get_IY() { return z80.getRegIY(); }
void z80Emulator::set_IY(uint16_t value) { z80.setRegIY(value); }

uint16_t z80Emulator::get_SP() { return z80.getRegSP(); }
void z80Emulator::set_SP(uint16_t value) { z80.setRegSP(value); }

uint16_t z80Emulator::get_PC() { return z80.getRegPC(); }
void z80Emulator::set_PC(uint16_t value) { z80.setRegPC(value); }

uint8_t z80Emulator::get_IFF1() { return z80.isIFF1() ? 1 : 0; }
void z80Emulator::set_IFF1(uint8_t value) { z80.setIFF1(value == 1);  }

uint8_t z80Emulator::get_IFF2() { return z80.isIFF2() ? 1 : 0; }
void z80Emulator::set_IFF2(uint8_t value) { z80.setIFF2(value == 1); }

uint8_t z80Emulator::get_IM() { return (uint8_t)z80.getIM(); }
void z80Emulator::set_IM(uint8_t value) { z80.setIM((Z80::IntMode)value); }

#endif