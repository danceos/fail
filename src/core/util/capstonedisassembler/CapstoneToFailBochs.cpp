#include <capstone/x86.h>
#include "CapstoneToFailBochs.hpp"
#include "sal/x86/X86Architecture.hpp"

using namespace fail;

CapstoneToFailBochs::CapstoneToFailBochs() {
	capstone_to_fail_map[X86_REG_AH] = reginfo_t(RID_CAX, 8,  8);
	capstone_to_fail_map[X86_REG_AL] = reginfo_t(RID_CAX, 8);
	capstone_to_fail_map[X86_REG_AX] = reginfo_t(RID_CAX, 16);
	capstone_to_fail_map[X86_REG_EAX] = reginfo_t(RID_CAX, 32);
	capstone_to_fail_map[X86_REG_RAX] = reginfo_t(RID_CAX, 64);

	capstone_to_fail_map[X86_REG_BH] = reginfo_t(RID_CBX, 8, 8);
	capstone_to_fail_map[X86_REG_BL] = reginfo_t(RID_CBX, 8);
	capstone_to_fail_map[X86_REG_BX] = reginfo_t(RID_CBX, 16);
	capstone_to_fail_map[X86_REG_EBX] = reginfo_t(RID_CBX, 32);
	capstone_to_fail_map[X86_REG_RBX] = reginfo_t(RID_CBX, 64);

	capstone_to_fail_map[X86_REG_CH] = reginfo_t(RID_CCX, 8, 8);
	capstone_to_fail_map[X86_REG_CL] = reginfo_t(RID_CCX, 8);
	capstone_to_fail_map[X86_REG_CX] = reginfo_t(RID_CCX, 16);
	capstone_to_fail_map[X86_REG_ECX] = reginfo_t(RID_CCX, 32);
	capstone_to_fail_map[X86_REG_RCX] = reginfo_t(RID_CCX, 64);

	capstone_to_fail_map[X86_REG_DH] = reginfo_t(RID_CDX, 8, 8);
	capstone_to_fail_map[X86_REG_DL] = reginfo_t(RID_CDX, 8);
	capstone_to_fail_map[X86_REG_DX] = reginfo_t(RID_CDX, 16);
	capstone_to_fail_map[X86_REG_EDX] = reginfo_t(RID_CDX, 32);
	capstone_to_fail_map[X86_REG_RDX] = reginfo_t(RID_CDX, 64);

	capstone_to_fail_map[X86_REG_R8] = reginfo_t(RID_R8, 64);
	capstone_to_fail_map[X86_REG_R8D] = reginfo_t(RID_R8, 32);
	capstone_to_fail_map[X86_REG_R8W] = reginfo_t(RID_R8, 16);
	capstone_to_fail_map[X86_REG_R8B] = reginfo_t(RID_R8, 8);
	capstone_to_fail_map[X86_REG_R9] = reginfo_t(RID_R9, 64);
	capstone_to_fail_map[X86_REG_R9D] = reginfo_t(RID_R9, 32);
	capstone_to_fail_map[X86_REG_R9W] = reginfo_t(RID_R9, 16);
	capstone_to_fail_map[X86_REG_R9B] = reginfo_t(RID_R9, 8);
	capstone_to_fail_map[X86_REG_R10] = reginfo_t(RID_R10, 64);
	capstone_to_fail_map[X86_REG_R10D] = reginfo_t(RID_R10, 32);
	capstone_to_fail_map[X86_REG_R10W] = reginfo_t(RID_R10, 16);
	capstone_to_fail_map[X86_REG_R10B] = reginfo_t(RID_R10, 8);
	capstone_to_fail_map[X86_REG_R11] = reginfo_t(RID_R11, 64);
	capstone_to_fail_map[X86_REG_R11D] = reginfo_t(RID_R11, 32);
	capstone_to_fail_map[X86_REG_R11W] = reginfo_t(RID_R11, 16);
	capstone_to_fail_map[X86_REG_R11B] = reginfo_t(RID_R11, 8);
	capstone_to_fail_map[X86_REG_R12] = reginfo_t(RID_R12, 64);
	capstone_to_fail_map[X86_REG_R12D] = reginfo_t(RID_R12, 32);
	capstone_to_fail_map[X86_REG_R12W] = reginfo_t(RID_R12, 16);
	capstone_to_fail_map[X86_REG_R12B] = reginfo_t(RID_R12, 8);
	capstone_to_fail_map[X86_REG_R13] = reginfo_t(RID_R13, 64);
	capstone_to_fail_map[X86_REG_R13D] = reginfo_t(RID_R13, 32);
	capstone_to_fail_map[X86_REG_R13W] = reginfo_t(RID_R13, 16);
	capstone_to_fail_map[X86_REG_R13B] = reginfo_t(RID_R13, 8);
	capstone_to_fail_map[X86_REG_R14] = reginfo_t(RID_R14, 64);
	capstone_to_fail_map[X86_REG_R14D] = reginfo_t(RID_R14, 32);
	capstone_to_fail_map[X86_REG_R14W] = reginfo_t(RID_R14, 16);
	capstone_to_fail_map[X86_REG_R14B] = reginfo_t(RID_R14, 8);
	capstone_to_fail_map[X86_REG_R15] = reginfo_t(RID_R15, 64);
	capstone_to_fail_map[X86_REG_R15D] = reginfo_t(RID_R15, 32);
	capstone_to_fail_map[X86_REG_R15W] = reginfo_t(RID_R15, 16);
	capstone_to_fail_map[X86_REG_R15B] = reginfo_t(RID_R15, 8);

	capstone_to_fail_map[X86_REG_DIL] = reginfo_t(RID_CDI, 8);
	capstone_to_fail_map[X86_REG_DI] = reginfo_t(RID_CDI, 16);
	capstone_to_fail_map[X86_REG_EDI] = reginfo_t(RID_CDI, 32);
	capstone_to_fail_map[X86_REG_RDI] = reginfo_t(RID_CDI, 64);

	capstone_to_fail_map[X86_REG_BPL] = reginfo_t(RID_CBP, 8);
	capstone_to_fail_map[X86_REG_BP] = reginfo_t(RID_CBP, 16);
	capstone_to_fail_map[X86_REG_EBP] = reginfo_t(RID_CBP, 32);
	capstone_to_fail_map[X86_REG_RBP] = reginfo_t(RID_CBP, 64);

	capstone_to_fail_map[X86_REG_EFLAGS] = reginfo_t(RID_FLAGS, 64);
	// RFLAGS doesn't exist in the x86.h of capstone, therefore X86_REG_EFLAGS is set to 64bit
	// capstone_to_fail_map[RFLAGS] = reginfo_t(RID_FLAGS, 64);

	capstone_to_fail_map[X86_REG_EIP] = reginfo_t(RID_PC, 32);
	capstone_to_fail_map[X86_REG_RIP] = reginfo_t(RID_PC, 64);

	capstone_to_fail_map[X86_REG_SIL] = reginfo_t(RID_CSI, 8);
	capstone_to_fail_map[X86_REG_SI] = reginfo_t(RID_CSI, 16);
	capstone_to_fail_map[X86_REG_ESI] = reginfo_t(RID_CSI, 32);
	capstone_to_fail_map[X86_REG_RSI] = reginfo_t(RID_CSI, 64);

	capstone_to_fail_map[X86_REG_SPL] = reginfo_t(RID_CSP, 8);
	capstone_to_fail_map[X86_REG_SP] = reginfo_t(RID_CSP, 16);
	capstone_to_fail_map[X86_REG_ESP] = reginfo_t(RID_CSP, 32);
	capstone_to_fail_map[X86_REG_RSP] = reginfo_t(RID_CSP, 64);

	capstone_to_fail_map[X86_REG_CR0] = reginfo_t(RID_CR0);
	capstone_to_fail_map[X86_REG_CR2] = reginfo_t(RID_CR2);
	capstone_to_fail_map[X86_REG_CR3] = reginfo_t(RID_CR3);
	capstone_to_fail_map[X86_REG_CR4] = reginfo_t(RID_CR4);

	capstone_to_fail_map[X86_REG_CS] = reginfo_t(RID_CS, 16);
	capstone_to_fail_map[X86_REG_DS] = reginfo_t(RID_DS, 16);
	capstone_to_fail_map[X86_REG_ES] = reginfo_t(RID_ES, 16);
	capstone_to_fail_map[X86_REG_FS] = reginfo_t(RID_FS, 16);
	capstone_to_fail_map[X86_REG_GS] = reginfo_t(RID_GS, 16);
	capstone_to_fail_map[X86_REG_SS] = reginfo_t(RID_SS, 16);
}
