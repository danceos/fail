#include "Gem5ArmCPU.hpp"

#include "Gem5Wrapper.hpp"

namespace fail {

regdata_t Gem5ArmCPU::getRegisterContent(const Register* reg) const
{
	return GetRegisterContent(m_System, m_Id, reg->getId());
}

void Gem5ArmCPU::setRegisterContent(const Register* reg, regdata_t value)
{
	SetRegisterContent(m_System, m_Id, reg->getId(), value);
}

} // end-of-namespace: fail
