#include "SailCPU.hpp"

using namespace fail;

SailBaseCPU::SailBaseCPU(unsigned int id)
	: id(id) {
	pc_reg = getRegister(RID_PC);
	sp_reg = getRegister(RID_SP);
}

void SailBaseCPU::serialize(std::ostream& os) {
	for (Register *reg : *static_cast<SailArchitecture *>(this)) {
		os << getRegisterContent(reg) << " ";
	}
}

void SailBaseCPU::unserialize(std::istream& is) {
	for (Register *reg : *static_cast<SailArchitecture *>(this)) {
		regdata_t data;
		is >> data;
		setRegisterContent(reg, data);
	}
}
