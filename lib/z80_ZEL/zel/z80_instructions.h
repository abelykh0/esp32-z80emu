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

/*! \file
 *
 * Functions for decoding and disassembling z80 instructions.
 * \author Stephen Checkoway
 * \version 0.1
 * \date 2008, 2017
 */
#ifndef ZEL_Z80_INSTRUCTIONS_H
#define ZEL_Z80_INSTRUCTIONS_H

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <zel/z80_instruction_types.h>

/*! The type of a z80 instruction. Many z80 instructions are similar
 * and share a common type. For example, <code>add a,b</code> and
 * <code>add a,c</code> have the same type.
 *
 * The type of each instruction can be decoded as follows. Given an
 * instruction \c FOO_BAR_BAZ, \c FOO is the mnemonic, \c BAR is the
 * type of the first operand and \c BAZ is the type of the second. The
 * operand types are as follows:
 * 	- \c N 8 bit immediate
 * 	- \c NN 16 bit immediate
 * 	- \c I 8 bit signed displacement from an index register
 * 	- \c R 8 bit register
 * 	- \c RR 16 bit paired register
 * 	- \c MRR 16 bit paired register used as an address
 * 	- \c MNN 16 bit immediate used as an address
 */

#ifndef BYTE_ORDER
#error define BYTE_ORDER
#endif
#if BYTE_ORDER == BIG_ENDIAN
/*! Register values for the 8 bit registers which comprise the 16 bit
 * paired registers.
 */
enum
{
	REG_B,   //!< z80 register b.
	REG_C,   //!< z80 register c.
	REG_D,   //!< z80 register d.
	REG_E,   //!< z80 register e.
	REG_H,   //!< z80 register h.
	REG_L,   //!< z80 register l.
	REG_A,   //!< z80 register a.
	REG_F,   //!< z80 register f.
	REG_IXH, //!< z80 register ixh.
	REG_IXL, //!< z80 register ixl.
	REG_IYH, //!< z80 register iyh.
	REG_IYL, //!< z80 register iyl.
	REG_PCH, //!< z80 register pch.
	REG_PCL, //!< z80 register pcl.
	/* No REG_SPH */
	/* No REG_SPL */
	/* No REG_BP */
	/* No REG_CP */
	/* No REG_DP */
	/* No REG_EP */
	/* No REG_HP */
	/* No REG_LP */
	/* No REG_AP */
	/* No REG_FP */
	REG_I = 24, //!< z80 register I. Interrupt page address register.
	REG_R, //!< z80 register r. Memory refresh register.
	INV = 0xff, //!< Invalid.
};
#elif BYTE_ORDER == LITTLE_ENDIAN
/*! Register values for the 8 bit registers which comprise the 16 bit
 * paired registers.
 */
enum
{
	REG_C,   //!< z80 register c.
	REG_B,   //!< z80 register b.
	REG_E,   //!< z80 register e.
	REG_D,   //!< z80 register d.
	REG_L,   //!< z80 register l.
	REG_H,   //!< z80 register h.
	REG_F,   //!< z80 register f.
	REG_A,   //!< z80 register a.
	REG_IXL, //!< z80 register ixl.
	REG_IXH, //!< z80 register ixh.
	REG_IYL, //!< z80 register iyl.
	REG_IYH, //!< z80 register iyh.
	REG_PCL, //!< z80 register pcl.
	REG_PCH, //!< z80 register pch.
	/* No single byte regs */
	REG_R = 24, //!< z80 register r. Memory refresh register.
	REG_I,   //!< z80 register I. Interrupt page address register.
	INV = 0xff, //!< Invalid.
};
#else
#error What endianness are you using?
#endif

/*! Condition flag bits as stored in register \c f.
 * The flags \c x and \c y are listed as unspecified by the z80 CPU
 * User's Manual but they are modified by a number of instructions.
 */
enum
{
	FLAG_C, //!< Carry flag.
	FLAG_N, //!< Add/subtract flag.
	FLAG_P, //!< Parity/overflow flag (P/V).
	FLAG_X, //!< Flag 3.
	FLAG_H, //!< Half-carry flag.
	FLAG_Y, //!< Flag 5.
	FLAG_Z, //!< Zero flag.
	FLAG_S, //!< Sign flag.
};

/*! Condition flags. The condition is met if the corresponding flag is
 * set appropriately.
 */
