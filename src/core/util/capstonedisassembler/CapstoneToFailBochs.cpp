#include <capstone/x86.h>
#include "CapstoneToFailBochs.hpp"
#include "sal/x86/X86Architecture.hpp"
#include "CapstoneDisassembler.hpp"

using namespace fail;

CapstoneToFailBochs::CapstoneToFailBochs(CapstoneDisassembler *disas) {
	capstone_to_fail_map[X86_REG_AH] = RegisterView(RID_CAX, 8,  8);
	capstone_to_fail_map[X86_REG_AL] = RegisterView(RID_CAX, 8);
	capstone_to_fail_map[X86_REG_AX] = RegisterView(RID_CAX, 16);
	capstone_to_fail_map[X86_REG_EAX] = RegisterView(RID_CAX, 32);
	capstone_to_fail_map[X86_REG_RAX] = RegisterView(RID_CAX, 64);

	capstone_to_fail_map[X86_REG_BH] = RegisterView(RID_CBX, 8, 8);
	capstone_to_fail_map[X86_REG_BL] = RegisterView(RID_CBX, 8);
	capstone_to_fail_map[X86_REG_BX] = RegisterView(RID_CBX, 16);
	capstone_to_fail_map[X86_REG_EBX] = RegisterView(RID_CBX, 32);
	capstone_to_fail_map[X86_REG_RBX] = RegisterView(RID_CBX, 64);

	capstone_to_fail_map[X86_REG_CH] = RegisterView(RID_CCX, 8, 8);
	capstone_to_fail_map[X86_REG_CL] = RegisterView(RID_CCX, 8);
	capstone_to_fail_map[X86_REG_CX] = RegisterView(RID_CCX, 16);
	capstone_to_fail_map[X86_REG_ECX] = RegisterView(RID_CCX, 32);
	capstone_to_fail_map[X86_REG_RCX] = RegisterView(RID_CCX, 64);

	capstone_to_fail_map[X86_REG_DH] = RegisterView(RID_CDX, 8, 8);
	capstone_to_fail_map[X86_REG_DL] = RegisterView(RID_CDX, 8);
	capstone_to_fail_map[X86_REG_DX] = RegisterView(RID_CDX, 16);
	capstone_to_fail_map[X86_REG_EDX] = RegisterView(RID_CDX, 32);
	capstone_to_fail_map[X86_REG_RDX] = RegisterView(RID_CDX, 64);

	capstone_to_fail_map[X86_REG_R8] = RegisterView(RID_R8, 64);
	capstone_to_fail_map[X86_REG_R8D] = RegisterView(RID_R8, 32);
	capstone_to_fail_map[X86_REG_R8W] = RegisterView(RID_R8, 16);
	capstone_to_fail_map[X86_REG_R8B] = RegisterView(RID_R8, 8);
	capstone_to_fail_map[X86_REG_R9] = RegisterView(RID_R9, 64);
	capstone_to_fail_map[X86_REG_R9D] = RegisterView(RID_R9, 32);
	capstone_to_fail_map[X86_REG_R9W] = RegisterView(RID_R9, 16);
	capstone_to_fail_map[X86_REG_R9B] = RegisterView(RID_R9, 8);
	capstone_to_fail_map[X86_REG_R10] = RegisterView(RID_R10, 64);
	capstone_to_fail_map[X86_REG_R10D] = RegisterView(RID_R10, 32);
	capstone_to_fail_map[X86_REG_R10W] = RegisterView(RID_R10, 16);
	capstone_to_fail_map[X86_REG_R10B] = RegisterView(RID_R10, 8);
	capstone_to_fail_map[X86_REG_R11] = RegisterView(RID_R11, 64);
	capstone_to_fail_map[X86_REG_R11D] = RegisterView(RID_R11, 32);
	capstone_to_fail_map[X86_REG_R11W] = RegisterView(RID_R11, 16);
	capstone_to_fail_map[X86_REG_R11B] = RegisterView(RID_R11, 8);
	capstone_to_fail_map[X86_REG_R12] = RegisterView(RID_R12, 64);
	capstone_to_fail_map[X86_REG_R12D] = RegisterView(RID_R12, 32);
	capstone_to_fail_map[X86_REG_R12W] = RegisterView(RID_R12, 16);
	capstone_to_fail_map[X86_REG_R12B] = RegisterView(RID_R12, 8);
	capstone_to_fail_map[X86_REG_R13] = RegisterView(RID_R13, 64);
	capstone_to_fail_map[X86_REG_R13D] = RegisterView(RID_R13, 32);
	capstone_to_fail_map[X86_REG_R13W] = RegisterView(RID_R13, 16);
	capstone_to_fail_map[X86_REG_R13B] = RegisterView(RID_R13, 8);
	capstone_to_fail_map[X86_REG_R14] = RegisterView(RID_R14, 64);
	capstone_to_fail_map[X86_REG_R14D] = RegisterView(RID_R14, 32);
	capstone_to_fail_map[X86_REG_R14W] = RegisterView(RID_R14, 16);
	capstone_to_fail_map[X86_REG_R14B] = RegisterView(RID_R14, 8);
	capstone_to_fail_map[X86_REG_R15] = RegisterView(RID_R15, 64);
	capstone_to_fail_map[X86_REG_R15D] = RegisterView(RID_R15, 32);
	capstone_to_fail_map[X86_REG_R15W] = RegisterView(RID_R15, 16);
	capstone_to_fail_map[X86_REG_R15B] = RegisterView(RID_R15, 8);

	capstone_to_fail_map[X86_REG_DIL] = RegisterView(RID_CDI, 8);
	capstone_to_fail_map[X86_REG_DI] = RegisterView(RID_CDI, 16);
	capstone_to_fail_map[X86_REG_EDI] = RegisterView(RID_CDI, 32);
	capstone_to_fail_map[X86_REG_RDI] = RegisterView(RID_CDI, 64);

	capstone_to_fail_map[X86_REG_BPL] = RegisterView(RID_CBP, 8);
	capstone_to_fail_map[X86_REG_BP] = RegisterView(RID_CBP, 16);
	capstone_to_fail_map[X86_REG_EBP] = RegisterView(RID_CBP, 32);
	capstone_to_fail_map[X86_REG_RBP] = RegisterView(RID_CBP, 64);

	capstone_to_fail_map[X86_REG_EFLAGS] = RegisterView(RID_FLAGS, disas->getWordWidth());
	// RFLAGS doesn't exist in the x86.h of capstone, therefore X86_REG_EFLAGS is set to 64bit
	// capstone_to_fail_map[RFLAGS] = RegisterView(RID_FLAGS, 64);

	capstone_to_fail_map[X86_REG_EIP] = RegisterView(RID_PC, 32);
	capstone_to_fail_map[X86_REG_RIP] = RegisterView(RID_PC, 64);

	capstone_to_fail_map[X86_REG_SIL] = RegisterView(RID_CSI, 8);
	capstone_to_fail_map[X86_REG_SI] = RegisterView(RID_CSI, 16);
	capstone_to_fail_map[X86_REG_ESI] = RegisterView(RID_CSI, 32);
	capstone_to_fail_map[X86_REG_RSI] = RegisterView(RID_CSI, 64);

	capstone_to_fail_map[X86_REG_SPL] = RegisterView(RID_CSP, 8);
	capstone_to_fail_map[X86_REG_SP] = RegisterView(RID_CSP, 16);
	capstone_to_fail_map[X86_REG_ESP] = RegisterView(RID_CSP, 32);
	capstone_to_fail_map[X86_REG_RSP] = RegisterView(RID_CSP, 64);

	capstone_to_fail_map[X86_REG_CR0] = RegisterView(RID_CR0);
	capstone_to_fail_map[X86_REG_CR2] = RegisterView(RID_CR2);
	capstone_to_fail_map[X86_REG_CR3] = RegisterView(RID_CR3);
	capstone_to_fail_map[X86_REG_CR4] = RegisterView(RID_CR4);

	capstone_to_fail_map[X86_REG_CS] = RegisterView(RID_CS, 16);
	capstone_to_fail_map[X86_REG_DS] = RegisterView(RID_DS, 16);
	capstone_to_fail_map[X86_REG_ES] = RegisterView(RID_ES, 16);
	capstone_to_fail_map[X86_REG_FS] = RegisterView(RID_FS, 16);
	capstone_to_fail_map[X86_REG_GS] = RegisterView(RID_GS, 16);
	capstone_to_fail_map[X86_REG_SS] = RegisterView(RID_SS, 16);
}
