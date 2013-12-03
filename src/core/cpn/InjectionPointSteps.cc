#include "InjectionPoint.hpp"

namespace fail {

void InjectionPointSteps::parseFromInjectionInstr(unsigned instr1, unsigned instr2) {
	// compute hops
	m_ip.set_injection_instr(instr2);
}

} /* namespace fail */
