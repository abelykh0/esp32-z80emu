// Z80 Emulator by Lin Ke-Fong

#include "z80Emulator.h"

#ifdef CPU_JLSANCHEZ

#include "z80.h"
#include "z80operations.h"

class Operations : public Z80operations
{
private:
    // Delay Contention: for emulating CPU slowing due to sharing bus with ULA
    // NOTE: Only 48K spectrum contention implemented. This function must be called
    // only when dealing with affected memory (use ADDRESS_IN_LOW_RAM macro)
    static uint8_t delayContention(uint32_t currentTstates);    

public:
    Z80Environment* _environment;

    uint8_t fetchOpcode(uint16_t address) override;
    uint8_t peek8(uint16_t address) override;
    void poke8(uint16_t address, uint8_t value) override;
    uint16_t peek16(uint16_t adddress) override;
    void poke16(uint16_t address, RegisterPair word) override;
    uint8_t inPort(uint16_t port) override;
    void outPort(uint16_t port, uint8_t value) override;
    void addressOnBus(uint16_t address, int32_t wstates) override;
    void interruptHandlingTime(int32_t wstates) override;
    bool isActiveINT(void) override;    
};

static Operations z80Operations;
static Z80 z80(&z80Operations);

static bool interruptPending = false;

void z80Emulator::setup(Z80Environment* environment)
{
    z80Operations._environment = environment;
}

void z80Emulator::reset()
{
    z80.reset();
}

int z80Emulator::emulate(int number_cycles)
{
    z80Operations._environment->TStates = 0;
	while (z80Operations._environment->TStates < number_cycles)
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


#define ADDRESS_IN_LOW_RAM(addr) (1 == (addr >> 14))

///////////////////////////////////////////////////////////////////////////////
//
// delay contention: emulates wait states introduced by the ULA (graphic chip)
// whenever there is a memory access to contended memory (shared between ULA and CPU).
// detailed info: https://worldofspectrum.org/faq/reference/48kreference.htm#ZXSpectrum
// from paragraph which starts with "The 50 Hz interrupt is synchronized with..."
// if you only read from https://worldofspectrum.org/faq/reference/48kreference.htm#Contention
// without reading the previous paragraphs about line timings, it may be confusing.
//
inline uint8_t Operations::delayContention(uint32_t currentTstates)
{
    // sequence of wait states
    static uint8_t wait_states[8] = { 6, 5, 4, 3, 2, 1, 0, 0 };

	// delay states one t-state BEFORE the first pixel to be drawn
	currentTstates += 1;

	// each line spans 224 t-states
	int line = currentTstates / 224;

	// only the 192 lines between 64 and 255 have graphic data, the rest is border
	if (line < 64 || line >= 256) return 0;

	// only the first 128 t-states of each line correspond to a graphic data transfer
	// the remaining 96 t-states correspond to border
	int halfpix = currentTstates % 224;
	if (halfpix >= 128) return 0;

	int modulo = halfpix % 8;
	return wait_states[modulo];
}

/* Read opcode from RAM */
uint8_t Operations::fetchOpcode(uint16_t address) {
    // 3 clocks to fetch opcode from RAM and 1 execution clock
    if (ADDRESS_IN_LOW_RAM(address))
    {
        this->_environment->TStates += Operations::delayContention(this->_environment->TStates);
    }

    this->_environment->TStates += 4;
    return this->_environment->ReadByte(address);
}

/* Read/Write byte from/to RAM */
uint8_t Operations::peek8(uint16_t address) {
    // 3 clocks for read byte from RAM
    if (ADDRESS_IN_LOW_RAM(address))
    {
        this->_environment->TStates += Operations::delayContention(this->_environment->TStates);
    }

    this->_environment->TStates += 3;
    return this->_environment->ReadByte(address);
}
void Operations::poke8(uint16_t address, uint8_t value) {
    // 3 clocks for write byte to RAM
    if (ADDRESS_IN_LOW_RAM(address))
    {
        this->_environment->TStates += Operations::delayContention(this->_environment->TStates);
    }

    this->_environment->TStates += 3;
    this->_environment->WriteByte(address, value);
}

/* Read/Write word from/to RAM */
uint16_t Operations::peek16(uint16_t address) {
    // Order matters, first read lsb, then read msb, don't "optimize"
    uint8_t lsb = this->peek8(address);
    uint8_t msb = this->peek8(address + 1);
    return (msb << 8) | lsb;
}
void Operations::poke16(uint16_t address, RegisterPair word) {
    // Order matters, first write lsb, then write msb, don't "optimize"
    this->poke8(address, word.byte8.lo);
    this->poke8(address + 1, word.byte8.hi);
}

/* In/Out byte from/to IO Bus */
uint8_t Operations::inPort(uint16_t port) {
    // 3 clocks for read byte from bus
    this->_environment->TStates += 3;
    uint8_t hiport = port >> 8;
    uint8_t loport = port & 0xFF;
    return this->_environment->Input(loport, hiport);
}
void Operations::outPort(uint16_t port, uint8_t value) {
    // 4 clocks for write byte to bus
    this->_environment->TStates += 4;
    uint8_t hiport = port >> 8;
    uint8_t loport = port & 0xFF;
    this->_environment->Output(loport, hiport, value);
}

/* Put an address on bus lasting 'tstates' cycles */
void Operations::addressOnBus(uint16_t address, int32_t wstates){
    // Additional clocks to be added on some instructions
    if (ADDRESS_IN_LOW_RAM(address)) 
    {
        for (int idx = 0; idx < wstates; idx++) 
        {
            this->_environment->TStates += Operations::delayContention(this->_environment->TStates) + 1;
        }
    }
    else
    {
        this->_environment->TStates += wstates;
    }
}

/* Clocks needed for processing INT and NMI */
void Operations::interruptHandlingTime(int32_t wstates) 
{
    this->_environment->TStates += wstates;
}

/* Callback to know when the INT signal is active */
bool Operations::isActiveINT(void) 
{
    if (!interruptPending) return false;
    interruptPending = false;
    return true;
}

#endif