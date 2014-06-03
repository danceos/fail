/*
 * ARM JTAG Fault Injector
 *
 * Author: Andreas Heinig <andreas.heinig@gmx.de>
 *
 * Copyright (C) 2011-2014 Department of Computer Science,
 * Design Automation of Embedded Systems Group,
 * Dortmund University of Technology, all rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include "arm-opcode.h"

int verbose_opcode = 0;
char opcode_string[128];
char * opcode_string_tmp;

int decode_instruction(uint32_t inst, arm_regs_t * regs, arm_instruction_t * op)
{
	memset(op, 0, sizeof(arm_instruction_t));
	op->inst = inst;
	op->regs = regs;

	opcode_string_tmp = opcode_string;

	/* will the instruction be executed ?*/
	uint32_t condtest = op_cond_test(inst, regs->cpsr);
	if(condtest == 0xFFFFFFFF)
	{
		//TODO: parse extension opcodes
		printf("TODO: parse extension opcodes");
		return 1;
	}
	else if(condtest == 0)
	{
		op->flags = OP_FLAG_CONDITION_FALSE;
		return 2;
	}

	uint32_t type = (inst >> 26) & 3;

	switch(type)
	{
	case 0:
		return decode_data_processing(op);
	case 1:
		return decode_load_store(op);
	case 2:
		return decode_load_store_multiple_and_branch(op);
	case 3:
		return decode_coprocessor(op);
	}
	return 1;
}

void dump_instruction(const arm_instruction_t * op)
{
	printf("R0: %08x R1: %08x R2: %08x R3: %08x IP: %08x SP: %08x\n",
			(unsigned int)op->regs->r[0],
			(unsigned int)op->regs->r[1],
			(unsigned int)op->regs->r[2],
			(unsigned int)op->regs->r[3],
			(unsigned int)op->regs->r[12],
			(unsigned int)op->regs->r[13]
	);
	printf("Instruction: %08x PC: %08x LR: %08x CPSR:%08x SPSR:%08x Mem:%08x(%c%c,%u) Flags:%08x\n\tRegisters: r:0x%04x r_fiq:0x%04x w:0x%04x w_fiq:0x%04x -- ",
			(unsigned int)op->inst,
			(unsigned int)op->regs->r[15],
			(unsigned int)op->regs->r[14],
			(unsigned int)op->regs->cpsr,
			(unsigned int)op->regs->spsr,
			(unsigned int)op->mem_addr,
			op->flags & OP_FLAG_READ ? 'R' : ' ',
			op->flags & OP_FLAG_WRITE ? 'W' : ' ',
			(unsigned int)op->mem_size,
			(unsigned int)op->flags,
			(unsigned int)op->regs_r,
			(unsigned int)op->regs_r_fiq,
			(unsigned int)op->regs_w,
			(unsigned int)op->regs_w_fiq
	);
	int i;
	for(i = 0; i < 16; i++)
	{
		if((op->regs_r & (1 << i)) ||
				(op->regs_r_fiq & (1 << i)) ||
				(op->regs_w     & (1 << i)) ||
				(op->regs_w_fiq & (1 << i)))
		{
			printf("R%d(", i);
			if(op->regs_r & (1 << i))
				printf("R");
			if(op->regs_r_fiq & (1 << i))
				printf("r");
			if(op->regs_w & (1 << i))
				printf("W");
			if(op->regs_w_fiq & (1 << i))
				printf("w");

			printf("), ");
		}
	}
	printf("\n");
}
