#include "InjectionPoint.hpp"

namespace fail {

void InjectionPointSteps::parseFromInjectionInstr(unsigned inj_instr) {
	// compute hops
	m_ip.set_injection_instr(inj_instr);
}

} /* namespace fail */
