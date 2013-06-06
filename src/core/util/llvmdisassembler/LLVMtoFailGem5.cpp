#include "LLVMtoFailGem5.hpp"
#include "sal/arm/ArmArchitecture.hpp"

using namespace fail;

LLVMtoFailGem5::LLVMtoFailGem5() {
#ifndef __puma
	/* These magic numbers are taken from the machine descriptions of
	   LLVM they (hopefully) will not change, since they are not exported
	   via a header */
	llvm_to_fail_map[60]   = reginfo_t(RI_R0);
	llvm_to_fail_map[61]   = reginfo_t(RI_R1);
	llvm_to_fail_map[62]   = reginfo_t(RI_R2);
	llvm_to_fail_map[63]   = reginfo_t(RI_R3);
	llvm_to_fail_map[64]   = reginfo_t(RI_R4);
	llvm_to_fail_map[65]   = reginfo_t(RI_R5);
	llvm_to_fail_map[66]   = reginfo_t(RI_R6);
	llvm_to_fail_map[67]   = reginfo_t(RI_R7);
	llvm_to_fail_map[68]   = reginfo_t(RI_R8);
	llvm_to_fail_map[69]   = reginfo_t(RI_R9);
	llvm_to_fail_map[70]   = reginfo_t(RI_R10);
	llvm_to_fail_map[71]   = reginfo_t(RI_R11);
	llvm_to_fail_map[72]   = reginfo_t(RI_R12);
	llvm_to_fail_map[105]	= reginfo_t(RI_SP);
	llvm_to_fail_map[40]   = reginfo_t(RI_LR);
	llvm_to_fail_map[43]   = reginfo_t(RI_IP);
#endif
}
