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
#include <stdio.h> /* for snprintf */
#include <string.h>
#include <zel/z80.h> /* Need registers */
#include <zel/z80_instructions.h>

#include "z80_types.h"

const InstructionTemplate Unprefixed[] =
{
#include "tables/no_prefix.tab"
};
const InstructionTemplate CB_Prefixed[] =
{
#include "tables/cb_prefix.tab"
};
const InstructionTemplate DD_Prefixed[] =
{
#include "tables/dd_prefix.tab"
};
const InstructionTemplate DDCB_Prefixed[] =
{
#include "tables/ddcb_prefix.tab"
};
const InstructionTemplate ED_Prefixed[] =
{
#include "tables/ed_prefix.tab"
};
const InstructionTemplate FD_Prefixed[] =
{
#include "tables/fd_prefix.tab"
};
const InstructionTemplate FDCB_Prefixed[] =
{
#include "tables/fdcb_prefix.tab"
};

int IF_ID( Instruction *inst, word address, ReadMemFunction ReadMem, void *data )
{
	byte opcode, opcode2 = 0;
	int length = 1;
	opcode = ReadMem( address, data );
	inst->additional_tstates = 0;
	inst->offset = 0;
	inst->immediate = 0;
	inst->r_increment = 1;
	if( opcode == 0xdd || opcode == 0xfd )
	{
		++length;
		++inst->r_increment;
		opcode2 = ReadMem( ++address, data );
		while( opcode2 == 0xdd || opcode2 == 0xfd )
		{
			inst->additional_tstates += 4;
			opcode = opcode2;
			++length;
			++inst->r_increment;
			opcode2 = ReadMem( ++address, data );
		}
		/* If cocode2 is 0xed, then the prefix should be
		 * completely ignored apart from the time it took to
		 * read and the length. */
		if( opcode2 == 0xed )
			opcode = opcode2;
	}

	switch( opcode )
	{
	case 0xcb:
		++length;
		++inst->r_increment;
		opcode = ReadMem( ++address, data );
		inst->IT = &CB_Prefixed[opcode];
		return length; // No immediates/offset
	case 0xdd:
		if( opcode2 == 0xcb )
		{
			length += 2;
			inst->offset = (sbyte)ReadMem( ++address, data );
			opcode = ReadMem( ++address, data );
			inst->IT = &DDCB_Prefixed[opcode];
			return length; // No immediates and offset is done
		}
		inst->IT = &DD_Prefixed[opcode2];
		break; // immediates
	case 0xed:
		++length;
		++inst->r_increment;
		opcode = ReadMem( ++address, data );
		inst->IT = &ED_Prefixed[opcode];
		break; // immediates
	case 0xfd:
		if( opcode2 == 0xcb )
		{
			length += 2;
			inst->offset = (sbyte)ReadMem( ++address, data );
			opcode = ReadMem( ++address, data );
			inst->IT = &FDCB_Prefixed[opcode];
			return length; // No immediates and offset is done
		}
		inst->IT = &FD_Prefixed[opcode2];
		break; // immediates
	default:
		inst->IT = &Unprefixed[opcode];
		break;
	}
	switch( inst->IT->operand_types )
	{
	case TYPE_NONE:
		break;
	case TYPE_IMM_N:
		++length;
		inst->immediate = ReadMem( ++address, data );
		break;
	case TYPE_OFFSET:
	case TYPE_DISP:
		++length;
		inst->offset = (sbyte)ReadMem( ++address, data );
		break;
	case TYPE_IMM_NN:
		length += 2;
		inst->immediate = ReadMem( ++address, data );
		inst->immediate |= ReadMem( ++address, data ) << 8;
		break;
	case  TYPE_OFFSET_IMM_N:
		length += 2;
		inst->offset = (sbyte)ReadMem( ++address, data );
		inst->immediate = ReadMem( ++address, data );
		break;
	}
	return length;
}

void DisassembleInstruction( const Instruction *inst, char *buffer )
{
	char c;
	int v;

	switch( inst->IT->operand_types )
	{
	case TYPE_NONE:
		strncpy( buffer, inst->IT->format, 25 );
		buffer[24] = '\0';
		break;
	case TYPE_IMM_N:
	case TYPE_IMM_NN:
		snprintf( buffer, 25, inst->IT->format, inst->immediate );
		break;
	case TYPE_OFFSET:
		if( inst->offset >= 0 )
		{
			c = '+';
			v = inst->offset;
		}
		else
		{
			c = '-';
			v = -inst->offset;
		}
		snprintf( buffer, 25, inst->IT->format, c, v );
		break;
	case TYPE_DISP:
		if( inst->offset >= -2 )
		{
			c = '+';
			v = inst->offset+2;
		}
		else
		{
			c = '-';
			v = -inst->offset - 2;
		}
		snprintf( buffer, 25, inst->IT->format, c, v );
		break;
	case TYPE_OFFSET_IMM_N:
		if( inst->offset >= 0 )
		{
			c = '+';
			v = inst->offset;
		}
		else
		{
			c = '-';
			v = -inst->offset;
		}
		snprintf( buffer, 25, inst->IT->format, c, v, inst->immediate );
		break;
	}
}
