#include "LLVMDisassembler.hpp"
#include "LLVMtoFailGem5.hpp"
#include "sal/arm/ArmArchitecture.hpp"

using namespace fail;

LLVMtoFailGem5::LLVMtoFailGem5(LLVMDisassembler *disas) {
	std::map<std::string, struct reginfo_t> reg_name_map;

	reg_name_map["R0"] = reginfo_t(RI_R0);
	reg_name_map["R1"] = reginfo_t(RI_R1);
	reg_name_map["R2"] = reginfo_t(RI_R2);
	reg_name_map["R3"] = reginfo_t(RI_R3);
	reg_name_map["R4"] = reginfo_t(RI_R4);
	reg_name_map["R5"] = reginfo_t(RI_R5);
	reg_name_map["R6"] = reginfo_t(RI_R6);
	reg_name_map["R7"] = reginfo_t(RI_R7);
	reg_name_map["R8"] = reginfo_t(RI_R8);
	reg_name_map["R9"] = reginfo_t(RI_R9);
	reg_name_map["R10"] = reginfo_t(RI_R10);
	reg_name_map["R11"] = reginfo_t(RI_R11);
	reg_name_map["R12"] = reginfo_t(RI_R12);
	reg_name_map["SP"] = reginfo_t(RI_SP);
	reg_name_map["LR"] = reginfo_t(RI_LR);
	reg_name_map["PC"] = reginfo_t(RI_IP);

	const llvm::MCRegisterInfo &reg_info = disas->getRegisterInfo();
	for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i){
		std::string name = reg_info.getName(i);
		if (reg_name_map.count(name) > 0) {
			llvm_to_fail_map[i] = reg_name_map[name];
		}
	}
}
