#include "Gem5ArmCPU.hpp"

namespace fail {

regdata_t Gem5ArmCPU::getRegisterContent(Register* reg)
{
	switch (reg->getType())	{
	case RT_GP:
		return m_System->getThreadContext(m_Id)->readIntReg(reg->getIndex());
			
	case RT_FP:
		return m_System->getThreadContext(m_Id)->readFloatReg(reg->getIndex());
	
	case RT_ST:
		return m_System->getThreadContext(m_Id)->readMiscReg(reg->getIndex());

	case RT_IP:
		return getRegisterContent(getRegister(RI_IP));
	}

	// This shouldn't be reached if a valid register is passed
	return 0;
}

void Gem5ArmCPU::setRegisterContent(Register* reg, regdata_t value)
{
	switch (reg->getType()) {
	case RT_GP:
		m_System->getThreadContext(m_Id)->setIntReg(reg->getIndex(), value);
	break;

	case RT_FP:
		m_System->getThreadContext(m_Id)->setFloatReg(reg->getIndex(), value);
	break;

	case RT_ST:
		return m_System->getThreadContext(m_Id)->setMiscReg(reg->getIndex(), value);

	case RT_IP:
		return setRegisterContent(getRegister(RI_IP), value);
	}
}

address_t Gem5ArmCPU::getInstructionPointer()
{
	return getRegisterContent(getRegister(RI_IP));
}

address_t Gem5ArmCPU::getStackPointer()
{
	return getRegisterContent(getRegister(RI_SP));
}

address_t Gem5ArmCPU::getLinkRegister()
{
	return getRegisterContent(getRegister(RI_LR));
}

} // end-of-namespace: fail
