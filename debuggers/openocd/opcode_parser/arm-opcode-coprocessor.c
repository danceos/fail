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

