// Z80 Emulator by Lin Ke-Fong

#include "z80Emulator.h"

#ifdef CPU_ANDREWEISSFLOG

#include "z80_AW.h"

static z80_t _zxCpu;
static Z80Environment* env;

extern "C" uint64_t cpu_tick(int num, uint64_t pins, void* user_data);

void z80Emulator::setup(Z80Environment* environment)
{
    env = environment;
    z80_desc_t init = { .tick_cb = cpu_tick, .user_data = NULL };
	z80_init(&_zxCpu, &init);    
}

void z80Emulator::reset()
{
    z80_reset(&_zxCpu);
}

int z80Emulator::emulate(int number_cycles)
{
    return z80_exec(&_zxCpu, number_cycles);
}

void z80Emulator::interrupt()
{
    _zxCpu.pins |= Z80_INT;
}

uint8_t z80Emulator::get_A() { return z80_a(&_zxCpu); }
void z80Emulator::set_A(uint8_t value) { z80_set_a(&_zxCpu, value); }

uint8_t z80Emulator::get_F() { return z80_f(&_zxCpu); }
void z80Emulator::set_F(uint8_t value) { z80_set_f(&_zxCpu, value); }

uint8_t z80Emulator::get_B() { return z80_b(&_zxCpu); }
void z80Emulator::set_B(uint8_t value) { z80_set_b(&_zxCpu, value); }

uint8_t z80Emulator::get_C() { return z80_c(&_zxCpu); }
void z80Emulator::set_C(uint8_t value) { z80_set_c(&_zxCpu, value); }

uint8_t z80Emulator::get_D() { return z80_d(&_zxCpu); }
void z80Emulator::set_D(uint8_t value) { z80_set_d(&_zxCpu, value); }

uint8_t z80Emulator::get_H() { return z80_h(&_zxCpu); }
void z80Emulator::set_H(uint8_t value) { z80_set_h(&_zxCpu, value); }

uint8_t z80Emulator::get_L() { return z80_l(&_zxCpu); }
void z80Emulator::set_L(uint8_t value) { z80_set_l(&_zxCpu, value); }

uint8_t z80Emulator::get_I() { return z80_i(&_zxCpu); }
void z80Emulator::set_I(uint8_t value) { z80_set_i(&_zxCpu, value); }

uint8_t z80Emulator::get_R() { return z80_r(&_zxCpu); }
void z80Emulator::set_R(uint8_t value) { z80_set_r(&_zxCpu, value); }

uint16_t z80Emulator::get_AF() { return z80_af(&_zxCpu); }
void z80Emulator::set_AF(uint16_t value) { z80_set_af(&_zxCpu, value); }

uint16_t z80Emulator::get_BC() { return z80_bc(&_zxCpu); }
void z80Emulator::set_BC(uint16_t value) { z80_set_bc(&_zxCpu, value); }

uint16_t z80Emulator::get_DE() { return z80_de(&_zxCpu); }
void z80Emulator::set_DE(uint16_t value) { z80_set_de(&_zxCpu, value); }

uint16_t z80Emulator::get_HL() { return z80_hl(&_zxCpu); }
void z80Emulator::set_HL(uint16_t value) { z80_set_hl(&_zxCpu, value); }

uint16_t z80Emulator::get_AFx() { return z80_af_(&_zxCpu); }
void z80Emulator::set_AFx(uint16_t value) { z80_set_af_(&_zxCpu, value); }

uint16_t z80Emulator::get_BCx() { return z80_bc_(&_zxCpu); }
void z80Emulator::set_BCx(uint16_t value) { z80_set_bc_(&_zxCpu, value); }

uint16_t z80Emulator::get_DEx() { return z80_de_(&_zxCpu); }
void z80Emulator::set_DEx(uint16_t value) { z80_set_de_(&_zxCpu, value); }

uint16_t z80Emulator::get_HLx() { return z80_hl_(&_zxCpu); }
void z80Emulator::set_HLx(uint16_t value) { z80_set_hl_(&_zxCpu, value); }

uint16_t z80Emulator::get_IX() { return z80_ix(&_zxCpu); }
void z80Emulator::set_IX(uint16_t value) { z80_set_ix(&_zxCpu, value); }

uint16_t z80Emulator::get_IY() { return z80_iy(&_zxCpu); }
void z80Emulator::set_IY(uint16_t value) { z80_set_iy(&_zxCpu, value); }

uint16_t z80Emulator::get_SP() { return z80_sp(&_zxCpu); }
void z80Emulator::set_SP(uint16_t value) {z80_set_sp(&_zxCpu, value); }

uint16_t z80Emulator::get_PC() { return z80_pc(&_zxCpu); }
void z80Emulator::set_PC(uint16_t value) { z80_set_pc(&_zxCpu, value); }

uint8_t z80Emulator::get_IFF1() { return z80_iff1(&_zxCpu); }
void z80Emulator::set_IFF1(uint8_t value) { z80_set_iff1(&_zxCpu, value); }

uint8_t z80Emulator::get_IFF2() { return z80_iff2(&_zxCpu); }
void z80Emulator::set_IFF2(uint8_t value) { z80_set_iff2(&_zxCpu, value); }

uint8_t z80Emulator::get_IM() { return z80_im(&_zxCpu); }
void z80Emulator::set_IM(uint8_t value) { z80_set_im(&_zxCpu, value); }

extern "C" uint64_t cpu_tick(int num, uint64_t pins, void* user_data)
{
    if (pins & Z80_MREQ)
    {
        if (pins & Z80_RD)
        {
            Z80_SET_DATA(pins, env->ReadByte(Z80_GET_ADDR(pins)));
        }
        else if (pins & Z80_WR)
        {
        	env->WriteByte(Z80_GET_ADDR(pins), Z80_GET_DATA(pins));
        }
    }
    else if (pins & Z80_IORQ)
    {
        if (pins & Z80_RD)
        {
            uint16_t port = Z80_GET_ADDR(pins);
            Z80_SET_DATA(pins, env->Input(port & 0xFF, port >> 8));
        }
        else if (pins & Z80_WR)
        {
        	uint16_t port = Z80_GET_ADDR(pins);
        	env->Output(port & 0xFF, port >> 8, Z80_GET_DATA(pins));
        }
    }
    return pins;
}

#endif