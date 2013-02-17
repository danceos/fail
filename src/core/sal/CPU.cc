#include "CPU.hpp"

namespace fail {

void CPUArchitecture::m_addRegister(Register* reg)
{
	assert(!reg->isAssigned() && "FATAL ERROR: The register is already assigned!");
	m_Registers.push_back(reg);

	UniformRegisterSet* urs = getRegisterSetOfType(reg->getType());
	if (urs == NULL) {
		urs = new UniformRegisterSet(reg->getType());
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
	return NULL;
}

} // end-of-namespace: fail
