#include "LLVMDisassembler.hpp"
#include "LLVMtoFailBochs.hpp"
#include "sal/x86/X86Architecture.hpp"

using namespace fail;

LLVMtoFailBochs::LLVMtoFailBochs(LLVMDisassembler *disas) {
	std::map<std::string, struct reginfo_t> reg_name_map;

	reg_name_map["AH"] = reginfo_t(RID_CAX, 8,  8);
	reg_name_map["AL"] = reginfo_t(RID_CAX, 8,  0);
	reg_name_map["AX"] = reginfo_t(RID_CAX, 16, 0);
	reg_name_map["EAX"] = reginfo_t(RID_CAX, 32, 0);

	reg_name_map["BH"] = reginfo_t(RID_CBX, 8,  8);
	reg_name_map["BL"] = reginfo_t(RID_CBX, 8,  0);
	reg_name_map["BX"] = reginfo_t(RID_CBX, 16, 0);
	reg_name_map["EBX"] = reginfo_t(RID_CBX, 32, 0);

	reg_name_map["CH"] = reginfo_t(RID_CCX, 8, 8);
	reg_name_map["CL"] = reginfo_t(RID_CCX, 8, 0);
	reg_name_map["CX"] = reginfo_t(RID_CCX, 16, 0);
	reg_name_map["ECX"] = reginfo_t(RID_CCX);

	reg_name_map["DH"] = reginfo_t(RID_CDX, 8, 8);
	reg_name_map["DL"] = reginfo_t(RID_CDX, 8, 0);
	reg_name_map["DX"] = reginfo_t(RID_CDX, 16, 0);
	reg_name_map["EDX"] = reginfo_t(RID_CDX);

	reg_name_map["DI"] = reginfo_t(RID_CDI, 16, 0);
	reg_name_map["DIL"] = reginfo_t(RID_CDI, 8, 0);
	reg_name_map["EDI"] = reginfo_t(RID_CDI);

	reg_name_map["BP"] = reginfo_t(RID_CBP, 16, 0);
	reg_name_map["BPL"] = reginfo_t(RID_CBP, 8, 0);
	reg_name_map["EBP"] = reginfo_t(RID_CBP);

	reg_name_map["EFLAGS"] = reginfo_t(RID_FLAGS);

	reg_name_map["EIP"] = reginfo_t(RID_PC);

	reg_name_map["SI"] = reginfo_t(RID_CSI, 16, 0);
	reg_name_map["ESI"] = reginfo_t(RID_CSI);

	reg_name_map["ESP"] = reginfo_t(RID_CSP);
	reg_name_map["SP"] = reginfo_t(RID_CSP, 16, 0);
	reg_name_map["SPL"] = reginfo_t(RID_CSP, 8, 0);

	reg_name_map["CR0"] = reginfo_t(RID_CR0);
	reg_name_map["CR2"] = reginfo_t(RID_CR2);
	reg_name_map["CR3"] = reginfo_t(RID_CR3);
	reg_name_map["CR4"] = reginfo_t(RID_CR4);

	reg_name_map["CS"] = reginfo_t(RID_CS, 16, 0);
	reg_name_map["DS"] = reginfo_t(RID_DS, 16, 0);
	reg_name_map["ES"] = reginfo_t(RID_ES, 16, 0);
	reg_name_map["FS"] = reginfo_t(RID_FS, 16, 0);
	reg_name_map["GS"] = reginfo_t(RID_GS, 16, 0);
	reg_name_map["SS"] = reginfo_t(RID_SS, 16, 0);

	const llvm::MCRegisterInfo &reg_info = disas->getRegisterInfo();
	for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i){
		std::string name = reg_info.getName(i);
		if (reg_name_map.count(name) > 0) {
			llvm_to_fail_map[i] = reg_name_map[name];
		}
	}
}
