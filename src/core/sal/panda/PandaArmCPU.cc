#include "PandaArmCPU.hpp"

#include "openocd_wrapper.hpp"

namespace fail {

regdata_t PandaArmCPU::getRegisterContent(const Register* reg) const
{
	regdata_t data;

	// ToDo: ID-translation
	oocdw_read_reg(reg->getId(), ARM_REGS_CORE, &data);
	
	return data;
}

void PandaArmCPU::setRegisterContent(const Register* reg, regdata_t value)
{
	// ToDo: ID-translation
	oocdw_write_reg(reg->getId(), ARM_REGS_CORE, value);
}

} // end-of-namespace: fail
