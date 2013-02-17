#include "Gem5ArmCPU.hpp"

namespace fail {

regdata_t Gem5ArmCPU::getRegisterContent(Register* reg) const
{
	switch (reg->getType())	{
	case RT_GP:
		if (reg->getIndex() == 15) {
			return m_System->getThreadContext(m_Id)->pcState().pc();
		}
		return m_System->getThreadContext(m_Id)->readIntReg(reg->getIndex());
	case RT_FP:
		return m_System->getThreadContext(m_Id)->readFloatReg(reg->getIndex());
	case RT_ST:
		return m_System->getThreadContext(m_Id)->readMiscReg(reg->getIndex());
	case RT_IP:
		return m_System->getThreadContext(m_Id)->pcState().pc();
	}

	// This shouldn't be reached if a valid register is passed
	// TODO: assertion?
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
	// TODO: assertion?
}

} // end-of-namespace: fail
