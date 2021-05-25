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
 * Create and run a z80 processor instance.
 * \author Stephen Checkoway
 * \version 0.1
 * \date 2008, 2017
 */
#ifndef ZEL_Z80_H
#define ZEL_Z80_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/*! Opaque type representing a z80 processor. */
typedef struct Z80_t *Z80;

/* NOTE: Changing the order of these requires changing z80_instructions.h! */
/*! 16 bit paired z80 registers. The registers ending in P are the
 * primed registers.
 */
enum
{
	REG_BC,  //!< z80 register bc.
	REG_DE,  //!< z80 register de.
	REG_HL,  //!< z80 register hl.
	REG_AF,  //!< z80 register af.
	REG_IX,  //!< z80 register ix.
	REG_IY,  //!< z80 register iy.
	REG_PC,  //!< z80 register pc.
	REG_SP,  //!< z80 register sp.
	REG_BCP, //!< z80 register bc'.
	REG_DEP, //!< z80 register de'.
	REG_HLP, //!< z80 register hl'.
	REG_AFP, //!< z80 register af'.
	REG_IR,  //!< z80 register ir.
	NUM_REG, //!< Number of 16 bit z80 paired registers.
};

/*! Enumerated value describing the control flow of the processor.
 * \sa Z80FunctionBlock
 */
typedef enum
{
	CF_CALL,	//!< Call instruction.
	CF_JUMP,	//!< Jump instruction.
	CF_RETURN,	//!< Return instruction.
	CF_RETURN_I,	//!< Return from interrupt instruction.
	CF_RETURN_N,	//!< Return from nonmaskable interrupt instruction.
	CF_RESTART,	//!< Restart instruction.
	CF_INTERRUPT,	//!< Maskable interrupt.
	CF_NMI,		//!< Nonmaskable interrupt.
	CF_HALT,	//!< Halt instruction.
} ControlFlowType;

/*! A block of callbacks used by Z80_New() to control how the
 * z80 interracts with its peripherials.
 * \headerfile z80.h zel/z80.h
 */
typedef struct
{
	/*! Read a byte of memory.
	 * \param addr The address to read.
	 * \param inst True if the z80 is reading instructions.
	 * \param cpu The \c Z80 instance making the read call.
	 * \return The byte from memory.
	 */
	uint8_t (*ReadMem)(uint16_t addr, bool inst, Z80 cpu);
	/*! Write a byte of memory.
	 * \param addr The address to write.
	 * \param val The byte to write.
	 * \param cpu The \c Z80 instance making the write call.
	 */
	void (*WriteMem)(uint16_t addr, uint8_t val, Z80 cpu);
	/*! Read the interrupt data.
	 * \param n Read the \a n th byte of data.
	 * \param cpu The \c Z80 instance making the read call.
	 */
	uint8_t (*ReadInterruptData)(uint16_t n, Z80 cpu);
	/*! Read a byte from an I/O port.
	 * \param addr The contents of the address bus during the
	 * request. The low 8 bits specify the port.
	 * \param cpu The \c Z80 instance making the read call.
	 * \return The byte from the I/O port.
	 */
	uint8_t (*ReadIO)(uint16_t addr, Z80 cpu);
	/*! Write a byte from an I/O port.
	 * \param addr The contents of the address bus during the
	 * request. The low 8 bits specify the port.
	 * \param val The byte to write.
	 * \param cpu The \c Z80 instance making the read call.
	 */
	void (*WriteIO)(uint16_t addr, uint8_t val, Z80 cpu);
	/*! Notify the peripherials that a return from interrupt
	 * instruction has occured.
	 * \param cpu The \c Z80 instance performing the notification.
	 */
	void (*InterruptComplete)(Z80 cpu);
	/*! Optional notification of control flow. This can be set to
	 * \c NULL if notification is not desired.
	 * \param pc The address of the current instruction.
	 * \param target The target address of the instruction. For
	 * example, the jump target.
	 * \param type The type of control flow.
	 * \param cpu The \c Z80 instance performing the notification.
	 */
	void (*ControlFlow)(uint16_t pc, uint16_t target, ControlFlowType type, Z80 cpu);
} Z80FunctionBlock;

/*! Create a new \c Z80 instance using the callbacks specified in \a blk.
 * \param blk A pointer to a block of callbacks. Only
 * Z80FunctionBlock.ControlFlow() may be \c NULL.
 * \return The new \c Z80 instance.
 */
Z80 Z80_New( const Z80FunctionBlock *blk );

/*! Frees the memory associated with \a cpu.
 * \param cpu The \c Z80 instance to free.
 */
void Z80_Free( Z80 cpu );

/*! Perform a single step of the processor cpu.
 * \param outPC If non\c NULL, \a *outPC is set to the program counter after
 * the current instruction is executed.
 * \param cpu The \c Z80 instance to step.
 * \return The number of clock ticks that have elapsed while executing
 * the instruction.
 */
int Z80_Step( uint16_t *outPC, Z80 cpu );

/*! Check if \a cpu has halted.
 * \param cpu The \c Z80 instance.
 * \return \c true if \a cpu has halted.
 */
bool Z80_HasHalted( Z80 cpu );

/*! Get a 16 bit paired register.
 * \param reg The register to get.
 * \param cpu The \c Z80 instance.
 * \return The contents of the register specified by \a reg.
 */
uint16_t Z80_GetReg( int reg, Z80 cpu );

/*! Set a 16 bit paired register.
 * \param reg The register to set.
 * \param value The value to assign to the register.
 * \param cpu The \c Z80 instance.
 */
void Z80_SetReg( int reg, uint16_t value, Z80 cpu );

/*! Disassemble the z80 instruction pointed to by \a address into \a buffer.
 * \param address The address of the beginning the instruction.
 * \param buffer A pointer to at least 25 bytes of storage. If buffer
 * is \c NULL, then the instruction is not disassembled and only the
 * length is returned.
 * \param cpu The \c Z80 instance. The Z80FunctionBlock.ReadMem()
 * function will be called to read the instructions.
 * \return The length of the instruction in bytes.
 */
int Z80_Disassemble( uint16_t address, char *buffer, Z80 cpu );

/*! Simulate the NMI pin going active. This causes the z80 to jump to
 * the nmi handler.
 * \param cpu The Z80 instance.
 */
void Z80_RaiseNMI( Z80 cpu );

/*! Simulate the interrupt pin going active. If interrupts are not
 * disabled, then the z80 handles the interrupt according to the
 * interrupt mode.
 * \param cpu The Z80 instance.
 */
void Z80_RaiseInterrupt( Z80 cpu );

/*! Cause the z80 to reissue the I/O instruction. This only has an
 * effect when called during the Z80FunctionBlock.ReadIO() or
 * Z80FunctionBlock.WriteIO() callbacks. It is to enable debugging by
 * breaking on I/O.
 * \param cpu The \c Z80 instance.
 */
void Z80_RestartIO( Z80 cpu );

/*! Clear the halt condition. Causes the processor to continue
 * fetching instructions rather than performing NOPs.
 * \param cpu The \c Z80 instance.
 */
void Z80_ClearHalt( Z80 cpu );

#ifdef __cplusplus
}
#endif

#endif
