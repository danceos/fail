#include <algorithm>
#include "Register.hpp"

namespace fail {

Register* UniformRegisterSet::getRegister(Register::id_t id) const
{
	assert(id < m_Regs.size() && "FATAL ERROR: Invalid index provided!");
	return m_Regs[id];
}

void UniformRegisterSet::m_add(Register* preg)
{
	assert(std::find(m_Regs.begin(), m_Regs.end(), preg) == m_Regs.end());
	m_Regs.push_back(preg);
}

} // end-of-namespace: fail
