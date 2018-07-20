#include "LLVMDisassembler.hpp"
#include "LLVMtoFailBochs.hpp"
#include "sal/x86/X86Architecture.hpp"

using namespace fail;

LLVMtoFailBochs::LLVMtoFailBochs(LLVMDisassembler *disas) {
	std::map<std::string, struct reginfo_t> reg_name_map;

	reg_name_map["AH"] = reginfo_t(RID_CAX, 8,  8);
	reg_name_map["AL"] = reginfo_t(RID_CAX, 8);
	reg_name_map["AX"] = reginfo_t(RID_CAX, 16);
	reg_name_map["EAX"] = reginfo_t(RID_CAX, 32);
	reg_name_map["RAX"] = reginfo_t(RID_CAX, 64);

	reg_name_map["BH"] = reginfo_t(RID_CBX, 8, 8);
	reg_name_map["BL"] = reginfo_t(RID_CBX, 8);
	reg_name_map["BX"] = reginfo_t(RID_CBX, 16);
	reg_name_map["EBX"] = reginfo_t(RID_CBX, 32);
	reg_name_map["RBX"] = reginfo_t(RID_CBX, 64);

	reg_name_map["CH"] = reginfo_t(RID_CCX, 8, 8);
	reg_name_map["CL"] = reginfo_t(RID_CCX, 8);
	reg_name_map["CX"] = reginfo_t(RID_CCX, 16);
	reg_name_map["ECX"] = reginfo_t(RID_CCX, 32);
	reg_name_map["RCX"] = reginfo_t(RID_CCX, 64);

	reg_name_map["DH"] = reginfo_t(RID_CDX, 8, 8);
	reg_name_map["DL"] = reginfo_t(RID_CDX, 8);
	reg_name_map["DX"] = reginfo_t(RID_CDX, 16);
	reg_name_map["EDX"] = reginfo_t(RID_CDX, 32);
	reg_name_map["RDX"] = reginfo_t(RID_CDX, 64);

	reg_name_map["R8"] = reginfo_t(RID_R8, 64);
	reg_name_map["R8D"] = reginfo_t(RID_R8, 32);
	reg_name_map["R8W"] = reginfo_t(RID_R8, 16);
	reg_name_map["R8B"] = reginfo_t(RID_R8, 8);
	reg_name_map["R9"] = reginfo_t(RID_R9, 64);
	reg_name_map["R9D"] = reginfo_t(RID_R9, 32);
	reg_name_map["R9W"] = reginfo_t(RID_R9, 16);
	reg_name_map["R9B"] = reginfo_t(RID_R9, 8);
	reg_name_map["R10"] = reginfo_t(RID_R10, 64);
	reg_name_map["R10D"] = reginfo_t(RID_R10, 32);
	reg_name_map["R10W"] = reginfo_t(RID_R10, 16);
	reg_name_map["R10B"] = reginfo_t(RID_R10, 8);
	reg_name_map["R11"] = reginfo_t(RID_R11, 64);
	reg_name_map["R11D"] = reginfo_t(RID_R11, 32);
	reg_name_map["R11W"] = reginfo_t(RID_R11, 16);
	reg_name_map["R11B"] = reginfo_t(RID_R11, 8);
	reg_name_map["R12"] = reginfo_t(RID_R12, 64);
	reg_name_map["R12D"] = reginfo_t(RID_R12, 32);
	reg_name_map["R12W"] = reginfo_t(RID_R12, 16);
	reg_name_map["R12B"] = reginfo_t(RID_R12, 8);
	reg_name_map["R13"] = reginfo_t(RID_R13, 64);
	reg_name_map["R13D"] = reginfo_t(RID_R13, 32);
	reg_name_map["R13W"] = reginfo_t(RID_R13, 16);
	reg_name_map["R13B"] = reginfo_t(RID_R13, 8);
	reg_name_map["R14"] = reginfo_t(RID_R14, 64);
	reg_name_map["R14D"] = reginfo_t(RID_R14, 32);
	reg_name_map["R14W"] = reginfo_t(RID_R14, 16);
	reg_name_map["R14B"] = reginfo_t(RID_R14, 8);
	reg_name_map["R15"] = reginfo_t(RID_R15, 64);
	reg_name_map["R15D"] = reginfo_t(RID_R15, 32);
	reg_name_map["R15W"] = reginfo_t(RID_R15, 16);
	reg_name_map["R15B"] = reginfo_t(RID_R15, 8);

	reg_name_map["DIL"] = reginfo_t(RID_CDI, 8);
	reg_name_map["DI"] = reginfo_t(RID_CDI, 16);
	reg_name_map["EDI"] = reginfo_t(RID_CDI, 32);
	reg_name_map["RDI"] = reginfo_t(RID_CDI, 64);

	reg_name_map["BPL"] = reginfo_t(RID_CBP, 8);
	reg_name_map["BP"] = reginfo_t(RID_CBP, 16);
	reg_name_map["EBP"] = reginfo_t(RID_CBP, 32);
	reg_name_map["RBP"] = reginfo_t(RID_CBP, 64);

	reg_name_map["EFLAGS"] = reginfo_t(RID_FLAGS, 32);
	reg_name_map["RFLAGS"] = reginfo_t(RID_FLAGS, 64);

	reg_name_map["EIP"] = reginfo_t(RID_PC, 32);
	reg_name_map["RIP"] = reginfo_t(RID_PC, 64);

	reg_name_map["SIL"] = reginfo_t(RID_CSI, 8);
	reg_name_map["SI"] = reginfo_t(RID_CSI, 16);
	reg_name_map["ESI"] = reginfo_t(RID_CSI, 32);
	reg_name_map["RSI"] = reginfo_t(RID_CSI, 64);

	reg_name_map["SPL"] = reginfo_t(RID_CSP, 8);
	reg_name_map["SP"] = reginfo_t(RID_CSP, 16);
	reg_name_map["ESP"] = reginfo_t(RID_CSP, 32);
	reg_name_map["RSP"] = reginfo_t(RID_CSP, 64);

	reg_name_map["CR0"] = reginfo_t(RID_CR0);
	reg_name_map["CR2"] = reginfo_t(RID_CR2);
	reg_name_map["CR3"] = reginfo_t(RID_CR3);
	reg_name_map["CR4"] = reginfo_t(RID_CR4);

	reg_name_map["CS"] = reginfo_t(RID_CS, 16);
	reg_name_map["DS"] = reginfo_t(RID_DS, 16);
	reg_name_map["ES"] = reginfo_t(RID_ES, 16);
	reg_name_map["FS"] = reginfo_t(RID_FS, 16);
	reg_name_map["GS"] = reginfo_t(RID_GS, 16);
	reg_name_map["SS"] = reginfo_t(RID_SS, 16);

	const llvm::MCRegisterInfo &reg_info = disas->getRegisterInfo();
	for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i){
		std::string name = reg_info.getName(i);
		if (reg_name_map.count(name) > 0) {
			llvm_to_fail_map[i] = reg_name_map[name];
		}
	}
}
