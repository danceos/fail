#include "PandaArmCPU.hpp"

#include "openocd_wrapper.hpp"

namespace fail {

regdata_t PandaArmCPU::getRegisterContent(const Register* reg) const
{
	regdata_t data;

	oocdw_read_reg(reg->getId(), &data);

	return data;
}

void PandaArmCPU::setRegisterContent(const Register* reg, regdata_t value)
{
	oocdw_write_reg(reg->getId(), value);
}

} // end-of-namespace: fail
