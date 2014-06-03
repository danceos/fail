/*
 * FAME - Fault Aware Micro-hypervisor Environment
 *
 * Author: Andreas Heinig <andreas.heinig@gmx.de>
 *
 * Copyright (C) 2011,2012 Department of Computer Science,
 * Design Automation of Embedded Systems Group
 * Dortmund University of Technology
 *
 * This program is proprietary software: you must not redistribute it.
 * Using this software is only allowed inside the DFG SPP1500 F.E.H.L.E.R project,
 * ls12-www.cs.tu-dortmund.de/daes/forschung/dependable-embedded-real-time-systems
 *
 * The complete license is depicted in the LICENSE file in the top level folder.
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
