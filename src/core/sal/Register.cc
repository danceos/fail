#include "Register.hpp"

namespace fail {

Register* UniformRegisterSet::getRegister(size_t i)
{
	assert(i < m_Regs.size() && "FATAL ERROR: Invalid index provided!");
	return m_Regs[i];
}

void UniformRegisterSet::m_add(Register* preg)
{
	assert(!preg->m_Assigned &&
		"FATAL ERROR: The register has already been assigned.");
	m_Regs.push_back(preg);
	preg->m_Assigned = true;
	preg->m_Index = m_Regs.size()-1; // the index within the vector (set)
}

} // end-of-namespace: fail