enum
{
	COND_NZ = -FLAG_Z-1, //!< Flag Z is reset.
	COND_Z  =  FLAG_Z,   //!< Flag Z is set.
	COND_NC = -FLAG_C-1, //!< Flag C is reset.
	COND_C  =  FLAG_C,   //!< Flag C is set.
	COND_PO = -FLAG_P-1, //!< Flag P/V is reset.
	COND_PE =  FLAG_P,   //!< Flag P/V is set.
	COND_P  = -FLAG_S-1, //!< Flag S is reset.
	COND_M  =  FLAG_S,   //!< Flag S is set.
};

/* Exactly two instructions contain both an offset and an immediate:
 * dd36 d n: ld (ix+d),n
 * fd36 d n: ld (iy+d),n */
/*! Describes the layout of the operands for an instruction. */
enum
{
	TYPE_NONE, 		//!< No operands.
	TYPE_IMM_N,		//!< 8 bit immediate.
	TYPE_IMM_NN,		//!< 16 bit immediate.
	TYPE_OFFSET,		//!< 8 bit signed offset.
	TYPE_DISP,		//!< 8 bit signed offset - 2.
	TYPE_OFFSET_IMM_N,	//!< 8 bit signed offset and 8 bit immediate.
};

// 4 (3) words native sized words on 32 (64) bit machine.
/*! Uniform template for the instruction tables.
 * \headerfile z80_instructions.h zel/z80_instructions.h
 */
typedef struct
{
	InstructionType type; //!< Type of the instruction.
	int16_t operand1; //!< Type of the first operand, if any.
	int16_t operand2; //!< Type of the second operand, if any.
	int16_t extra; //!< Type of third operand or tstates when a branch is taken, if applicable.
	uint8_t tstates; //!< Base number of clock ticks the instruction takes.
	uint8_t operand_types; //!< Operand layout in memory for the instruction.
	const char *format; //!< Format specifier string for disassembly.
} InstructionTemplate;

/*! Completely describes an instruction.
 * \headerfile z80_instructions.h zel/z80_instructions.h
 */
typedef struct
{
	const InstructionTemplate *IT;	//!< Template for the instruction.
	unsigned int immediate;		//!< Immediate value, if any.
	unsigned int additional_tstates;//!< Additional clock ticks.
	unsigned int r_increment;	//!< Amount by which the \c r register is incremented.
	int offset;			//!< Offset, if any.
} Instruction;


/*! Instruction table for unprefixed instructions. */
extern const InstructionTemplate Unprefixed[256];
/*! Instruction table for cb prefixed instructions. */
extern const InstructionTemplate CB_Prefixed[256];
/*! Instruction table for dd prefixed instructions. */
extern const InstructionTemplate DD_Prefixed[256];
/*! Instruction table for ddcb prefixed instructions. */
extern const InstructionTemplate DDCB_Prefixed[256];
/*! Instruction table for ed prefixed instructions. */
extern const InstructionTemplate ED_Prefixed[256];
/*! Instruction table for fd prefixed instructions. */
extern const InstructionTemplate FD_Prefixed[256];
/*! Instruction table for fdcb prefixed instructions. */
extern const InstructionTemplate FDCB_Prefixed[256];

/*! Memory reading callback.
 * \param addr The address to read.
 * \param data Callback data from IF_ID().
 * \return The value at address \a addr.
 */
typedef uint8_t (*ReadMemFunction)(uint16_t addr, void *data);

/*! Instruction fetch and instruction decode. Fetchs and decodes the
 * instruction pointed to by \a address into \a *inst.
 * \param inst Pointer to an \c Instruction. \a *inst is set to the
 * decoded instruction.
 * \param address The address of the instruction.
 * \param ReadMem Called repeatedly to get bytes of the instruction.
 * \a data is passed as the \c data argument.
 * \param data Arbitrary data passed to the \a ReadMem callback.
 * \return The length of the instruction.
 */
int IF_ID( Instruction *inst, uint16_t address, ReadMemFunction ReadMem, void *data );

/*! Disassemble the instruction pointed to by \a inst into \a buffer.
 * \param inst Pointer to an \c Instruction.
 * \param buffer Buffer into which the disassembly is written. It must
 * be large enough to hold 25 bytes.
 */
void DisassembleInstruction( const Instruction *inst, char *buffer );

#ifdef __cplusplus
}
#endif

#endif
