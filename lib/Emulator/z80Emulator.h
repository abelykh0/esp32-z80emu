#ifndef __Z80EMULATOR_INCLUDED__
#define __Z80EMULATOR_INCLUDED__

// Abstraction for z80 Emulator

#include <stdint.h>
#include "../../include/emulator.h"
#include "z80Environment.h"
#undef F

// Magic from https://www.codeproject.com/Articles/12358/C-object-properties-with-no-run-time-or-memory-ove.
template<class T, class V, V (T::*_get)(), void (T::*_set)(V)>
struct property
{
private:
	T* _this;
	V _value;
public:
	property(T* this_):_this(this_) { }
	operator V() { return (_this->*_get)(); }
	void operator=(V i) { (_this->*_set)(i);}
};
#define CLASS(NAME) typedef NAME ClassType
#define PROPERTY(TYPE, NAME) \
    property<ClassType, TYPE, &ClassType::get_##NAME, &ClassType::set_##NAME> NAME

struct z80Emulator
{
private:
    uint8_t get_A(); void set_A(uint8_t);
    uint8_t get_F(); void set_F(uint8_t);
    uint8_t get_B(); void set_B(uint8_t);
    uint8_t get_C(); void set_C(uint8_t);
    uint8_t get_D(); void set_D(uint8_t);
    uint8_t get_E(); void set_E(uint8_t);
    uint8_t get_H(); void set_H(uint8_t);
    uint8_t get_L(); void set_L(uint8_t);

    uint8_t get_I(); void set_I(uint8_t);
    uint8_t get_R(); void set_R(uint8_t);

    uint16_t get_AF(); void set_AF(uint16_t);
    uint16_t get_BC(); void set_BC(uint16_t);
    uint16_t get_DE(); void set_DE(uint16_t);
    uint16_t get_HL(); void set_HL(uint16_t);

    uint16_t get_AFx(); void set_AFx(uint16_t);
    uint16_t get_BCx(); void set_BCx(uint16_t);
    uint16_t get_DEx(); void set_DEx(uint16_t);
    uint16_t get_HLx(); void set_HLx(uint16_t);

    uint16_t get_IX(); void set_IX(uint16_t);
    uint16_t get_IY(); void set_IY(uint16_t);
    uint16_t get_SP(); void set_SP(uint16_t);

    uint16_t get_PC(); void set_PC(uint16_t);

    uint8_t get_IFF1(); void set_IFF1(uint8_t);
    uint8_t get_IFF2(); void set_IFF2(uint8_t);
    uint8_t get_IM(); void set_IM(uint8_t);

public:
    void setup(Z80Environment* environment);
    void reset();
    int emulate(int number_cycles);
    void interrupt();

    CLASS(z80Emulator);

    // 8 bit registers
    PROPERTY(uint8_t, A); // Accumulator 
    PROPERTY(uint8_t, F); // Flags
    PROPERTY(uint8_t, B);
    PROPERTY(uint8_t, C);
    PROPERTY(uint8_t, D);
    PROPERTY(uint8_t, E);
    PROPERTY(uint8_t, H);
    PROPERTY(uint8_t, L);

    // Other registers
    PROPERTY(uint8_t, I); // Interrupt vector
    PROPERTY(uint8_t, R); // Refresh counter

    // 16 bit registers
    PROPERTY(uint16_t, AF);
    PROPERTY(uint16_t, BC);
    PROPERTY(uint16_t, DE);
    PROPERTY(uint16_t, HL);

    // Alternate (shadow) registers
    PROPERTY(uint16_t, AFx);
    PROPERTY(uint16_t, BCx);
    PROPERTY(uint16_t, DEx);
    PROPERTY(uint16_t, HLx);

    // Index registers
    PROPERTY(uint16_t, IX);
    PROPERTY(uint16_t, IY);
    PROPERTY(uint16_t, SP);

    // Program counter
    PROPERTY(uint16_t, PC);

    // Mode
    PROPERTY(uint8_t, IFF1); // Interrupt flip-flop 1 (its value cannot be read)
    PROPERTY(uint8_t, IFF2); // Interrupt flip-flop 2
    PROPERTY(uint8_t, IM);   // Interrupt mode

    z80Emulator() :
        A(this), F(this), B(this), C(this), D(this), E(this), H(this), L(this),
        I(this), R(this), AF(this), BC(this), DE(this), HL(this),
        AFx(this), BCx(this), DEx(this), HLx(this),
        IX(this), IY(this), SP(this), PC(this), 
        IFF1(this), IFF2(this), IM(this)
    {
    }
};

#endif

