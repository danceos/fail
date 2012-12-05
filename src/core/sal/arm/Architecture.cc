#include "ArmArchitecture.hpp"
#include "../Register.hpp"

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
		addRegister(reg);
	}
}

ArmArchitecture::~ArmArchitecture()
{
	std::vector< Register* >::iterator it = m_Registers.begin();
	while (it != m_Registers.end()) {
		delete *it;
		it = m_Registers.erase(it);
	}
}

} // end-of-namespace: fail
