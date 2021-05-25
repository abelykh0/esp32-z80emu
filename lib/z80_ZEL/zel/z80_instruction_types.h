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
 * z80 instruction types.
 * \author Stephen Checkoway
 * \version 0.1
 * \date 2008, 2017
 */
#ifndef ZEL_Z80_INSTRUCTION_TYPES_H
#define ZEL_Z80_INSTRUCTION_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	ADC_RR_RR, //!< adc
	ADC_R_I, //!< adc
	ADC_R_MRR, //!< adc
	ADC_R_N, //!< adc
	ADC_R_R, //!< adc
	ADD_RR_RR, //!< add
	ADD_R_I, //!< add
	ADD_R_MRR, //!< add
	ADD_R_N, //!< add
	ADD_R_R, //!< add
	AND_I, //!< and
	AND_MRR, //!< and
	AND_N, //!< and
	AND_R, //!< and
	BIT_I, //!< bit
	BIT_MRR, //!< bit
	BIT_R, //!< bit
	CALL_C_MNN, //!< call
	CALL_MNN, //!< call
	CCF, //!< ccf
	CPD, //!< cpd
	CPDR, //!< cpdr
	CPI, //!< cpi
	CPIR, //!< cpir
	CPL, //!< cpl
	CP_I, //!< cp
	CP_MRR, //!< cp
	CP_N, //!< cp
	CP_R, //!< cp
	DAA, //!< daa
	DEC_I, //!< dec
	DEC_MRR, //!< dec
	DEC_R, //!< dec
	DEC_RR, //!< dec
	DI, //!< di
	DJNZ, //!< djnz
	EI, //!< ei
	EXX, //!< exx
	EX_MRR_RR, //!< ex
	EX_RR_RR, //!< ex
	HALT, //!< halt
	IM, //!< im
	INC_I, //!< inc
	INC_MRR, //!< inc
	INC_R, //!< inc
	INC_RR, //!< inc
	IND, //!< ind
	INDR, //!< indr
	INI, //!< ini
	INIR, //!< inir
	IN_R_MN, //!< in
	IN_R_R, //!< in
	JP_C_MNN, //!< jp
	JP_MNN, //!< jp
	JP_MRR, //!< jp
	JR, //!< jr
	JR_C, //!< jr
	LDD, //!< ldd
	LDDR, //!< lddr
	LDI, //!< ldi
	LDIR, //!< ldir
	LD_I_N, //!< ld
	LD_I_R, //!< ld
	LD_MNN_R, //!< ld
	LD_MNN_RR, //!< ld
	LD_MRR_N, //!< ld
	LD_MRR_R, //!< ld
	LD_RR_MNN, //!< ld
	LD_RR_NN, //!< ld
	LD_RR_RR, //!< ld
	LD_R_I, //!< ld
	LD_R_MNN, //!< ld
	LD_R_MRR, //!< ld
	LD_R_N, //!< ld
	LD_R_R, //!< ld
	NEG, //!< neg
	NOP, //!< nop
	OR_I, //!< or
	OR_MRR, //!< or
	OR_N, //!< or
	OR_R, //!< or
	OTDR, //!< otdr
	OTIR, //!< otir
	OUTD, //!< outd
	OUTI, //!< outi
	OUT_MN_R, //!< out
	OUT_R, //!< out
	OUT_R_R, //!< out
	POP_RR, //!< pop
	PUSH_RR, //!< push
	RES_I, //!< res
	RES_MRR, //!< res
	RES_R, //!< res
	RET, //!< ret
	RETI, //!< reti
	RETN, //!< retn
	RET_C, //!< ret
	RLA, //!< rla
	RLCA, //!< rlca
	RLC_I, //!< rlc
	RLC_MRR, //!< rlc
	RLC_R, //!< rlc
	RLD, //!< rld
	RL_I, //!< rl
	RL_MRR, //!< rl
	RL_R, //!< rl
	RRA, //!< rra
	RRCA, //!< rrca
	RRC_I, //!< rrc
	RRC_MRR, //!< rrc
	RRC_R, //!< rrc
	RRD, //!< rrd
	RR_I, //!< rr
	RR_MRR, //!< rr
	RR_R, //!< rr
	RST, //!< rst
	SBC_RR_RR, //!< sbc
	SBC_R_I, //!< sbc
	SBC_R_MRR, //!< sbc
	SBC_R_N, //!< sbc
	SBC_R_R, //!< sbc
	SCF, //!< scf
	SET_I, //!< set
	SET_MRR, //!< set
	SET_R, //!< set
	SLA_I, //!< sla
	SLA_MRR, //!< sla
	SLA_R, //!< sla
	SLL_I, //!< sll
	SLL_MRR, //!< sll
	SLL_R, //!< sll
	SRA_I, //!< sra
	SRA_MRR, //!< sra
	SRA_R, //!< sra
	SRL_I, //!< srl
	SRL_MRR, //!< srl
	SRL_R, //!< srl
	SUB_I, //!< sub
	SUB_MRR, //!< sub
	SUB_N, //!< sub
	SUB_R, //!< sub
	XOR_I, //!< xor
	XOR_MRR, //!< xor
	XOR_N, //!< xor
	XOR_R, //!< xor
} InstructionType;

#ifdef __cplusplus
}
#endif

#endif
