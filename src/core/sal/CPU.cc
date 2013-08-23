#include <algorithm>
#include "CPU.hpp"

namespace fail {

void CPUArchitecture::m_addRegister(Register* reg, RegisterType type)
{
	// We may be called multiple times with the same register, if it needs to
	// reside in multiple subsets.
	if (std::find(m_Registers.begin(), m_Registers.end(), reg) == m_Registers.end()) {
		m_Registers.push_back(reg);
	}

	UniformRegisterSet* urs = getRegisterSetOfType(type);
	if (!urs) {
		urs = new UniformRegisterSet(type);
		m_RegisterSubsets.push_back(urs);
	}
	urs->m_add(reg);
}

Register* CPUArchitecture::getRegister(size_t i) const
{
	assert(i < m_Registers.size() && "FATAL ERROR: Invalid index provided!");
	return m_Registers[i];
}

UniformRegisterSet& CPUArchitecture::getRegisterSet(size_t i) const
{
	assert(i < m_RegisterSubsets.size() && "FATAL ERROR: Invalid index provided!");
	return *m_RegisterSubsets[i];
}

UniformRegisterSet* CPUArchitecture::getRegisterSetOfType(RegisterType t) const
{
	for (std::vector< UniformRegisterSet* >::const_iterator it = m_RegisterSubsets.begin();
		it != m_RegisterSubsets.end(); it++) {
		if ((*it)->getType() == t)
			return *it;
	}
	return 0;
}

} // end-of-namespace: fail
