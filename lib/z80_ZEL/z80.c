/* Copyright (c) 2008, 2017 Stephen Checkoway
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <string.h>
#include <assert.h>
#include <zel/z80.h>
#include <zel/z80_instructions.h>

#include "z80_types.h"

/*! A 16 bit z80 word. */
typedef uint16_t word;

// short cuts
#define WORD_REG (cpu->word_reg)
#define BYTE_REG (cpu->byte_reg)
#define SP  (cpu->word_reg[REG_SP])
#define PC  (cpu->word_reg[REG_PC])
#define PCH (cpu->byte_reg[REG_PCH])
#define PCL (cpu->byte_reg[REG_PCL])
#define BC  (cpu->word_reg[REG_BC])
#define DE  (cpu->word_reg[REG_DE])
#define HL  (cpu->word_reg[REG_HL])
#define AF  (cpu->word_reg[REG_AF])
#define B   (cpu->byte_reg[REG_B])
#define C   (cpu->byte_reg[REG_C])
#define D   (cpu->byte_reg[REG_D])
#define E   (cpu->byte_reg[REG_E])
#define H   (cpu->byte_reg[REG_H])
#define L   (cpu->byte_reg[REG_L])
#define A   (cpu->byte_reg[REG_A])
#define FlagIsSet(f) (!FlagIsReset((f)))
#define FlagIsReset(f) (!(cpu->byte_reg[REG_F]&(1<<(f))))
#define SetFlag(f) (void)(cpu->byte_reg[REG_F]|=(1<<(f)))
#define ResetFlag(f) (void)(cpu->byte_reg[REG_F]&=~(1<<(f)))
#define SetFlagValue(f,v) \
	(void)(cpu->byte_reg[REG_F] = (cpu->byte_reg[REG_F] & ~(1<<(f))) | (!!(v)<<(f)))
#define CondIsMet(c) ( ((c)>=0 && FlagIsSet((c))) || ((c)<0 && FlagIsReset(-(c+1))) )


void IgnoreControlFlow( word pc, word target, ControlFlowType cf, Z80 cpu ) { }

Z80 Z80_New( const Z80FunctionBlock *blk )
{
	Z80 cpu = malloc( sizeof(struct Z80_t) );
	assert( cpu != NULL );
	memset( cpu, 0, sizeof(struct Z80_t) );
	cpu->byte_reg = (byte*)cpu->word_reg;
	SP = 0xffff;
	AF = 0xffff;
	cpu->can_handle_interrupt = true;
#define REQUIRE(x) assert(blk->x); cpu->x = blk->x
	REQUIRE(ReadMem);
	REQUIRE(WriteMem);
	REQUIRE(ReadInterruptData);
	REQUIRE(ReadIO);
	REQUIRE(WriteIO);
	REQUIRE(InterruptComplete);
#undef REQUIRE
	cpu->ControlFlow = blk->ControlFlow? blk->ControlFlow:IgnoreControlFlow;
	return cpu;
}

void Z80_Free( Z80 cpu )
{
	/* nothing special needed */
	free( cpu );
}

static byte ReadInstructionMemory( word address, void *data )
{
	Z80 cpu = data;
	return (cpu->ReadMem)( address, true, cpu );
}

static inline bool ParityIsEven( uint_fast8_t a )
{
	uint_fast8_t b = a & 0x55;
	uint_fast8_t c = (a>>1) & 0x55;
	a = b + c;
	b = a & 0x33;
	c = (a>>2) & 0x33;
	a = b + c;
	b = a & 0x0f;
	c = (a>>4) & 0x0f;
	return !((b + c)&1);
}

