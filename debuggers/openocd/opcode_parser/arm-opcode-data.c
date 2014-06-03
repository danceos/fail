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

#define UNDEF_INSTRUCTION_MASK			0x0FB00000
#define UNDEF_INSTRUCTION_SIG  			0x03000000

#define MISC_INSTRUCTION_1_MASK			0x0F900010
#define MISC_INSTRUCTION_1_SIG			0x01000000

#define MISC_INSTRUCTION_2_MASK			0x0F900090
#define MISC_INSTRUCTION_2_SIG			0x01000010

#define MSR_IMM_INSTRUCTION_MASK		0x0FB00000
#define MSR_IMM_INSTRUCTION_SIG			0x03200000

#define MUL_OR_EXTRA_LDSTR_INSTRUCTION_MASK	0x0E000090
#define MUL_OR_EXTRA_LDSTR_INSTRUCTION_SIG	0x00000090

#define OP_AND		0
#define OP_EOR		1
#define OP_SUB		2
#define OP_RSB		3
#define OP_ADD		4
#define OP_ADC		5
#define OP_SBC		6
#define OP_RSC		7

#define OP_TST		8
#define OP_TEQ		9
#define OP_CMP		10
#define OP_CMN		11
#define OP_ORR		12
#define OP_MOV		13
#define OP_BIC		14
#define OP_MVN		15

static const char * alu_op_str[16] = {
		"AND",
		"EOR",
		"SUB",
		"RSB",
		"ADD",
		"ADC",
		"SBC",
		"RSC",
		"TST",
		"TEQ",
		"CMP",
		"CMN",
		"ORR",
		"MOV",
		"BIC",
		"MVN"
};

