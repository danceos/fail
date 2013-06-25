#include "ArmArchitecture.hpp"
#include "../Register.hpp"
#include <sstream>

namespace fail {

ArmArchitecture::ArmArchitecture()
{
	// TODO: Add missing registers
	// 16x 32-Bit GP Registers
	for (int i = 0; i < 16; i++) {
		if (i != RI_IP) { // IP will be added separately (see below)
			Register *reg = new Register(i, 32);
			// Build and set the register name:
			std::stringstream sstr;
			sstr << "R" << i+1;
			reg->setName(sstr.str().c_str());
			m_addRegister(reg, RT_GP);
		}
	}

	// Instruction Pointer:
	Register *reg = new Register(RI_IP, 32);
	reg->setName("IP");
	m_addRegister(reg, RT_IP);

	// Registers used for extended tracing:
	size_t ids[] = {RI_R0, RI_R1, RI_R2, RI_R3, RI_R4, RI_R5, RI_R6, RI_R7,
	                RI_R8, RI_R9, RI_R10, RI_R11, RI_R12, RI_R13, RI_R14};
	for (size_t i = 0; i < sizeof(ids)/sizeof(*ids); ++i) {
		m_addRegister(getRegister(ids[i]), RT_TRACE);
	}
}

ArmArchitecture::~ArmArchitecture()
{
	for (std::vector<Register*>::iterator it = m_Registers.begin(); it != m_Registers.end(); it++)
		delete *it;
	m_Registers.clear();
}

} // end-of-namespace: fail