int Z80_Step( word *outPC, Z80 cpu )
{
	Instruction inst;
	uint_fast8_t r;
	const uint_fast16_t oldPC = PC;
	int ticks = 0;

	cpu->restart_io = false;
	if( cpu->nmi ||
	    (cpu->can_handle_interrupt && cpu->interrupt && cpu->iff1) )
	{
		cpu->halt = false;
		// Any interrupt increases R by one.
		r = BYTE_REG[REG_R];
		r = ((r + 1) & 0x7f) | (r & 0x80);
		BYTE_REG[REG_R] = r;
		if( cpu->nmi )
		{
			cpu->nmi = false;
			cpu->iff1 = false; /* Disable interrupts. */
			// 5 cycles fetching and ignoring the opcode
			// This can cause another nmi.
			(void)(cpu->ReadMem)( PC, true, cpu );
			// 6 cycles writing the PC
			(cpu->WriteMem)( --SP, PCH, cpu );
			(cpu->WriteMem)( --SP, PCL, cpu );
			PC = 0x0066; // Fixed location
			(cpu->ControlFlow)( oldPC, PC, CF_NMI, cpu );
			ticks = 11;
			goto interrupt_exit;
		}
		cpu->interrupt = false;
		// This depends on the mode
		switch( cpu->interrupt_mode )
		{
		case 0:
			// interrupting device supplies instruction.
			IF_ID( &inst, 0, (byte (*)(word, void*))cpu->ReadInterruptData, cpu );
			inst.additional_tstates += 2; // 2 wait states add to M1 cycle
			(cpu->ControlFlow)( oldPC, 0xffff, CF_INTERRUPT, cpu );
			break;
		case 1:
			// Insert a restart instruction + 2 cycles.
			// Handle it here since we know exactly what
			// it is supposed to do.
			(cpu->WriteMem)( --SP, PCH, cpu );
			(cpu->WriteMem)( --SP, PCL, cpu );
			PC = 0x0038; // Fixed location
			(cpu->ControlFlow)( oldPC, PC, CF_INTERRUPT, cpu );
			ticks = 13;
			goto interrupt_exit;
		case 2:
			// 7 cycles to read the 7 bits from the
			// interrupting device, 6 to push the PC, and
			// 6 to load the jump address.
			(cpu->WriteMem)( --SP, PCH, cpu );
			(cpu->WriteMem)( --SP, PCL, cpu );
			word address = ((word)BYTE_REG[REG_I]) << 8;
			address |= (cpu->ReadInterruptData)( 0, cpu ) & 0xfe;
			PCL = (cpu->ReadMem)( address, true, cpu );
			PCH = (cpu->ReadMem)( address+1, true, cpu );
			(cpu->ControlFlow)( oldPC, PC, CF_INTERRUPT, cpu );
			ticks = 19;
			goto interrupt_exit;
		}
	}
	else
	{
		// ei re-enables iff1 but interrupts cannot be handled
		// for another instruction.
		cpu->can_handle_interrupt = true;
		// Fetch the next instruction from memory.
		if( !cpu->halt )
		{
			PC += IF_ID( &inst, PC, ReadInstructionMemory, cpu );
		}
		else
		{
			inst.additional_tstates = 0;
			inst.offset = 0;
			inst.immediate = 0;
			inst.r_increment = 1;
			inst.IT = &Unprefixed[0x76]; // NOP
		}
	}

#define OP1 (inst.IT->operand1)
#define OP2 (inst.IT->operand2)
#define OFFSET (inst.offset)
#define IMM (inst.immediate)
	uint_fast32_t op1 = 0;
	uint_fast32_t op2 = 0;
	uint_fast32_t carry = 0;
	uint_fast32_t result = 0;
	int i = 0;
	bool took_branch = true;

	switch( inst.IT->type )
	{
	/* 8-Bit Load Group */
	case LD_I_N:
	case LD_MRR_N:
		(cpu->WriteMem)( WORD_REG[OP1]+OFFSET, IMM, cpu );
		break;

	case LD_I_R:
	case LD_MRR_R:
		(cpu->WriteMem)( WORD_REG[OP1]+OFFSET, BYTE_REG[OP2], cpu );
		break;

	case LD_MNN_R:
		(cpu->WriteMem)( IMM, BYTE_REG[OP2], cpu );
		break;

	case LD_R_I:
	case LD_R_MRR:
		result = (cpu->ReadMem)( WORD_REG[OP2]+OFFSET, false, cpu );
		BYTE_REG[OP1] = result;
		break;

	case LD_R_MNN:
		BYTE_REG[OP1] = (cpu->ReadMem)( IMM, false, cpu );
		break;

	case LD_R_N:
		BYTE_REG[OP1] = IMM;
		break;

	case LD_R_R:
		result = BYTE_REG[OP2];
		BYTE_REG[OP1] = result;
		if( OP2 == REG_I || OP2 == REG_R )
		{
			SetFlagValue( FLAG_S, result & 0x80 );
			SetFlagValue( FLAG_Z, !result );
			SetFlagValue( FLAG_Y, result & 0x20 );
			ResetFlag( FLAG_H );
			SetFlagValue( FLAG_X, result & 0x08 );
			SetFlagValue( FLAG_P, cpu->iff2 );
			ResetFlag( FLAG_N );
		}
		break;


	/* 16-Bit Load Group */
	case LD_RR_MNN:
		result = (cpu->ReadMem)( IMM, false, cpu );
		result |= (cpu->ReadMem)( IMM+1, false, cpu ) << 8;
		WORD_REG[OP1] = result;
		break;

	case LD_RR_NN:
		WORD_REG[OP1] = IMM;
		break;

	case LD_RR_RR:
		WORD_REG[OP1] = WORD_REG[OP2];
		break;

	case LD_MNN_RR:
		result = WORD_REG[OP2];
		(cpu->WriteMem)( IMM, result & 0xff, cpu );
		(cpu->WriteMem)( IMM+1, result >> 8, cpu );
		break;

	case POP_RR:
		result = (cpu->ReadMem)( SP++, false, cpu );
		result |= (cpu->ReadMem)( SP++, false, cpu ) << 8;
		WORD_REG[OP1] = result;
		break;

	case PUSH_RR:
		result = WORD_REG[OP1];
		(cpu->WriteMem)( --SP, result >> 8, cpu );
		(cpu->WriteMem)( --SP, result & 0xff, cpu );
		break;


	/* Exchange, Block Transfer, Search Group */
	case CPD:
		carry = -1;
		goto cpx;
	case CPI:
		carry = 1;
	cpx:
		op1 = A;
		op2 = (cpu->ReadMem)( HL, false, cpu );
		HL += carry;
		--BC;
		result = op1 - op2;
		goto cp_flags;
	case CPDR:
		carry = -1;
		goto cpxr;
	case CPIR:
		carry = 1;
	cpxr:
		op1 = A;
		op2 = (cpu->ReadMem)( HL, false, cpu );
		HL += carry;
		--BC;
		result = op1 - op2;
		if( (result&0xff) && BC )
			PC -= 2;
	cp_flags:
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !(result&0xff) );
		result -= FlagIsSet( FLAG_H );
		SetFlagValue( FLAG_H, (op1&0x03) < (op2&0x03) );
		SetFlagValue( FLAG_P, BC != 0 );
		SetFlag( FLAG_N );
		SetFlagValue( FLAG_Y, result & 0x02 ); // bit 1
		SetFlagValue( FLAG_X, result & 0x08 ); // bit 3
		break;

	case EX_MRR_RR:
		result = WORD_REG[OP1];
		op1 = (cpu->ReadMem)( result, false, cpu );
		op1 |= (cpu->ReadMem)( result+1, false, cpu ) << 8;
		op2 = WORD_REG[OP2];
		(cpu->WriteMem)( result, op2&0xff, cpu );
		(cpu->WriteMem)( result+1, op2>>8, cpu );
		WORD_REG[OP2] = op1;
		break;

	case EX_RR_RR:
		result = WORD_REG[OP1];
		WORD_REG[OP1] = WORD_REG[OP2];
		WORD_REG[OP2] = result;
		break;

	case EXX:
		op1 = BC;
		op2 = DE;
		result = HL;
		BC = WORD_REG[REG_BCP];
		DE = WORD_REG[REG_DEP];
		HL = WORD_REG[REG_HLP];
		WORD_REG[REG_BCP] = op1;
		WORD_REG[REG_DEP] = op2;
		WORD_REG[REG_HLP] = result;
		break;

	case LDD:
		carry = -1;
		goto ldx;
	case LDI:
		carry = 1;
	ldx:
		result = (cpu->ReadMem)( HL, false, cpu );
		(cpu->WriteMem)( DE, result, cpu );
		HL += carry;
		DE += carry;
		 --BC;
		 goto ld_flags;
	case LDDR:
		carry = -1;
		goto ldxr;
	case LDIR:
		carry = 1;
	ldxr:
		result = (cpu->ReadMem)( HL, false, cpu );
		(cpu->WriteMem)( DE, result, cpu );
		HL += carry;
		DE += carry;
		if( --BC )
			PC -= 2;
	ld_flags:
		// Very strange here
		op2 = result + A;
		SetFlagValue( FLAG_Y, op2&0x02 ); // bit 1
		ResetFlag( FLAG_H );
		SetFlagValue( FLAG_X, op2&0x08 ); // bit 3
		SetFlagValue( FLAG_P, BC != 0 );
		ResetFlag( FLAG_N );
		break;


	/* 8-Bit Arithmetic and Logical Group */
	case ADC_R_I:
	case ADC_R_MRR:
		op2 = (cpu->ReadMem)( WORD_REG[OP2]+OFFSET, false, cpu );
		goto adc_r;
	case ADC_R_N:
		op2 = IMM;
		goto adc_r;
	case ADC_R_R:
		op2 = BYTE_REG[OP2];
		goto adc_r;

	case ADD_R_I:
	case ADD_R_MRR:
		op2 = (cpu->ReadMem)( WORD_REG[OP2]+OFFSET, false, cpu );
		goto add_r;
	case ADD_R_N:
		op2 = IMM;
		goto add_r;
	case ADD_R_R:
		op2 = BYTE_REG[OP2];
		goto add_r;
	adc_r:
		carry = FlagIsSet( FLAG_C );
	add_r:
		op1 = BYTE_REG[OP1];
		result = op1 + op2 + carry;
		BYTE_REG[OP1] = result & 0xff;
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !(result & 0xff) );
		SetFlagValue( FLAG_Y, result & 0x20 ); // bit 5
		SetFlagValue( FLAG_H, (op1&0x0f)+(op2&0x0f)+carry>0x0f );
		SetFlagValue( FLAG_X, result & 0x08 ); // bit 3
		SetFlagValue( FLAG_P, (op2 == 0x7f && carry) ||
				      ((op1&0x80)==(op1&0x80) &&
				       (op1&0x80)!=(result&0x80)) );
		ResetFlag( FLAG_N );
		SetFlagValue( FLAG_C, result > 0xff );
		break;

	case SBC_R_I:
	case SBC_R_MRR:
		op2 = (cpu->ReadMem)( WORD_REG[OP2]+OFFSET, false, cpu );
		goto sbc_r;
	case SBC_R_N:
		op2 = IMM;
		goto sbc_r;
	case SBC_R_R:
		op2 = BYTE_REG[OP2];
		goto sbc_r;

	case SUB_I:
	case SUB_MRR:
		op2 = (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		goto sub_r;
	case SUB_N:
		op2 = IMM;
		goto sub_r;
	case SUB_R:
		op2 = BYTE_REG[OP1];
		goto sub_r;
	sbc_r:
		carry = FlagIsSet( FLAG_C );
	sub_r:
		op1 = A;
		result = op1 - op2 - carry;
		A = result & 0xff;
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !(result & 0xff) );
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlagValue( FLAG_H, (op1&0x0f) < (op2&0x0f)+carry );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, (carry && op1-op2 == 0x80) ||
				      ((op1&0x80) != (op2&0x80) &&
				       (op1&0x80) != (result&0x80)) );
		SetFlagValue( FLAG_C, op1 < op2 + carry );
		break;

	case DEC_I:
	case DEC_MRR:
		result = (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		carry = result;
		--result;
		(cpu->WriteMem)( WORD_REG[OP1]+OFFSET, result & 0xff, cpu );
		goto dec_x;
	case DEC_R:
		result = BYTE_REG[OP1];
		carry = result;
		BYTE_REG[OP1] = --result;
	dec_x:
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !(result&0xff) );
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlagValue( FLAG_H, !(carry&0x0f) );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, carry == 0x80 );
		SetFlag( FLAG_N );
		break;
		

	case INC_I:
	case INC_MRR:
		result = (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		carry = result;
		++result;
		(cpu->WriteMem)( WORD_REG[OP1]+OFFSET, result, cpu );
		goto inc_x;
	case INC_R:
		result = BYTE_REG[OP1];
		carry = result;
		++result;
		BYTE_REG[OP1] = result;
	inc_x:
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !(result&0xff) );
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlagValue( FLAG_H, (carry&0x0f)+1 > 0x0f );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, carry & 0x7f );
		ResetFlag( FLAG_N );
		break;

	case CP_I:
	case CP_MRR:
		op2 = (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		goto cp;
	case CP_N:
		op2 = IMM;
		goto cp;
	case CP_R:
		op2 = BYTE_REG[OP1];
	cp:
		op1 = A;
		result = op1 - op2;
                SetFlagValue( FLAG_S, result&0x80 ); 
                SetFlagValue( FLAG_Z, !(result&0xff) );
		SetFlagValue( FLAG_Y, result & 0x20 );
                SetFlagValue( FLAG_H, (op1&0x0f) < (op2&0x0f) );
		SetFlagValue( FLAG_X, result & 0x08 );
                SetFlagValue( FLAG_P, (op1&0x80) != (op2&0x80) &&
				      (op1&0x80) != (result&0x80) );
                SetFlag( FLAG_N );
		SetFlagValue( FLAG_C, op1 < op2 );
                break;

	case AND_I:
	case AND_MRR:
		result = A &= (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		SetFlag( FLAG_H );
		goto logical_flags;
	case AND_N:
		result = A &= IMM;
		SetFlag( FLAG_H );
		goto logical_flags;
	case AND_R:
		result = A &= BYTE_REG[OP1];
		SetFlag( FLAG_H );
		goto logical_flags;

	case OR_I:
	case OR_MRR:
		result = A | (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		ResetFlag( FLAG_H );
		goto logical_flags; /* Same flags as and */
	case OR_N:
		result = A | IMM;
		ResetFlag( FLAG_H );
		goto logical_flags;
	case OR_R:
		result = A | BYTE_REG[OP1];
		ResetFlag( FLAG_H );
		goto logical_flags;

	case XOR_I:
	case XOR_MRR:
		result = A ^ (cpu->ReadMem)( WORD_REG[OP1]+OFFSET, false, cpu );
		ResetFlag( FLAG_H );
		goto logical_flags;
	case XOR_N:
		result = A ^ IMM;
		ResetFlag( FLAG_H );
		goto logical_flags;
	case XOR_R:
		result = A ^ BYTE_REG[OP1];
		ResetFlag( FLAG_H );
	logical_flags:
		A = result;
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !result );
		SetFlagValue( FLAG_Y, result & 0x02 );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, ParityIsEven(result) );
		ResetFlag( FLAG_N );
		ResetFlag( FLAG_C );
		break;


	/* General-Purpose Arithmetic and CPU Control Group */
	case CCF:
		result = FlagIsSet( FLAG_C );
		SetFlagValue( FLAG_Y, A & 0x20 );
		SetFlagValue( FLAG_H, result );
		SetFlagValue( FLAG_X, A & 0x08 );
		ResetFlag( FLAG_N );
		SetFlagValue( FLAG_C, !result );
		break;

	case CPL:
		result = ~A;
		A = result;
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlag( FLAG_H );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlag( FLAG_N );
		break;

	case DAA:
		op1 = FlagIsSet( FLAG_N );
		op2 = FlagIsSet( FLAG_H );
		carry = FlagIsSet( FLAG_C );
		result = A;

		static const uint8_t daa_table[13][9] =
		{
			/*N  C  hi   hi   H  lo   lo   add   C */
			{ 0, 0, 0x0, 0x9, 0, 0x0, 0x9, 0x00, 0 },
			{ 0, 0, 0x0, 0x8, 0, 0xA, 0xF, 0x06, 0 },
			{ 0, 0, 0x0, 0x9, 1, 0x0, 0x3, 0x06, 0 },
			{ 0, 0, 0xA, 0xF, 0, 0x0, 0x9, 0x60, 1 },
			{ 0, 0, 0x9, 0xF, 0, 0xA, 0xF, 0x66, 1 },
			{ 0, 0, 0xA, 0xF, 1, 0x0, 0x3, 0x66, 1 },
			{ 0, 1, 0x0, 0x2, 0, 0x0, 0x9, 0x60, 1 },
			{ 0, 1, 0x0, 0x2, 0, 0xA, 0xF, 0x66, 1 },
			{ 0, 1, 0x0, 0x3, 1, 0x0, 0x3, 0x66, 1 },
			{ 1, 0, 0x0, 0x9, 0, 0x0, 0x9, 0x00, 0 },
			{ 1, 0, 0x0, 0x8, 1, 0x6, 0xF, 0xFA, 0 },
			{ 1, 1, 0x7, 0xF, 0, 0x0, 0x9, 0xA0, 1 },
			{ 1, 1, 0x6, 0xF, 1, 0x6, 0xF, 0x9A, 1 },	
		};
		for( i = 0; i < 13; ++i )
		{
			if( daa_table[i][0] == op1 &&
			    daa_table[i][1] == carry &&
			    daa_table[i][2] <= result >> 4 &&
			    daa_table[i][3] >= result >> 4 &&
			    daa_table[i][4] == op2 &&
			    daa_table[i][5] <= (result & 0x0f) &&
			    daa_table[i][6] >= (result & 0x0f) )
			{
				result = (result + daa_table[i][7]) & 0xff;
				SetFlagValue( FLAG_C, daa_table[i][7] );
				break;
			}
		}
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !result );
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, ParityIsEven(result) );
		break;

	case DI:
		cpu->iff1 = false;
		cpu->iff2 = false;
		break;

	case EI:
		cpu->iff1 = true;
		cpu->iff2 = true;
		cpu->can_handle_interrupt = false;
		break;

	case HALT:
		cpu->halt = true;
		(cpu->ControlFlow)( oldPC, PC, CF_HALT, cpu );
		break;

	case IM:
		cpu->interrupt_mode = OP1;
		break;

	case NEG: // This does A <- 0 - A, flags set accordingly.
		result = -A;
		A = result & 0xff;
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !result );
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlagValue( FLAG_H, result & 0x0f );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, (result&0xff) == 0x80 );
		SetFlag( FLAG_N );
		SetFlagValue( FLAG_C, !result );
		break;

	case NOP:
		break;

	case SCF:
		SetFlagValue( FLAG_Y, A & 0x20 );
		ResetFlag( FLAG_H );
		SetFlagValue( FLAG_X, A & 0x08 );
		ResetFlag( FLAG_N );
		SetFlag( FLAG_C );
		break;


	/* 16-Bit Arithmetic Group */
	case ADD_RR_RR:
		op1 = WORD_REG[OP1];
		op2 = WORD_REG[OP2];
		result = op1 + op2;
		goto add_rr_flags;
	case ADC_RR_RR:
		op1 = WORD_REG[OP1];
		op2 = WORD_REG[OP2];
		carry = FlagIsSet( FLAG_C );
		result = op1 + op2 + carry;
		SetFlagValue( FLAG_S, result & 0x8000 );
		SetFlagValue( FLAG_Z, !(result & 0xffff) );
		SetFlagValue( FLAG_P, (op2 == 0x7fff && carry) ||
				      ((op1&0x8000) == ((op2+carry)&0x8000) &&
				       (op1&0x8000) != (result&0x8000)) );
	add_rr_flags:
		SetFlagValue( FLAG_Y, result & 0x2000 ); // bit 13
		SetFlagValue( FLAG_H, (op1&0x0fff)+(op2&0x0fff)+carry>0x0fff );
		SetFlagValue( FLAG_X, result & 0x0800 ); // bit 11
		ResetFlag( FLAG_N );
		SetFlagValue( FLAG_C, result > 0xffff );
		WORD_REG[OP1] = result & 0xffff;
		break;

	case DEC_RR:
		--WORD_REG[OP1];
		break;

	case INC_RR:
		++WORD_REG[OP1];
		break;

	case SBC_RR_RR:
		op1 = WORD_REG[OP1];
		op2 = WORD_REG[OP2];
		carry = FlagIsSet( FLAG_C );
		result = op1 - op2 - carry;
		WORD_REG[OP1] = result & 0xffff;
		SetFlagValue( FLAG_S, result & 0x8000 );
		SetFlagValue( FLAG_Z, !(result & 0xffff) );
		SetFlagValue( FLAG_Y, result & 0x2000 );
		SetFlagValue( FLAG_H, (op1&0x0fff) < (op2&0x0fff)+carry );
		SetFlagValue( FLAG_X, result & 0x0800 );
		SetFlagValue( FLAG_P, (carry && op1-op2 == 0x8000) ||
				      ((op1&0x8000) != (op2&0x8000) &&
				       (op1&0x8000) != (result&0x8000)) );
		SetFlagValue( FLAG_C, op1 < op2 + carry );
		break;


	/* Rotate and Shift Group
	 * Almost all flags are set the same so jump to a common block
	 * of flag setting. */
	case RLCA:
		result = A;
		carry = result >> 7;
		result = (result << 1) | carry;
		A = result & 0xff;
		goto rotate_accum_flags;
	case RLA:
		result = A;
		carry = result >> 7;
		result = (result << 1) | FlagIsSet(FLAG_C);
		A = result & 0xff;
		goto rotate_accum_flags;
	case RRCA:
		result = A;
		carry = result & 0x1;
		result = (result >> 1) | (carry << 7);
		A = result;
		goto rotate_accum_flags;

	case RRA:
		result = A;
		carry = result & 0x1;
		result = (result >> 1) | (FlagIsSet(FLAG_C) << 7);
		A = result;
		goto rotate_accum_flags;
	case RLC_I:
	case RLC_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result >> 7;
		result = (result << 1) | carry;
		(cpu->WriteMem)( op1, result & 0xff, cpu );
		goto shift_flags;
	case RLC_R:
		result = BYTE_REG[OP1];
		carry = result >> 7;
		result = (result << 1) | carry;
		BYTE_REG[OP1] = result;
		goto shift_flags;
	case RL_I:
	case RL_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result >> 7;
		result = (result << 1) | FlagIsSet(FLAG_C);
		(cpu->WriteMem)( op1, result & 0xff, cpu );
		goto shift_flags;
	case RL_R:
		result = BYTE_REG[OP1];
		carry = result >> 7;
		result = (result << 1) | FlagIsSet(FLAG_C);
		BYTE_REG[OP1] = result & 0xff;
		goto shift_flags;
	case RRC_I:
	case RRC_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result & 0x1;
		result = (result >> 1) | (carry << 7);
		(cpu->WriteMem)( op1, result, cpu );
		goto shift_flags;
	case RRC_R:
		result = BYTE_REG[OP1];
		carry = result & 0x1;
		result = (result >> 1) | (carry << 7);
		BYTE_REG[OP1] = result;
		goto shift_flags;
	case RR_I:
	case RR_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result & 0x1;
		result = (result >> 1) | (FlagIsSet(FLAG_C) << 7);
		(cpu->WriteMem)( op1, result, cpu );
		goto shift_flags;
	case RR_R:
		result = BYTE_REG[OP1];
		carry = result & 0x1;
		result = (result >> 1) | (FlagIsSet(FLAG_C) << 7);
		BYTE_REG[OP1] = result;
		goto shift_flags;
	case SLA_I:
	case SLA_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result >> 7;
		result <<= 1;
		(cpu->WriteMem)( op1, result & 0xff, cpu );
		goto shift_flags;
	case SLA_R:
		result = BYTE_REG[OP1];
		carry = result >> 7;
		result <<= 1;
		BYTE_REG[OP1] = result & 0xff;
		goto shift_flags;
	case SLL_I:
	case SLL_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu);
		carry = result >> 7;
		result = (result << 1) | 0x1;
		(cpu->WriteMem)( op1, result & 0xff, cpu );
		goto shift_flags;
	case SLL_R:
		result = BYTE_REG[OP1];
		carry = result >> 7;
		result = (result << 1) | 0x1;
		BYTE_REG[OP1] = result & 0xff;
		goto shift_flags;
	case SRA_I:
	case SRA_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result & 0x1;
		result = (result & 0x80) | (result >> 1);
		(cpu->WriteMem)( op1, result, cpu );
		goto shift_flags;
	case SRA_R:
		result = BYTE_REG[OP1];
		carry = result & 0x1;
		result = (result & 0x80) | (result >> 1);
		BYTE_REG[OP1] = result;
		goto shift_flags;
	case SRL_I:
	case SRL_MRR:
		op1 = WORD_REG[OP1]+OFFSET;
		result = (cpu->ReadMem)( op1, false, cpu );
		carry = result & 0x1;
		result >>= 1;
		(cpu->WriteMem)( op1, result, cpu );
		goto shift_flags;
	case SRL_R:
		result = BYTE_REG[OP1];
		carry = result & 0x1;
		result >>= 1;
		BYTE_REG[OP1] = result;
		goto shift_flags;
	case RLD:
		op1 = (cpu->ReadMem)( HL, false, cpu );
		op2 = A;
		result = (op2 & 0xf0) | (op1 >> 4);
		op1 = (op1 << 4) | (op2 & 0x0f);
		(cpu->WriteMem)( HL, op1 & 0xff, cpu );
		A = result;
		carry = FlagIsSet( FLAG_C ); // makes code simpler
		goto shift_flags;
	case RRD:
		op1 = (cpu->ReadMem)( HL, false, cpu );
		op2 = A;
		result = (op2 & 0xf0) | (op1 & 0x0f);
		op1 = (op1 >> 4) | (op2 << 4);
		(cpu->WriteMem)( HL, op1 & 0xff, cpu );
		A = result;
		carry = FlagIsSet( FLAG_C ); // simpler code
	shift_flags:
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !result );
		SetFlagValue( FLAG_P, ParityIsEven(result) );
