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

typedef enum {C_EQ = 0x0, C_NE = 0x1, C_CS = 0x2, C_CC = 0x3,
	     C_MI = 0x4, C_PL = 0x5, C_VS = 0x6, C_VC = 0x7,
	     C_HI = 0x8, C_LS = 0x9, C_GE = 0xA, C_LT = 0xB,
             C_GT = 0xC, C_LE = 0xD, C_AL = 0xE, C_NV = 0xF
}condition_t;

#define V_FLAG 0x10000000
#define C_FLAG 0x20000000
#define Z_FLAG 0x40000000
#define N_FLAG 0x80000000

#define CZ_FLAGS 0x60000000
#define NV_FLAGS 0x90000000

uint32_t op_cond_test(uint32_t inst, uint32_t cpsr)
{
	condition_t cond = (condition_t) (inst >> 28);
	switch (cond)
	{
	case C_EQ:
		return ( (cpsr & Z_FLAG));
	case C_NE:
		return (!(cpsr & Z_FLAG));
	case C_CS:
		return ( (cpsr & C_FLAG));
	case C_CC:
		return (!(cpsr & C_FLAG));
	case C_MI:
		return ( (cpsr & N_FLAG));
	case C_PL:
		return (!(cpsr & N_FLAG));
	case C_VS:
		return ( (cpsr & V_FLAG));
	case C_VC:
		return (!(cpsr & V_FLAG));

	case C_HI:
		return ((cpsr & CZ_FLAGS) == C_FLAG);
	case C_LS:
		return ((cpsr & C_FLAG) == 0) ||
			((cpsr & Z_FLAG) == Z_FLAG);
	case C_GE:
		return (((cpsr & NV_FLAGS) == 0) ||
			((cpsr & NV_FLAGS) == NV_FLAGS));
	case C_LT:
		return (((cpsr & NV_FLAGS) == V_FLAG) ||
			((cpsr & NV_FLAGS) == N_FLAG));
	case C_GT:
		return (((cpsr & Z_FLAG) == 0) &&
			(((cpsr & NV_FLAGS) == NV_FLAGS) ||
					((cpsr & NV_FLAGS) == 0)));
	case C_LE:
		return (((cpsr & Z_FLAG) == Z_FLAG) ||
			(((cpsr & NV_FLAGS) == V_FLAG) ||
					((cpsr & NV_FLAGS) == N_FLAG)));
	case C_AL:
		return 1;
	case C_NV:
	default:
		return 0xFFFFFFFF;
	}
	return 0;
}