int decode_data_processing(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	uint32_t rn = (inst >> 16) & 0xF;
	uint32_t rd = (inst >> 12) & 0xF;
	uint32_t rs = (inst >> 8)  & 0xF;
	uint32_t rm = (inst)       & 0xF;

	/*
	 * Undefined instruction ?
	 */

	if((inst & UNDEF_INSTRUCTION_MASK) == UNDEF_INSTRUCTION_SIG)
	{
		OP_PRINTF("Undefined Instruction\n")
		return 0;
	}

	/*
	 * Miscellaneous instructions ?
	 */

	if((inst & MISC_INSTRUCTION_1_MASK) == MISC_INSTRUCTION_1_SIG ||
			(inst & MISC_INSTRUCTION_2_MASK) == MISC_INSTRUCTION_2_SIG)
	{
		uint32_t op1 = (inst >> 21) & 3;
		uint32_t op2 = (inst >> 4) & 0xf;

		if(op2 == 0)
		{
			if(op1 & 1)
			{
				OP_PRINTF("MSR\t")
				if(op1 & 2)
					OP_PRINTF("SPSR, ")
				else
					OP_PRINTF("CPSR, ")
				OP_PRINTF("R%d\n", (int)rm)
				arm_op_add_reg_r(op, rm);
				return 0;
			}
			else
			{
				OP_PRINTF("MRS\tR%d, ", (int)rd)
				if(op1 & 2)
					OP_PRINTF("SPSR\n")
				else
					OP_PRINTF("CPSR\n")
				arm_op_add_reg_w(op, rd);
				return 0;
			}
		}
		if(op2 == 1)
		{
			if(op1 == 1)
			{
				OP_PRINTF("BX\tR%d\n", (int)rm)
				arm_op_add_reg_r(op, rm);
				return 0;
			}
			else if(op1 == 3)
			{
				OP_PRINTF("CLZ\tR%d, R%d\n", (int)rd, (int)rm)
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_w(op, rd);
				return 0;
			}
			else
			{
				printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
				return 1;
			}
		}
		if(op2 == 2)
		{
			if(op1 == 1)
			{
				OP_PRINTF("BXJ\tR%d\n", (int)rm)
				arm_op_add_reg_r(op, rm);
				return 0;
			}
			else
			{
				printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
				return 1;
			}
		}
		if(op2 == 3)
		{
			if(op1 == 1)
			{
				OP_PRINTF("BLX\tR%d\n", (int)rm)
				arm_op_add_reg_r(op, rm);
				return 0;
			}
			else
			{
				printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
				return 1;
			}
		}
		if(op2 == 5)
		{
			arm_op_add_reg_r(op, rm);
			arm_op_add_reg_r(op, rn);
			arm_op_add_reg_w(op, rd);

			switch(op1)
			{
			case 0:
				OP_PRINTF("QADD\tR%d, R%d, R%d\n", (int)rd, (int)rm, (int)rn)
				return 0;
			case 1:
				OP_PRINTF("QSUB\tR%d, R%d, R%d\n", (int)rd, (int)rm, (int)rn)
				return 0;
			case 2:
				OP_PRINTF("QDADD\tR%d, R%d, R%d\n", (int)rd, (int)rm, (int)rn)
				return 0;
			case 3:
			default:
				OP_PRINTF("QDSUB\tR%d, R%d, R%d\n", (int)rd, (int)rm, (int)rn)
				return 0;
			}
		}
		if(op2 == 7)
		{
			if(op1 == 1)
			{
				OP_PRINTF("BKPT\n")
				return 0;
			}
			else
			{
				printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
				return 1;
			}
		}
		if((op2 & 9) == 8)
		{
			char x = BIT_IS_SET(5) ? 'T' : 'B';
			char y = BIT_IS_SET(6) ? 'T' : 'B';

			arm_op_add_reg_w(op, rd);
			switch(op1)
			{
			case 0:
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rs);
				OP_PRINTF("SMLA%c%c\tR%d, R%d, R%d, R%d\n", x, y, (int)rd, (int)rn, (int)rm, (int)rs)
				return 0;
			case 1:
				if(x == 'B')
				{
					arm_op_add_reg_r(op, rm);
					arm_op_add_reg_r(op, rn);
					arm_op_add_reg_r(op, rs);
					OP_PRINTF("SMLAW%c\tR%d, R%d, R%d, R%d\n", y, (int)rd, (int)rm, (int)rs, (int)rn)
					return 0;
				}
				else
				{
					arm_op_add_reg_r(op, rm);
					arm_op_add_reg_r(op, rs);
					OP_PRINTF("SMULW%c\tR%d, R%d, R%d\n", y, (int)rd, (int)rm, (int)rs)
					return 0;
				}
			case 2:
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rs);
				OP_PRINTF("SMLAL%c%c\tR%d, R%d, R%d, R%d\n", x, y, (int)rd, (int)rn, (int)rm, (int)rs)
				return 0;
			case 3:
			default:
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_r(op, rs);
				OP_PRINTF("SMLAL%c%c\tR%d, R%d, R%d\n", x, y, (int)rd, (int)rm, (int)rs)
				return 0;
			}
		}

		printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
		return 1;
	}

	/*
	 * MSR with immediate ?
	 */

	if((inst & MSR_IMM_INSTRUCTION_MASK) == MSR_IMM_INSTRUCTION_SIG)
	{
		OP_PRINTF("MSR\t")
		if(BIT_IS_SET(22))
			OP_PRINTF("SPSR, immediate\n")
		else
			OP_PRINTF("CPSR, immediate\n")
		return 0;
	}

	/*
	 * Multiplies or extra stores ?
	 */
	if((inst & MUL_OR_EXTRA_LDSTR_INSTRUCTION_MASK) == MUL_OR_EXTRA_LDSTR_INSTRUCTION_SIG)
	{
		uint32_t op1 = (inst >> 20) & 0x1F;
		uint32_t op2 = (inst >> 5) & 3;

		if(op2 == 0)
		{
			/* Multiply instructions */
			if((op1 & 0x1C) == 0)
			{
				arm_op_add_reg_w(op, rd);
				if(op1 & 2)
				{
					OP_PRINTF("MLA")
					if(op1 & 1)
						OP_PRINTF("S")
					OP_PRINTF("\tR%d, R%d, R%d, R%d\n", (int)rd, (int)rm, (int)rs, (int)rn)
					arm_op_add_reg_w(op, rd);
					arm_op_add_reg_r(op, rm);
					arm_op_add_reg_r(op, rn);
					arm_op_add_reg_r(op, rs);
					return 0;
				}
				else
				{
					OP_PRINTF("MUL")
					if(op1 & 1)
						OP_PRINTF("S")
					OP_PRINTF("\tR%d, R%d, R%d\n", (int)rd, (int)rm, (int)rs)
					arm_op_add_reg_w(op, rd);
					arm_op_add_reg_r(op, rm);
					arm_op_add_reg_r(op, rs);
					return 0;
				}
			}
			if((op1 & 0x1F) == 4)
			{
				OP_PRINTF("UMAAL\tR%d, R%d, R%d, R%d\n", (int)rd, (int)rn, (int)rm, (int)rs)
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rs);
				return 0;

			}
			if((op1 & 0x18) == 8)
			{
				if(op1 & 4)
					OP_PRINTF("S")
				else
					OP_PRINTF("U")
				if(op1 & 2)
					OP_PRINTF("MLAL")
				else
					OP_PRINTF("MULL")
				if(op1 & 1)
					OP_PRINTF("S")

				OP_PRINTF("\tR%d, R%d, R%d, R%d\n", (int)rd, (int)rn, (int)rm, (int)rs)
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rs);
				return 0;
			}

			/* Load store */
			if((op1 & 0x1B) == 0x10)
			{
				OP_PRINTF("SWP")
				if(op1 & 4)
				{
					OP_PRINTF("B")
					op->mem_size = 1;
				}
				else
					op->mem_size = 4;
				op->mem_addr = op->regs->r[rn];
				op->flags    = OP_FLAG_READ | OP_FLAG_WRITE;
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				arm_op_add_reg_r(op, rn);
				OP_PRINTF("\tR%d, R%d, R%d [Addr: %08x]\n", (int)rd, (int)rm, (int)rn, (unsigned int)op->mem_addr)
				return 0;
			}
			if((op1 & 0x1E) == 0x18)
			{
				op->mem_size = 4;
				op->mem_addr = op->regs->r[rn];
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rn);
				if(op1 & 1)
				{
					OP_PRINTF("LDREX")
					op->flags = OP_FLAG_READ;
				}
				else
				{
					OP_PRINTF("STREX")
					op->flags = OP_FLAG_WRITE;
				}
				OP_PRINTF("\tR%d, R%d [Addr: %08x]\n", (int)rd, (int)rn, (unsigned int)op->mem_addr)
				return 0;
			}
			printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
			return 1;
		}
		if(op2 == 1)
		{
			if(op1 & 1)
			{
				OP_PRINTF("LDRH\tR%d, ", (int)rd)
				op->flags = OP_FLAG_READ;
			}
			else
			{
				OP_PRINTF("STRH\tR%d, ", (int)rd)
				op->flags = OP_FLAG_WRITE;
			}
			op->mem_size = 2;
			arm_op_add_reg_w(op, rd);
			decode_addressing_mode3(op);
			OP_PRINTF("\n")
			return 0;

		}

		if(op2 & 2)
		{
			if(op1 & 1)
			{
				if(op2 & 1)
				{
					op->mem_size = 2;
					OP_PRINTF("LDRSH\tR%d, ", (int)rd)
				}
				else
				{
					op->mem_size = 1;
					OP_PRINTF("LDRSB\tR%d, ", (int)rd)
				}
				op->flags  = OP_FLAG_READ;
				arm_op_add_reg_w(op, rd);
				decode_addressing_mode3(op);
				OP_PRINTF("\n")
				return 0;
			}
			else
			{
				if(op2 & 1)
				{
					op->mem_size = 8;
					op->flags    = OP_FLAG_WRITE;
					OP_PRINTF("STRD\tR%d:%d, ",(int) rd, (int)rd + 1)
				}
				else
				{
					op->mem_size = 8;
					op->flags    = OP_FLAG_READ;
					OP_PRINTF("LDRD\tR%d:%d, ", (int)rd, (int)rd + 1)
				}
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_w(op, rd + 1);
				decode_addressing_mode3(op);
				OP_PRINTF("\n")
				return 0;
			}
		}
		printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
		return 1;
	}

	/*
	 * The other possible instructions are ALU ?
	 */

	/* ALU opcode */
	uint32_t opcode = (inst >> 21) & 0xF;


	OP_PRINTF("%s", alu_op_str[opcode])
	if(ALU_S_SET)
		OP_PRINTF("S")

	/* possibly normal ALU instructions */
	if(opcode == OP_MOV || opcode == OP_MVN)
	{
		OP_PRINTF("\tR%d, ", (int)rd)
		arm_op_add_reg_w(op, rd);
	}
	else if(opcode == OP_CMP || opcode == OP_CMN || opcode == OP_TST || opcode == OP_TEQ)
	{
		OP_PRINTF("\tR%d, ", (int)rn)
		arm_op_add_reg_r(op, rn);
	}
	else
	{
		OP_PRINTF("\tR%d, R%d, ", (int)rd, (int)rn)
		arm_op_add_reg_w(op, rd);
		arm_op_add_reg_r(op, rn);
	}

	decode_addressing_mode1(op);
	OP_PRINTF("\n")
	return 0;
}