rotate_accum_flags:
		SetFlagValue( FLAG_Y, result & 0x20 );
		ResetFlag( FLAG_H );
		SetFlagValue( FLAG_X, result & 0x08 );
		ResetFlag( FLAG_N );
		SetFlagValue( FLAG_C, carry );
		break;


	/* Bit Set, Reset, and Test Group
	 * In this group, the first operand is the bit to
	 * set/reset/test. */
	case BIT_I:
	case BIT_MRR:
		op2 = (cpu->ReadMem)( WORD_REG[OP2]+OFFSET, false, cpu );
		result = op2 & (0x1<<OP1);
		// XXX: This is wrong for BIT_MRR, but right for BIT_I
		// Does anyone know what the right thing for BIT_MRR
		// is?
		SetFlagValue( FLAG_Y, (WORD_REG[OP2]+OFFSET)&0x20 );
		SetFlagValue( FLAG_X, (WORD_REG[OP2]+OFFSET)&0x08 );
		goto bit;
	case BIT_R:
		op2 = BYTE_REG[OP2];
		result = op2 & (0x1<<OP1);
		SetFlagValue( FLAG_Y, result & 0x20 );
		SetFlagValue( FLAG_X, result & 0x08 );
	bit:
		SetFlagValue( FLAG_S, OP1==7 && result );
		SetFlagValue( FLAG_Z, !result );
		SetFlag( FLAG_H );
		SetFlagValue( FLAG_P, !result );
		ResetFlag( FLAG_N );
		break;

	case RES_I:
	case RES_MRR:
		op2 = WORD_REG[OP2] + OFFSET;
		result = (cpu->ReadMem)( op2, false, cpu ) & ~(1<<OP1);
		(cpu->WriteMem)( op2, result, cpu );
		if( inst.IT->extra != INV )
			BYTE_REG[inst.IT->extra] = result;
		break;
	case RES_R:
		result = BYTE_REG[OP2] & ~(1<<OP1);
		BYTE_REG[OP2] = result;
		if( inst.IT->extra != INV )
			BYTE_REG[inst.IT->extra] = result;
		break;

	case SET_I:
	case SET_MRR:
		op2 = WORD_REG[OP2] + OFFSET;
		result = (cpu->ReadMem)( op2, false, cpu ) | (1<<OP1);
		(cpu->WriteMem)( op2, result, cpu );
		if( inst.IT->extra != INV )
			BYTE_REG[inst.IT->extra] = result;
		break;

	case SET_R:
		result = BYTE_REG[OP2] | (1<<OP1);
		BYTE_REG[OP2] = result;
		if( inst.IT->extra != INV )
			BYTE_REG[inst.IT->extra] = result;
		break;
	

	/* Jump Group */
	case DJNZ:
		if( --B )
			PC += OFFSET;
		else
			took_branch = false;
		break;

	case JP_C_MNN:
		if( !CondIsMet(OP1) )
		{
			took_branch = false;
			break; // condition isn't met
		}
	case JP_MNN:
		PC = IMM;
		(cpu->ControlFlow)( oldPC, PC, CF_JUMP, cpu );
		break;
	case JP_MRR: // jp (hl); jp (ix); jp (iy)
		PC = WORD_REG[OP1];
		(cpu->ControlFlow)( oldPC, PC, CF_JUMP, cpu );
		break;
	
	case JR_C:
		if( !CondIsMet(OP1) )
		{
			took_branch = false;
			break;
		}
	case JR:
		PC += OFFSET;
		break;

	/* Call and Return Group */
	case CALL_C_MNN:
		if( !CondIsMet(OP1) )
		{
			took_branch = false;
			break; // condition isn't met
		}
	case CALL_MNN:
		(cpu->WriteMem)( --SP, PCH, cpu );
		(cpu->WriteMem)( --SP, PCL, cpu );
		PC = IMM;
		(cpu->ControlFlow)( oldPC, PC, CF_CALL, cpu );
		break;

	case RETI:
		// Some docs say that iff2 is copied to iff1 like in
		// reti. Some simulatores do that. Zilog docs are very
		// clear that this doesn't happen, but they're often
		// wrong.
		(cpu->InterruptComplete)( cpu );
		result = CF_RETURN_I;
		goto ret;
	case RETN:
		cpu->iff1 = cpu->iff2;
		result = CF_RETURN_N;
		goto ret;
	case RET_C:
		if( !CondIsMet(OP1) )
		{
			took_branch = false;
			break;
		}
	case RET:
		result = CF_RETURN;
	ret:
		PCL = (cpu->ReadMem)( SP++, false, cpu );
		PCH = (cpu->ReadMem)( SP++, false, cpu );
		(cpu->ControlFlow)( oldPC, PC, result, cpu );
		break;

	case RST:
		(cpu->WriteMem)( --SP, PCH, cpu );
		(cpu->WriteMem)( --SP, PCL, cpu );
		PC = OP1;
		(cpu->ControlFlow)( oldPC, PC, CF_RESTART, cpu );
		break;

	/* Input and Output Group */
	case IND:
		carry = -1;
		goto inx;
	case INI:
		carry = 1;
	inx:
		result = (cpu->ReadIO)( BC, cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		(cpu->WriteMem)( HL, result, cpu );
		op1 = --B;
		HL += carry;
	in_flags:
		SetFlagValue( FLAG_S, op1 & 0x80 );
		SetFlagValue( FLAG_Z, !(op1&0xff) );
		SetFlagValue( FLAG_Y, op1 & 0x20 );
		SetFlagValue( FLAG_X, op1 & 0x08 );
		SetFlagValue( FLAG_N, result & 0x80 );
		op2 = result + ((C+carry) & 0xff);
		SetFlagValue( FLAG_H, op2 > 0xff );
		SetFlagValue( FLAG_C, op2 > 0xff );
		SetFlagValue( FLAG_P, ParityIsEven((op2&0x07)^op1) );
		break;
	case INDR:
		carry = -1;
		goto inxr;
	case INIR:
		carry = 1;
	inxr:
		result = (cpu->ReadIO)( BC, cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		(cpu->WriteMem)( HL, result, cpu );
		op1 = --B;
		HL += carry;
		if( op1 == 0 )
			took_branch = false;
		else
			PC -= 2;
		goto in_flags;

	case IN_R_MN: //in a,(n)
		result = (cpu->ReadIO)( (A<<8)|IMM, cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		A = result;
		break;
	case IN_R_R: // in r,(c)
		result = (cpu->ReadIO)( BC, cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		if( OP1 != REG_F )
			BYTE_REG[OP1] = result;
		SetFlagValue( FLAG_S, result & 0x80 );
		SetFlagValue( FLAG_Z, !result );
		SetFlagValue( FLAG_Y, result & 0x20 );
		ResetFlag( FLAG_H );
		SetFlagValue( FLAG_X, result & 0x08 );
		SetFlagValue( FLAG_P, ParityIsEven(result) );
		ResetFlag( FLAG_N );
		break;

	case OTDR:
		carry = -1;
		goto otxr;
	case OTIR:
		carry = 1;
	otxr:
		result = (cpu->ReadMem)( HL, false, cpu );
		op1 = --B;
		(cpu->WriteIO)( BC, result, cpu );
		if( cpu->restart_io )
		{
			++B; // Fix up
			PC = oldPC;
			goto early_exit;
		}
		HL += carry;
		if( op1 )
			PC -= 2;
		else
			took_branch = false;
	out_flags:
		// More flag strangeness
		op2 = result + L;
		SetFlagValue( FLAG_S, op1 & 0x80 );
		SetFlagValue( FLAG_Z, !op1 );
		SetFlagValue( FLAG_Y, op1 & 0x20 );
		SetFlagValue( FLAG_H, op2 > 0xff );
		SetFlagValue( FLAG_X, op1 & 0x08 );
		SetFlagValue( FLAG_P, ParityIsEven((op2&0x07) ^ op1) );
		SetFlagValue( FLAG_N, result & 0x80 );
		SetFlagValue( FLAG_C, op2 > 0xff );
		break;
	case OUTD:
		carry = -1;
		goto outx;
	case OUTI:
		carry = 1;
	outx:
		result = (cpu->ReadMem)( HL, false, cpu );
		op1 = --B;
		(cpu->WriteIO)( BC, result, cpu );
		if( cpu->restart_io )
		{
			++B; // fix up
			PC = oldPC;
			goto early_exit;
		}
		HL += carry;
		goto out_flags;

	case OUT_MN_R: // out (n),a
		(cpu->WriteIO)( (A<<8)|IMM, A, cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		break;

	case OUT_R: // out (c),0
		// I don't know what is on the top half of the address
		// bus during this, I'm guessing B.
		(cpu->WriteIO)( BC, 0, cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		break;

	case OUT_R_R: // out (c),r
		(cpu->WriteIO)( BC, BYTE_REG[OP2], cpu );
		if( cpu->restart_io )
		{
			PC = oldPC;
			goto early_exit;
		}
		break;
	}
#undef OP1
#undef OP2
#undef OFFSET
#undef IMM

	ticks = took_branch? inst.IT->tstates:inst.IT->extra;
	ticks += inst.additional_tstates;

interrupt_exit:
	r = BYTE_REG[REG_R];
	r = ((r + inst.r_increment) & 0x7f) | (r & 0x80);
	BYTE_REG[REG_R] = r;

early_exit:
	if( outPC )
		*outPC = PC;
	return ticks;
}

int Z80_Disassemble( word address, char *buffer, Z80 cpu )
{
	Instruction inst;
	int length = IF_ID( &inst, address, ReadInstructionMemory, cpu );
	if( buffer != NULL )
		DisassembleInstruction( &inst, buffer );
	return length;
}

bool Z80_HasHalted( Z80 cpu )
{
	return cpu->halt;
}

word Z80_GetReg( int reg, Z80 cpu )
{
	assert( reg >= 0 && reg < NUM_REG );
	return WORD_REG[reg];
}

void Z80_SetReg( int reg, word value, Z80 cpu )
{
	assert( reg >= 0 && reg < NUM_REG );
	WORD_REG[reg] = value;
}

void Z80_RaiseNMI( Z80 cpu )
{
	cpu->nmi = true;
}

void Z80_RaiseInterrupt( Z80 cpu )
{
	cpu->interrupt = true;
}

void Z80_RestartIO( Z80 cpu )
{
	cpu->restart_io = true;
}

void Z80_ClearHalt( Z80 cpu )
{
	cpu->halt = false;
}
