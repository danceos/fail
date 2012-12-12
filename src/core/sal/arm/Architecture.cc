#include "Architecture.hpp"
#include "../Register.hpp"
#include <sstream>

namespace fail {

ArmArchitecture::ArmArchitecture()
{
	fillRegisterList();
}

void ArmArchitecture::fillRegisterList()
{
	// TODO: Add missing registers
	// 16x 32-Bit GP Registers
	for (int i = 0; i < 16; i++) {
		Register *reg = new Register(i, RT_GP, 32);
		// Build and set the register name:
		std::stringstream sstr;
		sstr << "R" << i+1;
		reg->setName(str.str());
		addRegister(reg);
	}
}

ArmArchitecture::~ArmArchitecture()
{
	for (std::vector<Register*>::iterator it = m_Registers.begin();
		 it != m_Registers.end(); it++)
		delete *it;
		it = m_Registers.erase(it);
	}
}

} // end-of-namespace: fail
