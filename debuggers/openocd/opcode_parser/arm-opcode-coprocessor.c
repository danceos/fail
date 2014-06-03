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

#define SVC_INSTRUCTION_MASK			0x0F000000
#define SVC_INSTRUCTION_SIG  			0x0F000000

#define BKPT_INSTRUCTION_MASK			0x0FF000F0
#define BKPT_INSTRUCTION_SIG  			0x01200070

#define MRR_INSTRUCTIONS_MASK			0x0FE00000
#define MRR_INSTRUCTIONS_SIG			0x0C400000

#define MR_INSTRUCTIONS_MASK			0x0F000010
#define MR_INSTRUCTIONS_SIG			0x0E000010

#define CDP_INSTRUCTION_MASK			0x0F000010
#define CDP_INSTRUCTION_SIG			0x0E000000

int decode_coprocessor(arm_instruction_t * op)
{
	uint32_t inst = op->inst;
	/* possible registers */
	uint32_t rn     = (inst >> 16) & 0xF;
	uint32_t rd     = (inst >> 12) & 0xF;
	uint32_t crm    = (inst)       & 0xF;
	uint32_t cp_num = (inst >> 8)  & 0xF;

	/*
	 * Software interrupt ?
	 */

	if((inst & SVC_INSTRUCTION_MASK) == SVC_INSTRUCTION_SIG)
	{
		OP_PRINTF("SVC\t%d\n", (int)(inst & 0x00FFFFFF))
		return 0;
	}

	/*
	 * Break point ?
	 */

	if((inst & BKPT_INSTRUCTION_MASK) == BKPT_INSTRUCTION_SIG)
	{
		OP_PRINTF("BKPT\t%d\n", (int)(((inst & 0x000FFF00) >> 8) | (inst & 0xF)))
		return 0;
	}

	/*
	 * MCRR / MRRC ?
	 */

	if((inst & MRR_INSTRUCTIONS_MASK) == MRR_INSTRUCTIONS_SIG)
	{
		if(_L_SET)
		{
			arm_op_add_reg_w(op, rd);
			arm_op_add_reg_w(op, rn);
			OP_PRINTF("MRRC")
		}
		else
		{
			arm_op_add_reg_r(op, rd);
			arm_op_add_reg_r(op, rn);
			OP_PRINTF("MCRR")
		}

		OP_PRINTF("\tp%d, %d, R%d, R%d, CR%d\n", (int)cp_num, (int)((inst >> 4) & 0xF), (int)rd, (int)rn, (int)crm)
		return 0;
	}


	/*
	 * MCR / MRC ?
	 */

	if((inst & MR_INSTRUCTIONS_MASK) == MR_INSTRUCTIONS_SIG)
	{

		if(_L_SET)
		{
			arm_op_add_reg_w(op, rd);
			OP_PRINTF("MRC")
		}
		else
		{
			arm_op_add_reg_r(op, rd);
			OP_PRINTF("MCR")
		}

		OP_PRINTF("\tp%d, %d, R%d, CR%d, CR%d, {%d}\n", (int)cp_num, (int)((inst >> 21) & 0x7), (int)rd, (int)rn, (int)crm, (int)((inst >> 5) & 0x7))
		return 0;
	}

	/*
	 * CDP ?
	 */

	if((inst & CDP_INSTRUCTION_MASK) == CDP_INSTRUCTION_SIG)
	{
		OP_PRINTF("CDP\tp%d, %d, CR%d, CR%d, CR%d, {%d}\n", (int)cp_num, (int)((inst >> 20) & 0xF), (int)rd, (int)rn, (int)crm, (int)((inst >> 5) & 0x7))
		return 0;
	}

	/*
	 * LDC / STC ?
	 */

	if(BIT_IS_CLEAR(25))
	{
		if(_L_SET)
			OP_PRINTF("LDC")
		else
			OP_PRINTF("STC")

		if(BIT_IS_SET(22))
			OP_PRINTF("L")
		OP_PRINTF("\tp%d, CR%d, ", (int)cp_num, (int)rd)
		decode_addressing_mode5(op);
		OP_PRINTF("\n")
		return 0;
	}

	printf("\n%d: CANNOT parse %08x\n", __LINE__, (unsigned int)inst);
	return 1;
}

