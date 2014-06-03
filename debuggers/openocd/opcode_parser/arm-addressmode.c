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

/**
 * @brief Parse addressing mode 1 - Data-processing operands
 */
uint32_t decode_addressing_mode1(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	// uint32_t rn = (inst >> 16) & 0xF;
	// uint32_t rd = (inst >> 12) & 0xF;
	uint32_t rs = (inst >> 8)  & 0xF;
	uint32_t rm = (inst)       & 0xF;

	if(I_SET)
	{
		/* Immediate */
		uint32_t immed_8 = inst & 0xFF;
		uint32_t rotate_imm = ((inst & 0xF00) >> 8) * 2;
		uint32_t immediate = (immed_8 >> rotate_imm) | (immed_8 << (32 - rotate_imm));
		OP_PRINTF("#%d", (int)immediate)
		return immediate;
	}

	if((inst & 0x0E000FF0) == 0x00000000)
	{
		/* Register */
		arm_op_add_reg_r(op, rm);
		OP_PRINTF("R%d", (int)rm)
		return op->regs->r[rm];
	}
	if((inst & 0x0E000070) == 0x00000000)
	{
		/* Logical shift left by immediate */
		arm_op_add_reg_r(op, rm);
		uint32_t shift_imm = (inst >> 7) & 0x1F;
		uint32_t shifter_operand = op->regs->r[rm];
		if(rm == 15)
			shifter_operand += 8;
		shifter_operand <<= shift_imm;
		OP_PRINTF("R%d, LSL #%d [Operand2: %d]", (int)rm, (int) shift_imm, (int)shifter_operand)
		return shifter_operand;
	}
	if((inst & 0x0E0000F0) == 0x00000010)
	{
		/* Logical shift left by register */
		arm_op_add_reg_r(op, rm);
		arm_op_add_reg_r(op, rs);
		uint32_t shift_reg = op->regs->r[rs];
		uint32_t shifter_operand = op->regs->r[rm];

		if(shift_reg != 0)
		{
			if(shift_reg < 32)
			{
				shifter_operand <<= (shift_reg & 0xFF);
			}
			else
			{
				shifter_operand = 0;
			}
		}

		OP_PRINTF("R%d, LSL R%d [Operand2: %d]", (int)rm, (int)rs, (int)shifter_operand)
		return shifter_operand;
	}
	if((inst & 0x0E000070) == 0x00000020)
	{
		/* Logical shift right by immediate */
		arm_op_add_reg_r(op, rm);
		uint32_t shift_imm = (inst >> 7) & 0x1F;
		uint32_t shifter_operand = op->regs->r[rm];
		if(rm == 15)
			shifter_operand += 8;
		if(shift_imm == 0)
		{
			shifter_operand = 0;
			shift_imm = 32;
		}
		else
			shifter_operand >>= shift_imm;
		OP_PRINTF("R%d, LSR #%d [Operand2: %d]", (int)rm, (int) shift_imm, (int)shifter_operand)
		return shifter_operand;
	}
	if((inst & 0x0E0000F0) == 0x00000030)
	{
		/* Logical shift right by register */
		arm_op_add_reg_r(op, rm);
		arm_op_add_reg_r(op, rs);
		uint32_t shift_reg = op->regs->r[rs];
		uint32_t shifter_operand = op->regs->r[rm];

		if(shift_reg != 0)
		{
			if(shift_reg < 32)
			{
				shifter_operand >>= (shift_reg & 0xFF);
			}
			else
			{
				shifter_operand = 0;
			}
		}

		OP_PRINTF("R%d, LSL R%d [Operand2: %d]", (int)rm, (int)rs, (int)shifter_operand)
		return shifter_operand;
	}
	if((inst & 0x0E000070) == 0x00000040)
	{
		/* Arithmetic shift right by immediate */
		arm_op_add_reg_r(op, rm);
		uint32_t shift_imm = (inst >> 7) & 0x1F;
		uint32_t shifter_operand = op->regs->r[rm];
		if(rm == 15)
			shifter_operand += 8;
		if(shift_imm == 0)
		{
			if(shifter_operand & 0x80000000)
				shifter_operand = 0xFFFFFFFF;
			else
				shifter_operand = 0;
			shift_imm = 32;
		}
		else
			shifter_operand = (uint32_t)((int32_t)shifter_operand >> (int32_t)shift_imm);
		OP_PRINTF("R%d, ASR #%d [Operand2: %d]", (int)rm, (int) shift_imm, (int)shifter_operand)
		return shifter_operand;
	}
	if((inst & 0x0E0000F0) == 0x00000050)
	{
		/* Arithmetic shift right by register */
		arm_op_add_reg_r(op, rm);
		arm_op_add_reg_r(op, rs);
		uint32_t shift_reg = op->regs->r[rs];
		uint32_t shifter_operand = op->regs->r[rm];

		if(shift_reg != 0)
		{
			if(shift_reg < 32)
			{
				shifter_operand = (uint32_t)((int32_t)shifter_operand >> (int32_t)(shift_reg & 0xFF));
			}
			else
			{
				if(shifter_operand & 0x80000000)
					shifter_operand = 0xFFFFFFFF;
				else
					shifter_operand = 0;
			}
		}

		OP_PRINTF("R%d, ASR R%d [Operand2: %d]", (int)rm, (int)rs, (int)shifter_operand)
		return shifter_operand;
	}
	if((inst & 0x0E000070) == 0x00000060)
	{
		/* Rotate right by immediate */
		arm_op_add_reg_r(op, rm);
		uint32_t shift_imm = (inst >> 7) & 0x1F;
		uint32_t shifter_operand = op->regs->r[rm];
		if(rm == 15)
			shifter_operand += 8;
		if(shift_imm == 0)
		{
			shifter_operand >>= 1;
			if((op->regs->cpsr) & (1 << 29))
				shifter_operand |= 0x80000000;
			shift_imm = 32;
			OP_PRINTF("R%d, RRX #%d [Operand2: %d]", (int)rm, (int) shift_imm, (int)shifter_operand)
		}
		else
		{
			shifter_operand >>= shift_imm;
			OP_PRINTF("R%d, ROR #%d [Operand2: %d]", (int)rm, (int) shift_imm, (int)shifter_operand)
		}
		return shifter_operand;
	}
	if((inst & 0x0E0000F0) == 0x00000070)
	{
		/* Rotate right by register */
		arm_op_add_reg_r(op, rm);
		arm_op_add_reg_r(op, rs);
		uint32_t shift_reg = op->regs->r[rs];
		uint32_t shifter_operand = op->regs->r[rm];

		if(shift_reg != 0)
		{
			if(shift_reg < 32)
			{
				shifter_operand = (shifter_operand >> shift_reg) | (shifter_operand << (32 - shift_reg));
			}
			else
			{
				shifter_operand = 0;
			}
		}

		OP_PRINTF("R%d, ROR R%d [Operand2: %d]", (int)rm, (int)rs, (int)shifter_operand)
		return shifter_operand;
	}
	printf("Cannot determine Operand2!");
	return 0;
}

