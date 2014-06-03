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

#define ARCH_UNDEF_INSTRUCTION_MASK		0x0FF000F0
#define ARCH_UNDEF_INSTRUCTION_SIG  		0x07F000F0

#define MEDIA_INSTRUCTION_MASK			0x0E000010
#define MEDIA_INSTRUCTION_SIG  			0x06000010

int decode_load_store(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	uint32_t rn = (inst >> 16) & 0xF;
	uint32_t rd = (inst >> 12) & 0xF;
	uint32_t rs = (inst >> 8)  & 0xF;
	uint32_t rm = (inst)       & 0xF;

	/*
	 * Architecturally undefined instruction ?
	 */

	if((inst & ARCH_UNDEF_INSTRUCTION_MASK) == ARCH_UNDEF_INSTRUCTION_SIG)
	{
		OP_PRINTF("Architecturally Undefined Instruction\n")
		return 0;
	}

	/*
	 * Media instruction ?
	 */

	if((inst & MEDIA_INSTRUCTION_MASK) == MEDIA_INSTRUCTION_SIG)
	{
		uint32_t opc  = (inst >> 23) & 3;
		uint32_t opc1 = (inst >> 20) & 7;
		uint32_t opc2 = (inst >>  5) & 7;

		if(opc == 0)
		{
			/* Parallel add/subtract */
			switch(opc1)
			{
			case 1:
				OP_PRINTF("S")
				break;
			case 2:
				OP_PRINTF("Q")
				break;
			case 3:
				OP_PRINTF("SH")
				break;
			case 5:
				OP_PRINTF("U")
				break;
			case 6:
				OP_PRINTF("UQ")
				break;
			case 7:
				OP_PRINTF("UH")
				break;
			case 0:
			case 4:
			default:
				printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
				return 1;
			}
			switch(opc2 & 3)
			{
			case 0:
				OP_PRINTF("ADD")
				if(opc2 & 4)
					OP_PRINTF("8")
				else
					OP_PRINTF("16")
				break;
			case 1:
				OP_PRINTF("ADDSUBX")
				break;
			case 2:
				OP_PRINTF("SUBADDX")
				break;
			case 3:
			default:
				OP_PRINTF("SUB")
				if(opc2 & 4)
					OP_PRINTF("8")
				else
					OP_PRINTF("16")
				break;
			}
			OP_PRINTF("\tR%d, R%d, R%d\n", (int)rd, (int)rn, (int)rm)
			arm_op_add_reg_w(op, rd);
			arm_op_add_reg_r(op, rn);
			arm_op_add_reg_r(op, rm);
			return 0;
		}
		if(opc == 1)
		{
			uint32_t shift_imm = (inst >> 7) & 0x1F;
			if(opc1 == 0)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rm);
				if(opc2 & 2)
				{
					if(shift_imm == 0)
						shift_imm = 32;
					OP_PRINTF("PKHTB\tR%d, R%d, R%d, ASR #%d\n", (int)rd, (int)rn, (int)rm, (int)shift_imm)
					return 0;
				}
				else
				{
					OP_PRINTF("PKHBT\tR%d, R%d, R%d", (int)rd, (int)rn, (int)rm)
					if(shift_imm)
						OP_PRINTF(", LSL #%d\n", (int)shift_imm)
					else
						OP_PRINTF("\n")
					return 0;
				}
			}
			if((opc1 & 2) == 2 && (opc2 & 1) == 0)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				uint32_t sat_imm   = (inst >> 16) & 0x1F;
				uint32_t shift_imm = (inst >>  7) & 0x1F;

				if(opc1 & 4)
					OP_PRINTF("U")
				else
					OP_PRINTF("S")
				OP_PRINTF("SAT\tR%d, #%d, R%d", (int)rd, (int)sat_imm + 1, (int)rm)
				if(BIT_IS_SET(6))
				{
					if(shift_imm == 0)
						OP_PRINTF(", ASR 32\n")
					else
						OP_PRINTF(", ASR #%d\n", (int)shift_imm)
				}
				else
				{
					if(shift_imm == 0)
					{
						OP_PRINTF("\n")
					}
					else
					{
						OP_PRINTF(", LSL #%d\n", (int)shift_imm)
					}
				}
				return 0;
			}
			if((opc1 & 3) == 2 && opc2 == 1)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				uint32_t sat_imm   = (inst >> 16) & 0xF;
				if(opc1 & 4)
					OP_PRINTF("U")
				else
					OP_PRINTF("S")
				OP_PRINTF("SAT16\tR%d, #%d, R%d\n", (int)rd, (int)sat_imm, (int)rm)
				return 0;
			}
			if(opc1 == 3 && opc2 == 1)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("REV\tR%d, R%d\n", (int)rd, (int)rm)
				return 0;
			}
			if(opc1 == 3 && opc2 == 5)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("REV16\tR%d, R%d\n", (int)rd, (int)rm)
				return 0;
			}
			if(opc1 == 7 && opc2 == 5)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("REVSH\tR%d, R%d\n", (int)rd, (int)rm)
				return 0;
			}
			if(opc1 == 0 && opc2 == 5)
			{
				arm_op_add_reg_w(op, rd);
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("SEL\tR%d, R%d, R%d\n", (int)rd, (int)rn, (int)rm)
				return 0;
			}
			if(opc2 == 3)
			{
				uint32_t rotate = (inst >> 10) & 3;
				if(opc1 & 4)
					OP_PRINTF("U")
				else
					OP_PRINTF("S")
				if(rn != 15)
				{
					arm_op_add_reg_w(op, rd);
					arm_op_add_reg_r(op, rn);
					arm_op_add_reg_r(op, rm);
					switch(opc1 & 3)
					{
					case 0:
						OP_PRINTF("XTAB16")
						break;
					case 2:
						OP_PRINTF("XTAB")
						break;
					case 3:
						OP_PRINTF("XTAH")
						break;
					case 1:
					default:
						printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
						return 1;
					}
					OP_PRINTF("\tR%d, R%d, R%d", (int)rd, (int)rn, (int)rm)
				}
				else
				{
					arm_op_add_reg_w(op, rd);
					arm_op_add_reg_r(op, rm);
					switch(opc1 & 3)
					{
					case 0:
						OP_PRINTF("XTB16")
						break;
					case 2:
						OP_PRINTF("XTB")
						break;
					case 3:
						OP_PRINTF("XTH")
						break;
					case 1:
					default:
						printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
						return 1;
					}
					OP_PRINTF("\tR%d, R%d", (int)rd, (int)rm)
				}
				if(rotate == 0)
					OP_PRINTF("\n")
				else if(rotate == 1)
					OP_PRINTF(", ROR #8\n")
				else if(rotate == 2)
					OP_PRINTF(", ROR #16\n")
				else if(rotate == 3)
					OP_PRINTF(", ROR #24\n")
				return 0;
			}
			printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
			return 1;
		}
		if(opc == 2)
		{
			/* RD and RN are switched!! */
			uint32_t temp = rd;
			rd = rn;
			rn = temp;

			arm_op_add_reg_w(op, rd);
			if(rn != 15)
			{
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rs);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("SML")
			}
			else
			{
				arm_op_add_reg_r(op, rs);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("SMU")
			}
			if(opc2 & 2)
				OP_PRINTF("S")
			else
				OP_PRINTF("A")
			if(opc1 & 4)
				OP_PRINTF("L")
			if(opc2 & 1)
				OP_PRINTF("X")
			if(rn != 15)
				OP_PRINTF("\tR%d, R%d, R%d, R%d", (int)rd, (int)rm, (int)rs, (int)rn)
			else
				OP_PRINTF("\tR%d, R%d, R%d", (int)rd, (int)rm, (int)rs)
			return 0;
		}
		if(opc == 3 && opc1 == 0 && opc2 == 0)
		{
			/* RD and RN are switched!! */
			uint32_t temp = rd;
			rd = rn;
			rn = temp;

			arm_op_add_reg_w(op, rd);
			if(rn != 15)
			{
				arm_op_add_reg_r(op, rn);
				arm_op_add_reg_r(op, rs);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("USADA8\tR%d, R%d, R%d, R%d", (int)rd, (int)rm, (int)rs, (int)rn)
			}
			else
			{
				arm_op_add_reg_r(op, rs);
				arm_op_add_reg_r(op, rm);
				OP_PRINTF("USAD8\tR%d, R%d, R%d", (int)rd, (int)rm, (int)rs)
			}
			return 0;
		}
		printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
		return 1;
	}

	/*
	 * ordinary load-store instruction
	 */

	if(_L_SET)
	{
		op->flags |= OP_FLAG_READ;
		arm_op_add_reg_w(op, rd);
		OP_PRINTF("LDR")
	}
	else
	{
		op->flags |= OP_FLAG_WRITE;
		arm_op_add_reg_r(op, rd);
		OP_PRINTF("STR")
	}

	if(B_SET)
	{
		op->mem_size = 1;
		OP_PRINTF("B")
	}
	else
	{
		op->mem_size = 4;
	}

	if(!P_SET && W_SET)
		OP_PRINTF("T")

	OP_PRINTF("\tR%d, ", (int)rd)
	decode_addressing_mode2(op);
	OP_PRINTF("\n")
	return 0;
}
