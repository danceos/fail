#include "LLVMDisassembler.hpp"
#include "LLVMtoFailBochs.hpp"
#include "sal/x86/X86Architecture.hpp"

using namespace fail;

LLVMtoFailBochs::LLVMtoFailBochs(LLVMDisassembler *disas) {
	std::map<std::string, struct RegisterView> reg_name_map;

	reg_name_map["AH"]  = RegisterView(RID_CAX, 8,  8);
	reg_name_map["AL"]  = RegisterView(RID_CAX, 8);
	reg_name_map["AX"]  = RegisterView(RID_CAX, 16);
	reg_name_map["EAX"] = RegisterView(RID_CAX, 32);
	reg_name_map["RAX"] = RegisterView(RID_CAX, 64);

	reg_name_map["BH"]  = RegisterView(RID_CBX, 8, 8);
	reg_name_map["BL"]  = RegisterView(RID_CBX, 8);
	reg_name_map["BX"]  = RegisterView(RID_CBX, 16);
	reg_name_map["EBX"] = RegisterView(RID_CBX, 32);
	reg_name_map["RBX"] = RegisterView(RID_CBX, 64);

	reg_name_map["CH"]  = RegisterView(RID_CCX, 8, 8);
	reg_name_map["CL"]  = RegisterView(RID_CCX, 8);
	reg_name_map["CX"]  = RegisterView(RID_CCX, 16);
	reg_name_map["ECX"] = RegisterView(RID_CCX, 32);
	reg_name_map["RCX"] = RegisterView(RID_CCX, 64);

	reg_name_map["DH"]  = RegisterView(RID_CDX, 8, 8);
	reg_name_map["DL"]  = RegisterView(RID_CDX, 8);
	reg_name_map["DX"]  = RegisterView(RID_CDX, 16);
	reg_name_map["EDX"] = RegisterView(RID_CDX, 32);
	reg_name_map["RDX"] = RegisterView(RID_CDX, 64);

	reg_name_map["R8"]   = RegisterView(RID_R8, 64);
	reg_name_map["R8D"]  = RegisterView(RID_R8, 32);
	reg_name_map["R8W"]  = RegisterView(RID_R8, 16);
	reg_name_map["R8B"]  = RegisterView(RID_R8, 8);
	reg_name_map["R9"]   = RegisterView(RID_R9, 64);
	reg_name_map["R9D"]  = RegisterView(RID_R9, 32);
	reg_name_map["R9W"]  = RegisterView(RID_R9, 16);
	reg_name_map["R9B"]  = RegisterView(RID_R9, 8);
	reg_name_map["R10"]  = RegisterView(RID_R10, 64);
	reg_name_map["R10D"] = RegisterView(RID_R10, 32);
	reg_name_map["R10W"] = RegisterView(RID_R10, 16);
	reg_name_map["R10B"] = RegisterView(RID_R10, 8);
	reg_name_map["R11"]  = RegisterView(RID_R11, 64);
	reg_name_map["R11D"] = RegisterView(RID_R11, 32);
	reg_name_map["R11W"] = RegisterView(RID_R11, 16);
	reg_name_map["R11B"] = RegisterView(RID_R11, 8);
	reg_name_map["R12"]  = RegisterView(RID_R12, 64);
	reg_name_map["R12D"] = RegisterView(RID_R12, 32);
	reg_name_map["R12W"] = RegisterView(RID_R12, 16);
	reg_name_map["R12B"] = RegisterView(RID_R12, 8);
	reg_name_map["R13"]  = RegisterView(RID_R13, 64);
	reg_name_map["R13D"] = RegisterView(RID_R13, 32);
	reg_name_map["R13W"] = RegisterView(RID_R13, 16);
	reg_name_map["R13B"] = RegisterView(RID_R13, 8);
	reg_name_map["R14"]  = RegisterView(RID_R14, 64);
	reg_name_map["R14D"] = RegisterView(RID_R14, 32);
	reg_name_map["R14W"] = RegisterView(RID_R14, 16);
	reg_name_map["R14B"] = RegisterView(RID_R14, 8);
	reg_name_map["R15"]  = RegisterView(RID_R15, 64);
	reg_name_map["R15D"] = RegisterView(RID_R15, 32);
	reg_name_map["R15W"] = RegisterView(RID_R15, 16);
	reg_name_map["R15B"] = RegisterView(RID_R15, 8);

	reg_name_map["DIL"] = RegisterView(RID_CDI, 8);
	reg_name_map["DI"]  = RegisterView(RID_CDI, 16);
	reg_name_map["EDI"] = RegisterView(RID_CDI, 32);
	reg_name_map["RDI"] = RegisterView(RID_CDI, 64);

	reg_name_map["BPL"] = RegisterView(RID_CBP, 8);
	reg_name_map["BP"]  = RegisterView(RID_CBP, 16);
	reg_name_map["EBP"] = RegisterView(RID_CBP, 32);
	reg_name_map["RBP"] = RegisterView(RID_CBP, 64);

	reg_name_map["EFLAGS"] = RegisterView(RID_FLAGS, 32);
	reg_name_map["RFLAGS"] = RegisterView(RID_FLAGS, 64);

	reg_name_map["EIP"] = RegisterView(RID_PC, 32);
	reg_name_map["RIP"] = RegisterView(RID_PC, 64);

	reg_name_map["SIL"] = RegisterView(RID_CSI, 8);
	reg_name_map["SI"]  = RegisterView(RID_CSI, 16);
	reg_name_map["ESI"] = RegisterView(RID_CSI, 32);
	reg_name_map["RSI"] = RegisterView(RID_CSI, 64);

	reg_name_map["SPL"] = RegisterView(RID_CSP, 8);
	reg_name_map["SP"]  = RegisterView(RID_CSP, 16);
	reg_name_map["ESP"] = RegisterView(RID_CSP, 32);
	reg_name_map["RSP"] = RegisterView(RID_CSP, 64);

	reg_name_map["CR0"] = RegisterView(RID_CR0);
	reg_name_map["CR2"] = RegisterView(RID_CR2);
	reg_name_map["CR3"] = RegisterView(RID_CR3);
	reg_name_map["CR4"] = RegisterView(RID_CR4);

	reg_name_map["CS"] = RegisterView(RID_CS, 16);
	reg_name_map["DS"] = RegisterView(RID_DS, 16);
	reg_name_map["ES"] = RegisterView(RID_ES, 16);
	reg_name_map["FS"] = RegisterView(RID_FS, 16);
	reg_name_map["GS"] = RegisterView(RID_GS, 16);
	reg_name_map["SS"] = RegisterView(RID_SS, 16);

	const llvm::MCRegisterInfo &reg_info = disas->getRegisterInfo();
	for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i){
		std::string name = reg_info.getName(i);
		if (reg_name_map.count(name) > 0) {
			llvm_to_fail_map[i] = reg_name_map[name];
		}
	}
}
