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
