// Z80 Emulator by Steve Checkoway

#include "z80Emulator.h"

#ifdef CPU_STEVECHECKOWAY

#include "zel/z80_instructions.h"
#include "zel/z80.h"
#include "z80_types.h"

static Z80 _zxCpu;
static Z80Environment* env;

extern "C" uint8_t ReadMem(uint16_t addr, bool inst, Z80 cpu);
extern "C" void WriteMem(uint16_t addr, uint8_t val, Z80 cpu);
extern "C" uint8_t ReadInterruptData(uint16_t n, Z80 cpu);
extern "C" uint8_t ReadIO(uint16_t addr, Z80 cpu);
extern "C" void WriteIO(uint16_t addr, uint8_t val, Z80 cpu);
extern "C" void InterruptComplete(Z80 cpu);

void z80Emulator::setup(Z80Environment* environment)
{
    env = environment;

    Z80FunctionBlock functionBlock;
    functionBlock.ReadMem = ReadMem;
    functionBlock.WriteMem = WriteMem;
    functionBlock.ReadInterruptData = ReadInterruptData;
    functionBlock.ReadIO = ReadIO;
    functionBlock.WriteIO = WriteIO;
    functionBlock.InterruptComplete = InterruptComplete;
    functionBlock.ControlFlow = nullptr;

    _zxCpu = Z80_New(&functionBlock);
}

void z80Emulator::reset()
{
    // set AF to 0xFFFF, all other regs are undefined
    Z80_SetReg(REG_AF, 0xFFFF, _zxCpu);

    // set SP to 0xFFFF, PC to 0x0000
    Z80_SetReg(REG_SP, 0xFFFF, _zxCpu);
    Z80_SetReg(REG_PC, 0x0000, _zxCpu);

    // IFF1 and IFF2 are off
    _zxCpu->iff1 = false;
    _zxCpu->iff2 = false;

    // IM is set to 0
    _zxCpu->interrupt_mode = 0;

    // after power-on or reset, R is set to 0
    this->set_R(0);

    Z80_ClearHalt(_zxCpu);
}

int z80Emulator::emulate(int number_cycles)
{
    int cycles = 0;
    while (cycles < number_cycles)
    {
        cycles += Z80_Step(nullptr, _zxCpu);
    }

    return cycles;
}

void z80Emulator::interrupt()
{
    Z80_RaiseInterrupt(_zxCpu);
}

uint8_t z80Emulator::get_A() { return _zxCpu->byte_reg[REG_A]; }
void z80Emulator::set_A(uint8_t value) { _zxCpu->byte_reg[REG_A] = value; }

uint8_t z80Emulator::get_F() { return _zxCpu->byte_reg[REG_F]; }
void z80Emulator::set_F(uint8_t value) { _zxCpu->byte_reg[REG_F] = value; }

uint8_t z80Emulator::get_B() { return _zxCpu->byte_reg[REG_B]; }
void z80Emulator::set_B(uint8_t value) { _zxCpu->byte_reg[REG_B] = value; }

uint8_t z80Emulator::get_C() { return _zxCpu->byte_reg[REG_C]; }
void z80Emulator::set_C(uint8_t value) { _zxCpu->byte_reg[REG_C] = value; }

uint8_t z80Emulator::get_D() { return _zxCpu->byte_reg[REG_D]; }
void z80Emulator::set_D(uint8_t value) { _zxCpu->byte_reg[REG_D] = value; }

uint8_t z80Emulator::get_H() { return _zxCpu->byte_reg[REG_H]; }
void z80Emulator::set_H(uint8_t value) { _zxCpu->byte_reg[REG_H] = value; }

uint8_t z80Emulator::get_L() { return _zxCpu->byte_reg[REG_L]; }
void z80Emulator::set_L(uint8_t value) { _zxCpu->byte_reg[REG_L] = value; }

uint8_t z80Emulator::get_I() { return _zxCpu->byte_reg[REG_I]; }
void z80Emulator::set_I(uint8_t value) { _zxCpu->byte_reg[REG_I] = value; }

uint8_t z80Emulator::get_R() { return _zxCpu->byte_reg[REG_R]; }
void z80Emulator::set_R(uint8_t value) { _zxCpu->byte_reg[REG_R] = value; }

uint16_t z80Emulator::get_AF() { return Z80_GetReg(REG_AF, _zxCpu); }
void z80Emulator::set_AF(uint16_t value) { Z80_SetReg(REG_AF, value, _zxCpu); }

uint16_t z80Emulator::get_BC() { return Z80_GetReg(REG_BC, _zxCpu); }
void z80Emulator::set_BC(uint16_t value) { Z80_SetReg(REG_BC, value, _zxCpu); }

