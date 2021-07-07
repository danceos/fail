#include "LLVMtoFailSailRiscv.hpp"
#include "LLVMDisassembler.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include "sal/sail/SailArchitecture.hpp"

using namespace fail;

LLVMtoFailSailRiscv::LLVMtoFailSailRiscv(LLVMDisassembler *disas) {
	std::map<std::string, struct RegisterView> m;
	unsigned xlen = disas->getWordWidth();

	std::string features = disas->GetSubtargetFeatures();

	std::cout << "features: " << features << std::endl;
	bool is_cheri = features.find("+xcheri") != std::string::npos;
	bool is_cheri_purecap = features.find("+cap-mode") != std::string::npos;

	std::cout << std::boolalpha << " is_cheri: " << is_cheri << " is_purecap: " << is_cheri_purecap << std::endl;

	m["pc"] = RegisterView(RID_PC, 2*xlen);
	m["ra"] = RegisterView(RID_RA, xlen);
	m["x1"] = RegisterView(RID_RA, xlen);
	m["sp"] = RegisterView(RID_SP, xlen);
	m["x2"] = RegisterView(RID_SP, xlen);
	m["gp"] = RegisterView(RID_GP, xlen);
	m["x3"] = RegisterView(RID_GP , xlen);
	m["tp"] = RegisterView(RID_TP, xlen);
	m["x4"] = RegisterView(RID_TP, xlen);
	m["t0"] = RegisterView(RID_T0, xlen);
	m["x5"] = RegisterView(RID_T0, xlen);
	m["t1"] = RegisterView(RID_T1, xlen);
	m["x6"] = RegisterView(RID_T1, xlen);
	m["t2"] = RegisterView(RID_T2, xlen);
	m["x7"] = RegisterView(RID_T2, xlen);
	m["s0"] = RegisterView(RID_S0, xlen);
	m["x8"] = RegisterView(RID_S0, xlen);
	m["s1"] = RegisterView(RID_S1, xlen);
	m["x9"] = RegisterView(RID_S1, xlen);
	m["a0"] = RegisterView(RID_A0, xlen);
	m["x10"] = RegisterView(RID_A0, xlen);
	m["a1"] = RegisterView(RID_A1, xlen);
	m["x11"] = RegisterView(RID_A1, xlen);
	m["a2"] = RegisterView(RID_A2, xlen);
	m["x12"] = RegisterView(RID_A2, xlen);
	m["a3"] = RegisterView(RID_A3, xlen);
	m["x13"] = RegisterView(RID_A3, xlen);
	m["a4"] = RegisterView(RID_A4, xlen);
	m["x14"] = RegisterView(RID_A4, xlen);
	m["a5"] = RegisterView(RID_A5, xlen);
	m["x15"] = RegisterView(RID_A5, xlen);
	m["a6"] = RegisterView(RID_A6, xlen);
	m["x16"] = RegisterView(RID_A6, xlen);
	m["a7"] = RegisterView(RID_A7, xlen);
	m["x17"] = RegisterView(RID_A7, xlen);
	m["s2"] = RegisterView(RID_S2, xlen);
	m["x18"] = RegisterView(RID_S2, xlen);
	m["s3"] = RegisterView(RID_S3, xlen);
	m["x19"] = RegisterView(RID_S3, xlen);
	m["s4"] = RegisterView(RID_S4, xlen);
	m["x20"] = RegisterView(RID_S4, xlen);
	m["s5"] = RegisterView(RID_S5, xlen);
	m["x21"] = RegisterView(RID_S5, xlen);
	m["s6"] = RegisterView(RID_S6, xlen);
	m["x22"] = RegisterView(RID_S6, xlen);
	m["s7"] = RegisterView(RID_S7, xlen);
	m["x23"] = RegisterView(RID_S7, xlen);
	m["s8"] = RegisterView(RID_S8, xlen);
	m["x24"] = RegisterView(RID_S8, xlen);
	m["s9"] = RegisterView(RID_S9, xlen);
	m["x25"] = RegisterView(RID_S9, xlen);
	m["s10"] = RegisterView(RID_S10, xlen);
	m["x26"] = RegisterView(RID_S10, xlen);
	m["s11"] = RegisterView(RID_S11, xlen);
	m["x27"] = RegisterView(RID_S11, xlen);
	m["t3"] = RegisterView(RID_T3, xlen);
	m["x28"] = RegisterView(RID_T3, xlen);
	m["t4"] = RegisterView(RID_T4, xlen);
	m["x29"] = RegisterView(RID_T4, xlen);
	m["t5"] = RegisterView(RID_T5, xlen);
	m["x30"] = RegisterView(RID_T5, xlen);
	m["t6"] = RegisterView(RID_T6, xlen);
	m["x31"] = RegisterView(RID_T6, xlen);

	if(is_cheri) {
#ifdef BUILD_RISCV_CHERI
		m["ddc"] = RegisterView(RID_DDC, 2*xlen);
		if(is_cheri_purecap) {
			m["c1"] = RegisterView(RID_RA, 2*xlen);
			m["c2"] = RegisterView(RID_SP, 2*xlen);
			m["c3"] = RegisterView(RID_GP, 2*xlen);
			m["c4"] = RegisterView(RID_TP, 2*xlen);
			m["c5"] = RegisterView(RID_T0, 2*xlen);
			m["c6"] = RegisterView(RID_T1, 2*xlen);
			m["c7"] = RegisterView(RID_T2, 2*xlen);
			m["c8"] = RegisterView(RID_S0, 2*xlen);
			m["c9"] = RegisterView(RID_S1, 2*xlen);
			m["c10"] = RegisterView(RID_A0, 2*xlen);
			m["c11"] = RegisterView(RID_A1, 2*xlen);
			m["c12"] = RegisterView(RID_A2, 2*xlen);
			m["c13"] = RegisterView(RID_A3, 2*xlen);
			m["c14"] = RegisterView(RID_A4, 2*xlen);
			m["c15"] = RegisterView(RID_A5, 2*xlen);
			m["c16"] = RegisterView(RID_A6, 2*xlen);
			m["c17"] = RegisterView(RID_A7, 2*xlen);
			m["c18"] = RegisterView(RID_S2, 2*xlen);
			m["c19"] = RegisterView(RID_S3, 2*xlen);
			m["c20"] = RegisterView(RID_S4, 2*xlen);
			m["c21"] = RegisterView(RID_S5, 2*xlen);
			m["c22"] = RegisterView(RID_S6, 2*xlen);
			m["c23"] = RegisterView(RID_S7, 2*xlen);
			m["c24"] = RegisterView(RID_S8, 2*xlen);
			m["c25"] = RegisterView(RID_S9, 2*xlen);
			m["c26"] = RegisterView(RID_S10, 2*xlen);
			m["c27"] = RegisterView(RID_S11, 2*xlen);
			m["c28"] = RegisterView(RID_T3, 2*xlen);
			m["c29"] = RegisterView(RID_T4, 2*xlen);
			m["c30"] = RegisterView(RID_T5, 2*xlen);
			m["c31"] = RegisterView(RID_T6, 2*xlen);
		}
		else {
			// FIXME: what is actually needed here?
			// TODO: explore how hybrid binary look
			throw std::runtime_error("non purecap code is currently not supported.");
		}
#endif
	}

	const llvm::MCRegisterInfo &reg_info = disas->getRegisterInfo();
	std::vector<std::string> failed_translations;
	for (unsigned int i = 0; i < reg_info.getNumRegs(); ++i) {
		std::string name = reg_info.getName(i);
		std::transform(name.begin(),name.end(),name.begin(),[] (auto c) { return std::tolower(c); });
		if (m.count(name) > 0) {
			llvm_to_fail_map[i] = m[name];
		} else {
			failed_translations.push_back(name);
		}
	}
	std::cout << "failed to find translations for: ";
	std::string out;
	for(const auto& n: failed_translations)
		out += "," + n;
	std::cout << out << std::endl;
}
