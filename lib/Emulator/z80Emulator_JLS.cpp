// Z80 Emulator by Lin Ke-Fong

#include "z80Emulator.h"

#ifdef CPU_JLSANCHEZ

#include "z80_JLS/z80.h"
#include "z80_JLS/z80operations.h"

static bool interruptPending = false;
static Z80operations z80Operations;
static Z80 z80(&z80Operations)
static Z80Environment* env;

void z80Emulator::setup(Z80Environment* environment)
{
    Z80::create();
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

uint8_t z80Emulator::get_A() { return regAF.byte8.hi; }
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

/* Read opcode from RAM */
uint8_t Z80operations::fetchOpcode(uint16_t address) {
    // 3 clocks to fetch opcode from RAM and 1 execution clock
    if (ADDRESS_IN_LOW_RAM(address))
        CPU::tstates += CPU::delayContention(CPU::tstates);

    CPU::tstates += 4;
    return Mem::readbyte(address);
}

/* Read/Write byte from/to RAM */
uint8_t Z80operations::peek8(uint16_t address) {
    // 3 clocks for read byte from RAM
    if (ADDRESS_IN_LOW_RAM(address))
        CPU::tstates += CPU::delayContention(CPU::tstates);

    CPU::tstates += 3;
    return Mem::readbyte(address);
}
void Z80operations::poke8(uint16_t address, uint8_t value) {
    // 3 clocks for write byte to RAM
    if (ADDRESS_IN_LOW_RAM(address))
        CPU::tstates += CPU::delayContention(CPU::tstates);

    CPU::tstates += 3;
    Mem::writebyte(address, value);
}

/* Read/Write word from/to RAM */
uint16_t Z80operations::peek16(uint16_t address) {
    // Order matters, first read lsb, then read msb, don't "optimize"
    uint8_t lsb = this->peek8(address);
    uint8_t msb = this->peek8(address + 1);
    return (msb << 8) | lsb;
}
void Z80operations::poke16(uint16_t address, RegisterPair word) {
    // Order matters, first write lsb, then write msb, don't "optimize"
    this->poke8(address, word.byte8.lo);
    this->poke8(address + 1, word.byte8.hi);
}

/* In/Out byte from/to IO Bus */
uint8_t Z80operations::inPort(uint16_t port) {
    // 3 clocks for read byte from bus
    CPU::tstates += 3;
    uint8_t hiport = port >> 8;
    uint8_t loport = port & 0xFF;
    return Ports::input(loport, hiport);
}
void Z80operations::outPort(uint16_t port, uint8_t value) {
    // 4 clocks for write byte to bus
    CPU::tstates += 4;
    uint8_t hiport = port >> 8;
    uint8_t loport = port & 0xFF;
    Ports::output(loport, hiport, value);
}

/* Put an address on bus lasting 'tstates' cycles */
void Z80operations::addressOnBus(uint16_t address, int32_t wstates){
    // Additional clocks to be added on some instructions
    if (ADDRESS_IN_LOW_RAM(address)) {
        for (int idx = 0; idx < wstates; idx++) {
            CPU::tstates += CPU::delayContention(CPU::tstates) + 1;
        }
    }
    else
        CPU::tstates += wstates;
}

/* Clocks needed for processing INT and NMI */
void Z80operations::interruptHandlingTime(int32_t wstates) {
    CPU::tstates += wstates;
}

/* Callback to know when the INT signal is active */
bool Z80operations::isActiveINT(void) {
    if (!interruptPending) return false;
    interruptPending = false;
    return true;
}

#endif