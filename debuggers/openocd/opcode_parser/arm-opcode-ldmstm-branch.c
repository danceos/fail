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
#include "arm-opcode.h"

int decode_load_store_multiple_and_branch(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/*
	 * Branch ?
	 */

	if(BIT_IS_SET(25))
	{
		uint32_t target = (inst & 0x00FFFFFF) << 8;
		target = ((int32_t)target) >> 6;

		target += op->regs->r[15] + 8;
		if(BIT_IS_SET(24))
		{
			OP_PRINTF("BL\t#0x%08x\n", (unsigned int)target)
			arm_op_add_reg_r(op, 14);
			arm_op_add_reg_w(op, 15);
			return 0;
		}
		else
		{
			OP_PRINTF("B\t#0x%08x\n", (unsigned int)target)
			arm_op_add_reg_w(op, 15);
			return 0;
		}
	}

	/* LDM/STM */

	uint32_t rn = (inst >> 16) & 0xF;

	op->mem_addr = op->regs->r[rn];
	arm_op_add_reg_r(op, rn);

	if(W_SET)
		arm_op_add_reg_w(op, rn);

	if(_L_SET)
	{
		op->flags   = OP_FLAG_READ;
		op->regs_w |= (inst & 0x80FF);
		if(IS_OP_FIQ(op) && !(BIT_IS_SET(22)))
			op->regs_w_fiq |= (inst & 0x7F00);
		else
			op->regs_w |= (inst & 0x7F00);
		OP_PRINTF("LDM")
	}
	else
	{
		op->flags   = OP_FLAG_WRITE;
		op->regs_r |= (inst & 0x80FF);
		if(IS_OP_FIQ(op) && !(S_SET))
			op->regs_r_fiq |= (inst & 0x7F00);
		else
			op->regs_r |= (inst & 0x7F00);
		OP_PRINTF("STM")
	}

	/* determine memory range size */
	unsigned int i;
	for(i = 0; i < 16; i++)
	{
		if(inst & (1 << i))
			op->mem_size += 4;
	}

	if(!P_SET)
	{
		/* Word addressed by Rn is _included_ in range of memory */
		if(!U_SET)
		{
			/* Downward addressing */
			op->mem_addr -= op->mem_size;
			OP_PRINTF("DA")
		}
		else
		{
			/* Upward addressing */
			// nothing to adjust in this case
			OP_PRINTF("IA")
		}
	}
	else
	{
		/* Word addressed by Rn is _excluded_ in range of memory */
		if(!U_SET)
		{
			/* Downward addressing */
			op->mem_addr -= op->mem_size;
			op->mem_addr -= 4;
			OP_PRINTF("DB")
		}
		else
		{
			/* Upward addressing */
			op->mem_addr += 4;
			OP_PRINTF("IB")
		}
	}
	OP_PRINTF("\tR%d", (int)rn)
	if(W_SET)
		OP_PRINTF("!")
	OP_PRINTF(", {")
	for(i = 0; i < 16; i++)
	{
		if(inst & (1 << i))
			OP_PRINTF("R%d, ", i)
	}
	OP_PRINTF("}")
	if(S_SET)
		OP_PRINTF("^")

	OP_PRINTF(" [Address: %08x]", (unsigned int)op->mem_addr)
	return 0;
}