/**
 * @brief Parse addressing mode 2 - Load and Store Word or Unsigned Byte
 */
void decode_addressing_mode2(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	uint32_t rn = (inst >> 16) & 0xF;
	// uint32_t rd = (inst >> 12) & 0xF;
	uint32_t rm = (inst)       & 0xF;

	arm_op_add_reg_r(op, rn);

	char U = U_SET ? '+' : '-';

	uint32_t index;

	if(!I_SET)
	{
		index = inst & 0xFFF;
		if(!U_SET)
			index *= -1;

		if(!P_SET)
		{
			/* post-indexed addressing */
			op->mem_addr = op->regs->r[rn];
			OP_PRINTF("[R%d], #%d", (int)rn, (int)index)
		}
		else if(W_SET)
		{
			/* pre-indexed addressing */
			op->mem_addr = op->regs->r[rn] + index;
			OP_PRINTF("[R%d, #%d]!", (int)rn, (int)index)

		}
		else
		{
			/* offset addressing */
			op->mem_addr = op->regs->r[rn] + index;
			OP_PRINTF("[R%d, #%d]", (int)rn, (int)index)
		}
		OP_PRINTF(" [Address: %08x]", (unsigned int)op->mem_addr)
		return;
	}
	else
	{
		arm_op_add_reg_r(op, rm);

		if((inst & 0x00000FF0) == 0)
		{
			index = op->regs->r[rm];
			if(!U_SET)
				index *= -1;
			if(!P_SET)
			{
				/* post-indexed addressing */
				op->mem_addr = op->regs->r[rn];
				OP_PRINTF("[R%d], #%cR%d", (int)rn, U, (int)rm)
			}
			else if(W_SET)
			{
				/* pre-indexed addressing */
				op->mem_addr = op->regs->r[rn] + index;
				OP_PRINTF("[R%d, #%cR%d]!", (int)rn, U, (int)rm)

			}
			else
			{
				/* offset addressing */
				op->mem_addr = op->regs->r[rn] + index;
				OP_PRINTF("[R%d, #%cR%d]", (int)rn, U, (int)rm)
			}
			if(rn == 15)
				op->mem_addr += 8;
			OP_PRINTF(" [Address: %08x]", (unsigned int)op->mem_addr)
			return;
		}
		else
		{
			uint32_t shift  = (inst >> 7) & 0x1F;
			uint32_t type   = (inst >> 5) & 0x03;
			uint32_t rm_val = op->regs->r[rm];

			if(!P_SET)
			{
				/* post-indexed addressing */
				OP_PRINTF("[R%d], #%cR%d, ", (int)rn, U, (int)rm)
			}
			else
			{
				/* pre-indexed addressing, or
				 * offset addressing */
				OP_PRINTF("[R%d, #%cR%d, ", (int)rn, U, (int)rm)

			}

			switch(type)
			{
			case 0: /* LSL */
				index = rm_val << shift;
				OP_PRINTF("LSL #%d", (int)shift)
				break;
			case 1: /* LSR */
				if(shift == 0)
					index = 0;
				else
					index = rm_val >> shift;
				OP_PRINTF("LSR #%d", (int)shift)
				break;
			case 2: /* ASR */
				if(shift == 0)
				{
					if(rm_val & 0x80000000)
						index = 0xFFFFFFFF;
					else
						index = 0;
				}
				else
					index = (uint32_t)(((int32_t)rm_val) >> (int32_t)shift);
				OP_PRINTF("ASR #%d", (int)shift)
				break;
			case 3: /* ROR */
			default:

				if(shift == 0) /* RRX */
				{
					index = ((op->regs->cpsr & (1 << 29)) << 31) | (rm_val > 1);
					OP_PRINTF("RRX")
				}
				else
				{
					index = (rm_val >> shift) | (rm_val << (32 - shift));
					OP_PRINTF("ROR #%d", (int)shift)
				}
				break;
			}

			if(!U_SET)
				index *= -1;

			if(!P_SET)
			{
				/* post-indexed addressing */
				op->mem_addr = op->regs->r[rn];
			}
			else if(W_SET)
			{
				/* pre-indexed addressing */
				op->mem_addr = op->regs->r[rn] + index;
				OP_PRINTF("]!")
				arm_op_add_reg_w(op, rn);
			}
			else
			{
				/* offset addressing */
				op->mem_addr = op->regs->r[rn] + index;
				OP_PRINTF("]")
			}
			if(rn == 15)
				op->mem_addr += 8;
			OP_PRINTF(" [Address: %08x]", (unsigned int)op->mem_addr)
			return;
		}
	}

	printf("Cannot determine Operand2!");
	return;
}