uint16_t z80Emulator::get_DE() { return Z80_GetReg(REG_DE, _zxCpu); }
void z80Emulator::set_DE(uint16_t value) { Z80_SetReg(REG_DE, value, _zxCpu); }

uint16_t z80Emulator::get_HL() { return Z80_GetReg(REG_HL, _zxCpu);; }
void z80Emulator::set_HL(uint16_t value) { Z80_SetReg(REG_HL, value, _zxCpu); }

uint16_t z80Emulator::get_AFx() { return Z80_GetReg(REG_AFP, _zxCpu); }
void z80Emulator::set_AFx(uint16_t value) { Z80_SetReg(REG_AFP, value, _zxCpu); }

uint16_t z80Emulator::get_BCx() { return Z80_GetReg(REG_BCP, _zxCpu); }
void z80Emulator::set_BCx(uint16_t value) { Z80_SetReg(REG_BCP, value, _zxCpu); }

uint16_t z80Emulator::get_DEx() { return Z80_GetReg(REG_DEP, _zxCpu);; }
void z80Emulator::set_DEx(uint16_t value) { Z80_SetReg(REG_DEP, value, _zxCpu); }

uint16_t z80Emulator::get_HLx() { return Z80_GetReg(REG_HLP, _zxCpu); }
void z80Emulator::set_HLx(uint16_t value) { Z80_SetReg(REG_HLP, value, _zxCpu); }

uint16_t z80Emulator::get_IX() { return Z80_GetReg(REG_IX, _zxCpu);; }
void z80Emulator::set_IX(uint16_t value) { Z80_SetReg(REG_IX, value, _zxCpu); }

uint16_t z80Emulator::get_IY() { return Z80_GetReg(REG_IY, _zxCpu);; }
void z80Emulator::set_IY(uint16_t value) { Z80_SetReg(REG_IY, value, _zxCpu); }

uint16_t z80Emulator::get_SP() { return Z80_GetReg(REG_SP, _zxCpu); }
void z80Emulator::set_SP(uint16_t value) { Z80_SetReg(REG_SP, value, _zxCpu); }

uint16_t z80Emulator::get_PC() { return Z80_GetReg(REG_PC, _zxCpu); }
void z80Emulator::set_PC(uint16_t value) { Z80_SetReg(REG_PC, value, _zxCpu); }

uint8_t z80Emulator::get_IFF1() { return _zxCpu->iff1 ? 1 : 0; }
void z80Emulator::set_IFF1(uint8_t value) { _zxCpu->iff1 = (value == 1); }

uint8_t z80Emulator::get_IFF2() { return _zxCpu->iff2 ? 1 : 0; }
void z80Emulator::set_IFF2(uint8_t value) { _zxCpu->iff2 = (value == 1); }

uint8_t z80Emulator::get_IM() { return (uint8_t)_zxCpu->interrupt_mode; }
void z80Emulator::set_IM(uint8_t value) { _zxCpu->interrupt_mode = value; }


/*! Read a byte of memory.
* \param addr The address to read.
* \param inst True if the z80 is reading instructions.
* \param cpu The \c Z80 instance making the read call.
* \return The byte from memory.
*/
extern "C" uint8_t ReadMem(uint16_t addr, bool inst, Z80 cpu)
{
    return env->ReadByte(addr);
}

/*! Write a byte of memory.
* \param addr The address to write.
* \param val The byte to write.
* \param cpu The \c Z80 instance making the write call.
*/
extern "C" void WriteMem(uint16_t addr, uint8_t val, Z80 cpu)
{
    env->WriteByte(addr, val);
}

/*! Read the interrupt data.
* \param n Read the \a n th byte of data.
* \param cpu The \c Z80 instance making the read call.
*/
extern "C" uint8_t ReadInterruptData(uint16_t n, Z80 cpu)
{
    return 0xFF;
}

/*! Read a byte from an I/O port.
* \param addr The contents of the address bus during the
* request. The low 8 bits specify the port.
* \param cpu The \c Z80 instance making the read call.
* \return The byte from the I/O port.
*/
extern "C" uint8_t ReadIO(uint16_t addr, Z80 cpu)
{
    return env->Input(addr &0xFF, addr >> 8);
}

/*! Write a byte from an I/O port.
* \param addr The contents of the address bus during the
* request. The low 8 bits specify the port.
* \param val The byte to write.
* \param cpu The \c Z80 instance making the read call.
*/
extern "C" void WriteIO(uint16_t addr, uint8_t val, Z80 cpu)
{
    env->Output(addr &0xFF, addr >> 8, val);
}

/*! Notify the peripherials that a return from interrupt
* instruction has occured.
* \param cpu The \c Z80 instance performing the notification.
*/
extern "C" void InterruptComplete(Z80 cpu)
{

}

#endif