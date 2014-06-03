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
