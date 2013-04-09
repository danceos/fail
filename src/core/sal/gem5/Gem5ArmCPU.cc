#include "Gem5ArmCPU.hpp"

#include "Gem5Wrapper.hpp"

namespace fail {

regdata_t Gem5ArmCPU::getRegisterContent(Register* reg) const
{
	return GetRegisterContent(m_System, m_Id, reg->getType(), reg->getIndex());
}

void Gem5ArmCPU::setRegisterContent(Register* reg, regdata_t value)
{
	SetRegisterContent(m_System, m_Id, reg->getType(), reg->getIndex(), value);
}

} // end-of-namespace: fail
