// Z80 Emulator by Lin Ke-Fong

#include "z80Emulator.h"

#ifdef CPU_LINKEFONG

#include "z80_LKF/z80emu.h"
#include "z80_LKF/z80user.h"

static Z80_STATE z80_state;
static Z80_STATE* state = &z80_state;
static CONTEXT context;
static Z80Environment* env;

extern "C"
{
    uint8_t readbyte(uint16_t addr);
    uint16_t readword(uint16_t addr);
    void writebyte(uint16_t addr, uint8_t data);
    void writeword(uint16_t addr, uint16_t data);
    uint8_t input(uint8_t portLow, uint8_t portHigh);
    void output(uint8_t portLow, uint8_t portHigh, uint8_t data);
}

void z80Emulator::setup(Z80Environment* environment)
{
    env = environment;
    context.readbyte = readbyte;
    context.readword = readword;
    context.writeword = writeword;
    context.writebyte = writebyte;
    context.input = input;
    context.output = output;
}

void z80Emulator::reset()
{
    Z80Reset(state);
}

int z80Emulator::emulate(int number_cycles)
{
    return Z80Emulate(state, number_cycles, &context);
}

void z80Emulator::interrupt()
{
    Z80Interrupt(state, 0xff, &context);
}

uint8_t z80Emulator::get_A() { return state->registers.byte[Z80_A]; }
void z80Emulator::set_A(uint8_t value) { state->registers.byte[Z80_A] = value; }

uint8_t z80Emulator::get_F() { return state->registers.byte[Z80_F]; }
void z80Emulator::set_F(uint8_t value) { state->registers.byte[Z80_F] = value; }

uint8_t z80Emulator::get_B() { return state->registers.byte[Z80_B]; }
void z80Emulator::set_B(uint8_t value) { state->registers.byte[Z80_B] = value; }

uint8_t z80Emulator::get_C() { return state->registers.byte[Z80_C]; }
void z80Emulator::set_C(uint8_t value) { state->registers.byte[Z80_C] = value; }

uint8_t z80Emulator::get_D() { return state->registers.byte[Z80_D]; }
void z80Emulator::set_D(uint8_t value) { state->registers.byte[Z80_D] = value; }

uint8_t z80Emulator::get_H() { return state->registers.byte[Z80_H]; }
void z80Emulator::set_H(uint8_t value) { state->registers.byte[Z80_H] = value; }

uint8_t z80Emulator::get_L() { return state->registers.byte[Z80_L]; }
void z80Emulator::set_L(uint8_t value) { state->registers.byte[Z80_L] = value; }

uint8_t z80Emulator::get_I() { return (uint8_t)state->i; }
void z80Emulator::set_I(uint8_t value) { state->i = value; }

uint8_t z80Emulator::get_R() { return (uint8_t)state->r; }
void z80Emulator::set_R(uint8_t value) { state->r = value; }

uint16_t z80Emulator::get_AF() { return state->registers.word[Z80_AF]; }
void z80Emulator::set_AF(uint16_t value) { state->registers.word[Z80_AF] = value; }

uint16_t z80Emulator::get_BC() { return state->registers.word[Z80_BC]; }
void z80Emulator::set_BC(uint16_t value) { state->registers.word[Z80_BC] = value; }

uint16_t z80Emulator::get_DE() { return state->registers.word[Z80_DE]; }
void z80Emulator::set_DE(uint16_t value) { state->registers.word[Z80_DE] = value; }

uint16_t z80Emulator::get_HL() { return state->registers.word[Z80_HL]; }
void z80Emulator::set_HL(uint16_t value) { state->registers.word[Z80_HL] = value; }

uint16_t z80Emulator::get_AFx() { return state->alternates[Z80_AF]; }
void z80Emulator::set_AFx(uint16_t value) { state->alternates[Z80_AF] = value; }

uint16_t z80Emulator::get_BCx() { return state->alternates[Z80_BC]; }
void z80Emulator::set_BCx(uint16_t value) { state->alternates[Z80_BC] = value; }

uint16_t z80Emulator::get_DEx() { return state->alternates[Z80_DE]; }
void z80Emulator::set_DEx(uint16_t value) { state->alternates[Z80_DE] = value; }

uint16_t z80Emulator::get_HLx() { return state->alternates[Z80_HL]; }
void z80Emulator::set_HLx(uint16_t value) { state->alternates[Z80_HL] = value; }

uint16_t z80Emulator::get_IX() { return state->registers.word[Z80_IX]; }
void z80Emulator::set_IX(uint16_t value) { state->registers.word[Z80_IX] = value; }

uint16_t z80Emulator::get_IY() { return state->registers.word[Z80_IY]; }
void z80Emulator::set_IY(uint16_t value) { state->registers.word[Z80_IY] = value; }

uint16_t z80Emulator::get_SP() { return state->registers.word[Z80_SP]; }
void z80Emulator::set_SP(uint16_t value) { state->registers.word[Z80_SP] = value; }

uint16_t z80Emulator::get_PC() { return (uint16_t)state->pc; }
void z80Emulator::set_PC(uint16_t value) { state->pc = value; }

uint8_t z80Emulator::get_IFF1() { return (uint8_t)state->iff1; }
void z80Emulator::set_IFF1(uint8_t value) { state->iff1 = value; }

uint8_t z80Emulator::get_IFF2() { return (uint8_t)state->iff2; }
void z80Emulator::set_IFF2(uint8_t value) { state->iff2 = value; }

uint8_t z80Emulator::get_IM() { return (uint8_t)state->im; }
void z80Emulator::set_IM(uint8_t value) { state->im = value; }

extern "C" uint8_t readbyte(uint16_t addr)
{
    return env->ReadByte(addr);
}

extern "C" uint16_t readword(uint16_t addr)
{
    return env->ReadWord(addr);
}

extern "C" void writebyte(uint16_t addr, uint8_t data)
{
    env->WriteByte(addr, data);
}

extern "C" void writeword(uint16_t addr, uint16_t data)
{
    env->WriteWord(addr, data);
}

extern "C" uint8_t input(uint8_t portLow, uint8_t portHigh)
{
    return env->Input(portLow, portHigh);
}

extern "C" void output(uint8_t portLow, uint8_t portHigh, uint8_t data)
{
    env->Output(portLow, portHigh, data);
}

#endif