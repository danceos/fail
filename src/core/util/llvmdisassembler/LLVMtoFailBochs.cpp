#include "LLVMtoFailBochs.hpp"
#include "sal/x86/X86Architecture.hpp"

using namespace fail;

LLVMtoFailBochs::LLVMtoFailBochs() {
	/* These magic numbers are taken from the llvm compiler (MC), they
	   do not appear in any header. They hopefully will never
	   change */
	llvm_to_fail_map[1]	 = reginfo_t(RID_CAX, 8,  8) ;	// AH
	llvm_to_fail_map[2]	 = reginfo_t(RID_CAX, 8,  0);		// AL
	llvm_to_fail_map[3]	 = reginfo_t(RID_CAX, 16, 0);		// AX
	llvm_to_fail_map[43] = reginfo_t(RID_CAX, 32, 0);		// EAX

	llvm_to_fail_map[4]	 = reginfo_t(RID_CBX, 8,  8);		// BH
	llvm_to_fail_map[5]	 = reginfo_t(RID_CBX, 8,  0);		// BL
	llvm_to_fail_map[8]	 = reginfo_t(RID_CBX, 16, 0);		// BX
	llvm_to_fail_map[45] = reginfo_t(RID_CBX, 32, 0);		// EBX

	llvm_to_fail_map[9]	 = reginfo_t(RID_CCX, 8, 8);		// CH
	llvm_to_fail_map[10] = reginfo_t(RID_CCX, 0xff);		// CL
	llvm_to_fail_map[28] = reginfo_t(RID_CCX, 16, 0);		// CX
	llvm_to_fail_map[46] = reginfo_t(RID_CCX);	// ECX

	llvm_to_fail_map[29] = reginfo_t(RID_CDX, 8, 8);		// DH
	llvm_to_fail_map[32] = reginfo_t(RID_CDX, 0xff);		// DL
	llvm_to_fail_map[42] = reginfo_t(RID_CDX, 16, 0);		// DX
	llvm_to_fail_map[48] = reginfo_t(RID_CDX);	// EDX

	llvm_to_fail_map[30] = reginfo_t(RID_CDI, 16, 0);		// DI
	llvm_to_fail_map[31] = reginfo_t(RID_CDI, 8, 0);		// DIL
	llvm_to_fail_map[47] = reginfo_t(RID_CDI);	// EDI

	llvm_to_fail_map[6]	 = reginfo_t(RID_CBP, 16, 0);		// BP
	llvm_to_fail_map[7]	 = reginfo_t(RID_CBP, 8, 0);		// BPL
	llvm_to_fail_map[44] = reginfo_t(RID_CBP);	// EBP

	llvm_to_fail_map[49]   = reginfo_t(RID_FLAGS);		  // EFLAGS

	llvm_to_fail_map[50]   = reginfo_t(RID_PC);			   // EIP

	llvm_to_fail_map[115] = reginfo_t(RID_CSI, 16, 0);	// SI
	llvm_to_fail_map[53]  = reginfo_t(RID_CSI); // ESI

	llvm_to_fail_map[54]  = reginfo_t(RID_CSP); // ESP
	llvm_to_fail_map[117] = reginfo_t(RID_CSP, 16, 0);	// SP
	llvm_to_fail_map[118] = reginfo_t(RID_CSP, 8, 0);		// SPL
}