/**
 * @brief Parse addressing mode 3 - Miscellaneous Loads and Stores
 */
void decode_addressing_mode3(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	uint32_t rn = (inst >> 16) & 0xF;
	uint32_t rm = (inst)       & 0xF;

	arm_op_add_reg_r(op, rn);
	if(W_SET)
		arm_op_add_reg_w(op, rm);
	OP_PRINTF("[R%d", (int)rn)

	uint32_t index;
	if(BIT_IS_SET(22))
	{
		index = ((inst & 0xF00) >> 4) | (inst & 0xF);
	}
	else
	{
		arm_op_add_reg_r(op, rm);
		index = op->regs->r[rm];
	}
	char U;
	if(U_SET)
	{
		U = '+';
	}
	else
	{
		U = '-';
		index *= -1;
	}

	if(!P_SET)
	{
		/* post-indexed addressing */
		op->mem_addr = op->regs->r[rn];
		if(BIT_IS_SET(22))
		{
			OP_PRINTF("[R%d], #%d", (int)rn, (int)index)
		}
		else
		{
			OP_PRINTF("[R%d], %cR%d", (int)rn, U, (int)rm)
		}
	}
	else if(W_SET)
	{
		/* pre-indexed addressing */
		op->mem_addr = op->regs->r[rn] + index;
		if(BIT_IS_SET(22))
		{
			OP_PRINTF("[R%d, #%d]!", (int)rn, (int)index)
		}
		else
		{
			OP_PRINTF("[R%d, %cR%d]!", (int)rn, U, (int)rm)
		}
	}
	else
	{
		/* offset addressing */
		op->mem_addr = op->regs->r[rn] + index;
		if(BIT_IS_SET(22))
		{
			OP_PRINTF("[R%d, #%d]", (int)rn, (int)index)
		}
		else
		{
			OP_PRINTF("[R%d, %cR%d]", (int)rn, U, (int)rm)
		}
	}
	OP_PRINTF(" [Address: %08x]", (unsigned int)op->mem_addr)
}

/**
 * @brief Parse addressing mode 5 - Load and Store Coprocessor
 */
void decode_addressing_mode5(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	uint32_t rn = (inst >> 16) & 0xF;
	uint32_t offset_8 = (inst & 0xF) * 4;

	if(!U_SET)
		offset_8 *= -1;

	arm_op_add_reg_r(op, rn);
	if(W_SET)
		arm_op_add_reg_w(op, rn);

	OP_PRINTF("[R%d", (int)rn)

	if(P_SET && BIT_IS_CLEAR(21))
	{
		/* Immediate offset */
		OP_PRINTF(", #%d]", (int)offset_8)
	}
	else if(P_SET && W_SET)
	{
		/* Immediate pre-indexed */
		OP_PRINTF(", #%d]!",(int) offset_8)
	}
	else if(BIT_IS_CLEAR(24) && W_SET)
	{
		/* Immediate post-indexed */
		OP_PRINTF("] , #%d", (int)offset_8)
	}
	else
	{
		/* Unindexed */
		OP_PRINTF("] , %d", (int)(inst & 0xF))
	}
}
