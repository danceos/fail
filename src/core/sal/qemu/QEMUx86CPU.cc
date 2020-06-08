#include "QEMUx86CPU.hpp"
#include "../SALConfig.hpp"

#include <cassert>

namespace fail {

regdata_t QEMUX86CPU::getRegisterContent(const Register* reg) const
{
	// TODO
	return 0;
}

void QEMUX86CPU::setRegisterContent(const Register* reg, regdata_t value)
{
	// TODO
}

} // end-of-namespace: fail
